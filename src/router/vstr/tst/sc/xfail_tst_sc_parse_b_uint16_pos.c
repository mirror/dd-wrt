#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  ASSERT(!vstr_sc_parse_b_uint16(s1, 1));
}
