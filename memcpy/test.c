#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <dlfcn.h>

#include "common.h"

volatile char *str = "1234567890";

int main(int argc, char *argv[])
{
  volatile char buffer[64];
  void *p;

  wait_for_livepatch();
  if ((p = memcpy((char *)buffer, (char *)str, 11)) != buffer) {
    printf("Incorrect return value: got 0x%lx, expected %lx\n", (unsigned long) p, (unsigned long) buffer);
    return 2;
  }

  if (strcmp((char *)buffer, (char *)str) == 0) {
    printf("buffer = %s\n", buffer);
    return 0;
  }

  return 1;
}
