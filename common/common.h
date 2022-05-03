#include <stdlib.h>
#include <string.h>

/* Avoid ugly casting on code.  */
static void *add_long_to_ptr(void *ptr, long val)
{
  return (void *) ((char *)ptr + val);
}

/* Skip the ULP prologue of a function, so that when 'func' is called it runs
   its original code.  This should save us from including libc headers and copy
   & pasting code from libc itself.  */
static void *skip_ulp_redirect_insns(void *func)
{
  /* On x86_64, the JMP insns used for redirecting the old function
     into the new one takes 2 bytes.  So add 2 bytes to skip it.  */

  return add_long_to_ptr(func, 2L);
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
