#define VSTR_SECT_C
/*
 *  Copyright (C) 2002, 2003  James Antill
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
/* functions to manage a Vstr section */
#include "main.h"

Vstr_sects *vstr_sects_make(unsigned int beg_num)
{
  Vstr_sects *sects = VSTR__MK(sizeof(Vstr_sects));
  Vstr_sect_node *ptr = NULL;
  if (!sects)
    return (NULL);

  if (beg_num && !(ptr = VSTR__MK(sizeof(Vstr_sect_node) * beg_num)))
  {
    VSTR__F(sects);
    return (NULL);
  }

  VSTR_SECTS_INIT(sects, beg_num, ptr, TRUE);

  return (sects);
}

void vstr_sects_free(Vstr_sects *sects)
{
  if (!sects)
    return;

  if (sects->free_ptr)
    VSTR__F(sects->ptr);

  VSTR__F(sects);
}

static int vstr__sects_mv(Vstr_sects *sects, unsigned int sz)
{
  struct Vstr_sect_node *tmp_ptr = NULL;

  if (!VSTR__MV(sects->ptr, tmp_ptr, sizeof(Vstr_sect_node) * sz))
  {
    sects->malloc_bad = TRUE;
    return (FALSE);
  }
  sects->sz = sz;

  return (TRUE);
}

static int vstr__sects_add(Vstr_sects *sects)
{
  unsigned int sz = sects->sz;

  if (!sz)
  {
    if (!(sects->ptr = VSTR__MK(sizeof(Vstr_sect_node) * 1)))
      goto malloc_fail;

    sects->sz = 1;
    return (TRUE);
  }
  
  if (sects->alloc_double)
    sz <<= 1;
    
  if (sz <= sects->sz)
    sz = sects->sz + 1;
  
  if (sz <= sects->sz)
    goto malloc_fail;
  
  return (vstr__sects_mv(sects, sz));
  
 malloc_fail:
  sects->malloc_bad = TRUE;
  return (FALSE);
}

static int vstr__sects_del(Vstr_sects *sects)
{
  unsigned int sz = sects->sz;

  sz >>= 1;
  assert(sz >= sects->num);

  return (vstr__sects_mv(sects, sz));
}

int vstr_extern_inline_sects_add(Vstr_sects *sects,
                                 size_t VSTR__ATTR_UNUSED(pos),
                                 size_t VSTR__ATTR_UNUSED(len))
{
  /* see vstr-extern.h for why */
  assert(sizeof(struct Vstr_sects) >= sizeof(struct Vstr_sect_node));

  return (vstr__sects_add(sects));
}

int vstr_sects_del(Vstr_sects *sects, unsigned int num)
{
  ASSERT_RET((sects->sz && num), FALSE);
  ASSERT_RET((sects->num >= num), FALSE);

  if (!sects->ptr[num - 1].pos)
    return (FALSE);

  sects->ptr[num - 1].pos = 0;

  while (sects->num && !sects->ptr[sects->num - 1].pos)
    --sects->num;

  if (sects->can_del_sz && (sects->num < (sects->sz / 2)))
    vstr__sects_del(sects); /* can't return this error */

  return (TRUE);
}

unsigned int vstr_sects_srch(Vstr_sects *sects, size_t pos, size_t len)
{
  unsigned int count = 0;

  if (!sects->sz)
    return (0);

  while (count++ < sects->num)
  {
    size_t scan_pos = VSTR_SECTS_NUM(sects, count)->pos;
    size_t scan_len = VSTR_SECTS_NUM(sects, count)->len;

    if ((scan_pos == pos) && (scan_len == len))
      return (count);
  }

  return (0);
}

int vstr_sects_foreach(const Vstr_base *base,
                       Vstr_sects *sects, const unsigned int flags,
                       unsigned int (*foreach_func)(const Vstr_base *,
                                                    size_t, size_t, void *),
                       void *data)
{
  unsigned int count = 0;
  unsigned int scan = 0;

  if (!sects->sz)
    return (0);

  if (flags & VSTR_FLAG_SECTS_FOREACH_BACKWARD)
    scan = sects->num;

  while ((!(flags & VSTR_FLAG_SECTS_FOREACH_BACKWARD) &&
          (scan < sects->num)) ||
         ((flags & VSTR_FLAG_SECTS_FOREACH_BACKWARD) && scan))
  {
    size_t pos = 0;
    size_t len = 0;

    if (flags & VSTR_FLAG_SECTS_FOREACH_BACKWARD)
      --scan;

    pos = sects->ptr[scan].pos;
    len = sects->ptr[scan].len;

    if (pos && (len || (flags & VSTR_FLAG_SECTS_FOREACH_ALLOW_NULL)))
    {
      ++count;

      switch ((*foreach_func)(base, pos, len, data))
      {
        case VSTR_TYPE_SECTS_FOREACH_RET:
          goto shorten_and_return;

        case VSTR_TYPE_SECTS_FOREACH_DEL:
          sects->ptr[scan].pos = 0;
          /* FALL THROUGH */
        case VSTR_TYPE_SECTS_FOREACH_DEF:
          
          ASSERT_NO_SWITCH_DEF();
      }
    }

    if (!(flags & VSTR_FLAG_SECTS_FOREACH_BACKWARD))
      ++scan;
  }

 shorten_and_return:
  while (sects->num && !sects->ptr[sects->num - 1].pos)
    --sects->num;

  if (sects->can_del_sz && (sects->num < (sects->sz / 2)))
    vstr__sects_del(sects); /* can't return this error */

  return (count);
}

typedef struct Vstr__sects_cache_data
{
 unsigned int sz;
 unsigned int len;
 Vstr_sects *VSTR__STRUCT_HACK_ARRAY(updates);
} Vstr__sects_cache_data;

static void *vstr__sects_update_cb(const Vstr_base *base,size_t pos, size_t len,
                                   unsigned int type, void *passed_data)
{
  Vstr__sects_cache_data *data = passed_data;
  unsigned int count = 0;

  ASSERT(base->conf->cache_pos_cb_sects);
  ASSERT(data == vstr_cache_get(base, base->conf->cache_pos_cb_sects));

  if (type == VSTR_TYPE_CACHE_FREE)
  {
    VSTR__F(data);
    return (NULL);
  }

  if (type == VSTR_TYPE_CACHE_SUB) /* do nothing for substitutions ... */
    return (data);

  while (count < data->len)
  {
    Vstr_sects *sects = data->updates[count];
    unsigned int scan = 0;

    switch (type)
    {
      case VSTR_TYPE_CACHE_ADD:
        while (scan < sects->num)
        {
          if (sects->ptr[scan].pos && sects->ptr[scan].len)
          {
            if (pos < sects->ptr[scan].pos)
              sects->ptr[scan].pos += len;
            if ((pos >= sects->ptr[scan].pos) &&
                (pos < (sects->ptr[scan].pos + sects->ptr[scan].len - 1)))
              sects->ptr[scan].len += len;
          }

          ++scan;
        }
        break;

      case VSTR_TYPE_CACHE_DEL:
        while (scan < sects->num)
        {
          if (sects->ptr[scan].pos && sects->ptr[scan].len)
          {
            if (pos <= sects->ptr[scan].pos)
            {
              size_t tmp = sects->ptr[scan].pos - pos;

              if (tmp >= len)
                sects->ptr[scan].pos -= len;
              else
              {
                len -= tmp;
                if (len >= sects->ptr[scan].len)
                  sects->ptr[scan].pos = 0;
                else
                {
                  sects->ptr[scan].pos -= tmp;
                  sects->ptr[scan].len -= len;
                }
              }
            }
            else if ((pos > sects->ptr[scan].pos) &&
                     (pos <= (sects->ptr[scan].pos + sects->ptr[scan].len - 1)))
            {
              size_t tmp = pos - sects->ptr[scan].pos;

              if (len >= (sects->ptr[scan].len - tmp))
                sects->ptr[scan].len = tmp;
              else
                sects->ptr[scan].len -= len;
            }
          }

          ++scan;
        }
        ASSERT_NO_SWITCH_DEF();
    }

    ++count;
  }

  return (data);
}

static Vstr_sects **vstr__sects_update_srch(Vstr__sects_cache_data *data,
                                            Vstr_sects *sects)
{
  unsigned int count = 0;

  while (count < data->len)
  {
    if (data->updates[count] == sects)
      return (&data->updates[count]);

    ++count;
  }

  return (NULL);
}

static void vstr__sects_update_del(Vstr__sects_cache_data *data,
                                   Vstr_sects **sects)
{
  Vstr_sects **end = (data->updates + (data->len - 1));

  --data->len;

  if (sects != end)
    vstr_wrap_memmove(sects, sects + 1,
                      (end - sects) * sizeof(Vstr_sects *));
}

int vstr_sects_update_add(const Vstr_base *base,
                          Vstr_sects *sects)
{
  Vstr__sects_cache_data *data = NULL;
  unsigned int sz = 1;

  if (!base->conf->cache_pos_cb_sects)
  {
    unsigned int tmp = 0;

    tmp = vstr_cache_add(base->conf, "/vstr__/sects/update",
                         vstr__sects_update_cb);

    if (!tmp)
      goto malloc_bad;

    base->conf->cache_pos_cb_sects = tmp;
  }

  if (!(data = vstr_cache_get(base, base->conf->cache_pos_cb_sects)))
  {
    if (!vstr_cache_set(base, base->conf->cache_pos_cb_sects, NULL))
      goto malloc_bad;

    data = VSTR__MK(sizeof(Vstr__sects_cache_data) +
                    (sz * sizeof(Vstr_sects *)));
    if (!data)
      goto malloc_bad;

    data->sz = 1;
    data->len = 0;

    vstr_cache_set(base, base->conf->cache_pos_cb_sects, data);
  }

  /* it can't be valid to have the same sects twice */
  ASSERT(!vstr__sects_update_srch(data, sects));
  
  sz = data->len + 1;

  /* this is basically impossible to test (size overflow)...
   * done this way for coverage */
  ASSERT_RET((sz > data->len) || !(base->conf->malloc_bad = TRUE), FALSE);
  
  ASSERT(data->sz);
  ASSERT(data->len <= data->sz);
  
  if (data->len >= data->sz)
  {
    Vstr__sects_cache_data *tmp_data = NULL;

    if (!(VSTR__MV(data, tmp_data, sizeof(Vstr__sects_cache_data) +
                   (sz * sizeof(Vstr_sects *)))))
      goto malloc_bad;

    data->sz = data->len + 1;

    vstr_cache_set(base, base->conf->cache_pos_cb_sects, data);
  }

  data->updates[data->len] = sects;
  ++data->len;

  return (TRUE);

 malloc_bad:
  base->conf->malloc_bad = TRUE;
  return (FALSE);
}

int vstr_sects_update_del(const Vstr_base *base,
                          Vstr_sects *sects)
{
  Vstr__sects_cache_data *data = NULL;
  Vstr_sects **srch = NULL;

  if (!sects)
    return (FALSE);
  
  ASSERT_RET(base->conf->cache_pos_cb_sects, FALSE);

  data = vstr_cache_get(base, base->conf->cache_pos_cb_sects);
  ASSERT_RET(data, FALSE);

  srch = vstr__sects_update_srch(data, sects);
  ASSERT_RET(srch, FALSE);

  vstr__sects_update_del(data, srch);

  if (!data->len)
  {
    VSTR__F(data);
    vstr_cache_set(base, base->conf->cache_pos_cb_sects, NULL);
  }

  return (TRUE);
}
#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(extern_inline_sects_add);
VSTR__SYM_ALIAS(sects_del);
VSTR__SYM_ALIAS(sects_foreach);
VSTR__SYM_ALIAS(sects_free);
VSTR__SYM_ALIAS(sects_make);
VSTR__SYM_ALIAS(sects_srch);
VSTR__SYM_ALIAS(sects_update_add);
VSTR__SYM_ALIAS(sects_update_del);
