#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;

  VSTR_ADD_CSTR_BUF(s1, 0, "abcd xyz");

  TST_B_TST(ret,  1, VSTR_CSRCH_CSTR_CHRS_FWD(s1, 1, s1->len, "acd xyz") != 2);
  TST_B_TST(ret,  2, VSTR_CSRCH_CSTR_CHRS_FWD(s1, 2, 7, "acd xyz") != 2);
  TST_B_TST(ret,  3, VSTR_CSRCH_CSTR_CHRS_FWD(s1, 2, 1, "acd xyz") != 2);

  TST_B_TST(ret,  4, VSTR_CSRCH_CSTR_CHRS_FWD(s1, 1, 8, "abcd xy") != 8);
  TST_B_TST(ret,  5, VSTR_CSRCH_CSTR_CHRS_FWD(s1, 2, 7, "abcd xy") != 8);
  TST_B_TST(ret,  6, VSTR_CSRCH_CSTR_CHRS_FWD(s1, 8, 1, "abcd xy") != 8);

  TST_B_TST(ret,  7, vstr_csrch_cstr_chrs_fwd(s1, 4, 3, "dx") != 5);
  TST_B_TST(ret,  8, vstr_csrch_cstr_chrs_fwd(s1, 5, 1, "dx") != 5);

  /* rev */

  TST_B_TST(ret, 11, VSTR_CSRCH_CSTR_CHRS_REV(s1, 1, s1->len, "acd xyz") != 2);
  TST_B_TST(ret, 12, VSTR_CSRCH_CSTR_CHRS_REV(s1, 2, 7, "acd xyz") != 2);
  TST_B_TST(ret, 13, VSTR_CSRCH_CSTR_CHRS_REV(s1, 2, 1, "acd xyz") != 2);

  TST_B_TST(ret, 14, VSTR_CSRCH_CSTR_CHRS_REV(s1, 1, 8, "abcd xy") != 8);
  TST_B_TST(ret, 15, VSTR_CSRCH_CSTR_CHRS_REV(s1, 2, 7, "abcd xy") != 8);
  TST_B_TST(ret, 16, VSTR_CSRCH_CSTR_CHRS_REV(s1, 8, 1, "abcd xy") != 8);

  TST_B_TST(ret, 17, vstr_csrch_cstr_chrs_rev(s1, 4, 3, "dx") != 5);
  TST_B_TST(ret, 18, vstr_csrch_cstr_chrs_rev(s1, 5, 1, "dx") != 5);

  /* misc */
  TST_B_TST(ret, 19, vstr_csrch_cstr_chrs_fwd(s1, 1, s1->len, "abcd xyz") != 0);
  TST_B_TST(ret, 20, vstr_csrch_cstr_chrs_rev(s1, 1, s1->len, "abcd xyz") != 0);
  
  return (TST_B_RET(ret));
}
