#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  const char *const cb1 = "aaaa";
  const char *const cb2 = "XXXXXXXX";
  size_t app_len = 0;

  vstr_add_rep_chr(s1, 0, 'a', 8);
  vstr_add_rep_chr(s1, 4, 'X', 8);
  vstr_add_rep_chr(s2, 0, 'a', 8);
  vstr_add_rep_chr(s2, 4, 'X', 8);
  vstr_add_rep_chr(s3, 0, 'a', 8);
  vstr_add_rep_chr(s3, 4, 'X', 8);
  vstr_add_rep_chr(s4, 0, 'a', 8);
  vstr_add_rep_chr(s4, 4, 'X', 8);

  TST_B_TST(ret,  1, !VSTR_CMP_CSTR_EQ(s1,  1, s1->len / 4, cb1));
  TST_B_TST(ret,  2, !VSTR_CMP_CSTR_EQ(s1,  5, s1->len / 2, cb2));
  TST_B_TST(ret,  3, !VSTR_CMP_CSTR_EQ(s1, 13, s1->len / 4, cb1));

  TST_B_TST(ret,  4, !VSTR_CMP_CSTR_EQ(s2,  1, s1->len / 4, cb1));
  TST_B_TST(ret,  5, !VSTR_CMP_CSTR_EQ(s2,  5, s1->len / 2, cb2));
  TST_B_TST(ret,  6, !VSTR_CMP_CSTR_EQ(s2, 13, s1->len / 4, cb1));

  TST_B_TST(ret,  7, !VSTR_CMP_CSTR_EQ(s3,  1, s1->len / 4, cb1));
  TST_B_TST(ret,  8, !VSTR_CMP_CSTR_EQ(s3,  5, s1->len / 2, cb2));
  TST_B_TST(ret,  9, !VSTR_CMP_CSTR_EQ(s3, 13, s1->len / 4, cb1));

  TST_B_TST(ret, 10, !VSTR_CMP_CSTR_EQ(s4,  1, s1->len / 4, cb1));
  TST_B_TST(ret, 10, !VSTR_CMP_CSTR_EQ(s4,  5, s1->len / 2, cb2));
  TST_B_TST(ret, 10, !VSTR_CMP_CSTR_EQ(s4, 13, s1->len / 4, cb1));

  app_len = s1->len;
  ASSERT(s1->len == s2->len);
  ASSERT(s1->len == s3->len);

  vstr_export_iovec_ptr_all(s1, NULL, NULL);
  vstr_export_iovec_ptr_all(s2, NULL, NULL);
  vstr_export_iovec_ptr_all(s3, NULL, NULL);
  ASSERT(s1->iovec_upto_date);
  ASSERT(s2->iovec_upto_date);
  ASSERT(s3->iovec_upto_date);

  vstr_add_rep_chr(s1, app_len, 'Z', 5);
  TST_B_TST(ret, 11,
            !VSTR_CMP_CSTR_EQ(s1, app_len + 1, 5, "ZZZZZ"));
  TST_B_TST(ret, 12, !s1->iovec_upto_date);

  vstr_add_rep_chr(s2, app_len, 'Z', 5);
  TST_B_TST(ret, 13,
            !VSTR_CMP_CSTR_EQ(s2, app_len + 1, 5, "ZZZZZ"));
  TST_B_TST(ret, 14, !s2->iovec_upto_date);

  vstr_add_rep_chr(s3, app_len, 'Z', 5);
  TST_B_TST(ret, 15,
            !VSTR_CMP_CSTR_EQ(s3, app_len + 1, 5, "ZZZZZ"));
  TST_B_TST(ret, 16,  s3->iovec_upto_date); /* too much data */

  return (TST_B_RET(ret));
}
