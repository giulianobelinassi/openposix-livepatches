#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main()
{
  char buffer[128];

  /* Loop waiting for any input. */
  printf("Waiting for input.\n");
  while (1) {
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
      if (errno) {
        perror("fgets");
        return 1;
      }
    }

    printf("strdup(NULL): %s\n", strdup(NULL));

    if (access("/lib64/libc.so.6", F_OK) != -1) {
      printf("File found\n");
    } else {
      printf("File not found\n");
    }
  }

  return 0;
}
