#include "tst-main.c"

static const char *rf = __FILE__;

static unsigned int lens_fwd[4];
static int ret = 0;

static void tst_srch_vstr(Vstr_base *t1, unsigned int off)
{
  TST_B_TST(ret, off + 1,
            vstr_srch_vstr_fwd(t1, 1, t1->len, s2, 1, 4) != lens_fwd[0]);
  TST_B_TST(ret, off + 2,
            vstr_srch_vstr_rev(t1, 1, t1->len, s2, 1, 4) != lens_fwd[3]);
  TST_B_TST(ret, off + 3,
            vstr_srch_vstr_fwd(t1, 1, t1->len, s2, 6, 4) != lens_fwd[1]);
  TST_B_TST(ret, off + 4,
            vstr_srch_vstr_rev(t1, 1, t1->len, s2, 6, 4) != lens_fwd[1]);
  TST_B_TST(ret, off + 5,
            vstr_srch_vstr_fwd(t1, 1, t1->len, s2, 10, 3)  != lens_fwd[2]);
  TST_B_TST(ret, off + 6,
            vstr_srch_vstr_rev(t1, 1, t1->len, s2, 10, 3)  != lens_fwd[2]);
  TST_B_TST(ret, off + 7,
            vstr_srch_vstr_fwd(t1, lens_fwd[0], t1->len - (lens_fwd[0] - 1),
                               s2, 1, 5) != lens_fwd[0]);
  TST_B_TST(ret, off + 8,
            vstr_srch_vstr_rev(t1, lens_fwd[0], t1->len - (lens_fwd[0] - 1),
                               s2, 1, 5) != lens_fwd[0]);

  ASSERT(!vstr_srch_vstr_fwd(t1, 1, 1, s2, 1, 2));
  ASSERT(!vstr_srch_vstr_rev(t1, 1, 1, s2, 1, 2));

  ASSERT(!vstr_srch_vstr_fwd(t1, 1, 0, s2, 1, 1));
  ASSERT(!vstr_srch_vstr_rev(t1, 1, 0, s2, 1, 1));
  
  ASSERT(!vstr_srch_vstr_fwd(t1, 4, t1->len - 3, s2, 1, s2->len / 2));
  ASSERT(!vstr_srch_vstr_rev(t1, 4, t1->len - 3, s2, 1, s2->len / 2));
}

int tst(void)
{
#ifdef USE_RESTRICTED_HEADERS /* %n doesn't work in dietlibc */
  return (EXIT_FAILED_OK);
#endif

  sprintf(buf, "%d%nabcd %d%nxyz %u%n!& %u%nabcd",
          INT_MAX,  lens_fwd + 0,
          INT_MIN,  lens_fwd + 1,
          0,        lens_fwd + 2,
          UINT_MAX, lens_fwd + 3);
  VSTR_ADD_CSTR_BUF(s1, s1->len, buf);
  VSTR_ADD_CSTR_BUF(s3, s3->len, buf);
  VSTR_ADD_CSTR_BUF(s4, s4->len, buf);

  ++lens_fwd[0]; /* convert to position of char after %n */
  ++lens_fwd[1];
  ++lens_fwd[2];
  ++lens_fwd[3];

  VSTR_ADD_CSTR_BUF(s2, 0, "abcd xyz !& ");
  /*                        123456789 12   */

  tst_srch_vstr(s1, 0);
  tst_srch_vstr(s3, 8);

  /* make sure it's got a iovec cache */
  vstr_export_iovec_ptr_all(s1, NULL, NULL);
  vstr_export_iovec_ptr_all(s3, NULL, NULL);

  tst_srch_vstr(s1, 0);
  tst_srch_vstr(s3, 8);
  tst_srch_vstr(s4, 16);

  vstr_del(s1, 1, s1->len);
  vstr_del(s2, 1, s2->len);
  vstr_add_cstr_buf(s1, s1->len, "Xabcd");
  vstr_add_cstr_buf(s2, s2->len, "abcd");

  ASSERT(vstr_srch_vstr_fwd(s1, 1, s1->len, s2, 1, s2->len));
  vstr_add_cstr_ptr(s2, s2->len, "X");
  ASSERT(!vstr_srch_vstr_fwd(s1, 1, s1->len, s2, 1, s2->len));

  return (TST_B_RET(ret));
}
