#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  Vstr_sects *sects = vstr_sects_make(4);
  
  vstr_sects_free(sects);
  vstr_sects_free(sects);
}
