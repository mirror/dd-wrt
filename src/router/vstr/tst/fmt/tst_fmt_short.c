#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  sprintf(buf, "%hd %hd %hu %hu", SHRT_MAX, SHRT_MIN, 0, USHRT_MAX);
  vstr_add_fmt(s1, 0, "%hd %hd %hu %hu", SHRT_MAX, SHRT_MIN, 0, USHRT_MAX);

  return (!VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
}
