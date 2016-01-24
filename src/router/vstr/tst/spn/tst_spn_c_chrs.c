#include "tst-main.c"

static const char *rf = __FILE__;

static unsigned int lens_fwd[4];
static unsigned int lens_rev[4];
static int ret = 0;

static void tst_c_chrs(Vstr_base *t1, unsigned int off)
{
  TST_B_TST(ret, off + 1,
            VSTR_CSPN_CSTR_CHRS_FWD(t1, 1, t1->len, "x") !=
            lens_fwd[1]);
  TST_B_TST(ret, off + 2,
            VSTR_CSPN_CSTR_CHRS_REV(t1, 1, t1->len, "x") !=
            lens_rev[1] - 1);
  TST_B_TST(ret, off + 3,
            vstr_cspn_cstr_chrs_fwd(t1, 1, t1->len, "!") !=
            lens_fwd[2]);
  TST_B_TST(ret, off + 3,
            vstr_cspn_cstr_chrs_rev(t1, 1, t1->len, "!x") !=
            lens_rev[2] - 1);
  TST_B_TST(ret, off + 4,
            VSTR_CSPN_CSTR_CHRS_FWD(t1, 1, t1->len, "0123456789") != 0);
  TST_B_TST(ret, off + 5,
            VSTR_CSPN_CSTR_CHRS_REV(t1, 1, t1->len, "0123456789") != 4);
  TST_B_TST(ret, off + 6,
            VSTR_CSPN_CSTR_CHRS_FWD(t1, 1, lens_fwd[3], "abcd") !=
            lens_fwd[0]);
  TST_B_TST(ret, off + 8,
            VSTR_CSPN_CSTR_CHRS_REV(t1, 1, lens_fwd[3], "abcd") !=
            (lens_fwd[3] - lens_fwd[0]) - 4);
  TST_B_TST(ret, off + 9,
            VSTR_CSPN_CSTR_CHRS_FWD(t1, lens_fwd[1] + 1, t1->len - lens_fwd[1],
                                    "0123456789") != 4);
  TST_B_TST(ret, off + 10,
            VSTR_CSPN_CSTR_CHRS_REV(t1, lens_fwd[1] + 1, t1->len - lens_fwd[1],
                                    "0123456789") != 4);

  TST_B_TST(ret, off + 11,
            VSTR_CSPN_CSTR_CHRS_FWD(t1, 1, t1->len, "Z") != t1->len);
  TST_B_TST(ret, off + 12,
            VSTR_CSPN_CSTR_CHRS_REV(t1, 1, t1->len, "Z") != t1->len);

  TST_B_TST(ret, off + 13,
            VSTR_CSPN_CSTR_CHRS_FWD(t1, 1, t1->len, "ZZ") != t1->len);
  TST_B_TST(ret, off + 14,
            VSTR_CSPN_CSTR_CHRS_REV(t1, 1, t1->len, "ZZ") != t1->len);

  TST_B_TST(ret, off + 15,
            vstr_cspn_chrs_fwd(t1, 1, t1->len, NULL, 1)  != t1->len);
  TST_B_TST(ret, off + 16,
            vstr_cspn_chrs_rev(t1, 1, t1->len, NULL, 1)  != t1->len);
  TST_B_TST(ret, off + 17,
            vstr_cspn_chrs_fwd(t1, 1, 0, "a", 1)  != 0);
  TST_B_TST(ret, off + 18,
            vstr_cspn_chrs_rev(t1, 1, 0, "a", 1)  != 0);
  TST_B_TST(ret, off + 19,
            vstr_cspn_chrs_fwd(t1, 1, 0, "aa", 1)  != 0);
  TST_B_TST(ret, off + 20,
            vstr_cspn_chrs_rev(t1, 1, 0, "aa", 1)  != 0);
}

static void tst_c_non_chrs(Vstr_base *t1)
{
  TST_B_TST(ret, 1,
            VSTR_CSPN_CSTR_CHRS_FWD(t1, 5, t1->len - 4, "0123456789") != 4);
  TST_B_TST(ret, 2,
            VSTR_CSPN_CSTR_CHRS_REV(t1, 1, 8,           "0123456789") != 4);
  TST_B_TST(ret, 3,
            vstr_cspn_chrs_fwd(t1, 1, t1->len, NULL, 1) != 4);
  TST_B_TST(ret, 4,
            vstr_cspn_chrs_rev(t1, 1, 12,      NULL, 1) != 4);
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

  lens_rev[0] = s3->len - lens_fwd[0];
  lens_rev[1] = s3->len - lens_fwd[1];
  lens_rev[2] = s3->len - lens_fwd[2];
  lens_rev[3] = s3->len - lens_fwd[3];

  tst_c_chrs(s1, 0); /* overlap */
  tst_c_chrs(s3, 10);

  /* make sure it's got a iovec cache */
  vstr_export_iovec_ptr_all(s1, NULL, NULL);
  vstr_export_iovec_ptr_all(s3, NULL, NULL);

  tst_c_chrs(s1, 0); /* overlap */
  tst_c_chrs(s3, 10);
  tst_c_chrs(s4, 20);

  vstr_add_non(s1, 4, 4);
  vstr_add_non(s3, 4, 4);
  vstr_add_non(s4, 4, 4);

  /* won't have a cache again... */
  ASSERT(!s1->iovec_upto_date);
  ASSERT(!s3->iovec_upto_date);

  tst_c_non_chrs(s1);
  tst_c_non_chrs(s3);

  /* make sure it's got a iovec cache */
  vstr_export_iovec_ptr_all(s1, NULL, NULL);
  vstr_export_iovec_ptr_all(s3, NULL, NULL);

  tst_c_non_chrs(s1);
  tst_c_non_chrs(s3);
  tst_c_non_chrs(s4);

  return (TST_B_RET(ret));
}
