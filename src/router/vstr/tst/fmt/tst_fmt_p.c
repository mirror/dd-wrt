#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;

  sprintf(buf, "%p", buf);
  if (!strcmp("%p", buf))
    return (EXIT_FAILED_OK);
  if ((buf[0] != '0') ||
      (buf[1] != 'x'))
    return (EXIT_FAILED_OK); /* Solaris formats differently, of course */
  
  vstr_add_fmt(s1, 0, "%p", buf);

  TST_B_TST(ret, 1, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));

  return (TST_B_RET(ret));
}
