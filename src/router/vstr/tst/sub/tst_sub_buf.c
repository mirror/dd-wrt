#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  unsigned int n3 = 0;
  int mfail_count = 0;

  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);
  VSTR_ADD_CSTR_BUF(s1, 0, buf);
  VSTR_ADD_CSTR_BUF(s2, 0, buf);
  VSTR_ADD_CSTR_BUF(s3, 0, buf);
  n3 = s3->num;

  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, buf);
  VSTR_SUB_CSTR_BUF(s2, 1, s2->len, buf);
  VSTR_SUB_CSTR_BUF(s3, 1, s3->len, buf);

  TST_B_TST(ret, 1, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
  TST_B_TST(ret, 2, !VSTR_CMP_CSTR_EQ(s2, 1, s2->len, buf));
  TST_B_TST(ret, 3, !VSTR_CMP_CSTR_EQ(s3, 1, s3->len, buf));

  TST_B_TST(ret, 4, (s1->num != 1));
  TST_B_TST(ret, 5, (s2->num != 1));
  TST_B_TST(ret, 6, (s3->num != n3));

  VSTR_ADD_CSTR_PTR(s1, 4, buf);
  VSTR_ADD_CSTR_PTR(s1, s1->len, buf);
  TST_B_TST(ret, 7, (s1->num != 4));
  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, buf);
  TST_B_TST(ret, 8, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
  TST_B_TST(ret, 9, (s1->num != 1));

  VSTR_ADD_CSTR_PTR(s1, 4, buf);
  VSTR_ADD_CSTR_PTR(s1, 0, buf);
  VSTR_ADD_CSTR_PTR(s1, s1->len, buf);
  TST_B_TST(ret, 10, (s1->num != 5));
  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, buf);
  TST_B_TST(ret, 11, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
  TST_B_TST(ret, 12, (s1->num != 1));

  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "abcd");
  TST_B_TST(ret, 13, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "abcd"));
  TST_B_TST(ret, 14, (s1->num != 1));

  vstr_del(s1, 1, s1->len);
  vstr_add_cstr_ptr(s1, 0, buf);
  vstr_sub_cstr_buf(s1, 1, s1->len, buf);
  TST_B_TST(ret, 15, !vstr_cmp_cstr_eq(s1, 1, s1->len, buf));
  TST_B_TST(ret, 16, (s1->num != 1));

  TST_B_TST(ret, 17, !vstr_sub_buf(s1, 1, s1->len, "", 0));
  TST_B_TST(ret, 18, (s1->len != 0));
  TST_B_TST(ret, 19, (s1->num != 0));

  /* test OOM on slow path */
  vstr_del(s1, 1, s1->len);
  vstr_del(s2, 1, s2->len);

  vstr_add_non(s2, 0, 4);
  vstr_add_cstr_ptr(s2, 0, "x");
  vstr_add_non(s2, 0, 4);
  vstr_add_vstr(s1, 0, s2, 1, s2->len, VSTR_TYPE_ADD_DEF);

  mfail_count = 0;
  do
  {
    ASSERT(vstr_cmp_eq(s1, 1, s1->len, s2, 1, s2->len));

    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_NON, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_PTR, 1000);
    
    tst_mfail_num(++mfail_count);
  } while (!vstr_sub_cstr_buf(s1, 2, s1->len - 2, "abcd"));
  tst_mfail_num(0);

  ASSERT(vstr_cmp_eq(s1, 1, 1, s2, 1, 1));
  ASSERT(vstr_cmp_cstr_eq(s1, 2, s1->len - 2, "abcd"));
  ASSERT(vstr_cmp_eq(s1, s1->len, 1, s2, s2->len, 1));

  /* test OOM on relativly slow path */
  
  vstr_add_cstr_buf(s2, 5, "y");
  
  vstr_sub_vstr(s1, 1, s1->len, s2, 1, s2->len, VSTR_TYPE_SUB_DEF);
  
  mfail_count = 0;
  do
  {
    ASSERT(vstr_cmp_eq(s1, 1, s1->len, s2, 1, s2->len));

    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_NON, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_PTR, 1000);
    
    tst_mfail_num(++mfail_count);
  } while (!vstr_sub_cstr_buf(s1, 2, s1->len - 2, "abcd"));
  tst_mfail_num(0);

  ASSERT(vstr_cmp_eq(s1, 1, 1, s2, 1, 1));
  ASSERT(vstr_cmp_cstr_eq(s1, 2, s1->len - 2, "abcd"));
  ASSERT(vstr_cmp_eq(s1, s1->len, 1, s2, s2->len, 1));

  vstr_del(s2, 1, s2->len);
  vstr_add_cstr_buf(s2, 0, "y");
  
  vstr_sub_vstr(s1, 1, s1->len, s2, 1, s2->len, VSTR_TYPE_SUB_DEF);
  
  mfail_count = 0;
  do
  {
    ASSERT(vstr_cmp_eq(s1, 1, s1->len, s2, 1, s2->len));

    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_NON, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_PTR, 1000);
    
    tst_mfail_num(++mfail_count);
  } while (!vstr_sub_cstr_buf(s1, 1, s1->len, "abcd"));
  tst_mfail_num(0);

  ASSERT(vstr_cmp_cstr_eq(s1, 1, s1->len, "abcd"));
  
  return (TST_B_RET(ret));
}
