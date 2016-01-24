#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  struct iovec *iov = NULL;
  unsigned int num = 0;
  unsigned int count = 0;
  unsigned int ret_num = 0;
  size_t len = 0;

  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);

  VSTR_ADD_CSTR_BUF(s1, 0, buf);

  num = vstr_num(s1, 1, s1->len);
  iov = malloc(sizeof(struct iovec) * num);

  vstr_export_iovec_cpy_ptr(s1, 1, 2, iov, 1, NULL);

  len = vstr_export_iovec_cpy_ptr(s1, 1, s1->len, iov, num, &ret_num);

  TST_B_TST(ret, 1, (len != s1->len));
  TST_B_TST(ret, 2, (num != s1->num));
  TST_B_TST(ret, 3, (num != ret_num));

  len = 0;
  while (len < s1->len)
  {
    TST_B_TST(ret, 4, memcmp(buf + len,
                             iov[count].iov_base, iov[count].iov_len));
    len += iov[count].iov_len;
    ++count;
  }
  TST_B_TST(ret, 5, (len != s1->len));
  TST_B_TST(ret, 6, (num != count));
  TST_B_TST(ret, 7, (num != vstr_num(s1, 1, s1->len - 1)));

  len = vstr_export_iovec_cpy_ptr(s1, 1, 0, iov, num, NULL);
  TST_B_TST(ret, 8, len);

  len = vstr_export_iovec_cpy_ptr(s1, 1, s1->len, iov, 0, NULL);
  TST_B_TST(ret, 9, len);

  free(iov);

  return (TST_B_RET(ret));
}
