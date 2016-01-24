#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  vstr_add_cstr_buf(s1, 0, "abcd");
  vstr_export_iovec_ptr_all(s1, NULL, NULL);

  ASSERT(!vstr_srch_cstr_buf_rev(s1, 4, 2, "ab"));
}
