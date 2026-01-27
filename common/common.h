#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdatomic.h>
#include <sys/mman.h>
#include <stdint.h>

/* Macros for calling the old code.  */
#define CALL_OLD_FUNCTION_0(func)   \
  call_old_func(NULL, NULL, NULL, NULL, NULL, NULL, func)

#define CALL_OLD_FUNCTION_1(func, arg1)   \
  call_old_func((void *)(arg1), NULL, NULL, NULL, NULL, NULL, func)

#define CALL_OLD_FUNCTION_2(func, arg1, arg2)   \
  call_old_func((void *)arg1, (void *)(arg2), NULL, NULL, NULL, NULL, func)

#define CALL_OLD_FUNCTION_3(func, arg1, arg2, arg3)   \
  call_old_func((void *)(arg1), (void *)(arg2), (void *)(arg3), NULL, NULL, NULL, func)

#define CALL_OLD_FUNCTION_4(func, arg1, arg2, arg3, arg4)   \
  call_old_func((void *)(arg1), (void *)(arg2), (void *)(arg3), (void *)(arg4), NULL, NULL, func)

#define CALL_OLD_FUNCTION_5(func, arg1, arg2, arg3, arg4, arg5)   \
  call_old_func((void *)(arg1), (void *)(arg2), (void *)(arg3), (void *)(arg4), (void *)(arg5), NULL, func)

#define CALL_OLD_FUNCTION_6(func, arg1, arg2, arg3, arg4, arg5, arg6)   \
  call_old_func((void *)(arg1), (void *)(arg2), (void *)(arg3), (void *)(arg4), (void *)(arg5), (void*)(arg6), func)

/* Avoid ugly casting on code.  */
static inline void *add_long_to_ptr(void *ptr, long val)
{
  return (void *) ((char *)ptr + val);
}

#if !defined(_VERSION) || !defined(_PACKAGE_NAME)
# error "_VERSION or _PACKAGE_NAME not defined."
#endif

#define STRINGFY(s) #s
#define STRINGFY_VALUE(s) STRINGFY(s)

#define LP_SUFFIX "-lp-" STRINGFY_VALUE(_VERSION)
#define ARR_LEN(v) (sizeof(v)/sizeof(*(v)))

/* Declare the version function.  */
extern const char *gnu_get_libc_version(void);

/* Aux livepatch functions.  */
const char *Build_Glibc_LP_Version_String(const char *str);
const char *gnu_get_libc_version_lp(void);
void wait_for_livepatch(void);

/* Skip the ULP prologue of a function, so that when 'func' is called it runs
   its original code.  This should save us from including libc headers and copy
   & pasting code from libc itself.  */

__attribute__((visibility("default")))
void *call_old_func(void *p1, void *p2, void *p3, void *p4,
                           void *p5, void *p6, void *func);
