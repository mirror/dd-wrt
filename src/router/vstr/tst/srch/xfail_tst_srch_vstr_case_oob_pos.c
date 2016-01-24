#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  vstr_add_cstr_buf(s2, s2->len, "Ab");

  ASSERT(!vstr_srch_case_vstr_fwd(s1, 1, 2, s2, 1, s2->len));
}
