#define VSTR_SRCH_C
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
/* functions for searching within a vstr */
#include "main.h"

size_t vstr_srch_chr_fwd(const Vstr_base *base, size_t pos, size_t len,
                         char srch)
{
  Vstr_iter iter[1];
  char *found = NULL;

  if (!vstr_iter_fwd_beg(base, pos, len, iter))
    return (0);

  ASSERT(len == vstr_iter_len(iter));

  do
  {
    if (iter->node->type != VSTR_TYPE_NODE_NON)
    {
      found = vstr_wrap_memchr(iter->ptr, srch, iter->len);
      if (found)
        return (vstr_iter_pos(iter, pos, len) + (found - iter->ptr));
    }
  } while (vstr_iter_fwd_nxt(iter));

  return (0);
}

static size_t vstr__srch_chr_rev_slow(const Vstr_base *base,
                                      size_t pos, size_t len,
                                      char srch)
{
  size_t ret = 0;
  size_t scan_pos = pos;
  size_t scan_len = len;

  while (scan_len)
  {
    size_t tmp = 0;

    assert(scan_pos < (pos + len));

    if (!(tmp = vstr_srch_chr_fwd(base, scan_pos, scan_len, srch)))
      break;

    ret = tmp;

    scan_pos = ret + 1;
    scan_len = len - VSTR_SC_POSDIFF(pos, ret);
  }

  return (ret);
}

static size_t vstr__srch_chr_rev_fast(const Vstr_base *base,
                                      size_t pos, size_t len,
                                      char srch)
{
  unsigned int num = 0;
  unsigned int type = 0;
  char *scan_str = NULL;
  size_t scan_len = 0;
  char *found = NULL;

  if (!vstr__base_scan_rev_beg(base, pos, &len, &num, &type,
                               &scan_str, &scan_len))
    return (0);

  do
  {
    if (type != VSTR_TYPE_NODE_NON)
    {
      found = vstr_wrap_memrchr(scan_str, srch, scan_len);
      if (found)
        return (pos + len + (found - scan_str));
    }
  } while (vstr__base_scan_rev_nxt(base, &len, &num, &type,
                                   &scan_str, &scan_len));

  return (0);
}

size_t vstr_srch_chr_rev(const Vstr_base *base, size_t pos, size_t len,
                         char srch)
{
  if (base->iovec_upto_date)
    return (vstr__srch_chr_rev_fast(base, pos, len, srch));

  return (vstr__srch_chr_rev_slow(base, pos, len, srch));
}

size_t vstr_srch_chrs_fwd(const Vstr_base *base, size_t pos, size_t len,
                          const char *srch, size_t chrs_len)
{
  size_t num = vstr_cspn_chrs_fwd(base, pos, len, srch, chrs_len);

  if (num == len)
    return (0);

  return (pos + num);
}

size_t vstr_srch_chrs_rev(const Vstr_base *base, size_t pos, size_t len,
                          const char *srch, size_t chrs_len)
{
  size_t num = vstr_cspn_chrs_rev(base, pos, len, srch, chrs_len);

  if (num == len)
    return (0);

  return (pos + ((len - num) - 1));
}

size_t vstr_csrch_chrs_fwd(const Vstr_base *base, size_t pos, size_t len,
                           const char *srch, size_t chrs_len)
{
  size_t num = vstr_spn_chrs_fwd(base, pos, len, srch, chrs_len);

  if (num == len)
    return (0);

  return (pos + num);
}

size_t vstr_csrch_chrs_rev(const Vstr_base *base, size_t pos, size_t len,
                           const char *srch, size_t chrs_len)
{
  size_t num = vstr_spn_chrs_rev(base, pos, len, srch, chrs_len);

  if (num == len)
    return (0);

  return (pos + ((len - num) - 1));
}

size_t vstr_srch_buf_fwd(const Vstr_base *base, size_t pos, size_t len,
                         const void *const str, const size_t str_len)
{
  Vstr_iter iter[1];

  if (!len || (str_len > len))
    return (0);

  if (!str_len)
    return (pos);

  if (!str && !base->node_non_used)
    return (0);

  if (str && (str_len == 1))
    return (vstr_srch_chr_fwd(base, pos, len, *(const char *)str));

  if (!vstr_iter_fwd_beg(base, pos, len, iter))
    return (0);

  assert(len == vstr_iter_len(iter));

  do
  {
    if ((iter->node->type == VSTR_TYPE_NODE_NON) && !str)
    {
      if (!vstr_cmp_buf(base, vstr_iter_pos(iter, pos, len), str_len,
                        NULL, str_len))
        return (vstr_iter_pos(iter, pos, len));
      goto next_loop;
    }
    if (!str)
      goto next_loop;

    if (iter->node->type == VSTR_TYPE_NODE_NON)
      goto next_loop;

    /* find buf */
    while (vstr_iter_len(iter) >= str_len)
    {
      size_t tmp = 0;
      size_t beg_pos = 0;

      assert(iter->len);

      if (*iter->ptr != *(const char *)str)
      {
        char *som = NULL; /* start of a possible match */

        if (!(som = vstr_wrap_memchr(iter->ptr, *(const char *)str,iter->len)))
          goto next_loop;
        iter->len -= (som - iter->ptr);
        iter->ptr = som;
        continue;
      }

      tmp = iter->len;
      if (tmp > str_len)
        tmp = str_len;

      beg_pos = vstr_iter_pos(iter, pos, len);

      if (!vstr_wrap_memcmp(iter->ptr, str, tmp) &&
          ((tmp == str_len) ||
           !vstr_cmp_buf(base, beg_pos + tmp, str_len - tmp,
                         (const char *)str + tmp, str_len - tmp)))
        return (beg_pos);

      ++iter->ptr;
      --iter->len;

      if (!iter->len) break;
    }

   next_loop:
    continue;
  } while (vstr_iter_fwd_nxt(iter) && (vstr_iter_len(iter) >= str_len));

  return (0);
}

static size_t vstr__srch_buf_rev_slow(const Vstr_base *base,
                                      size_t pos, size_t len,
                                      const void *const str,
                                      const size_t str_len)
{
  size_t ret = 0;
  size_t scan_pos = pos;
  size_t scan_len = len;

  while (scan_len >= str_len)
  {
    size_t tmp = 0;

    assert(scan_pos < (pos + len));

    if (!(tmp = vstr_srch_buf_fwd(base, scan_pos, scan_len, str, str_len)))
      break;

    ret = tmp;

    scan_pos = ret + 1;
    scan_len = len - VSTR_SC_POSDIFF(pos, ret);
  }

  return (ret);
}

/* only to be used from srch_buf_rev_fast ... assumes a bunch of crap */
static int vstr__cmp_eq_rev_buf(const Vstr_base *base, size_t len,
                                unsigned int num, unsigned int type,
                                const char *str, size_t str_len,
                                char *scan_str, size_t scan_len)
{
  assert((type != VSTR_TYPE_NODE_NON) && str && str_len &&
         scan_str && scan_len);
  
  if (str_len > (len + scan_len))
    return (FALSE);
  
  while (str_len)
  {
    size_t tmp = scan_len;

    if (type == VSTR_TYPE_NODE_NON)
      return (FALSE);
    
    if (tmp > str_len)
      tmp = str_len;

    if (vstr_wrap_memcmp((str + str_len) - tmp,
                         (scan_str + scan_len) - tmp, tmp))
      return (FALSE);
    ASSERT(tmp);
    
    str_len  -= tmp;
    scan_len -= tmp;
    if (!scan_len)
      tmp = vstr__base_scan_rev_nxt(base, &len, &num, &type,
                                    &scan_str, &scan_len);
    ASSERT(tmp || (!str_len && !scan_len));
  }
  
  return (TRUE);
}

/* only to be used from srch_buf_rev_fast ... assumes a bunch of crap */
static int vstr__cmp_eq_rev_non(const Vstr_base *base, size_t len,
                                unsigned int num, unsigned int type,
                                size_t str_len, size_t scan_len)
{
  char *scan_str = NULL;
  int tmp = FALSE;
  
  assert(type == VSTR_TYPE_NODE_NON);
  
  if (str_len > (len + scan_len))
    return (FALSE);

  while (type == VSTR_TYPE_NODE_NON)
  {
    if (str_len <= scan_len)
      return (TRUE);

    str_len -= scan_len;

    tmp = vstr__base_scan_rev_nxt(base, &len, &num, &type,
                                  &scan_str, &scan_len);
    ASSERT(tmp);
  }

  return (FALSE);
}

static size_t vstr__srch_buf_rev_fast(const Vstr_base *base,
                                      size_t pos, size_t len,
                                      const void *const str,
                                      size_t str_len)
{
  unsigned int num = 0;
  unsigned int type = 0;
  char *scan_str = NULL;
  size_t scan_len = 0;
  size_t orig_len = len;
  const char *end_str = ((const char *)str) + (str_len - 1);

  if (!vstr__base_scan_rev_beg(base, pos, &len, &num, &type,
                               &scan_str, &scan_len))
    return (0);

  assert(orig_len == (len + scan_len));

  do
  {
    size_t count = 0;

    if ((type == VSTR_TYPE_NODE_NON) && !str)
    {
      if (vstr__cmp_eq_rev_non(base, len, num, type, str_len, scan_len))
        return ((pos + len + scan_len) - str_len);
      continue;
    }

    if (!str)
      continue;

    if (type == VSTR_TYPE_NODE_NON)
      continue;

    /* find buf */
    while (count < scan_len)
    {
      size_t rest = 0;
      size_t len_end = 0;

      ++count;
      rest = scan_len - count;
      if (scan_str[scan_len - count] != end_str[0])
      {
        char *som = NULL; /* start of a possible match */

        if (!rest || !(som = vstr_wrap_memrchr(scan_str, end_str[0], rest)))
          continue;
        count += ((scan_str + rest) - som);
        rest = scan_len - count;
      }
      assert(scan_str[scan_len - count] == end_str[0]);

      len_end = VSTR_SC_POSDIFF(count, scan_len);
      if (vstr__cmp_eq_rev_buf(base, len, num, type,
                               str,      str_len,
                               scan_str, len_end))
        return ((pos + len + len_end) - str_len);
    }
  } while (vstr__base_scan_rev_nxt(base, &len, &num, &type,
                                   &scan_str, &scan_len));

  return (0);
}

size_t vstr_srch_buf_rev(const Vstr_base *base, size_t pos, size_t len,
                         const void *const str, const size_t str_len)
{
  if (!len || (str_len > len))
    return (0);

  if (!str_len)
    return (pos + len - 1);

  if (str && (str_len == 1))
    return (vstr_srch_chr_rev(base, pos, len, *(const char *)str));

  if (base->iovec_upto_date)
    return (vstr__srch_buf_rev_fast(base, pos, len, str, str_len));

  return (vstr__srch_buf_rev_slow(base, pos, len, str, str_len));
}

size_t vstr_srch_vstr_fwd(const Vstr_base *base, size_t pos, size_t len,
                          const Vstr_base *ndl_base,
                          size_t ndl_pos, size_t ndl_len)
{ /* TODO: this could be faster, esp. with NON nodes */
  Vstr_iter iter[1];
  size_t scan_pos = pos;
  size_t scan_len = len;

  if (ndl_len > len)
    return (0);

  if (!vstr_iter_fwd_beg(ndl_base, ndl_pos, ndl_len, iter))
    return (0);

  while ((scan_pos < vstr_sc_poslast(pos, len)) && (scan_len >= ndl_len))
  {
    if (!vstr_cmp(base, scan_pos, ndl_len, ndl_base, ndl_pos, ndl_len))
      return (scan_pos);

    --scan_len;
    ++scan_pos;

    if (iter->node->type != VSTR_TYPE_NODE_NON)
    {
      size_t tmp = 0;

      if (!(tmp = vstr_srch_buf_fwd(base, scan_pos, scan_len,
                                    iter->ptr, iter->len)))
        return (0);

      ASSERT(tmp >= scan_pos);
      scan_len -= tmp - scan_pos;
      scan_pos = tmp;
    }
  }

  return (0);
}

static size_t vstr__srch_vstr_rev_slow(const Vstr_base *base,
                                       size_t pos, size_t len,
                                       const Vstr_base *ndl_base,
                                       size_t ndl_pos, size_t ndl_len)
{
  size_t ret = 0;
  size_t scan_pos = pos;
  size_t scan_len = len;

  while (scan_len >= ndl_len)
  {
    size_t tmp = 0;

    assert(scan_pos < (pos + len));

    if (!(tmp = vstr_srch_vstr_fwd(base, scan_pos, scan_len,
                                   ndl_base, ndl_pos, ndl_len)))
      break;

    ret = tmp;

    scan_pos = ret + 1;
    scan_len = len - VSTR_SC_POSDIFF(pos, ret);
  }

  return (ret);
}

size_t vstr_srch_vstr_rev(const Vstr_base *base, size_t pos, size_t len,
                          const Vstr_base *ndl_base,
                          size_t ndl_pos, size_t ndl_len)
{
  return (vstr__srch_vstr_rev_slow(base, pos, len, ndl_base, ndl_pos, ndl_len));
}

#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(csrch_chrs_fwd);
VSTR__SYM_ALIAS(csrch_chrs_rev);
VSTR__SYM_ALIAS(srch_buf_fwd);
VSTR__SYM_ALIAS(srch_buf_rev);
VSTR__SYM_ALIAS(srch_chr_fwd);
VSTR__SYM_ALIAS(srch_chr_rev);
VSTR__SYM_ALIAS(srch_chrs_fwd);
VSTR__SYM_ALIAS(srch_chrs_rev);
VSTR__SYM_ALIAS(srch_vstr_fwd);
VSTR__SYM_ALIAS(srch_vstr_rev);
