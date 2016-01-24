#include "tst-main.c"

static const char *rf = __FILE__;

static unsigned int lens_fwd[4];
static int ret = 0;

static void tst_srch_buf(Vstr_base *t1, unsigned int off)
{
  TST_B_TST(ret, off + 1,
            VSTR_SRCH_CSTR_BUF_FWD(t1, 1, t1->len, "abcd") != lens_fwd[0]);
  TST_B_TST(ret, off + 2,
            VSTR_SRCH_CSTR_BUF_REV(t1, 1, t1->len, "abcd") != lens_fwd[3]);
  TST_B_TST(ret, off + 3,
            vstr_srch_cstr_buf_fwd(t1, 1, t1->len, "xyz ") != lens_fwd[1]);
  TST_B_TST(ret, off + 4,
            vstr_srch_cstr_buf_rev(t1, 1, t1->len, "xyz ") != lens_fwd[1]);
  TST_B_TST(ret, off + 5,
            VSTR_SRCH_CSTR_BUF_FWD(t1, 1, t1->len, "!")  != lens_fwd[2]);
  TST_B_TST(ret, off + 6,
            VSTR_SRCH_CSTR_BUF_REV(t1, 1, t1->len, "!")  != lens_fwd[2]);
  TST_B_TST(ret, off + 7,
            VSTR_SRCH_CSTR_BUF_FWD(t1, lens_fwd[0], t1->len - (lens_fwd[0] - 1),
                                   "abcd ") != lens_fwd[0]);
  TST_B_TST(ret, off + 8,
            VSTR_SRCH_CSTR_BUF_REV(t1, lens_fwd[0], t1->len - (lens_fwd[0] - 1),
                                   "abcd ") != lens_fwd[0]);
  
  assert(!VSTR_SRCH_CSTR_BUF_REV(t1, 1, t1->len, " abcd"));
  assert(!VSTR_SRCH_CSTR_BUF_REV(t1, 1, t1->len, "bcde"));

  ASSERT(!vstr_srch_buf_fwd(t1, 1, 1, "ab", 2));
  ASSERT(!vstr_srch_buf_rev(t1, 1, 1, "ab", 2));

  ASSERT(!vstr_srch_buf_fwd(t1, 1, 0, "a", 1));
  ASSERT(!vstr_srch_buf_rev(t1, 1, 0, "a", 1));

  ASSERT(vstr_srch_buf_fwd(t1, 3, 2, "a", 0) == 3);
  ASSERT(vstr_srch_buf_rev(t1, 3, 2, "a", 0) == 4);
}

static void tst_srch_non_buf(Vstr_base *t1)
{
  TST_B_TST(ret, 25,
            vstr_srch_buf_fwd(t1, 1, t1->len, NULL, 4) != 8);

  TST_B_TST(ret, 26,
            vstr_srch_buf_rev(t1, 1, t1->len, NULL, 4) != 16);

  TST_B_TST(ret, 27,
            vstr_srch_buf_fwd(t1, 1, t1->len, NULL, 12) != 8);

  TST_B_TST(ret, 28,
            vstr_srch_buf_rev(t1, 1, t1->len, NULL, 12) != 8);

  TST_B_TST(ret, 29,
            vstr_srch_buf_fwd(t1, 1, t1->len, NULL, 13) != 0);

  TST_B_TST(ret, 29,
            vstr_srch_buf_rev(t1, 1, t1->len, NULL, 13) != 0);

  TST_B_TST(ret, 29,
            vstr_srch_buf_rev(t1, 4, 8, "", 0) != 11);
  
  {
    unsigned int scan = 0;
    while (++scan <= t1->len)
    {
      ASSERT(vstr_srch_buf_fwd(t1, scan, 1, "", 0) == scan);
      ASSERT(vstr_srch_buf_rev(t1, scan, 1, "", 0) == scan);
    }
  }
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
  VSTR_ADD_CSTR_BUF(s1, s1->len, buf); /* norm */
  VSTR_ADD_CSTR_BUF(s3, s3->len, buf); /* small */
  VSTR_ADD_CSTR_BUF(s4, s4->len, buf); /* no iovec */

  ++lens_fwd[0]; /* convert to position of char after %n */
  ++lens_fwd[1];
  ++lens_fwd[2];
  ++lens_fwd[3];

  tst_srch_buf(s1, 0);
  tst_srch_buf(s3, 8);

  /* make sure it's got a iovec cache */
  vstr_export_iovec_ptr_all(s1, NULL, NULL);
  vstr_export_iovec_ptr_all(s3, NULL, NULL);

  tst_srch_buf(s1, 0);
  tst_srch_buf(s3, 8);
  tst_srch_buf(s4, 16);

  TST_B_TST(ret, 29,
            vstr_srch_buf_rev(s1, 4, 8, NULL, 1) != 0);

  vstr_sub_non(s1, 2, 6, 6);
  vstr_sub_non(s1, lens_fwd[3] - 7, 6, 6);
  
  vstr_export_iovec_ptr_all(s1, NULL, NULL);
  vstr_export_iovec_ptr_all(s3, NULL, NULL);
  
  tst_srch_buf(s1, 0);
  tst_srch_buf(s3, 8);
  tst_srch_buf(s4, 16);

  vstr_sub_rep_chr(s1, 2, 6, 'x', 6);
  vstr_sub_rep_chr(s1, lens_fwd[3] - 7, 6, 'x', 6);
  
  vstr_add_non(s1, 7, 12);
  {
    unsigned int pre_num = s3->num;

    vstr_add_non(s3, 7,     3); /* merges on append, not prepend */
    vstr_add_non(s3, 7 + 3, 3);
    vstr_add_non(s3, 7, 6);
    ASSERT(pre_num == (s3->num - 3)); /* split + 2 _NON */
    vstr_add_ptr(s3, 7 + 3, "a", 1);
    vstr_del(s3, 7 + 4, 1);
    ASSERT(pre_num == (s3->num - 4)); /* split + 3 _NON */
  }

  vstr_add_non(s4, 7, 12);

  /* won't have a cache again... */
  ASSERT(!s1->iovec_upto_date);
  ASSERT(!s3->iovec_upto_date);

  tst_srch_non_buf(s1);
  tst_srch_non_buf(s3);

  /* make sure it's got a iovec cache */
  vstr_export_iovec_ptr_all(s1, NULL, NULL);
  vstr_export_iovec_ptr_all(s3, NULL, NULL);

  tst_srch_non_buf(s1);
  tst_srch_non_buf(s3);
  tst_srch_non_buf(s4);

  vstr_del(s1, 1, s1->len);
  vstr_add_cstr_buf(s1, s1->len, "ab");
  vstr_add_cstr_ptr(s1, s1->len, "cd");
  vstr_add_cstr_buf(s1, s1->len, "ef");
  vstr_add_cstr_ptr(s1, s1->len, "gh");
  vstr_add_cstr_buf(s1, s1->len, "ij");

  ASSERT(!vstr_srch_cstr_buf_fwd(s1, 1, s1->len, "Xabcdefghi"));
  ASSERT(!vstr_srch_cstr_buf_rev(s1, 1, s1->len, "Xabcdefghi"));
  ASSERT(!vstr_srch_cstr_buf_fwd(s1, 1, s1->len, "abcdefghiX"));
  ASSERT(!vstr_srch_cstr_buf_rev(s1, 1, s1->len, "abcdefghiX"));

  vstr_export_iovec_ptr_all(s1, NULL, NULL);
  
  ASSERT(!vstr_srch_cstr_buf_fwd(s1, 1, s1->len, "Xabcdefghi"));
  ASSERT(!vstr_srch_cstr_buf_rev(s1, 1, s1->len, "Xabcdefghi"));
  ASSERT(!vstr_srch_cstr_buf_fwd(s1, 1, s1->len, "abcdefghiX"));
  ASSERT(!vstr_srch_cstr_buf_rev(s1, 1, s1->len, "abcdefghiX"));

  vstr_add_non(s1, 2, 1);
  vstr_add_non(s1, s1->len - 2, 1);
  
  ASSERT(!vstr_srch_cstr_buf_fwd(s1, 1, s1->len, "abcdefghi"));
  ASSERT(!vstr_srch_cstr_buf_rev(s1, 1, s1->len, "abcdefghi"));
  
  vstr_export_iovec_ptr_all(s1, NULL, NULL);
  
  ASSERT(!vstr_srch_cstr_buf_fwd(s1, 1, s1->len, "abcdefghi"));
  ASSERT(!vstr_srch_cstr_buf_rev(s1, 1, s1->len, "abcdefghi"));
  
  return (TST_B_RET(ret));
}
