#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  vstr_free_base(s1);
  vstr_exit();
  exit (EXIT_SUCCESS);
}
