#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  unsigned int num = 0;
  
  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);
                                                                                
  VSTR_ADD_CSTR_BUF(s1, 0, buf);
                                                                                
  num = vstr_num(s1, 1, s1->len);
  TST_B_TST(ret, 1, (num != s1->num));

  num = vstr_num(s1, 1, 0);
  TST_B_TST(ret, 2, (num != 0));

  num = vstr_num(s1, 2, 0);
  TST_B_TST(ret, 3, (num != 0));

  VSTR_ADD_CSTR_PTR(s1, 0, buf);

  num = vstr_num(s1, 1, s1->len / 2);
  TST_B_TST(ret, 4, (num != 1));

  num = vstr_num(s1, 1, (s1->len / 2) + 1);
  TST_B_TST(ret, 5, (num != 2));

  return (TST_B_RET(ret));
}
