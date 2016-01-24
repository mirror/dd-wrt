#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  vstr_add_vstr(s1, 0, xfail_NULL_ptr, 1, 1, 0);
}
