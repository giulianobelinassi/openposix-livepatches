#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

#include "common.h"

int main(int argc, char *argv[])
{
  char buffer[64];

  wait_for_livepatch();

  char *v = malloc(32);
  free(v);

  return 0;
}
