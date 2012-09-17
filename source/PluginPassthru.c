//
// PluginPassthru.c - MrsWatson
// Created by Nik Reiman on 8/17/12.
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

#include <stdlib.h>
#include "PluginPassthru.h"

static void _pluginPassthruEmpty(void* pluginPtr) {
  // Nothing to do here
}

static boolByte _pluginPassthruOpen(void* pluginPtr) {
  return true;
}

static int _pluginPassthruGetSetting(void* pluginPtr, PluginSetting pluginSetting) {
  return 0;
}

static void _pluginPassthruProcessAudio(void* pluginPtr, SampleBuffer inputs, SampleBuffer outputs) {
  copySampleBuffers(outputs, inputs);
}

static void _pluginPassthruProcessMidiEvents(void* pluginPtr, LinkedList midiEvents) {
  // Nothing to do here
}

static void _pluginPassthruSetParameter(void* pluginPtr, int i, float value) {
  // Nothing to do here
}

Plugin newPluginPassthru(void) {
  Plugin plugin = (Plugin)malloc(sizeof(PluginMembers));

  plugin->interfaceType = PLUGIN_TYPE_PASSTHRU;
  plugin->pluginType = PLUGIN_TYPE_EFFECT;
  plugin->pluginName = newCharString();
  copyToCharString(plugin->pluginName, "Passthru");
  plugin->pluginLocation = newCharString();
  copyToCharString(plugin->pluginLocation, "(Internal)");

  plugin->open = _pluginPassthruOpen;
  plugin->displayInfo = _pluginPassthruEmpty;
  plugin->getSetting = _pluginPassthruGetSetting;
  plugin->processAudio = _pluginPassthruProcessAudio;
  plugin->processMidiEvents = _pluginPassthruProcessMidiEvents;
  plugin->setParameter = _pluginPassthruSetParameter;
  plugin->closePlugin = _pluginPassthruEmpty;
  plugin->freePluginData = _pluginPassthruEmpty;

  plugin->extraData = NULL;
  return plugin;
}
