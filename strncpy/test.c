#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

static volatile bool gate = false;

void sig_handler(int signum)
{
  if (signum == SIGUSR1)
    gate = true;
}

int main(int argc, char *argv[])
{
  char buffer[64];
  signal(SIGUSR1, sig_handler);

  printf("Waiting for SIGUSR1\n");
  while (gate == false)
    usleep(1000);

  printf("strdup(NULL): %s\n", strdup(NULL));

  strncpy(buffer, argv[0], 64);
  printf(buffer);
  printf("\n");
  if (!strcmp(buffer, argv[0])) {
    printf("Copy successful\n");
  } else {
    printf("Copy unsuccessful\n");
    return 1;
  }

  return 0;
}
