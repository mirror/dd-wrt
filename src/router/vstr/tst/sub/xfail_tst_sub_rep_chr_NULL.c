#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  ASSERT(!vstr_sub_rep_chr(TST__NULL_ptr, 1, 0, 'a', 4));
}
