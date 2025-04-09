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
  CALL_OLD_FUNCTION(aio_read64, aiocbp);
}
