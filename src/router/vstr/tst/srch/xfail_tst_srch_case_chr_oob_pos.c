#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  ASSERT(!vstr_srch_case_chr_fwd(s1, 1, 2, 'a'));
}
