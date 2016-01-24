#include "tst-main.c"

static const char *rf = __FILE__;

static unsigned int lens_fwd[4];
static int ret = 0;

static void tst_srch_vstr(Vstr_base *t1, unsigned int off)
{
  Vstr_base *sx = vstr_make_base(t1->conf);

  ASSERT(sx);
  
  TST_B_TST(ret, off + 1,
            vstr_srch_case_vstr_fwd(t1, 1, t1->len, s2, 1, 4) != lens_fwd[0]);
  TST_B_TST(ret, off + 2,
            vstr_srch_case_vstr_rev(t1, 1, t1->len, s2, 1, 4) != lens_fwd[3]);
  TST_B_TST(ret, off + 3,
            vstr_srch_case_vstr_fwd(t1, 1, t1->len, s2, 6, 4) != lens_fwd[1]);
  TST_B_TST(ret, off + 4,
            vstr_srch_case_vstr_rev(t1, 1, t1->len, s2, 6, 4) != lens_fwd[1]);
  TST_B_TST(ret, off + 5,
            vstr_srch_case_vstr_fwd(t1, 1, t1->len, s2, 10, 3)  != lens_fwd[2]);
  TST_B_TST(ret, off + 6,
            vstr_srch_case_vstr_rev(t1, 1, t1->len, s2, 10, 3)  != lens_fwd[2]);
  TST_B_TST(ret, off + 7,
            vstr_srch_case_vstr_fwd(t1, lens_fwd[0],
                                    t1->len - (lens_fwd[0] - 1),
                                    s2, 1, 5) != lens_fwd[0]);
  TST_B_TST(ret, off + 8,
            vstr_srch_case_vstr_rev(t1, lens_fwd[0],
                                    t1->len - (lens_fwd[0] - 1),
                                    s2, 1, 5) != lens_fwd[0]);
  
  ASSERT(!vstr_srch_case_vstr_fwd(t1, 1, 1, s2, 1, 2));
  ASSERT(!vstr_srch_case_vstr_rev(t1, 1, 1, s2, 1, 2));

  ASSERT(!vstr_srch_case_vstr_fwd(t1, 1, 0, s2, 1, 1));
  ASSERT(!vstr_srch_case_vstr_rev(t1, 1, 0, s2, 1, 1));
  
  ASSERT(!vstr_srch_case_vstr_fwd(t1, 4, t1->len - 3, s2, 1, s2->len / 2));
  ASSERT(!vstr_srch_case_vstr_rev(t1, 4, t1->len - 3, s2, 1, s2->len / 2));

  vstr_add_rep_chr(sx, 0, 'C', 4);
  
  ASSERT(!vstr_srch_case_vstr_fwd(t1, 1, t1->len, sx, 1, sx->len));
  ASSERT(!vstr_srch_case_vstr_rev(t1, 1, t1->len, sx, 1, sx->len));

  vstr_add_non(sx, 0, 1);
  
  ASSERT(!vstr_srch_case_vstr_fwd(t1, 1, t1->len, sx, 1, sx->len));
  ASSERT(!vstr_srch_case_vstr_rev(t1, 1, t1->len, sx, 1, sx->len));

  vstr_free_base(sx);
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

  VSTR_ADD_CSTR_BUF(s2, 0, "AbCd XyZ !& ");
  /*                        123456789 12   */

  tst_srch_vstr(s1, 0);
  tst_srch_vstr(s3, 8);

  VSTR_SUB_CSTR_BUF(s2, 1, 8, "aBcd xYz");

  tst_srch_vstr(s1, 0);
  tst_srch_vstr(s3, 8);

  VSTR_SUB_CSTR_BUF(s2, 1, 8, "AbCd XyZ");

  /* make sure it's got a iovec cache */
  vstr_export_iovec_ptr_all(s1, NULL, NULL);
  vstr_export_iovec_ptr_all(s3, NULL, NULL);

  tst_srch_vstr(s1, 0);
  tst_srch_vstr(s3, 8);
  tst_srch_vstr(s4, 16);

  VSTR_SUB_CSTR_BUF(s2, 1, 8, "aBcd xYz");

  tst_srch_vstr(s1, 0);
  tst_srch_vstr(s3, 8);
  tst_srch_vstr(s4, 16);

  return (TST_B_RET(ret));
}
