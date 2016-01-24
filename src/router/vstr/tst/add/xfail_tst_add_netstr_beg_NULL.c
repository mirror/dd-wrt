#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  vstr_add_netstr_beg(TST__NULL_ptr, 0);
}
