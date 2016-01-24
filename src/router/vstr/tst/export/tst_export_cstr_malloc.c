#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  char *ptr = NULL;
  int ret = 0;

  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);

  VSTR_ADD_CSTR_BUF(s1, 0, buf);

  ptr = vstr_export_cstr_malloc(s1, 1, s1->len);

  TST_B_TST(ret, 1, !!strcmp(buf, ptr));

  free(ptr);

  ptr = vstr_export_cstr_malloc(s1, 4, s1->len - 3);

  TST_B_TST(ret, 2, !!strcmp(buf + 3, ptr));

  free(ptr);

  return (TST_B_RET(ret));
}
