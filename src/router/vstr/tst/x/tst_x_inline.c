
#define VSTR_COMPILE_INLINE 0

#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  VSTR_ADD_CSTR_BUF(s1, 0, "abcd"); /* make sure we can run inline functions */

  return (EXIT_SUCCESS);
}
