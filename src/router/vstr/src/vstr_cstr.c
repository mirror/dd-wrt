#define VSTR_CSTR_C
/*
 *  Copyright (C) 1999, 2000, 2001, 2002, 2003, 2005  James Antill
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
/* Functions to allow export of a "C string" like interface */
#include "main.h"

static Vstr_ref *vstr__export_cstr_ref(const Vstr_base *base,
                                       size_t pos, size_t len)
{
  Vstr_ref *ref = NULL;

  if (!(ref = vstr_ref_make_malloc(len + 1)))
  {
    base->conf->malloc_bad = TRUE;
    return (NULL);
  }

  assert(((Vstr__buf_ref *)ref)->buf == ref->ptr);

  vstr_export_cstr_buf(base, pos, len, ref->ptr, len + 1);

  return (ref);
}

static Vstr__cache_data_cstr *vstr__export_cstr_cache(const Vstr_base *base,
                                                      size_t pos, size_t len,
                                                      size_t *ret_off)
{
  Vstr_ref *ref = NULL;
  Vstr__cache_data_cstr *data = NULL;
  unsigned int off = 3;

  ASSERT_RET(base && ret_off && pos &&
             (((pos <= base->len) &&
               (vstr_sc_poslast(pos, len) <= base->len)) || !len), NULL);

  ASSERT(off == base->conf->cache_pos_cb_cstr);

  if (!(data = vstr_cache_get(base, off)))
  {
    int ret = FALSE;

    ASSERT(base->grpalloc_cache < VSTR_TYPE_CNTL_CONF_GRPALLOC_CSTR);
    
    if (!vstr_cache_set(base, off, NULL))
      return (NULL);

    if (!(data = VSTR__MK(sizeof(Vstr__cache_data_cstr))))
    {
      base->conf->malloc_bad = TRUE;
      return (NULL);
    }
    data->ref = NULL;
    
    ret = vstr_cache_set(base, off, data);
    ASSERT(ret);
  }

  if (data->ref && data->len)
  {
    ASSERT(data->sz);
    ASSERT(((Vstr__buf_ref *)data->ref)->buf == data->ref->ptr);

    *ret_off = data->off;

    if (pos >= data->pos)
    {
      size_t tmp = (pos - data->pos);
      if ((data->len - tmp) == len)
      {
        *ret_off += tmp;
        return (data);
      }
    }
  }
  if (data->ref)
  {
    if ((data->sz <= len) || (data->ref->ref != 1))
    {
      vstr_ref_del(data->ref);
      data->ref = NULL;
    }
    else
    { /* can overwrite previous cache entry */
      ref = data->ref;
      vstr_export_cstr_buf(base, pos, len, ref->ptr, len + 1);

      goto export_new_cstr;
    }
  }

  if (!(ref = vstr__export_cstr_ref(base, pos, len)))
    return (NULL);
  data->sz = len + 1;

 export_new_cstr:
  data->ref = ref;
  data->off = 0;
  data->pos = pos;
  data->len = len;

  *ret_off = 0;
  return (data);
}

const char *vstr_export_cstr_ptr(const Vstr_base *base, size_t pos, size_t len)
{
  Vstr__cache_data_cstr *data = NULL;
  size_t off = 0;

  if (!(data = vstr__export_cstr_cache(base, pos, len, &off)))
    return (NULL);

  ASSERT(vstr__check_real_nodes(base));

  return (((char *)data->ref->ptr) + off);
}

char *vstr_export_cstr_malloc(const Vstr_base *base, size_t pos, size_t len)
{
  void *ptr = malloc(len + 1);

  if (!ptr)
  {
    base->conf->malloc_bad = TRUE;
    return (NULL);
  }

  vstr_export_cstr_buf(base, pos, len, ptr, len + 1);

  return (ptr);
}

Vstr_ref *vstr_export_cstr_ref(const Vstr_base *base, size_t pos, size_t len,
                               size_t *ret_off)
{
  Vstr__cache_data_cstr *data = NULL;

  ASSERT_RET(base && ret_off, NULL);

  if (!base->cache_available)
  {
    Vstr_ref *ref = vstr__export_cstr_ref(base, pos, len);

    if (!ref)
      return (NULL);

    *ret_off = 0;
    return (ref);
  }

  if (!(data = vstr__export_cstr_cache(base, pos, len, ret_off)))
    return (NULL);

  return (vstr_ref_add(data->ref));
}

size_t vstr_export_cstr_buf(const Vstr_base *base, size_t pos, size_t len,
                            void *buf, size_t buf_len)
{
  size_t cpy_len = len;

  ASSERT_RET(base && buf && pos &&
             (((pos <= base->len) &&
               (vstr_sc_poslast(pos, len) <= base->len)) || !len), 0);
  
  if (!buf_len)
    return (0);

  if (cpy_len >= buf_len)
    cpy_len = (buf_len - 1);

  vstr_export_buf(base, pos, len, buf, cpy_len);
  ((char *)buf)[cpy_len] = 0;

  return (cpy_len + 1);
}
#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(export_cstr_buf);
VSTR__SYM_ALIAS(export_cstr_malloc);
VSTR__SYM_ALIAS(export_cstr_ptr);
VSTR__SYM_ALIAS(export_cstr_ref);
