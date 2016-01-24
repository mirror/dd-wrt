#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  Vstr_conf *conf = NULL;
  Vstr_base *tmp = NULL;
  int ret = 0;

  vstr_cntl_opt(VSTR_CNTL_OPT_GET_CONF, &conf);

  TST_B_TST(ret, 1, conf != s1->conf);

  vstr_free_conf(conf);
  conf = vstr_make_conf();

  vstr_cntl_opt(VSTR_CNTL_OPT_SET_CONF, conf);
  vstr_free_conf(conf);

  tmp = vstr_make_base(NULL);

  TST_B_TST(ret, 2, conf == s1->conf);
  TST_B_TST(ret, 3, conf != tmp->conf);

  vstr_free_base(tmp);

  return (TST_B_RET(ret));
}
