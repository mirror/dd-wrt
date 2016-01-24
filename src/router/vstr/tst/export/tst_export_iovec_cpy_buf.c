#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  char *ptr = NULL;
  struct iovec iov[4];
  unsigned int num = 0;
  size_t len = 0;

  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);
  ptr = strdup(buf);

  VSTR_ADD_CSTR_BUF(s1, 0, buf);

  iov[0].iov_base = buf;
  iov[0].iov_len  = 1;
  iov[1].iov_base = buf + 1;
  iov[1].iov_len  = 2;
  iov[2].iov_base = buf + 3;
  iov[2].iov_len  = 3;
  iov[3].iov_base = buf + 6;
  iov[3].iov_len  = sizeof(buf) - 6;

  memset(buf, 'X', 2);
  vstr_export_iovec_cpy_buf(s1, 1, 2, iov, 2, NULL);

  TST_B_TST(ret, 1, iov[0].iov_len != 1);
  TST_B_TST(ret, 2, memcmp(iov[0].iov_base, ptr,     1));
  TST_B_TST(ret, 3, iov[1].iov_len != 1);
  TST_B_TST(ret, 4, memcmp(iov[1].iov_base, ptr + 1, 1));
  TST_B_TST(ret, 5, memcmp(buf, ptr, s1->len));
  TST_B_TST(ret, 6, iov[2].iov_len != 3);
  TST_B_TST(ret, 7, iov[3].iov_len != (sizeof(buf) - 6));

  iov[1].iov_len = 2;

  memset(buf, 'X', sizeof(buf));
  len = vstr_export_iovec_cpy_buf(s1, 1, s1->len, iov, 4, &num);

  TST_B_TST(ret,  9, (len != s1->len));
  TST_B_TST(ret, 10, (num != 4));
  TST_B_TST(ret, 11, iov[0].iov_len != 1);
  TST_B_TST(ret, 12, memcmp(iov[0].iov_base, ptr,     1));
  TST_B_TST(ret, 13, iov[1].iov_len != 2);
  TST_B_TST(ret, 14, memcmp(iov[1].iov_base, ptr + 1, 2));
  TST_B_TST(ret, 15, iov[2].iov_len != 3);
  TST_B_TST(ret, 16, memcmp(iov[2].iov_base, ptr + 3, 3));
  TST_B_TST(ret, 17, iov[3].iov_len != len - 6);
  TST_B_TST(ret, 18, memcmp(iov[3].iov_base, ptr + 6, len - 6));
  TST_B_TST(ret, 19, memcmp(ptr, buf, len));

  iov[2].iov_len  = 2;

  memset(buf, 'X', sizeof(buf));
  len = vstr_export_iovec_cpy_buf(s1, 1, s1->len, iov, 3, &num);

  TST_B_TST(ret, 20, (len != 5));
  TST_B_TST(ret, 21, (num != 3));
  TST_B_TST(ret, 22, iov[0].iov_len != 1);
  TST_B_TST(ret, 23, memcmp(iov[0].iov_base, ptr,     1));
  TST_B_TST(ret, 24, iov[1].iov_len != 2);
  TST_B_TST(ret, 25, memcmp(iov[1].iov_base, ptr + 1, 2));
  TST_B_TST(ret, 26, iov[2].iov_len != 2);
  TST_B_TST(ret, 27, memcmp(iov[2].iov_base, ptr + 3, 2));

  free(ptr);

  len = vstr_export_iovec_cpy_buf(s1, 1, 0, iov, 4, NULL);
  TST_B_TST(ret, 28, len);
  len = vstr_export_iovec_cpy_buf(s1, 1, s1->len, iov, 0, NULL);
  TST_B_TST(ret, 29, len);

  return (TST_B_RET(ret));
}
