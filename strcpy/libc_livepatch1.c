#include "common.h"

#include <stdio.h>
#include <stdlib.h>

void *strcpy_actual(char *dst, const char *src)
{
  char *odst = dst;
  while ((*dst++ = *src++) != '\0')
    ;
  return odst;
}

/* glibc uses strcpy_ifunc to find an optimized version of strcpy to patch.
   Hence, we patch it to select our function.  If strcpy was already run we
   must find a way to patch the .plt entry instead.  */
void *strcpy_lp()
{
  return strcpy_actual;
}
