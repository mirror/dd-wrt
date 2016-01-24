#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  int mfail_count = 0;
  
  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);
  VSTR_ADD_CSTR_BUF(s1, 0, buf);
  VSTR_ADD_CSTR_BUF(s2, 0, buf);
  VSTR_ADD_CSTR_BUF(s3, 0, buf);
  VSTR_ADD_CSTR_BUF(s4, 0, buf);

  sprintf(buf, "%s", "aaaaaaaaaa");
  
  do
  {
    ASSERT(vstr_cmp_eq(s1, 1, s1->len, s4, 1, s4->len));

    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_sub_rep_chr(s1, 1, s1->len, 'a', 10));
  tst_mfail_num(0);

  vstr_sub_rep_chr(s2, 1, s2->len, 'a', 10);

  ASSERT(vstr_sub_rep_chr(s3, 1, 0, 'a', 0));
  ASSERT(vstr_cmp_eq(s3, 1, s3->len, s4, 1, s4->len));
  
  vstr_sub_rep_chr(s3, 1, 2, 'a', 1);
  vstr_sub_rep_chr(s3, 1, 2, 'a', 2);
  
  vstr_sub_rep_chr(s3, 1, s3->len, 'a', 10);

  TST_B_TST(ret, 1, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
  TST_B_TST(ret, 2, !VSTR_CMP_CSTR_EQ(s2, 1, s2->len, buf));
  TST_B_TST(ret, 3, !VSTR_CMP_CSTR_EQ(s3, 1, s3->len, buf));

  TST_B_TST(ret, 4, !vstr_sub_rep_chr(s1, 1, s1->len, 0, 0));
  TST_B_TST(ret, 5, (s1->len != 0));
  TST_B_TST(ret, 6, (s1->num != 0));
  
  return (TST_B_RET(ret));
}
