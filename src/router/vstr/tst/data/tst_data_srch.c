#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  Vstr_ref *foo = vstr_ref_make_malloc(4);
  
  TST_B_TST(ret,  1, !vstr_data_add(NULL, "/foo1", foo));
  TST_B_TST(ret,  2, !vstr_data_add(NULL, "/foo2", foo));
  TST_B_TST(ret,  3, !vstr_data_add(NULL, "/foo3", foo));
  TST_B_TST(ret,  4, !vstr_data_add(NULL, "/foo4", foo));
  
  TST_B_TST(ret,  6, !vstr_data_add(s4->conf, "/foo4", foo));
  TST_B_TST(ret,  7, !vstr_data_add(s4->conf, "/foo3", foo));
  TST_B_TST(ret,  8, !vstr_data_add(s4->conf, "/foo2", foo));
  TST_B_TST(ret,  9, !vstr_data_add(s4->conf, "/foo1", foo));
  
  TST_B_TST(ret, 10, foo->ref != 9);
  vstr_ref_del(foo);
  
  TST_B_TST(ret, 11, vstr_data_srch(NULL, "/foo1") != 1);
  TST_B_TST(ret, 12, vstr_data_srch(NULL, "/foo2") != 2);
  TST_B_TST(ret, 13, vstr_data_srch(NULL, "/foo3") != 3);
  TST_B_TST(ret, 14, vstr_data_srch(NULL, "/foo4") != 4);
  TST_B_TST(ret, 15, vstr_data_srch(s1->conf, "/foo1") != 1);
  TST_B_TST(ret, 16, vstr_data_srch(s1->conf, "/foo2") != 2);
  TST_B_TST(ret, 17, vstr_data_srch(s1->conf, "/foo3") != 3);
  TST_B_TST(ret, 18, vstr_data_srch(s1->conf, "/foo4") != 4);
  
  TST_B_TST(ret, 19, vstr_data_srch(s4->conf, "/foo4") != 1);
  TST_B_TST(ret, 20, vstr_data_srch(s4->conf, "/foo3") != 2);
  TST_B_TST(ret, 21, vstr_data_srch(s4->conf, "/foo2") != 3);
  TST_B_TST(ret, 22, vstr_data_srch(s4->conf, "/foo1") != 4);
  
  return (TST_B_RET(ret));
}
