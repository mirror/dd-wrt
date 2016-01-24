#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  Vstr_base *x_str = vstr_make_base(NULL);
  Vstr_base *y_str = vstr_make_base(NULL);

  vstr_add_fmt(x_str, x_str->len, "abcd");
  vstr_add_fmt(y_str, y_str->len, "abcd");

  vstr_add_non(x_str, x_str->len, 4);
  vstr_add_non(y_str, y_str->len, 4);

  vstr_add_fmt(x_str, x_str->len, "xyz");
  vstr_add_fmt(y_str, y_str->len, "xyz");

  TST_B_TST(ret,  1, vstr_cmp_vers(x_str, 1, x_str->len, y_str, 1, y_str->len));
  TST_B_TST(ret,  2, vstr_cmp_case(x_str, 1, x_str->len, y_str, 1, y_str->len));
  TST_B_TST(ret,  3, vstr_cmp(x_str, 1, x_str->len, y_str, 1, y_str->len));

  TST_B_TST(ret,  4, !VSTR_CMP_VERS_CSTR_EQ(x_str, 1, 4, "abcd"));
  TST_B_TST(ret,  5, !VSTR_CMP_CASE_CSTR_EQ(x_str, 1, 4, "AbCd"));
  TST_B_TST(ret,  6, !VSTR_CMP_CSTR_EQ(x_str, 1, 4, "abcd"));

  TST_B_TST(ret,  7, !VSTR_CMP_VERS_BUF_EQ(x_str, 5, 4, NULL, 4));
  TST_B_TST(ret,  8, !VSTR_CMP_CASE_BUF_EQ(x_str, 5, 4, NULL, 4));
  TST_B_TST(ret,  9, !VSTR_CMP_BUF_EQ(x_str, 5, 4, NULL, 4));

  TST_B_TST(ret, 10, !VSTR_CMP_VERS_CSTR_EQ(x_str, 9, 3, "xyz"));
  TST_B_TST(ret, 11, !VSTR_CMP_CASE_CSTR_EQ(x_str, 9, 3, "XyZ"));
  TST_B_TST(ret, 12, !VSTR_CMP_CSTR_EQ(x_str, 9, 3, "xyz"));

  vstr_free_base(x_str);
  vstr_free_base(y_str);

  return (TST_B_RET(ret));
}
