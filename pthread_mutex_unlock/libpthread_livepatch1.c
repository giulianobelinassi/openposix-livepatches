#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <pthread.h>
#include <gnu/lib-names.h>

int __pthread_mutex_unlock(pthread_mutex_t *);

int pthread_mutex_unlock_lp(pthread_mutex_t *lock)
{
  CALL_OLD_FUNCTION(__pthread_mutex_unlock, lock);
}
