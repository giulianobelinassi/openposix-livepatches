#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdatomic.h>
#include <sys/mman.h>

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

#if defined(__x86_64__)

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

static void release_any_generated_code(void *func)
{
  /* x86_64 version do not need to release anything.  */
  (void) func;
}

#elif defined(__powerpc64__)

/* On powerpc64le we can't simply skip the redirect instructions, as this
   platform have global and local entrypoints.  In the fist case, we actually
   must make sure the global entrypoint is executed, so the way we do this is
   actually copying the global entrypoint to a trampoline, and then this
   trampoline redirects to the original function bypassing the redirect
   instructions.*/
static void *generate_trampoline_bypassing_redirect(void *function)
{
  /* Allocate a memory area where we will build our code.  */
  void *page = mmap(NULL, 128, PROT_EXEC | PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  /* Asm code boilerplate.  */
  unsigned char asm_insn[] = {
    0x22, 0x11, 0x20, 0x3d,   //  lis     r9,0x1122
    0x44, 0x33, 0x29, 0x61,   //  ori     r9,r9,0x3344
    0x66, 0x55, 0x40, 0x3d,   //  lis     r10,0x5566
    0x88, 0x77, 0x4a, 0x61,   //  ori     r10,r10,0x7788
    0x0e, 0x00, 0x2a, 0x79,   //  rldimi  r10,r9,32,0
    0x78, 0x53, 0x4c, 0x7d,   //  mr      r12,r10
    0x0c, 0x00, 0x4a, 0x39,   //  addi    r10,r10,12
    0xa6, 0x03, 0x49, 0x7d,   //  mtctr   r12
    0x00, 0x00, 0x00, 0x60,   //  nop
    0x00, 0x00, 0x00, 0x60,   //  nop
    0x20, 0x04, 0x80, 0x4e,   //  bctr
  };

  /* Assemble a jump to function from the `function` address.  */
  asm_insn[12] = ((unsigned long)function & 0x00000000000000FF) >> 0;
  asm_insn[13] = ((unsigned long)function & 0x000000000000FF00) >> 8;
  asm_insn[8]  = ((unsigned long)function & 0x0000000000FF0000) >> 16;
  asm_insn[9]  = ((unsigned long)function & 0x00000000FF000000) >> 24;
  asm_insn[4]  = ((unsigned long)function & 0x000000FF00000000) >> 32;
  asm_insn[5]  = ((unsigned long)function & 0x0000FF0000000000) >> 40;
  asm_insn[0]  = ((unsigned long)function & 0x00FF000000000000) >> 48;
  asm_insn[1]  = ((unsigned long)function & 0xFF00000000000000) >> 56;

  /* Copy the global entrypoint to the offset where the 2 nops are.  */
  memcpy(asm_insn + 32, function, 8);

  /* Copy the complete code into the executable page and return it.  */
  memcpy(page, asm_insn, sizeof(asm_insn));

  return page;
}

static void *skip_ulp_redirect_insns(void *func)
{
  /* Check if func points to the global entrypoint.  */
  unsigned int insn1, insn2;
  unsigned int *func_4b = (unsigned int *) func;
  insn1 = func_4b[0] & 0xFFFF0000, insn2 = func_4b[1] & 0xFFFF0000;

  if (insn1 == 0x3C4C0000 && insn2 == 0x38420000) {
    /* Function has global entrypoint.  */
    return generate_trampoline_bypassing_redirect(func);
  }
  /* Skip redirect insn.  */
  func_4b += 1;
  return (void *) func_4b;
}

static void release_any_generated_code(void *func)
{
  /* Check if instructions ressemble the code we created.  */
  const unsigned char cmp1[] = {0x20, 0x3d};
  const unsigned char cmp2[] = {0x29, 0x61};
  const unsigned char cmp3[] = {0x40, 0x3d};
  const unsigned char cmp4[] = {0x4a, 0x61};

  if (func != NULL &&
      memcmp(func +  2, cmp1, 2) == 0 &&
      memcmp(func +  6, cmp2, 2) == 0 &&
      memcmp(func + 10, cmp3, 2) == 0 &&
      memcmp(func + 14, cmp4, 2) == 0) {

    /* Looks like so.  */
    munmap(func, 128);
  }
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
    const char *(*old_glibc_version)(void) = skip_ulp_redirect_insns(gnu_get_libc_version);
    const char *str = old_glibc_version();
    new_ver_str = Build_Glibc_LP_Version_String(str);
    atomic_store(&lock, 2);
    release_any_generated_code(old_glibc_version);
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
