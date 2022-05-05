#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <gnu/lib-names.h>

typedef typeof(access) func_t;

int access_lp(const char *pathname, int mode)
{
  void *libc_handle = dlopen(LIBC_SO, RTLD_LAZY);
  func_t *func_ptr =  dlsym(libc_handle, "access");
  func_ptr = skip_ulp_redirect_insns(func_ptr);

  int ret = func_ptr(pathname, mode);

  dlclose(libc_handle);
  return ret;
}
