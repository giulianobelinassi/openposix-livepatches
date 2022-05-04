#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <gnu/lib-names.h>

typedef typeof(strchr) func_t;

char *strchr_lp(const char *s, int c)
{
  void *libc_handle = dlopen(LIBC_SO, RTLD_LAZY);
  func_t *func_ptr =  dlsym(libc_handle, "access");
  func_ptr = skip_ulp_redirect_insns(func_ptr);

  char *ret = func_ptr(s, c);

  dlclose(libc_handle);
  return ret;
}
