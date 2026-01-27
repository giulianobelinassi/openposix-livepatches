#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdatomic.h>
#include <sys/mman.h>
#include <stdint.h>

#include "common.h"

#if defined(__x86_64__)

#define INSN_ENDBR64 0xf3, 0x0f, 0x1e, 0xfa

/* Skip the ULP prologue of a function, so that when 'func' is called it runs
   its original code.  This should save us from including libc headers and copy
   & pasting code from libc itself.  */
void *call_old_func(void *p1, void *p2, void *p3, void *p4,
                           void *p5, void *p6, void *func)
{
  /* Check if function contain the jump trampoline.  */

  const unsigned char *as_bytes = (const unsigned char *) func;
  const unsigned char insn_endbr64[] = {INSN_ENDBR64};
  unsigned int i;
  int bias = 0;
  void *(*ptr)(void *, void *, void *, void *, void *, void *);

  for (i = 0; i < sizeof(insn_endbr64); i++) {
    if (as_bytes[i] != insn_endbr64[i])
      break;
  }

  if (i == sizeof(insn_endbr64)) {
    /* Comparison successful.  */
    as_bytes += sizeof(insn_endbr64);
    bias += sizeof(insn_endbr64);
  }

  if (as_bytes[0] != 0xEB) {
    /* JMP rel8 opcode not found. Function definitely not livepatched.
       Check if it is livepatchable*/

    if (as_bytes[1] == 0x90) {
      if (as_bytes [0] == 0x90 || as_bytes[0] == 0x66) {
        printf("Function not livepatched\n");
        goto add;
      }
    }

    printf("Function not livepatchable. Patching it would break the application\n");
    abort();
  }

  /* On x86_64, the JMP insns used for redirecting the old function
     into the new one takes 2 bytes.  So add 2 bytes to skip it.  */
add:
  ptr = add_long_to_ptr(func, 2 + bias);
  return ptr(p1, p2, p3, p4, p5, p6);
}

#elif defined(__powerpc64__)
/* For the powerpc64le version, check common_ppc64le.c.in and common_ppc64le.S.  */
#endif


const char *Build_Glibc_LP_Version_String(const char *str)
{
  if (str == NULL)
    return NULL;

  /* Create a new string.  */
  size_t str_len = strlen(str);
  size_t size = str_len + ARR_LEN(LP_SUFFIX) + 1;
  char *new_ver_str = (char *) calloc(size, sizeof(char));
  if (new_ver_str == NULL) {
    /* If allocation failed, then return the original string.  */
    return str;
  }

  char *p;

  /* Concatenate the suffix.  */
  p = memcpy(new_ver_str, str, str_len + 1);

  /* Make sure the return values are sane.  */
  assert(p == new_ver_str && "memcpy return value is broken");

  p = strcat(new_ver_str, LP_SUFFIX);

  /* Make sure the return values are sane.  */
  assert(p == new_ver_str && "strcat return value is broken");

  return new_ver_str;
}

/* Override the gnu_get_libc_version to append a lp suffix to the original
   version.  */
const char *gnu_get_libc_version_lp(void)
{
  static volatile const char *new_ver_str;
  const int _0 = 0;

  /* Implement a lock without using the pthread functions.  We can't assume
     that the function will be called only in single-threaded applications.
     Hence if two threads called this function we have to be careful with the
     state that thread #1 is building the string and thread #2 called this
     function. In this case we have to block thread #2 until the string is
     completed.  */
  static atomic_int lock = 0;

  /* Initialize variables.  */
  if (atomic_compare_exchange_strong(&lock, &_0, 1)) {
    const char *str = CALL_OLD_FUNCTION_0(gnu_get_libc_version);
    new_ver_str = Build_Glibc_LP_Version_String(str);
    atomic_store(&lock, 2);
  }

  while (atomic_load(&lock) < 2)
    ; /* Wait for variable initialization to end.  */

  return (const char *) new_ver_str;
}

/* Wait until the livepatch occurs.  */
void wait_for_livepatch(void)
{
  while (1) {
    const char *version = gnu_get_libc_version();
    const char *l = strchr(version, '-');

    if (l && strcmp(l, LP_SUFFIX) == 0) {
      /* Livepatching occured.  */
      puts(version);
      break;
    }

    usleep(1000);
  }
}
