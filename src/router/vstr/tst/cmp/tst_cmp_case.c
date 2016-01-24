#include "tst-main.c"

static const char *rf = __FILE__;

#include "cmp/tst-cmp.h"

#define TEST_CMP_VSTR_FUNC(x1, x2, x3, y1, y2, y3) \
        vstr_cmp_case(x1, x2, x3, y1, y2, y3)
#define TEST_CMP_CSTR_FUNC(x1, x2, x3, y1) \
        vstr_cmp_case_cstr(x1, x2, x3, y1)

int tst(void)
{
  int ret = 0;
  unsigned int count = 0;

  TEST_CMP_EQ_0("",           "");
  TEST_CMP_EQ_0("AbCd",       "abcd");
  TEST_CMP_EQ_0("AbCd1",      "abcd1");
  TEST_CMP_EQ_0("AbCd12345",  "abcd12345");
  TEST_CMP_EQ_0("AbCd012345", "abcd012345");

  TEST_CMP_GT_0("Ab",           "");
  TEST_CMP_GT_0("AbCd1234",   "abcd123");
  TEST_CMP_GT_0("AbCd01234",  "abcd0123");
  TEST_CMP_GT_0("AbCd00",     "abcd0");
  TEST_CMP_GT_0("AbCd009",    "abcd00");
  TEST_CMP_GT_0("AbCd09",     "abcd00");
  TEST_CMP_GT_0("AbCd00z",    "abcd00a");
  TEST_CMP_GT_0("AbCd09z",    "abcd09a");
  TEST_CMP_GT_0("AbCd0",      "abcd");
  TEST_CMP_GT_0("AbCd1",      "abcd");

  vstr_del(s1, 1, s1->len);
  vstr_add_cstr_buf(s1, 0, "AbCd");
  vstr_del(s2, 1, s2->len);
  vstr_add_cstr_buf(s2, 0, "aBcD");
  TST_B_TST(ret, 25, !!vstr_cmp_case_cstr(s1, 1, s1->len, "aBcD"));
  TST_B_TST(ret, 25, !!VSTR_CMP_CASE_CSTR(s1, 1, s1->len, "aBcD"));
  TST_B_TST(ret, 25,  !vstr_cmp_case_buf_eq(s1, 1, s1->len, "aBcD", 4));
  TST_B_TST(ret, 25,  !vstr_cmp_case_cstr_eq(s1, 1, s1->len, "aBcD"));
  TST_B_TST(ret, 25,  !VSTR_CMP_CASE_CSTR_EQ(s1, 1, s1->len, "aBcD"));

  TST_B_TST(ret, 26,  !vstr_cmp_case_bod_cstr_eq(s1, 1, s1->len, "aBcD"));
  TST_B_TST(ret, 26,  !vstr_cmp_case_bod_cstr_eq(s1, 1, s1->len, "aBcDXXXX"));
  TST_B_TST(ret, 26, !!vstr_cmp_case_bod_cstr(s1, 1, s1->len, "aBcDXXXX"));
  TST_B_TST(ret, 26, !!vstr_cmp_case_bod_buf(s1, 1, s1->len, "aB", 2));
  TST_B_TST(ret, 26,  !vstr_cmp_case_bod_buf_eq(s1, 1, s1->len, "aB", 2));

  TST_B_TST(ret, 27,  !vstr_cmp_case_eod_cstr_eq(s1, 1, s1->len, "aBcD"));
  TST_B_TST(ret, 27,  !vstr_cmp_case_eod_cstr_eq(s1, 1, s1->len, "XXXXaBcD"));
  TST_B_TST(ret, 27, !!vstr_cmp_case_eod_cstr(s1, 1, s1->len, "XXXXaBcD"));
  TST_B_TST(ret, 27, !!vstr_cmp_case_eod_buf(s1, 1, s1->len, "cD", 2));
  TST_B_TST(ret, 27,  !vstr_cmp_case_eod_buf_eq(s1, 1, s1->len, "cD", 2));

  TST_B_TST(ret, 28,  !vstr_cmp_case_eq(s1, 1, s1->len, s2, 1, s2->len));
  TST_B_TST(ret, 28,  !vstr_cmp_case_bod_eq(s1, 1, s1->len, s2, 1, s2->len));
  TST_B_TST(ret, 28,  !vstr_cmp_case_bod_eq(s1, 1, 2, s2, 1, s2->len));
  TST_B_TST(ret, 28, !!vstr_cmp_case_bod(s1, 1, 2, s2, 1, s2->len));
  TST_B_TST(ret, 28,  !vstr_cmp_case_bod_eq(s1, 1, s1->len, s2, 1, 2));

  TST_B_TST(ret, 29,  !vstr_cmp_case_eod_eq(s1, 1, s1->len, s2, 1, s2->len));
  TST_B_TST(ret, 29,  !vstr_cmp_case_eod_eq(s1, 3, 2, s2, 1, s2->len));
  TST_B_TST(ret, 29, !!vstr_cmp_case_eod(s1, 3, 2, s2, 1, s2->len));
  TST_B_TST(ret, 29,  !vstr_cmp_case_eod_eq(s1, 1, s1->len, s2, 3, 2));

  vstr_del(s1, 1, s1->len);
  vstr_add_non(s1, 0, 4);
  TST_B_TST(ret, 30,  !vstr_cmp_case_cstr(s1, 1, s1->len, "aBcD"));
  TST_B_TST(ret, 30,  !VSTR_CMP_CASE_CSTR(s1, 1, s1->len, "aBcD"));
  TST_B_TST(ret, 30, !!vstr_cmp_case_buf_eq(s1, 1, s1->len, "aBcD", 4));
  TST_B_TST(ret, 30, !!vstr_cmp_case_cstr_eq(s1, 1, s1->len, "aBcD"));
  TST_B_TST(ret, 30, !!VSTR_CMP_CASE_CSTR_EQ(s1, 1, s1->len, "aBcD"));
  TST_B_TST(ret, 30,  !vstr_cmp_case_buf_eq(s1, 1, s1->len, NULL, 4));
  TST_B_TST(ret, 30, !!vstr_cmp_case_buf_eq(s2, 1, s2->len, NULL, 4));
  TST_B_TST(ret, 30,  !vstr_cmp_case(s1, 1, s1->len, s2, 1, s2->len));
  TST_B_TST(ret, 30, !!vstr_cmp_case_eq(s2, 1, s2->len, s1, 1, s1->len));
  
  return (TST_B_RET(ret));
}
