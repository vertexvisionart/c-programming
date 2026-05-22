#include "logger.h"
#include <stdarg.h>
#include <stdio.h>

static FILE *current_log_file = NULL;
static int log_calls = 0;

void logSetFile(FILE *file) { current_log_file = file; }

void logPrintf(const char *format, ...) {
  if (current_log_file != NULL) {
    va_list args;
    va_start(args, format);
    vfprintf(current_log_file, format, args);
    va_end(args);
    log_calls++;
  }
}

int getLogCallsCount() { return log_calls; }
