#include "common.h"

#include <stdio.h>
#include <stdlib.h>

#define MIN(x, y) ((x) < (y) ? (x) : (y))

extern void *__libc_malloc(size_t);

void *malloc_lp(size_t s)
{
  static const char *const lp_string = "Livepatched";

  char *block = calloc(1, s);
  if (block && s > 0) {
    int lp_string_len = strlen(lp_string);
    int copy_len = MIN(lp_string_len + 1, s);

    memcpy(block, lp_string, copy_len);
    block[s-1] = '\0';
  }

  return block;
}
