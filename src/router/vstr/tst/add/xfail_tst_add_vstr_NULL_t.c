#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  VSTR_ADD_CSTR_BUF(s2, 0, "abcd");
  vstr_add_vstr(xfail_NULL_ptr, 0, s2, 1, s2->len, 0);
}
