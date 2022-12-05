#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

#include "common.h"

int main()
{

  wait_for_livepatch();

  const char *str = strchr("u/a/b/c", '/');
  const char *str2 = strchr("", '\0');

  if (!strcmp(str, "/a/b/c") && *str2 == '\0') {
    printf("Found\n");
  } else {
    printf("Not found\n");
  }

  return 0;
}
