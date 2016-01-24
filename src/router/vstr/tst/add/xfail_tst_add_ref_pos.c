#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  Vstr_ref ref[1] = {{vstr_ref_cb_free_nothing, (char *)"abcd", 0}};

  VSTR_ADD_CSTR_REF(s1, 1, ref, 0);
}
