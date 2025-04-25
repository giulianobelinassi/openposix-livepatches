#include "common.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <aio.h>

int aio_read64(struct aiocb *aiocbp);

int aio_read_lp(struct aiocb *aiocbp)
{
  return (int) CALL_OLD_FUNCTION_1(aio_read64, aiocbp);
}
