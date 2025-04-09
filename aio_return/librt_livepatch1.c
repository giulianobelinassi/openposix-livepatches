#include "common.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <aio.h>

ssize_t aio_return64(struct aiocb *);

ssize_t aio_return_lp(struct aiocb *aiocbp)
{
  CALL_OLD_FUNCTION(aio_return64, aiocbp);
}
