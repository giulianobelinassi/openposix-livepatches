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
