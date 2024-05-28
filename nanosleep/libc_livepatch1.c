#include "common.h"

#include <time.h>

extern int __nanosleep(const struct timespec *duration,
                       struct timespec *rem);

void nanosleep_lp(const struct timespec *duration,
                  struct timespec *rem)
{
  static typeof(nanosleep) *real_nanosleep = NULL;

  if (real_nanosleep == NULL) {
    real_nanosleep = skip_ulp_redirect_insns(__nanosleep);
  }

  real_nanosleep(duration, rem);
}
