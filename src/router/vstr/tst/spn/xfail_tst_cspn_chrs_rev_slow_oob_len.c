#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  vstr_add_cstr_buf(s1, 0, "a");
  ASSERT(!vstr_cspn_cstr_chrs_rev(s1, 1, 2, "ab"));
}
