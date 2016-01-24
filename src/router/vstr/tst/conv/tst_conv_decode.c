#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int mfail_count = 0;

  VSTR_ADD_CSTR_BUF(s1, s1->len, "abcd    x%yz\nX");
  VSTR_ADD_CSTR_PTR(s1, s1->len, "abcd    x%yz\nX%2");

  VSTR_ADD_CSTR_BUF(s2, s2->len, "abcd%20%20%20%20x%yz%0AX");
  VSTR_ADD_CSTR_PTR(s2, s2->len, "abcd%20%20%20%20x%yz%0aX%2");

  VSTR_ADD_CSTR_BUF(s3, s3->len, "abcd%20%20%20%20x%yz%0AX");
  VSTR_ADD_CSTR_PTR(s3, s3->len, "abcd%20%20%20%20x%yz%0aX%2");

  do
  {
    ASSERT(vstr_cmp_eq(s2, 1, s2->len, s3, 1, s3->len));
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_PTR, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_conv_decode_uri(s3, 1, s3->len));
  tst_mfail_num(0);
  
  return (!VSTR_CMP_EQ(s1, 1, s1->len, s3, 1, s3->len));
}
