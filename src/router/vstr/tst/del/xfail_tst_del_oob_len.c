#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  vstr_add_rep_chr(s1, 0, 'a', 1);
  ASSERT(!vstr_del(s1, 1, 2));
}
