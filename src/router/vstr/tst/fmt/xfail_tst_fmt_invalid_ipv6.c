#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  unsigned int ips[8];
  
  vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');
  vstr_sc_fmt_add_all(s1->conf);
  ASSERT(!vstr_add_fmt(s1, 0, "${ipv6.v:%p%u}", (void *)ips, 99));
  ASSERT(!s1->conf->malloc_bad);
}
