#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  Vstr_base *t1 = vstr_dup_non(s1->conf, 128);

  TST_B_TST(ret, 1, !VSTR_CMP_BUF_EQ(t1, 1, t1->len, NULL, 128));

  vstr_free_base(t1);

  if (MFAIL_NUM_OK)
  {
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_NON, 1000);
    vstr_free_spare_nodes(NULL,     VSTR_TYPE_NODE_NON, 1000);
    vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_NUM_SPARE_BASE, 0);
    vstr_cntl_conf(s3->conf, VSTR_CNTL_CONF_SET_NUM_SPARE_BASE, 0);

    ASSERT(!s1->conf->spare_base_num);
    ASSERT(!s3->conf->spare_base_num);
    
    TST_B_TST(ret,  5, !tst_mfail_num(1));
    TST_B_TST(ret,  6, vstr_dup_non(s3->conf, 4));
    TST_B_TST(ret,  7, !tst_mfail_num(2));
    TST_B_TST(ret,  8, vstr_dup_non(s3->conf, 4));
    TST_B_TST(ret,  9, !tst_mfail_num(1));
    TST_B_TST(ret, 10, vstr_dup_non(NULL, 4));
    TST_B_TST(ret, 11, !tst_mfail_num(2));
    TST_B_TST(ret, 12, vstr_dup_non(NULL, 4));
    TST_B_TST(ret, 13, !tst_mfail_num(3));
    TST_B_TST(ret, 14, vstr_dup_non(NULL, 4));
  }

  return (TST_B_RET(ret));
}
