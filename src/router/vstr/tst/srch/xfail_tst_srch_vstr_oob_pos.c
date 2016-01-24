#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  vstr_add_cstr_buf(s1, s1->len, "ab");

  ASSERT(!vstr_srch_vstr_fwd(s1, 1, 2, s2, 1, 2));
}
