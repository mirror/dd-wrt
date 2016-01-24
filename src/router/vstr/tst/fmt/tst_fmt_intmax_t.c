#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;

  sprintf(buf, "%jd%ju", INTMAX_MAX, UINTMAX_MAX);
  if (!strcmp(buf, "jdju"))
    return (EXIT_FAILED_OK); /* sucky host sprintf() implementions */

  sprintf(buf, "%jd %jd %ju %ju",
          INTMAX_MAX, INTMAX_MIN, (intmax_t)0, UINTMAX_MAX);
  vstr_add_fmt(s1, 0, "%jd %jd %ju %ju",
               INTMAX_MAX, INTMAX_MIN, (intmax_t)0, UINTMAX_MAX);
  TST_B_TST(ret, 1, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));

  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%'jd", (intmax_t)1000);

  TST_B_TST(ret, 2, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "1000"));

#ifndef USE_RESTRICTED_HEADERS
  sprintf(buf, "%'jd %'jd %'ju %'ju",
          INTMAX_MAX, INTMAX_MIN, (intmax_t)0, UINTMAX_MAX);
  vstr_add_fmt(s2, 0, "%'jd %'jd %'ju %'ju",
               INTMAX_MAX, INTMAX_MIN, (intmax_t)0, UINTMAX_MAX);
  TST_B_TST(ret, 3, !VSTR_CMP_CSTR_EQ(s2, 1, s2->len, buf));
#endif
  
  return (TST_B_RET(ret));
}
