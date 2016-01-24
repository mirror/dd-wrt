#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;

  TST_B_TST(ret, 1,
            !vstr_cntl_conf(s3->conf,
                            VSTR_CNTL_CONF_SET_LOC_CSTR_NAME_NUMERIC,
                            "custom"));
  TST_B_TST(ret, 2,
            !vstr_cntl_conf(s3->conf,
                            VSTR_CNTL_CONF_SET_LOC_CSTR_THOU_SEP, "_."));
  TST_B_TST(ret, 3,
            !vstr_cntl_conf(s3->conf,
                            VSTR_CNTL_CONF_SET_LOC_CSTR_THOU_GRP, "\3"));
  vstr_del(s3, 1, s3->len);
  vstr_add_fmt(s3, s3->len, "%'u", (10 * 1000 * 1000));

  TST_B_TST(ret, 4,
             !VSTR_CMP_CSTR_EQ(s3, 1, s3->len, "10_.000_.000"));

  TST_B_TST(ret, 5,
            !vstr_cntl_conf(s3->conf,
                            VSTR_CNTL_CONF_SET_LOC_CSTR_THOU_GRP, "\1\3\255"));
  vstr_del(s3, 1, s3->len);
  vstr_add_fmt(s3, s3->len, "%'u", (10 * 1000 * 1000));

  TST_B_TST(ret, 6,
             !VSTR_CMP_CSTR_EQ(s3, 1, s3->len, "1000_.000_.0"));

  TST_B_TST(ret, 7,
            !vstr_cntl_conf(s3->conf,
                            VSTR_CNTL_CONF_SET_LOC_CSTR_THOU_GRP, "\1\2\3\1"));
  vstr_del(s3, 1, s3->len);
  vstr_add_fmt(s3, s3->len, "%'u", (10 * 1000 * 1000));

  TST_B_TST(ret, 8,
            !VSTR_CMP_CSTR_EQ(s3, 1, s3->len, "1_.0_.000_.00_.0"));

  return (TST_B_RET(ret));
}
