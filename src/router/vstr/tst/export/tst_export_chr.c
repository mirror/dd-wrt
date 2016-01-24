#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  size_t len = 0;

  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);

  VSTR_ADD_CSTR_BUF(s1, 0, buf);

  while (len < s1->len)
  {
    ++len;
    if (vstr_export_chr(s1, len) != buf[len - 1])
      return (len * 2);
  }

  vstr_add_non(s2, 0, 16);

  len = 0;
  while (++len <= 16)
    ASSERT(!vstr_export_chr(s2, len));
  
  return (EXIT_SUCCESS);
}
