
#define VSTR_COMPILE_MACRO_FUNCTIONS 0

#include "tst-main.c"

static const char *rf = __FILE__;

static int tst_func(int ret, int dum1, int dum2)
{
  if (0) { }
  else if (dum1) return (ret);
  else if (dum2) return (ret);
  else           return (ret);
}

/* so the automatic macro function finder doesn't see it */
#define P(x, y) x ## y
struct
{
 int (* P(VS, TR_ADD_CSTR_BUF))(int, int, int);
} tst_mac = {tst_func};

int tst(void)
{
  return (tst_mac.VSTR_ADD_CSTR_BUF(EXIT_SUCCESS, 0, 0));
}
