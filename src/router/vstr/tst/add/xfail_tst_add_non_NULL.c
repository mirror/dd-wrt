#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  vstr_add_non(TST__NULL_ptr, 0, 0);
}
