#include "tst-main.c"

static const char *rf = __FILE__;

static unsigned int lens_fwd[4];
static unsigned int lens_rev[4];
static int ret = 0;

#define BMAP_ABCD    (1<<1)
#define BMAP_NUM     (1<<2)
#define BMAP_X       (1<<3)
#define BMAP_Z       (1<<4)
#define BMAP_PLING   (1<<5)
#define BMAP_XPLING ((1<<3) | (1<<5))
#define BMAP_A       (1<<0)
static unsigned char bmap[256];

static void tst_c_chrs(Vstr_base *t1, unsigned int off)
{
  TST_B_TST(ret, off + 1,
            vstr_cspn_bmap_and_fwd(t1, 1, t1->len, bmap, BMAP_X) !=
            lens_fwd[1]);
  TST_B_TST(ret, off + 2,
            vstr_cspn_bmap_and_rev(t1, 1, t1->len, bmap, BMAP_X) !=
            lens_rev[1] - 1);
  TST_B_TST(ret, off + 3,
            vstr_cspn_bmap_and_fwd(t1, 1, t1->len, bmap, BMAP_PLING) !=
            lens_fwd[2]);
  TST_B_TST(ret, off + 3,
            vstr_cspn_bmap_and_rev(t1, 1, t1->len, bmap, BMAP_XPLING) !=
            lens_rev[2] - 1);
  TST_B_TST(ret, off + 4,
            vstr_cspn_bmap_and_fwd(t1, 1, t1->len, bmap, BMAP_NUM) != 0);
  TST_B_TST(ret, off + 5,
            vstr_cspn_bmap_and_rev(t1, 1, t1->len, bmap, BMAP_NUM) != 4);
  TST_B_TST(ret, off + 6,
            vstr_cspn_bmap_and_fwd(t1, 1, lens_fwd[3], bmap, BMAP_ABCD) !=
            lens_fwd[0]);
  TST_B_TST(ret, off + 8,
            vstr_cspn_bmap_and_rev(t1, 1, lens_fwd[3], bmap, BMAP_ABCD) !=
            (lens_fwd[3] - lens_fwd[0]) - 4);
  TST_B_TST(ret, off + 9,
            vstr_cspn_bmap_and_fwd(t1, lens_fwd[1] + 1, t1->len - lens_fwd[1],
                                  bmap, BMAP_NUM) != 4);
  TST_B_TST(ret, off + 10,
            vstr_cspn_bmap_and_rev(t1, lens_fwd[1] + 1, t1->len - lens_fwd[1],
                                  bmap, BMAP_NUM) != 4);

  TST_B_TST(ret, off + 11,
            vstr_cspn_bmap_and_fwd(t1, 1, t1->len, bmap, BMAP_Z) != t1->len);
  TST_B_TST(ret, off + 12,
            vstr_cspn_bmap_and_rev(t1, 1, t1->len, bmap, BMAP_Z) != t1->len);

  TST_B_TST(ret, off + 17,
            vstr_cspn_bmap_and_fwd(t1, 1, 0, bmap, BMAP_A)  != 0);
  TST_B_TST(ret, off + 18,
            vstr_cspn_bmap_and_rev(t1, 1, 0, bmap, BMAP_A)  != 0);
}

static void tst_c_non_chrs(Vstr_base *t1)
{
  TST_B_TST(ret, 1,
            vstr_cspn_bmap_and_fwd(t1, 5, t1->len - 4, bmap, BMAP_NUM) != 4);
  TST_B_TST(ret, 2,
            vstr_cspn_bmap_and_rev(t1, 1, 8,           bmap, BMAP_NUM) != 4);
}

int tst(void)
{
#ifdef USE_RESTRICTED_HEADERS /* %n doesn't work in dietlibc */
  return (EXIT_FAILED_OK);
#endif

  ASSERT(bmap[(unsigned char)'a'] == 0);
  ASSERT(bmap[(unsigned char)'b'] == 0);
  ASSERT(bmap[(unsigned char)'c'] == 0);
  ASSERT(bmap[(unsigned char)'d'] == 0);
  vstr_sc_bmap_init_eq_spn_cstr(bmap, "a", 1<<0 | 1<<1);
  vstr_sc_bmap_init_eq_spn_cstr(bmap, "bcd", 1<<1);
  ASSERT(bmap[(unsigned char)'a'] == 3);
  ASSERT(bmap[(unsigned char)'b'] == 2);
  ASSERT(bmap[(unsigned char)'c'] == 2);
  ASSERT(bmap[(unsigned char)'d'] == 2);
  vstr_sc_bmap_init_eq_spn_cstr(bmap, "0123456789", 1<<2);
  vstr_sc_bmap_init_eq_spn_cstr(bmap, "x", 1<<3);
  vstr_sc_bmap_init_eq_spn_cstr(bmap, "Z", 1<<4);
  vstr_sc_bmap_init_eq_spn_cstr(bmap,  "!", 1<<5);
  
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
