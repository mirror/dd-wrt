#define VSTR_ADD_FMT_C
/*
 *  Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004  James Antill
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  email: james@and.org
*/
/* this code is hacked from:
 *   vsprintf in linux/lib -- by Lars Wirzenius <liw@iki.fi>
 * He gave his permission for it to be released as LGPL.
 * ... it's changed a lot since then. */
/* functions for portable printf functionality in a vstr -- does C99 std. and
 * custom format specifiers (registration via. vstr_fmt.c).
 * Has a "portable" implementation of double printing */
#include "main.h"

#if !USE_WIDE_CHAR_T
# undef wchar_t
# define wchar_t int /* doesn't matter ... fails out anyway */
# undef wint_t
# define wint_t  int /* doesn't matter ... fails out anyway */
#endif



#define VSTR__ADD_FMT_ISDIGIT(x) (((x) >= '0') && ((x) <= '9'))

#define VSTR__ADD_FMT_CHAR2INT_1(x) ((x) - '0')
#define VSTR__ADD_FMT_CHAR2INT_2(x, y) ((((x) - '0') * 10) + \
                                         ((y) - '0'))

/* a very small number of digits is _by far_ the norm */
#define VSTR__ADD_FMT_STRTOL(x) \
 (VSTR__ADD_FMT_ISDIGIT((x)[1]) ? \
  VSTR__ADD_FMT_ISDIGIT((x)[2]) ? strtol(x, (char **) &(x), 10) : \
  ((x) += 2, VSTR__ADD_FMT_CHAR2INT_2((x)[-2], (x)[-1])) : \
  ((x) += 1, VSTR__ADD_FMT_CHAR2INT_1((x)[-1])))

/* 1 byte for the base of the number, can go upto base 255 */
#define BASE_MASK ((1 << 8) - 1)

#define ZEROPAD (1 << 8) /* pad with zero */
#define SIGN (1 << 9) /* unsigned/signed */
#define PLUS (1 << 10) /* show plus sign '+' */
#define SPACE (1 << 11) /* space if plus */
#define LEFT (1 << 12) /* left justified */
#define SPECIAL (1 << 13) /* 0x */
#define LARGE (1 << 14) /* use 'ABCDEF' instead of 'abcdef' */
#define THOUSAND_SEP (1 << 15) /* split at grouping marks according to locale */
#define ALT_DIGITS (1 << 16) /* does nothing in core code, can be used in
                              * custom format specifiers */

#define PREC_USR (1 << 28) /* user specified precision */
#define NUM_IS_ZERO (1 << 29) /* is the number zero */
#define NUM_IS_NEGATIVE (1 << 30) /* is the number negative */

#define VSTR__FMT_ADD(x, y, z) vstr_add_buf((x), ((x)->len - pos_diff), y, z)
#define VSTR__FMT_ADD_REP_CHR(x, y, z) \
 vstr_add_rep_chr((x), ((x)->len - pos_diff), y, z)
#define VSTR__FMT_ADD_GRPBASENUM(x, NB, y, z) \
 vstr_sc_add_grpbasenum_buf((x), ((x)->len - pos_diff), NB, y, z)

/* deals with signed integer overflow */
#define VSTR__FMT_S2U_NUM(unum, snum) do { \
      unum = snum; unum = -unum;           \
    } while (FALSE)
#define VSTR__FMT_ABS_NUM(unum, snum) do {      \
      if (snum < 0) {                           \
        spec->flags |= NUM_IS_NEGATIVE;         \
        VSTR__FMT_S2U_NUM(unum, snum);          \
      } else                                    \
        unum = snum;                            \
    } while (FALSE)


/* functions for outputing thousands grouping ... */
unsigned int vstr__add_fmt_grouping_mod(const char *grouping, unsigned int num)
{
  unsigned int tmp = 0;

  if (!*grouping)
    return (num);

  while (((unsigned char)*grouping < SCHAR_MAX) &&
         ((tmp + *grouping) < num))
  {
    tmp += *grouping;
    if (grouping[1])
      ++grouping;
  }

  return (num - tmp);
}

size_t vstr__add_fmt_grouping_num_sz(Vstr_base *base,
                                     unsigned int num_base, size_t len)
{
  size_t ret = 0;
  int done = FALSE;
  Vstr_locale *loc = base->conf->loc;
  const char *grouping = vstr__loc_num_grouping(loc, num_base);
  size_t sep_len = vstr__loc_num_sep_len(loc, num_base);
  
  while (len)
  {
    unsigned int num = vstr__add_fmt_grouping_mod(grouping, len);

    if (done)
      ret += sep_len;

    ret += num;
    assert(num <= len);
    len -= num;

    done = TRUE;
  }

  return (ret);
}


struct Vstr__fmt_spec
{
 union Vstr__fmt_sp_un
 {
  unsigned char data_c;
  unsigned short data_s;
  unsigned int data_i;
  wint_t data_wint;
  unsigned long data_l;
  Vstr__unsigned_long_long data_L;
  size_t data_sz;
  uintmax_t data_m;

  double data_d;
  long double data_Ld;

  void *data_ptr;
 } u;

 ptrdiff_t data_t; /* NOTE: no uptrdiff_t, but need real ptrdiff_t for
                    * custom formatters. So normal uintmax_t */
 
 unsigned char fmt_code;
 int num_base;
 unsigned int int_type;
 unsigned int flags;
 unsigned int field_width;
 unsigned int precision;

 unsigned int main_param;
 unsigned int field_width_param;
 unsigned int precision_param;

 Vstr__fmt_usr_name_node *usr_spec;

 /* these two needed for double, not used elsewhere */
 /* unsigned int precision_usr : 1; done with the PREC_USR flag */
 unsigned int field_width_usr : 1; /* did the usr specify a field width */
 unsigned int escape_usr : 1;  /* did the usr specify this escape */

 struct Vstr__fmt_spec *next;
};

static int vstr__add_fmt_number(Vstr_base *base, size_t pos_diff,
                                struct Vstr__fmt_spec *spec)
{
  char sign = 0;
  /* used to hold the actual number */
  char buf_beg[BUF_NUM_TYPE_SZ(uintmax_t)];
  char *buf = buf_beg + sizeof(buf_beg);
  size_t i = 0;
  size_t real_i = 0;
  /* can't make array */
  const char *chrs_base = "0123456789abcdefghijklmnopqrstuvwxyz";
  const char *grouping = NULL;
  unsigned char grp_num = 0;
  const char *thou = NULL;
  size_t thou_len = 0;
  size_t max_p_i = 0;
  unsigned int field_width = 0;
  unsigned int precision = 1;
  unsigned int wr_hex_0x = FALSE;

  if (spec->flags & PREC_USR)
    precision = spec->precision;

  if (spec->field_width_usr)
    field_width = spec->field_width;
  
  /* if the usr specified a precision the '0' flag is ignored */
  if (spec->flags & PREC_USR)
    spec->flags &= ~ZEROPAD;
  
  if (spec->flags & LARGE)
    chrs_base = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  if (spec->flags & LEFT)
    spec->flags &= ~ZEROPAD;

  switch (spec->num_base)
  {
    case 10:
    case 16:
    case  8:

    ASSERT_NO_SWITCH_DEF();
  }
  
  if (spec->flags & SIGN)
  {
    if (spec->flags & NUM_IS_NEGATIVE)
      sign = '-';
    else
    {
      if (spec->flags & PLUS)
        sign = '+';
      else
        if (spec->flags & SPACE)
          sign = ' ';
        else
          ++field_width;
    }
    
    if (field_width) --field_width;
  }

  grouping = vstr__loc_num_grouping(base->conf->loc, spec->num_base);
  thou_len = vstr__loc_num_sep_len(base->conf->loc, spec->num_base);
  thou     = vstr__loc_num_sep_ptr(base->conf->loc, spec->num_base);

  grp_num = *grouping;
  if (!thou_len || (grp_num >= SCHAR_MAX) || (grp_num <= 0))
    spec->flags &= ~THOUSAND_SEP; /* don't do it */

  switch (spec->int_type)
  {
    case VSTR_TYPE_FMT_UCHAR:
      VSTR__ADD_FMT_NUM(unsigned char, spec->u.data_c, spec->num_base);  break;
    case VSTR_TYPE_FMT_USHORT:
      VSTR__ADD_FMT_NUM(unsigned short, spec->u.data_s, spec->num_base); break;
    case VSTR_TYPE_FMT_UINT:
      VSTR__ADD_FMT_NUM(unsigned int, spec->u.data_i, spec->num_base);   break;
    case VSTR_TYPE_FMT_ULONG:
      VSTR__ADD_FMT_NUM(unsigned long, spec->u.data_l, spec->num_base);  break;
    case VSTR_TYPE_FMT_ULONG_LONG:
      VSTR__ADD_FMT_NUM(Vstr__unsigned_long_long, spec->u.data_L,
                        spec->num_base);                                 break;
    case VSTR_TYPE_FMT_SIZE_T:
      VSTR__ADD_FMT_NUM(size_t, spec->u.data_sz, spec->num_base);        break;
      /* ptrdiff_t is actually promoted to intmax_t so that unsigned works */
    case VSTR_TYPE_FMT_UINTMAX_T:
      VSTR__ADD_FMT_NUM(uintmax_t, spec->u.data_m, spec->num_base);      break;
    case VSTR_TYPE_FMT_PTR_VOID:
      VSTR__ADD_FMT_NUM(uintptr_t, (uintptr_t)spec->u.data_ptr, spec->num_base);
    ASSERT_NO_SWITCH_DEF(); /* only valid types above */
  }
  
  i = sizeof(buf_beg) - (buf - buf_beg);
  
  real_i = i;
  if (spec->flags & THOUSAND_SEP)
    real_i = vstr__add_fmt_grouping_num_sz(base, spec->num_base, i);

  if (spec->flags & SPECIAL)
  {
    if ((spec->num_base == 16) && !(spec->flags & NUM_IS_ZERO))
    { /* only append 0x if it is a non-zero value, but not if precision == 0 */
      wr_hex_0x = TRUE;
      if (field_width) --field_width;
      if (field_width) --field_width;
    }
    else
    { /* hacky spec, if octal then we up the precision so that a 0 is printed as
       * the first character, if there isn't a 0 there to start with
       * -- even if num == 0 and precision == 0 */
      if ((spec->num_base == 8) &&
          (((spec->flags & NUM_IS_ZERO) && !precision) ||
           (precision <= real_i)))
        precision = real_i + 1;
    }
  }
  
  if (real_i > precision)
    max_p_i = real_i;
  else
    max_p_i = precision;
  
  if (field_width < max_p_i)
    field_width = 0;
  else
    field_width -= max_p_i;
  
  if (!(spec->flags & (ZEROPAD | LEFT)))
    if (field_width > 0)
    { /* right justify number, with spaces -- zeros done after sign/spacials */
      if (!VSTR__FMT_ADD_REP_CHR(base, ' ', field_width))
        goto failed_alloc;
      field_width = 0;
    }
  
  if (sign)
    if (!VSTR__FMT_ADD_REP_CHR(base, sign, 1))
      goto failed_alloc;
  
  if (spec->flags & SPECIAL)
  {
    if (wr_hex_0x)
    {
      if (!VSTR__FMT_ADD_REP_CHR(base, '0', 1))
        goto failed_alloc;
      if (!VSTR__FMT_ADD_REP_CHR(base, chrs_base[33], 1))
        goto failed_alloc;
    }
  }
  
  if (!(spec->flags & LEFT))
    if (field_width > 0)
    { /* right justify number, with zeros */
      assert(spec->flags & ZEROPAD);
      if (!VSTR__FMT_ADD_REP_CHR(base, '0', field_width))
        goto failed_alloc;
      field_width = 0;
    }
  
  if (precision > real_i) /* make number the desired length */
  {
   if (!VSTR__FMT_ADD_REP_CHR(base, '0', precision - real_i))
     goto failed_alloc;
  }

  if (i)
  {
    int ret = FALSE;
    
    /* output number */
    if (spec->flags & THOUSAND_SEP)
      ret = VSTR__FMT_ADD_GRPBASENUM(base, spec->num_base, buf, i);
    else
      ret = VSTR__FMT_ADD(base, buf, i);
    
    if (!ret)
      goto failed_alloc;
  }

  if (field_width > 0)
  {
    assert(spec->flags & LEFT);
    if (!VSTR__FMT_ADD_REP_CHR(base, ' ', field_width))
      goto failed_alloc;
  }

  return (TRUE);

 failed_alloc:
  return (FALSE);
}

void vstr__add_fmt_free_conf(Vstr_conf *conf)
{
  struct Vstr__fmt_spec *scan = conf->vstr__fmt_spec_make;
  unsigned int num = 0;
  
  assert(!conf->vstr__fmt_spec_list_beg && !conf->vstr__fmt_spec_list_beg);

  while (scan)
  {
    struct Vstr__fmt_spec *scan_next = scan->next;
    VSTR__F(scan);
    scan = scan_next;
  }
  conf->vstr__fmt_spec_make = NULL;

  /* slow ... but who cares? */
  while (conf->fmt_usr_names)
  {
    Vstr__fmt_usr_name_node *tmp = conf->fmt_usr_names;

    vstr_fmt_del(conf, tmp->name_str);
  }

  while (num < 37)
  {
    while (conf->fmt_usr_name_hash[num])
    {
      Vstr__fmt_usr_name_node *tmp = conf->fmt_usr_name_hash[num];

      vstr_fmt_del(conf, tmp->name_str);
    }
    ++num;
  }
}

static int vstr__fmt_add_spec(Vstr_conf *conf)
{
  struct Vstr__fmt_spec *spec = NULL;

  if (!(spec = VSTR__MK(sizeof(struct Vstr__fmt_spec))))
    return (FALSE);

  assert(vstr_wrap_memset(spec, 0xeF, sizeof(struct Vstr__fmt_spec)));

  spec->next = conf->vstr__fmt_spec_make;
  conf->vstr__fmt_spec_make = spec;

  return (TRUE);
}

#define VSTR__FMT_MV_SPEC(conf, x) vstr__fmt_mv_spec(conf, spec, x, &params)
static void vstr__fmt_mv_spec(Vstr_conf *conf, struct Vstr__fmt_spec *spec,
                              int main_param, unsigned int *params)
{
  conf->vstr__fmt_spec_make = spec->next;

  if (!conf->vstr__fmt_spec_list_beg)
  {
    conf->vstr__fmt_spec_list_end = spec;
    conf->vstr__fmt_spec_list_beg = spec;
  }
  else
  {
    conf->vstr__fmt_spec_list_end->next = spec;
    conf->vstr__fmt_spec_list_end = spec;
  }

  spec->next = NULL;

  /* increment if this counts towards users $x numbers */
  if (main_param && !spec->main_param)
    spec->main_param = ++*params;
}

# define VSTR__FMT_ANAL_ZERO 1 /* anally zero stuff that shouldn't really need
                                * it but seems to *sigh*/
static void vstr__fmt_init_spec(struct Vstr__fmt_spec *spec)
{
  assert(spec);

  if (VSTR__FMT_ANAL_ZERO)
  {
    spec->u.data_ptr = NULL;
  }

  spec->fmt_code = 0;
  spec->num_base = 10;
  spec->int_type = VSTR_TYPE_FMT_UINT;
  spec->flags = 0;

  if (VSTR__FMT_ANAL_ZERO)
  {
    spec->precision = 0;
    spec->field_width = 0;
  }

  spec->main_param = 0;
  spec->field_width_param = 0;
  spec->precision_param = 0;

  spec->usr_spec = NULL;

  spec->field_width_usr = FALSE;
  spec->escape_usr = FALSE;
}

/* must match with code in vstr_version.c */
#if defined(VSTR_AUTOCONF_FMT_DBL_glibc)
# include "vstr_dbl/vstr_add_fmt_dbl_glibc.c"
#elif defined(VSTR_AUTOCONF_FMT_DBL_none)
# include "vstr_dbl/vstr_add_fmt_dbl_none.c"
#elif defined(VSTR_AUTOCONF_FMT_DBL_host)
# include "vstr_dbl/vstr_add_fmt_dbl_host.c"
#else
# error "Please configure properly..."
#endif

static int vstr__add_fmt_char(Vstr_base *base, size_t pos_diff,
                              struct Vstr__fmt_spec *spec)
{
  if (spec->field_width > 0)
    --spec->field_width; /* account for char */

  if (spec->field_width_usr && !(spec->flags & LEFT))
    if (spec->field_width > 0)
    {
      if (!VSTR__FMT_ADD_REP_CHR(base, ' ', spec->field_width))
        goto failed_alloc;
      spec->field_width_usr = FALSE;
    }

  if (!VSTR__FMT_ADD_REP_CHR(base, spec->u.data_c, 1))
    goto failed_alloc;

  if (spec->field_width_usr && (spec->field_width > 0))
  {
    if (!VSTR__FMT_ADD_REP_CHR(base, ' ', spec->field_width))
      goto failed_alloc;
  }

  return (TRUE);

 failed_alloc:
  return (FALSE);
}

#if USE_WIDE_CHAR_T
static int vstr__add_fmt_wide_char(Vstr_base *base, size_t pos_diff,
                                   struct Vstr__fmt_spec *spec)
{
  mbstate_t state;
  size_t len_mbs = 0;
  char *buf_mbs = NULL;

  vstr_wrap_memset(&state, 0, sizeof(mbstate_t));
  len_mbs = wcrtomb(NULL, spec->u.data_wint, &state);
  if (len_mbs != (size_t)-1)
  { /* stupid glibc only returns -1 when you pass a buf */
    len_mbs += wcrtomb(NULL, L'\0', &state);

    if (!(buf_mbs = VSTR__MK(len_mbs)))
    {
      base->conf->malloc_bad = TRUE;
      goto failed_alloc;
    }
    vstr_wrap_memset(&state, 0, sizeof(mbstate_t));
    len_mbs = wcrtomb(buf_mbs, spec->u.data_wint, &state);
  }
  if (len_mbs == (size_t)-1)
    goto failed_EILSEQ;
  len_mbs += wcrtomb(buf_mbs + len_mbs, L'\0', &state);
  --len_mbs; /* remove terminator */

  if (spec->field_width > 0)
    --spec->field_width; /* account for char */

  if (spec->field_width_usr && !(spec->flags & LEFT))
    if (spec->field_width > 0)
    {
      if (!VSTR__FMT_ADD_REP_CHR(base, ' ', spec->field_width))
        goto C_failed_alloc_free_buf_mbs;
      spec->field_width_usr = FALSE;
    }

  if (!VSTR__FMT_ADD(base, buf_mbs, len_mbs))
    goto C_failed_alloc_free_buf_mbs;

  if (spec->field_width_usr && (spec->field_width > 0))
  {
    if (!VSTR__FMT_ADD_REP_CHR(base, ' ', spec->field_width))
      goto C_failed_alloc_free_buf_mbs;
  }

  VSTR__F(buf_mbs);
  return (TRUE);

 C_failed_alloc_free_buf_mbs:
 failed_EILSEQ: /* FIXME: need a good way to see this */
  VSTR__F(buf_mbs);
 failed_alloc:
  return (FALSE);
}
#else
static int vstr__add_fmt_wide_char(Vstr_base *VSTR__ATTR_UNUSED(base),
                                   size_t VSTR__ATTR_UNUSED(pos_diff),
                                   struct Vstr__fmt_spec *
                                   VSTR__ATTR_UNUSED(spec))
{
  assert(FALSE);
  return (FALSE);
}
#endif

static int vstr__add_fmt_cstr(Vstr_base *base, size_t pos_diff,
                              struct Vstr__fmt_spec *spec)
{
  size_t len = 0;
  const char *str = spec->u.data_ptr;

  if (!str)
  {
    str = base->conf->loc->null_ref->ptr;
    len = base->conf->loc->null_len;
    if ((spec->flags & PREC_USR) && (len > spec->precision))
      len = spec->precision;
  }
  else if (spec->flags & PREC_USR)
    len = strnlen(str, spec->precision);
  else
    len = strlen(str);
  
  if ((spec->flags & PREC_USR) && spec->field_width_usr &&
      (spec->field_width > spec->precision))
    spec->field_width = spec->precision;

  if (spec->field_width_usr && !(spec->flags & LEFT))
    if (spec->field_width > len)
    {
      if (!VSTR__FMT_ADD_REP_CHR(base, ' ', spec->field_width - len))
        goto failed_alloc;
      spec->field_width_usr = FALSE;
    }

  if (!VSTR__FMT_ADD(base, str, len))
    goto failed_alloc;

  if (spec->field_width_usr && (spec->field_width > len))
    if (!VSTR__FMT_ADD_REP_CHR(base, ' ', spec->field_width - len))
      goto failed_alloc;

  return (TRUE);

 failed_alloc:
  return (FALSE);
}

#if USE_WIDE_CHAR_T
static int vstr__add_fmt_wide_cstr(Vstr_base *base, size_t pos_diff,
                                   struct Vstr__fmt_spec *spec)
{ /* note precision is number of wchar_t's allowed through, not bytes
   * as in sprintf() */
  mbstate_t state;
  size_t len_mbs = 0;
  char *buf_mbs = NULL;
  const wchar_t *wstr = spec->u.data_ptr;
  const wchar_t *tmp  = NULL;

  if (!wstr)
    return (vstr__add_fmt_cstr(base, pos_diff, spec));
  
  tmp = wstr;

  vstr_wrap_memset(&state, 0, sizeof(mbstate_t));

  if (!(spec->flags & PREC_USR))
    spec->precision = UINT_MAX;
  len_mbs = wcsnrtombs(NULL, &tmp, spec->precision, 0, &state);
  if (len_mbs == (size_t)-1)
    goto failed_EILSEQ;
  /* NULL when found end of input -- glibc doesn't do this when arg1 NULL
   * if (!tmp)
   *   ++len_mbs;
   * else
   */
  { /* wcslen() > spec->precision */
    size_t tmp_mbs = wcrtomb(NULL, L'\0', &state);

    if (tmp_mbs != (size_t)-1)
      len_mbs += tmp_mbs; /* include '\0' */
  }

  if (!(buf_mbs = VSTR__MK(len_mbs)))
  {
    base->conf->malloc_bad = TRUE;
    goto failed_alloc;
  }
  tmp = wstr;
  vstr_wrap_memset(&state, 0, sizeof(mbstate_t));
  len_mbs = wcsnrtombs(buf_mbs, &tmp, spec->precision, len_mbs, &state);
  if (tmp) /* wcslen() > spec->precision */
  {
    size_t tmp_mbs = wcrtomb(buf_mbs + len_mbs, L'\0', &state);
    len_mbs += tmp_mbs - 1; /* ignore '\0' */
  }

  if ((spec->flags & PREC_USR) && spec->field_width_usr &&
      (spec->field_width > spec->precision))
    spec->field_width = spec->precision;

  if (spec->field_width_usr && !(spec->flags & LEFT))
    if (spec->field_width > len_mbs)
    {
      if (!VSTR__FMT_ADD_REP_CHR(base, ' ', spec->field_width - len_mbs))
        goto S_failed_alloc_free_buf_mbs;
      spec->field_width_usr = FALSE;
    }

  if (!VSTR__FMT_ADD(base, buf_mbs, len_mbs))
    goto S_failed_alloc_free_buf_mbs;

  if (spec->field_width_usr && (spec->field_width > len_mbs))
    if (!VSTR__FMT_ADD_REP_CHR(base, ' ', spec->field_width - len_mbs))
      goto S_failed_alloc_free_buf_mbs;

  VSTR__F(buf_mbs);
  return (TRUE);

 S_failed_alloc_free_buf_mbs:
  VSTR__F(buf_mbs);
 failed_alloc:
 failed_EILSEQ: /* FIXME: */
  return (FALSE);
}
#else
static int vstr__add_fmt_wide_cstr(Vstr_base *base,
                                   size_t pos_diff,
                                   struct Vstr__fmt_spec *spec)
{
  if (!spec->u.data_ptr)
    return (vstr__add_fmt_cstr(base, pos_diff, spec));
  
  assert(FALSE);
  return (FALSE);
}
#endif

static
struct Vstr__fmt_spec *
vstr__add_fmt_usr_write_spec(Vstr_base *base, size_t orig_len, size_t pos_diff,
                             struct Vstr__fmt_spec *spec, int sve_errno)
{
  struct Vstr__fmt_spec *last = NULL;
  struct
  {
   Vstr_fmt_spec usr_spec;
   void *params[VSTR__FMT_USR_SZ];
  } dummy;
  Vstr_fmt_spec *usr_spec = NULL;
  unsigned int scan = 0;
  int (*func)(Vstr_base *, size_t, struct Vstr_fmt_spec *) =
    spec->usr_spec->func;
  unsigned int *types = spec->usr_spec->types;
  unsigned int sz     = spec->usr_spec->sz;
  
  assert(spec->escape_usr);
  assert(spec->usr_spec);

  if (sz <= VSTR__FMT_USR_SZ)
    usr_spec = &dummy.usr_spec;
  else
  {
    if (!(usr_spec = VSTR__MK(sizeof(Vstr_fmt_spec) +
                              (spec->usr_spec->sz * sizeof(void *)))))
      return (NULL);
  }

  usr_spec->vstr_orig_len = orig_len;
  usr_spec->name = spec->usr_spec->name_str;

  usr_spec->obj_precision = 0;
  usr_spec->obj_field_width = 0;

  if ((usr_spec->fmt_precision = !!(spec->flags & PREC_USR)))
    usr_spec->obj_precision = spec->precision;
  if ((usr_spec->fmt_field_width =  spec->field_width_usr))
    usr_spec->obj_field_width = spec->field_width;

  usr_spec->fmt_minus = !!(spec->flags & LEFT);
  usr_spec->fmt_plus =  !!(spec->flags & PLUS);
  usr_spec->fmt_space = !!(spec->flags & SPACE);
  usr_spec->fmt_hash =  !!(spec->flags & SPECIAL);
  usr_spec->fmt_zero =  !!(spec->flags & ZEROPAD);
  usr_spec->fmt_quote = !!(spec->flags & THOUSAND_SEP);
  usr_spec->fmt_I =     !!(spec->flags & ALT_DIGITS);

  if (!types[scan])
  {
    assert(spec->escape_usr && !!spec->usr_spec);

    last = spec;
    spec = spec->next;
  }

  while (types[scan])
  {
    switch (types[scan])
    {
      case VSTR_TYPE_FMT_INT:
      case VSTR_TYPE_FMT_UINT:
      case VSTR_TYPE_FMT_LONG:
      case VSTR_TYPE_FMT_ULONG:
      case VSTR_TYPE_FMT_LONG_LONG:
      case VSTR_TYPE_FMT_ULONG_LONG:
      case VSTR_TYPE_FMT_SSIZE_T:
      case VSTR_TYPE_FMT_SIZE_T:
      case VSTR_TYPE_FMT_INTMAX_T:
      case VSTR_TYPE_FMT_UINTMAX_T:
      case VSTR_TYPE_FMT_DOUBLE:
      case VSTR_TYPE_FMT_DOUBLE_LONG:
        usr_spec->data_ptr[scan] = &spec->u;
        break;
      case VSTR_TYPE_FMT_PTRDIFF_T:
        ASSERT(spec->data_t == (ptrdiff_t)spec->u.data_m);
        usr_spec->data_ptr[scan] = &spec->data_t;
        break;
        
      case VSTR_TYPE_FMT_PTR_VOID:
      case VSTR_TYPE_FMT_PTR_CHAR:
      case VSTR_TYPE_FMT_PTR_WCHAR_T:
      case VSTR_TYPE_FMT_PTR_SIGNED_CHAR:
      case VSTR_TYPE_FMT_PTR_SHORT:
      case VSTR_TYPE_FMT_PTR_INT:
      case VSTR_TYPE_FMT_PTR_LONG:
      case VSTR_TYPE_FMT_PTR_LONG_LONG:
      case VSTR_TYPE_FMT_PTR_SSIZE_T:
      case VSTR_TYPE_FMT_PTR_PTRDIFF_T:
      case VSTR_TYPE_FMT_PTR_INTMAX_T:
        usr_spec->data_ptr[scan] =  spec->u.data_ptr;
        break;
      case VSTR_TYPE_FMT_ERRNO:
        errno = sve_errno;
        ASSERT_NO_SWITCH_DEF();
    }
    assert(spec->escape_usr && (scan ? !spec->usr_spec : !!spec->usr_spec));

    ++scan;
    last = spec;
    spec = spec->next;
  }
  assert(!spec || !spec->escape_usr || spec->usr_spec);
  usr_spec->data_ptr[scan] = NULL;

  if (!(*func)(base, base->len - pos_diff, usr_spec))
    last = NULL;
  
  if (sz > VSTR__FMT_USR_SZ)
    VSTR__F(usr_spec);

  return (last);
}

#define VSTR__FMT_N_PTR(x, y) \
     case x: do \
       { \
        y *len_curr = spec->u.data_ptr; \
        *len_curr = len; \
       } while (FALSE)

static int vstr__fmt_write_spec(Vstr_base *base, size_t pos_diff,
                                size_t orig_len, int sve_errno)
{
  struct Vstr__fmt_spec *const beg  = base->conf->vstr__fmt_spec_list_beg;
  struct Vstr__fmt_spec *const end  = base->conf->vstr__fmt_spec_list_end;
  struct Vstr__fmt_spec *      spec = base->conf->vstr__fmt_spec_list_beg;
  char buf_errno[2048];
  
  if (!end) /* invalid format... */
    return (TRUE);
  
  ASSERT(beg && end);

  /* allow vstr_add_vfmt() to be called form a usr cb */
  base->conf->vstr__fmt_spec_list_beg = NULL;
  base->conf->vstr__fmt_spec_list_end = NULL;
  
  while (spec)
  {
    if (spec->escape_usr)
    {
      if (!(spec = vstr__add_fmt_usr_write_spec(base, orig_len,
                                                pos_diff, spec, sve_errno)))
        goto failed_user;
    }
    else switch (spec->fmt_code)
    {
      case 'c':
        if (!vstr__add_fmt_char(base, pos_diff, spec))
          goto failed_alloc;
        break;

      case 'C':
        if (!vstr__add_fmt_wide_char(base, pos_diff, spec))
          goto failed_alloc;
        break;

      case 'm':
        if (!strerror_r(sve_errno, buf_errno, sizeof(buf_errno)))
          spec->u.data_ptr = buf_errno;
        else
          spec->u.data_ptr = strerror(sve_errno);
      case 's':
        if (!vstr__add_fmt_cstr(base, pos_diff, spec))
          goto failed_alloc;
        break;

      case 'S':
        if (!vstr__add_fmt_wide_cstr(base, pos_diff, spec))
          goto failed_alloc;
        break;

      case 'p': /* convert ptr to unsigned long and print */
      {
        assert(spec->int_type == VSTR_TYPE_FMT_PTR_VOID);
        assert(!(spec->flags & SIGN));
      }
      /* FALLTHROUGH */
      case 'd':
      case 'i':
      case 'u':
      case 'X':
      case 'x':
      case 'o':
        if (!vstr__add_fmt_number(base, pos_diff, spec))
          goto failed_alloc;
        break;

      case 'n':
      {
        size_t len = (base->len - orig_len);

        switch (spec->int_type)
        {
          VSTR__FMT_N_PTR(VSTR_TYPE_FMT_UCHAR, signed char);          break;
          VSTR__FMT_N_PTR(VSTR_TYPE_FMT_USHORT, short);               break;
          VSTR__FMT_N_PTR(VSTR_TYPE_FMT_UINT, int);                   break;
          VSTR__FMT_N_PTR(VSTR_TYPE_FMT_ULONG, long);                 break;
          VSTR__FMT_N_PTR(VSTR_TYPE_FMT_ULONG_LONG, Vstr__long_long); break;
          VSTR__FMT_N_PTR(VSTR_TYPE_FMT_SIZE_T, ssize_t);             break;
          VSTR__FMT_N_PTR(VSTR_TYPE_FMT_UINTMAX_T, intmax_t);         break;
          VSTR__FMT_N_PTR(VSTR_TYPE_FMT_PTRDIFF_T, ptrdiff_t);
          ASSERT_NO_SWITCH_DEF();
        }
      }
      break;

      case 'a':
      case 'A':
      case 'e':
      case 'E':
      case 'f':
      case 'F':
      case 'g':
      case 'G':
        assert(VSTR__FMT_ANAL_ZERO &&
               (spec->field_width_usr || !spec->field_width));
        assert(VSTR__FMT_ANAL_ZERO &&
               ((spec->flags & PREC_USR) || !spec->precision));
        if (!vstr__add_fmt_dbl(base, pos_diff, spec))
          goto failed_alloc;
        ASSERT_NO_SWITCH_DEF();
    }

    spec = spec->next;
  }

  end->next = base->conf->vstr__fmt_spec_make;
  base->conf->vstr__fmt_spec_make = beg;

  return (TRUE);

 failed_alloc:
 failed_user:
  end->next = base->conf->vstr__fmt_spec_make;
  base->conf->vstr__fmt_spec_make = beg;

  return (FALSE);
}

#undef VSTR__FMT_N_PTR
#define VSTR__FMT_N_PTR(x, y) \
          case x: \
            u.data_ptr = va_arg(ap, y *)

#define VSTR__FMT_ARG_NUM(sT, asT, auT, memb)                           \
    if (spec->flags & SIGN)                                             \
    {                                                                   \
      sT tmp = va_arg(ap, asT);                                         \
      VSTR__FMT_ABS_NUM(u.memb, tmp);                                   \
    }                                                                   \
    else                                                                \
      u.memb = va_arg(ap, auT);                                         \
    if (!u.memb) spec->flags |= NUM_IS_ZERO;                            \
    break

static int vstr__fmt_fillin_spec(Vstr_conf *conf, va_list ap, int have_dollars)
{
  struct Vstr__fmt_spec *beg = conf->vstr__fmt_spec_list_beg;
  unsigned int count = 0;
  unsigned int need_to_fin_now = FALSE;

  while (beg)
  {
    struct Vstr__fmt_spec *spec = NULL;
    int done = FALSE;
    union Vstr__fmt_sp_un u;

    ++count;
    ASSERT(beg);

    while (beg &&
           !beg->field_width_param &&
           !beg->precision_param &&
           !beg->main_param)
      beg = beg->next;
    
    ASSERT_RET(!(beg && need_to_fin_now), FALSE); /* incomplete spec */

    spec = beg;
    while (spec)
    {
      if (count == spec->field_width_param)
      {
        int tmp_fw_p = 0;
        if (!done) u.data_i = va_arg(ap, int);
        done = TRUE;

        assert(spec->field_width_usr);

        tmp_fw_p = u.data_i;
        if (tmp_fw_p < 0) /* negative field width == flag '-' */
        {
          spec->flags |= LEFT;
          VSTR__FMT_S2U_NUM(spec->field_width, tmp_fw_p);
        }
        else
          spec->field_width = tmp_fw_p;
        
        spec->field_width_param = 0;
        if (!have_dollars)
          break;
      }

      if (count == spec->precision_param)
      {
        int tmp_fw_p = 0;
        if (!done) u.data_i = va_arg(ap, int);
        done = TRUE;

        assert(spec->flags & PREC_USR);

        tmp_fw_p = u.data_i;
        if (tmp_fw_p < 0) /* negative precision == pretend one wasn't given */
          spec->flags &= ~PREC_USR;
        else
          spec->precision = tmp_fw_p;
        spec->precision_param = 0;
        if (!have_dollars)
          break;
      }
      
      if (count == spec->main_param)
      {
        if (!done)
          switch (spec->fmt_code)
          {
            case 'c':
              u.data_c = va_arg(ap, int);
              break;

            case 'C':
              u.data_wint = va_arg(ap, wint_t);
              break;

            case 'm':
              break;

            case 's':
              u.data_ptr = va_arg(ap, char *);
              break;

            case 'S':
              u.data_ptr = va_arg(ap, wchar_t *);
              break;

            case 'd':
            case 'i':
            case 'u':
            case 'X':
            case 'x':
            case 'o':
              switch (spec->int_type)
              {                        
                case VSTR_TYPE_FMT_UCHAR:
                  VSTR__FMT_ARG_NUM(signed char, int, unsigned int, data_c);
                case VSTR_TYPE_FMT_USHORT:
                  VSTR__FMT_ARG_NUM(signed short, int, unsigned int, data_s);
                case VSTR_TYPE_FMT_UINT:
                  VSTR__FMT_ARG_NUM(int, int, unsigned int, data_i);
                case VSTR_TYPE_FMT_ULONG:
                  VSTR__FMT_ARG_NUM(long, long, unsigned long, data_l);
                case VSTR_TYPE_FMT_ULONG_LONG:
                  VSTR__FMT_ARG_NUM(Vstr__long_long, Vstr__long_long,
                                    Vstr__unsigned_long_long, data_L);
                case VSTR_TYPE_FMT_SIZE_T:
                  VSTR__FMT_ARG_NUM(ssize_t, ssize_t, size_t, data_sz);
                case VSTR_TYPE_FMT_UINTMAX_T:
                  VSTR__FMT_ARG_NUM(intmax_t, intmax_t, uintmax_t, data_m);
                  
                case VSTR_TYPE_FMT_PTRDIFF_T:
                  if (1)
                  { /* no unsigned type ... */
                    /* FIXME: volatile for bug in gcc-2.95.x */
                    volatile ptrdiff_t ttmp = va_arg(ap, ptrdiff_t);
                    volatile intmax_t jtmp = ttmp;

                    spec->data_t = ttmp;
                    VSTR__FMT_ABS_NUM(u.data_m, jtmp);
                  }
                  if (!u.data_m) spec->flags |= NUM_IS_ZERO;
                  spec->int_type = VSTR_TYPE_FMT_UINTMAX_T;

                  ASSERT_NO_SWITCH_DEF();
              }
              break;
              
            case 'p':
              u.data_ptr = va_arg(ap, void *);
              break;
            
            case 'n':
              switch (spec->int_type)
              {
                VSTR__FMT_N_PTR(VSTR_TYPE_FMT_UCHAR, signed char);        break;
                VSTR__FMT_N_PTR(VSTR_TYPE_FMT_USHORT, short);             break;
                VSTR__FMT_N_PTR(VSTR_TYPE_FMT_UINT, int);                 break;
                VSTR__FMT_N_PTR(VSTR_TYPE_FMT_ULONG, long);               break;
                VSTR__FMT_N_PTR(VSTR_TYPE_FMT_ULONG_LONG, Vstr__long_long);
                break;
                VSTR__FMT_N_PTR(VSTR_TYPE_FMT_SIZE_T, ssize_t);           break;
                VSTR__FMT_N_PTR(VSTR_TYPE_FMT_UINTMAX_T, intmax_t);       break;
                VSTR__FMT_N_PTR(VSTR_TYPE_FMT_PTRDIFF_T, ptrdiff_t);
                ASSERT_NO_SWITCH_DEF();
              }
              break;

            case 'a': /* print like [-]x.xxxpxx -- in hex using abcdef */
            case 'A': /* print like [-]x.xxxPxx -- in hex using ABCDEF */
            case 'e': /* print like [-]x.xxxexx */
            case 'E': /* use big E instead */
            case 'f': /* print like an int */
            case 'F': /* print like an int - upper case infinity/nan */
            case 'g': /* use the smallest of e and f */
            case 'G': /* use the smallest of E and F */
              if (spec->int_type == VSTR_TYPE_FMT_ULONG_LONG)
                u.data_Ld = va_arg(ap, long double);
              else
                u.data_d = va_arg(ap, double);
              
              ASSERT_NO_SWITCH_DEF();
          }
        done = TRUE;
        spec->u = u;
        spec->main_param = 0;
        
        if (!have_dollars)
          break;
      }
      
      spec = spec->next;
    }
    
    if (!done)
      need_to_fin_now = 1;
  }
  
 return (TRUE);
}
#undef VSTR__FMT_N_PTR
#undef VSTR__FMT_ARG_NUM

static const char *vstr__add_fmt_usr_esc(Vstr_conf *conf,
                                         const char *fmt,
                                         struct Vstr__fmt_spec *spec,
                                         unsigned int *passed_params)
{
  unsigned int params = *passed_params;
  Vstr__fmt_usr_name_node *node = NULL;
  unsigned int have_i18n_args = spec->main_param;
  
  if ((node = vstr__fmt_usr_match(conf, fmt)))
  {
    unsigned int scan = 0;

    spec->usr_spec = node;

    while (TRUE)
    {
      int have_arg = TRUE;

      spec->escape_usr = TRUE;

      switch (node->types[scan])
      {
        case VSTR_TYPE_FMT_END:
          spec->fmt_code = 0;
          have_arg = FALSE;
          /* it's possible, but stupid, with i18n */
          ASSERT(!scan && !spec->main_param);
          spec->main_param = 0;
          break;
        case VSTR_TYPE_FMT_INT:
          spec->flags |= SIGN;
          spec->fmt_code = 'd';
          break;
        case VSTR_TYPE_FMT_UINT:
          spec->fmt_code = 'u';
          break;
        case VSTR_TYPE_FMT_LONG:
          spec->int_type = VSTR_TYPE_FMT_ULONG;
          spec->flags |= SIGN;
          spec->fmt_code = 'd';
          break;
        case VSTR_TYPE_FMT_ULONG:
          spec->int_type = VSTR_TYPE_FMT_ULONG;
          spec->fmt_code = 'u';
          break;
        case VSTR_TYPE_FMT_LONG_LONG:
          spec->int_type = VSTR_TYPE_FMT_ULONG_LONG;
          spec->flags |= SIGN;
          spec->fmt_code = 'd';
          break;
        case VSTR_TYPE_FMT_ULONG_LONG:
          spec->int_type = VSTR_TYPE_FMT_ULONG_LONG;
          spec->fmt_code = 'u';
          break;
        case VSTR_TYPE_FMT_SSIZE_T:
          spec->int_type = VSTR_TYPE_FMT_SIZE_T;
          spec->flags |= SIGN;
          spec->fmt_code = 'd';
          break;
        case VSTR_TYPE_FMT_SIZE_T:
          spec->int_type = VSTR_TYPE_FMT_SIZE_T;
          spec->fmt_code = 'u';
          break;
        case VSTR_TYPE_FMT_PTRDIFF_T:
          spec->int_type = VSTR_TYPE_FMT_PTRDIFF_T;
          spec->flags |= SIGN;
          spec->fmt_code = 'd';
          break;
        case VSTR_TYPE_FMT_INTMAX_T:
          spec->int_type = VSTR_TYPE_FMT_UINTMAX_T;
          spec->flags |= SIGN;
          spec->fmt_code = 'd';
          break;
        case VSTR_TYPE_FMT_UINTMAX_T:
          spec->int_type = VSTR_TYPE_FMT_UINTMAX_T;
          spec->fmt_code = 'u';
          break;
        case VSTR_TYPE_FMT_DOUBLE:
          spec->fmt_code = 'f';
          break;
        case VSTR_TYPE_FMT_DOUBLE_LONG:
          spec->int_type = VSTR_TYPE_FMT_ULONG_LONG;
          spec->fmt_code = 'f';
          break;
        case VSTR_TYPE_FMT_PTR_VOID:
          spec->fmt_code = 'p';
          break;
        case VSTR_TYPE_FMT_PTR_CHAR:
          spec->fmt_code = 's';
          break;
        case VSTR_TYPE_FMT_PTR_WCHAR_T:
          spec->fmt_code = 'S';
          break;
        case VSTR_TYPE_FMT_ERRNO:
          /* it's possible, but stupid, with i18n */
          ASSERT(!spec->main_param);
          spec->main_param = 0;
          spec->fmt_code = 0;
          have_arg = FALSE;
          break;
          
        case VSTR_TYPE_FMT_PTR_SIGNED_CHAR:
          spec->fmt_code = 'n'; spec->int_type = VSTR_TYPE_FMT_UCHAR;
          break;
        case VSTR_TYPE_FMT_PTR_SHORT:
          spec->fmt_code = 'n'; spec->int_type = VSTR_TYPE_FMT_USHORT;
          break;
        case VSTR_TYPE_FMT_PTR_INT:
          spec->fmt_code = 'n'; spec->int_type = VSTR_TYPE_FMT_UINT;
          break;
        case VSTR_TYPE_FMT_PTR_LONG:
          spec->fmt_code = 'n'; spec->int_type = VSTR_TYPE_FMT_ULONG;
          break;
        case VSTR_TYPE_FMT_PTR_LONG_LONG:
          spec->fmt_code = 'n'; spec->int_type = VSTR_TYPE_FMT_ULONG_LONG;
          break;
        case VSTR_TYPE_FMT_PTR_SSIZE_T:
          spec->fmt_code = 'n'; spec->int_type = VSTR_TYPE_FMT_SIZE_T;
          break;
        case VSTR_TYPE_FMT_PTR_PTRDIFF_T:
          spec->fmt_code = 'n'; spec->int_type = VSTR_TYPE_FMT_PTRDIFF_T;
          break;
        case VSTR_TYPE_FMT_PTR_INTMAX_T:
          spec->fmt_code = 'n'; spec->int_type = VSTR_TYPE_FMT_UINTMAX_T;
          
          ASSERT_NO_SWITCH_DEF();
      }

      /* custom formatters have multiple args, they _must_ follow each other
       * so if the first is set, set the rest based off that */
      if (have_i18n_args && have_arg)
        spec->main_param = have_i18n_args++;
      VSTR__FMT_MV_SPEC(conf, have_arg);
      
      /* allow _END to be the only thing passed */
      if (!node->types[scan] || !node->types[++scan])
        break;

      if (!conf->vstr__fmt_spec_make && !vstr__fmt_add_spec(conf))
        goto failed_alloc;
      spec = conf->vstr__fmt_spec_make;
      vstr__fmt_init_spec(spec);
    }

    *passed_params = params;

    return (fmt + node->name_len);
  }
  
  return (fmt);

 failed_alloc:
  return (NULL);
}

static const char *vstr__add_fmt_spec(const char *fmt,
                                      struct Vstr__fmt_spec *spec,
                                      unsigned int *params,
                                      unsigned int *have_dollars)
{
  int tmp_num = 0;

  /* get i18n param number */
  if (VSTR__ADD_FMT_ISDIGIT(*fmt) && (*fmt != '0'))
  {
    tmp_num = VSTR__ADD_FMT_STRTOL(fmt);

    if (*fmt != '$')
      goto use_field_width;

    ++fmt;
    *have_dollars = TRUE;
    spec->main_param = tmp_num;
  }

  /* process flags */
  while (TRUE)
  {
    switch (*fmt)
    {
      case '-':  spec->flags |= LEFT;         break;
      case '+':  spec->flags |= PLUS;         break;
      case ' ':  spec->flags |= SPACE;        break;
      case '#':  spec->flags |= SPECIAL;      break;
      case '0':  spec->flags |= ZEROPAD;      break;
      case '\'': spec->flags |= THOUSAND_SEP; break;
      case 'I':  spec->flags |= ALT_DIGITS;   break;

      default:
        goto got_flags;
    }
    ++fmt;
  }
 got_flags:
  
  /* get field width */
  if (VSTR__ADD_FMT_ISDIGIT(*fmt))
  {
    tmp_num = VSTR__ADD_FMT_STRTOL(fmt);

   use_field_width:
    spec->field_width_usr = TRUE;
    spec->field_width = tmp_num;
    ASSERT(tmp_num >= 0);
  }
  else if (*fmt == '*')
  {
    const char *dollar_start = fmt;

    spec->field_width_usr = TRUE;

    ++fmt;
    tmp_num = 0;
    if (VSTR__ADD_FMT_ISDIGIT(*fmt))
      tmp_num = VSTR__ADD_FMT_STRTOL(fmt);

    if (*fmt != '$')
    {
      fmt = dollar_start + 1;
      spec->field_width_param = ++*params;
    }
    else
    {
      ++fmt;
      spec->field_width_param = tmp_num;
    }
  }

  /* get the precision */
  if (*fmt == '.')
  {
    spec->flags |= PREC_USR;

    ++fmt;
    if (VSTR__ADD_FMT_ISDIGIT(*fmt))
    {
      tmp_num = VSTR__ADD_FMT_STRTOL(fmt);

      spec->precision = tmp_num;
      ASSERT(tmp_num >= 0);
    }
    else if (*fmt == '*')
    {
      const char *dollar_start = fmt;
      ++fmt;

      tmp_num = 0;
      if (VSTR__ADD_FMT_ISDIGIT(*fmt))
        tmp_num = VSTR__ADD_FMT_STRTOL(fmt);

      if (*fmt != '$')
      {
        fmt = dollar_start + 1;
        spec->precision_param = ++*params;
      }
      else
      {
        ++fmt;
        spec->precision_param = tmp_num;
      }
    }
  }

  return (fmt);
}

static size_t vstr__add_vfmt(Vstr_base *base, size_t pos, unsigned int userfmt,
                             const char *fmt, va_list ap)
{
  int sve_errno = errno;
  size_t start_pos = pos + 1;
  size_t orig_len = 0;
  size_t pos_diff = 0;
  unsigned int params = 0;
  unsigned int have_dollars = FALSE; /* have posix %2$d etc. stuff */
  unsigned char fmt_usr_escape = 0;
  unsigned int orig_malloc_bad = FALSE;
  
  ASSERT_RET(!(!base || !fmt || (pos > base->len)), 0);

  orig_len = base->len;
  
  /* so things can use this as a flag */
  orig_malloc_bad = base->conf->malloc_bad;
  base->conf->malloc_bad = FALSE;

  /* this will be correct as you add chars */
  pos_diff = base->len - pos;

  if (userfmt)
    fmt_usr_escape = base->conf->fmt_usr_escape;
  
  while (*fmt)
  {
    const char *fmt_orig = fmt;
    struct Vstr__fmt_spec *spec = NULL;
    
    if (!base->conf->vstr__fmt_spec_make && !vstr__fmt_add_spec(base->conf))
      goto failed_alloc;
    
    spec = base->conf->vstr__fmt_spec_make;
    vstr__fmt_init_spec(spec);

    assert(VSTR__FMT_ANAL_ZERO &&
           (spec->field_width_usr || !spec->field_width));
    assert(VSTR__FMT_ANAL_ZERO &&
           ((spec->flags & PREC_USR) || !spec->precision));

    if ((*fmt != '%') && (*fmt != fmt_usr_escape))
    {
      char *next_escape = strchr(fmt, '%');

      spec->fmt_code = 's';
      spec->u.data_ptr = (char *)fmt;

      if (fmt_usr_escape)
      { /* find first of the two escapes */
        char *next_usr_escape = strchr(fmt, fmt_usr_escape);
        if (next_usr_escape &&
            (!next_escape || (next_usr_escape < next_escape)))
          next_escape = next_usr_escape;
      }

      if (next_escape)
      {
        size_t len = next_escape - fmt;
        spec->precision = len;
        spec->flags |= PREC_USR;
        fmt = fmt + len;
      }
      else
        fmt = "";

      VSTR__FMT_MV_SPEC(base->conf, FALSE);

      continue;
    }

    if (fmt[0] == fmt[1])
    {
      spec->u.data_c = fmt[0];
      spec->fmt_code = 'c';
      VSTR__FMT_MV_SPEC(base->conf, FALSE);
      fmt += 2; /* skip escs */
      continue;
    }
    assert(fmt_orig == fmt);
    ++fmt; /* skip esc */

    fmt = vstr__add_fmt_spec(fmt, spec, &params, &have_dollars);

    assert(VSTR__FMT_ANAL_ZERO &&
           (spec->field_width_usr || !spec->field_width));
    assert(VSTR__FMT_ANAL_ZERO &&
           ((spec->flags & PREC_USR) || !spec->precision));
    
    if (fmt_usr_escape && (*fmt_orig == fmt_usr_escape))
    {
      const char *fmt_end = vstr__add_fmt_usr_esc(base->conf, fmt,
                                                  spec, &params);
      
      if (!fmt_end)
        goto failed_alloc;
      else if (fmt_end == fmt)
      {
        if (fmt_usr_escape == '%')
          goto vstr__fmt_sys_escapes;
        
        assert(FALSE); /* $$ etc. is already done before here */
        fmt = ""; /* should end because we don't know if the types are
                   * screwed up */
      }
      else
        fmt = fmt_end;
      continue;
    }
    
   vstr__fmt_sys_escapes:
    /* get width of type */
    switch (*fmt)
    {
      case 'h':
        ++fmt;
        if (*fmt == 'h')
        {
          ++fmt;
          spec->int_type = VSTR_TYPE_FMT_UCHAR;
        }
        else
          spec->int_type = VSTR_TYPE_FMT_USHORT;
        break;
      case 'l':
        ++fmt;
        if (*fmt == 'l')
        {
          ++fmt;
          spec->int_type = VSTR_TYPE_FMT_ULONG_LONG;
        }
        else
          spec->int_type = VSTR_TYPE_FMT_ULONG;
        break;
      case 'L': ++fmt; spec->int_type = VSTR_TYPE_FMT_ULONG_LONG; break;
      case 'z': ++fmt; spec->int_type = VSTR_TYPE_FMT_SIZE_T;     break;
      case 't': ++fmt; spec->int_type = VSTR_TYPE_FMT_PTRDIFF_T;  break;
      case 'j': ++fmt; spec->int_type = VSTR_TYPE_FMT_UINTMAX_T;  break;

      default:
        break;
    }

    spec->fmt_code = *fmt;
    switch (*fmt)
    {
      case 'c':
        if (spec->int_type != VSTR_TYPE_FMT_ULONG) /* %lc == %C */
          break;
        /* FALL THROUGH */

      case 'C':
        spec->fmt_code = 'C';
        break;

      case 'm':
        ASSERT(!spec->main_param); /* it's possible, but stupid, with i18n */
        spec->main_param = 0;
        break;

      case 's':
        if (spec->int_type != VSTR_TYPE_FMT_ULONG) /* %ls == %S */
          break;
        /* FALL THROUGH */

      case 'S':
        spec->fmt_code = 'S';
        break;

      case 'd':
      case 'i':
        spec->flags |= SIGN;
      case 'u':
        break;

      case 'X':
        spec->flags |= LARGE;
      case 'x':
        spec->num_base = 16;
        break;

      case 'o':
        spec->num_base = 8;
        break;

      case 'p':
        spec->num_base = 16;
        spec->int_type = VSTR_TYPE_FMT_PTR_VOID;
        spec->flags |= SPECIAL;
        break;

      case 'n':
        break;

      case 'A': /* print like [-]x.xxxPxx -- in hex using ABCDEF */
      case 'E': /* use big E instead */
      case 'F': /* print like an int - upper case infinity/nan */
      case 'G': /* use the smallest of E and F */
        spec->flags |= LARGE;
      case 'a': /* print like [-]x.xxxpxx -- in hex using abcdef */
      case 'e': /* print like [-]x.xxxexx */
      case 'f': /* print like an int */
      case 'g': /* use the smallest of e and f */
        break;

      default:
        assert(FALSE);
        fmt = "";
        continue;
    }

    VSTR__FMT_MV_SPEC(base->conf, TRUE);
    ++fmt;
  }
  
  if (!vstr__fmt_fillin_spec(base->conf, ap, have_dollars))
    goto failed_alloc;
  
  errno = sve_errno;
  if (!vstr__fmt_write_spec(base, pos_diff, orig_len, sve_errno))
    goto failed_write_spec;
  
  /* restore the original state of the bad malloc flag... */
  base->conf->malloc_bad = orig_malloc_bad;
  
  return (base->len - orig_len);

 failed_write_spec:

  if (base->len - orig_len)
    vstr_del(base, start_pos, base->len - orig_len);

  if (0) /* already done in write_spec */
  {
   failed_alloc:
    if (base->conf->vstr__fmt_spec_list_end)
      base->conf->vstr__fmt_spec_list_end->next = base->conf->vstr__fmt_spec_make;
    base->conf->vstr__fmt_spec_make = base->conf->vstr__fmt_spec_list_beg;
    base->conf->vstr__fmt_spec_list_beg = NULL;
    base->conf->vstr__fmt_spec_list_end = NULL;
  }
  base->conf->malloc_bad |= orig_malloc_bad;

  return (0);
}

size_t vstr_add_vfmt(Vstr_base *base, size_t pos,
                        const char *fmt, va_list ap)
{
  return (vstr__add_vfmt(base, pos, TRUE,  fmt, ap));
}

size_t vstr_add_vsysfmt(Vstr_base *base, size_t pos,
                           const char *fmt, va_list ap)
{
  return (vstr__add_vfmt(base, pos, FALSE, fmt, ap));
}

size_t vstr_add_fmt(Vstr_base *base, size_t pos, const char *fmt, ...)
{
  size_t len = 0;
  va_list ap;
  
  va_start(ap, fmt);

  len =   vstr__add_vfmt(base, pos, TRUE,  fmt, ap);

  va_end(ap);

  return (len);
}

size_t vstr_add_sysfmt(Vstr_base *base, size_t pos, const char *fmt, ...)
{
  size_t len = 0;
  va_list ap;

  va_start(ap, fmt);

  len =   vstr__add_vfmt(base, pos, FALSE, fmt, ap);

  va_end(ap);

  return (len);
}

#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(add_fmt);
VSTR__SYM_ALIAS(add_sysfmt);
VSTR__SYM_ALIAS(add_vfmt);
VSTR__SYM_ALIAS(add_vsysfmt);
