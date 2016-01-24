#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int mfail_count = 0;
  
  VSTR_ADD_CSTR_BUF(s3, s3->len, "abcd XYZ foo BAR\n");
  VSTR_ADD_CSTR_PTR(s3, s3->len, "ABCD xyz FOO bar\n");

  VSTR_ADD_CSTR_BUF(s2, s2->len, "abcd xyz foo bar\n");
  VSTR_ADD_CSTR_PTR(s2, s2->len, "abcd xyz foo bar\n");

  do
  {
    ASSERT(s3->node_ptr_used);
    ASSERT(!vstr_cmp_eq(s2, 1, s2->len, s3, 1, s3->len));
    ASSERT(VSTR_CMP_CASE_EQ(s2, 1, s2->len, s3, 1, s3->len));
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_PTR, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_conv_lowercase(s3, 1, s3->len));
  tst_mfail_num(0);
  
  return (!VSTR_CMP_EQ(s3, 1, s3->len, s2, 1, s2->len));
}
