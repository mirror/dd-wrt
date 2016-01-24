#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  size_t pos = 0;
  size_t len = 0;

  /* 1 */
  VSTR_ADD_CSTR_BUF(s1, s1->len, "/tmp/james");
  /*                              123456789 */

  vstr_sc_basename(s1, 1, s1->len, &pos, &len);
  TST_B_TST(ret, 1, (pos != 6));
  TST_B_TST(ret, 2, (len != 5));

  VSTR_ADD_CSTR_BUF(s1, s1->len, "/abcd");
  /*                              123456789 */

  vstr_sc_basename(s1, 1, s1->len, &pos, &len);
  TST_B_TST(ret, 3, (pos != 12));
  TST_B_TST(ret, 4, (len != 4));

  VSTR_ADD_CSTR_BUF(s1, s1->len, "////");
  /*                              6789 */

  vstr_sc_basename(s1, 1, s1->len, &pos, &len);
  TST_B_TST(ret, 5, (pos != 12));
  TST_B_TST(ret, 6, (len != 4));

  /* 2 */
  VSTR_ADD_CSTR_BUF(s2, s2->len, "/");

  vstr_sc_basename(s2, 1, s2->len, &pos, &len);
  TST_B_TST(ret, 7, (pos != 1));
  TST_B_TST(ret, 8, (len != 0));

  VSTR_ADD_CSTR_BUF(s2, s2->len, "///");

  vstr_sc_basename(s2, 1, s2->len, &pos, &len);
  TST_B_TST(ret, 9, (pos != 1));
  TST_B_TST(ret, 10, (len != 0));

  VSTR_ADD_CSTR_BUF(s2, s2->len, "abcd");

  vstr_sc_basename(s2, 1, s2->len, &pos, &len);
  TST_B_TST(ret, 11, (pos != 5));
  TST_B_TST(ret, 12, (len != 4));

  /* 3 */
  VSTR_ADD_CSTR_BUF(s3, s3->len, "abcd");

  vstr_sc_basename(s3, 1, s3->len, &pos, &len);
  TST_B_TST(ret, 13, (pos != 1));
  TST_B_TST(ret, 14, (len != 4));

  VSTR_ADD_CSTR_BUF(s3, s3->len, "///");

  vstr_sc_basename(s3, 1, s3->len, &pos, &len);
  TST_B_TST(ret, 15, (pos != 1));
  TST_B_TST(ret, 16, (len != 4));

  return (TST_B_RET(ret));
}
