#define VSTR_SPN_C
/*
 *  Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005 James Antill
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
/* functionss for counting a "spanning" of chars within a vstr */
#include "main.h"

static size_t vstr__spn_chr_fwd(const Vstr_base *base, size_t pos, size_t len,
                                char spn_chr)
{
  Vstr_iter iter[1];
  size_t ret = 0;

  if (!vstr_iter_fwd_beg(base, pos, len, iter))
    return (0);

  do
  {
    size_t count = 0;

    if (iter->node->type == VSTR_TYPE_NODE_NON)
      return (ret);

    while (count < iter->len)
    {
      if (spn_chr != iter->ptr[count])
        return (ret + count);
      ++count;
    }
    assert(count == iter->len);

    ret += iter->len;
  } while (vstr_iter_fwd_nxt(iter));

  return (ret);
}

size_t vstr_spn_chrs_fwd(const Vstr_base *base, size_t pos, size_t len,
                         const char *spn_chrs, size_t spn_len)
{
  Vstr_iter iter[1];
  size_t ret = 0;

  ASSERT_RET(base, 0);
  
  if (!spn_chrs && !base->node_non_used)
    return (0);

  if (spn_chrs && (spn_len == 1))
    return (vstr__spn_chr_fwd(base, pos, len, *spn_chrs));

  if (!vstr_iter_fwd_beg(base, pos, len, iter))
    return (0);

  do
  {
    size_t count = 0;

    if ((iter->node->type == VSTR_TYPE_NODE_NON) && spn_chrs)
      return (ret);

    if (iter->node->type == VSTR_TYPE_NODE_NON)
    {
      assert(!spn_chrs);
      goto next_loop;
    }

    if (!spn_chrs)
      return (ret);

    while (count < iter->len)
    {
      if (!vstr_wrap_memchr(spn_chrs, iter->ptr[count], spn_len))
        return (ret + count);
      ++count;
    }
    assert(count == iter->len);

   next_loop:
    ret += iter->len;
  } while (vstr_iter_fwd_nxt(iter));

  return (ret);
}

#if 0
static size_t VSTR__ATTR_I() vstr__spn_chrs_rev_slow(const Vstr_base *base,
                                      size_t pos, size_t len,
                                      const char *spn_chrs, size_t spn_len);
#endif
/* go through fwd, reset everytime it fails then start doing it again */
static size_t vstr__spn_chrs_rev_slow(const Vstr_base *base,
                                      size_t pos, size_t len,
                                      const char *spn_chrs, size_t spn_len)
{
  Vstr_iter iter[1];
  size_t ret = 0;

  if (!spn_chrs && !base->node_non_used)
    return (0);

  if (!vstr_iter_fwd_beg(base, pos, len, iter))
    return (0);

  do
  {
    size_t count = 0;

    if ((iter->node->type == VSTR_TYPE_NODE_NON) && spn_chrs)
    {
      ret = 0;
      continue;
    }

    if (iter->node->type == VSTR_TYPE_NODE_NON)
    {
      assert(!spn_chrs);
      goto next_loop_all_good;
    }

    if (!spn_chrs)
    {
      ret = 0;
      continue;
    }

    count = iter->len;
    while (count-- > 0)
      if (!vstr_wrap_memchr(spn_chrs, iter->ptr[count], spn_len))
      {
        ret = ((iter->len - count) - 1);
        goto next_loop_memchr_fail;
      }

   next_loop_all_good:
    ret += iter->len;
   next_loop_memchr_fail:
    continue;
  } while (vstr_iter_fwd_nxt(iter));

  return (ret);
}

static size_t vstr__spn_chrs_rev_fast(const Vstr_base *base,
                                      size_t pos, size_t len,
                                      const char *spn_chrs, size_t spn_len)
{
  unsigned int num = 0;
  unsigned int type = 0;
  size_t ret = 0;
  char *scan_str = NULL;
  size_t scan_len = 0;

  if (!spn_chrs && !base->node_non_used)
    return (0);

  if (!vstr__base_scan_rev_beg(base, pos, &len, &num, &type,
                               &scan_str, &scan_len))
    return (0);

  do
  {
    size_t count = 0;

    if ((type == VSTR_TYPE_NODE_NON) && spn_chrs)
      return (ret);

    if (type == VSTR_TYPE_NODE_NON)
    {
      assert(!spn_chrs);
      goto next_loop;
    }

    if (!spn_chrs)
      return (ret);

    while (count < scan_len)
    {
      ++count;
      if (!vstr_wrap_memchr(spn_chrs, scan_str[scan_len - count], spn_len))
        return (ret + (count - 1));
    }
    assert(count == scan_len);

   next_loop:
    ret += scan_len;
  } while (vstr__base_scan_rev_nxt(base, &len, &num, &type,
                                   &scan_str, &scan_len));

  return (ret);
}

size_t vstr_spn_chrs_rev(const Vstr_base *base, size_t pos, size_t len,
                         const char *spn_chrs, size_t spn_len)
{
  ASSERT_RET(base, 0);
  
  if (base->iovec_upto_date)
    return (vstr__spn_chrs_rev_fast(base, pos, len, spn_chrs, spn_len));

  return (vstr__spn_chrs_rev_slow(base, pos, len, spn_chrs, spn_len));
}

size_t vstr_cspn_chrs_fwd(const Vstr_base *base, size_t pos, size_t len,
                          const char *cspn_chrs, size_t cspn_len)
{
  Vstr_iter iter[1];
  size_t ret = 0;

  ASSERT_RET(base, 0);
  if (!cspn_chrs && !base->node_non_used)
    return (len);

  if (cspn_chrs && (cspn_len == 1))
  {
    size_t f_pos = vstr_srch_chr_fwd(base, pos, len, cspn_chrs[0]);

    if (!f_pos)
      return (len);

    return (f_pos - pos);
  }

  if (!vstr_iter_fwd_beg(base, pos, len, iter))
    return (0);

  do
  {
    size_t count = 0;

    if ((iter->node->type == VSTR_TYPE_NODE_NON) && cspn_chrs)
      goto next_loop;

    if (iter->node->type == VSTR_TYPE_NODE_NON)
    {
      assert(!cspn_chrs);
      return (ret);
    }

    if (!cspn_chrs)
      goto next_loop;

    while (count < iter->len)
    {
      if (vstr_wrap_memchr(cspn_chrs, iter->ptr[count], cspn_len))
        return (ret + count);
      ++count;
    }
    assert(count == iter->len);

   next_loop:
    ret += iter->len;
  } while (vstr_iter_fwd_nxt(iter));

  return (ret);
}

/* go through fwd, reset everytime it fails then start doing it again */
static size_t vstr__cspn_chrs_rev_slow(const Vstr_base *base,
                                       size_t pos, size_t len,
                                       const char *cspn_chrs, size_t cspn_len)
{
  Vstr_iter iter[1];
  size_t ret = 0;

  if (!vstr_iter_fwd_beg(base, pos, len, iter))
    return (0);

  do
  {
    size_t count = 0;

    if ((iter->node->type == VSTR_TYPE_NODE_NON) && cspn_chrs)
      goto next_loop_all_good;

    if (iter->node->type == VSTR_TYPE_NODE_NON)
    {
      assert(!cspn_chrs);
      ret = 0;
      continue;
    }

    if (!cspn_chrs)
      goto next_loop_all_good;

    count = iter->len;
    while (count-- > 0)
      if (vstr_wrap_memchr(cspn_chrs, iter->ptr[count], cspn_len))
      {
        ret = ((iter->len - count) - 1);
        goto next_loop_memchr_fail;
      }

   next_loop_all_good:
    ret += iter->len;
   next_loop_memchr_fail:
    continue;
  } while (vstr_iter_fwd_nxt(iter));

  return (ret);
}

static size_t vstr__cspn_chrs_rev_fast(const Vstr_base *base,
                                       size_t pos, size_t len,
                                       const char *cspn_chrs, size_t cspn_len)
{
  unsigned int num = 0;
  unsigned int type = 0;
  size_t ret = 0;
  char *scan_str = NULL;
  size_t scan_len = 0;

  if (!vstr__base_scan_rev_beg(base, pos, &len, &num, &type,
                               &scan_str, &scan_len))
    return (0);

  do
  {
    size_t count = 0;

    if ((type == VSTR_TYPE_NODE_NON) && cspn_chrs)
      goto next_loop;

    if (type == VSTR_TYPE_NODE_NON)
    {
      assert(!cspn_chrs);
      return (ret);
    }

    if (!cspn_chrs)
      goto next_loop;

    while (count < scan_len)
    {
      ++count;
      if (vstr_wrap_memchr(cspn_chrs, scan_str[scan_len - count], cspn_len))
        return (ret + (count - 1));
    }
    assert(count == scan_len);

   next_loop:
    ret += scan_len;
  } while (vstr__base_scan_rev_nxt(base, &len, &num, &type,
                                   &scan_str, &scan_len));

  return (ret);
}

size_t vstr_cspn_chrs_rev(const Vstr_base *base, size_t pos, size_t len,
                          const char *cspn_chrs, size_t cspn_len)
{
  ASSERT_RET(base, 0);
  
  if (!cspn_chrs && !base->node_non_used)
    return (len);

  if (cspn_chrs && (cspn_len == 1))
  {
    size_t f_pos = vstr_srch_chr_rev(base, pos, len, cspn_chrs[0]);

    if (!f_pos)
      return (len);

    return ((pos + (len - 1)) - f_pos);
  }

  if (base->iovec_upto_date)
    return (vstr__cspn_chrs_rev_fast(base, pos, len, cspn_chrs, cspn_len));

  return (vstr__cspn_chrs_rev_slow(base, pos, len, cspn_chrs, cspn_len));
}

/* Byte mapping. With inspiration from:
   http://groups-beta.google.com/group/comp.unix.programmer/msg/f92e10e01279f708?hl=en
*/
size_t vstr_spn_bmap_eq_fwd(const Vstr_base *base, size_t pos, size_t len,
                            const unsigned char bmap[VSTR__COMPILE_STATIC_ARRAY() 256],
                            unsigned char val)
{
  Vstr_iter iter[1];
  size_t ret = 0;

  ASSERT_RET(base, 0);
  
  if (!vstr_iter_fwd_beg(base, pos, len, iter))
    return (0);

  do
  {
    size_t count = 0;

    if (iter->node->type == VSTR_TYPE_NODE_NON)
      return (ret);

    while (count < iter->len)
    {
      if (bmap[0xFF & (unsigned char)iter->ptr[count]] != val)
        return (ret + count);
      ++count;
    }
    assert(count == iter->len);

    ret += iter->len;
  } while (vstr_iter_fwd_nxt(iter));

  return (ret);  
}

size_t vstr_spn_bmap_eq_rev(const Vstr_base *base, size_t pos, size_t len,
                            const unsigned char bmap[VSTR__COMPILE_STATIC_ARRAY() 256],
                            unsigned char val)
{
  Vstr_iter iter[1];
  size_t ret = 0;

  ASSERT_RET(base, 0);
  
  if (!vstr_iter_fwd_beg(base, pos, len, iter))
    return (0);

  do
  {
    size_t count = 0;

    if (iter->node->type == VSTR_TYPE_NODE_NON)
    {
      ret = 0;
      continue;
    }

    count = iter->len;
    while (count-- > 0)
      if (bmap[0xFF & (unsigned char)iter->ptr[count]] != val)
      {
        ret = ((iter->len - count) - 1);
        goto next_loop_bmap_fail;
      }

    ret += iter->len;
   next_loop_bmap_fail:
    continue;
  } while (vstr_iter_fwd_nxt(iter));

  return (ret);  
}

size_t vstr_spn_bmap_and_fwd(const Vstr_base *base, size_t pos, size_t len,
                             const unsigned char bmap[VSTR__COMPILE_STATIC_ARRAY() 256],
                             unsigned char val)
{
  Vstr_iter iter[1];
  size_t ret = 0;

  ASSERT_RET(base, 0);
  
  if (!vstr_iter_fwd_beg(base, pos, len, iter))
    return (0);

  do
  {
    size_t count = 0;

    if (iter->node->type == VSTR_TYPE_NODE_NON)
      return (ret);

    while (count < iter->len)
    {
      if (!(bmap[0xFF & (unsigned char)iter->ptr[count]] & val))
        return (ret + count);
      ++count;
    }
    assert(count == iter->len);

    ret += iter->len;
  } while (vstr_iter_fwd_nxt(iter));

  return (ret);  
}

size_t vstr_spn_bmap_and_rev(const Vstr_base *base, size_t pos, size_t len,
                             const unsigned char bmap[VSTR__COMPILE_STATIC_ARRAY() 256],
                             unsigned char val)
{
  Vstr_iter iter[1];
  size_t ret = 0;

  ASSERT_RET(base, 0);
  
  if (!vstr_iter_fwd_beg(base, pos, len, iter))
    return (0);

  do
  {
    size_t count = 0;

    if (iter->node->type == VSTR_TYPE_NODE_NON)
    {
      ret = 0;
      continue;
    }

    count = iter->len;
    while (count-- > 0)
      if (!(bmap[0xFF & (unsigned char)iter->ptr[count]] & val))
      {
        ret = ((iter->len - count) - 1);
        goto next_loop_bmap_fail;
      }

    ret += iter->len;
   next_loop_bmap_fail:
    continue;
  } while (vstr_iter_fwd_nxt(iter));

  return (ret);  
}


size_t vstr_cspn_bmap_eq_fwd(const Vstr_base *base, size_t pos, size_t len,
                             const unsigned char bmap[VSTR__COMPILE_STATIC_ARRAY() 256],
                             unsigned char val)
{
  Vstr_iter iter[1];
  size_t ret = 0;

  ASSERT_RET(base, 0);
  
  if (!vstr_iter_fwd_beg(base, pos, len, iter))
    return (0);

  do
  {
    size_t count = 0;

    if (iter->node->type == VSTR_TYPE_NODE_NON)
      goto next_loop_all_good;

    while (count < iter->len)
    {
      if (bmap[0xFF & (unsigned char)iter->ptr[count]] == val)
        return (ret + count);
      ++count;
    }
    assert(count == iter->len);

   next_loop_all_good:
    ret += iter->len;
  } while (vstr_iter_fwd_nxt(iter));

  return (ret);  
}

size_t vstr_cspn_bmap_eq_rev(const Vstr_base *base, size_t pos, size_t len,
                             const unsigned char bmap[VSTR__COMPILE_STATIC_ARRAY() 256],
                             unsigned char val)
{
  Vstr_iter iter[1];
  size_t ret = 0;

  ASSERT_RET(base, 0);
  
  if (!vstr_iter_fwd_beg(base, pos, len, iter))
    return (0);

  do
  {
    size_t count = 0;

    if (iter->node->type == VSTR_TYPE_NODE_NON)
      goto next_loop_all_good;

    count = iter->len;
    while (count-- > 0)
      if (bmap[0xFF & (unsigned char)iter->ptr[count]] == val)
      {
        ret = ((iter->len - count) - 1);
        goto next_loop_bmap_fail;
      }

   next_loop_all_good:
    ret += iter->len;
   next_loop_bmap_fail:
    continue;
  } while (vstr_iter_fwd_nxt(iter));

  return (ret);  
}

size_t vstr_cspn_bmap_and_fwd(const Vstr_base *base, size_t pos, size_t len,
                              const unsigned char bmap[VSTR__COMPILE_STATIC_ARRAY() 256],
                              unsigned char val)
{
  Vstr_iter iter[1];
  size_t ret = 0;

  ASSERT_RET(base, 0);
  
  if (!vstr_iter_fwd_beg(base, pos, len, iter))
    return (0);

  do
  {
    size_t count = 0;

    if (iter->node->type == VSTR_TYPE_NODE_NON)
      goto next_loop_all_good;

    while (count < iter->len)
    {
      if (bmap[0xFF & (unsigned char)iter->ptr[count]] & val)
        return (ret + count);
      ++count;
    }
    assert(count == iter->len);

   next_loop_all_good:
    ret += iter->len;
  } while (vstr_iter_fwd_nxt(iter));

  return (ret);
}

size_t vstr_cspn_bmap_and_rev(const Vstr_base *base, size_t pos, size_t len,
                              const unsigned char bmap[VSTR__COMPILE_STATIC_ARRAY() 256],
                              unsigned char val)
{
  Vstr_iter iter[1];
  size_t ret = 0;

  ASSERT_RET(base, 0);
  
  if (!vstr_iter_fwd_beg(base, pos, len, iter))
    return (0);

  do
  {
    size_t count = 0;

    if (iter->node->type == VSTR_TYPE_NODE_NON)
      goto next_loop_all_good;

    count = iter->len;
    while (count-- > 0)
      if (bmap[0xFF & (unsigned char)iter->ptr[count]] & val)
      {
        ret = ((iter->len - count) - 1);
        goto next_loop_bmap_fail;
      }

   next_loop_all_good:
    ret += iter->len;
   next_loop_bmap_fail:
    continue;
  } while (vstr_iter_fwd_nxt(iter));

  return (ret);  
}

#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(cspn_bmap_eq_fwd);
VSTR__SYM_ALIAS(cspn_bmap_eq_rev);
VSTR__SYM_ALIAS(cspn_bmap_and_fwd);
VSTR__SYM_ALIAS(cspn_bmap_and_rev);
VSTR__SYM_ALIAS(cspn_chrs_fwd);
VSTR__SYM_ALIAS(cspn_chrs_rev);
VSTR__SYM_ALIAS(spn_bmap_eq_fwd);
VSTR__SYM_ALIAS(spn_bmap_eq_rev);
VSTR__SYM_ALIAS(spn_bmap_and_fwd);
VSTR__SYM_ALIAS(spn_bmap_and_rev);
VSTR__SYM_ALIAS(spn_chrs_fwd);
VSTR__SYM_ALIAS(spn_chrs_rev);
