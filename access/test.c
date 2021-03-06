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

int main()
{
  signal(SIGUSR1, sig_handler);

  printf("Waiting for SIGUSR1\n");
  while (gate == false)
    usleep(1000);

  printf("strdup(NULL): %s\n", strdup(NULL));

  if (access("/lib64/libc.so.6", F_OK) != -1) {
    printf("File found\n");
  } else {
    printf("File not found\n");
  }

  return 0;
}
