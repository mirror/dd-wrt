#include "tst-main.c"

static const char *rf = __FILE__;

#define CSTREQ(x, y) (!strcmp(x, y))

int tst(void)
{
  int ret = 0;
  long double ldz = 0.0;

#ifdef USE_RESTRICTED_HEADERS /* sucky host sprintf() implementions */
  return (EXIT_FAILED_OK);
#endif

  sprintf(buf, "%8La", ldz);
  if (!CSTREQ(buf, "  0x0p+0")) /* debian woody has a broken glibc */
    return (EXIT_FAILED_OK);
  
  sprintf(buf,        "%LE %Le %LF %Lf %LG %Lg %LA %La %#La %#20.4La %#Lg %#.Lg %.20Lg %.Lf %#.Lf %-20.8Le",
          ldz, ldz, ldz, ldz,
          ldz, ldz, ldz, ldz,
          ldz, ldz, ldz, ldz,
          ldz, ldz, ldz, ldz);
  vstr_add_fmt(s1, 0, "%LE %Le %LF %Lf %LG %Lg %LA %La %#La %#20.4La %#Lg %#.Lg %.20Lg %.Lf %#.Lf %-20.8Le",
          ldz, ldz, ldz, ldz,
          ldz, ldz, ldz, ldz,
          ldz, ldz, ldz, ldz,
          ldz, ldz, ldz, ldz);

  TST_B_TST(ret, 1, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
  vstr_del(s1, 1, s1->len);

#ifdef FMT_DBL_none
  if (ret) return (TST_B_RET(ret));
  return (EXIT_FAILED_OK);
#endif

  sprintf(buf, "%La %LA",
          -LDBL_MAX, LDBL_MAX);
  vstr_add_fmt(s1, 0, "%La %LA",
               -LDBL_MAX, LDBL_MAX);

  TST_B_TST(ret, 2, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
  vstr_del(s1, 1, s1->len);

  sprintf(buf,        "%Le %LE %Lg %LG %La %LA",
          -LDBL_MAX, LDBL_MAX, /* %Lf doesn't work atm. */
          -LDBL_MAX, LDBL_MAX, -LDBL_MAX, LDBL_MAX);
  vstr_add_fmt(s1, 0, "%Le %LE %Lg %LG %La %LA",
               -LDBL_MAX, LDBL_MAX, /* %Lf doesn't work atm. */
               -LDBL_MAX, LDBL_MAX, -LDBL_MAX, LDBL_MAX);

  TST_B_TST(ret, 3, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
  vstr_del(s1, 1, s1->len);

#if 0 /* used to work to test edge cases */
  /* this test the sign */
  ldz = ((union __convert_long_double) {__convert_long_double_i: {0x00000000, 0, 0x007ffe, 0x0}}).__convert_long_double_d;
  sprintf(buf,        "%Le %LE %Lf %LF %Lg %LG %La %LA",
          ldz, ldz, ldz, ldz,
          ldz, ldz, ldz, ldz);
  vstr_add_fmt(s1, 0, "%Le %LE %Lf %LF %Lg %LG %La %LA",
          ldz, ldz, ldz, ldz,
          ldz, ldz, ldz, ldz);

  TST_B_TST(ret, 4, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
  vstr_del(s1, 1, s1->len);

  ldz = ((union __convert_long_double) {__convert_long_double_i: {0xffffffff, 0xffffffff, 0x107e137, 0x0}}).__convert_long_double_d;
  sprintf(buf,        "%Le %LE %Lf %LF %Lg %LG %La %LA",
          ldz, ldz, ldz, ldz,
          ldz, ldz, ldz, ldz);
  vstr_add_fmt(s1, 0, "%Le %LE %Lf %LF %Lg %LG %La %LA",
          ldz, ldz, ldz, ldz,
          ldz, ldz, ldz, ldz);

  TST_B_TST(ret, 4, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
  vstr_del(s1, 1, s1->len);
#endif

  sprintf(buf,        "%Le %LE %Lf %LF %Lg %LG %La %LA",
          -LDBL_MAX, LDBL_MAX, -LDBL_MAX, LDBL_MAX,
          -LDBL_MAX, LDBL_MAX, -LDBL_MAX, LDBL_MAX);
  vstr_add_fmt(s1, 0, "%Le %LE %Lf %LF %Lg %LG %La %LA",
               -LDBL_MAX, LDBL_MAX, -LDBL_MAX, LDBL_MAX,
               -LDBL_MAX, LDBL_MAX, -LDBL_MAX, LDBL_MAX);

  TST_B_TST(ret, 5, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
  vstr_del(s1, 1, s1->len);

  sprintf(buf,        "%Le %LE %'Lf %'LF %'Lg %'LG %La %LA",
          -LDBL_MAX, LDBL_MAX, -LDBL_MAX, LDBL_MAX,
          -LDBL_MAX, LDBL_MAX, -LDBL_MAX, LDBL_MAX);
  vstr_add_fmt(s2, 0, "%Le %LE %'Lf %'LF %'Lg %'LG %La %LA",
               -LDBL_MAX, LDBL_MAX, -LDBL_MAX, LDBL_MAX,
               -LDBL_MAX, LDBL_MAX, -LDBL_MAX, LDBL_MAX);
  TST_B_TST(ret, 6, !VSTR_CMP_CSTR_EQ(s2, 1, s2->len, buf));

#ifdef NAN
  vstr_del(s1, 1, s1->len);
  sprintf(buf,        "%8LE %8Le %8LF %8Lf %8LG %8Lg %8LA %8La",
          (long double)NAN, (long double)NAN, (long double)NAN, (long double)NAN,
          (long double)NAN, (long double)NAN, (long double)NAN, (long double)NAN);
  vstr_add_fmt(s1, 0, "%8LE %8Le %8LF %8Lf %8LG %8Lg %8LA %8La",
          (long double)NAN, (long double)NAN, (long double)NAN, (long double)NAN,
          (long double)NAN, (long double)NAN, (long double)NAN, (long double)NAN);
  TST_B_TST(ret, 7, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
#endif
  
#ifdef INFINITY
  vstr_del(s1, 1, s1->len);
  sprintf(buf,        "%LE %+Le %-LF % Lf %-8LG %Lg %LA %La",
          (long double)-INFINITY, (long double)INFINITY, (long double)-INFINITY, (long double)INFINITY,
          (long double)-INFINITY, (long double)INFINITY, (long double)-INFINITY, (long double)INFINITY);
  vstr_add_fmt(s1, 0, "%LE %+Le %-LF % Lf %-8LG %Lg %LA %La",
          (long double)-INFINITY, (long double)INFINITY, (long double)-INFINITY, (long double)INFINITY,
          (long double)-INFINITY, (long double)INFINITY, (long double)-INFINITY, (long double)INFINITY);
  TST_B_TST(ret, 8, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
#endif
  
  {
    long double f = 1;
    long double d = 1;
    unsigned int count = 0;

    while (count++ < 512)
    {
      f += count;
      d /= count;
      
      vstr_del(s1, 1, s1->len);
      sprintf(buf,        "%LE %Le %LF %Lf %LG %Lg %LA %La",
              f, f, f, f, f, f, f, f);
      vstr_add_fmt(s1, 0, "%LE %Le %LF %Lf %LG %Lg %LA %La",
              f, f, f, f, f, f, f, f);
      TST_B_TST(ret, 11, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));

      vstr_del(s1, 1, s1->len);
      sprintf(buf,        "%LE %Le %LF %Lf %LG %Lg %LA %La",
              d, d, d, d, d, d, d, d);
      vstr_add_fmt(s1, 0, "%LE %Le %LF %Lf %LG %Lg %LA %La",
              d, d, d, d, d, d, d, d);
      TST_B_TST(ret, 12, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));

      
      vstr_del(s1, 1, s1->len);
      sprintf(buf,        "%LE %Le %LF %Lf %LG %Lg %LA %La",
              -f, -f, -f, -f, -f, -f, -f, -f);
      vstr_add_fmt(s1, 0, "%LE %Le %LF %Lf %LG %Lg %LA %La",
              -f, -f, -f, -f, -f, -f, -f, -f);
      TST_B_TST(ret, 13, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));

      vstr_del(s1, 1, s1->len);
      sprintf(buf,        "%LE %Le %LF %Lf %LG %Lg %LA %La",
              -d, -d, -d, -d, -d, -d, -d, -d);
      vstr_add_fmt(s1, 0, "%LE %Le %LF %Lf %LG %Lg %LA %La",
              -d, -d, -d, -d, -d, -d, -d, -d);
      TST_B_TST(ret, 14, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
    }
  }
  
  return (TST_B_RET(ret));
}
