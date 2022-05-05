#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <pthread.h>
#include <gnu/lib-names.h>

typedef typeof(pthread_mutex_lock) func_t;

int pthread_mutex_unlock_lp(pthread_mutex_t *lock)
{
  func_t *func_ptr = pthread_mutex_unlock;
  func_ptr = skip_ulp_redirect_insns(func_ptr);
  printf("livepatched: in pthread_mutex_unlock_lp\n");

  int ret = func_ptr(lock);
  return ret;
}
