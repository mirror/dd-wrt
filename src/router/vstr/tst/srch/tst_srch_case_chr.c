#include "tst-main.c"

static const char *rf = __FILE__;

static unsigned int lens_fwd[4];
static int ret = 0;

static void tst_srch_chr(Vstr_base *t1, unsigned int off)
{
  TST_B_TST(ret, off + 1,
            vstr_srch_case_chr_fwd(t1, 1, t1->len, 'A') != lens_fwd[0]);
  TST_B_TST(ret, off + 2,
            vstr_srch_case_chr_rev(t1, 1, t1->len, 'a') != lens_fwd[3]);
  TST_B_TST(ret, off + 3,
            vstr_srch_case_chr_fwd(t1, 1, t1->len, 'x') != lens_fwd[1]);
  TST_B_TST(ret, off + 4,
            vstr_srch_case_chr_rev(t1, 1, t1->len, 'X') != lens_fwd[1]);
  TST_B_TST(ret, off + 5,
            vstr_srch_case_chr_fwd(t1, 1, t1->len, '!') != lens_fwd[2]);
  TST_B_TST(ret, off + 6,
            vstr_srch_case_chr_rev(t1, 1, t1->len, '!') != lens_fwd[2]);
  /* note that t1->len - lens_fwd[0] isn't the rest of the string, it's
   * the rest minus 1 */
  TST_B_TST(ret, off + 7,
            vstr_srch_case_chr_fwd(t1, lens_fwd[0], t1->len - lens_fwd[0],
                                   'A') != lens_fwd[0]);
  TST_B_TST(ret, off + 8,
            vstr_srch_case_chr_rev(t1, lens_fwd[0], t1->len - lens_fwd[0],
                                   'A') != lens_fwd[3]);
  
  ASSERT(!vstr_srch_case_chr_fwd(t1, 1, t1->len, 'J'));
  ASSERT(!vstr_srch_case_chr_rev(t1, 1, t1->len, 'J'));
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

  tst_srch_chr(s1, 0);
  tst_srch_chr(s3, 8);

  /* make sure it's got a iovec cache */
  vstr_export_iovec_ptr_all(s1, NULL, NULL);
  vstr_export_iovec_ptr_all(s3, NULL, NULL);

  tst_srch_chr(s1, 0);
  tst_srch_chr(s3, 8);
  tst_srch_chr(s4, 16);

  return (TST_B_RET(ret));
}
