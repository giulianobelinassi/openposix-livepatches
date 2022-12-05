#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

/* Avoid ugly casting on code.  */
static void *add_long_to_ptr(void *ptr, long val)
{
  return (void *) ((char *)ptr + val);
}

#define INSN_ENDBR64 0xf3, 0x0f, 0x1e, 0xfa

#if !defined(_VERSION) || !defined(_PACKAGE_NAME)
# error "_VERSION or _PACKAGE_NAME not defined."
#endif

#define xstr(s) str(s)
#define str(s) #s

void *__libc_malloc(unsigned long size);
void *__libc_calloc(unsigned long nmemb, unsigned long size);

static typeof(malloc) *real_malloc = NULL;

#define ARR_LEN(x) (sizeof(x) / sizeof(*x))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

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

static const char *const lp_string = xstr(_PACKAGE_NAME) " " xstr(_VERSION);

/* On a conversation with Martin Doucha we agreed on the following:
   every livepatch should contain a patched version of malloc which returns
   an string containing "openposix-livepatches VERSION".  Therefore, we
   insert this function here.  */

void *malloc_lp(size_t s)
{

  if (real_malloc == NULL) {
    real_malloc = skip_ulp_redirect_insns(__libc_malloc);
  }

  char *block = real_malloc(s);
  if (block && s > 0) {
    int lp_string_len = strlen(lp_string);
    int copy_len = MIN(lp_string_len + 1, s);

    memcpy(block, lp_string, copy_len);
    block[s-1] = '\0';
  }

  return block;
}

/* Wait until the livepatch occurs.  */
static void wait_for_livepatch(void)
{
  while (1) {
    char *block = malloc(32);
    assert(block && "malloc(32) returned null.");

    if (strcmp(block, lp_string) == 0) {
      /* Livepatching occured.  */
      free(block);
      return;
    }

    usleep(1000);
    free(block);
  }
}
