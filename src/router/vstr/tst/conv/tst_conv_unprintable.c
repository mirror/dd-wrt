#include "tst-main.c"

static const char *rf = __FILE__;

static char t[UCHAR_MAX + 1];
static char g[UCHAR_MAX + 1];

#define A_BUF(x1) do { \
 vstr_del(x1, 1, (x1)->len); vstr_add_buf((x1), (x1)->len, t, UCHAR_MAX + 1); \
 } while (FALSE)
#define A_PTR(x1) do { \
 vstr_del(x1, 1, (x1)->len); vstr_add_ptr((x1), (x1)->len, t, UCHAR_MAX + 1); \
 } while (FALSE)

#define  SP "<space>" /*  this here so we know nothing is interp. */
#define _SP "<space>" /*  this here so we know nothing is interp. */

static unsigned int cmp(size_t boff, size_t blen,
                        size_t voff, size_t vlen, size_t vtotlen,
                        unsigned int flags)
{
  int ret = 0;
  int mfail_count = 0;

  A_BUF(s1);
  do
  {
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_conv_unprintable_del(s1, 1, s1->len, flags));
  tst_mfail_num(0);
    
  TST_B_TST(ret, 1,
            !((vtotlen == s1->len) &&
              VSTR_CMP_BUF_EQ(s1, voff, vlen, g + boff, blen)));
  
  A_PTR(s1);
  do
  {
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_PTR, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_conv_unprintable_del(s1, 1, s1->len, flags));
  tst_mfail_num(0);
  
  TST_B_TST(ret, 2,
            !((vtotlen == s1->len) &&
              VSTR_CMP_BUF_EQ(s1, voff, vlen, g + boff, blen)));

  A_BUF(s1);
  mfail_count = 0;
  do
  {
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_conv_unprintable_chr(s1, 1, s1->len, flags, 'X'));
  tst_mfail_num(0);

  TST_B_TST(ret, 3,
            !VSTR_CMP_BUF_EQ(s1, 1, s1->len, g, UCHAR_MAX + 1));

  A_PTR(s1);
  do
  {
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_PTR, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_conv_unprintable_chr(s1, 1, s1->len, flags, 'X'));
  tst_mfail_num(0);
  TST_B_TST(ret, 4,
            !VSTR_CMP_BUF_EQ(s1, 1, s1->len, g, UCHAR_MAX + 1));

  return (ret);
}

int tst(void)
{
  unsigned int scan = 0;
  unsigned int flags = 0;
  int ret = 0;

  while (scan <= UCHAR_MAX)
  {
    t[scan] = scan;
    if ((scan >= 0x20) && (scan <= 0x7E))
      g[scan] = scan;
    else
      g[scan] = 'X';
    ++scan;
  }

  /* don't allow " ", ",", "." or "_" */
  g[0x20] = 'X';
  g[0x2C] = 'X';
  g[0x2E] = 'X';
  g[0x5F] = 'X';
  flags = VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_NONE;
  ret |= (cmp(0x21, 0x2C - 0x21,  1, 0x2C - 0x21, 0x5F - 4, flags) << 0);
  ret |= (cmp(0x2D, 1,           12, 1,           0x5F - 4, flags) << 0);
  ret |= (cmp(0x2F, 0x5F - 0x2F, 13, 0x5F - 0x2F, 0x5F - 4, flags) << 0);
  ret |= (cmp(0x60, 0x7F - 0x60, 61, 0x7F - 0x60, 0x5F - 4, flags) << 0);
  g[0x20] = 0x20;
  g[0x2C] = 0x2C;
  g[0x2E] = 0x2E;
  g[0x5F] = 0x5F;

  /* normal ASCII line -- tab/return etc. removed */
  flags = VSTR_FLAG_CONV_UNPRINTABLE_DEF;
  ret |= (cmp(0x20, 0x7F - 0x20,  1, 0x7F - 0x20, (0x7F - 0x20), flags) << 4);

  /* allow NUL, DEL */
  g[0x00] = 0x00;
  g[0x7F] = 0x7F;
  flags = VSTR_FLAG06(CONV_UNPRINTABLE_ALLOW, SP, COMMA, DOT, _, NUL, DEL);
  ret |= (cmp(0x20, 0x80 - 0x20,  2, 0x80 - 0x20, 0x60 + 1, flags) << 8);
  ret |= (cmp(0x00, 1,            1, 1,           0x60 + 1, flags) << 8);

  /* allow 0x07 - 0x0D, BEL, Backspace, Tabs, Return etc. */
  g[0x07] = 0x07;
  g[0x08] = 0x08;
  g[0x09] = 0x09;
  g[0x0A] = 0x0A;
  g[0x0B] = 0x0B;
  g[0x0C] = 0x0C;
  g[0x0D] = 0x0D;
  flags = (VSTR_FLAG05(CONV_UNPRINTABLE_ALLOW, SP, COMMA, DOT, _, NUL) |
           VSTR_FLAG08(CONV_UNPRINTABLE_ALLOW, BEL, BS, HT, LF,
                       VT, FF, CR, DEL));
  ret |= (cmp(0x20, 0x80 - 0x20,  9, 0x80 - 0x20, 0x60 + 8, flags) << 12);
  ret |= (cmp(0x00, 1,            1,           1, 0x60 + 8, flags) << 12);
  ret |= (cmp(0x07, 7,            2,           7, 0x60 + 8, flags) << 12);

  /* allow ESC */
  g[0x1B] = 0x1B;
  flags = (VSTR_FLAG05(CONV_UNPRINTABLE_ALLOW, SP, COMMA, DOT, _, NUL) |
           VSTR_FLAG09(CONV_UNPRINTABLE_ALLOW, BEL, BS, HT, LF,
                       VT, FF, CR, ESC, DEL));
  ret |= (cmp(0x20, 0x80 - 0x20, 10, 0x80 - 0x20, 0x60 + 9, flags) << 16);
  ret |= (cmp(0x00, 1,            1,           1, 0x60 + 9, flags) << 16);
  ret |= (cmp(0x07, 7,            2,           7, 0x60 + 9, flags) << 16);
  ret |= (cmp(0x1B, 1,            9,           1, 0x60 + 9, flags) << 16);

  /* allow HSP, HIGH */
  scan = 0xA0;
  while (scan <= UCHAR_MAX)
  {
    g[scan] = scan;
    ++scan;
  }

  flags = (VSTR_FLAG06(CONV_UNPRINTABLE_ALLOW, SP, COMMA, DOT, _, NUL, BEL) |
           VSTR_FLAG10(CONV_UNPRINTABLE_ALLOW,
                       BS, HT, LF, VT, FF, CR, ESC, DEL, HSP, HIGH));
  ret |= (cmp(0x20, 0x80 - 0x20, 10, 0x80 - 0x20, 0xC9, flags) << 24);
  ret |= (cmp(0x00, 1,            1,           1, 0xC9, flags) << 24);
  ret |= (cmp(0x07, 7,            2,           7, 0xC9, flags) << 24);
  ret |= (cmp(0x1B, 1,            9,           1, 0xC9, flags) << 24);
  ret |= (cmp(0xA0, 0x60,      0x6A,        0x60, 0xC9, flags) << 24);

  vstr_del(s1, 1, s1->len);
  vstr_del(s2, 1, s2->len);
  VSTR_ADD_CSTR_BUF(s1, 0, "abcd");
  vstr_add_non(s1, 0, 4);
  vstr_add_non(s1, s1->len, 4);
  VSTR_ADD_CSTR_BUF(s2, 0, "abcd");
  vstr_add_non(s2, 0, 4);
  vstr_add_non(s2, s2->len, 4);

  vstr_conv_unprintable_chr(s1, 1, s1->len,
                            VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_NUL, 'X');
  vstr_conv_unprintable_del(s1, 1, s1->len,
                            VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_NUL);
  TST_B_TST(ret, 28, !VSTR_CMP_EQ(s1, 1, s1->len, s2, 1, s2->len));
  vstr_conv_unprintable_chr(s1, 1, s1->len,
                            VSTR_FLAG_CONV_UNPRINTABLE_ALLOW__,  'A');
  vstr_conv_unprintable_del(s1, 1, s1->len, VSTR_FLAG_CONV_UNPRINTABLE_ALLOW__);
  TST_B_TST(ret, 29, !VSTR_CMP_EQ(s1, 1, s1->len, s2, 1, s2->len));

  return (TST_B_RET(ret));
}

/* tst_coverage
 *
 * VSTR_FLAG_CONV_UNPRINTABLE_ALLOW__
 * VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_BS
 * VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_BEL
 * VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_COMMA
 * VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_CR
 * VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_DEL
 * VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_DOT
 * VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_ESC
 * VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_FF
 * VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_HIGH
 * VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_HSP
 * VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_HT
 * VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_LF
 * VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_NUL
 * VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_SP
 * VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_VT
 *
 */
