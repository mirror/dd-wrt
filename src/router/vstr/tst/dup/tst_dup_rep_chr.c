#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  Vstr_base *t1 = NULL;
  const char *const cb1 = "aaaa";
  const char *const cb2 = "XXXXXXXX";

  t1 = vstr_dup_rep_chr(s1->conf, 'a', 8);

  TST_B_TST(ret,  1, !VSTR_CMP_CSTR_EQ(t1,  1, 4, cb1));
  TST_B_TST(ret,  2, !VSTR_CMP_CSTR_EQ(t1,  5, 4, cb1));

  vstr_free_base(t1);
  t1 = vstr_dup_rep_chr(s1->conf, 'X', 8);

  TST_B_TST(ret,  3, !VSTR_CMP_CSTR_EQ(t1,  1, t1->len, cb2));

  vstr_free_base(t1);



  t1 = vstr_dup_rep_chr(s2->conf, 'a', 8);

  TST_B_TST(ret,  4, !VSTR_CMP_CSTR_EQ(t1,  1, 4, cb1));
  TST_B_TST(ret,  5, !VSTR_CMP_CSTR_EQ(t1,  5, 4, cb1));

  vstr_free_base(t1);
  t1 = vstr_dup_rep_chr(s2->conf, 'X', 8);

  TST_B_TST(ret,  6, !VSTR_CMP_CSTR_EQ(t1,  1, t1->len, cb2));

  vstr_free_base(t1);



  t1 = vstr_dup_rep_chr(s3->conf, 'a', 8);

  TST_B_TST(ret,  7, !VSTR_CMP_CSTR_EQ(t1,  1, 4, cb1));
  TST_B_TST(ret,  8, !VSTR_CMP_CSTR_EQ(t1,  5, 4, cb1));

  vstr_free_base(t1);
  t1 = vstr_dup_rep_chr(s3->conf, 'X', 8);

  TST_B_TST(ret,  9, !VSTR_CMP_CSTR_EQ(t1,  1, t1->len, cb2));

  vstr_free_base(t1);



  t1 = vstr_dup_rep_chr(s4->conf, 'a', 8);

  TST_B_TST(ret, 10, !VSTR_CMP_CSTR_EQ(t1,  1, 4, cb1));
  TST_B_TST(ret, 11, !VSTR_CMP_CSTR_EQ(t1,  5, 4, cb1));

  vstr_free_base(t1);
  t1 = vstr_dup_rep_chr(s4->conf, 'X', 8);

  TST_B_TST(ret, 12, !VSTR_CMP_CSTR_EQ(t1,  1, t1->len, cb2));

  vstr_free_base(t1);

  if (MFAIL_NUM_OK)
  {
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(NULL,     VSTR_TYPE_NODE_BUF, 1000);
    vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_NUM_SPARE_BASE, 0);
    vstr_cntl_conf(s3->conf, VSTR_CNTL_CONF_SET_NUM_SPARE_BASE, 0);

    ASSERT(!s1->conf->spare_base_num);
    ASSERT(!s3->conf->spare_base_num);
    
    TST_B_TST(ret, 13, !tst_mfail_num(1));
    TST_B_TST(ret, 14, vstr_dup_rep_chr(s3->conf, '!', 32));
    TST_B_TST(ret, 15, !tst_mfail_num(2));
    TST_B_TST(ret, 16, vstr_dup_rep_chr(s3->conf, '!', 32));
    TST_B_TST(ret, 17, !tst_mfail_num(1));
    TST_B_TST(ret, 18, vstr_dup_rep_chr(NULL, '!', 32));
    TST_B_TST(ret, 19, !tst_mfail_num(2));
    TST_B_TST(ret, 20, vstr_dup_rep_chr(NULL, '!', 32));
    TST_B_TST(ret, 21, !tst_mfail_num(3));
    TST_B_TST(ret, 22, vstr_dup_rep_chr(NULL, '!', 32));
  }

  t1 = vstr_dup_rep_chr(s1->conf, 'a', 0);
  TST_B_TST(ret, 23, !t1);
  TST_B_TST(ret, 24,  t1->len);
  vstr_free_base(t1);

  return (TST_B_RET(ret));
}
