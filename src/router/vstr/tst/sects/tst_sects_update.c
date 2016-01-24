#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  unsigned int ret = 0;
  Vstr_sects *sects1 = vstr_sects_make(1);
  Vstr_sects *sects2 = vstr_sects_make(1);
  Vstr_sects *sects3 = vstr_sects_make(1);
  int mfail_count = 0;
  
  VSTR_ADD_CSTR_BUF(s1, 0, "123456789 123456789 123456789 123456789 ");
  vstr_add_vstr(s2, 0, s1, 1, s1->len, 0);

  sects2->alloc_double = FALSE;

  vstr_sects_add(sects1,  1, 10);
  vstr_sects_add(sects1, 11, 10);

  vstr_sects_add(sects2,  1, 10);
  vstr_sects_add(sects2, 11, 10);
  vstr_sects_add(sects2, 21, 10);
  vstr_sects_add(sects2, 31, 10);

  vstr_sects_add(sects3,  1, 10);
  vstr_sects_add(sects3, 11, 10);

  mfail_count = 3;
  do
  {
    tst_mfail_num(++mfail_count / 4);
  } while (!vstr_sects_update_add(s1, sects1));
  tst_mfail_num(0);

  mfail_count = 3;
  do
  {
    tst_mfail_num(++mfail_count / 4);
  } while (!vstr_sects_update_add(s1, sects2));
  tst_mfail_num(0);

  mfail_count = 3;
  do
  {
    tst_mfail_num(++mfail_count / 4);
  } while (!vstr_sects_update_add(s1, sects3));
  tst_mfail_num(0);

  ASSERT(!s2->conf->malloc_bad);
  TST_B_TST(ret,  2, !vstr_sects_update_add(s2, sects2));
  ASSERT(!s2->conf->malloc_bad);

  TST_B_TST(ret,  3, !vstr_sects_update_del(s1, sects2)); /* out of order */
  TST_B_TST(ret,  4, !vstr_sects_update_del(s1, sects3));

  TST_B_TST(ret,  5, ((sects1->ptr[0].pos !=  1) ||
                      (sects1->ptr[0].len != 10)));
  TST_B_TST(ret,  6, ((sects1->ptr[1].pos != 11) ||
                      (sects1->ptr[1].len != 10)));

  vstr_del(s1, 2, 1);

  TST_B_TST(ret,  7, ((sects1->ptr[0].pos !=  1) ||
                      (sects1->ptr[0].len !=  9)));
  TST_B_TST(ret,  8, ((sects1->ptr[1].pos != 10) ||
                      (sects1->ptr[1].len != 10)));

  vstr_del(s1, 1, 1);

  TST_B_TST(ret,  9, ((sects1->ptr[0].pos !=  1) ||
                      (sects1->ptr[0].len !=  8)));
  TST_B_TST(ret, 10, ((sects1->ptr[1].pos !=  9) ||
                      (sects1->ptr[1].len != 10)));

  vstr_del(s1, 9, 5);

  TST_B_TST(ret, 11, ((sects1->ptr[0].pos !=  1) ||
                      (sects1->ptr[0].len !=  8)));
  TST_B_TST(ret, 12, ((sects1->ptr[1].pos !=  9) ||
                      (sects1->ptr[1].len !=  5)));

  VSTR_ADD_CSTR_BUF(s1, 0, "abcd");

  TST_B_TST(ret, 13, ((sects1->ptr[0].pos !=  5) ||
                      (sects1->ptr[0].len !=  8)));
  TST_B_TST(ret, 14, ((sects1->ptr[1].pos != 13) ||
                      (sects1->ptr[1].len !=  5)));

  VSTR_ADD_CSTR_BUF(s1, 5, "abcd");

  TST_B_TST(ret, 15, ((sects1->ptr[0].pos !=  5) ||
                      (sects1->ptr[0].len != 12)));
  TST_B_TST(ret, 15, ((sects1->ptr[1].pos != 17) ||
                      (sects1->ptr[1].len !=  5)));

  vstr_del(s1, 11, 8);

  TST_B_TST(ret, 16, ((sects1->ptr[0].pos !=  5) ||
                      (sects1->ptr[0].len !=  6)));
  TST_B_TST(ret, 16, ((sects1->ptr[1].pos != 11) ||
                      (sects1->ptr[1].len !=  3)));

  vstr_del(s1, 12, s1->len - 11);

  TST_B_TST(ret, 17, ((sects1->ptr[0].pos !=  5) ||
                      (sects1->ptr[0].len !=  6)));
  TST_B_TST(ret, 17, ((sects1->ptr[1].pos != 11) ||
                      (sects1->ptr[1].len !=  1)));
  
  vstr_sub_rep_chr(s1, 1, 1, 'X', 1);

  TST_B_TST(ret, 18, ((sects1->ptr[0].pos !=  5) ||
                      (sects1->ptr[0].len !=  6)));
  TST_B_TST(ret, 18, ((sects1->ptr[1].pos != 11) ||
                      (sects1->ptr[1].len !=  1)));

  vstr_del(s1, 7, 5);

  TST_B_TST(ret, 19, ((sects1->ptr[0].pos !=  5) ||
                      (sects1->ptr[0].len !=  2)));
  TST_B_TST(ret, 19, (sects1->ptr[1].pos != 0));

  vstr_del(s1, 6, 1);

  TST_B_TST(ret, 20, ((sects1->ptr[0].pos !=  5) ||
                      (sects1->ptr[0].len !=  1)));
  
  TST_B_TST(ret, 21, !vstr_sects_update_del(s1, sects1));
  TST_B_TST(ret, 22,  vstr_sects_update_del(s1, NULL));
  TST_B_TST(ret, 23,  vstr_cache_get(s1, s1->conf->cache_pos_cb_sects));

  vstr_sects_free(sects1);
  vstr_sects_free(sects2);
  vstr_sects_free(sects3);

  vstr_sects_update_del(s1, NULL);
  vstr_sects_free(NULL);

  return (TST_B_RET(ret));
}
