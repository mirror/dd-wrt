#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  vstr_free_base(s1);
  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_NUM_SPARE_BASE, 0);
  vstr_free_base(s1);
}
