#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  VSTR_ADD_CSTR_BUF(s1, 0, "abcd");
  ASSERT(!vstr_sub_rep_chr(s1, s1->len + 1, 4, 'a', 4));
}
