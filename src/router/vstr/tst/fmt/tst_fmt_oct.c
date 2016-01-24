#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;

  vstr_add_fmt(s1, 0, "%.0o", 0);
  ret |= s1->len;
  vstr_add_fmt(s1, 0, "%.o", 0);
  ret |= s1->len;
  vstr_add_fmt(s1, 0, "%.*o", 0, 0);
  ret |= s1->len;

  return (!VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
}
