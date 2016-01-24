#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  unsigned int a_pos = 0, b_pos = 0, c_pos = 0, d_pos = 0;
  int ret = 0;
  VSTR_SECTS_DECL(sects8, 8);
  VSTR_SECTS_DECL(sects4, 4);
  VSTR_SECTS_DECL(sects2, 2);
  unsigned int split2_num = 0;
  unsigned int split4_num = 0;
  unsigned int split8_num = 0;
  int mfail_count = 0;
  Vstr_sects *msects = NULL;
  
  VSTR_SECTS_DECL_INIT(sects8);
  VSTR_SECTS_DECL_INIT(sects4);
  VSTR_SECTS_DECL_INIT(sects2);

  VSTR_ADD_CSTR_BUF(s1, 0, ":::a::b:!::c::d:");
  a_pos = 3;
  b_pos = 7;
  c_pos = 12;
  d_pos = 15;

  split2_num = VSTR_SPLIT_CSTR_BUF(s1, 1, s1->len, "::",
                                   sects2, sects2->sz, VSTR_FLAG_SPLIT_DEF);
  split4_num = VSTR_SPLIT_CSTR_BUF(s1, 1, s1->len, "::",
                                   sects4, sects4->sz, VSTR_FLAG_SPLIT_DEF);
  split8_num = vstr_split_cstr_buf(s1, 1, s1->len, "::",
                                   sects8, sects8->sz, VSTR_FLAG_SPLIT_DEF);

  VSTR_SECTS_DECL_INIT(sects8); /* can do many times without harm */
  VSTR_SECTS_DECL_INIT(sects4);
  VSTR_SECTS_DECL_INIT(sects2);

  /* limited to size - done for speed */
  TST_B_TST(ret, 1, ((sects8->num != 4) ||
                     (split8_num  != 4) ||
                     (VSTR_SECTS_NUM(sects8, 1)->pos != a_pos) ||
                     (VSTR_SECTS_NUM(sects8, 1)->len != 2) ||
                     (VSTR_SECTS_NUM(sects8, 2)->pos != b_pos) ||
                     (VSTR_SECTS_NUM(sects8, 2)->len != 3) ||
                     (VSTR_SECTS_NUM(sects8, 3)->pos != c_pos) ||
                     (VSTR_SECTS_NUM(sects8, 3)->len != 1) ||
                     (VSTR_SECTS_NUM(sects8, 4)->pos != d_pos) ||
                     (VSTR_SECTS_NUM(sects8, 4)->len != 2)));

  TST_B_TST(ret, 2, ((sects4->num != 4) ||
                     (split4_num  != 4) ||
                     (VSTR_SECTS_NUM(sects4, 1)->pos != a_pos) ||
                     (VSTR_SECTS_NUM(sects4, 1)->len != 2) ||
                     (VSTR_SECTS_NUM(sects4, 2)->pos != b_pos) ||
                     (VSTR_SECTS_NUM(sects4, 2)->len != 3) ||
                     (VSTR_SECTS_NUM(sects4, 3)->pos != c_pos) ||
                     (VSTR_SECTS_NUM(sects4, 3)->len != 1) ||
                     (VSTR_SECTS_NUM(sects4, 4)->pos != d_pos) ||
                     (VSTR_SECTS_NUM(sects4, 4)->len != 2)));

  TST_B_TST(ret, 3, ((sects2->num != 2) ||
                     (split2_num  != 2) ||
                     (VSTR_SECTS_NUM(sects2, 1)->pos != a_pos) ||
                     (VSTR_SECTS_NUM(sects2, 1)->len != 2) ||
                     (VSTR_SECTS_NUM(sects2, 2)->pos != b_pos) ||
                     (VSTR_SECTS_NUM(sects2, 2)->len != 3)));

  /* no limit - check return value */
  sects2->num = 0;
  split2_num = VSTR_SPLIT_CSTR_BUF(s1, 1, s1->len, "::",
                                   sects2, 0, VSTR_FLAG_SPLIT_DEF);

  TST_B_TST(ret, 4, ((sects2->num != 2) ||
                     (split2_num  != 4) ||
                     (VSTR_SECTS_NUM(sects2, 1)->pos != a_pos) ||
                     (VSTR_SECTS_NUM(sects2, 1)->len != 2) ||
                     (VSTR_SECTS_NUM(sects2, 2)->pos != b_pos) ||
                     (VSTR_SECTS_NUM(sects2, 2)->len != 3)));

  /* with REMAIN flag -- also does second split into same section for sects8 */
  sects2->num = 0;
  split2_num = VSTR_SPLIT_CSTR_BUF(s1, 1, s1->len, "::",
                                   sects2, sects2->sz, VSTR_FLAG_SPLIT_REMAIN);
  sects4->num = 0;
  split4_num = vstr_split_cstr_buf(s1, 1, s1->len, "::",
                                   sects4, sects4->sz, VSTR_FLAG_SPLIT_REMAIN);
  split8_num = VSTR_SPLIT_CSTR_BUF(s1, 1, s1->len, "::",
                                   sects8, sects8->sz, VSTR_FLAG_SPLIT_REMAIN);

  TST_B_TST(ret, 5, ((sects8->num != 8) ||
                     (split8_num  != 4) ||
                     (VSTR_SECTS_NUM(sects8, 5)->pos != a_pos) ||
                     (VSTR_SECTS_NUM(sects8, 5)->len != 2) ||
                     (VSTR_SECTS_NUM(sects8, 6)->pos != b_pos) ||
                     (VSTR_SECTS_NUM(sects8, 6)->len != 3) ||
                     (VSTR_SECTS_NUM(sects8, 7)->pos != c_pos) ||
                     (VSTR_SECTS_NUM(sects8, 7)->len != 1) ||
                     (VSTR_SECTS_NUM(sects8, 8)->pos != d_pos) ||
                     (VSTR_SECTS_NUM(sects8, 8)->len != 2)));

  TST_B_TST(ret, 6, ((sects4->num != 4) ||
                     (split4_num  != 4) ||
                     (VSTR_SECTS_NUM(sects4, 1)->pos != a_pos) ||
                     (VSTR_SECTS_NUM(sects4, 1)->len != 2) ||
                     (VSTR_SECTS_NUM(sects4, 2)->pos != b_pos) ||
                     (VSTR_SECTS_NUM(sects4, 2)->len != 3) ||
                     (VSTR_SECTS_NUM(sects4, 3)->pos != c_pos) ||
                     (VSTR_SECTS_NUM(sects4, 3)->len != 1) ||
                     (VSTR_SECTS_NUM(sects4, 4)->pos != d_pos) ||
                     (VSTR_SECTS_NUM(sects4, 4)->len != 2)));

  TST_B_TST(ret, 7, ((sects2->num != 2) ||
                     (split2_num  != 2) ||
                     (VSTR_SECTS_NUM(sects2, 1)->pos != a_pos) ||
                     (VSTR_SECTS_NUM(sects2, 1)->len != 2) ||
                     (VSTR_SECTS_NUM(sects2, 2)->pos != b_pos) ||
                     (VSTR_SECTS_NUM(sects2, 2)->len != 10)));

  /* with REMAIN and beg null flags */
  sects2->num = 0;
  split2_num = VSTR_SPLIT_CSTR_BUF(s1, 1, s1->len, "::",
                                   sects2, sects2->sz,
                                   VSTR_FLAG_SPLIT_BEG_NULL |
                                   VSTR_FLAG_SPLIT_REMAIN);

  sects8->num = 0;
  split8_num = VSTR_SPLIT_CSTR_BUF(s1, 1, s1->len, "::",
                                   sects8, sects8->sz,
                                   VSTR_FLAG_SPLIT_BEG_NULL |
                                   VSTR_FLAG_SPLIT_REMAIN);

  TST_B_TST(ret, 8, ((sects8->num != 5) ||
                     (split8_num  != 5) ||
                     (VSTR_SECTS_NUM(sects8, 1)->pos != 1) ||
                     (VSTR_SECTS_NUM(sects8, 1)->len != 0) ||
                     (VSTR_SECTS_NUM(sects8, 2)->pos != a_pos) ||
                     (VSTR_SECTS_NUM(sects8, 2)->len != 2) ||
                     (VSTR_SECTS_NUM(sects8, 3)->pos != b_pos) ||
                     (VSTR_SECTS_NUM(sects8, 3)->len != 3) ||
                     (VSTR_SECTS_NUM(sects8, 4)->pos != c_pos) ||
                     (VSTR_SECTS_NUM(sects8, 4)->len != 1) ||
                     (VSTR_SECTS_NUM(sects8, 5)->pos != d_pos) ||
                     (VSTR_SECTS_NUM(sects8, 5)->len != 2)));

  TST_B_TST(ret, 9, ((sects2->num != 2) ||
                     (split2_num  != 2) ||
                     (VSTR_SECTS_NUM(sects2, 1)->pos != 1) ||
                     (VSTR_SECTS_NUM(sects2, 1)->len != 0) ||
                     (VSTR_SECTS_NUM(sects2, 2)->pos != a_pos) ||
                     (VSTR_SECTS_NUM(sects2, 2)->len != 14)));

  /* with beg_null, and no remain ... check ret */
  sects2->num = 0;
  split2_num = VSTR_SPLIT_CSTR_BUF(s1, 1, s1->len, "::",
                                   sects2, 0,
                                   VSTR_FLAG_SPLIT_BEG_NULL);

  TST_B_TST(ret, 10, ((sects2->num != 2) ||
                      (split2_num  != 5) ||
                      (VSTR_SECTS_NUM(sects2, 1)->pos != 1) ||
                      (VSTR_SECTS_NUM(sects2, 1)->len != 0) ||
                      (VSTR_SECTS_NUM(sects2, 2)->pos != a_pos) ||
                      (VSTR_SECTS_NUM(sects2, 2)->len != 2)));

  /* with beg_null, no remain and no ret needed */
  sects2->num = 0;
  split2_num = VSTR_SPLIT_CSTR_BUF(s1, 1, s1->len, "::",
                                   sects2, 0,
                                   VSTR_FLAG_SPLIT_BEG_NULL |
                                   VSTR_FLAG_SPLIT_NO_RET);

  TST_B_TST(ret, 11, ((sects2->num != 2) ||
                      !split2_num ||
                      (VSTR_SECTS_NUM(sects2, 1)->pos != 1) ||
                      (VSTR_SECTS_NUM(sects2, 1)->len != 0) ||
                      (VSTR_SECTS_NUM(sects2, 2)->pos != a_pos) ||
                      (VSTR_SECTS_NUM(sects2, 2)->len != 2)));

  {
    Vstr_sects *s_tmp = vstr_sects_make(32);
    unsigned int num = 0;

    VSTR_ADD_CSTR_BUF(s1, s1->len, ":");

    num = VSTR_SPLIT_CSTR_BUF(s1, 1, s1->len, ":",
                              s_tmp, 0,
                              VSTR_FLAG_SPLIT_BEG_NULL |
                              VSTR_FLAG_SPLIT_MID_NULL |
                              VSTR_FLAG_SPLIT_END_NULL |
                              VSTR_FLAG_SPLIT_POST_NULL);

    TST_B_TST(ret, 11, (FALSE ||
                        (s_tmp->sz  != 32) ||
                        (s_tmp->num != 13) ||
                        (num != 13) ||
                        FALSE));

    TST_B_TST(ret, 12, (FALSE ||
                        (VSTR_SECTS_NUM(s_tmp, 1)->pos != 1) ||
                        (VSTR_SECTS_NUM(s_tmp, 1)->len != 0) ||
                        FALSE));

    TST_B_TST(ret, 13, (FALSE ||
                        (VSTR_SECTS_NUM(s_tmp, 2)->pos != 2) ||
                        (VSTR_SECTS_NUM(s_tmp, 2)->len != 0) ||
                        FALSE));

    TST_B_TST(ret, 14, (FALSE ||
                        (VSTR_SECTS_NUM(s_tmp, 3)->pos != 3) ||
                        (VSTR_SECTS_NUM(s_tmp, 3)->len != 0) ||
                        FALSE));

    TST_B_TST(ret, 15, (FALSE ||
                        (VSTR_SECTS_NUM(s_tmp, 4)->pos != 4) ||
                        (VSTR_SECTS_NUM(s_tmp, 4)->len != 1) ||
                        FALSE));

    TST_B_TST(ret, 16, (FALSE ||
                        (VSTR_SECTS_NUM(s_tmp, 5)->pos != 6) ||
                        (VSTR_SECTS_NUM(s_tmp, 5)->len != 0) ||
                        FALSE));

    TST_B_TST(ret, 17, (FALSE ||
                        (VSTR_SECTS_NUM(s_tmp, 6)->pos != 7) ||
                        (VSTR_SECTS_NUM(s_tmp, 6)->len != 1) ||
                        FALSE));

    TST_B_TST(ret, 18, (FALSE ||
                        (VSTR_SECTS_NUM(s_tmp, 7)->pos != 9) ||
                        (VSTR_SECTS_NUM(s_tmp, 7)->len != 1) ||
                        FALSE));

    TST_B_TST(ret, 19, (FALSE ||
                        (VSTR_SECTS_NUM(s_tmp, 8)->pos != 11) ||
                        (VSTR_SECTS_NUM(s_tmp, 8)->len !=  0) ||
                        FALSE));

    TST_B_TST(ret, 20, (FALSE ||
                        (VSTR_SECTS_NUM(s_tmp, 9)->pos != 12) ||
                        (VSTR_SECTS_NUM(s_tmp, 9)->len !=  1) ||
                        FALSE));

    TST_B_TST(ret, 21, (FALSE ||
                        (VSTR_SECTS_NUM(s_tmp, 10)->pos != 14) ||
                        (VSTR_SECTS_NUM(s_tmp, 10)->len !=  0) ||
                        FALSE));

    TST_B_TST(ret, 22, (FALSE ||
                        (VSTR_SECTS_NUM(s_tmp, 11)->pos != 15) ||
                        (VSTR_SECTS_NUM(s_tmp, 11)->len !=  1) ||
                        FALSE));

    TST_B_TST(ret, 23, (FALSE ||
                        (VSTR_SECTS_NUM(s_tmp, 12)->pos != 17) ||
                        (VSTR_SECTS_NUM(s_tmp, 12)->len !=  0) ||
                        FALSE));

    TST_B_TST(ret, 24, (FALSE ||
                        (VSTR_SECTS_NUM(s_tmp, 13)->pos != 18) ||
                        (VSTR_SECTS_NUM(s_tmp, 13)->len !=  0) ||
                        FALSE));

    vstr_sects_free(s_tmp);
  }

  vstr_del(s1, 1, s1->len);
  vstr_add_cstr_buf(s1, s1->len, ":::a");

  sects8->num = 0;
  split8_num = vstr_split_cstr_buf(s1, 1, s1->len, ":",
                                   sects8, sects8->sz,
                                   VSTR_FLAG_SPLIT_DEF);
  TST_B_TST(ret, 25, split8_num != 1);
  sects8->num = 0;
  split8_num = vstr_split_cstr_buf(s1, 1, s1->len, ":",
                                   sects8, 2,
                                   VSTR_FLAG_SPLIT_BEG_NULL);
  TST_B_TST(ret, 26, split8_num  != 2);
  TST_B_TST(ret, 27, sects8->num != 2);
  sects8->num = 0;
  split8_num = vstr_split_cstr_buf(s1, 1, s1->len, ":a",
                                   sects8, 0,
                                   VSTR_FLAG_SPLIT_BEG_NULL |
                                   VSTR_FLAG_SPLIT_POST_NULL);
  TST_B_TST(ret, 28, split8_num != 2);

  sects8->num = 0;
  split8_num = vstr_split_cstr_buf(s1, 1, 3, "::",
                                   sects8, 0,
                                   VSTR_FLAG_SPLIT_BEG_NULL |
                                   VSTR_FLAG_SPLIT_END_NULL |
                                   VSTR_FLAG_SPLIT_POST_NULL);
  TST_B_TST(ret, 29, split8_num != 2);
  
  sects8->num = 0;
  split8_num = vstr_split_cstr_buf(s1, 1, 3, ":",
                                   sects8, 0,
                                   VSTR_FLAG_SPLIT_BEG_NULL |
                                   VSTR_FLAG_SPLIT_END_NULL |
                                   VSTR_FLAG_SPLIT_POST_NULL);
  TST_B_TST(ret, 29, split8_num != 4);
  
  sects8->num = 0;
  split8_num = vstr_split_cstr_buf(s1, 1, 3, ":",
                                   sects8, 0, VSTR_FLAG_SPLIT_DEF);
  ASSERT(!split8_num);

  mfail_count = 0;
  do
  {
    tst_mfail_num(0);
    vstr_sects_free(msects);
    msects = vstr_sects_make(1);
    ASSERT(msects && (msects->sz == 1) && !msects->num);
    
    ASSERT(!sects8->num);
    tst_mfail_num(++mfail_count);
  } while (!(split8_num = vstr_split_cstr_buf(s1, 1, 3, ":",
                                              msects, 2,
                                              VSTR_FLAG_SPLIT_BEG_NULL |
                                              VSTR_FLAG_SPLIT_END_NULL |
                                              VSTR_FLAG_SPLIT_POST_NULL)));
  tst_mfail_num(0);
  TST_B_TST(ret, 30, split8_num  != 2);
  TST_B_TST(ret, 30, msects->num != 2);
  
  vstr_add_cstr_ptr(s1, 0, "x");
  
  mfail_count = 0;
  do
  {
    tst_mfail_num(0);
    vstr_sects_free(msects);
    msects = vstr_sects_make(1);
    ASSERT(msects && (msects->sz == 1) && !msects->num);
    
    tst_mfail_num(++mfail_count);
  } while (!(split8_num = vstr_split_cstr_buf(s1, 1, s1->len, ":",
                                              msects, 3,
                                              VSTR_FLAG_SPLIT_REMAIN)));
  tst_mfail_num(0);
  TST_B_TST(ret, 30, split8_num  != 2);
  TST_B_TST(ret, 30, msects->num != 2);

  vstr_sc_reduce(s1, 1, s1->len, 1);
  
  mfail_count = 0;
  do
  {
    tst_mfail_num(0);
    vstr_sects_free(msects);
    msects = vstr_sects_make(1);
    ASSERT(msects && (msects->sz == 1) && !msects->num);
    
    tst_mfail_num(++mfail_count);
  } while (!(split8_num = vstr_split_cstr_buf(s1, 1, s1->len, ":",
                                              msects, 3,
                                              VSTR_FLAG_SPLIT_REMAIN |
                                              VSTR_FLAG_SPLIT_POST_NULL )));
  tst_mfail_num(0);
  TST_B_TST(ret, 30, split8_num  != 2);
  TST_B_TST(ret, 30, msects->num != 2);

  mfail_count = 0;
  do
  {
    tst_mfail_num(0);
    vstr_sects_free(msects);
    msects = vstr_sects_make(1);
    ASSERT(msects && (msects->sz == 1) && !msects->num);
    
    tst_mfail_num(++mfail_count);
  } while (!(split8_num = vstr_split_cstr_buf(s1, 1, s1->len, ":",
                                              msects, 0,
                                              VSTR_FLAG_SPLIT_BEG_NULL |
                                              VSTR_FLAG_SPLIT_END_NULL |
                                              VSTR_FLAG_SPLIT_POST_NULL)));
  tst_mfail_num(0);
  TST_B_TST(ret, 30, split8_num  != 4);
  TST_B_TST(ret, 30, msects->num != 4);

  vstr_sects_free(msects);

  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "::::!!!!----");
  /*                                 123456789 1 */
  /* beg_nul err */
  VSTR_SECTS_NUM(sects8, 1)->pos = 88;
  VSTR_SECTS_NUM(sects8, 1)->len = 88;
  sects8->num        = 0;
  sects8->sz         = 1;
  sects8->can_add_sz = FALSE;
  split8_num = vstr_split_cstr_buf(s1, 1, s1->len, ":",
                                   sects8, 0,
                                   VSTR_FLAG_SPLIT_BEG_NULL |
                                   VSTR_FLAG_SPLIT_MID_NULL |
                                   VSTR_FLAG_SPLIT_END_NULL |
                                   VSTR_FLAG_SPLIT_POST_NULL);

  ASSERT(split8_num  == 5);
  ASSERT(sects8->num == 1);
  ASSERT(VSTR_SECTS_NUM(sects8, 1)->pos == 1);
  ASSERT(VSTR_SECTS_NUM(sects8, 1)->len == 0);

  if (MFAIL_NUM_OK)
  {
    Vstr_sects *mst = vstr_sects_make(1);

    ASSERT(mst);

    VSTR_SECTS_NUM(mst, 1)->pos = 88;
    VSTR_SECTS_NUM(mst, 1)->len = 88;
      
    tst_mfail_num(1);
    mst->num = 0;
    mst->sz  = 1;
    split8_num = vstr_split_cstr_buf(s1, 1, s1->len, ":", mst, 0,
                                     VSTR_FLAG_SPLIT_BEG_NULL |
                                     VSTR_FLAG_SPLIT_MID_NULL |
                                     VSTR_FLAG_SPLIT_END_NULL |
                                     VSTR_FLAG_SPLIT_POST_NULL);

    ASSERT(split8_num == 0);
    ASSERT(mst->num   == 0);
    ASSERT(VSTR_SECTS_NUM(mst, 1)->pos == 1);
    ASSERT(VSTR_SECTS_NUM(mst, 1)->len == 0);

    vstr_sects_free(mst);
  }

  /* mid_nul err */
  VSTR_SECTS_NUM(sects8, 1)->pos = 88;
  VSTR_SECTS_NUM(sects8, 1)->len = 88;
  sects8->num        = 0;
  sects8->sz         = 1;
  sects8->can_add_sz = FALSE;
  split8_num = vstr_split_cstr_buf(s1, 1, s1->len, "!",
                                   sects8, 0,
                                   VSTR_FLAG_SPLIT_BEG_NULL |
                                   VSTR_FLAG_SPLIT_MID_NULL |
                                   VSTR_FLAG_SPLIT_END_NULL |
                                   VSTR_FLAG_SPLIT_POST_NULL);
  
  ASSERT(split8_num  == 5);
  ASSERT(sects8->num == 1);
  ASSERT(VSTR_SECTS_NUM(sects8, 1)->pos == 1);
  ASSERT(VSTR_SECTS_NUM(sects8, 1)->len == 4);

  if (MFAIL_NUM_OK)
  {
    Vstr_sects *mst = vstr_sects_make(1);

    ASSERT(mst);

    VSTR_SECTS_NUM(mst, 1)->pos = 88;
    VSTR_SECTS_NUM(mst, 1)->len = 88;
      
    tst_mfail_num(1);
    mst->num = 0;
    mst->sz  = 1;
    split8_num = vstr_split_cstr_buf(s1, 1, s1->len, "!", mst, 0,
                                     VSTR_FLAG_SPLIT_BEG_NULL |
                                     VSTR_FLAG_SPLIT_MID_NULL |
                                     VSTR_FLAG_SPLIT_END_NULL |
                                     VSTR_FLAG_SPLIT_POST_NULL);

    ASSERT(split8_num == 0);
    ASSERT(mst->num   == 0);
    ASSERT(VSTR_SECTS_NUM(mst, 1)->pos == 1);
    ASSERT(VSTR_SECTS_NUM(mst, 1)->len == 4);

    vstr_sects_free(mst);
  }
  
  
  return (TST_B_RET(ret));
}
