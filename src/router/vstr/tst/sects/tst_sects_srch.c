#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  Vstr_sects *sects = vstr_sects_make(4);
  int ret = 0;

  TST_B_TST(ret, 1, (sects->sz != 4));
  TST_B_TST(ret, 2, (sects->num != 0));

  vstr_sects_add(sects, 4, 2);
  vstr_sects_add(sects, 5, 3);
  vstr_sects_add(sects, 6, 4);
  vstr_sects_add(sects, 7, 5);
  vstr_sects_add(sects, 8, 6);

  TST_B_TST(ret, 3, (sects->sz != 8));
  TST_B_TST(ret, 4, (sects->num != 5));

  TST_B_TST(ret, 5, vstr_sects_srch(sects, 4, 2) != 1);
  TST_B_TST(ret, 6, vstr_sects_srch(sects, 5, 3) != 2);
  TST_B_TST(ret, 7, vstr_sects_srch(sects, 6, 4) != 3);
  TST_B_TST(ret, 8, vstr_sects_srch(sects, 7, 5) != 4);
  TST_B_TST(ret, 9, vstr_sects_srch(sects, 8, 6) != 5);

  vstr_sects_add(sects, 4, 2);

  TST_B_TST(ret, 10, vstr_sects_srch(sects, 4, 2) != 1);

  vstr_sects_del(sects, 1);

  TST_B_TST(ret, 11, vstr_sects_srch(sects, 4, 2) != 6);

  TST_B_TST(ret, 12, vstr_sects_srch(sects, 32, 2) != 0);

  sects->sz = 0;
  TST_B_TST(ret, 13, vstr_sects_srch(sects, 4, 2) != 0);

  vstr_sects_free(sects);

  return (TST_B_RET(ret));
}
