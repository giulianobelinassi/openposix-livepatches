#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <gnu/lib-names.h>

typedef typeof(access) func_t;

int access_lp(const char *pathname, int mode)
{
  func_t *func_ptr = skip_ulp_redirect_insns(access);

  int ret = func_ptr(pathname, mode);
  return ret;
}
