#include <stdio.h>
#include <time.h>

#include "common.h"

int main(int argc, char *argv[])
{
  wait_for_livepatch();
  struct timespec ts = { .tv_sec = 0, .tv_nsec = 10 };

  nanosleep(&ts, NULL);

  return 0;
}
