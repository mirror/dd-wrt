#include "tst-main.c"

static const char *rf = __FILE__;

static unsigned int lens_fwd[4];
static unsigned int lens_rev[4];
static int ret = 0;

static void tst_chrs(Vstr_base *t1, unsigned int off)
{
  TST_B_TST(ret, off + 1,
            VSTR_SPN_CSTR_CHRS_FWD(t1, 1, t1->len, "0123456789") !=
            lens_fwd[0]);
  TST_B_TST(ret, off + 2,
            VSTR_SPN_CSTR_CHRS_REV(t1, 1, t1->len, "abcd") !=
            lens_rev[3]);
  TST_B_TST(ret, off + 3,
            vstr_spn_cstr_chrs_fwd(t1, 1, t1->len, "0123456789abcd -") !=
            lens_fwd[1]);
  TST_B_TST(ret, off + 4,
            vstr_spn_cstr_chrs_rev(t1, 1, t1->len, "0123456789abcd -") !=
            lens_rev[2] - 2);
  TST_B_TST(ret, off + 5,
            VSTR_SPN_CSTR_CHRS_FWD(t1, 1, t1->len, "0123456789abcd -xyz") !=
            lens_fwd[2]);
  TST_B_TST(ret, off + 6,
            vstr_spn_cstr_chrs_rev(t1, 1, t1->len, "0123456789abcd -!&") !=
            lens_rev[1] - 3);
  TST_B_TST(ret, off + 7,
            vstr_spn_cstr_chrs_fwd(t1, lens_fwd[0] + 1, t1->len - lens_fwd[0],
                                   "abcd ") != 5);
  TST_B_TST(ret, off + 8,
            VSTR_SPN_CSTR_CHRS_REV(t1, lens_fwd[0] + 1, t1->len - lens_fwd[0],
                                   "abcd")  != 4);

  TST_B_TST(ret, off + 9,
            VSTR_SPN_CSTR_CHRS_FWD(t1, 1, t1->len, buf)  != t1->len);
  TST_B_TST(ret, off + 9,
            VSTR_SPN_CSTR_CHRS_REV(t1, 1, t1->len, buf)  != t1->len);
  
  TST_B_TST(ret, off + 10,
            vstr_spn_chrs_fwd(t1, 1, t1->len, NULL, 1)  != 0);
  TST_B_TST(ret, off + 11,
            vstr_spn_chrs_rev(t1, 1, t1->len, NULL, 1)  != 0);
  TST_B_TST(ret, off + 12,
            vstr_spn_chrs_fwd(t1, 1, 0, "a", 1)  != 0);
  TST_B_TST(ret, off + 13,
            vstr_spn_chrs_rev(t1, 1, 0, "a", 1)  != 0);
}

static void tst_non_chrs(Vstr_base *t1, unsigned int off)
{
  char pos_4_chr[1];
  
  TST_B_TST(ret, off + 1,
            VSTR_SPN_CSTR_CHRS_FWD(t1, 1, t1->len, "0123456789") != 4);
  TST_B_TST(ret, off + 2,
            VSTR_SPN_CSTR_CHRS_REV(t1, 1, 12,      "0123456789") != 4);
  TST_B_TST(ret, off + 3,
            vstr_spn_chrs_fwd(t1, 5, t1->len - 4, NULL, 1) != 4);
  TST_B_TST(ret, off + 4,
            vstr_spn_chrs_rev(t1, 1, 8,           NULL, 1) != 4);

  pos_4_chr[0] = vstr_export_chr(t1, 4);
  
  TST_B_TST(ret, off + 5,
            vstr_spn_cstr_chrs_fwd(t1, 4, t1->len - 8, "0123456789") != 1);
  TST_B_TST(ret, off + 5,
            vstr_spn_chrs_fwd(t1, 4, t1->len - 8, pos_4_chr, 1) != 1);
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

  tst_chrs(s1, 0);
  tst_chrs(s3, 9);

  /* make sure it's got a iovec cache */
  vstr_export_iovec_ptr_all(s1, NULL, NULL);
  vstr_export_iovec_ptr_all(s3, NULL, NULL);

  tst_chrs(s1, 0);
  tst_chrs(s3, 9);
  tst_chrs(s4, 18);

  vstr_add_non(s1, 4, 4);
  vstr_add_non(s3, 4, 4);
  vstr_add_non(s4, 4, 4);

  /* won't have a cache again... */
  ASSERT(!s1->iovec_upto_date);
  ASSERT(!s3->iovec_upto_date);

  tst_non_chrs(s1, 24);
  tst_non_chrs(s3, 24);

  /* make sure it's got a iovec cache */
  vstr_export_iovec_ptr_all(s1, NULL, NULL);
  vstr_export_iovec_ptr_all(s3, NULL, NULL);

  tst_non_chrs(s1, 24);
  tst_non_chrs(s3, 24);
  tst_non_chrs(s4, 24);

  return (TST_B_RET(ret));
}
