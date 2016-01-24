#include "tst-main.c"

static const char *rf = __FILE__;

static int ret = 0;

static void tst_iovec(Vstr_base *t1)
{
  size_t len = 0;
  struct iovec *iov = NULL;
  unsigned int num = 0;
  unsigned int count = 0;
  unsigned int mfail_count = 0;
  
  vstr_export_cstr_buf(t1, 1, t1->len, buf, sizeof(buf));

  mfail_count = 3;
  do
  {
    tst_mfail_num(++mfail_count / 4);
  } while (!(len = vstr_export_iovec_ptr_all(t1, &iov, &num)) && !iov);
  tst_mfail_num(0);
  
  TST_B_TST(ret, 1, (len != t1->len));
  TST_B_TST(ret, 2, (num != t1->num));

  len = 0;
  while (len < t1->len)
  {
    TST_B_TST(ret, 3, !!memcmp(buf + len,
                               iov[count].iov_base, iov[count].iov_len));
    len += iov[count].iov_len;
    ++count;
  }
  TST_B_TST(ret, 4, (len   != t1->len));
  TST_B_TST(ret, 5, (count != vstr_num(t1, 1, t1->len)));
}

int tst(void)
{
  const char *tmp_s = NULL;
  
  ASSERT(!vstr_export_iovec_ptr_all(s2, NULL, NULL));
  
  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);

  {
    int tmp = 0;

    ASSERT(!!vstr_cntl_conf(s3->conf,
                            VSTR_CNTL_CONF_GET_NUM_IOV_MIN_OFFSET, &tmp) &&
           tmp == 0);
    ASSERT(!!vstr_cntl_conf(s3->conf,
                            VSTR_CNTL_CONF_SET_NUM_IOV_MIN_OFFSET, 4));
    ASSERT(!!vstr_cntl_conf(s3->conf,
                            VSTR_CNTL_CONF_GET_NUM_IOV_MIN_OFFSET, &tmp) &&
           tmp == 4);
  }

  VSTR_ADD_CSTR_BUF(s1, 0, buf);
  VSTR_ADD_CSTR_PTR(s1, 0, buf);
  VSTR_ADD_CSTR_BUF(s1, 0, buf);
  VSTR_ADD_CSTR_BUF(s1, 0, buf);
  VSTR_ADD_CSTR_PTR(s1, 0, buf);
  VSTR_ADD_CSTR_BUF(s1, 0, buf);
  vstr_add_vstr(s3, 0, s1, 1, s1->len, 0);

  tst_iovec(s1);
  tst_iovec(s3);

  vstr_add_vstr(s1, s1->len, s1, 1, s1->len, 0);
  vstr_add_vstr(s1, s1->len, s1, 1, s1->len, 0);
  tst_iovec(s1);

  vstr_add_vstr(s1, s1->len, s1, 1, s1->len, 0);
  vstr_add_vstr(s1, s1->len, s1, 1, s1->len, 0);
  tst_iovec(s1);

  vstr_add_vstr(s1, s1->len, s1, 1, s1->len, 0);
  vstr_add_vstr(s1, s1->len, s1, 1, s1->len, 0);
  tst_iovec(s1);

  TST_B_TST(ret, 6, !s1->iovec_upto_date);
  vstr_del(s1, 1, s1->len / 2);
  TST_B_TST(ret, 6, !s1->iovec_upto_date);
  tmp_s = vstr_export_cstr_ptr(s1, 1, s1->len);
  TST_B_TST(ret, 6, !tmp_s);
  TST_B_TST(ret, 6, !s1->iovec_upto_date);
  {
    struct iovec *iovs = NULL;
    unsigned int num = 0;

    vstr_add_iovec_buf_beg(s1, s1->len, 1, 1, &iovs, &num);
    vstr_add_iovec_buf_end(s1, s1->len, 0);
    tst_iovec(s1);
  }

  vstr_add_vstr(s1, s1->len, s1, 1, s1->len, 0);
  tst_iovec(s1);

  tmp_s = vstr_export_cstr_ptr(s3, 1, s3->len);
  TST_B_TST(ret, 10, !tmp_s);
  TST_B_TST(ret, 10, !s3->iovec_upto_date);
  vstr_add_ptr(s3, 0, "abcd", 4);
  vstr_add_buf(s3, 0, "abcd", 4);
  vstr_del(s3, s3->len - 4, 5);
  vstr_add_buf(s3, s3->len, "abcd", 4);
  vstr_add_rep_chr(s3, 0, 'X', s3->conf->buf_sz);
  TST_B_TST(ret, 11, !s3->iovec_upto_date);

  tst_iovec(s3);

  vstr_add_non(s3, 0, 4);

  TST_B_TST(ret, 12, !s3->iovec_upto_date);

  vstr_del(s3, 1, 4);
  vstr_add_ptr(s3, 0, "abcd", 4);
  vstr_add_buf(s3, 0, "abcd", 4);
  vstr_add_ptr(s3, 0, "abcd", 4);
  TST_B_TST(ret, 13, s3->iovec_upto_date);

  tst_iovec(s3);

  vstr_add_non(s3, 0, 4);
  vstr_add_non(s3, 0, 4);
  vstr_add_non(s3, 0, 4);
  vstr_add_non(s3, 0, 4);
  vstr_add_non(s3, 0, 4);

  ASSERT(vstr_export_iovec_ptr_all(s3, NULL, NULL));
 
  return (TST_B_RET(ret));
}
