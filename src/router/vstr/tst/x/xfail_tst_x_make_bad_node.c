#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  vstr_make_spare_nodes(NULL, 666, 1);
}
