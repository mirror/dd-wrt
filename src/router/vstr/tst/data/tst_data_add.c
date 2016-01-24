#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  int mfail_count = 0;
  Vstr_ref *foo = vstr_ref_make_malloc(4);
  
  TST_B_TST(ret, 1, s1->conf->data_usr_sz != 1);
  TST_B_TST(ret, 2, s1->conf->data_usr_len != 0);

  do
  {
    tst_mfail_num(++mfail_count);
  } while (!vstr_data_add(s1->conf, "/foo", foo));
  tst_mfail_num(0);

  TST_B_TST(ret, 3, !vstr_data_add(s2->conf, "/foo", foo));
  TST_B_TST(ret, 4, !vstr_data_add(s3->conf, "/foo", foo));
  TST_B_TST(ret, 5, !vstr_data_add(s4->conf, "/foo", foo));

  mfail_count = 0;
  do
  {
    tst_mfail_num(++mfail_count);
  } while (!vstr_data_add(s2->conf, "/NULL", NULL));
  tst_mfail_num(0);
  
  TST_B_TST(ret, 6, !vstr_data_add(NULL, "/baz", NULL));
  TST_B_TST(ret, 7, !vstr_data_add(NULL, "/bar", NULL));
  
  TST_B_TST(ret, 8, s1->conf->data_usr_sz != 4);
  TST_B_TST(ret, 8, s1->conf->data_usr_len != 3);
  
  TST_B_TST(ret, 9, foo->ref != 5);
  vstr_ref_del(foo);
  
  return (TST_B_RET(ret));
}
