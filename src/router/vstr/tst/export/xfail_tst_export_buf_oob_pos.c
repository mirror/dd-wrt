#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  memset(buf, 'X', 4);
  vstr_export_buf(s1, 1, 1, buf + 1, 2);
  ASSERT(!memcmp(buf, "XXXX", 4));
}
