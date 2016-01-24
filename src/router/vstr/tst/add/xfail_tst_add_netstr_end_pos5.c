#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  vstr_add_cstr_buf(s1, 0, "abcd");
  vstr_add_netstr_end(s1, 1, 2);
}
