#include "tst-main.c"

static const char *rf = __FILE__;

static void tst_nums(int *ret,
                     unsigned int num_buf, unsigned int num_non,
                     unsigned int num_ptr, unsigned int num_ref)
{
  unsigned int val = 0;

  TST_B_TST(*ret, 1,
            !vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_GET_NUM_SPARE_BUF, &val));
  TST_B_TST(*ret, 2, (val != num_buf));

  TST_B_TST(*ret, 3,
            !vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_GET_NUM_SPARE_NON, &val));
  TST_B_TST(*ret, 4, (val != num_non));

  TST_B_TST(*ret, 5,
            !vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_GET_NUM_SPARE_PTR, &val));
  TST_B_TST(*ret, 6, (val != num_ptr));

  TST_B_TST(*ret, 7,
            !vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_GET_NUM_SPARE_REF, &val));
  TST_B_TST(*ret, 8, (val != num_ref));
}

int tst(void)
{
  int ret = 0;

  tst_nums(&ret, 0, 0, 0, 0);

  TST_B_TST(ret, 10,
            !vstr_make_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 5));
  TST_B_TST(ret, 11,
            !vstr_make_spare_nodes(s1->conf, VSTR_TYPE_NODE_NON, 6));
  TST_B_TST(ret, 12,
            !vstr_make_spare_nodes(s1->conf, VSTR_TYPE_NODE_PTR, 7));
  TST_B_TST(ret, 13,
            !vstr_make_spare_nodes(s1->conf, VSTR_TYPE_NODE_REF, 8));
  tst_nums(&ret, 5, 6, 7, 8);

  vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 4);
  vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_NON, 3);
  vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_PTR, 2);
  vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_REF, 1);
  tst_nums(&ret, 1, 3, 5, 7);

  vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 2048);
  vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_NON, 1024);
  vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_PTR, 512);
  vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_REF, 256);
  tst_nums(&ret, 0, 0, 0, 0);

  TST_B_TST(ret, 14,
            !vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_SET_NUM_SPARE_BUF, 11));
  TST_B_TST(ret, 15,
            !vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_SET_NUM_SPARE_NON, 12));
  TST_B_TST(ret, 16,
            !vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_SET_NUM_SPARE_PTR, 13));
  TST_B_TST(ret, 17,
            !vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_SET_NUM_SPARE_REF, 14));
  tst_nums(&ret, 11, 12, 13, 14);

  TST_B_TST(ret, 14,
            !vstr_cntl_conf(s1->conf,
                            VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_BUF, 110, 120));
  TST_B_TST(ret, 15,
            !vstr_cntl_conf(s1->conf,
                            VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_NON, 120, 130));
  TST_B_TST(ret, 16,
            !vstr_cntl_conf(s1->conf,
                            VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_PTR, 130, 140));
  TST_B_TST(ret, 17,
            !vstr_cntl_conf(s1->conf,
                            VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_REF, 140, 150));
  tst_nums(&ret, 110, 120, 130, 140);

  TST_B_TST(ret, 21,
            !vstr_cntl_conf(s1->conf,
                            VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_BUF, 10, 20));
  TST_B_TST(ret, 22,
            !vstr_cntl_conf(s1->conf,
                            VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_NON, 20, 30));
  TST_B_TST(ret, 23,
            !vstr_cntl_conf(s1->conf,
                            VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_PTR, 30, 40));
  TST_B_TST(ret, 24,
            !vstr_cntl_conf(s1->conf,
                            VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_REF, 40, 50));
  tst_nums(&ret, 20, 30, 40, 50);

  if (MFAIL_NUM_OK)
  {
    Vstr_conf *succeeded = NULL;
    unsigned long mfail_count = 0;

    while (!succeeded)
    {
      tst_mfail_num(++mfail_count);
      succeeded = vstr_make_conf();
    }
    tst_mfail_num(0);
    ASSERT(mfail_count > 1);
    vstr_free_conf(succeeded);
  }
  
  if (MFAIL_NUM_OK)
  {
    Vstr_base *succeeded = NULL;
    unsigned long mfail_count = 0;

    while (!succeeded)
    {
      tst_mfail_num(++mfail_count);
      succeeded = vstr_make_base(NULL);
    }
    tst_mfail_num(0);
    ASSERT(mfail_count > 1);
    vstr_free_base(succeeded);
  }

  /* pre-init all data */
  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_TYPE_GRPALLOC_CACHE,
                 VSTR_TYPE_CNTL_CONF_GRPALLOC_CSTR);
  if (MFAIL_NUM_OK)
  {
    Vstr_base *succeeded = NULL;
    unsigned long mfail_count = 0;

    while (!succeeded)
    {
      tst_mfail_num(++mfail_count);
      succeeded = vstr_make_base(NULL);
    }
    tst_mfail_num(0);
    ASSERT(mfail_count > 1);
    vstr_free_base(succeeded);
  }

  vstr_free_conf(NULL);

  return (TST_B_RET(ret));
}
