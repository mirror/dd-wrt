#include "tst-main.c"

static const char *rf = __FILE__;

static int ret = 0;

static void tst_vstr_i(unsigned int num, Vstr_base *t1,
                       Vstr_base *t_from, size_t pos, size_t len,
                       unsigned int flags)
{
  vstr_del(t_from, 1, t_from->len); /* setup each time for _BUF converters */
  VSTR_ADD_CSTR_BUF(t_from, t_from->len, buf);
  VSTR_ADD_CSTR_PTR(t_from, t_from->len, buf);
  VSTR_ADD_CSTR_PTR(t_from, t_from->len, " xy");
  vstr_del(t_from, 1, strlen(" xy"));

  vstr_export_iovec_ptr_all(t_from, NULL, NULL);

  vstr_del(t1, 1, t1->len);

  vstr_add_vstr(t1, t1->len, t_from,   1,   0, flags);
  vstr_add_vstr(t1, t1->len, t_from, pos, len, flags);
  vstr_add_vstr(t1, t1->len, t_from,   2,   0, flags);

  TST_B_TST(ret, num, !VSTR_CMP_EQ(t1, 1, t1->len, t_from, pos, len));
}

static void tst_vstr_a(unsigned int num,
                       size_t pos, size_t len, unsigned int flags)
{
  tst_vstr_i(num, s2, s1, pos, len, flags);
  tst_vstr_i(num, s3, s1, pos, len, flags);
}

static void tst_vstr_b(unsigned int num, Vstr_base *t1, unsigned int flags)
{
  size_t len = 0;

  vstr_del(t1, 1, t1->len);
  VSTR_ADD_CSTR_BUF(t1, t1->len, buf);
  VSTR_ADD_CSTR_PTR(t1, t1->len, buf);
  
  len = t1->len / 2;

  vstr_add_vstr(t1, t1->len, t1, 1, t1->len, flags);
  vstr_del(t1, 8, len * 2);
  vstr_add_vstr(t1, len, t1, 1, len, flags);
  vstr_del(t1, 8, len);
  vstr_add_vstr(t1, 7, t1, len + 1, len, flags);
  vstr_del(t1, 8, len);
  vstr_add_vstr(t1, 7, t1, 2, len, flags);
  vstr_del(t1, 2, len * 2);

  TST_B_TST(ret, num, !VSTR_CMP_CSTR_EQ(t1, 1, t1->len, buf));
}

static void tst_vstr_m(unsigned int num, Vstr_base *t1, unsigned int flags)
{
  Vstr_base *t_from = s1;
  size_t pos = 1;
  size_t len = 0;
  int mfail_count = 0;

  tst_mfail_num(0);

  vstr_del(t_from, 1, t_from->len); /* setup each time for _BUF converters */
  VSTR_ADD_CSTR_BUF(t_from, t_from->len, buf);
  VSTR_ADD_CSTR_PTR(t_from, t_from->len, buf);
  VSTR_ADD_CSTR_BUF(t_from, t_from->len, buf);
  VSTR_ADD_CSTR_PTR(t_from, t_from->len, " xy");
  vstr_del(t_from, 1, strlen(" xy"));

  len = t_from->len;

  vstr_export_iovec_ptr_all(t_from, NULL, NULL);
  TST_B_TST(ret, 29, !vstr_cmp_cstr_eq(t_from, (strlen(buf) * 2) - 3 + 1,
                                       strlen(buf), buf));
  
  vstr_del(t1, 1, t1->len);
  mfail_count = 3;
  do
  {
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_NON, 1000);
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_PTR, 1000);
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_REF, 1000);
    
    tst_mfail_num(++mfail_count / 4);
  } while (!vstr_add_vstr(t1, t1->len, t_from,   pos,   len, flags));
  tst_mfail_num(0);

  TST_B_TST(ret, num, !VSTR_CMP_EQ(t1, 1, t1->len, t_from, pos, len));
}

static void tst_vstr_hack_grp(Vstr_base *t1)
{ /* FIXME: Massive hack */
  Vstr_ref_grp_ptr *grp = t1->conf->ref_grp_buf2ref;

  if (!grp)
    return;
      
  if (grp->make_num)
    grp->flags &= ~(1U<<6);
  else
  {
    Vstr_ref *tmp = vstr_ref_make_malloc(4);
    tmp->func((Vstr_ref *)grp);
    vstr_ref_del(tmp);
  }
  
  t1->conf->ref_grp_buf2ref = NULL;
}

int tst(void)
{
  size_t len = 0;
  int mfail_count = 0;
  
  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);
  len = strlen(buf);

  tst_vstr_a( 1, 1, s1->len, VSTR_TYPE_ADD_DEF);
  tst_vstr_a( 2, 1, s1->len, VSTR_TYPE_ADD_BUF_PTR);
  tst_vstr_a( 3, 1, s1->len, VSTR_TYPE_ADD_ALL_REF);
  tst_vstr_a( 4, 1, s1->len, VSTR_TYPE_ADD_ALL_BUF);
  tst_vstr_a( 5, 1, s1->len, VSTR_TYPE_ADD_BUF_REF);

  tst_vstr_a( 6, len / 2, 4, VSTR_TYPE_ADD_DEF);
  tst_vstr_a( 7, len / 2, 4, VSTR_TYPE_ADD_BUF_PTR);
  tst_vstr_a( 8, len / 2, 4, VSTR_TYPE_ADD_ALL_REF);
  tst_vstr_a( 9, len / 2, 4, VSTR_TYPE_ADD_ALL_BUF);
  tst_vstr_a(10, len / 2, 4, VSTR_TYPE_ADD_BUF_REF);

  tst_vstr_a(11, len / 2, len, VSTR_TYPE_ADD_DEF);
  tst_vstr_a(12, len / 2, len, VSTR_TYPE_ADD_BUF_PTR);
  tst_vstr_a(13, len / 2, len, VSTR_TYPE_ADD_ALL_REF);
  tst_vstr_a(14, len / 2, len, VSTR_TYPE_ADD_ALL_BUF);
  tst_vstr_a(15, len / 2, len, VSTR_TYPE_ADD_BUF_REF);

  tst_vstr_a(16, len + len / 2, 4, VSTR_TYPE_ADD_DEF);
  tst_vstr_a(17, len + len / 2, 4, VSTR_TYPE_ADD_BUF_PTR);
  tst_vstr_a(18, len + len / 2, 4, VSTR_TYPE_ADD_ALL_REF);
  tst_vstr_a(19, len + len / 2, 4, VSTR_TYPE_ADD_ALL_BUF);
  tst_vstr_a(20, len + len / 2, 4, VSTR_TYPE_ADD_BUF_REF);

  tst_vstr_b(21, s1, VSTR_TYPE_ADD_DEF);
  tst_vstr_b(21, s1, VSTR_TYPE_ADD_BUF_PTR);
  tst_vstr_b(21, s1, VSTR_TYPE_ADD_ALL_REF);
  tst_vstr_b(21, s1, VSTR_TYPE_ADD_ALL_BUF);
  tst_vstr_b(21, s1, VSTR_TYPE_ADD_BUF_REF);

  tst_vstr_b(22, s2, VSTR_TYPE_ADD_DEF);
  tst_vstr_b(22, s2, VSTR_TYPE_ADD_BUF_PTR);
  tst_vstr_b(22, s2, VSTR_TYPE_ADD_ALL_REF);
  tst_vstr_b(22, s2, VSTR_TYPE_ADD_ALL_BUF);
  tst_vstr_b(22, s2, VSTR_TYPE_ADD_BUF_REF);

  tst_vstr_b(23, s3, VSTR_TYPE_ADD_DEF);
  tst_vstr_b(23, s3, VSTR_TYPE_ADD_BUF_PTR);
  tst_vstr_b(23, s3, VSTR_TYPE_ADD_ALL_REF);
  tst_vstr_b(23, s3, VSTR_TYPE_ADD_ALL_BUF);
  tst_vstr_b(23, s3, VSTR_TYPE_ADD_BUF_REF);

  tst_vstr_b(24, s4, VSTR_TYPE_ADD_DEF);
  tst_vstr_b(24, s4, VSTR_TYPE_ADD_BUF_PTR);
  tst_vstr_b(24, s4, VSTR_TYPE_ADD_ALL_REF);
  tst_vstr_b(24, s4, VSTR_TYPE_ADD_ALL_BUF);
  tst_vstr_b(24, s4, VSTR_TYPE_ADD_BUF_REF);

  tst_vstr_m(25, s2, VSTR_TYPE_ADD_DEF);
  tst_vstr_m(25, s2, VSTR_TYPE_ADD_BUF_PTR);
  tst_vstr_m(25, s2, VSTR_TYPE_ADD_ALL_REF);
  tst_vstr_m(25, s2, VSTR_TYPE_ADD_ALL_BUF);
  tst_vstr_m(25, s2, VSTR_TYPE_ADD_BUF_REF);

  tst_vstr_m(26, s3, VSTR_TYPE_ADD_DEF);
  tst_vstr_m(26, s3, VSTR_TYPE_ADD_BUF_PTR);
  tst_vstr_m(26, s3, VSTR_TYPE_ADD_ALL_REF);
  tst_vstr_m(26, s3, VSTR_TYPE_ADD_ALL_BUF);
  tst_vstr_m(26, s3, VSTR_TYPE_ADD_BUF_REF);

  tst_vstr_m(27, s4, VSTR_TYPE_ADD_DEF);
  tst_vstr_m(27, s4, VSTR_TYPE_ADD_BUF_PTR);
  tst_vstr_m(27, s4, VSTR_TYPE_ADD_ALL_REF);
  tst_vstr_m(27, s4, VSTR_TYPE_ADD_ALL_BUF);
  tst_vstr_m(27, s4, VSTR_TYPE_ADD_BUF_REF);

  tst_vstr_hack_grp(s1);
  tst_vstr_m(30, s2, VSTR_TYPE_ADD_BUF_REF);
  tst_vstr_hack_grp(s1);
  tst_vstr_m(30, s3, VSTR_TYPE_ADD_BUF_REF);
  tst_vstr_hack_grp(s1);
  tst_vstr_m(30, s4, VSTR_TYPE_ADD_BUF_REF);

  {
    const char *p1 = vstr_export_cstr_ptr(s1, 1, s1->len);
    const char *p2 = vstr_export_cstr_ptr(s2, 1, s2->len);
    const char *p1_save = p1;

    ASSERT(p1 != p2);

    vstr_del(s2, 1, s2->len);
    vstr_add_vstr(s2, 0, s1, 1, s1->len, VSTR_TYPE_ADD_DEF);

    p1 = vstr_export_cstr_ptr(s1, 1, s1->len);
    p2 = vstr_export_cstr_ptr(s2, 1, s2->len);

    TST_B_TST(ret, 31, p1 != p1_save);
    TST_B_TST(ret, 31, p1 != p2);

    vstr_del(s3, 1, s3->len);
    vstr_add_rep_chr(s3, 0, 'x', 8);

    p1 = vstr_export_cstr_ptr(s3, 1, s3->len);
    ASSERT(!memcmp(p1, "xxxxxxxx", 8));
    
    vstr_add_vstr(s2,       0, s3, 1, s3->len, VSTR_TYPE_ADD_DEF);
    vstr_add_vstr(s2, s2->len, s3, 1, s3->len, VSTR_TYPE_ADD_DEF);
    
    p2 = vstr_export_cstr_ptr(s2, 9, s2->len - 16);
    
    TST_B_TST(ret, 31, p1_save != p2);

    p2 = vstr_export_cstr_ptr(s2, 1, 8);
    
    TST_B_TST(ret, 31, p1 == p2);
    TST_B_TST(ret, 31, p1_save == p2);
    ASSERT(!memcmp(p2, "xxxxxxxx", 8));

    vstr_del(s2, 1, s2->len);
    vstr_add_vstr(s2, 0, s1, 1, s1->len, VSTR_TYPE_ADD_DEF);
    vstr_add_vstr(s2,       0, s3, 1, s3->len, VSTR_TYPE_ADD_DEF);
    vstr_add_vstr(s2, s2->len, s3, 1, s3->len, VSTR_TYPE_ADD_DEF);
    
    p2 = vstr_export_cstr_ptr(s2, 9, s2->len - 16);
    
    TST_B_TST(ret, 31, p1_save != p2);
    
    vstr_del(s3, 1, s3->len);
    vstr_add_vstr(s3, 0, s2, 1, s2->len, VSTR_TYPE_ADD_DEF);
    
    p1 = vstr_export_cstr_ptr(s3, 9, s3->len - 16);

    TST_B_TST(ret, 31, p1 != p2);
    TST_B_TST(ret, 31, p1 != p1_save);
  }

  vstr_del(s3, 1, s3->len);

  vstr_add_cstr_ptr(s3, s3->len, "a1a2a3a4" "b1b2b3b4" "c1c2c3c4" "d1d2d3d4");

  ASSERT(vstr_num(s3, 1, s3->len) == 1);
  mfail_count = 3; /* div 4 means this will be 1 on incr. */
  do
  {
    ASSERT(vstr_cmp_cstr_eq(s3, 1, s3->len,
                            "a1a2a3a4" "b1b2b3b4" "c1c2c3c4" "d1d2d3d4"));

    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_PTR, 1000);
    tst_mfail_num(++mfail_count / 4);
  } while (!vstr_add_vstr(s3, 2, s3, 9, 8, VSTR_TYPE_ADD_DEF));
  tst_mfail_num(0);

  ASSERT(vstr_cmp_cstr_eq(s3, 1, s3->len,
                          "a1"
                          "b1b2b3b4"
                          "a2a3a4" "b1b2b3b4" "c1c2c3c4" "d1d2d3d4"));
  ASSERT(vstr_num(s3, 1, s3->len) == 3);
  
  mfail_count = 3; /* div 4 means this will be 1 on incr. */
  do
  {
    ASSERT(vstr_cmp_cstr_eq(s3, 1, s3->len,
                            "a1"
                            "b1b2b3b4"
                            "a2a3a4" "b1b2b3b4" "c1c2c3c4" "d1d2d3d4"));

    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_PTR, 1000);
    tst_mfail_num(++mfail_count / 4);
  } while (!vstr_add_vstr(s3, 38, s3, 25, 8, VSTR_TYPE_ADD_DEF));
  tst_mfail_num(0);
  
  ASSERT(vstr_cmp_cstr_eq(s3, 1, s3->len,
                          "a1"
                          "b1b2b3b4"
                          "a2a3a4" "b1b2b3b4" "c1c2c3c4" "d1d2d3"
                          "c1c2c3c4"
                          "d4"));
  ASSERT(vstr_num(s3, 1, s3->len) == 5);

  vstr_del(s1, 1, s1->len);
  vstr_add_vstr(s1, 0, s3, 1, s3->len, VSTR_TYPE_ADD_DEF);
  
  ASSERT(vstr_cmp_cstr_eq(s1, 1, s1->len,
                          "a1"
                          "b1b2b3b4"
                          "a2a3a4" "b1b2b3b4" "c1c2c3c4" "d1d2d3"
                          "c1c2c3c4"
                          "d4"));
  ASSERT(vstr_num(s1, 1, s1->len) == 5);

  vstr_del(s1, 1, s1->len);
  mfail_count = 3; /* div 4 means this will be 1 on incr. */
  do
  {
    ASSERT(!s1->len);
    ASSERT(vstr_num(s1, 1, s1->len) == 0);
    ASSERT(vstr_num(s3, 1, s3->len) == 5);
    ASSERT(vstr_cmp_cstr_eq(s3, 1, s3->len,
                            "a1"
                            "b1b2b3b4"
                            "a2a3a4" "b1b2b3b4" "c1c2c3c4" "d1d2d3"
                            "c1c2c3c4"
                            "d4"));
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_REF, 1000);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_PTR, 1000);
    tst_mfail_num(++mfail_count / 4);
  } while (!vstr_add_vstr(s1, 0, s3, 1, s3->len, VSTR_TYPE_ADD_ALL_REF));
  tst_mfail_num(0);
  
  ASSERT(vstr_cmp_cstr_eq(s1, 1, s1->len,
                          "a1"
                          "b1b2b3b4"
                          "a2a3a4" "b1b2b3b4" "c1c2c3c4" "d1d2d3"
                          "c1c2c3c4"
                          "d4"));
  ASSERT(vstr_num(s1, 1, s1->len) == 1);

  vstr_del(s2, 1, s2->len);
  vstr_add_non(s2, 0, 15);
  vstr_add_cstr_ptr(s2, 0, "xx");
  vstr_add_non(s2, 0, 15);
  ASSERT(vstr_num(s2, 1, s2->len) == 3);
  
  vstr_del(s1, 1, s1->len);
  mfail_count = 3; /* div 4 means this will be 1 on incr. */
  do
  {
    ASSERT(!s1->len);
    ASSERT(vstr_num(s1, 1, s1->len) == 0);
    ASSERT(vstr_num(s2, 1, s2->len) == 3);
    ASSERT(s2->len == 32);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_NON, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_PTR, 1000);
    tst_mfail_num(++mfail_count / 4);
  } while (!vstr_add_vstr(s1, 0, s2, 1, s2->len, VSTR_TYPE_ADD_DEF));
  tst_mfail_num(0);
  
  ASSERT(s1->len == 32);
  ASSERT(vstr_cmp_eq(s1, 1, s1->len, s2, 1, s2->len));
  ASSERT(vstr_num(s1, 1, s1->len) == 3);

  vstr_del(s1, 1, s1->len);
  mfail_count = 3; /* div 4 means this will be 1 on incr. */
  do
  {
    ASSERT(!s1->len);
    ASSERT(vstr_num(s1, 1, s1->len) == 0);
    ASSERT(vstr_num(s2, 1, s2->len) == 3);
    ASSERT(s2->len == 32);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_NON, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_PTR, 1000);
    tst_mfail_num(++mfail_count / 4);
  } while (!vstr_add_vstr(s1, 0, s2, 6, s2->len - 10, VSTR_TYPE_ADD_DEF));
  tst_mfail_num(0);
  
  ASSERT(vstr_num(s1, 1, s1->len) == 3);
  ASSERT(vstr_num(s2, 1, s2->len) == 3);
  ASSERT(s1->len == 22);
  ASSERT(s2->len == 32);
  ASSERT(vstr_cmp_eq(s1, 1, s1->len, s2, 6, s2->len - 10));
  
  vstr_del(s1, 1, s1->len);
  vstr_add_cstr_buf(s1, 0, "a1a2a3a4" "b1b2b3b4" "c1c2c3c4" "d1d2d3d4");

  ASSERT(vstr_num(s1, 1, s1->len) == 1);
  mfail_count = 3; /* div 4 means this will be 1 on incr. */
  do
  {
    ASSERT(vstr_cmp_cstr_eq(s1, 1, s1->len,
                            "a1a2a3a4" "b1b2b3b4" "c1c2c3c4" "d1d2d3d4"));

    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count / 4);
  } while (!vstr_add_vstr(s1, 2, s1, 9, 8, VSTR_TYPE_ADD_DEF));
  tst_mfail_num(0);

  ASSERT(vstr_cmp_cstr_eq(s1, 1, s1->len,
                          "a1" "b1b2b3b4"
                          "a2a3a4"
                          "b1b2b3b4" "c1c2c3c4" "d1d2d3d4"));
  ASSERT(vstr_num(s1, 1, s1->len) == 3);
  
  mfail_count = 3; /* div 4 means this will be 1 on incr. */
  do
  {
    ASSERT(s1->len == 40);
    ASSERT(vstr_cmp_cstr_eq(s1, 1, s1->len,
                            "a1" "b1b2b3b4"
                            "a2a3a4"
                            "b1b2b3b4" "c1c2c3c4" "d1d2d3d4"));

    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count / 4);
  } while (!vstr_add_vstr(s1, 38, s1, 25, 8, VSTR_TYPE_ADD_DEF));
  tst_mfail_num(0);
  
  ASSERT(vstr_cmp_cstr_eq(s1, 1, s1->len,
                          "a1" "b1b2b3b4"
                          "a2a3a4"
                          "b1b2b3b4"
                          "c1c2c3c4" "d1d2d3" "c1c2c3c4"
                          "d4"));
  ASSERT(vstr_num(s1, 1, s1->len) == 5);

  /* tests ->used */
  vstr_del(s1, 1, 1);
  vstr_add_vstr(s1, 9, s1, 1, 1, VSTR_TYPE_ADD_DEF);
  ASSERT(vstr_cmp_cstr_eq(s1, 1, s1->len,
                          "1" "b1b2b3b41"
                          "a2a3a4"
                          "b1b2b3b4"
                          "c1c2c3c4" "d1d2d3" "c1c2c3c4"
                          "d4"));
  ASSERT(vstr_num(s1, 1, s1->len) == 5);
  { /* should merge back */
    size_t off = 0;
    
    vstr_add_vstr(s1, off, s1, off + 11,  1, VSTR_TYPE_ADD_DEF); off +=  1;
    vstr_add_vstr(s1, off, s1, off + 10, 29, VSTR_TYPE_ADD_DEF); off += 29;
    vstr_add_vstr(s1, off, s1, off + 47,  2, VSTR_TYPE_ADD_DEF); off +=  2;
    
    vstr_del(s1, off + 1, vstr_sc_posdiff(off + 1, s1->len));
  }
  
  ASSERT(vstr_cmp_cstr_eq(s1, 1, s1->len,
                          "a1a2a3a4" "b1b2b3b4" "c1c2c3c4" "d1d2d3d4"));
  ASSERT(vstr_num(s1, 1, s1->len) == 1);

  /* test into middle */
  mfail_count = 3; /* div 4 means this will be 1 on incr. */
  do
  {
    ASSERT(vstr_cmp_cstr_eq(s1, 1, s1->len,
                            "a1a2a3a4" "b1b2b3b4" "c1c2c3c4" "d1d2d3d4"));

    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count / 4);
  } while (!vstr_add_vstr(s1, 12, s1, 1, s1->len, VSTR_TYPE_ADD_DEF));
  tst_mfail_num(0);
  
  ASSERT(vstr_cmp_cstr_eq(s1, 1, s1->len,
                          "a1a2a3a4" "b1b2" "a1a2a3a4" "b1b2b3b4" "c1c2c3c4" "d1d2d3d4"
                          "b3b4" "c1c2c3c4" "d1d2d3d4"));
  ASSERT(vstr_num(s1, 1, s1->len) == 2);

  return (TST_B_RET(ret));
}
