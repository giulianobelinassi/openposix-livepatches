#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <pthread.h>
#include <gnu/lib-names.h>

int __pthread_mutex_lock(pthread_mutex_t *);
typedef typeof(pthread_mutex_lock) func_t;

int pthread_mutex_lock_lp(pthread_mutex_t *lock)
{
  func_t *func_ptr = __pthread_mutex_lock;
  func_ptr = skip_ulp_redirect_insns(func_ptr);

  int ret = func_ptr(lock);
  return ret;
}
