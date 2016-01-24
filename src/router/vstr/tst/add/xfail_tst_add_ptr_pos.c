#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  VSTR_ADD_CSTR_PTR(s1, 1, "abcd");
}
