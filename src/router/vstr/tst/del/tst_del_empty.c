#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;

  TST_B_TST(ret, 1, !vstr_del(s1, 1, 0)); /* empty */
  TST_B_TST(ret, 2, !vstr_del(s1, 2, 0)); /* empty, at offset */

  return (TST_B_RET(ret));
}
