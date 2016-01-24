#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  ASSERT(!VSTR_SUB_CSTR_BUF(TST__NULL_ptr, 1, 0, "abcd"));
}
