#include "common.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <aio.h>

typedef typeof(aio_read) func_t;

int aio_read64(struct aiocb *aiocbp);

int aio_read_lp(struct aiocb *aiocbp)
{
  func_t *func_ptr = skip_ulp_redirect_insns(aio_read64);
  printf("aio_read livepatched\n");
  return func_ptr(aiocbp);
}
