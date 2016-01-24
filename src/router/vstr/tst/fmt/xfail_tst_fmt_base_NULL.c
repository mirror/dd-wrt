#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  ASSERT(!vstr_add_fmt(TST__NULL_ptr, 0, "a"));
}
