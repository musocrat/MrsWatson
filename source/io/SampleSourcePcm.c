//
// SampleSourcePcm.c - MrsWatson
// Created by Nik Reiman on 1/2/12.
// Copyright (c) 2012 Teragon Audio. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio/AudioSettings.h"
#include "base/PlatformUtilities.h"
#include "io/SampleSourcePcm.h"
#include "logging/EventLogger.h"

boolByte isValidBitDepth(unsigned short bitDepth) {
  switch(bitDepth) {
    case 8:
    case 16:
    case 24:
    case 32:
      return true;
    default:
      return false;
  }
}

static boolByte openSampleSourcePcm(void* sampleSourcePtr, const SampleSourceOpenAs openAs) {
  SampleSource sampleSource = (SampleSource)sampleSourcePtr;
  SampleSourcePcmData extraData = (SampleSourcePcmData)(sampleSource->extraData);

  extraData->dataBufferNumItems = 0;
  if(openAs == SAMPLE_SOURCE_OPEN_READ) {
    if(charStringIsEqualToCString(sampleSource->sourceName, "-", false)) {
      extraData->fileHandle = stdin;
      charStringCopyCString(sampleSource->sourceName, "stdin");
      extraData->isStream = true;
    }
    else {
      extraData->fileHandle = fopen(sampleSource->sourceName->data, "rb");
    }
  }
  else if(openAs == SAMPLE_SOURCE_OPEN_WRITE) {
    if(charStringIsEqualToCString(sampleSource->sourceName, "-", false)) {
      extraData->fileHandle = stdout;
      charStringCopyCString(sampleSource->sourceName, "stdout");
      extraData->isStream = true;
    }
    else {
      extraData->fileHandle = fopen(sampleSource->sourceName->data, "wb");
    }
  }
  else {
    logInternalError("Invalid type for openAs in PCM file");
    return false;
  }

  if(extraData->fileHandle == NULL) {
    logError("PCM File '%s' could not be opened for %s",
      sampleSource->sourceName->data, openAs == SAMPLE_SOURCE_OPEN_READ ? "reading" : "writing");
    return false;
  }

  sampleSource->openedAs = openAs;
  return true;
}

size_t sampleSourcePcmRead(SampleSourcePcmData self, SampleBuffer sampleBuffer) {
  size_t pcmSamplesRead = 0;
  size_t sampleSize = 0;
  void* sampleData = NULL;

  if(self == NULL || self->fileHandle == NULL) {
    logCritical("Corrupt PCM data structure");
    return 0;
  }

  if(self->dataBufferNumItems == 0) {
    self->dataBufferNumItems = (size_t)(sampleBuffer->numChannels * sampleBuffer->blocksize);
    switch(self->bitDepth) {
      case 8:
      case 16:
        self->interlacedPcmBuffer.shorts = (short*)malloc(sizeof(short) * self->dataBufferNumItems);
        break;
      case 24:
      case 32:
        self->interlacedPcmBuffer.ints = (int*)malloc(sizeof(int) * self->dataBufferNumItems);
        break;
      default:
        logInternalError("Invalid bit depth in sampleSourcePcmRead");
        return 0;
    }
  }

  switch(self->bitDepth) {
    case 8:
    case 16:
      sampleSize = sizeof(short);
      sampleData = self->interlacedPcmBuffer.shorts;
      break;
    case 24:
    case 32:
      sampleSize = sizeof(int);
      sampleData = self->interlacedPcmBuffer.ints;
      break;
    default:
      logInternalError("Invalid bit depth in sampleSourcePcmRead");
      return 0;
  }

  // Clear the PCM data buffer, or else the last block will have dirty samples in the end
  memset(sampleData, 0, sampleSize * self->dataBufferNumItems);

  pcmSamplesRead = fread(sampleData, sampleSize, self->dataBufferNumItems, self->fileHandle);
  if(pcmSamplesRead < self->dataBufferNumItems) {
    logDebug("End of PCM file reached");
    // Set the blocksize of the sample buffer to be the number of frames read
    sampleBuffer->blocksize = pcmSamplesRead / sampleBuffer->numChannels;
  }
  logDebug("Read %d samples from PCM file", pcmSamplesRead);

  sampleBufferCopyPcmSamples(sampleBuffer, sampleData, self->bitDepth);
  return pcmSamplesRead;
}

static boolByte readBlockFromPcmFile(void* sampleSourcePtr, SampleBuffer sampleBuffer) {
  SampleSource sampleSource = (SampleSource)sampleSourcePtr;
  SampleSourcePcmData extraData = (SampleSourcePcmData)(sampleSource->extraData);
  size_t originalBlocksize = sampleBuffer->blocksize;
  size_t samplesRead = sampleSourcePcmRead(extraData, sampleBuffer);
  sampleSource->numSamplesProcessed += (unsigned long)samplesRead;
  return (boolByte)(originalBlocksize == sampleBuffer->blocksize);
}

size_t sampleSourcePcmWrite(SampleSourcePcmData self, const SampleBuffer sampleBuffer) {
  size_t pcmSamplesWritten = 0;
  size_t numSamplesToWrite = (size_t)(sampleBuffer->numChannels * sampleBuffer->blocksize);
  // Right now we only support writing 16-bit audio files, otherwise the option
  // arguments get really complicated. However supporting different bit rates
  // shouldn't be impossible here.
  size_t sampleSize = sizeof(short);

  if(self == NULL || self->fileHandle == NULL) {
    logCritical("Corrupt PCM data structure");
    return false;
  }

  if(self->dataBufferNumItems == 0) {
    self->dataBufferNumItems = (size_t)(sampleBuffer->numChannels * sampleBuffer->blocksize);
    self->interlacedPcmBuffer.shorts = (short*)malloc(sampleSize * self->dataBufferNumItems);
  }

  // Clear the PCM data buffer just to be safe
  memset(self->interlacedPcmBuffer.shorts, 0, sampleSize * self->dataBufferNumItems);

  sampleBufferGetPcmSamples(sampleBuffer, self->interlacedPcmBuffer.shorts,
    self->bitDepth, (boolByte)(self->isLittleEndian != isHostLittleEndian()));
  pcmSamplesWritten = fwrite(self->interlacedPcmBuffer.shorts, sampleSize, numSamplesToWrite, self->fileHandle);
  if(pcmSamplesWritten < numSamplesToWrite) {
    logWarn("Short write to PCM file");
    return pcmSamplesWritten;
  }

  logDebug("Wrote %d samples to PCM file", pcmSamplesWritten);
  return pcmSamplesWritten;
}

static boolByte writeBlockToPcmFile(void* sampleSourcePtr, const SampleBuffer sampleBuffer) {
  SampleSource sampleSource = (SampleSource)sampleSourcePtr;
  SampleSourcePcmData extraData = (SampleSourcePcmData)(sampleSource->extraData);
  size_t samplesWritten = sampleSourcePcmWrite(extraData, sampleBuffer);
  sampleSource->numSamplesProcessed += samplesWritten;
  return (boolByte)(samplesWritten == sampleBuffer->blocksize);
}

static void _closeSampleSourcePcm(void* sampleSourcePtr) {
  SampleSource sampleSource = (SampleSource)sampleSourcePtr;
  SampleSourcePcmData extraData = (SampleSourcePcmData)sampleSource->extraData;
  if(extraData->fileHandle != NULL) {
    fclose(extraData->fileHandle);
  }
}

void sampleSourcePcmSetBitDepth(void* sampleSourcePtr, unsigned short bitDepth) {
  SampleSource sampleSource = (SampleSource)sampleSourcePtr;
  SampleSourcePcmData extraData = (SampleSourcePcmData)sampleSource->extraData;
  extraData->bitDepth = bitDepth;
}

void sampleSourcePcmSetNumChannels(void* sampleSourcePtr, unsigned short numChannels) {
  SampleSource sampleSource = (SampleSource)sampleSourcePtr;
  SampleSourcePcmData extraData = (SampleSourcePcmData)sampleSource->extraData;
  extraData->numChannels = numChannels;
}

void sampleSourcePcmSetSampleRate(void* sampleSourcePtr, double sampleRate) {
  SampleSource sampleSource = (SampleSource)sampleSourcePtr;
  SampleSourcePcmData extraData = (SampleSourcePcmData)sampleSource->extraData;
  extraData->sampleRate = (unsigned int)sampleRate;
}

void freeSampleSourceDataPcm(void* sampleSourceDataPtr) {
  SampleSourcePcmData extraData = (SampleSourcePcmData)sampleSourceDataPtr;
  switch(extraData->bitDepth) {
    case 8:
    case 16:
      free(extraData->interlacedPcmBuffer.shorts);
      break;
    case 24:
    case 32:
      free(extraData->interlacedPcmBuffer.ints);
      break;
    default:
      break;
  }
  free(extraData);
}

SampleSource _newSampleSourcePcm(const CharString sampleSourceName) {
  SampleSource sampleSource = (SampleSource)malloc(sizeof(SampleSourceMembers));
  SampleSourcePcmData extraData = (SampleSourcePcmData)malloc(sizeof(SampleSourcePcmDataMembers));

  sampleSource->sampleSourceType = SAMPLE_SOURCE_TYPE_PCM;
  sampleSource->openedAs = SAMPLE_SOURCE_OPEN_NOT_OPENED;
  sampleSource->sourceName = newCharString();
  charStringCopy(sampleSource->sourceName, sampleSourceName);
  sampleSource->numSamplesProcessed = 0;

  sampleSource->openSampleSource = openSampleSourcePcm;
  sampleSource->readSampleBlock = readBlockFromPcmFile;
  sampleSource->writeSampleBlock = writeBlockToPcmFile;
  sampleSource->closeSampleSource = _closeSampleSourcePcm;
  sampleSource->freeSampleSourceData = freeSampleSourceDataPcm;

  extraData->isStream = false;
  extraData->isLittleEndian = true;
  extraData->fileHandle = NULL;
  extraData->dataBufferNumItems = 0;
  // Since this is a union with members of equal sizes (2 pointers), we can just
  // set one of them to NULL and effectively NULL the entire structure.
  extraData->interlacedPcmBuffer.ints = NULL;

  extraData->numChannels = getNumChannels();
  extraData->bitDepth = DEFAULT_BIT_DEPTH;
  extraData->sampleRate = (unsigned int)getSampleRate();
  sampleSource->extraData = extraData;

  return sampleSource;
}
