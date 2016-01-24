#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  vstr_add_cstr_ptr(s1, 0, "a");
  memset(buf, 'X', 4);
  vstr_export_buf(s1, 1, 2, buf + 1, 2);
  ASSERT(!memcmp(buf, "XXXX", 4));
}
