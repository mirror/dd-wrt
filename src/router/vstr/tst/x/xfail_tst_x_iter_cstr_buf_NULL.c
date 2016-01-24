#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  Vstr_iter iter[1];
  unsigned int ern = 444;
  
  vstr_add_rep_chr(s1, s1->len, 0, 10);
  
  ASSERT(vstr_iter_fwd_beg(s1, 1, s1->len, iter));
  ASSERT(vstr_iter_fwd_cstr(iter, 263, TST__NULL_ptr, 0, &ern) == 0);
}
