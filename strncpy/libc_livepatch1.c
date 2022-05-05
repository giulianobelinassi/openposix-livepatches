#include "common.h"

#include <stdio.h>
#include <stdlib.h>

void *strncpy_actual(char *dst, const char *src, size_t n)
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

/* glibc uses strcpy_ifunc to find an optimized version of strcpy to patch.
   Hence, we patch it to select our function.  If strcpy was already run we
   must find a way to patch the .plt entry instead.  */
void *strncpy_lp()
{
  return strncpy_actual;
}
