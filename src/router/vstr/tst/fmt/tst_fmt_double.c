#include "tst-main.c"

static const char *rf = __FILE__;

#define CSTREQ(x, y) (!strcmp(x, y))

int tst(void)
{
  int ret = 0;

#ifdef USE_RESTRICTED_HEADERS /* sucky host sprintf() implementions */
  return (EXIT_FAILED_OK);
#endif

  sprintf(buf, "%8a", 0.0);
  if (!CSTREQ(buf, "  0x0p+0")) /* debian woody has a broken glibc */
    return (EXIT_FAILED_OK);
  
  vstr_del(s1, 1, s1->len);
  sprintf(buf,        "%E %e %F %f %G %g %A %8a %#a %#20.4a %#g %#.g %.20g %.f %#.f %-20.8e",
          0.0, 0.0, 0.0, 0.0,
          0.0, 0.0, 0.0, 0.0,
          0.0, 0.0, 0.0, 0.0,
          0.0, 0.0, 0.0, 0.0);
  vstr_add_fmt(s1, 0, "%E %e %F %f %G %g %A %8a %#a %#20.4a %#g %#.g %.20g %.f %#.f %-20.8e",
               0.0, 0.0, 0.0, 0.0,
               0.0, 0.0, 0.0, 0.0,
               0.0, 0.0, 0.0, 0.0,
               0.0, 0.0, 0.0, 0.0);

  TST_B_TST(ret, 1, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));

#ifdef FMT_DBL_none
  if (ret) return (TST_B_RET(ret));
  return (EXIT_FAILED_OK);
#endif

  vstr_del(s1, 1, s1->len);
  sprintf(buf, "%a %A",
          -DBL_MAX, DBL_MAX);
  vstr_add_fmt(s1, 0, "%a %A",
               -DBL_MAX, DBL_MAX);

  TST_B_TST(ret, 2, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));

  vstr_del(s1, 1, s1->len);
  sprintf(buf,        "%E %e %F %f %G %g %A %a",
          -DBL_MAX, DBL_MAX, -DBL_MAX, DBL_MAX,
          -DBL_MAX, DBL_MAX, -DBL_MAX, DBL_MAX);
  vstr_add_fmt(s1, 0, "%E %e %F %f %G %g %A %a",
               -DBL_MAX, DBL_MAX, -DBL_MAX, DBL_MAX,
               -DBL_MAX, DBL_MAX, -DBL_MAX, DBL_MAX);

  TST_B_TST(ret, 3, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));

  vstr_del(s1, 1, s1->len);
  sprintf(buf,        "%'F %'f %'G %'g",
          -DBL_MAX, DBL_MAX, -DBL_MAX, DBL_MAX);
  vstr_add_fmt(s2, 0, "%'F %'f %'G %'g",
               -DBL_MAX, DBL_MAX, -DBL_MAX, DBL_MAX);
  TST_B_TST(ret, 4, !VSTR_CMP_CSTR_EQ(s2, 1, s2->len, buf));

#ifdef NAN
  vstr_del(s1, 1, s1->len);
  sprintf(buf,        "%E %e %F %f %G %g %A %a",
          NAN, NAN, NAN, NAN,
          NAN, NAN, NAN, NAN);
  vstr_add_fmt(s1, 0, "%E %e %F %f %G %g %A %a",
          NAN, NAN, NAN, NAN,
          NAN, NAN, NAN, NAN);
  TST_B_TST(ret, 5, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
#endif
  
#ifdef INFINITY
  vstr_del(s1, 1, s1->len);
  sprintf(buf,        "%E %e %F %f %G %g %A %a",
          -INFINITY, INFINITY, -INFINITY, INFINITY,
          -INFINITY, INFINITY, -INFINITY, INFINITY);
  vstr_add_fmt(s1, 0, "%E %e %F %f %G %g %A %a",
          -INFINITY, INFINITY, -INFINITY, INFINITY,
          -INFINITY, INFINITY, -INFINITY, INFINITY);
  TST_B_TST(ret, 6, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
#endif
  
  {
    float f = 1;
    double d = 1;
    unsigned int count = 0;

    while (count++ < 512)
    {
      int mfail_count = 0;
      
      f += count;
      d /= count;
      
      vstr_del(s1, 1, s1->len);
      sprintf(buf,        "%E %12e %F %0f %G %g %A %a",
              f, f, f, f, f, f, f, f);
      mfail_count = 0;
      do
      {
        vstr_free_spare_nodes(s2->conf, VSTR_TYPE_NODE_BUF, 1000);
        tst_mfail_num(++mfail_count);
      } while (!vstr_add_fmt(s1, 0, "%E %12e %F %0f %G %g %A %a",
                             f, f, f, f, f, f, f, f));
      tst_mfail_num(0);
      TST_B_TST(ret, 11, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));

      vstr_del(s2, 1, s2->len);
      sprintf(buf,        "%E %e %'12F %'f %'G %'g %A %a",
              d, d, d, d, d, d, d, d);
      mfail_count = 0;
      do
      {
        vstr_free_spare_nodes(s2->conf, VSTR_TYPE_NODE_BUF, 1000);
        tst_mfail_num(++mfail_count);
      } while (!vstr_add_fmt(s2, 0, "%E %e %'12F %'f %'G %'g %A %a",
                             d, d, d, d, d, d, d, d));
      tst_mfail_num(0);
      TST_B_TST(ret, 12, !VSTR_CMP_CSTR_EQ(s2, 1, s2->len, buf));

      
      vstr_del(s1, 1, s1->len);
      sprintf(buf,        "%E %e %F %12f %G %g %A %a",
              -f, -f, -f, -f, -f, -f, -f, -f);
      mfail_count = 0;
      do
      {
        vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 1000);
        tst_mfail_num(++mfail_count);
      } while (!vstr_add_fmt(s1, 0, "%E %e %F %12f %G %g %A %a",
                             -f, -f, -f, -f, -f, -f, -f, -f));
      tst_mfail_num(0);
      TST_B_TST(ret, 13, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
      
      vstr_del(s1, 1, s1->len);
      sprintf(buf,        "%E %e %0F %0f %G %g %12A %a",
              -d, -d, -d, -d, -d, -d, -d, -d);
      mfail_count = 0;
      do
      {
        vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 1000);
        tst_mfail_num(++mfail_count);
      } while (!vstr_add_fmt(s1, 0, "%E %e %0F %0f %G %g %12A %a",
                             -d, -d, -d, -d, -d, -d, -d, -d));
      tst_mfail_num(0);
      TST_B_TST(ret, 14, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
    }
  }
  
  return (TST_B_RET(ret));
}
