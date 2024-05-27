#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdatomic.h>

/* Avoid ugly casting on code.  */
static void *add_long_to_ptr(void *ptr, long val)
{
  return (void *) ((char *)ptr + val);
}

#define INSN_ENDBR64 0xf3, 0x0f, 0x1e, 0xfa

#if !defined(_VERSION) || !defined(_PACKAGE_NAME)
# error "_VERSION or _PACKAGE_NAME not defined."
#endif

#define STRINGFY(s) #s
#define STRINGFY_VALUE(s) STRINGFY(s)

#define LP_SUFFIX "-lp-" STRINGFY_VALUE(_VERSION)
#define ARR_LEN(v) (sizeof(v)/sizeof(*(v)))

/* Declare the version function.  */
extern const char *gnu_get_libc_version(void);

static typeof(gnu_get_libc_version) *real_gnu_get_libc_version = NULL;

/* Skip the ULP prologue of a function, so that when 'func' is called it runs
   its original code.  This should save us from including libc headers and copy
   & pasting code from libc itself.  */
static void *skip_ulp_redirect_insns(void *func)
{
  /* Check if function contain the jump trampoline.  */

  const unsigned char *as_bytes = (const unsigned char *) func;
  const unsigned char insn_endbr64[] = {INSN_ENDBR64};
  int i, bias = 0;

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
  return add_long_to_ptr(func, 2 + bias);
}

static const char *Build_Glibc_LP_Version_String(const char *str)
{
  if (str == NULL)
    return NULL;

  /* Create a new string.  */
  size_t size = strlen(str) + ARR_LEN(LP_SUFFIX) + 1;
  char *new_ver_str = (char *) calloc(size, sizeof(char));
  if (new_ver_str == NULL) {
    /* If allocation failed, then return the original string.  */
    return str;
  }

  /* Concatenate the suffix.  */
  strcpy(new_ver_str, str);
  strcat(new_ver_str, LP_SUFFIX);

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
    const char *(*old_glibc_version)(void) = skip_ulp_redirect_insns(gnu_get_libc_version);
    const char *str = old_glibc_version();
    new_ver_str = Build_Glibc_LP_Version_String(str);
    atomic_store(&lock, 2);
  }

  while (atomic_load(&lock) < 2)
    ; /* Wait for variable initialization to end.  */

  return (const char *) new_ver_str;
}

/* Wait until the livepatch occurs.  */
static void wait_for_livepatch(void)
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
