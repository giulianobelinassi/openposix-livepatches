#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <gnu/lib-names.h>

typedef typeof(access) access_func_t;

int access_lp(const char *pathname, int mode)
{
  void *libc_handle = dlopen(LIBC_SO, RTLD_LAZY);
  access_func_t *access_ptr =  dlsym(libc_handle, "access");
  access_ptr = skip_ulp_redirect_insns(access_ptr);
  printf("Livepatched\n");

  int ret = access_ptr(pathname, mode);
  dlclose(libc_handle);
}
