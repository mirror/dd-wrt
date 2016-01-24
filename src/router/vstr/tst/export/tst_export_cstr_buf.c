#include "tst-main.c"

static const char *rf = __FILE__;

#define SM_BUF_LEN 8

int tst(void)
{
  int ret = 0;
  char *ptr = NULL;
  size_t len = 0;
  char sm_buf[SM_BUF_LEN + 1];

  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);
  ptr = strdup(buf);

  VSTR_ADD_CSTR_BUF(s1, 0, buf);

  memset(buf, 'X', sizeof(buf));
  len = vstr_export_cstr_buf(s1, 1, s1->len, buf, sizeof(buf));

  --len;

  TST_B_TST(ret, 1, (len != s1->len));
  TST_B_TST(ret, 2, strcmp(buf, ptr));

  /* overflow */
  memset(sm_buf, 'X', sizeof(sm_buf));
  len = vstr_export_cstr_buf(s1, 1, s1->len, sm_buf, sizeof(sm_buf));

  --len;

  TST_B_TST(ret, 3, (len != SM_BUF_LEN));
  TST_B_TST(ret, 4, memcmp(sm_buf, ptr, SM_BUF_LEN));
  TST_B_TST(ret, 5, (sm_buf[SM_BUF_LEN] != '\0'));

  free(ptr);

  memset(sm_buf, 'X', sizeof(sm_buf));
  memset(buf,    'X', sizeof(sm_buf));
  vstr_export_cstr_buf(s1, 1, s1->len, sm_buf, 0);

  TST_B_TST(ret, 6, memcmp(sm_buf, buf, sizeof(sm_buf)));

  return (TST_B_RET(ret));
}
