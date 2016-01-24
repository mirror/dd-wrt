#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  vstr_add_rep_chr(s1, 0, 'x', 1);
  ASSERT(!vstr_srch_case_cstr_buf_fwd(s1, 1, 2, "aB"));
}
