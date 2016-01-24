#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  ASSERT(!vstr_spn_cstr_chrs_fwd(s1, 1, 2, "ab"));
}
