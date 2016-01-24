#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  Vstr_ref ref[1];
  
  ref->ref = 1;
  ref->ptr = (char *)"abcd";
  ref->func = vstr_ref_cb_free_nothing;
  
  VSTR_ADD_CSTR_BUF(s1, 0, "abcd");
  ASSERT(!VSTR_SUB_CSTR_REF(TST__NULL_ptr, 1, 4, ref, 0));
}
