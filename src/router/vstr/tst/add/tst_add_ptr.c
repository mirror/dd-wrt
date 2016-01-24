#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;

  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);

  VSTR_ADD_CSTR_PTR(s1, s1->len, "");
  VSTR_ADD_CSTR_PTR(s1, s1->len, buf);
  VSTR_ADD_CSTR_PTR(s1, s1->len, "");

  TST_B_TST(ret, 1, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));

  VSTR_ADD_CSTR_PTR(s2, s2->len, "");
  VSTR_ADD_CSTR_PTR(s2, s2->len, buf);
  VSTR_ADD_CSTR_PTR(s2, s2->len, "");

  TST_B_TST(ret, 2, !VSTR_CMP_CSTR_EQ(s2, 1, s2->len, buf));

  vstr_add_cstr_ptr(s3, s3->len, "");
  vstr_add_ptr     (s3, s3->len, buf,  4);
  vstr_add_cstr_ptr(s3, s3->len, buf + 4);
  vstr_add_cstr_ptr(s3, s3->len, "");

  TST_B_TST(ret, 3, !vstr_cmp_cstr_eq(s3, 1, s3->len, buf));

  vstr_export_iovec_ptr_all(s3, NULL, NULL);

  TST_B_TST(ret, 4, vstr_num(s3, 1, s3->len) != 1); /* merged ptrs */

  vstr_add_ptr     (s3, s3->len, buf,  4);
  
  vstr_export_iovec_ptr_all(s3, NULL, NULL);

  TST_B_TST(ret, 4, vstr_num(s3, 1, s3->len) != 2);

  vstr_del(s3, 1, s3->len);
  TST_B_TST(ret, 17, !s3->iovec_upto_date);
  VSTR_ADD_CSTR_PTR(s3, s3->len, "abcdX");
  VSTR_ADD_CSTR_PTR(s3, s3->len, "abcdX");
  TST_B_TST(ret, 18, !s3->iovec_upto_date);
  vstr_del(s3, 1, s3->len);
  TST_B_TST(ret, 19, !s3->iovec_upto_date);
  
  return (TST_B_RET(ret));
}
