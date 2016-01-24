#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  VSTR_ADD_CSTR_BUF(s1, 0, "abcd");
  ASSERT(!VSTR_SUB_CSTR_BUF(s1, s1->len - 1, 4, "abcd"));
}
