#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
#if !defined(USE_RESTRICTED_HEADERS)
  exit (EXIT_FAILED_OK);
#endif
  
  ASSERT(!vstr_add_fmt(s1, 0, "%lc", (wint_t)L'a'));
}
