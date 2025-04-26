#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

extern void __libc_free(void *);

void free_lp(void *p)
{
  CALL_OLD_FUNCTION_1(__libc_free, p);
}
