#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  sprintf(buf, "%ld %ld %lu %lu",
          LONG_MAX, LONG_MIN, (unsigned long)0, ULONG_MAX);
  vstr_add_fmt(s1, 0, "%ld %ld %lu %lu",
               LONG_MAX, LONG_MIN, (unsigned long)0, ULONG_MAX);

  return (!VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
}
