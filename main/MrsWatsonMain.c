#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedImportStatement"
// Clang thinks that this is unused on MacOSX, however it is definitely
// needed on other platforms.
#include <signal.h>
#pragma clang diagnostic pop

#include <stdio.h>
#include <stdlib.h>
#include "base/PlatformUtilities.h"
#include "MrsWatson.h"
#include "logging/ErrorReporter.h"
#include "logging/EventLogger.h"

// This must be global so that in case of a crash or signal, we can still generate
// a complete error report with a reference
static ErrorReporter gErrorReporter = NULL;

static void handleSignal(int signum) {
  logCritical("Sent signal %d, exiting", signum);
  if(gErrorReporter != NULL && gErrorReporter->started) {
    errorReporterClose(gErrorReporter);
  }
  else {
    logPossibleBug("MrsWatson (or one of its hosted plugins) has encountered a serious error and crashed.");
  }
  exit(RETURN_CODE_SIGNAL + signum);
}

int main(int argc, char* argv[]) {
  gErrorReporter = newErrorReporter();

  // Set up signal handling only after logging is initialized. If we crash before
  // here, something is seriously wrong.
#ifdef SIGHUP
  signal(SIGHUP, handleSignal);
#endif
#ifdef SIGINT
  signal(SIGINT, handleSignal);
#endif
#ifdef SIGQUIT
  signal(SIGQUIT, handleSignal);
#endif
#ifdef SIGILL
  signal(SIGILL, handleSignal);
#endif
#ifdef SIGABRT
  signal(SIGABRT, handleSignal);
#endif
#ifdef SIGFPE
  signal(SIGFPE, handleSignal);
#endif
#ifdef SIGKILL
  signal(SIGKILL, handleSignal);
#endif
#ifdef SIGBUS
  signal(SIGBUS, handleSignal);
#endif
#ifdef SIGSEGV
  signal(SIGSEGV, handleSignal);
#endif
#ifdef SIGSYS
  signal(SIGSYS, handleSignal);
#endif
#ifdef SIGPIPE
  signal(SIGPIPE, handleSignal);
#endif
#ifdef SIGTERM
  signal(SIGTERM, handleSignal);
#endif

  return mrsWatsonMain(gErrorReporter, argc, argv);
}
