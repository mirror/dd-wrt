#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  vstr_add_non(s1, s1->len, 0);
  vstr_add_non(s1, s1->len, 128);
  vstr_add_non(s1, s1->len, 0);

  assert(VSTR_CMP_BUF_EQ(s1, 1, s1->len, NULL, 128));

  vstr_del(s1, 1, s1->len);
  
  vstr_add_non(s1, 0, SSIZE_MAX);

  ASSERT(s1->len == SSIZE_MAX);

  vstr_add_non(s1, 0, (SIZE_MAX - SSIZE_MAX));

  ASSERT(s1->len == SIZE_MAX);

  return (EXIT_SUCCESS);
}
