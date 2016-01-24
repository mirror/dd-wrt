#include "xfail-tst-main.c"

static const char *rf = __FILE__;

extern struct Vstr__cache_data_pos *
vstr_extern_inline_make_cache_pos(const Vstr_base *);
void xfail_tst(void)
{
  ASSERT(!vstr_extern_inline_make_cache_pos(NULL));
}
