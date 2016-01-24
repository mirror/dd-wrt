#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  Vstr_sects *sects1 = vstr_sects_make(1);

  vstr_sects_update_add(s1, sects1);
  vstr_sects_update_del(s1, sects1);
  vstr_sects_update_del(s1, sects1);
}
