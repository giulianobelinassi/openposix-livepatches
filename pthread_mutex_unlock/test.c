#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

#include "common.h"

#define N 1000
#define NUM_THREADS 4

volatile int counter = 0;
static pthread_mutex_t mutex;

void *threaded_func(void *args __attribute__((unused)))
{
  int i;

  for (i = 0; i < N; i++) {
    if (pthread_mutex_lock(&mutex) != 0)
      abort();

    counter++;

    if (pthread_mutex_unlock(&mutex) != 0)
      abort();
  }
}

int main()
{
  wait_for_livepatch();

  pthread_t threads[NUM_THREADS];

  for (int i = 0; i < NUM_THREADS; i++) {
    if (pthread_create(&threads[i], NULL, threaded_func, NULL) != 0)
      abort();
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    if (pthread_join(threads[i], NULL) != 0)
      abort();
  }

  return counter - (NUM_THREADS * N);
}
