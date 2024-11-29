#include "common.h"

#include <stdio.h>
#include <stdlib.h>

void *strncpy_lp(char *dst, const char *src, size_t n)
{
  size_t count = 0;
  char *odst = dst;
  while (count < n) {
    if (*src == '\0') {
      *dst++ = *src++;
      count++;
      break;
    } else {
      *dst++ = *src++;
      count++;
    }
  }

  while (count < n) {
    *dst++ = '\0';
    count++;
  }

  return odst;
}
