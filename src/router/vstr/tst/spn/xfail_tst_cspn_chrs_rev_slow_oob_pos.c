#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  ASSERT(!vstr_cspn_cstr_chrs_rev(s1, 1, 2, "ab"));
}
