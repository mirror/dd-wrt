#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  struct iovec *iovs = NULL;
  size_t blen = 0;
  size_t len = 0;
  unsigned int num = 0;
  unsigned int scan = 0;
  const char *bscan = buf;
  int done = FALSE;
  int mfail_count = 0;

  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);
  blen = strlen(buf);

  do
  {
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!(len = vstr_add_iovec_buf_beg(s3, 0, 32, 32, &iovs, &num)));
  tst_mfail_num(0);

  if (len < blen)
    return (2);
  len = blen;

  while (blen && (scan < num))
  {
    size_t tmp = iovs[scan].iov_len;

    if (tmp > blen)
      tmp = blen;

    memcpy(iovs[scan].iov_base, bscan, tmp);

    bscan += tmp;
    blen -= tmp;

    if (!blen)
      break;

    if ((scan == 2) && !done)
    {
      ASSERT(iovs[scan].iov_len == 4);
      done = TRUE;
      continue;
    }

    ++scan;
  }
  vstr_add_iovec_buf_end(s3, 0, len - 4);

  len = vstr_add_iovec_buf_beg(s3, 8, 2, 2, &iovs, &num);
  if (!len)
    return (3);
  if (len < 4)
    return (4);

  ASSERT(iovs[0].iov_len == 4);
  memcpy(iovs[0].iov_base, buf + 8, 4);
  vstr_add_iovec_buf_end(s3, 8, 4);

  vstr_add_iovec_buf_beg(s3, 0, 2, 2, &iovs, &num);
  vstr_add_iovec_buf_end(s3, 0, 0);

  vstr_add_iovec_buf_beg(s3, 0, 2, 2, &iovs, &num);
  vstr_add_iovec_buf_end(s3, 0, 0);

  vstr_add_iovec_buf_beg(s3, 2, 2, 2, &iovs, &num);
  vstr_add_iovec_buf_end(s3, 2, 0);

  vstr_add_iovec_buf_beg(s3, s3->len, 2, 2, &iovs, &num);
  vstr_add_iovec_buf_end(s3, s3->len, 0);

  assert(VSTR_CMP_CSTR_EQ(s3, 1, s3->len, buf));

  vstr_del(s3, 1, s3->len);
  vstr_add_cstr_ptr(s3, s3->len, "1234");

  assert(VSTR_CMP_CSTR_EQ(s3, 1, s3->len, "1234"));
  assert(vstr_num(s3, 1, s3->len) == 1);

  mfail_count = 0;
  do
  {
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_PTR, 1000);
    tst_mfail_num(++mfail_count);
  } while (!(len = vstr_add_iovec_buf_beg(s3, 2, 32, 32, &iovs, &num)));
  tst_mfail_num(0);

  vstr_add_iovec_buf_end(s3, 2, 0);

  assert(VSTR_CMP_CSTR_EQ(s3, 1, s3->len, "1234"));
  assert(vstr_num(s3, 1, s3->len) == 2);
  
  return (EXIT_SUCCESS);
}
