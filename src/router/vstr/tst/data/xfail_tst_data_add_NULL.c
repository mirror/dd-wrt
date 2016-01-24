#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  ASSERT(!vstr_data_add(NULL, TST__NULL_ptr, NULL));
}
