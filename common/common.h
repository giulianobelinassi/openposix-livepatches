#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Avoid ugly casting on code.  */
static void *add_long_to_ptr(void *ptr, long val)
{
  return (void *) ((char *)ptr + val);
}

#define INSN_ENDBR64 0xf3, 0x0f, 0x1e, 0xfa

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

  printf("Livepatched\n");
  /* On x86_64, the JMP insns used for redirecting the old function
     into the new one takes 2 bytes.  So add 2 bytes to skip it.  */
add:
  return add_long_to_ptr(func, 2 + bias);
}

/* On a conversation with Cyril Hrubis we agreed on the following:
   every livepatch should contain a patched version of strdup which returns
   an string when called with NULL as input.  Therefore, we insert this
   function here.  */
char *strdup_lp(const char *in)
{
  if (in == NULL) {
    return "returned by strdup_lp";
  }

  char *out = malloc(strlen(in) + 1);
  strcpy(out, in);
  return out;
}
