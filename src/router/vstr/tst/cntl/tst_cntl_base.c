#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  Vstr_conf *conf = NULL;
  Vstr_conf *tmp = NULL;
  int ret = 0;

  vstr_cntl_opt(VSTR_CNTL_OPT_GET_CONF, &conf);

  vstr_cntl_base(s1, VSTR_CNTL_BASE_GET_CONF, &tmp);

  TST_B_TST(ret, 1, conf != tmp);

  vstr_cntl_base(s2, VSTR_CNTL_BASE_SET_CONF, tmp); /* empty so should work */
  vstr_cntl_base(s3, VSTR_CNTL_BASE_SET_CONF, tmp);
  vstr_cntl_base(s4, VSTR_CNTL_BASE_SET_CONF, tmp);

  vstr_free_conf(tmp);

  vstr_cntl_base(s2, VSTR_CNTL_BASE_GET_CONF, &tmp);
  TST_B_TST(ret, 1, conf != tmp);
  vstr_free_conf(tmp);

  vstr_cntl_base(s3, VSTR_CNTL_BASE_GET_CONF, &tmp);
  TST_B_TST(ret, 1, conf != tmp);
  vstr_free_conf(tmp);

  vstr_cntl_base(s4, VSTR_CNTL_BASE_GET_CONF, &tmp);
  TST_B_TST(ret, 1, conf != tmp);
  vstr_free_conf(tmp);

  vstr_free_conf(conf);

  return (TST_B_RET(ret));
}
