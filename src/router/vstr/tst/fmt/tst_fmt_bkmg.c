#include "tst-main.c"

static const char *rf = __FILE__;

static int ret = 0;

static void tst_bkmg(Vstr_base *t1, unsigned int off)
{
  int mfail_count = 0;
  
  vstr_del(t1, 1, t1->len);
  vstr_add_fmt(t1, 0, "$06{BKMG.u:%u}", 10 + 2);

  TST_B_TST(ret, off +  1, !VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "00012B"));

  vstr_del(t1, 1, t1->len);
  vstr_add_fmt(t1, 0, "$+{BKMG.u:%u}", 10 * 1000 + 4321);

  TST_B_TST(ret, off +  2, !VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "+14.32KB"));
  
  vstr_del(t1, 1, t1->len);
  vstr_add_fmt(t1, 0, "$ .{BKMG.u:%u}", 10 * 1000 + 4321);

  TST_B_TST(ret, off +  3, !VSTR_CMP_CSTR_EQ(t1, 1, t1->len, " 14KB"));

  vstr_del(t1, 1, t1->len);
  vstr_add_fmt(t1, 0, "$.4{BKMG.u:%u}", 10 * 1000 + 321);

  TST_B_TST(ret, off +  4, !VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "10.321KB"));

  vstr_del(t1, 1, t1->len);
  vstr_add_fmt(t1, 0, "$.4{bKMG.u:%u}", 10 * 1000 * 1000 + 7654321);

  TST_B_TST(ret, off +  5, !VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "17.6543Mb"));

  vstr_del(t1, 1, t1->len);
  
  mfail_count = 0;
  do
  {
    ASSERT(!t1->len);
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, 0, "$08{BKMG.u:%u}",
                         1 * 1000 * 1000 * 1000 + 7654321));
  tst_mfail_num(0);

  TST_B_TST(ret, off +  6, !VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "001.00GB"));

  vstr_del(t1, 1, t1->len);
  vstr_add_fmt(t1, 0, "||$8{bKMG.u:%u}", 1 * 1000 * 1000 * 1000 + 7654321);

  TST_B_TST(ret, off +  7, !VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "||  1.00Gb"));

  vstr_del(t1, 1, t1->len);

  mfail_count = 0;
  do
  {
    ASSERT(!t1->len);
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, 0, "$-19.{BKMG.u:%u}||",
                         2 * 100 * 1000 * 1000));
  tst_mfail_num(0);

  TST_B_TST(ret, off +  8, !VSTR_CMP_CSTR_EQ(t1, 1, t1->len,
                                             "200MB              ||"));

  vstr_del(t1, 1, t1->len);
  
  mfail_count = 0;
  do
  {
    ASSERT(!t1->len);
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, 0, "$.3{BKMG/s.u:%u}ec",
                         (4U * 1000U * 1000U * 1000U) +
                         187654321U +
                         0U));
  tst_mfail_num(0);

  TST_B_TST(ret, off +  9, !VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "4.187GB/sec"));

  vstr_del(t1, 1, t1->len);
  vstr_add_fmt(t1, 0, "$.6{bKMG/s.u:%u}",
               (1U * 1000U * 1000U * 1000U) +
               (234U * 1000U * 1000U) +
               0U);

  TST_B_TST(ret, off +  10, !VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "1.234000Gb/s"));
}

static void tst_bkmg_max(Vstr_base *t1)
{
  /* assume intmax_t and long long come as both or none */
#ifdef VSTR_AUTOCONF_HAVE_LONG_LONG
  int mfail_count = 0;
  uintmax_t num = 1;

  vstr_del(t1, 1, t1->len);
  vstr_add_fmt(t1, 0, "${BKMG.ju:%ju}", num);
  assert(VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "1B"));

  num *= 1000;
  vstr_del(t1, 1, t1->len);
  vstr_add_fmt(t1, 0, "$.{BKMG.ju:%ju}", num);
  assert(VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "1KB"));
    
  num *= 1000;
  vstr_del(t1, 1, t1->len);
  vstr_add_fmt(t1, 0, "$.{BKMG.ju:%ju}", num);
  assert(VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "1MB"));
    
  num *= 1000;
  vstr_del(t1, 1, t1->len);
  vstr_add_fmt(t1, 0, "$.{BKMG.ju:%ju}", num);
  assert(VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "1GB"));
    
  num *= 1000;
  vstr_del(t1, 1, t1->len);
  vstr_add_fmt(t1, 0, "$.{bKMG.ju:%ju}", num);
  assert(VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "1Tb"));
    
  num *= 4500;

  vstr_del(t1, 1, t1->len);
  mfail_count = 0;
  do
  {
    ASSERT(!t1->len);
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, 0, "${BKMG.ju:%ju}", num));
  tst_mfail_num(0);
  
  assert(VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "4.50PB"));

  num *= 1500;

  vstr_del(t1, 1, t1->len);
  mfail_count = 0;
  do
  {
    ASSERT(!t1->len);
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, 0, "${BKMG.ju:%ju}", num));
  tst_mfail_num(0);
  
  assert(VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "6.75EB"));
  
  vstr_del(t1, 1, t1->len);
  mfail_count = 0;
  do
  {
    ASSERT(!t1->len);
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, 0,
                         "$.{BKMG/s.ju:%ju}",
                         (uintmax_t)(25000000000000ULL)));
  tst_mfail_num(0);
  
  assert(VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "25TB/s"));
  
  vstr_del(t1, 1, t1->len);
  mfail_count = 0;
  do
  {
    ASSERT(!t1->len);
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, 0, "$016.4{bKMG/s.ju:%ju}",
                         (uintmax_t)(12345000000000000ULL)));
  tst_mfail_num(0);

  assert(VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "0000012.3450Pb/s"));
#endif  
}

int tst(void)
{
  vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');
  vstr_cntl_conf(s3->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');
  vstr_cntl_conf(s4->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');

  vstr_sc_fmt_add_all(NULL);
  vstr_sc_fmt_add_all(s3->conf);
  vstr_sc_fmt_add_all(s4->conf);

  /* output */
  tst_bkmg(s1, 0);
  tst_bkmg(s3, 10);
  tst_bkmg(s4, 20);

  tst_bkmg_max(s1);
  tst_bkmg_max(s3);
  tst_bkmg_max(s4);  
  
  return (TST_B_RET(ret));
}
