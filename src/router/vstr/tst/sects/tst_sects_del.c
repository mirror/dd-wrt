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

  TST_B_TST(ret, 5, (sects->ptr[0].pos != 4));
  TST_B_TST(ret, 6, (sects->ptr[0].len != 2));

  TST_B_TST(ret, 7, (sects->ptr[4].pos != 8));
  TST_B_TST(ret, 8, (sects->ptr[4].len != 6));

  vstr_sects_del(sects, 1);

  TST_B_TST(ret, 9, (sects->sz != 8));
  TST_B_TST(ret, 10, (sects->num != 5));

  vstr_sects_del(sects, 1);
  vstr_sects_del(sects, 5);
  vstr_sects_del(sects, 4);

  TST_B_TST(ret, 11, (sects->sz != 8));
  TST_B_TST(ret, 12, (sects->num != 3));

  sects->can_del_sz = TRUE;

  vstr_sects_del(sects, 3);

  TST_B_TST(ret, 13, (sects->sz != 4));
  TST_B_TST(ret, 14, (sects->num != 2));
  
  vstr_sects_free(sects);

  return (TST_B_RET(ret));
}
