#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef typeof(strchr) func_t;

char *strchr_lp(const char *s, int c)
{
  func_t *func_ptr = skip_ulp_redirect_insns(strchr);
  char *ret = func_ptr(s, c);
  return ret;
}
