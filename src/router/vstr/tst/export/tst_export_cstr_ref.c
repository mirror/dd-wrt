#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  size_t off = 0;
  Vstr_ref *ref = NULL;

  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);

  VSTR_ADD_CSTR_BUF(s1, 0, buf);
  VSTR_ADD_CSTR_BUF(s4, 0, buf);

  ref = vstr_export_cstr_ref(s1, 1, s1->len, &off);

  TST_B_TST(ret, 1, strcmp(buf, ((char *)ref->ptr) + off));

  vstr_ref_del(ref);

  ref = vstr_export_cstr_ref(s1, 2, s1->len - 1, &off);

  TST_B_TST(ret, 2, strcmp(buf + 1, ((char *)ref->ptr) + off));

  vstr_ref_del(ref);

  /* no cache */

  ref = vstr_export_cstr_ref(s4, 1, s4->len, &off);

  TST_B_TST(ret, 3, strcmp(buf, ((char *)ref->ptr) + off));

  vstr_ref_del(ref);

  if (MFAIL_NUM_OK)
  {
    Vstr_ref *ref2 = NULL;

    vstr_cache_cb_free(s1, 0);
    TST_B_TST(ret, 4, !tst_mfail_num(1));
    TST_B_TST(ret, 5, (ref2 = vstr_export_cstr_ref(s1, 1, s1->len, &off)));
    TST_B_TST(ret, 6, !tst_mfail_num(2));
    TST_B_TST(ret, 7, (ref2 = vstr_export_cstr_ref(s1, 1, s1->len, &off)));
    TST_B_TST(ret, 8, !tst_mfail_num(3));
    TST_B_TST(ret, 9, !(ref2 = vstr_export_cstr_ref(s1, 1, s1->len, &off)));
    vstr_ref_del(ref2);

    /* no cache */
    TST_B_TST(ret, 10, !tst_mfail_num(1));
    TST_B_TST(ret, 11, (ref2 = vstr_export_cstr_ref(s4, 1, s4->len, &off)));
    TST_B_TST(ret, 12, !tst_mfail_num(2));
    TST_B_TST(ret, 13, !(ref2 = vstr_export_cstr_ref(s4, 1, s4->len, &off)));
    vstr_ref_del(ref2);
  }

  return (TST_B_RET(ret));
}
