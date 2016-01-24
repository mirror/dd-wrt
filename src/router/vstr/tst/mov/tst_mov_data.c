#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  int mfail_count = 0;

  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);

  /* 1 */
  VSTR_ADD_CSTR_BUF(s2, 0, buf);
  VSTR_ADD_CSTR_BUF(s3, 0, buf);

  vstr_mov(s1, 0, s2, 1, s2->len);
  TST_B_TST(ret, 1, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
  vstr_del(s1, 1, s1->len);

  vstr_mov(s1, 0, s3, 1, s3->len);
  TST_B_TST(ret, 2, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
  vstr_del(s1, 1, s1->len);

  /* 2 */
  VSTR_ADD_CSTR_PTR(s2, 0, buf);
  VSTR_ADD_CSTR_PTR(s3, 0, buf);

  vstr_mov(s1, 0, s2, 1, s2->len);
  TST_B_TST(ret, 3, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
  vstr_del(s1, 1, s1->len);

  vstr_mov(s1, 0, s3, 1, s3->len);
  TST_B_TST(ret, 4, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
  vstr_del(s1, 1, s1->len);

  /* 3 */
  VSTR_ADD_CSTR_BUF(s2, 0, buf);
  VSTR_ADD_CSTR_BUF(s3, 0, buf);

  vstr_mov(s1, 0, s2, 9, s2->len - 8);
  vstr_mov(s1, 0, s2, 1, 8);
  TST_B_TST(ret, 5, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
  vstr_del(s1, 1, s1->len);

  vstr_mov(s1, 0, s3, 9, s3->len - 8);
  vstr_mov(s1, 0, s3, 1, 8);
  TST_B_TST(ret, 6, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
  vstr_del(s1, 1, s1->len);

  /* 4 */
  VSTR_ADD_CSTR_PTR(s2, 0, buf);
  VSTR_ADD_CSTR_PTR(s3, 0, buf);

  vstr_mov(s1, 0, s2, 9, s2->len - 8);
  vstr_mov(s1, 0, s2, 1, 8);
  TST_B_TST(ret, 5, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
  /* no delete */

  TST_B_TST(ret, 7, !vstr_mov(s1, 0, s1, 1, s1->len));
  TST_B_TST(ret, 8, !vstr_mov(s1, 8, s1, 1, s1->len));
  TST_B_TST(ret, 9, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
  vstr_del(s1, 1, s1->len);

  VSTR_ADD_CSTR_PTR(s1, 0, buf);

  TST_B_TST(ret, 10, !vstr_mov(s1, 4, s1, 8, s1->len - 7));
  TST_B_TST(ret, 11, !vstr_mov(s3, s3->len, s3, 5, 3));
  TST_B_TST(ret, 12, !VSTR_CMP_EQ(s1, 1, s1->len, s3, 1, s3->len));

  TST_B_TST(ret, 13, !vstr_mov(s3, 4, s3, s3->len - 2, 3));
  TST_B_TST(ret, 13, !vstr_mov(s1, 4, s1, s1->len - 2, 3));
  TST_B_TST(ret, 15, !VSTR_CMP_EQ(s1, 1, s1->len, s3, 1, s3->len));
  TST_B_TST(ret, 16, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));

  TST_B_TST(ret, 16, !vstr_add_rep_chr(s1, 0, 'x', 16));
  TST_B_TST(ret, 16, !vstr_add_vstr(s1, 0, s1, 1, 16, VSTR_TYPE_ADD_BUF_PTR));
#ifndef VSTR_AUTOCONF_NDEBUG
  /* split poisoning screws this up in debug mode ...
   * should work ok otherwise */
  TST_B_TST(ret, 16, !vstr_sub_cstr_ptr(s1, 1, 16, "xxxxxxxxxxxxxxxx"));
#endif
  do
  {
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_PTR, 1000);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_PTR, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_mov(s3, 0, s1, 2, 8));
  tst_mfail_num(0);

  mfail_count = 0;
  do
  {
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_PTR, 1000);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_PTR, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_mov(s3, 8, s1, 2, 8));
  tst_mfail_num(0);
  
  TST_B_TST(ret, 18, !VSTR_CMP_EQ(s1, 1, s1->len, s3, 1, s3->len));

  TST_B_TST(ret, 19, !vstr_mov(s1, 4, s1, 9, 4));
  TST_B_TST(ret, 20, !vstr_mov(s1, 4, s1, 9, 2));
  TST_B_TST(ret, 21, !vstr_mov(s1, 6, s1, 9, 1));
  TST_B_TST(ret, 22, !vstr_mov(s1, 7, s1, 9, 1));
  TST_B_TST(ret, 23, !VSTR_CMP_EQ(s1, 1, s1->len, s3, 1, s3->len));
  TST_B_TST(ret, 20, !vstr_mov(s1, 16, s1, 17 - 4, 2)); /* magic num */
  TST_B_TST(ret, 21, !vstr_mov(s1, 16, s1, 17 - 4, 1));
  TST_B_TST(ret, 22, !vstr_mov(s1, 16, s1, 17 - 4, 1));
  TST_B_TST(ret, 23, !VSTR_CMP_EQ(s1, 1, s1->len, s3, 1, s3->len));

  mfail_count = 0;
  do
  {
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_PTR, 1000);
    vstr_free_spare_nodes(s4->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s4->conf, VSTR_TYPE_NODE_PTR, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_mov(s4, 0, s1, 1, s1->len));
  tst_mfail_num(0);
  
  TST_B_TST(ret, 24, !VSTR_CMP_EQ(s3, 1, s3->len, s4, 1, s4->len));
  
  {
    unsigned int count = 0;
    
    count = s4->len;
    while (count > 0)
    {
      mfail_count = 0;
      do
      {
        vstr_free_spare_nodes(s4->conf, VSTR_TYPE_NODE_BUF, 1000);
        vstr_free_spare_nodes(s4->conf, VSTR_TYPE_NODE_PTR, 1000);
        tst_mfail_num(++mfail_count);
      } while (!vstr_mov(s4, 0, s4, s4->len, 1));
      tst_mfail_num(0);
      
      --count;
    }
  }
  
  TST_B_TST(ret, 26, !VSTR_CMP_EQ(s3, 1, s3->len, s4, 1, s4->len));
  
  return (TST_B_RET(ret));
}
