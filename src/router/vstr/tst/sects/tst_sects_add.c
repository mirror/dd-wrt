#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  Vstr_sects *sects = vstr_sects_make(4);
  int ret = 0;
  int mfail_count = 0;
  
  TST_B_TST(ret, 1, (sects->sz != 4));
  TST_B_TST(ret, 2, (sects->num != 0));

  vstr_sects_add(sects, 4, 2);

  TST_B_TST(ret, 3, (sects->sz != 4));
  TST_B_TST(ret, 4, (sects->num != 1));

  TST_B_TST(ret, 5, (sects->ptr[0].pos != 4));
  TST_B_TST(ret, 6, (sects->ptr[0].len != 2));

  vstr_sects_free(sects);
  sects = vstr_sects_make(0);
  
  do
  {
    tst_mfail_num(++mfail_count);
  } while (!vstr_sects_add(sects, 1, 1));
  tst_mfail_num(0);

  TST_B_TST(ret, 7, (sects->sz != 1));
  TST_B_TST(ret, 8, (sects->num != 1));

  mfail_count = 0;
  do
  {
    tst_mfail_num(++mfail_count);
  } while (!vstr_sects_add(sects, 1, 1));
  tst_mfail_num(0);

  TST_B_TST(ret, 7, (sects->sz != 2));
  TST_B_TST(ret, 8, (sects->num != 2));

  mfail_count = 0;
  do
  {
    tst_mfail_num(++mfail_count);
  } while (!vstr_sects_add(sects, 1, 1));
  tst_mfail_num(0);

  TST_B_TST(ret, 7, (sects->sz != 4));
  TST_B_TST(ret, 8, (sects->num != 3));

  /* test overflow ... probably hard to trigger */
  sects->malloc_bad = FALSE;
  sects->num        = UINT_MAX;
  sects->sz         = UINT_MAX;
  TST_B_TST(ret, 30, vstr_sects_add(sects, 1, 1));
  TST_B_TST(ret, 31, !sects->malloc_bad);
  
  vstr_sects_free(sects);

  return (TST_B_RET(ret));
}
