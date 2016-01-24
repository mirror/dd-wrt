#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  int mfail_count = 0;
  Vstr_ref *ref = NULL;
  
  vstr_cntl_conf(s3->conf, VSTR_CNTL_CONF_SET_LOC_CSTR_THOU_SEP, "__--==--__");
  vstr_cntl_conf(s3->conf, VSTR_CNTL_CONF_SET_LOC_CSTR_THOU_GRP, "\3");

  ref = vstr_ref_make_strdup("__--!--__");
  ASSERT(ref);
  mfail_count = 0;
  do
  {
    tst_mfail_num(++mfail_count);
  } while (!vstr_cntl_conf(s3->conf, VSTR_CNTL_CONF_SET_LOC_REF_THOU_SEP,
                           10, ref, strlen(ref->ptr)));
  vstr_ref_del(ref);
  
  mfail_count = 0;
  do
  {
    ASSERT(!s2->len);
    vstr_free_spare_nodes(s2->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!VSTR_SC_ADD_CSTR_GRPNUM_BUF(s2, 0, "AXXXYYYZZZ"));
  
  vstr_del(s2, 1, s2->len);
  mfail_count = 0;
  do
  {
    ASSERT(!s2->len);
    vstr_free_spare_nodes(s2->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_sc_add_cstr_grpnum_buf(s2, 0, "AXXXYYYZZZ"));
  
  mfail_count = 0;
  do
  {
    ASSERT(!s3->len);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_sc_add_cstr_grpbasenum_buf(s3, 0, 8, "AXXXYYYZZZ"));

  TST_B_TST(ret, 1, !VSTR_CMP_CSTR_EQ(s2, 1, s2->len, "A,XXX,YYY,ZZZ"));
  TST_B_TST(ret, 2,
            !VSTR_CMP_CSTR_EQ(s3, 1, s3->len,
                              "A__--==--__XXX__--==--__YYY__--==--__ZZZ"));

  vstr_del(s3, 1, s3->len);
  mfail_count = 0;
  do
  {
    ASSERT(!s3->len);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_PTR, 1000);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_REF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_sc_add_cstr_grpbasenum_ptr(s3, 0, 10, "AXXXYYYZZZ"));

  TST_B_TST(ret, 3,
            !VSTR_CMP_CSTR_EQ(s3, 1, s3->len,
                              "A__--!--__XXX__--!--__YYY__--!--__ZZZ"));
  
  tst_mfail_num(0);
  ref = VSTR_REF_MAKE_STRDUP("AXXXYYYZZZ");
  ASSERT(ref);
  
  vstr_del(s3, 1, s3->len);
  mfail_count = 0;
  do
  {
    ASSERT(!s3->len);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_PTR, 1000);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_REF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!VSTR_SC_ADD_CSTR_GRPBASENUM_REF(s3, 0, 10, ref, 0));

  TST_B_TST(ret, 4,
            !VSTR_CMP_CSTR_EQ(s3, 1, s3->len,
                              "A__--!--__XXX__--!--__YYY__--!--__ZZZ"));
  
  vstr_del(s3, 1, s3->len);
  mfail_count = 0;
  do
  {
    ASSERT(!s3->len);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_PTR, 1000);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_REF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_sc_add_cstr_grpbasenum_ref(s3, 0, 10, ref, 0));
  tst_mfail_num(0);

  TST_B_TST(ret, 4,
            !VSTR_CMP_CSTR_EQ(s3, 1, s3->len,
                              "A__--!--__XXX__--!--__YYY__--!--__ZZZ"));
  vstr_ref_del(ref);
  
  vstr_del(s3, 1, s3->len);
  VSTR_SC_ADD_CSTR_GRPBASENUM_PTR(s3, 0, 10, "AXXXYYYZZZ");
  TST_B_TST(ret, 5,
            !VSTR_CMP_CSTR_EQ(s3, 1, s3->len,
                              "A__--!--__XXX__--!--__YYY__--!--__ZZZ"));
  
  vstr_del(s3, 1, s3->len);
  VSTR_SC_ADD_CSTR_GRPBASENUM_BUF(s3, 0, 10, "AXXXYYYZZZ");
  TST_B_TST(ret, 6,
            !VSTR_CMP_CSTR_EQ(s3, 1, s3->len,
                              "A__--!--__XXX__--!--__YYY__--!--__ZZZ"));
  
  vstr_del(s3, 1, s3->len);
  vstr_sc_add_grpbasenum_buf(s3, 0, 10, "YZZZ", 4);
  TST_B_TST(ret, 6,
            !VSTR_CMP_CSTR_EQ(s3, 1, s3->len, "Y__--!--__ZZZ"));
  
  vstr_del(s3, 1, s3->len);
  vstr_sc_add_grpbasenum_ptr(s3, 0, 10, "YZZZ", 4);
  TST_B_TST(ret, 6,
            !vstr_cmp_cstr_eq(s3, 1, s3->len, "Y__--!--__ZZZ"));
  
  
  
  return (TST_B_RET(ret));
}
