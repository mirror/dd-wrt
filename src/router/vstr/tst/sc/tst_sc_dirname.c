#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  size_t len = 0;

  /* 1 */
  VSTR_ADD_CSTR_BUF(s1, s1->len, "/tmp/james");
  /*                              123456789 */

  vstr_sc_dirname(s1, 1, s1->len, &len);
  TST_B_TST(ret, 1, (len != 5));

  VSTR_ADD_CSTR_BUF(s1, s1->len, "/abcd");
  /*                              123456789 */

  vstr_sc_dirname(s1, 1, s1->len, &len);
  TST_B_TST(ret, 2, (len != 11));

  VSTR_ADD_CSTR_BUF(s1, s1->len, "////");
  /*                              6789 */

  vstr_sc_dirname(s1, 1, s1->len, &len);
  TST_B_TST(ret, 3, (len != 11));

  /* 2 */
  VSTR_ADD_CSTR_BUF(s2, s2->len, "/");

  vstr_sc_dirname(s2, 1, s2->len, &len);
  TST_B_TST(ret, 4, (len != 1));

  VSTR_ADD_CSTR_BUF(s2, s2->len, "///");

  vstr_sc_dirname(s2, 1, s2->len, &len);
  TST_B_TST(ret, 5, (len != 1));

  VSTR_ADD_CSTR_BUF(s2, s2->len, "abcd");

  vstr_sc_dirname(s2, 1, s2->len, &len);
  TST_B_TST(ret, 6, (len != 1));

  /* 3 */
  VSTR_ADD_CSTR_BUF(s3, s3->len, "abcd");

  vstr_sc_dirname(s3, 1, s3->len, &len);
  TST_B_TST(ret, 7, (len != 0));

  VSTR_ADD_CSTR_BUF(s3, s3->len, "///");

  vstr_sc_dirname(s3, 1, s3->len, &len);
  TST_B_TST(ret, 8, (len != 0));

  return (TST_B_RET(ret));
}
