#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int mfail_count = 0;
  
  VSTR_ADD_CSTR_BUF(s3, s3->len, "XYZabcd");
  VSTR_ADD_CSTR_PTR(s3, s3->len, "abcd");

  VSTR_ADD_CSTR_PTR(s2, s2->len, "xyz1234");

  do
  {
    ASSERT(vstr_cmp_cstr_eq(s3, 1, 3, "XYZ"));
    ASSERT(VSTR_CMP_CASE_EQ(s2, 1, 3, s3, 1, 3));
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_PTR, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_conv_lowercase(s3, 1, 3));
  tst_mfail_num(0);
  
  return (!VSTR_CMP_EQ(s3, 1, 3, s2, 1, 3));
}
