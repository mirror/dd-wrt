#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  int mfail_count = 0;

  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);
  VSTR_ADD_CSTR_PTR(s1, 0, buf);
  VSTR_ADD_CSTR_PTR(s2, 0, buf);
  VSTR_ADD_CSTR_PTR(s3, 0, buf);

  do
  {
    ASSERT(vstr_cmp_eq(s1, 1, s1->len, s3, 1, s3->len));

    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_PTR, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_NON, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_sub_non(s1, 2, s1->len - 2, 2));
  tst_mfail_num(0);

  TST_B_TST(ret, 1, !VSTR_CMP_BUF_EQ(s1, 2, s1->len - 2, NULL, 2));

  TST_B_TST(ret, 2, !VSTR_CMP_EQ(s1, 1, 1, s2, 1, 1));
  TST_B_TST(ret, 3, !VSTR_CMP_EQ(s1, s1->len, 1, s2, s2->len, 1));

  vstr_sub_non(s1, 1, 1, 1);
  vstr_sub_non(s1, s1->len, 1, 1);

  TST_B_TST(ret, 4, !VSTR_CMP_BUF_EQ(s1, 1, s1->len, NULL, 4));

  TST_B_TST(ret, 5, !vstr_sub_non(s1, 1, s1->len, 0));
  TST_B_TST(ret, 6, (s1->len != 0));
  TST_B_TST(ret, 7, (s1->num != 0));

  vstr_del(s1, 1, s1->len);
  vstr_add_non(s1, 0, 128);

  mfail_count = 0;
  do
  {
    ASSERT(vstr_cmp_buf_eq(s1, 1, s1->len, NULL, 128));

    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_PTR, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_NON, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_ptr(s1, 64, "", 1));
  tst_mfail_num(0);

  TST_B_TST(ret,  8, !VSTR_CMP_BUF_EQ(s1,  1, 64, NULL, 64));
  TST_B_TST(ret,  9, !VSTR_CMP_BUF_EQ(s1, 65,  1,   "",  1));
  TST_B_TST(ret, 10, !VSTR_CMP_BUF_EQ(s1, 66, 64, NULL, 64));

  return (TST_B_RET(ret));
}
