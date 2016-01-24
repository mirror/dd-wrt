#define VSTR_PARSE_C
/*
 *  Copyright (C) 2002, 2003, 2004  James Antill
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
/* functions for parsing data out of the Vstr */
#include "main.h"

#ifdef HAVE_ASCII_ALPHA
# define VSTR__PARSE_NUM_USE_LOCAL 0
#else
# define VSTR__PARSE_NUM_USE_LOCAL 1
#endif


/* basically uses: [ ]*[-+](0x|0X|0)[0-9a-z_]+ */
static int vstr__parse_num_beg(const Vstr_base *base,
                               size_t *passed_pos, size_t *passed_len,
                               unsigned int flags, int *is_neg, int *is_zeroed,
                               unsigned int *err)
{
  size_t pos = *passed_pos;
  size_t len = *passed_len;
  unsigned int num_base = flags & VSTR__MASK_PARSE_NUM_BASE;
  int auto_base = FALSE;
  char num_0 = '0';
  char let_x_low = 'x';
  char let_X_high = 'X';
  char let_b_low = 'b';
  char let_B_high = 'B';
  char sym_minus = '-';
  char sym_plus = '+';
  char sym_space = ' ';
  size_t tmp = 0;

  if (!num_base)
    auto_base = TRUE;
  else if (num_base > 36)
    num_base = 36;
  else if (num_base == 1)
    ++num_base;

  assert(!(auto_base && (flags & VSTR_FLAG_PARSE_NUM_NO_BEG_ZERO)));

  if (!(flags & VSTR_FLAG_PARSE_NUM_LOCAL))
  {
    num_0 = 0x30;
    let_x_low = 0x78;
    let_X_high = 0x58;
    let_b_low = 0x62;
    let_B_high = 0x42;
    sym_minus = 0x2D;
    sym_plus = 0x2B;
    sym_space = 0x20;
  }

  if (flags & VSTR_FLAG_PARSE_NUM_SPACE)
  {
    tmp = vstr_spn_chrs_fwd(base, pos, len, &sym_space, 1);
    if (tmp >= len)
    {
      *err = VSTR_TYPE_PARSE_NUM_ERR_ONLY_S;
      return (0);
    }

    pos += tmp;
    len -= tmp;
  }

  if (!(flags & VSTR_FLAG_PARSE_NUM_NO_BEG_PM))
  {
    tmp = vstr_spn_chrs_fwd(base, pos, len, &sym_minus, 1);
    if (tmp > 1)
    {
      *err = VSTR_TYPE_PARSE_NUM_ERR_OOB;
      return (0);
    }
    else if (!tmp)
    {
      tmp = vstr_spn_chrs_fwd(base, pos, len, &sym_plus, 1);
      if (tmp > 1)
      {
        *err = VSTR_TYPE_PARSE_NUM_ERR_OOB;
        return (0);
      }
    }
    else
      *is_neg = TRUE;

    pos += tmp;
    len -= tmp;
  }

  if (!len)
  {
    *err = VSTR_TYPE_PARSE_NUM_ERR_ONLY_SPM;
    return (0);
  }

  tmp = vstr_spn_chrs_fwd(base, pos, len, &num_0, 1);
  *is_zeroed = !!tmp;
  if ((tmp == 1) && (auto_base || (num_base == 16) || (num_base ==  2)))
  {
    char xX[2];

    if (tmp == len)
    {
      *passed_len = 0;
      return (1);
    }

    pos += tmp;
    len -= tmp;

    xX[0] = let_x_low;
    xX[1] = let_X_high;
    tmp = vstr_spn_chrs_fwd(base, pos, len, xX, 2);

    if (tmp > 1)
    { /* It's a 0 */
      *err = VSTR_TYPE_PARSE_NUM_ERR_OOB;
      *passed_len = len;
      return (1);
    }
    if (tmp == 1)
    {
      if (auto_base)
        num_base = 16;

      pos += tmp;
      len -= tmp;

      if (!len)
      {
        *passed_pos = pos + len;
        *passed_len = 0;
        *err = VSTR_TYPE_PARSE_NUM_ERR_ONLY_SPMX;
        return (0);
      }

      tmp = vstr_spn_chrs_fwd(base, pos, len, &num_0, 1);
    }
    else
    {
      xX[0] = let_b_low;
      xX[1] = let_B_high;
      tmp = vstr_spn_chrs_fwd(base, pos, len, xX, 2);

      if (tmp > 1)
      { /* It's a 0 */
        *err = VSTR_TYPE_PARSE_NUM_ERR_OOB;
        *passed_len = len;
        return (1);
      }
      if (tmp == 1)
      {
        if (auto_base)
          num_base =  2;

        pos += tmp;
        len -= tmp;

        if (!len)
        {
          *passed_pos = pos + len;
          *passed_len = 0;
          *err = VSTR_TYPE_PARSE_NUM_ERR_ONLY_SPMX;
          return (0);
        }

        tmp = vstr_spn_chrs_fwd(base, pos, len, &num_0, 1);
      }
      else if (auto_base)
        num_base =  8;
    }
  }
  else if (tmp && auto_base)
  {
    num_base =  8;
    --tmp;
  }
  else if (auto_base)
    num_base = 10;

  if (tmp && (flags & VSTR_FLAG_PARSE_NUM_NO_BEG_ZERO))
  {
    *passed_len = len - 1;
    if ((tmp != 1) || (len != 1))
      *err = VSTR_TYPE_PARSE_NUM_ERR_BEG_ZERO;

    return (1);
  }

  if (tmp == len)
  { /* It's a 0 */
    *passed_len = 0;
    return (1);
  }

  pos += tmp;
  len -= tmp;

  *passed_pos = pos;
  *passed_len = len;

  return (num_base);
}

#define VSTR__PARSE_NUM_BEG_A(num_type)                                 \
    unsigned int dummy_err;                                             \
    num_type ret = 0;                                                   \
    unsigned int num_base = 0;                                          \
    int is_neg = FALSE;                                                 \
    int is_zeroed = FALSE;                                              \
    size_t orig_len = len;                                              \
    char sym_sep = '_';                                                 \
    char ascii_num_end = 0x39;                                          \
    char ascii_let_low_end = 0x7A;                                      \
    char ascii_let_high_end = 0x5A;                                     \
    char local_num_end = '9';                                           \
    static const char local_let_low[]  = "abcdefghijklmnopqrstuvwxyz";  \
    static const char local_let_high[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";  \
                                                                        \
    if (ret_len) *ret_len = 0;                                          \
    if (!err) err = &dummy_err;                                         \
    *err = VSTR_TYPE_PARSE_NUM_ERR_NONE;                                \
                                                                        \
    if (!(num_base = vstr__parse_num_beg(base, &pos, &len, flags,       \
                                         &is_neg, &is_zeroed, err)))    \
      return (0);                                                       \
    else if (num_base == 1) goto ret_num_len

#define VSTR__PARSE_NUM_BEG_S(num_type)                         \
    VSTR__PARSE_NUM_BEG_A(num_type);                            \
                                                                \
    if (is_neg && (flags & VSTR_FLAG_PARSE_NUM_NO_NEGATIVE))    \
    {                                                           \
      *err = VSTR_TYPE_PARSE_NUM_ERR_NEGATIVE;                  \
      return (0);                                               \
    }
       
#define VSTR__PARSE_NUM_BEG_U(num_type)         \
    VSTR__PARSE_NUM_BEG_A(num_type);            \
                                                \
    if (is_neg)                                 \
    {                                           \
      *err = VSTR_TYPE_PARSE_NUM_ERR_NEGATIVE;  \
      return (0);                               \
    }

#define VSTR__PARSE_NUM_ASCII() do {                    \
      if (!(flags & VSTR_FLAG_PARSE_NUM_LOCAL))         \
      {                                                 \
        sym_sep = 0x5F;                                 \
                                                        \
        if (num_base <= 10)                             \
          ascii_num_end = 0x30 + num_base - 1;          \
        else if (num_base > 10)                         \
        {                                               \
          ascii_let_low_end  = 0x61 + (num_base - 11);  \
          ascii_let_high_end = 0x41 + (num_base - 11);  \
        }                                               \
      }                                                 \
      else if (num_base <= 10)                          \
        local_num_end = '0' + num_base - 1;             \
} while (FALSE)

#define VSTR__PARSE_NUM_BIN_CALC_NUM(num_type) do {                     \
      num_type old_ret = ret;                                           \
                                                                        \
      ret = (ret * num_base) + add_num;                                 \
      if ((flags & VSTR_FLAG_PARSE_NUM_OVERFLOW) &&                     \
          (((ret - add_num) / num_base) != old_ret))                    \
      {                                                                 \
        *err = VSTR_TYPE_PARSE_NUM_ERR_OVERFLOW;                        \
        break;                                                          \
      }                                                                 \
    } while (FALSE)                                                     \

#define VSTR__PARSE_NUM_LOOP_BEG() do {                                 \
      int done_once = is_zeroed;                                        \
      Vstr_iter iter[1];                                                \
      int iter_ret = vstr_iter_fwd_beg(base, pos, len, iter);           \
                                                                        \
      ASSERT(iter_ret);                                                 \
      while (len)                                                       \
      {                                                                 \
        char scan = vstr_iter_fwd_chr(iter, NULL);                      \
        const char *end = NULL;                                         \
        unsigned int add_num = 0;                                       \
                                                                        \
        if (done_once && (scan == sym_sep))                             \
        {                                                               \
          if (!(flags & VSTR_FLAG_PARSE_NUM_SEP)) break;                \
          --len;                                                        \
          continue;                                                     \
        }                                                               \
        else if (VSTR__PARSE_NUM_USE_LOCAL &&                           \
                 (flags & VSTR_FLAG_PARSE_NUM_LOCAL))                   \
        {                                                               \
          if ((scan >= '0') && (scan <= local_num_end))                 \
            add_num = (scan - '0');                                     \
          else if (num_base <= 10)                                      \
            break;                                                      \
          else if ((end = vstr_wrap_memchr(local_let_low,  scan,        \
                                           num_base - 10)))             \
            add_num = 10 + (end - local_let_low);                       \
          else if ((end = vstr_wrap_memchr(local_let_high, scan,        \
                                           num_base - 10)))             \
            add_num = 10 + (end - local_let_high);                      \
          else                                                          \
            break;                                                      \
        }                                                               \
        else                                                            \
        {                                                               \
          if (scan < (char)0x30) break;                                 \
                                                                        \
          if (scan <= ascii_num_end)                                    \
            add_num = (scan - (char)0x30);                              \
          else if (num_base <= 10)                                      \
            break;                                                      \
          else if ((scan >= (char)0x41) && (scan <= ascii_let_high_end)) \
            add_num = 10 + (scan - (char)0x41);                         \
          else if ((scan >= 0x61) && (scan <= ascii_let_low_end))       \
            add_num = 10 + (scan - (char)0x61);                         \
          else                                                          \
            break;                                                      \
        }                                                               \

#define VSTR__PARSE_NUM_LOOP_END()                                      \
        --len;                                                          \
        done_once = TRUE;                                               \
      }                                                                 \
} while (FALSE)

/* assume negative numbers can be one bigger than signed positive numbers */
#define VSTR__PARSE_NUM_END_S(num_max) do {                     \
      if ((flags & VSTR_FLAG_PARSE_NUM_OVERFLOW) &&             \
          ((ret - is_neg) > num_max))                           \
      {                                                         \
        *err = VSTR_TYPE_PARSE_NUM_ERR_OVERFLOW;                \
        ret = num_max + is_neg;                                 \
      }                                                         \
      if (len && !*err) *err = VSTR_TYPE_PARSE_NUM_ERR_OOB;     \
                                                                \
     ret_num_len:                                               \
      if (ret_len)                                              \
        *ret_len = (orig_len - len);                            \
                                                                \
      if (is_neg)                                               \
        return (-ret);                                          \
                                                                \
      return (ret);                                             \
    } while (FALSE)

#define VSTR__PARSE_NUM_END_U() do {                            \
      if (len && !*err) *err = VSTR_TYPE_PARSE_NUM_ERR_OOB;     \
                                                                \
     ret_num_len:                                               \
      if (ret_len)                                              \
        *ret_len = (orig_len - len);                            \
                                                                \
      return (ret);                                             \
    } while (FALSE)

#define VSTR__PARSE_NUM_SFUNC(num_type, num_max) do { \
      VSTR__PARSE_NUM_BEG_S(num_type);                \
      VSTR__PARSE_NUM_ASCII();                        \
      VSTR__PARSE_NUM_LOOP_BEG();                     \
      VSTR__PARSE_NUM_BIN_CALC_NUM(num_type);         \
      VSTR__PARSE_NUM_LOOP_END();                     \
      VSTR__PARSE_NUM_END_S(num_max);                 \
    } while (FALSE)

#define VSTR__PARSE_NUM_UFUNC(num_type) do {  \
      VSTR__PARSE_NUM_BEG_U(num_type);        \
      VSTR__PARSE_NUM_ASCII();                \
      VSTR__PARSE_NUM_LOOP_BEG();             \
      VSTR__PARSE_NUM_BIN_CALC_NUM(num_type); \
      VSTR__PARSE_NUM_LOOP_END();             \
      VSTR__PARSE_NUM_END_U();                \
    } while (FALSE)

void *vstr_parse_num(const Vstr_base *base, size_t pos, size_t len,
                     unsigned int flags, size_t *ret_len,
                     unsigned int *err,
                     void *(*func)(unsigned int, int, unsigned int *, void *),
                     void *data)
{
  VSTR__PARSE_NUM_BEG_S(void *);
  ret = data;
  VSTR__PARSE_NUM_ASCII();
  VSTR__PARSE_NUM_LOOP_BEG();

  if (is_neg) add_num = -add_num;
  
  if (!(ret = func(num_base, add_num, err, ret)) && !*err)
    return (NULL); /* mem error */
  
  VSTR__PARSE_NUM_LOOP_END();
  VSTR__PARSE_NUM_END_U();
}

short vstr_parse_short(const Vstr_base *base, size_t pos, size_t len,
                       unsigned int flags, size_t *ret_len,
                       unsigned int *err)
{ VSTR__PARSE_NUM_SFUNC(unsigned short, SHRT_MAX); }

unsigned short vstr_parse_ushort(const Vstr_base *base,
                                 size_t pos, size_t len,
                                 unsigned int flags, size_t *ret_len,
                                 unsigned int *err)
{ VSTR__PARSE_NUM_UFUNC(unsigned short); }

int vstr_parse_int(const Vstr_base *base, size_t pos, size_t len,
                   unsigned int flags, size_t *ret_len, unsigned int *err)
{ VSTR__PARSE_NUM_SFUNC(unsigned int, INT_MAX); }

unsigned int vstr_parse_uint(const Vstr_base *base, size_t pos, size_t len,
                             unsigned int flags, size_t *ret_len,
                             unsigned int *err)
{ VSTR__PARSE_NUM_UFUNC(unsigned int); }

long vstr_parse_long(const Vstr_base *base, size_t pos, size_t len,
                     unsigned int flags, size_t *ret_len,
                     unsigned int *err)
{ VSTR__PARSE_NUM_SFUNC(unsigned long, LONG_MAX); }

unsigned long vstr_parse_ulong(const Vstr_base *base, size_t pos, size_t len,
                               unsigned int flags, size_t *ret_len,
                               unsigned int *err)
{ VSTR__PARSE_NUM_UFUNC(unsigned long); }

intmax_t vstr_parse_intmax(const struct Vstr_base *base,
                           size_t pos, size_t len,
                           unsigned int flags, size_t *ret_len,
                           unsigned int *err)
{ VSTR__PARSE_NUM_SFUNC(uintmax_t, INTMAX_MAX); }

uintmax_t vstr_parse_uintmax(const struct Vstr_base *base,
                             size_t pos, size_t len,
                             unsigned int flags, size_t *ret_len,
                             unsigned int *err)
{ VSTR__PARSE_NUM_UFUNC(uintmax_t); }

static int vstr__parse_ipv4_netmask(const struct Vstr_base *base,
                                    size_t pos, size_t *passed_len,
                                    unsigned int flags,
                                    unsigned int num_flags,
                                    char sym_dot,
                                    unsigned int *cidr, unsigned int *err)
{
  int zero_rest = FALSE;
  size_t len = *passed_len;
  unsigned int scan = 0;

  while (len)
  {
    unsigned int tmp = 0;
    size_t num_len = 3;

    if (num_len > len)
      num_len = len;

    tmp = vstr_parse_uint(base, pos, num_len, 10 | num_flags,
                          &num_len, NULL);
    if (!num_len)
      break;

    pos += num_len;
    len -= num_len;

    if (zero_rest && tmp)
    {
      *err = VSTR_TYPE_PARSE_IPV4_ERR_NETMASK_OOB;
      return (FALSE);
    }
    else if (!zero_rest)
    {
      if (tmp > 255)
      {
        *err = VSTR_TYPE_PARSE_IPV4_ERR_NETMASK_OOB;
        return (FALSE);
      }

      if (tmp != 255)
      {
        *cidr = scan * 8;
        switch (tmp)
        {
          default:
            *err = VSTR_TYPE_PARSE_IPV4_ERR_NETMASK_OOB;
            return (FALSE);

          case 254: *cidr += 7; break;
          case 252: *cidr += 6; break;
          case 248: *cidr += 5; break;
          case 240: *cidr += 4; break;
          case 224: *cidr += 3; break;
          case 192: *cidr += 2; break;
          case 128: *cidr += 1; break;
          case   0:             break;
        }
        zero_rest = TRUE;
      }
    }

    ++scan;

    if (scan == 4)
      break;

    if (len && (vstr_export_chr(base, pos) != sym_dot))
      break;

    if (len)
    { /* skip the dot */
      ++pos;
      --len;
    }
  }
  if (!zero_rest)
    *cidr = scan * 8;
  
  if ((flags & VSTR_FLAG_PARSE_IPV4_NETMASK_FULL) && (scan != 4))
  {
    *err = VSTR_TYPE_PARSE_IPV4_ERR_NETMASK_FULL;
    return (FALSE);
  }

  *passed_len = len;

  return (TRUE);
}

static int vstr__parse_ipv4_cidr(const struct Vstr_base *base,
                                 size_t pos, size_t *passed_len,
                                 unsigned int flags,
                                 unsigned int num_flags,
                                 char sym_dot,
                                 unsigned int *cidr, unsigned int *err)
{
  size_t len = *passed_len;
  size_t num_len = 0;

  if (len)
    *cidr = vstr_parse_uint(base, pos, len, 10 | num_flags,
                            &num_len, NULL);
  if (num_len)
  {
    if ((flags & VSTR_FLAG_PARSE_IPV4_NETMASK) &&
        (*cidr > 32) && (len > num_len) &&
        (vstr_export_chr(base, pos + num_len) == sym_dot))
    {
      if (!vstr__parse_ipv4_netmask(base, pos, &len, flags, num_flags,
                                    sym_dot, cidr, err))
        return (FALSE);
    }
    else if (*cidr <= 32)
    {
      pos += num_len;
      len -= num_len;
    }
    else
    {
      *err = VSTR_TYPE_PARSE_IPV4_ERR_CIDR_OOB;
      return (FALSE);
    }
  }
  else if (flags & VSTR_FLAG_PARSE_IPV4_CIDR_FULL)
  {
    *err = VSTR_TYPE_PARSE_IPV4_ERR_CIDR_FULL;
    return (FALSE);
  }
  else
    *cidr = 32;

  *passed_len = len;

  return (TRUE);
}

int vstr_parse_ipv4(const struct Vstr_base *base,
                    size_t pos, size_t len,
                    unsigned char *ips, unsigned int *cidr,
                    unsigned int flags, size_t *ret_len, unsigned int *err)
{
  size_t orig_len = len;
  char sym_slash = 0x2F;
  char sym_dot = 0x2E;
  unsigned int num_flags = VSTR_FLAG_PARSE_NUM_NO_BEG_PM;
  unsigned int scan = 0;
  unsigned int dummy_err = 0;

  assert(ips);

  if (ret_len) *ret_len = 0;
  if (!err)
    err = &dummy_err;

  *err = 0;

  if (flags & VSTR_FLAG_PARSE_IPV4_LOCAL)
  {
    num_flags |= VSTR_FLAG_PARSE_NUM_LOCAL;
    sym_slash = '/';
    sym_dot = '.';
  }

  if (!(flags & VSTR_FLAG_PARSE_IPV4_ZEROS))
    num_flags |= VSTR_FLAG_PARSE_NUM_NO_BEG_ZERO;

  while (len)
  {
    unsigned int tmp = 0;
    size_t num_len = 3;

    if (num_len > len)
      num_len = len;

    tmp = vstr_parse_uint(base, pos, num_len, 10 | num_flags,
                          &num_len, NULL);
    if (!num_len)
      break;

    if (tmp > 255)
    {
      *err = VSTR_TYPE_PARSE_IPV4_ERR_IPV4_OOB;
      return (FALSE);
    }

    pos += num_len;
    len -= num_len;

    ips[scan++] = tmp;

    if (scan == 4)
      break;

    if (len && (vstr_export_chr(base, pos) == sym_slash))
      break;

    if (len && (vstr_export_chr(base, pos) != sym_dot))
      break;

    if (len)
    { /* skip the dot */
      ++pos;
      --len;
    }

    if (len && (vstr_export_chr(base, pos) == sym_slash))
      break;
  }

  if ((scan != 4) && (flags & VSTR_FLAG_PARSE_IPV4_FULL))
  {
    *err = VSTR_TYPE_PARSE_IPV4_ERR_IPV4_FULL;
    return (FALSE);
  }

  while (scan < 4)
    ips[scan++] = 0;

  if (cidr)
    *cidr = 32;

  if (len && (vstr_export_chr(base, pos) == sym_slash))
  {
    if (flags & VSTR_FLAG_PARSE_IPV4_CIDR)
    {
      ++pos;
      --len;

      if (!vstr__parse_ipv4_cidr(base, pos, &len, flags, num_flags, sym_dot,
                                 cidr, err))
        return (FALSE);
    }
    else if (flags & VSTR_FLAG_PARSE_IPV4_NETMASK)
    {
      ++pos;
      --len;

      if (!vstr__parse_ipv4_netmask(base, pos, &len, flags, num_flags, sym_dot,
                                    cidr, err))
        return (FALSE);
    }
  }

  if (len && (flags & VSTR_FLAG_PARSE_IPV4_ONLY))
    *err = VSTR_TYPE_PARSE_IPV4_ERR_ONLY;

  if (ret_len)
    *ret_len = orig_len - len;

  return (TRUE);
}

static int vstr__parse_ipv6_cidr(const struct Vstr_base *base,
                                 size_t pos, size_t *passed_len,
                                 unsigned int flags,
                                 unsigned int num_flags,
                                 unsigned int *cidr, unsigned int *err)
{
  size_t len = *passed_len;
  size_t num_len = 0;

  if (len)
    *cidr = vstr_parse_uint(base, pos, len, 10 | num_flags,
                            &num_len, NULL);
  if (num_len)
  {
    if (*cidr <= 128)
    {
      pos += num_len;
      len -= num_len;
    }
    else
    {
      *err = VSTR_TYPE_PARSE_IPV6_ERR_CIDR_OOB;
      return (FALSE);
    }
  }
  else if (flags & VSTR_FLAG_PARSE_IPV6_CIDR_FULL)
  {
    *err = VSTR_TYPE_PARSE_IPV6_ERR_CIDR_FULL;
    return (FALSE);
  }
  else
    *cidr = 128;

  *passed_len = len;

  return (TRUE);
}

/* see rfc 2373 */
/*
 *   1. 1 to 4 hexdigits in 8 groups: hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh
 *                                    h:h:h:h:h:h:h:h
 *   2. Any number of 0000 may be abbreviated as "::", but only once
 *   3. Note this means that "::" is a valid ipv6 address (all zeros)
 *   4. The last two words may be written as an IPv4 address ::1234:127.0.0.1
 */
int vstr_parse_ipv6(const struct Vstr_base *base,
                    size_t pos, size_t len,
                    unsigned int *ips, unsigned int *cidr,
                    unsigned int flags, size_t *ret_len, unsigned int *err)
{
  size_t orig_len = len;
  char sym_slash = 0x2F;
  char sym_colon = 0x3a;
  unsigned int num_flags = VSTR_FLAG_PARSE_NUM_NO_BEG_PM;
  unsigned int scan = 0;
  unsigned int dummy_err = 0;
  unsigned int off_null = 0;
  VSTR_SECTS_DECL(sects, 8);
  size_t num_len = 0;

  VSTR_SECTS_DECL_INIT(sects);

  assert(ips);
  vstr_wrap_memset(ips, 0, sizeof(int) * 8);

  if (ret_len) *ret_len = 0;
  if (!err)
    err = &dummy_err;

  *err = 0;

  if (len < 2)
  {
    *err = VSTR_TYPE_PARSE_IPV6_ERR_IPV6_FULL;
    return (FALSE);
  }

  if (flags & VSTR_FLAG_PARSE_IPV6_LOCAL)
  {
    num_flags |= VSTR_FLAG_PARSE_NUM_LOCAL;
    sym_slash = '/';
    sym_colon = ':';
  }

  {
    const int split_flags = (VSTR_FLAG_SPLIT_MID_NULL | VSTR_FLAG_SPLIT_NO_RET);
    char buf[2]; buf[0] = sym_colon; buf[1] = sym_colon;

    if (VSTR_CMP_BUF_EQ(base, pos, 2, buf, 2))
      vstr_sects_add(sects, pos, 0);

    vstr_split_buf(base, pos, len, buf, 1, sects, sects->sz, split_flags);
  }

  while (scan < sects->num)
  {
    unsigned int tmp = 0;

    ++scan;
    if (!VSTR_SECTS_NUM(sects, scan)->len)
    {
      if (off_null)
      {
        *err = VSTR_TYPE_PARSE_IPV6_ERR_IPV6_NULL;
        return (FALSE);
      }

      off_null = scan;
      ips[scan - 1] = 0;
      continue;
    }

    tmp = vstr_parse_uint(base,
                          VSTR_SECTS_NUM(sects, scan)->pos,
                          VSTR_SECTS_NUM(sects, scan)->len, 16 | num_flags,
                          &num_len, NULL);
    if (!num_len && off_null && (off_null == (scan - 1)) &&
        (scan == sects->num))
      break;

    if ((num_len > 4) || !num_len || (tmp > 0xFFFF))
    {
      *err = VSTR_TYPE_PARSE_IPV6_ERR_IPV6_OOB;
      return (FALSE);
    }

    ips[scan - 1] = tmp;
  }

  len -= VSTR_SECTS_NUM(sects, scan)->pos - pos;
  pos = VSTR_SECTS_NUM(sects, scan)->pos;
  if (scan != 8)
  {
    ASSERT(scan < 8);

    if ((VSTR_SECTS_NUM(sects, scan)->len > 4) && (off_null || (scan == 7)))
    { /* try an ipv4 address at the end... */
      unsigned char ipv4_ips[4];
      unsigned int ipv4_flags = VSTR_FLAG_PARSE_IPV4_FULL;
      size_t tmp_num_len = 0;

      if (flags & VSTR_FLAG_PARSE_IPV6_LOCAL)
        ipv4_flags |= VSTR_FLAG_PARSE_IPV4_LOCAL;

      ASSERT(off_null != scan);

      if (vstr_parse_ipv4(base,
                          VSTR_SECTS_NUM(sects, scan)->pos,
                          VSTR_SECTS_NUM(sects, scan)->len,
                          ipv4_ips, NULL, ipv4_flags, &tmp_num_len, NULL))
      {
        num_len = tmp_num_len;
        ips[scan - 1] = ipv4_ips[1] + (((unsigned int)ipv4_ips[0]) << 8);
        ips[scan++]   = ipv4_ips[3] + (((unsigned int)ipv4_ips[2]) << 8);
      }
    }
    if (scan != 8 && !off_null)
    {
      *err = VSTR_TYPE_PARSE_IPV6_ERR_IPV6_FULL;
      return (FALSE);
    }

    if (off_null != scan) /* don't have to do anything for ends with :: */
    {
      unsigned int *beg = ips + off_null;
      unsigned int off_end = (8 - scan);
      unsigned int num = (scan - off_null);

      vstr_wrap_memmove(beg + off_end, beg, num * sizeof(unsigned int));
      vstr_wrap_memset(beg, 0, off_end * sizeof(unsigned int));
    }
  }

  ASSERT(len >= num_len);
  pos += num_len;
  len -= num_len;
  if (cidr)
    *cidr = 128;

  if (len && (vstr_export_chr(base, pos) == sym_slash))
  {
    if (flags & VSTR_FLAG_PARSE_IPV6_CIDR)
    {
      ++pos;
      --len;

      if (!vstr__parse_ipv6_cidr(base, pos, &len, flags, num_flags, cidr, err))
        return (FALSE);
    }
  }

  if (len && (flags & VSTR_FLAG_PARSE_IPV6_ONLY))
    *err = VSTR_TYPE_PARSE_IPV6_ERR_ONLY;

  if (ret_len)
    *ret_len = orig_len - len;

  return (TRUE);
}
#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(parse_int);
VSTR__SYM_ALIAS(parse_intmax);
VSTR__SYM_ALIAS(parse_ipv4);
VSTR__SYM_ALIAS(parse_ipv6);
VSTR__SYM_ALIAS(parse_long);
VSTR__SYM_ALIAS(parse_num);
VSTR__SYM_ALIAS(parse_short);
VSTR__SYM_ALIAS(parse_uint);
VSTR__SYM_ALIAS(parse_uintmax);
VSTR__SYM_ALIAS(parse_ulong);
VSTR__SYM_ALIAS(parse_ushort);
