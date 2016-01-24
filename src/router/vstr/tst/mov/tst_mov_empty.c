#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;

  /* fast path ... */
  TST_B_TST(ret, 1, !vstr_mov(s1, 0, s2, 1, 0)); /* empty */
  TST_B_TST(ret, 2, !vstr_mov(s1, 0, s2, 4, 0)); /* empty, offset */

  /* slow ... */
  TST_B_TST(ret, 3, !vstr_mov(s1, 0, s3, 1, 0)); /* empty */
  TST_B_TST(ret, 4, !vstr_mov(s1, 0, s3, 4, 0)); /* empty, offset */

  return (0);
}
