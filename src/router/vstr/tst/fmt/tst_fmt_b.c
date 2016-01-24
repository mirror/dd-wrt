#include "tst-main.c"

static const char *rf = __FILE__;

static int ret = 0;

static void tst_b(Vstr_base *t1, unsigned int off)
{
  int mfail_count = 0;
  unsigned int   u = 0;
  unsigned long lu = 0;
  size_t        zu = 0;
  uintmax_t     ju = 0;
  size_t set = 0;
  
  vstr_del(t1, 1, t1->len);
  vstr_add_fmt(t1, 0, "$06{B.u:%u}", 1);

  TST_B_TST(ret, off +  1, !VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "000001"));
  if (ret) PRNT_VSTR(t1);
  
  vstr_del(t1, 1, t1->len);
  vstr_add_fmt(t1, 0, "$+#{B.u:%u}", 1);

  TST_B_TST(ret, off +  2, !VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "+0B1"));
  if (ret) PRNT_VSTR(t1);

  vstr_del(t1, 1, t1->len);
  vstr_add_fmt(t1, 0, "$+#{b.u:%u}", 2);

  TST_B_TST(ret, off +  2, !VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "+0b10"));
  if (ret) PRNT_VSTR(t1);

  vstr_del(t1, 1, t1->len);
  vstr_add_fmt(t1, 0, "$ .{B.u:%u}", 16 + 0 + 4 + 2 + 1);

  TST_B_TST(ret, off +  3, !VSTR_CMP_CSTR_EQ(t1, 1, t1->len, " 10111"));
  if (ret) PRNT_VSTR(t1);

  vstr_del(t1, 1, t1->len);
  vstr_add_fmt(t1, 0, "$.12{B.u:%u}", 128 + 64 + 32 + 16 + 0 + 4 + 2 + 1);

  TST_B_TST(ret, off +  4, !VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "000011110111"));
  if (ret) PRNT_VSTR(t1);

  vstr_del(t1, 1, t1->len);
  vstr_add_fmt(t1, 0, "$#.12{B.lu:%lu}",
               (unsigned long)(128 + 64 + 32 + 16 + 0 + 4 + 2 + 1));

  TST_B_TST(ret, off +  5, !VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "0B000011110111"));
  if (ret) PRNT_VSTR(t1);

  vstr_del(t1, 1, t1->len);
  vstr_add_fmt(t1, 0, "$#012{b.lu:%lu}",
               (unsigned long)(128 + 64 + 32 + 16 + 0 + 4 + 2 + 1));

  TST_B_TST(ret, off +  5, !VSTR_CMP_CSTR_EQ(t1, 1, t1->len, "0b0011110111"));
  if (ret) PRNT_VSTR(t1);

  vstr_del(t1, 1, t1->len);

  u  = 0xF0;
  lu = 0xFF00F0UL;
  zu = 0xAFF00F0UL;
  ju = 0xA5FF00F0UL;

while (set <= 128)
{
  vstr_del(t1, 1, t1->len);
  vstr_add_rep_chr(t1, 0, 'x', set);
  mfail_count = 0;
  do
  {
    ASSERT(vstr_cmp_buf_eq(t1, 1, t1->len, buf, set));
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, t1->len, "$#012{B.u:%u}", u));
  tst_mfail_num(0);

  TST_B_TST(ret, off +  6, !VSTR_CMP_CSTR_EQ(t1, set + 1, t1->len - set, "0B0011110000"));
  if (ret) PRNT_VSTR(t1);
  
  vstr_del(t1, 1, t1->len);
  vstr_add_rep_chr(t1, 0, 'x', set);
  mfail_count = 0;
  do
  {
    ASSERT(vstr_cmp_buf_eq(t1, 1, t1->len, buf, set));
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, t1->len, "$#-12{b.u:%u}", u));
  tst_mfail_num(0);

  TST_B_TST(ret, off +  6, !VSTR_CMP_CSTR_EQ(t1, set + 1, t1->len - set, "0b11110000  "));
  if (ret) PRNT_VSTR(t1);

  vstr_del(t1, 1, t1->len);
  vstr_add_rep_chr(t1, 0, 'x', set);
  mfail_count = 0;
  do
  {
    ASSERT(vstr_cmp_buf_eq(t1, 1, t1->len, buf, set));
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, t1->len, "$-12{B.u:%u}", u));
  tst_mfail_num(0);

  TST_B_TST(ret, off +  6, !VSTR_CMP_CSTR_EQ(t1, set + 1, t1->len - set, "11110000    "));
  if (ret) PRNT_VSTR(t1);

  vstr_del(t1, 1, t1->len);
  vstr_add_rep_chr(t1, 0, 'x', set);
  mfail_count = 0;
  do
  {
    ASSERT(vstr_cmp_buf_eq(t1, 1, t1->len, buf, set));
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, t1->len, "$#12{b.u:%u}", u));
  tst_mfail_num(0);

  TST_B_TST(ret, off +  6, !VSTR_CMP_CSTR_EQ(t1, set + 1, t1->len - set, "  0b11110000"));
  if (ret) PRNT_VSTR(t1);

  /* lu */
  vstr_del(t1, 1, t1->len);
  vstr_add_rep_chr(t1, 0, 'x', set);
  mfail_count = 0;
  do
  {
    ASSERT(vstr_cmp_buf_eq(t1, 1, t1->len, buf, set));
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, t1->len, "$#.28{b.lu:%lu}", lu));
  tst_mfail_num(0);

  TST_B_TST(ret, off +  7, !VSTR_CMP_CSTR_EQ(t1, set + 1, t1->len - set,
                                             "0b0000111111110000000011110000"));
  if (ret) PRNT_VSTR(t1);

  vstr_del(t1, 1, t1->len);  
  vstr_add_rep_chr(t1, 0, 'x', set);
  mfail_count = 0;
  do
  {
    ASSERT(vstr_cmp_buf_eq(t1, 1, t1->len, buf, set));
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, t1->len, "$-38.{B.lu:%lu}||", lu));
  tst_mfail_num(0);

  TST_B_TST(ret, off +  7, !VSTR_CMP_CSTR_EQ(t1, set + 1, t1->len - set,
                                             "111111110000000011110000              ||"));
  if (ret) PRNT_VSTR(t1);
  
  vstr_del(t1, 1, t1->len);
  vstr_add_rep_chr(t1, 0, 'x', set);
  mfail_count = 0;
  do
  {
    ASSERT(vstr_cmp_buf_eq(t1, 1, t1->len, buf, set));
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, t1->len, "$#28{b.lu:%lu}", lu));
  tst_mfail_num(0);

  TST_B_TST(ret, off +  7, !VSTR_CMP_CSTR_EQ(t1, set + 1, t1->len - set,
                                             "  0b111111110000000011110000"));
  if (ret) PRNT_VSTR(t1);

  vstr_del(t1, 1, t1->len);  
  vstr_add_rep_chr(t1, 0, 'x', set);
  mfail_count = 0;
  do
  {
    ASSERT(vstr_cmp_buf_eq(t1, 1, t1->len, buf, set));
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, t1->len, "||$38.{B.lu:%lu}||", lu));
  tst_mfail_num(0);

  TST_B_TST(ret, off +  7, !VSTR_CMP_CSTR_EQ(t1, set + 1, t1->len - set,
                                             "||              111111110000000011110000||"));
  if (ret) PRNT_VSTR(t1);
  
  /* zu */
  vstr_del(t1, 1, t1->len);
  vstr_add_rep_chr(t1, 0, 'x', set);
  mfail_count = 0;
  do
  {
    ASSERT(vstr_cmp_buf_eq(t1, 1, t1->len, buf, set));
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, t1->len, "$-38.{B.zu:%zu}||", zu));
  tst_mfail_num(0);

  TST_B_TST(ret, off +  8, !VSTR_CMP_CSTR_EQ(t1, set + 1, t1->len - set,
                                             "1010111111110000000011110000          ||"));
  if (ret) PRNT_VSTR(t1);
  
  vstr_del(t1, 1, t1->len);
  vstr_add_rep_chr(t1, 0, 'x', set);
  mfail_count = 0;
  do
  {
    ASSERT(vstr_cmp_buf_eq(t1, 1, t1->len, buf, set));
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, t1->len, "$-38.{b.zu:%zu}||", zu));
  tst_mfail_num(0);

  TST_B_TST(ret, off +  8, !VSTR_CMP_CSTR_EQ(t1, set + 1, t1->len - set,
                                             "1010111111110000000011110000          ||"));
  if (ret) PRNT_VSTR(t1);
  
  vstr_del(t1, 1, t1->len);
  vstr_add_rep_chr(t1, 0, 'x', set);
  mfail_count = 0;
  do
  {
    ASSERT(vstr_cmp_buf_eq(t1, 1, t1->len, buf, set));
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, t1->len, "$38.{B.zu:%zu}", zu));
  tst_mfail_num(0);

  TST_B_TST(ret, off +  8, !VSTR_CMP_CSTR_EQ(t1, set + 1, t1->len - set,
                                             "          1010111111110000000011110000"));
  if (ret) PRNT_VSTR(t1);
  
  vstr_del(t1, 1, t1->len);
  vstr_add_rep_chr(t1, 0, 'x', set);
  mfail_count = 0;
  do
  {
    ASSERT(vstr_cmp_buf_eq(t1, 1, t1->len, buf, set));
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, t1->len, "||$38.{b.zu:%zu}||", zu));
  tst_mfail_num(0);

  TST_B_TST(ret, off +  8, !VSTR_CMP_CSTR_EQ(t1, set + 1, t1->len - set,
                                             "||          1010111111110000000011110000||"));
  if (ret) PRNT_VSTR(t1);
  
  /* ju */
  vstr_del(t1, 1, t1->len);
  vstr_add_rep_chr(t1, 0, 'x', set);
  mfail_count = 0;
  do
  {
    ASSERT(vstr_cmp_buf_eq(t1, 1, t1->len, buf, set));
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, t1->len, "$-38.{B.ju:%ju}||", ju));
  tst_mfail_num(0);

  TST_B_TST(ret, off +  9, !VSTR_CMP_CSTR_EQ(t1, set + 1, t1->len - set,
                                             "10100101111111110000000011110000      ||"));
  if (ret) PRNT_VSTR(t1);
  
  vstr_del(t1, 1, t1->len);
  vstr_add_rep_chr(t1, 0, 'x', set);
  mfail_count = 0;
  do
  {
    ASSERT(vstr_cmp_buf_eq(t1, 1, t1->len, buf, set));
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, t1->len, "$-38.{b.ju:%ju}||", ju));
  tst_mfail_num(0);

  TST_B_TST(ret, off +  9, !VSTR_CMP_CSTR_EQ(t1, set + 1, t1->len - set,
                                             "10100101111111110000000011110000      ||"));
  if (ret) PRNT_VSTR(t1);
  
  vstr_del(t1, 1, t1->len);
  vstr_add_rep_chr(t1, 0, 'x', set);
  mfail_count = 0;
  do
  {
    ASSERT(vstr_cmp_buf_eq(t1, 1, t1->len, buf, set));
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, t1->len, "$38.{B.ju:%ju}", ju));
  tst_mfail_num(0);

  TST_B_TST(ret, off +  9, !VSTR_CMP_CSTR_EQ(t1, set + 1, t1->len - set,
                                             "      10100101111111110000000011110000"));
  if (ret) PRNT_VSTR(t1);
  
  vstr_del(t1, 1, t1->len);
  vstr_add_rep_chr(t1, 0, 'x', set);
  mfail_count = 0;
  do
  {
    ASSERT(vstr_cmp_buf_eq(t1, 1, t1->len, buf, set));
    vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(t1, t1->len, "$38.{b.ju:%ju}", ju));
  tst_mfail_num(0);

  TST_B_TST(ret, off +  9, !VSTR_CMP_CSTR_EQ(t1, set + 1, t1->len - set,
                                             "      10100101111111110000000011110000"));
  if (ret) PRNT_VSTR(t1);

  ++set;
}
}

int tst(void)
{
  memset(buf, 'x', sizeof(buf));
  
  vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');
  vstr_cntl_conf(s3->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');
  vstr_cntl_conf(s4->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');

  vstr_sc_fmt_add_all(NULL);
  vstr_sc_fmt_add_all(s3->conf);
  vstr_sc_fmt_add_all(s4->conf);

  /* output */
  tst_b(s1, 0);
  tst_b(s3, 10);
  tst_b(s4, 20);

  return (TST_B_RET(ret));
}
/* Crap for tst_coverage constants....
 *
 * VSTR_FLAG_SC_FMT_CB_BEG_OBJ_BINNUM_H
 * VSTR_FLAG_SC_FMT_CB_BEG_OBJ_BINNUM_L
 */
