#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdatomic.h>

void *memcpy_actual(void *dest, const void *src, size_t n)
{
  unsigned char *destp = (unsigned char *) dest;
  const unsigned char *srcp = (const unsigned char *) src;

  while (n > 0) {
    *destp++ = *srcp++;
    n--;
  }

  return dest;
}

/* ifunc, see strcpy comment.  */
void *memcpy_lp()
{
  return memcpy_actual;
}
