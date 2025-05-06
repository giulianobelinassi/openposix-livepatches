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
static void *add_long_to_ptr(void *ptr, long val)
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

/* Skip the ULP prologue of a function, so that when 'func' is called it runs
   its original code.  This should save us from including libc headers and copy
   & pasting code from libc itself.  */
void *call_old_func(void *p1, void *p2, void *p3, void *p4,
                           void *p5, void *p6, void *func)
{
  /* Check if func points to the global entrypoint.  */
  uint32_t *func_4b = (uint32_t *) func;
  uint32_t insn1 = func_4b[0], insn2 = func_4b[1];
  register uintptr_t skipped_func = (uintptr_t) func;

  /* Check if insn1 == addis r2, r12, CST16 and insn2 == addi, r2, r12, CST16.  */
  if ((insn1 & 0xFFFF0000) == 0x3C4C0000 && (insn2 & 0xFFFF0000) == 0x38420000) {
    int16_t ofsup = (int16_t)(insn1 & 0x0000FFFF);
    int16_t oflow = (int16_t)(insn2 & 0x0000FFFF);

    /* Emulate the addis and addi instruction.  */
    long signedofs = 0x10000 * (long)ofsup + (long)oflow;

    /* Sum with function to get the TOC pointer*/
    register uintptr_t target_toc = (uintptr_t)func + signedofs;

    /* Move the TOC pointer to r2.  */
    asm volatile ("mr   %%r2, %0\n"  : : "r"(target_toc) : );

    /* Move the function address to r12.  */
    asm volatile ("mr   %%r12, %0\n" : : "r"(func) : "r12");

    /* Skip the global entrypoint.  */
    skipped_func += 8;
  }

  /* Skip the ULP redirect instruction.  */
  skipped_func += 4;

  /* Move the call point address to mtctr so we can call it.  */
  asm volatile ("mtctr  %0\n" : : "r"(skipped_func) : );

  /* Make sure we have the parameters on registers.  */
  asm ("mr   %%r3, %0\n"  : : "r"(p1) : );
  asm ("mr   %%r4, %0\n"  : : "r"(p2) : );
  asm ("mr   %%r5, %0\n"  : : "r"(p3) : );
  asm ("mr   %%r6, %0\n"  : : "r"(p4) : );
  asm ("mr   %%r7, %0\n"  : : "r"(p5) : );
  asm ("mr   %%r8, %0\n"  : : "r"(p6) : );

  /* Call the old function.  Return address will end up in r3 so even if we
     don't return anything in C, the value will be there from the asm
     perspective.  */
  asm ("bctr\n" :::);
}

#endif

static const char *Build_Glibc_LP_Version_String(const char *str)
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
