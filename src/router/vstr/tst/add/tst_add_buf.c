#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  size_t app_len = 0;

  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);

  VSTR_ADD_CSTR_BUF(s1, s1->len, "");
  vstr_add_buf(s1, s1->len, buf, 1);
  VSTR_ADD_CSTR_BUF(s1, s1->len, buf + 1);
  VSTR_ADD_CSTR_BUF(s1, s1->len, "");

  TST_B_TST(ret,  1, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));

  VSTR_ADD_CSTR_BUF(s2, s2->len, "");
  VSTR_ADD_CSTR_BUF(s2, s2->len, buf);
  VSTR_ADD_CSTR_BUF(s2, s2->len, "");

  TST_B_TST(ret,  2, !VSTR_CMP_CSTR_EQ(s2, 1, s2->len, buf));

  VSTR_ADD_CSTR_BUF(s3, s3->len, "");
  VSTR_ADD_CSTR_BUF(s3, s3->len, buf);
  VSTR_ADD_CSTR_BUF(s3, s3->len, "");

  TST_B_TST(ret,  3, !VSTR_CMP_CSTR_EQ(s3, 1, s3->len, buf));

  VSTR_ADD_CSTR_BUF(s1, 1, buf);

  TST_B_TST(ret,  5, !VSTR_CMP_CSTR_EQ(s1, 2, s1->len / 2, buf));
  TST_B_TST(ret,  6, !VSTR_CMP_CSTR_EQ(s1, 2 + (s1->len / 2), (s1->len / 2) - 1,
                                       buf + 1));

  VSTR_ADD_CSTR_BUF(s2, 1, buf);

  TST_B_TST(ret,  7, !VSTR_CMP_CSTR_EQ(s2, 2, s2->len / 2, buf));
  TST_B_TST(ret,  8, !VSTR_CMP_CSTR_EQ(s2, 2 + (s2->len / 2), (s2->len / 2) - 1,
                                       buf + 1));

  VSTR_ADD_CSTR_BUF(s3, 1, buf);

  TST_B_TST(ret,  9, !VSTR_CMP_CSTR_EQ(s3, 2, s3->len / 2, buf));
  TST_B_TST(ret, 10, !VSTR_CMP_CSTR_EQ(s3, 2 + (s3->len / 2), (s3->len / 2) - 1,
                                       buf + 1));

  app_len = s1->len;
  ASSERT(s1->len == s2->len);
  ASSERT(s1->len == s3->len);

  vstr_export_iovec_ptr_all(s1, NULL, NULL);
  vstr_export_iovec_ptr_all(s2, NULL, NULL);
  vstr_export_iovec_ptr_all(s3, NULL, NULL);
  ASSERT(s1->iovec_upto_date);
  ASSERT(s2->iovec_upto_date);
  ASSERT(s3->iovec_upto_date);

  VSTR_ADD_CSTR_BUF(s1, app_len, "abcdX");
  TST_B_TST(ret, 11,
            !VSTR_CMP_CSTR_EQ(s1, app_len + 1, 5, "abcdX"));
  TST_B_TST(ret, 12, !s1->iovec_upto_date);

  VSTR_ADD_CSTR_BUF(s2, app_len, "abcdX");
  TST_B_TST(ret, 13,
            !VSTR_CMP_CSTR_EQ(s2, app_len + 1, 5, "abcdX"));
  TST_B_TST(ret, 14, !s2->iovec_upto_date);

  vstr_add_cstr_buf(s3, app_len, "abcdX");
  TST_B_TST(ret, 15,
            !VSTR_CMP_CSTR_EQ(s3, app_len + 1, 5, "abcdX"));
  TST_B_TST(ret, 16,  s3->iovec_upto_date); /* too much data */

  vstr_del(s1, 1, s1->len);
  ASSERT(((0) == s1->len) && (vstr_num(s1, 1, s1->len) == 0));
  TST_B_TST(ret, 17, !s1->iovec_upto_date);
  VSTR_ADD_CSTR_BUF(s1, s1->len, "abcdX");
  ASSERT(((5) == s1->len) && (vstr_num(s1, 1, s1->len) == 1));
  TST_B_TST(ret, 18, !s1->iovec_upto_date);
  vstr_del(s1, 1, s1->len);
  ASSERT(((0) == s1->len) && (vstr_num(s1, 1, s1->len) == 0));
  TST_B_TST(ret, 19, !s1->iovec_upto_date);
  VSTR_ADD_CSTR_BUF(s1, s1->len, "abcdX");
  ASSERT(((5) == s1->len) && (vstr_num(s1, 1, s1->len) == 1));
  TST_B_TST(ret, 20, !s1->iovec_upto_date);
  
  vstr_add_non(s1, 0, 2);
  ASSERT(((2 + 5) == s1->len) && (vstr_num(s1, 1, s1->len) == 2));
  vstr_add_cstr_buf(s1, 1, "abcdX");
  ASSERT(((1 + 5 + 1 + 5) == s1->len) && (vstr_num(s1, 1, s1->len) == 4));
  vstr_del(s1, 1, 1);
  ASSERT(((5 + 1 + 5) == s1->len) && (vstr_num(s1, 1, s1->len) == 3));
  vstr_sc_reduce(s1, 1, s1->len, 1);
  ASSERT(((5 + 1 + 4) == s1->len) && (vstr_num(s1, 1, s1->len) == 3));

  TST_B_TST(ret, 28,
            !VSTR_CMP_CSTR_EQ(s1, 1, 5, "abcdX"));

  return (TST_B_RET(ret));
}
