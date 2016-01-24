#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  const char *inval = "%b";

  vstr_add_fmt(s1, 0, inval);
}
