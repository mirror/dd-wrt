#define VSTR_COMPILE_ATTRIBUTES 0 /* ignore "pure" attribute or
                                   * the below might go away */

#include "tst-main.c"

extern VSTR_SECTS_EXTERN_DECL(blahfoo, 4);

static const char *rf = __FILE__;

static void tst_v(const char *msg, ...)
{
  va_list ap;

  va_start(ap, msg);
  vstr_add_vfmt(s1, 0, msg, ap);
  va_end(ap);

  va_start(ap, msg);
  vstr_add_vsysfmt(s1, 0, msg, ap);
  va_end(ap);
}


/* misc crap for tst_covorage -- it's called by other stuff */
int tst(void)
{
  VSTR_SECTS_DECL(abcd, 4);
#ifdef __GNUC__
  typeof(blahfoo[0]) *tmp = abcd;

  assert(sizeof(blahfoo) == sizeof(abcd));
  assert(tmp->sz == 4);
#endif

  vstr_make_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 8);

  tst_v("%s", "123456789 123456789 123456789 123456789 123456789 123456789 ");
  assert(s1->len == 120);

  vstr_sc_fmt_add_bkmg_bits_uint(NULL, "1");
  vstr_sc_fmt_add_bkmg_bit_uint(NULL, "2");
  vstr_sc_fmt_add_bkmg_Bytes_uint(NULL, "3");
  vstr_sc_fmt_add_bkmg_Byte_uint(NULL, "4");
  vstr_sc_fmt_add_buf(NULL, "5");
  vstr_sc_fmt_add_non(NULL, "6");
  vstr_sc_fmt_add_ptr(NULL, "7");
  vstr_sc_fmt_add_ref(NULL, "8");
  vstr_sc_fmt_add_rep_chr(NULL, "9");
  vstr_sc_fmt_add_vstr(NULL, "a");
  vstr_sc_fmt_add_ipv4_ptr(NULL, "b");
  vstr_sc_fmt_add_ipv6_ptr(NULL, "c");
  vstr_sc_fmt_add_ipv4_vec(NULL, "d");
  vstr_sc_fmt_add_ipv6_vec(NULL, "e");
  vstr_sc_fmt_add_ipv4_vec_cidr(NULL, "f");
  vstr_sc_fmt_add_ipv6_vec_cidr(NULL, "g");
  vstr_sc_fmt_add_lower_base2_size(NULL, "h");
  vstr_sc_fmt_add_lower_base2_uint(NULL, "i");
  vstr_sc_fmt_add_lower_base2_uintmax(NULL, "j");
  vstr_sc_fmt_add_lower_base2_ulong(NULL, "k");
  vstr_sc_fmt_add_upper_base2_size(NULL, "l");
  vstr_sc_fmt_add_upper_base2_uint(NULL, "m");
  vstr_sc_fmt_add_upper_base2_uintmax(NULL, "n");
  vstr_sc_fmt_add_upper_base2_ulong(NULL, "o");
  vstr_sc_fmt_add_bkmg_bits_uintmax(NULL, "p");
  vstr_sc_fmt_add_bkmg_bit_uintmax(NULL, "q");
  vstr_sc_fmt_add_bkmg_Bytes_uintmax(NULL, "r");
  vstr_sc_fmt_add_bkmg_Byte_uintmax(NULL, "s");

  vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_BUF, 1);

  return (!VSTR_FLAG31(CONV_UNPRINTABLE_ALLOW,
                       NUL, BEL, BS, HT, LF, VT, FF, CR, SP, COMMA,
                       NUL, BEL, BS, HT, LF, VT, FF, CR, SP, COMMA,
                       NUL, BEL, BS, HT, LF,
                       DOT, _, ESC, DEL, HSP, HIGH));
}

/* Crap for tst_coverage constants....
 *
 * VSTR_MAX_NODE_ALL
 *
 * VSTR_SECTS_INIT()
 *
 */
