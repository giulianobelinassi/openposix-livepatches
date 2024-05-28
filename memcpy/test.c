#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <dlfcn.h>

#include "common.h"

int main(int argc, char *argv[])
{
  char buffer[64];

  wait_for_livepatch();
  memcpy(buffer, "1234567890", 11);

  if (strcmp(buffer, "1234567890") == 0) {
    printf("buffer = %s\n", buffer);
    return 0;
  }

  return 1;
}
