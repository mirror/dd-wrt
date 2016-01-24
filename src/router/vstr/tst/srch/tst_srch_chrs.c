#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;

  VSTR_ADD_CSTR_BUF(s1, 0, "abcd xyz");

  TST_B_TST(ret,  1, VSTR_SRCH_CSTR_CHRS_FWD(s1, 1, s1->len, "b") != 2);

  TST_B_TST(ret,  2, VSTR_SRCH_CSTR_CHRS_FWD(s1, 1, s1->len, "b") != 2);
  TST_B_TST(ret,  3, VSTR_SRCH_CSTR_CHRS_FWD(s1, 2, s1->len - 1, "b") != 2);

  TST_B_TST(ret,  4, VSTR_SRCH_CSTR_CHRS_FWD(s1, 1, s1->len, "z") != 8);
  TST_B_TST(ret,  5, VSTR_SRCH_CSTR_CHRS_FWD(s1, 2, s1->len - 1, "z") != 8);

  TST_B_TST(ret,  6, VSTR_SRCH_CSTR_CHRS_FWD(s1, 1, s1->len, "x") != 6);
  TST_B_TST(ret,  7, vstr_srch_cstr_chrs_fwd(s1, 2, s1->len - 1, "x") != 6);
  TST_B_TST(ret,  8, vstr_srch_cstr_chrs_fwd(s1, 2, s1->len - 2, "x") != 6);

  /* rev */

  TST_B_TST(ret, 11, VSTR_SRCH_CSTR_CHRS_REV(s1, 1, s1->len, "b") != 2);

  TST_B_TST(ret, 12, VSTR_SRCH_CSTR_CHRS_REV(s1, 1, s1->len, "b") != 2);
  TST_B_TST(ret, 13, VSTR_SRCH_CSTR_CHRS_REV(s1, 2, s1->len - 1, "b") != 2);

  TST_B_TST(ret, 14, VSTR_SRCH_CSTR_CHRS_REV(s1, 1, s1->len, "z") != 8);
  TST_B_TST(ret, 15, VSTR_SRCH_CSTR_CHRS_REV(s1, 2, s1->len - 1, "z") != 8);

  TST_B_TST(ret, 16, VSTR_SRCH_CSTR_CHRS_REV(s1, 1, s1->len, "x") != 6);
  TST_B_TST(ret, 17, vstr_srch_cstr_chrs_rev(s1, 2, s1->len - 1, "x") != 6);
  TST_B_TST(ret, 18, vstr_srch_cstr_chrs_rev(s1, 2, s1->len - 2, "x") != 6);

  /* misc */
  TST_B_TST(ret, 19, vstr_srch_cstr_chrs_rev(s1, 1, s1->len, "Z") != 0);
  
  return (TST_B_RET(ret));
}
