#include "unit/TestRunner.h"
#include "audio/AudioSettings.h"
#include "midi/MidiEvent.h"
#include "plugin/PluginChain.h"
#include "plugin/PluginPassthru.h"

#include "PluginMock.h"
#include "PluginPresetMock.h"

static void _pluginChainTestSetup(void) {
  initPluginChain();
}

static void _pluginChainTestTeardown(void) {
  freePluginChain(getPluginChain());
}

static int _testInitPluginChain(void) {
  PluginChain p = getPluginChain();
  assertIntEquals(p->numPlugins, 0);
  assertNotNull(p->plugins);
  assertNotNull(p->presets);
  return 0;
}

static int _testAddFromArgumentStringNull(void) {
  PluginChain p = getPluginChain();
  CharString c = newCharStringWithCString("/");

  assertFalse(pluginChainAddFromArgumentString(p, NULL, c));
  assertIntEquals(p->numPlugins, 0);

  freeCharString(c);
  return 0;
}

static int _testAddFromArgumentStringEmpty(void) {
  PluginChain p = getPluginChain();
  CharString c = newCharStringWithCString("/");
  CharString empty = newCharString();

  assertFalse(pluginChainAddFromArgumentString(p, empty, c));
  assertIntEquals(p->numPlugins, 0);

  freeCharString(c);
  freeCharString(empty);
  return 0;
}

static int _testAddFromArgumentStringEmptyLocation(void) {
  PluginChain p = getPluginChain();
  CharString c = newCharStringWithCString(kInternalPluginPassthruName);
  CharString empty = newCharString();

  assert(pluginChainAddFromArgumentString(p, c, empty));
  assertIntEquals(p->numPlugins, 1);

  freeCharString(c);
  freeCharString(empty);
  return 0;
}

static int _testAddFromArgumentStringNullLocation(void) {
  PluginChain p = getPluginChain();
  CharString c = newCharStringWithCString(kInternalPluginPassthruName);

  assert(pluginChainAddFromArgumentString(p, c, NULL));
  assertIntEquals(p->numPlugins, 1);

  freeCharString(c);
  return 0;
}

static int _testAddFromArgumentString(void) {
  PluginChain p = getPluginChain();
  CharString testArgs = newCharStringWithCString(kInternalPluginPassthruName);

  assert(pluginChainAddFromArgumentString(p, testArgs, NULL));
  assertIntEquals(p->numPlugins, 1);
  assertNotNull(p->plugins[0]);
  assertIntEquals(p->plugins[0]->pluginType, PLUGIN_TYPE_INTERNAL);
  assertCharStringEquals(p->plugins[0]->pluginName, kInternalPluginPassthruName);

  freeCharString(testArgs);
  return 0;
}

static int _testAddFromArgumentStringMultiple(void) {
  PluginChain p = getPluginChain();
  CharString testArgs = newCharStringWithCString(kInternalPluginPassthruName);
  unsigned int i;

  assert(pluginChainAddFromArgumentString(p, testArgs, NULL));
  assert(pluginChainAddFromArgumentString(p, testArgs, NULL));
  assertIntEquals(p->numPlugins, 2);
  for(i = 0; i < p->numPlugins; i++) {
    assertNotNull(p->plugins[i]);
    assertIntEquals(p->plugins[i]->pluginType, PLUGIN_TYPE_INTERNAL);
    assertCharStringEquals(p->plugins[i]->pluginName, kInternalPluginPassthruName);
  }

  freeCharString(testArgs);
  return 0;
}

static int _testAddPluginWithPresetFromArgumentString(void) {
  PluginChain p = getPluginChain();
  CharString testArgs = newCharStringWithCString("mrs_passthru,testPreset.fxp");

  assert(pluginChainAddFromArgumentString(p, testArgs, NULL));
  assertIntEquals(p->numPlugins, 1);
  assertIntEquals(p->plugins[0]->pluginType, PLUGIN_TYPE_INTERNAL);
  assertCharStringEquals(p->plugins[0]->pluginName, kInternalPluginPassthruName);
  assertNotNull(p->presets[0]);
  assertCharStringEquals(p->presets[0]->presetName, "testPreset.fxp");

  freeCharString(testArgs);
  return 0;
}

static int _testAddFromArgumentStringWithPresetSpaces(void) {
  PluginChain p = getPluginChain();
  CharString testArgs = newCharStringWithCString("mrs_passthru,test preset.fxp");

  assert(pluginChainAddFromArgumentString(p, testArgs, NULL));
  assertIntEquals(p->numPlugins, 1);
  assertIntEquals(p->plugins[0]->pluginType, PLUGIN_TYPE_INTERNAL);
  assertCharStringEquals(p->plugins[0]->pluginName, kInternalPluginPassthruName);
  assertNotNull(p->presets[0]);
  assertCharStringEquals(p->presets[0]->presetName, "test preset.fxp");

  freeCharString(testArgs);
  return 0;
}

static int _testAppendPlugin(void) {
  Plugin mock = newPluginMock();
  PluginChain p = getPluginChain();

  assert(pluginChainAppend(p, mock, NULL));

  return 0;
}

static int _testAppendWithNullPlugin(void) {
  PluginChain p = getPluginChain();

  assertFalse(pluginChainAppend(p, NULL, NULL));

  return 0;  
}

static int _testAppendWithPreset(void) {
  Plugin mock = newPluginMock();
  PluginChain p = getPluginChain();
  PluginPreset mockPreset = newPluginPresetMock();

  assert(pluginChainAppend(p, mock, mockPreset));

  return 0;  
}

static int _testInitializePluginChain(void) {
  Plugin mock = newPluginMock();
  PluginChain p = getPluginChain();
  PluginPreset mockPreset = newPluginPresetMock();

  assert(pluginChainAppend(p, mock, mockPreset));
  pluginChainInitialize(p);
  assert(((PluginMockData)mock->extraData)->isOpen);
  assert(((PluginPresetMockData)mockPreset->extraData)->isOpen);
  assert(((PluginPresetMockData)mockPreset->extraData)->isLoaded);

  return 0;
}

static int _testGetMaximumTailTime(void) {
  Plugin mock = newPluginMock();
  PluginChain p = getPluginChain();
  int maxTailTime = 0;

  assert(pluginChainAppend(p, mock, NULL));
  maxTailTime = pluginChainGetMaximumTailTimeInMs(p);
  assertIntEquals(maxTailTime, kPluginMockTailTime);

  return 0;
}

static int _testPrepareForProcessing(void) {
  Plugin mock = newPluginMock();
  PluginChain p = getPluginChain();

  assert(pluginChainAppend(p, mock, NULL));
  assertIntEquals(pluginChainInitialize(p), RETURN_CODE_SUCCESS);
  pluginChainPrepareForProcessing(p);
  assert(((PluginMockData)mock->extraData)->isPrepared);

  return 0;  
}

static int _testProcessPluginChainAudio(void) {
  Plugin mock = newPluginMock();
  PluginChain p = getPluginChain();
  SampleBuffer inBuffer = newSampleBuffer(DEFAULT_NUM_CHANNELS, DEFAULT_BLOCKSIZE);
  SampleBuffer outBuffer = newSampleBuffer(DEFAULT_NUM_CHANNELS, DEFAULT_BLOCKSIZE);

  assert(pluginChainAppend(p, mock, NULL));
  pluginChainProcessAudio(p, inBuffer, outBuffer);
  assert(((PluginMockData)mock->extraData)->processAudioCalled);

  freeSampleBuffer(inBuffer);
  freeSampleBuffer(outBuffer);
  return 0;
}

static int _testProcessPluginChainAudioRealtime(void) {
  Plugin mock = newPluginMock();
  PluginChain p = getPluginChain();
  SampleBuffer inBuffer = newSampleBuffer(DEFAULT_NUM_CHANNELS, DEFAULT_BLOCKSIZE);
  SampleBuffer outBuffer = newSampleBuffer(DEFAULT_NUM_CHANNELS, DEFAULT_BLOCKSIZE);
  TaskTimer t = newTaskTimerWithCString("test", "test");

  assert(pluginChainAppend(p, mock, NULL));
  pluginChainSetRealtime(p, true);
  taskTimerStart(t);
  pluginChainProcessAudio(p, inBuffer, outBuffer);
  assertTimeEquals(taskTimerStop(t), 1000 * DEFAULT_BLOCKSIZE / getSampleRate(), 0.1);
  assert(((PluginMockData)mock->extraData)->processAudioCalled);

  freeTaskTimer(t);
  freeSampleBuffer(inBuffer);
  freeSampleBuffer(outBuffer);
  return 0;
}

static int _testProcessPluginChainMidiEvents(void) {
  Plugin mock = newPluginMock();
  PluginChain p = getPluginChain();
  SampleBuffer inBuffer = newSampleBuffer(DEFAULT_NUM_CHANNELS, DEFAULT_BLOCKSIZE);
  SampleBuffer outBuffer = newSampleBuffer(DEFAULT_NUM_CHANNELS, DEFAULT_BLOCKSIZE);
  LinkedList list = newLinkedList();
  MidiEvent midi = newMidiEvent();

  linkedListAppend(list, midi);
  assert(pluginChainAppend(p, mock, NULL));
  pluginChainProcessMidi(p, list);
  assert(((PluginMockData)mock->extraData)->processMidiCalled);

  freeMidiEvent(midi);
  freeLinkedList(list);
  freeSampleBuffer(inBuffer);
  freeSampleBuffer(outBuffer);
  return 0;
}

static int _testShutdown(void) {
  Plugin mock = newPluginMock();
  PluginChain p = getPluginChain();

  assert(pluginChainAppend(p, mock, NULL));
  pluginChainShutdown(p);
  assertFalse(((PluginMockData)mock->extraData)->isOpen);

  return 0;
}

TestSuite addPluginChainTests(void);
TestSuite addPluginChainTests(void) {
  TestSuite testSuite = newTestSuite("PluginChain", _pluginChainTestSetup, _pluginChainTestTeardown);
  addTest(testSuite, "Initialization", _testInitPluginChain);
  addTest(testSuite, "AddFromArgumentStringNull", _testAddFromArgumentStringNull);
  addTest(testSuite, "AddFromArgumentStringEmpty", _testAddFromArgumentStringEmpty);
  addTest(testSuite, "AddFromArgumentStringEmptyLocation", _testAddFromArgumentStringEmptyLocation);
  addTest(testSuite, "AddFromArgumentStringNullLocation", _testAddFromArgumentStringNullLocation);
  addTest(testSuite, "AddFromArgumentString", _testAddFromArgumentString);
  addTest(testSuite, "AddFromArgumentStringMultiple", _testAddFromArgumentStringMultiple);
  addTest(testSuite, "AddPluginWithPresetFromArgumentString", _testAddPluginWithPresetFromArgumentString);
  addTest(testSuite, "AddFromArgumentStringWithPresetSpaces", _testAddFromArgumentStringWithPresetSpaces);
  addTest(testSuite, "AppendPlugin", _testAppendPlugin);
  addTest(testSuite, "AppendWithNullPlugin", _testAppendWithNullPlugin);
  addTest(testSuite, "AppendWithPreset", _testAppendWithPreset);
  addTest(testSuite, "InitializePluginChain", _testInitializePluginChain);

  addTest(testSuite, "GetMaximumTailTime", _testGetMaximumTailTime);

  addTest(testSuite, "PrepareForProcessing", _testPrepareForProcessing);
  addTest(testSuite, "ProcessPluginChainAudio", _testProcessPluginChainAudio);
  addTest(testSuite, "ProcessPluginChainAudioRealtime", _testProcessPluginChainAudioRealtime);
  addTest(testSuite, "ProcessPluginChainMidiEvents", _testProcessPluginChainMidiEvents);

  addTest(testSuite, "Shutdown", _testShutdown);

  return testSuite;
}
