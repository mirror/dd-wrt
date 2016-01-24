#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  const char *inval = "${b                                 }";

  vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');
  vstr_sc_fmt_add_all(s1->conf);
  ASSERT(!vstr_add_fmt(s1, 0, inval));
}
