#include "tst-main.c"

static const char *rf = __FILE__;

#include "cmp/tst-cmp.h"

#define TEST_CMP_VSTR_FUNC(x1, x2, x3, y1, y2, y3) \
        vstr_cmp_fast (x1, x2, x3, y1, y2, y3)
#define TEST_CMP_CSTR_FUNC(x1, x2, x3, y1) \
        vstr_cmp_fast_cstr(x1, x2, x3, y1)

int tst(void)
{
  int ret = 0;
  unsigned int count = 0;

  TEST_CMP_EQ_0("",           "");
  TEST_CMP_EQ_0("abcd",       "abcd");
  TEST_CMP_EQ_0("abcd1",      "abcd1");
  TEST_CMP_EQ_0("abcd12345",  "abcd12345");
  TEST_CMP_EQ_0("abcd012345", "abcd012345");

  TEST_CMP_GT_0("",           "a");
  TEST_CMP_GT_0("abcd123",    "abcd1234");
  TEST_CMP_GT_0("abcd0123",   "abcd01234");
  TEST_CMP_GT_0("abcd0",      "abcd00");
  TEST_CMP_GT_0("abcd00",     "abcd009");
  TEST_CMP_GT_0("abcd00",     "abcd09");
  TEST_CMP_GT_0("abcd00a",    "abcd00z");
  TEST_CMP_GT_0("abcd09a",    "abcd09z");
  TEST_CMP_GT_0("abcd",       "abcd0");
  TEST_CMP_GT_0("abcd",       "abcd1");

  return (TST_B_RET(ret));
}
