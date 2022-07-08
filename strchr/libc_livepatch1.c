#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const char *strchr_actual(const char *s, int c)
{
  if (!s)
    return NULL;

  do {
    if (*s == c) {
      return s;
    }
  } while (*s++ != '\0');

  return NULL;
}

void *strchr_lp()
{
  return strchr_actual;
}
