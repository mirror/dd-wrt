#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  vstr_export_buf(s1, 1, 0, TST__NULL_ptr, 1);
}
