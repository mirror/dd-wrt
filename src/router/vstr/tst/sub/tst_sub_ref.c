#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  static Vstr_ref ref;
  int ret = 0;
  int mfail_count = 0;

  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);
  VSTR_ADD_CSTR_PTR(s1, s1->len, buf);

  ref.ptr = buf;
  ref.func = vstr_ref_cb_free_nothing;
  ref.ref = 0;

  vstr_add_vstr(s3, 0, s1, 1, s1->len, 0);

  mfail_count = 3;
  do
  {
    ASSERT(vstr_cmp_eq(s1, 1, s1->len, s3, 1, s3->len));

    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_PTR, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_REF, 1000);
    tst_mfail_num(++mfail_count / 4);
  } while (!VSTR_SUB_CSTR_REF(s1, 1, s1->len, &ref, 0));
  tst_mfail_num(0);

  TST_B_TST(ret, 1, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));

  vstr_del(s1, 1, 1);
  mfail_count = 3;
  do
  {
    ASSERT(vstr_cmp_cstr_eq(s1, 1, s1->len, buf + 1));

    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_PTR, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_REF, 1000);
    tst_mfail_num(++mfail_count / 4);
  } while (!VSTR_SUB_CSTR_REF(s1, 1, s1->len, &ref, 0));
  tst_mfail_num(0);
  
  TST_B_TST(ret, 2, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));

  mfail_count = 3;
  do
  {
    ASSERT(vstr_cmp_cstr_eq(s1, 1, s1->len, buf));

    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_PTR, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_REF, 1000);
    tst_mfail_num(++mfail_count / 4);
  } while (!VSTR_SUB_CSTR_REF(s1, 2, s1->len - 1, &ref, 1));
  tst_mfail_num(0);
  
  TST_B_TST(ret, 3, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));

  vstr_del(s1, 1, s1->len);
  VSTR_ADD_CSTR_BUF(s1, s1->len, "abcd");

  vstr_sub_cstr_ref(s1, 1, s1->len, &ref, 0);
  TST_B_TST(ret, 4, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));

  {
    static Vstr_ref a;
    static Vstr_ref b;
    static Vstr_ref c;
    static Vstr_ref d;
    static Vstr_ref XY;
    int tmp = 0;
    
    a.ptr  = (void *)"a";  a.func  = vstr_ref_cb_free_nothing; a.ref  = 0;
    b.ptr  = (void *)"b";  b.func  = vstr_ref_cb_free_nothing; b.ref  = 0;
    c.ptr  = (void *)"c";  c.func  = vstr_ref_cb_free_nothing; c.ref  = 0;
    d.ptr  = (void *)"d";  d.func  = vstr_ref_cb_free_nothing; d.ref  = 0;
    XY.ptr = (void *)"XY"; XY.func = vstr_ref_cb_free_nothing; XY.ref = 0;

    strcat(buf, "abcd");
    ASSERT(vstr_add_cstr_ref(s1, s1->len, &a,  0));
    ASSERT(vstr_add_cstr_ref(s1, s1->len, &XY, 0));
    ASSERT(vstr_add_cstr_ref(s1, s1->len, &c,  0));
    ASSERT(vstr_add_cstr_ref(s1, s1->len, &d,  0));
    /* sets up the pos cache */
    tmp = vstr_srch_cstr_buf_fwd(s1, s1->len - 1, 2, "XY");
    ASSERT(!tmp);
    TST_B_TST(ret, 5, !vstr_sub_cstr_ref(s1, s1->len - 3, 2, &b, 0));
    TST_B_TST(ret, 6, !vstr_cmp_cstr_eq(s1, 1, s1->len, buf));

    TST_B_TST(ret, 7, !vstr_sub_ref(s1, 1, s1->len, &ref, 0, 0));
    TST_B_TST(ret, 8, (s1->len != 0));
    TST_B_TST(ret, 9, (s1->num != 0));
  }

  vstr_del(s1, 1, s1->len);
  vstr_add_cstr_ref(s1, 0, &ref, 0);

  mfail_count = 0;
  do
  {
    ASSERT(vstr_cmp_cstr_eq(s1, 1, s1->len, ref.ptr));

    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_PTR, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_REF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_ptr(s1, 16, "", 1));
  tst_mfail_num(0);

  TST_B_TST(ret, 10, !VSTR_CMP_BUF_EQ(s1,  1, 16, ref.ptr, 16));
  TST_B_TST(ret, 11, !VSTR_CMP_BUF_EQ(s1, 17,  1,      "",  1));
  TST_B_TST(ret, 12, !VSTR_CMP_BUF_EQ(s1, 18, s1->len - 17,
                                      ((char *)ref.ptr) + 16,
                                      strlen(ref.ptr)   - 16));
  
  return (TST_B_RET(ret));
}
