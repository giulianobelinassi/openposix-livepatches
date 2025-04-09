#include "common.h"

#include <time.h>

extern int __nanosleep(const struct timespec *duration,
                       struct timespec *rem);

void nanosleep_lp(const struct timespec *duration,
                  struct timespec *rem)
{
  CALL_OLD_FUNCTION_VOID(__nanosleep, duration, rem);
}
