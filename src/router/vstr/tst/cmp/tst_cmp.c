#include "tst-main.c"

static const char *rf = __FILE__;

#include "cmp/tst-cmp.h"

#define TEST_CMP_VSTR_FUNC(x1, x2, x3, y1, y2, y3) \
        vstr_cmp (x1, x2, x3, y1, y2, y3)
#define TEST_CMP_CSTR_FUNC(x1, x2, x3, y1) \
        vstr_cmp_cstr(x1, x2, x3, y1)

int tst(void)
{
  int ret = 0;
  unsigned int count = 0;

  TEST_CMP_EQ_0("",           "");
  TEST_CMP_EQ_0("abcd",       "abcd");
  TEST_CMP_EQ_0("abcd1",      "abcd1");
  TEST_CMP_EQ_0("abcd12345",  "abcd12345");
  TEST_CMP_EQ_0("abcd012345", "abcd012345");

  TEST_CMP_GT_0("a",          "");
  TEST_CMP_GT_0("abcd1234",   "abcd123");
  TEST_CMP_GT_0("abcd01234",  "abcd0123");
  TEST_CMP_GT_0("abcd00",     "abcd0");
  TEST_CMP_GT_0("abcd009",    "abcd00");
  TEST_CMP_GT_0("abcd09",     "abcd00");
  TEST_CMP_GT_0("abcd00z",    "abcd00a");
  TEST_CMP_GT_0("abcd09z",    "abcd09a");
  TEST_CMP_GT_0("abcd0",      "abcd");
  TEST_CMP_GT_0("abcd1",      "abcd");

  vstr_del(s1, 1, s1->len);
  vstr_add_cstr_buf(s1, 0, "abcd");
  vstr_del(s2, 1, s2->len);
  vstr_add_cstr_buf(s2, 0, "abcd");
  TST_B_TST(ret, 25, !!vstr_cmp_cstr(s1, 1, s1->len, "abcd"));
  TST_B_TST(ret, 25, !!VSTR_CMP_CSTR(s1, 1, s1->len, "abcd"));
  TST_B_TST(ret, 25,  !vstr_cmp_buf_eq(s1, 1, s1->len, "abcd", 4));
  TST_B_TST(ret, 25,  !vstr_cmp_cstr_eq(s1, 1, s1->len, "abcd"));
  TST_B_TST(ret, 25,  !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "abcd"));

  TST_B_TST(ret, 26,  !vstr_cmp_bod_cstr_eq(s1, 1, s1->len, "abcd"));
  TST_B_TST(ret, 26,  !vstr_cmp_bod_cstr_eq(s1, 1, s1->len, "abcdXXXX"));
  TST_B_TST(ret, 26, !!vstr_cmp_bod_cstr(s1, 1, s1->len, "abcdXXXX"));
  TST_B_TST(ret, 26, !!vstr_cmp_bod_buf(s1, 1, s1->len, "ab", 2));
  TST_B_TST(ret, 26,  !vstr_cmp_bod_buf_eq(s1, 1, s1->len, "ab", 2));

  TST_B_TST(ret, 27,  !vstr_cmp_eod_cstr_eq(s1, 1, s1->len, "abcd"));
  TST_B_TST(ret, 27,  !vstr_cmp_eod_cstr_eq(s1, 1, s1->len, "XXXXabcd"));
  TST_B_TST(ret, 27, !!vstr_cmp_eod_cstr(s1, 1, s1->len, "XXXXabcd"));
  TST_B_TST(ret, 27, !!vstr_cmp_eod_buf(s1, 1, s1->len, "cd", 2));
  TST_B_TST(ret, 27,  !vstr_cmp_eod_buf_eq(s1, 1, s1->len, "cd", 2));

  TST_B_TST(ret, 28,  !vstr_cmp_eq(s1, 1, s1->len, s2, 1, s2->len));
  TST_B_TST(ret, 28,  !vstr_cmp_bod_eq(s1, 1, s1->len, s2, 1, s2->len));
  TST_B_TST(ret, 28,  !vstr_cmp_bod_eq(s1, 1, 2, s2, 1, s2->len));
  TST_B_TST(ret, 28, !!vstr_cmp_bod(s1, 1, 2, s2, 1, s2->len));
  TST_B_TST(ret, 28,  !vstr_cmp_bod_eq(s1, 1, s1->len, s2, 1, 2));

  TST_B_TST(ret, 29,  !vstr_cmp_eod_eq(s1, 1, s1->len, s2, 1, s2->len));
  TST_B_TST(ret, 29,  !vstr_cmp_eod_eq(s1, 3, 2, s2, 1, s2->len));
  TST_B_TST(ret, 29, !!vstr_cmp_eod(s1, 3, 2, s2, 1, s2->len));
  TST_B_TST(ret, 29,  !vstr_cmp_eod_eq(s1, 1, s1->len, s2, 3, 2));

  vstr_del(s1, 1, s1->len);
  vstr_add_non(s1, 0, 4);
  TST_B_TST(ret, 30,  !vstr_cmp_cstr(s1, 1, s1->len, "aBcD"));
  TST_B_TST(ret, 30,  !VSTR_CMP_CSTR(s1, 1, s1->len, "aBcD"));
  TST_B_TST(ret, 30, !!vstr_cmp_buf_eq(s1, 1, s1->len, "aBcD", 4));
  TST_B_TST(ret, 30, !!vstr_cmp_cstr_eq(s1, 1, s1->len, "aBcD"));
  TST_B_TST(ret, 30, !!VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "aBcD"));
  TST_B_TST(ret, 30,  !vstr_cmp_buf_eq(s1, 1, s1->len, NULL, 4));
  TST_B_TST(ret, 30, !!vstr_cmp_buf_eq(s2, 1, s2->len, NULL, 4));
  TST_B_TST(ret, 30,  !vstr_cmp(s1, 1, s1->len, s2, 1, s2->len));
  TST_B_TST(ret, 30, !!vstr_cmp_eq(s2, 1, s2->len, s1, 1, s1->len));

  return (TST_B_RET(ret));
}
