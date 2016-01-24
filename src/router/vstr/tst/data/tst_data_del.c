#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  Vstr_ref *foo = vstr_ref_make_malloc(4);
  
  TST_B_TST(ret, 1, !vstr_data_add(NULL, "/foo1", foo));
  TST_B_TST(ret, 2, !vstr_data_add(NULL, "/foo2", foo));
  TST_B_TST(ret, 3, !vstr_data_add(NULL, "/foo3", foo));
  TST_B_TST(ret, 4, !vstr_data_add(NULL, "/foo4", foo));
  TST_B_TST(ret, 5, !vstr_data_add(NULL, "/foo5", foo));
  
  TST_B_TST(ret, 6, foo->ref != 6);
  vstr_ref_del(foo);

  vstr_data_del(NULL, 5);
  vstr_data_del(NULL, 3);
  vstr_data_del(NULL, 4);
  vstr_data_del(NULL, 1);
  vstr_data_del(NULL, 2);
  
  return (TST_B_RET(ret));
}
