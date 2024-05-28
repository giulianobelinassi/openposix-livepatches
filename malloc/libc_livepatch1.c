#include "common.h"

#include <stdio.h>
#include <stdlib.h>

#define MIN(x, y) ((x) < (y) ? (x) : (y))

extern void *__libc_malloc(size_t);

void *malloc_lp(size_t s)
{
  static typeof(malloc) *real_malloc = NULL;
  static const char *const lp_string = "Livepatched";

  if (real_malloc == NULL) {
    real_malloc = skip_ulp_redirect_insns(__libc_malloc);
  }

  char *block = real_malloc(s);
  if (block && s > 0) {
    int lp_string_len = strlen(lp_string);
    int copy_len = MIN(lp_string_len + 1, s);

    memcpy(block, lp_string, copy_len);
    block[s-1] = '\0';
  }

  return block;
}
