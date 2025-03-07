#include "common.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <aio.h>

typedef typeof(aio_return) func_t;
ssize_t aio_return64(struct aiocb *);

ssize_t aio_return_lp(struct aiocb *aiocbp)
{
  func_t *func_ptr = skip_ulp_redirect_insns(aio_return64);
  ssize_t ret = func_ptr(aiocbp);
  release_any_generated_code(func_ptr);
  return ret;
}
