#include "common.h"

#include <stdio.h>
#include <stdlib.h>

void *strcpy_lp(char *dst, const char *src)
{
  char *odst = dst;
  while ((*dst++ = *src++) != '\0')
    ;
  return odst;
}
