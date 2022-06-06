#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char *strchr_actual(const char *s, int c)
{
  for (; *s != '\0'; s++) {
    if (*s == c) {
      return s;
    }
  }

  return NULL;
}

char *strchr_lp()
{
  return strchr_actual;
}
