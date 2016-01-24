#define VSTR_SPLIT_C
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
/* functions to split a Vstr into sections (ala. perl split()) */
#include "main.h"


#define VSTR__SPLIT_LIM(x) (!limit || (limit > (x)))

/* do the null fields go to the end ? */
static int vstr__split_buf_null_end(const Vstr_base *base,
                                    size_t pos, size_t len,
                                    const void *buf, size_t buf_len,
                                    unsigned int *ret_num)
{
  assert(vstr_cmp_buf_eq(base, pos, buf_len, buf, buf_len));
  assert(len >= buf_len);

  *ret_num = 1;

  if (len == buf_len)
    return (TRUE);

  pos += buf_len;
  len -= buf_len;

  while (len >= buf_len)
  {
    if (!vstr_cmp_buf_eq(base, pos, buf_len, buf, buf_len))
      return (FALSE);

    ++*ret_num;
    pos += buf_len;
    len -= buf_len;
  }

  return (!len);
}

static unsigned int vstr__split_hdl_err(Vstr_sects *sects,
                                        unsigned int flags, unsigned int added,
                                        int *bad_ret)
{
  ASSERT(bad_ret);
  ASSERT(!*bad_ret);
  
  if (sects->malloc_bad)
  {
    ASSERT(sects->num >= added);
    sects->num -= added;
    *bad_ret = TRUE;
    return (0);
  }
  
  ASSERT(!sects->can_add_sz);
  ASSERT(sects->num == sects->sz);
  
  if (flags & VSTR_FLAG_SPLIT_NO_RET)
    *bad_ret = TRUE;
  
  return (1);
}
    
#define VSTR__SPLIT_HDL_ERR(bad_ret) do {                               \
      unsigned int err_ret = vstr__split_hdl_err(sects, flags, added,   \
                                                 bad_ret);              \
                                                                        \
      if (*bad_ret)                                                     \
        return (err_ret);                                               \
    } while (FALSE)

static unsigned int vstr__split_hdl_null_beg(size_t *pos, size_t *len,
                                             size_t buf_len,
                                             Vstr_sects *sects,
                                             unsigned int flags,
                                             unsigned int count,
                                             unsigned int limit,
                                             unsigned int added,
                                             int *bad_ret)
{
  const int is_remain = !!(flags & VSTR_FLAG_SPLIT_REMAIN);

  ASSERT(count);

  if (limit && (count >= (limit - added)))
    count = (limit - is_remain) - added;

  while (count)
  {
    if (flags & VSTR_FLAG_SPLIT_BEG_NULL)
    {
      if (!vstr_sects_add(sects, *pos, 0))
        VSTR__SPLIT_HDL_ERR(bad_ret);

      ++added;
    }

    *pos += buf_len;
    *len -= buf_len;
    --count;
  }

  return (added);
}

static unsigned int vstr__split_hdl_null_mid(size_t *pos, size_t *len,
                                             size_t buf_len,
                                             Vstr_sects *sects,
                                             unsigned int flags,
                                             unsigned int count,
                                             unsigned int limit,
                                             unsigned int added,
                                             int *bad_ret)
{
  const int is_remain = !!(flags & VSTR_FLAG_SPLIT_REMAIN);

  ASSERT(count);

  if (limit && (count >= (limit - added)))
    count = (limit - is_remain) - added;

  while (count)
  {
    if (flags & VSTR_FLAG_SPLIT_MID_NULL)
    {
      if (!vstr_sects_add(sects, *pos, 0))
        VSTR__SPLIT_HDL_ERR(bad_ret);

      ++added;
    }

    *pos += buf_len;
    *len -= buf_len;
    --count;
  }

  return (added);
}

static unsigned int vstr__split_hdl_null_end(size_t pos, size_t len,
                                             size_t buf_len,
                                             Vstr_sects *sects,
                                             unsigned int flags,
                                             unsigned int count,
                                             unsigned int limit,
                                             unsigned int added)
{
  const int is_remain = !!(flags & VSTR_FLAG_SPLIT_REMAIN);
  int bad_ret = FALSE;
  
  assert(len);

  if (!(flags & VSTR_FLAG_SPLIT_END_NULL))
    goto no_end_null;

  if (limit && (count >= (limit - added)))
    count = (limit - is_remain) - added;

  while (count)
  {
    if (!vstr_sects_add(sects, pos, 0))
      VSTR__SPLIT_HDL_ERR(&bad_ret);

    ++added;

    pos += buf_len;
    len -= buf_len;
    --count;
  }

 no_end_null:
  if (((len &&  (flags & VSTR_FLAG_SPLIT_REMAIN)) ||
       (!len && (flags & VSTR_FLAG_SPLIT_POST_NULL))))
  {
    if (!vstr_sects_add(sects, pos, len))
      VSTR__SPLIT_HDL_ERR(&bad_ret);
    ++added;
  }

  return (added);
}

static unsigned int vstr__split_hdl_end(size_t pos, size_t len,
                                        size_t split_pos,
                                        Vstr_sects *sects,
                                        unsigned int limit, unsigned int flags,
                                        unsigned int added)
{
  int bad_ret = FALSE;
  
  if (len)
  {
    if (flags & VSTR_FLAG_SPLIT_REMAIN)
    {
      ASSERT(!limit || (added <= (limit - 1)));

      if (!vstr_sects_add(sects, pos, len))
        VSTR__SPLIT_HDL_ERR(&bad_ret);

      ++added;
    }
    else if (!split_pos)
    {
      if (!vstr_sects_add(sects, pos, len))
        VSTR__SPLIT_HDL_ERR(&bad_ret);

      ++added;
    }
  }
  else if ((flags & VSTR_FLAG_SPLIT_POST_NULL) && (!limit || (added < limit)))
  {
    if (!vstr_sects_add(sects, pos, 0))
      VSTR__SPLIT_HDL_ERR(&bad_ret);
    
    ++added;
  }

  return (added);
}

static unsigned int vstr__split_hdl_def(size_t *pos, size_t *len,
                                        size_t split_pos, size_t buf_len, 
                                        Vstr_sects *sects,
                                        unsigned int flags,
                                        unsigned int added,
                                        int *bad_ret)
{
  size_t split_len = (split_pos - *pos);
  
  ASSERT(split_pos > *pos);
  
  if (!vstr_sects_add(sects, *pos, split_len))
    VSTR__SPLIT_HDL_ERR(bad_ret);
  
  ++added;
  
  split_len += buf_len;
  
  *pos += split_len;
  *len -= split_len;
  
  return (added);
}

unsigned int vstr_split_buf(const Vstr_base *base, size_t pos, size_t len,
                            const void *buf, size_t buf_len,
                            Vstr_sects *sects,
                            unsigned int limit, unsigned int flags)
{
  size_t orig_pos = pos;
  size_t split_pos = 0;
  unsigned int added = 0;
  const int is_remain = !!(flags & VSTR_FLAG_SPLIT_REMAIN);
  int bad_ret = FALSE;
  
  while (len && (!limit || (added < (limit - is_remain))) &&
         (split_pos = vstr_srch_buf_fwd(base, pos, len, buf, buf_len)))
  {
    if (split_pos == orig_pos)
    {
      unsigned int count = 0;

      assert(orig_pos == pos);

      if (vstr__split_buf_null_end(base, pos, len, buf, buf_len, &count))
      {
        if (!(flags & VSTR_FLAG_SPLIT_BEG_NULL))
          return (0);

        return (vstr__split_hdl_null_end(pos, len, buf_len, sects, flags,
                                         count, limit, added));
      }
      added = vstr__split_hdl_null_beg(&pos, &len, buf_len, sects, flags,
                                       count, limit, added, &bad_ret);
      if (bad_ret)
        return (added);
    }
    else if (split_pos == pos)
    {
      unsigned int count = 0;

      if (vstr__split_buf_null_end(base, pos, len, buf, buf_len, &count))
        return (vstr__split_hdl_null_end(pos, len, buf_len, sects, flags,
                                         count, limit, added));
      added = vstr__split_hdl_null_mid(&pos, &len, buf_len, sects, flags,
                                       count, limit, added, &bad_ret);
      if (bad_ret)
        return (added);
    }
    else
    {
      added = vstr__split_hdl_def(&pos, &len, split_pos, buf_len, sects,
                                  flags, added, &bad_ret);
      if (bad_ret)
        return (added);
    }
  }

  return (vstr__split_hdl_end(pos, len, split_pos, sects, limit, flags, added));
}

unsigned int vstr_split_chrs(const Vstr_base *base, size_t pos, size_t len,
                             const char *chrs, size_t chrs_len,
                             Vstr_sects *sects,
                             unsigned int limit, unsigned int flags)
{
  size_t orig_pos = pos;
  size_t split_pos = 0;
  unsigned int added = 0;
  const int is_remain = !!(flags & VSTR_FLAG_SPLIT_REMAIN);
  int bad_ret = FALSE;

  while (len && (!limit || (added < (limit - is_remain))) &&
         (split_pos = vstr_srch_chrs_fwd(base, pos, len, chrs, chrs_len)))
  {
    if (split_pos == orig_pos)
    {
      unsigned int count = 0;

      assert(orig_pos == pos);

      if ((count = vstr_spn_chrs_fwd(base, pos, len, chrs, chrs_len)) == len)
      {
        if (!(flags & VSTR_FLAG_SPLIT_BEG_NULL))
          return (0);

        return (vstr__split_hdl_null_end(pos, len, 1, sects, flags,
                                         count, limit, added));
      }
      added = vstr__split_hdl_null_beg(&pos, &len, 1, sects, flags,
                                       count, limit, added, &bad_ret);
      if (bad_ret)
        return (added);
    }
    else if (split_pos == pos)
    {
      unsigned int count = 0;

      if ((count = vstr_spn_chrs_fwd(base, pos, len, chrs, chrs_len)) == len)
        return (vstr__split_hdl_null_end(pos, len, 1, sects, flags,
                                         count, limit, added));

      added = vstr__split_hdl_null_mid(&pos, &len, 1, sects, flags,
                                       count, limit, added, &bad_ret);
      if (bad_ret)
        return (added);
    }
    else
    {
      added = vstr__split_hdl_def(&pos, &len, split_pos, 1, sects,
                                  flags, added, &bad_ret);
      if (bad_ret)
        return (added);
    }
  }

  return (vstr__split_hdl_end(pos, len, split_pos, sects, limit, flags, added));
}
#undef VSTR__SPLIT_HDL_ERR
#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(split_buf);
VSTR__SYM_ALIAS(split_chrs);
