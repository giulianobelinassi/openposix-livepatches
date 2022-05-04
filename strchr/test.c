#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

static volatile bool gate = false;

void sig_handler(int signum)
{
  if (signum == SIGUSR1) {
    gate = true;
  }
}

int main()
{
  signal(SIGUSR1, sig_handler);

  printf("Waiting for SIGUSR1\n");
  while (gate == false)
    usleep(1000);

  printf("strdup(NULL): %s\n", strdup(NULL));
  const char *str = strchr("u/a/b/c", '/');

  if (str) {
    printf("Found\n");
  } else {
    printf("Not found\n");
  }

  return 0;
}
