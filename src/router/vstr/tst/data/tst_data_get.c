#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  Vstr_ref *foo1 = vstr_ref_make_malloc(4);
  Vstr_ref *foo2 = vstr_ref_make_malloc(4);
  Vstr_ref *foo3 = vstr_ref_make_malloc(4);
  Vstr_ref *foo4 = vstr_ref_make_malloc(4);
  
  TST_B_TST(ret,  1, !vstr_data_add(NULL, "/foo1", foo1));
  TST_B_TST(ret,  2, !vstr_data_add(NULL, "/foo2", foo2));
  TST_B_TST(ret,  3, !vstr_data_add(NULL, "/foo3", foo3));
  TST_B_TST(ret,  4, !vstr_data_add(NULL, "/foo4", foo4));
  
  TST_B_TST(ret,  5, !vstr_data_add(s4->conf, "/foo1", foo4));
  TST_B_TST(ret,  6, !vstr_data_add(s4->conf, "/foo2", foo3));
  TST_B_TST(ret,  7, !vstr_data_add(s4->conf, "/foo3", foo2));
  TST_B_TST(ret,  8, !vstr_data_add(s4->conf, "/foo4", foo1));
  
  TST_B_TST(ret, 11, vstr_data_get(NULL, 1) != foo1->ptr);
  TST_B_TST(ret, 12, vstr_data_get(NULL, 2) != foo2->ptr);
  TST_B_TST(ret, 13, vstr_data_get(NULL, 3) != foo3->ptr);
  TST_B_TST(ret, 14, vstr_data_get(NULL, 4) != foo4->ptr);
  TST_B_TST(ret, 15, vstr_data_get(s1->conf, 1) != foo1->ptr);
  TST_B_TST(ret, 16, vstr_data_get(s1->conf, 2) != foo2->ptr);
  TST_B_TST(ret, 17, vstr_data_get(s1->conf, 3) != foo3->ptr);
  TST_B_TST(ret, 18, vstr_data_get(s1->conf, 4) != foo4->ptr);
  
  TST_B_TST(ret, 19, vstr_data_get(s4->conf, 4) != foo1->ptr);
  TST_B_TST(ret, 20, vstr_data_get(s4->conf, 3) != foo2->ptr);
  TST_B_TST(ret, 21, vstr_data_get(s4->conf, 2) != foo3->ptr);
  TST_B_TST(ret, 22, vstr_data_get(s4->conf, 1) != foo4->ptr);

  TST_B_TST(ret, 23, foo1->ref != 3);
  vstr_ref_del(foo1);
  TST_B_TST(ret, 24, foo2->ref != 3);
  vstr_ref_del(foo2);
  TST_B_TST(ret, 25, foo3->ref != 3);
  vstr_ref_del(foo3);
  TST_B_TST(ret, 26, foo4->ref != 3);
  vstr_ref_del(foo4);  

  TST_B_TST(ret, 27, !vstr_data_add(s2->conf, "/NULL", NULL));
  TST_B_TST(ret, 28,  vstr_data_get(s2->conf, 1));
  
  return (TST_B_RET(ret));
}
