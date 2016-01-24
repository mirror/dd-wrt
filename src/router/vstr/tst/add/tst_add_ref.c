#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  static Vstr_ref ref1;
  static Vstr_ref ref2;
  static Vstr_ref ref3;
  int ret = 0;
  char nil[1] = "";

  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);

  ref1.func = vstr_ref_cb_free_nothing;
  ref1.ptr = nil;
  ref1.ref = 0;

  ref2.func = vstr_ref_cb_free_nothing;
  ref2.ptr = buf;
  ref2.ref = 0;

  ref3.func = vstr_ref_cb_free_nothing;
  ref3.ptr = nil;
  ref3.ref = 0;

  VSTR_ADD_CSTR_REF(s1, s1->len, &ref1, 0);
  VSTR_ADD_CSTR_REF(s1, s1->len, &ref2, 0);
  VSTR_ADD_CSTR_REF(s1, s1->len, &ref3, 0);

  TST_B_TST(ret, 1, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));

  VSTR_ADD_CSTR_REF(s2, s2->len, &ref1, 0);
  VSTR_ADD_CSTR_REF(s2, s2->len, &ref2, 0);
  VSTR_ADD_CSTR_REF(s2, s2->len, &ref3, 0);

  TST_B_TST(ret, 2, !VSTR_CMP_CSTR_EQ(s2, 1, s2->len, buf));

  vstr_add_cstr_ref(s3, s3->len, &ref1, 0);
  vstr_add_ref     (s3, s3->len, &ref2, 0, 4);
  vstr_add_cstr_ref(s3, s3->len, &ref2, 4);
  vstr_add_cstr_ref(s3, s3->len, &ref3, 0);

  TST_B_TST(ret, 3, !vstr_cmp_cstr_eq(s3, 1, s3->len, buf));

  return (TST_B_RET(ret));
}
