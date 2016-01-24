#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  int mfail_count = 0;

  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);

  VSTR_ADD_CSTR_BUF(s1, 0, buf);
  VSTR_ADD_CSTR_PTR(s1, 0, buf);

  VSTR_ADD_CSTR_BUF(s2, 0, buf);
  VSTR_ADD_CSTR_PTR(s2, 0, buf);

  do
  {
    ASSERT(vstr_cmp_eq(s1, 1, s1->len, s2, 1, s2->len));

    vstr_free_spare_nodes(s2->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s2->conf, VSTR_TYPE_NODE_PTR, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_sub_vstr(s2, 1, s2->len - 1, s1, 1, s1->len,
                          VSTR_TYPE_SUB_DEF));
  tst_mfail_num(0);

  vstr_del(s2, s2->len, 1);

  TST_B_TST(ret,  1, !VSTR_CMP_EQ(s1, 1, s1->len, s2, 1, s2->len));

  vstr_sub_vstr(s2, 1, s2->len, s1, 1, s1->len, VSTR_TYPE_SUB_BUF_PTR);
  TST_B_TST(ret,  2, !VSTR_CMP_EQ(s1, 1, s1->len, s2, 1, s2->len));

  vstr_sub_vstr(s2, 1, s2->len, s1, 1, s1->len, VSTR_TYPE_SUB_BUF_REF);
  TST_B_TST(ret,  3, !VSTR_CMP_EQ(s1, 1, s1->len, s2, 1, s2->len));

  vstr_sub_vstr(s2, 1, s2->len, s1, 1, s1->len, VSTR_TYPE_SUB_ALL_REF);
  TST_B_TST(ret,  4, !VSTR_CMP_EQ(s1, 1, s1->len, s2, 1, s2->len));

  vstr_sub_vstr(s2, 1, s2->len, s1, 1, s1->len, VSTR_TYPE_SUB_ALL_BUF);
  TST_B_TST(ret,  5, !VSTR_CMP_EQ(s1, 1, s1->len, s2, 1, s2->len));

  vstr_sub_vstr(s2, 1, s2->len, s1, 1, s1->len, VSTR_TYPE_SUB_DEF);
  TST_B_TST(ret,  6, !VSTR_CMP_EQ(s1, 1, s1->len, s2, 1, s2->len));

  TST_B_TST(ret,  7, !VSTR_CMP_CSTR_EQ(s2, 1, s2->len,
                                       vstr_export_cstr_ptr(s1, 1, s1->len)));

  TST_B_TST(ret,  8, (vstr_export_cstr_ptr(s1, 1, s1->len) !=
                      vstr_export_cstr_ptr(s1, 1, s1->len)));

  memcpy(buf, "abcd", 4); /* alter the data at ptr */
  vstr_cache_cb_sub(s1, strlen(buf) + 1, strlen(buf));
  vstr_cache_cb_sub(s2, strlen(buf) + 1, strlen(buf)); /* should be nop */

  TST_B_TST(ret,  9, !VSTR_CMP_CSTR_EQ(s2, 1, s2->len,
                                       vstr_export_cstr_ptr(s1, 1, s1->len)));

  TST_B_TST(ret, 10,
            !vstr_sub_vstr(s2, 1, s2->len, s3, 1, s3->len, VSTR_TYPE_SUB_DEF));
  TST_B_TST(ret, 11, !VSTR_CMP_EQ(s3, 1, s3->len, s2, 1, s2->len));

  TST_B_TST(ret, 12,
            !vstr_sub_vstr(s2, 1, s2->len, s3, 1, s3->len, VSTR_TYPE_SUB_DEF));
  TST_B_TST(ret, 13, !VSTR_CMP_EQ(s3, 1, s3->len, s2, 1, s2->len));

  vstr_del(s3, 1, s3->len);
  vstr_add_vstr(s3, 0, s1, 1, s1->len, 0);

  TST_B_TST(ret, 14, !VSTR_CMP_EQ(s1, 1, s1->len, s3, 1, s3->len));
  TST_B_TST(ret, 15, !vstr_sub_vstr(s1, 1, 0, s3, 1, 0, VSTR_TYPE_SUB_DEF));
  TST_B_TST(ret, 16, !VSTR_CMP_EQ(s1, 1, s1->len, s3, 1, s3->len));

  TST_B_TST(ret, 17, !vstr_sub_vstr(s1, 1, 0, s3, 1, 1,
                                    VSTR_TYPE_SUB_DEF)); /* add */
  TST_B_TST(ret, 18, !VSTR_CMP_EQ(s1, 2, s1->len - 1, s3, 1, s3->len));
  TST_B_TST(ret, 18, !VSTR_CMP_EQ(s1, 1, 1, s3, 1, 1));

  TST_B_TST(ret, 19, !vstr_sub_vstr(s1, 1, 1, s3, 1, 0,
                                    VSTR_TYPE_SUB_DEF)); /* del */
  TST_B_TST(ret, 20, !VSTR_CMP_EQ(s1, 1, s1->len, s3, 1, s3->len));

  return (TST_B_RET(ret));
}
