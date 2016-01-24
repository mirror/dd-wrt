#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  vstr_add_rep_chr(s1, 0, 'x', 8);
  vstr_add_cstr_buf(s2, s2->len, "aB");

  ASSERT(!vstr_srch_case_vstr_fwd(s1, 1, 8, s2, 4, s2->len));
}
