#include "tst-main.c"

static const char *rf = __FILE__;

#define T20 "123456789 123456789 "

# define TST_PERCENT_N(RNUM, n) do { \
  vstr_del(tst_s, 1, tst_s->len); \
  vstr_add_fmt(tst_s, 0, T20 "%" #n T20, & x ## n); \
  TST_B_TST(ret, RNUM, (x ## n != 20)); \
 } while (FALSE)

int tst(void)
{
  Vstr_base *tst_s = s1;
  int ret = 0;
  char      xhhn = (char)~0U;
  short      xhn = (short)~0U;
  int         xn = ~0U;
  long       xln = ~0U;
#ifdef HAVE_LONG_LONG
  long long xlln = ~0U;
#endif
  ptrdiff_t  xtn = ~0U;
  size_t     xzn = ~0U;
  intmax_t   xjn = ~0U;

  tst_s = s1;
  TST_PERCENT_N( 1,  hhn);
  TST_PERCENT_N( 2,  hn);
  TST_PERCENT_N( 3,   n);
  TST_PERCENT_N( 4,  ln);
  TST_PERCENT_N( 5, lln);
  TST_PERCENT_N( 6,  tn);
  TST_PERCENT_N( 7,  zn);
  TST_PERCENT_N( 8,  jn);

  tst_s = s3;
  TST_PERCENT_N( 9,  hhn);
  TST_PERCENT_N(10,  hn);
  TST_PERCENT_N(11,   n);
  TST_PERCENT_N(12,  ln);
  TST_PERCENT_N(13, lln);
  TST_PERCENT_N(14,  tn);
  TST_PERCENT_N(15,  zn);
  TST_PERCENT_N(16,  jn);

  return (TST_B_RET(ret));
}
