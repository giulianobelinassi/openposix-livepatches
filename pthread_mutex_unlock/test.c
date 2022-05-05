#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

static volatile bool gate = false;

void sig_handler(int signum)
{
  if (signum == SIGUSR1)
    gate = !gate;
}

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
  signal(SIGUSR1, sig_handler);

  printf("Waiting for SIGUSR1\n");

  while (gate == false)
    usleep(1000);

  printf("strdup(NULL): %s\n", strdup(NULL));

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
