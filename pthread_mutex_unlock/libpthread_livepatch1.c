#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <pthread.h>
#include <gnu/lib-names.h>

typedef typeof(pthread_mutex_lock) func_t;
int __pthread_mutex_unlock(pthread_mutex_t *);

int pthread_mutex_unlock_lp(pthread_mutex_t *lock)
{
  func_t *func_ptr = __pthread_mutex_unlock;
  func_ptr = skip_ulp_redirect_insns(func_ptr);

  int ret = func_ptr(lock);
  release_any_generated_code(func_ptr);
  return ret;
}
