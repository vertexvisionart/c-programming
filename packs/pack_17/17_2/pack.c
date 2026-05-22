#include "pack.h"
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

int pack(char *buffer, const char *format, ...) {
  va_list args;
  va_start(args, format);

  int total_bytes = 0;
  const char *p = format;

  while (*p != '\0') {
    if (*p == '%') {
      p++;
      if (*p == 'd') {
        int val = va_arg(args, int);
        if (buffer != NULL) {
          memcpy(buffer + total_bytes, &val, sizeof(int));
        }
        total_bytes += sizeof(int);
      } else if (*p == 'l' && *(p + 1) == 'f') {
        p++;

        double val = va_arg(args, double);
        if (buffer != NULL) {
          memcpy(buffer + total_bytes, &val, sizeof(double));
        }
        total_bytes += sizeof(double);
      } else if (*p == 's') {
        const char *str = va_arg(args, const char *);
        size_t len = strlen(str) + 1;
        if (buffer != NULL) {
          memcpy(buffer + total_bytes, str, len);
        }
        total_bytes += len;
      }
    }
    p++;
  }

  va_end(args);
  return total_bytes;
}
