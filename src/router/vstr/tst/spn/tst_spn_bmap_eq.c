#include "tst-main.c"

static const char *rf = __FILE__;

static unsigned int lens_fwd[4];
static unsigned int lens_rev[4];
static int ret = 0;

static unsigned char bmap_abcd[256];
static unsigned char bmap_dash[256];
static unsigned char bmap_xyz[256];
static unsigned char bmap_and[256];
static unsigned char bmap_spac[256];
static unsigned char bmap_a[256];
static unsigned char bmap_all[256];

static void tst_chrs(Vstr_base *t1, unsigned int off)
{
  TST_B_TST(ret, off + 1,
            vstr_spn_bmap_eq_fwd(t1, 1, t1->len, bmap_abcd, 4) !=
            lens_fwd[0]);
  TST_B_TST(ret, off + 2,
            vstr_spn_bmap_eq_rev(t1, 1, t1->len, bmap_abcd, 3) !=
            lens_rev[3]);
  TST_B_TST(ret, off + 3,
            vstr_spn_bmap_eq_fwd(t1, 1, t1->len, bmap_dash, 1) !=
            lens_fwd[1]);
  TST_B_TST(ret, off + 4,
            vstr_spn_bmap_eq_rev(t1, 1, t1->len, bmap_dash, 1) !=
            lens_rev[2] - 2);
  TST_B_TST(ret, off + 5,
            vstr_spn_bmap_eq_fwd(t1, 1, t1->len, bmap_xyz, 1) !=
            lens_fwd[2]);
  TST_B_TST(ret, off + 6,
            vstr_spn_bmap_eq_rev(t1, 1, t1->len, bmap_and, 1) !=
            lens_rev[1] - 3);
  TST_B_TST(ret, off + 7,
            vstr_spn_bmap_eq_fwd(t1, lens_fwd[0] + 1, t1->len - lens_fwd[0],
                                 bmap_spac, 1) != 5);
  TST_B_TST(ret, off + 8,
            vstr_spn_bmap_eq_rev(t1, lens_fwd[0] + 1, t1->len - lens_fwd[0],
                                 bmap_spac, 1)  != 4);

  TST_B_TST(ret, off + 9,
            vstr_spn_bmap_eq_fwd(t1, 1, t1->len, bmap_all, 1)  != t1->len);
  TST_B_TST(ret, off + 9,
            vstr_spn_bmap_eq_rev(t1, 1, t1->len, bmap_all, 1)  != t1->len);
  
  TST_B_TST(ret, off + 12,
            vstr_spn_bmap_eq_fwd(t1, 1, 0, bmap_a, 1)  != 0);
  TST_B_TST(ret, off + 13,
            vstr_spn_bmap_eq_rev(t1, 1, 0, bmap_a, 1)  != 0);
}

static void tst_non_chrs(Vstr_base *t1, unsigned int off)
{
  unsigned char bmap_pos_4_chr[256] = {0};
  
  TST_B_TST(ret, off + 1,
            vstr_spn_bmap_eq_fwd(t1, 1, t1->len, bmap_abcd, 4) != 4);
  TST_B_TST(ret, off + 2,
            vstr_spn_bmap_eq_rev(t1, 1, 12,      bmap_abcd, 4) != 4);

  bmap_pos_4_chr[(unsigned char)vstr_export_chr(t1, 4)] = 1;
  
  TST_B_TST(ret, off + 5,
            vstr_spn_bmap_eq_fwd(t1, 4, t1->len - 8, bmap_abcd, 4) != 1);
  TST_B_TST(ret, off + 5,
            vstr_spn_bmap_eq_fwd(t1, 4, t1->len - 8, bmap_pos_4_chr, 1) != 1);
}

int tst(void)
{
#ifdef USE_RESTRICTED_HEADERS /* %n doesn't work in dietlibc */
  return (EXIT_FAILED_OK);
#endif

  ASSERT(bmap_abcd[(unsigned char)'a'] == 0);
  ASSERT(bmap_abcd[(unsigned char)'b'] == 0);
  ASSERT(bmap_abcd[(unsigned char)'c'] == 0);
  ASSERT(bmap_abcd[(unsigned char)'d'] == 0);
  vstr_sc_bmap_init_eq_spn_cstr(bmap_abcd, "abcd", 1);
  ASSERT(bmap_abcd[(unsigned char)'a'] == 1);
  ASSERT(bmap_abcd[(unsigned char)'b'] == 1);
  ASSERT(bmap_abcd[(unsigned char)'c'] == 1);
  ASSERT(bmap_abcd[(unsigned char)'d'] == 1);
  vstr_sc_bmap_init_or_spn_cstr(bmap_abcd, "abcd", 2);
  ASSERT(bmap_abcd[(unsigned char)'a'] == 3);
  ASSERT(bmap_abcd[(unsigned char)'b'] == 3);
  ASSERT(bmap_abcd[(unsigned char)'c'] == 3);
  ASSERT(bmap_abcd[(unsigned char)'d'] == 3);
  vstr_sc_bmap_init_eq_spn_cstr(bmap_abcd, "0123456789", 4);
  vstr_sc_bmap_init_eq_spn_cstr(bmap_dash, "0123456789abcd -", 1);
  vstr_sc_bmap_init_eq_spn_cstr(bmap_xyz,  "0123456789abcd -xyz", 1);
  vstr_sc_bmap_init_eq_spn_cstr(bmap_and,  "0123456789abcd -!&", 1);
  vstr_sc_bmap_init_eq_spn_cstr(bmap_spac, "abcd ", 1);
  vstr_sc_bmap_init_eq_spn_cstr(bmap_a, "a", 1);
  
  sprintf(buf, "%d%nabcd %d%nxyz %u%n!& %u%nabcd",
          INT_MAX,  lens_fwd + 0,
          INT_MIN,  lens_fwd + 1,
          0,        lens_fwd + 2,
          UINT_MAX, lens_fwd + 3);
  vstr_sc_bmap_init_eq_spn_cstr(bmap_all, buf, 1);
  
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
