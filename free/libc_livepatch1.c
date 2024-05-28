#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

extern void __libc_free(void *);

void free_lp(void *p)
{
  static typeof(free) *real_free = NULL;

  if (real_free == NULL) {
    real_free = skip_ulp_redirect_insns(__libc_free);
  }

  real_free(p);
}
