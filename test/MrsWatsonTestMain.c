//
//  MrsWatsonTestMain.c
//  MrsWatson
//
//  Created by Nik Reiman on 8/9/12.
//  Copyright (c) 2012 Teragon Audio. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app/ProgramOption.h"
#include "base/FileUtilities.h"
#include "base/PlatformUtilities.h"
#include "base/StringUtilities.h"
#include "unit/ApplicationRunner.h"
#include "unit/TestRunner.h"

#include "MrsWatsonTestMain.h"
#include "MrsWatson.h"

extern TestSuite findTestSuite(char* testSuiteName);
extern TestCase findTestCase(TestSuite testSuite, char* testName);
extern void printInternalTests(void);
extern void runInternalTestSuite(boolByte onlyPrintFailing);
extern int runApplicationTestSuite(TestEnvironment testEnvironment);

static const char* DEFAULT_TEST_SUITE_NAME = "all";

#if UNIX
static const char* DEFAULT_MRSWATSON_PATH = "mrswatson";
static const char* DEFAULT_RESOURCES_PATH = "../share";
#elif WINDOWS
static const char* DEFAULT_MRSWATSON_PATH = "mrswatson.exe";
static const char* DEFAULT_RESOURCES_PATH = "..\\share";
#else
static const char* DEFAULT_MRSWATSON_PATH = "mrswatson";
static const char* DEFAULT_RESOURCES_PATH = "share";
#endif

static ProgramOptions newTestProgramOptions(void) {
  ProgramOptions programOptions = newProgramOptions(NUM_TEST_OPTIONS);
  
  programOptionsAdd(programOptions, newProgramOptionWithValues(OPTION_TEST_SUITE, "suite",
    "Choose a test suite to run. Current suites include:\n\
\t- Application run audio quality tests against actual executable\n\
\t- Internal: run all internal function tests\n\
\t- All: run all tests (default)\n\
\t- A suite name (use '--list' to see all suite names)",
    true, kProgramOptionArgumentTypeRequired, NO_DEFAULT_VALUE));
  programOptionsAdd(programOptions, newProgramOptionWithValues(OPTION_TEST_NAME, "test",
    "Run a single test. Tests are named 'Suite:Name', for example:\n\
\t-t 'LinkedList:Append Item'",
    true, kProgramOptionArgumentTypeRequired, NO_DEFAULT_VALUE));
  programOptionsAdd(programOptions, newProgramOptionWithValues(OPTION_TEST_PRINT_TESTS, "list-tests",
    "List all internal tests in the same format required by --test",
    true, kProgramOptionArgumentTypeNone, NO_DEFAULT_VALUE));
  programOptionsAdd(programOptions, newProgramOptionWithValues(OPTION_TEST_MRSWATSON_PATH, "mrswatson-path",
    "Path to mrswatson executable. Only required for running application test suite.",
    true, kProgramOptionArgumentTypeRequired, NO_DEFAULT_VALUE));
  programOptionsAdd(programOptions, newProgramOptionWithValues(OPTION_TEST_RESOURCES_PATH, "resources",
    "Path to resources directory. Only required for running application test suite.",
    true, kProgramOptionArgumentTypeRequired, NO_DEFAULT_VALUE));
  /* TODO: Finish this option
  addNewProgramOption(programOptions, newProgramOptionWithValues(OPTION_TEST_LOG_FILE, "log-file",
    "Save test output to log file",
    true, ARGUMENT_TYPE_REQUIRED, NO_DEFAULT_VALUE);
  */
  programOptionsAdd(programOptions, newProgramOptionWithValues(OPTION_TEST_PRINT_ONLY_FAILING, "failing",
    "Print only failing tests. Note that if a test causes the suite to crash, the \
bad test's name will not be printed. In this case, re-run without this option, as \
the test names will be printed before the tests are executed.",
    true, kProgramOptionArgumentTypeNone, NO_DEFAULT_VALUE));
  programOptionsAdd(programOptions, newProgramOptionWithValues(OPTION_TEST_HELP, "help",
    "Print full program help (this screen), or just the help for a single argument.",
    true, kProgramOptionArgumentTypeOptional, NO_DEFAULT_VALUE));

  return programOptions;
}

int main(int argc, char* argv[]) {
  ProgramOptions programOptions;
  int totalTestsFailed = 0;
  CharString testSuiteToRun;
  CharString executablePath;
  CharString currentPath;
  CharString mrsWatsonPath;
  CharString resourcesPath;
  boolByte runInternalTests = false;
  boolByte runApplicationTests = false;
  TestCase testCase;
  TestSuite testSuite;
  TestEnvironment testEnvironment;
  char* testArgument;
  char* colon;
  char* testCaseName;
  char* testSuiteName;

  programOptions = newTestProgramOptions();
  if(!programOptionsParseArgs(programOptions, argc, argv)) {
    printf("Or run %s --help (option) to see help for a single option\n", getFileBasename(argv[0]));
    return -1;
  }

  if(programOptions->options[OPTION_TEST_HELP]->enabled) {
    printf("Run with '--help full' to see extended help for all options.\n");
    if(charStringIsEmpty(programOptions->options[OPTION_TEST_HELP]->argument)) {
      printf("All options, where <argument> is required and [argument] is optional\n");
      programOptionsPrintHelp(programOptions, false, DEFAULT_INDENT_SIZE);
    }
    else {
      programOptionsPrintHelp(programOptions, true, DEFAULT_INDENT_SIZE);
    }
    return -1;
  }
  else if(programOptions->options[OPTION_TEST_PRINT_TESTS]->enabled) {
    printInternalTests();
    return -1;
  }

  testSuiteToRun = newCharString();
  if(programOptions->options[OPTION_TEST_SUITE]->enabled) {
    charStringCopy(testSuiteToRun, programOptions->options[OPTION_TEST_SUITE]->argument);
  }
  if(charStringIsEmpty(testSuiteToRun)) {
    charStringCopyCString(testSuiteToRun, DEFAULT_TEST_SUITE_NAME);
  }

  if(programOptions->options[OPTION_TEST_NAME]->enabled) {
    runInternalTests = false;
    runApplicationTests = false;

    testArgument = programOptions->options[OPTION_TEST_NAME]->argument->data;
    colon = strchr(testArgument, ':');
    if(colon == NULL) {
      printf("ERROR: Invalid test name");
      programOptionPrintHelp(programOptions->options[OPTION_TEST_NAME], true, DEFAULT_INDENT_SIZE, 0);
      return -1;
    }
    testCaseName = strdup(colon + 1);
    *colon = '\0';
    testSuiteName = strdup(programOptions->options[OPTION_TEST_NAME]->argument->data);
    testSuite = findTestSuite(testSuiteName);
    if(testSuite == NULL) {
      printf("ERROR: Could not find test suite '%s'\n", testSuiteName);
      return -1;
    }
    testCase = findTestCase(testSuite, testCaseName);
    if(testCase == NULL) {
      printf("ERROR: Could not find test case '%s'\n", testCaseName);
      return -1;
    }
    else {
      printf("Running test in %s:\n", testSuite->name);
      runTestCase(testCase, testSuite);
    }
  }
  else if(charStringIsEqualToCString(testSuiteToRun, "all", true)) {
    runInternalTests = true;
    runApplicationTests = true;
  }
  else if(charStringIsEqualToCString(testSuiteToRun, "internal", true)) {
    runInternalTests = true;
  }
  else if(charStringIsEqualToCString(testSuiteToRun, "application", true)) {
    runApplicationTests = true;
  }
  else {
    testSuite = findTestSuite(testSuiteToRun->data);
    if(testSuite == NULL) {
      printf("ERROR: Invalid test suite '%s'\n", testSuiteToRun->data);
      printf("Run '%s --list' suite to show possible test suites\n", getFileBasename(argv[0]));
      return -1;
    }
    else {
      testSuite->onlyPrintFailing = programOptions->options[OPTION_TEST_PRINT_ONLY_FAILING]->enabled;
      runTestSuite(testSuite, NULL);
    }
  }

  if(runInternalTests) {
    printf("=== Internal tests ===\n");
    runInternalTestSuite(programOptions->options[OPTION_TEST_PRINT_ONLY_FAILING]->enabled);
    totalTestsFailed = 0;
  }

  // Find out where this program is being run from to find mrswatson and resources
  executablePath = getExecutablePath();
  currentPath = newCharString();
  getFileDirname(executablePath, currentPath);

  mrsWatsonPath = newCharString();
  if(programOptions->options[OPTION_TEST_MRSWATSON_PATH]->enabled) {
    charStringCopy(mrsWatsonPath, programOptions->options[OPTION_TEST_MRSWATSON_PATH]->argument);
  }
  else {
    buildAbsolutePath(currentPath, newCharStringWithCString(DEFAULT_MRSWATSON_PATH), NULL, mrsWatsonPath);
  }
  if(runApplicationTests && !fileExists(mrsWatsonPath->data)) {
    printf("Could not find mrswatson at '%s', skipping application tests\n", mrsWatsonPath->data);
    runApplicationTests = false;
  }

  resourcesPath = newCharString();
  if(programOptions->options[OPTION_TEST_RESOURCES_PATH]->enabled) {
    charStringCopy(resourcesPath, programOptions->options[OPTION_TEST_RESOURCES_PATH]->argument);
  }
  else {
    buildAbsolutePath(currentPath, newCharStringWithCString(DEFAULT_RESOURCES_PATH), NULL, resourcesPath);
  }
  if(runApplicationTests && !fileExists(resourcesPath->data)) {
    printf("Could not find test resources at '%s', skipping application tests\n", resourcesPath->data);
    runApplicationTests = false;
  }

  if(runApplicationTests) {
    printf("\n=== Application tests ===\n");
    testEnvironment = newTestEnvironment(mrsWatsonPath->data, resourcesPath->data);
    testEnvironment->results->onlyPrintFailing = programOptions->options[OPTION_TEST_PRINT_ONLY_FAILING]->enabled;
    totalTestsFailed += runApplicationTestSuite(testEnvironment);
  }

  printf("\n=== Finished with %d total failed tests ===\n", totalTestsFailed);
  return totalTestsFailed;
}
