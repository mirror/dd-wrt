#include "tst-main.c"

static const char *rf = __FILE__;

static int ret = 0;

static void tst_vstr_ci(unsigned int num, Vstr_conf *conf,
                        Vstr_base *t_from, size_t pos, size_t len,
                        unsigned int flags)
{
  Vstr_base *t1 = vstr_dup_vstr(conf, t_from, pos, len, flags);

  if ((flags == VSTR_TYPE_ADD_ALL_REF) &&
      vstr_srch_buf_fwd(t_from, pos, len, NULL, 1))
  { /* if it's a _ALL_REF and has _NON data, then
     *   the _NON data becomes random crap ... so replace it before we test */
    size_t np = vstr_srch_buf_fwd(t_from, pos, len, NULL, 1);
    size_t nl = vstr_spn_chrs_fwd(t_from, np, len - vstr_sc_posdiff(pos, np) -1,
                                  NULL, 1);
    vstr_sub_non(t1, vstr_sc_posdiff(pos, np), nl, nl);
  }

  TST_B_TST(ret, num, !VSTR_CMP_EQ(t1, 1, t1->len, t_from, pos, len));

  vstr_free_base(t1);

  if (MFAIL_NUM_OK)
  {
    unsigned int mfail_count = 2;

    num += 16;

    vstr_free_spare_nodes(conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(conf, VSTR_TYPE_NODE_NON, 1000);
    vstr_free_spare_nodes(conf, VSTR_TYPE_NODE_PTR, 1000);
    vstr_free_spare_nodes(conf, VSTR_TYPE_NODE_REF, 1000);
    vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_NUM_SPARE_BASE, 0);

    ASSERT(!conf->spare_base_num);
    
    TST_B_TST(ret, num, !tst_mfail_num(1));
    TST_B_TST(ret, num, vstr_dup_vstr(conf, t_from, pos, len, flags));
    TST_B_TST(ret, num, !tst_mfail_num(2));
    TST_B_TST(ret, num, vstr_dup_vstr(conf, t_from, pos, len, flags));

    t1 = NULL;
    while (!t1) /* keep trying until we succeed now */
    {
      vstr_free_spare_nodes(conf, VSTR_TYPE_NODE_BUF, 1000);
      vstr_free_spare_nodes(conf, VSTR_TYPE_NODE_NON, 1000);
      vstr_free_spare_nodes(conf, VSTR_TYPE_NODE_PTR, 1000);
      vstr_free_spare_nodes(conf, VSTR_TYPE_NODE_REF, 1000);

      tst_mfail_num(++mfail_count);
      t1 = vstr_dup_vstr(conf, t_from, pos, len, flags);
    }
    vstr_free_base(t1);
    tst_mfail_num(0);
  }
}

static void tst_vstr_ca(unsigned int num, Vstr_conf *conf,
                        size_t pos, size_t len, unsigned int flags)
{
  tst_vstr_ci(num, conf, s1, pos, len, flags);
  tst_vstr_ci(num, conf, s2, pos, len, flags);
  tst_vstr_ci(num, conf, s3, pos, len, flags);
}

static void tst_vstr_a(unsigned int num,
                       size_t pos, size_t len, unsigned int flags)
{
  tst_vstr_ca(num, s1->conf, pos, len, flags);
  tst_vstr_ca(num, s2->conf, pos, len, flags);
  tst_vstr_ca(num, s3->conf, pos, len, flags);
}

#define ADD(x, T) do { const char *tmp = NULL; Vstr_ref *ref = NULL; \
  VSTR_ADD_CSTR_BUF(x, 0, buf); \
  VSTR_ADD_CSTR_PTR(x, 0, buf); tmp = vstr_export_cstr_ptr((x), 1, (x)->len); \
  ASSERT((T) == !!tmp); \
  vstr_add_non(x, 0, 4); \
  ref = vstr_ref_make_malloc(4); memset(ref->ptr, 'X', 4); \
  vstr_add_ref(x, 0, ref, 0, 4); \
  vstr_ref_del(ref); \
 } while (FALSE)

int tst(void)
{
  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);

  ADD(s1, TRUE);
  ADD(s2, TRUE);
  ADD(s3, TRUE);
  ADD(s4, FALSE);

  tst_vstr_a( 1, 1, s1->len, VSTR_TYPE_ADD_DEF);
  tst_vstr_a( 2, 1, s1->len, VSTR_TYPE_ADD_BUF_PTR);
  tst_vstr_a( 3, 1, s1->len, VSTR_TYPE_ADD_ALL_REF);
  tst_vstr_a( 4, 1, s1->len, VSTR_TYPE_ADD_ALL_BUF);
  tst_vstr_a( 5, 1, s1->len, VSTR_TYPE_ADD_BUF_REF);

  tst_vstr_a( 6, 4, 16, VSTR_TYPE_ADD_DEF);
  tst_vstr_a( 7, 4, 16, VSTR_TYPE_ADD_BUF_PTR);
  tst_vstr_a( 8, 4, 16, VSTR_TYPE_ADD_ALL_REF);
  tst_vstr_a( 9, 4, 16, VSTR_TYPE_ADD_ALL_BUF);
  tst_vstr_a(10, 4, 16, VSTR_TYPE_ADD_BUF_REF);

  tst_vstr_a(11, 16, 32, VSTR_TYPE_ADD_DEF);
  tst_vstr_a(12, 16, 32, VSTR_TYPE_ADD_BUF_PTR);
  tst_vstr_a(13, 16, 32, VSTR_TYPE_ADD_ALL_REF);
  tst_vstr_a(14, 16, 32, VSTR_TYPE_ADD_ALL_BUF);
  tst_vstr_a(15, 16, 32, VSTR_TYPE_ADD_BUF_REF);

  return (TST_B_RET(ret));
}
