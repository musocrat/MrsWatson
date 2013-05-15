//
// SampleSource.h - MrsWatson
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

#ifndef MrsWatson_SampleSource_h
#define MrsWatson_SampleSource_h

#include "audio/SampleBuffer.h"
#include "base/CharString.h"
#include "base/Types.h"

// Force all samples to be within {1.0, -1.0} range. This uses a bit of extra
// CPU, and I'm not sure it's even necessary, so it is disabled at present.
#ifndef USE_BRICKWALL_LIMITER
#define USE_BRICKWALL_LIMITER 0
#endif

// Define to 1 to use audiofile to read files, otherwise an internal WAVE
// sample source will be used instead. AIFF and other file types (except for
// PCM) are not supported, hence the migration to libaudiofile.
// However, if only PCM or WAVE support is needed, this can be disabled and
// a much smaller binary will be produced as a result.
#ifndef USE_LIBAUDIOFILE
#define USE_LIBAUDIOFILE 1
#endif

// Support FLAC audio via audiofile
#ifndef USE_LIBFLAC
#define USE_LIBFLAC 0
#endif

typedef enum {
  SAMPLE_SOURCE_TYPE_INVALID,
  SAMPLE_SOURCE_TYPE_SILENCE,
#if USE_LIBAUDIOFILE
  SAMPLE_SOURCE_TYPE_AIFF,
#if USE_LIBFLAC
  SAMPLE_SOURCE_TYPE_FLAC,
#endif
#endif
  SAMPLE_SOURCE_TYPE_PCM,
  SAMPLE_SOURCE_TYPE_WAVE,
  NUM_SAMPLE_SOURCES
} SampleSourceType;
#define DEFAULT_OUTPUT_SOURCE_TYPE SAMPLE_SOURCE_TYPE_WAVE

typedef enum {
  SAMPLE_SOURCE_OPEN_NOT_OPENED,
  SAMPLE_SOURCE_OPEN_READ,
  SAMPLE_SOURCE_OPEN_WRITE,
  NUM_SAMPLE_SOURCE_OPEN_AS
} SampleSourceOpenAs;

typedef boolByte (*OpenSampleSourceFunc)(void*, const SampleSourceOpenAs);
typedef boolByte (*ReadSampleBlockFunc)(void*, SampleBuffer);
typedef boolByte (*WriteSampleBlockFunc)(void*, const SampleBuffer);
typedef void (*CloseSampleSourceFunc)(void*);
typedef void (*FreeSampleSourceDataFunc)(void*);

typedef struct {
  SampleSourceType sampleSourceType;
  SampleSourceOpenAs openedAs;
  CharString sourceName;
  unsigned long numSamplesProcessed;

  OpenSampleSourceFunc openSampleSource;
  ReadSampleBlockFunc readSampleBlock;
  WriteSampleBlockFunc writeSampleBlock;
  CloseSampleSourceFunc closeSampleSource;
  FreeSampleSourceDataFunc freeSampleSourceData;

  void* extraData;
} SampleSourceMembers;
typedef SampleSourceMembers* SampleSource;

SampleSource newSampleSource(SampleSourceType sampleSourceType, const CharString sampleSourceName);

void sampleSourcePrintSupportedTypes(void);
SampleSourceType sampleSourceGuess(const CharString sampleSourceTypeString);
boolByte sampleSourceIsStreaming(SampleSource sampleSource);

void freeSampleSource(SampleSource sampleSource);

#endif
