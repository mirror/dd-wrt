#define VSTR_CACHE_C
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
/* Functions to allow things to be cached for the string */
#include "main.h"
/* NOTE: some assert stuff is in vstr.c also vstr_add.c and vstr_del.c
 * know a bit about the internals of this file as well */


/* we don't call the cb is it's grpallocated, we do that by always having a
 * NULL data */
#define ASSERT_VALID_IOVEC_POS(base) \
 assert((base->conf->cache_pos_cb_iovec == 2) && \
        ((VSTR__CACHE(base)->vec == vstr_cache_get(base, 2)) || \
         ((base->grpalloc_cache >= VSTR_TYPE_CNTL_CONF_GRPALLOC_IOVEC) && \
          VSTR__CACHE(base)->vec && !vstr_cache_get(base, 2))))



static void vstr__cache_cbs(const Vstr_base *base, size_t pos, size_t len,
                            unsigned int type, unsigned int skip_internal)
{
  unsigned int scan = 0;
  unsigned int last = 0;

  ASSERT(!skip_internal ||
         (type == VSTR_TYPE_CACHE_FREE) || (type == VSTR_TYPE_CACHE_NOTHING));
  
  if (skip_internal)
    switch (base->grpalloc_cache)
    {
      case VSTR_TYPE_CNTL_CONF_GRPALLOC_POS:   scan = 1; break;
      case VSTR_TYPE_CNTL_CONF_GRPALLOC_IOVEC: scan = 2; break;
      case VSTR_TYPE_CNTL_CONF_GRPALLOC_CSTR:  scan = 3;
      ASSERT_NO_SWITCH_DEF();
    }
  
  while (scan < VSTR__CACHE(base)->sz)
  {
    void *data = VSTR__CACHE(base)->data[scan];

    if (data)
    {
      if (type != VSTR_TYPE_CACHE_NOTHING)
      {
        void *(*cb_func)(const struct Vstr_base *, size_t, size_t,
                         unsigned int, void *);
        cb_func = base->conf->cache_cbs_ents[scan].cb_func;
        VSTR__CACHE(base)->data[scan] = (*cb_func)(base, pos, len, type, data);
      }

      assert((type != VSTR_TYPE_CACHE_FREE) || !VSTR__CACHE(base)->data[scan]);

      if (VSTR__CACHE(base)->data[scan])
        last = scan;
    }

    ++scan;
  }

  if (last < VSTR__CACHE_INTERNAL_POS_MAX) /* last is one less than the pos */
    ((Vstr_base *)base)->cache_internal = TRUE;
}

void vstr__cache_del(const Vstr_base *base, size_t pos, size_t len)
{
  if (!base->cache_available)
    return;

  assert(VSTR__CACHE(base));
  
  vstr__cache_cbs(base, pos, len, VSTR_TYPE_CACHE_DEL, FALSE);
}

void vstr__cache_add(const Vstr_base *base, size_t pos, size_t len)
{
  if (!base->cache_available)
    return;

  assert(VSTR__CACHE(base));
  
  vstr__cache_cbs(base, pos, len, VSTR_TYPE_CACHE_ADD, FALSE);
}

void vstr_cache_cb_sub(const Vstr_base *base, size_t pos, size_t len)
{
  if (!base->cache_available)
    return;

  assert(VSTR__CACHE(base));
  
  vstr__cache_cbs(base, pos, len, VSTR_TYPE_CACHE_SUB, FALSE);
}

void vstr_cache_cb_free(const Vstr_base *base, unsigned int num)
{
  if (!base->cache_available)
    return;

  assert(VSTR__CACHE(base));

  switch (base->grpalloc_cache)
  {
    case VSTR_TYPE_CNTL_CONF_GRPALLOC_CSTR:
      if (num == 3)
      {
        struct Vstr__cache_data_cstr *data = NULL;

        data = vstr_cache_get(base, base->conf->cache_pos_cb_cstr);
        vstr_ref_del(data->ref);
        data->ref = NULL;
        return;
      }
    case VSTR_TYPE_CNTL_CONF_GRPALLOC_IOVEC: if (num == 2) return;
    case VSTR_TYPE_CNTL_CONF_GRPALLOC_POS:   if (num == 1) return;
      ASSERT_NO_SWITCH_DEF();
  }
  
  if (num && (--num < VSTR__CACHE(base)->sz))
  { /* free'ing a single callbacks data ... */
    void *data = VSTR__CACHE(base)->data[num];

    if (data)
    {
      void *(*cb_func)(const struct Vstr_base *, size_t, size_t,
                       unsigned int, void *);

      cb_func = base->conf->cache_cbs_ents[num].cb_func;
      VSTR__CACHE(base)->data[num] = (*cb_func)(base, 0, 0,
                                                VSTR_TYPE_CACHE_FREE, data);
      
      vstr__cache_cbs(base, 0, 0, VSTR_TYPE_CACHE_NOTHING, TRUE);
    }

    return;
  }

  /* free all */
  vstr__cache_cbs(base, 0, 0, VSTR_TYPE_CACHE_FREE, TRUE);
}

void vstr__cache_cstr_cpy(const Vstr_base *base, size_t pos, size_t len,
                          const Vstr_base *from_base, size_t from_pos)
{
  Vstr__cache_data_cstr *data = NULL;
  Vstr__cache_data_cstr *from_data = NULL;
  unsigned int off      = 3;
  unsigned int from_off = 3;
  
  ASSERT(off      == base->conf->cache_pos_cb_cstr);
  ASSERT(from_off == from_base->conf->cache_pos_cb_cstr);

  if (!base->cache_available)
    return;

  assert(VSTR__CACHE(base));
  
  if (!(data = vstr_cache_get(base, off)))
    return;

  if (!(from_data = vstr_cache_get(from_base, from_off)))
    return;

  if ((data->ref && data->len) || (!from_data->ref || !from_data->len))
    return;

  if ((from_pos <= vstr_sc_poslast(from_data->pos, from_data->len)) &&
      (vstr_sc_poslast(from_pos,       len) >=
       vstr_sc_poslast(from_data->pos, from_data->len)))
  {
    size_t overlap = from_data->len;
    size_t begskip = 0;
    size_t offskip = 0;
    
    if (from_pos < from_data->pos)
      begskip  = (from_data->pos - from_pos);
    else
    {
      overlap -= (from_pos - from_data->pos);
      offskip  = (from_pos - from_data->pos);
    }
    
    if (data->ref)
      vstr_ref_del(data->ref);

    data->ref = vstr_ref_add(from_data->ref);
    data->pos = pos + 1 + begskip;
    data->len = overlap;
    data->sz  = from_data->sz;
    data->off = from_data->off + offskip;
  }
}

static void vstr__cache_iovec_memmove(const Vstr_base *base)
{
  Vstr__cache_data_iovec *vec = VSTR__CACHE(base)->vec;
  size_t sz = sizeof(struct iovec) * base->num;
  unsigned int off = base->conf->iov_min_offset;

  ASSERT((off + base->num) <= vec->sz);

  vstr_wrap_memmove(vec->v + off, vec->v + vec->off, sz);
  vstr_wrap_memmove(vec->t + off, vec->t + vec->off, base->num);

  vec->off = off;
}

int vstr__cache_iovec_alloc(const Vstr_base *base, unsigned int sz)
{
  Vstr__cache_data_iovec *vec = NULL;
  size_t alloc_sz = sz + base->conf->iov_min_alloc + base->conf->iov_min_offset;
  Vstr_conf *conf = base->conf;

  if (!base->cache_available)
    return (FALSE);

  assert(VSTR__CACHE(base));

  vec = VSTR__CACHE(base)->vec;
  if (!vec)
  {
    if (!(vec = VSTR__MK(sizeof(Vstr__cache_data_iovec))))
      goto malloc_bad;
    if (!vstr_cache_set(base, conf->cache_pos_cb_iovec, vec))
      goto cache_vec_malloc_bad;

    VSTR__CACHE(base)->vec = vec;

    if (!(vec->v = VSTR__MK(sizeof(struct iovec) * alloc_sz)))
      goto vec_v_malloc_bad;

    if (!(vec->t = VSTR__MK(alloc_sz)))
      goto vec_t_malloc_bad;

    vec->sz  = alloc_sz;
    vec->off = 0;
  }
  ASSERT_VALID_IOVEC_POS(base);
  ASSERT(vec->v);

  if (!base->iovec_upto_date)
    vec->off = base->conf->iov_min_offset;
  else if ((vec->off > base->conf->iov_min_offset) &&
           (sz > (vec->sz - vec->off)))
    vstr__cache_iovec_memmove(base);

  if (sz > (vec->sz - vec->off))
  {
    struct iovec  *vec_v = NULL;
    unsigned char *vec_t = NULL;

    alloc_sz = sz + base->conf->iov_min_offset;

    if (!VSTR__MV(vec->v, vec_v, sizeof(struct iovec) * alloc_sz))
    {
      ASSERT(!vec_v);
      ASSERT(vec->v);
      conf->malloc_bad = TRUE;
      return (FALSE);
    }
    
    if (!VSTR__MV(vec->t, vec_t, alloc_sz))
    {
      ASSERT(!vec_t);
      ASSERT(vec->t);
      conf->malloc_bad = TRUE;
      return (FALSE);
    }

    vec->sz  = alloc_sz;
  }

  return (TRUE);

 vec_t_malloc_bad:
  VSTR__F(vec->v);
 vec_v_malloc_bad:
  VSTR__CACHE(base)->vec = NULL;
  conf->malloc_bad = FALSE;
  vstr_cache_set(base, conf->cache_pos_cb_iovec, NULL); /* must work */
  ASSERT(!conf->malloc_bad);
 cache_vec_malloc_bad:
  VSTR__F(vec);
 malloc_bad:
  conf->malloc_bad = TRUE;
  return (FALSE);
}

void vstr__free_cache(const Vstr_base *base)
{
  if (!base->cache_available)
    return;

  assert(VSTR__CACHE(base));
  
  ASSERT_VALID_IOVEC_POS(base);

  vstr__cache_cbs(base, 0, 0, VSTR_TYPE_CACHE_FREE, FALSE);

  ((Vstr_base *)base)->iovec_upto_date = FALSE;
}

static int vstr__resize_cache(const Vstr_base *base, unsigned int sz)
{
  Vstr__cache *cache = NULL;

  assert(base->cache_available && VSTR__CACHE(base));
  assert(VSTR__CACHE(base)->sz < sz);

  if (!VSTR__MV(VSTR__CACHE(base), cache,
                sizeof(Vstr__cache) + (sizeof(void *) * sz)))
  {
    base->conf->malloc_bad = TRUE;
    return (FALSE);
  }
  cache = VSTR__CACHE(base);

  vstr_wrap_memset(cache->data + cache->sz, 0,
                   sizeof(void *) * (sz - cache->sz));
  cache->sz = sz;

  return (TRUE);
}

static void *vstr__cache_pos_cb(const Vstr_base *base,
                                size_t pos, size_t VSTR__ATTR_UNUSED(len),
                                unsigned int type, void *passed_data)
{
  Vstr__cache_data_pos *data = passed_data;

  (void)base; /* no warnings when arsserts are off */
  ASSERT(base->grpalloc_cache >= VSTR_TYPE_CNTL_CONF_GRPALLOC_POS);
  
  if (type == VSTR_TYPE_CACHE_FREE)
    return (NULL);

  if (!data->node)
    return (data);

  if ((type == VSTR_TYPE_CACHE_ADD) && (pos >= data->pos))
    return (data);

  if ((type == VSTR_TYPE_CACHE_DEL) && (pos > data->pos))
    return (data);

  if (type == VSTR_TYPE_CACHE_SUB)
    return (data);

  /* can't update data->num properly */
  data->node = NULL;
  return (data);
}

static void *vstr__cache_iovec_cb(const Vstr_base *base,
                                  size_t VSTR__ATTR_UNUSED(pos),
                                  size_t VSTR__ATTR_UNUSED(len),
                                  unsigned int type, void *passed_data)
{
  Vstr__cache_data_iovec *data = passed_data;

  (void)base; /* no warnings when arsserts are off */
  assert(VSTR__CACHE(base)->vec == data);
  ASSERT(base->grpalloc_cache < VSTR_TYPE_CNTL_CONF_GRPALLOC_IOVEC);
  
  if (type == VSTR_TYPE_CACHE_FREE)
  {
    assert(MALLOC_CHECK_MEM(data));
    VSTR__F(data->v);
    VSTR__F(data->t);
    VSTR__F(data);
    VSTR__CACHE(base)->vec = NULL;
    
    return (NULL);
  }

  /* NOTE: add/del/sub/etc. handled by hand in the code, as we can do a lot more
   * optimizations there ... :O */

  return (data);
}

static void *vstr__cache_cstr_cb(const Vstr_base *base,
                                 size_t pos, size_t len,
                                 unsigned int type, void *passed_data)
{
  Vstr__cache_data_cstr *data = passed_data;
  const size_t end_pos = vstr_sc_poslast(pos, len);
  const size_t data_end_pos = vstr_sc_poslast(data->pos, data->len);

  ASSERT(passed_data);

  if (type == VSTR_TYPE_CACHE_FREE)
  {
    vstr_ref_del(data->ref);
    data->ref = NULL;
    if (base->grpalloc_cache < VSTR_TYPE_CNTL_CONF_GRPALLOC_CSTR)
      VSTR__F(data);
    return (NULL);
  }

  if (!data->ref || !data->len)
    return (data);

  if (data_end_pos < pos)
    return (data);

  if (type == VSTR_TYPE_CACHE_ADD)
  {
    if (data_end_pos == pos)
      return (data);

    if (data->pos > pos)
    {
      data->pos += len;
      return (data);
    }
  }
  else if (data->pos > end_pos)
  {
    if (type == VSTR_TYPE_CACHE_DEL)
      data->pos -= len;

    return (data);
  }
  else if ((type == VSTR_TYPE_CACHE_DEL) && (data_end_pos > end_pos))
  { /* delete some of the begining of the cache cstr */
    data->len -= vstr_sc_posdiff(data->pos, end_pos);
    data->off += vstr_sc_posdiff(data->pos, end_pos);

    data->pos = pos;

    return (data);
  }

  data->len = 0;

  return (data);
}

int vstr__cache_conf_init(Vstr_conf *conf)
{
  if (!(conf->cache_cbs_ents = VSTR__MK(sizeof(Vstr_cache_cb) * 3)))
    return (FALSE);

  conf->cache_cbs_sz = 3;

  conf->cache_pos_cb_sects = 0; /* NOTE: should really be in vstr_sects.c ...
                                 * but it's not a big problem */

  conf->cache_cbs_ents[0].name = "/vstr__/pos";
  conf->cache_cbs_ents[0].cb_func = vstr__cache_pos_cb;
  conf->cache_pos_cb_pos = 1;

  conf->cache_cbs_ents[1].name = "/vstr__/iovec";
  conf->cache_cbs_ents[1].cb_func = vstr__cache_iovec_cb;
  conf->cache_pos_cb_iovec = 2;

  conf->cache_cbs_ents[2].name = "/vstr__/cstr";
  conf->cache_cbs_ents[2].cb_func = vstr__cache_cstr_cb;
  conf->cache_pos_cb_cstr = 3;

  assert(VSTR__CACHE_INTERNAL_POS_MAX == 2);

  return (TRUE);
}

/* initial stuff done in vstr.c */
unsigned int vstr_cache_srch(Vstr_conf *passed_conf, const char *name)
{
  Vstr_conf *conf = passed_conf ? passed_conf : vstr__options.def;
  unsigned int pos = 0;

  ASSERT(name);
  
  while (pos < conf->cache_cbs_sz)
  {
    if (!strcmp(name, conf->cache_cbs_ents[pos++].name))
      return (pos);
  }

  return (0);
}

unsigned int vstr_cache_add(Vstr_conf *passed_conf, const char *name,
                            void *(*func)(const Vstr_base *, size_t, size_t,
                                          unsigned int, void *))
{
  Vstr_conf *conf = passed_conf ? passed_conf : vstr__options.def;
  unsigned int sz = conf->cache_cbs_sz + 1;
  struct Vstr_cache_cb *ents = NULL;

  ASSERT(!vstr_cache_srch(conf, name));
  
  if (!VSTR__MV(conf->cache_cbs_ents, ents, sizeof(Vstr_cache_cb) * sz))
  {
    conf->malloc_bad = TRUE;
    return (0);
  }
  conf->cache_cbs_sz = sz;

  conf->cache_cbs_ents[sz - 1].name    = name;
  conf->cache_cbs_ents[sz - 1].cb_func = func;

  ASSERT(vstr_cache_srch(conf, name));
  
  return (sz);
}

int vstr_cache_set(const Vstr_base *base, unsigned int pos, void *data)
{
  ASSERT_RET(pos, FALSE);

  if (!base->cache_available)
    return (FALSE);

  assert(VSTR__CACHE(base));

  if (pos-- > VSTR__CACHE(base)->sz)
  {
    if (!vstr__resize_cache(base, pos + 1))
      return (FALSE);
  }

  /* we've set data for a user cache */
  if ((pos > VSTR__CACHE_INTERNAL_POS_MAX) && data)
    ((Vstr_base *)base)->cache_internal = FALSE;

  VSTR__CACHE(base)->data[pos] = data;

  return (TRUE);
}

int vstr__cache_subset_cbs(Vstr_conf *ful_conf, Vstr_conf *sub_conf)
{
  unsigned int scan = 0;
  struct Vstr_cache_cb *sub_cbs_ents = sub_conf->cache_cbs_ents;
  struct Vstr_cache_cb *ful_cbs_ents = ful_conf->cache_cbs_ents;

  if (sub_conf->cache_cbs_sz < ful_conf->cache_cbs_sz)
    return (FALSE);

  while (scan < ful_conf->cache_cbs_sz)
  {
    if (strcmp(ful_cbs_ents[scan].name, sub_cbs_ents[scan].name))
      return (FALSE); /* different ... so bad */

    /* NOTE: While "possible" this is crack if this isn't valid and is
     * bound to cause untracable bugs  */
    ASSERT(ful_cbs_ents[scan].cb_func == sub_cbs_ents[scan].cb_func);
    
    ++scan;
  }

  return (TRUE);
}

int vstr__cache_dup_cbs(Vstr_conf *conf, Vstr_conf *dupconf)
{
  Vstr_cache_cb *ents = conf->cache_cbs_ents;
  unsigned int scan = 0;

  if (conf->cache_cbs_sz < dupconf->cache_cbs_sz)
  {
    if (!VSTR__MV(conf->cache_cbs_ents, ents,
                  sizeof(Vstr_cache_cb) * dupconf->cache_cbs_sz))
    {
      conf->malloc_bad = TRUE;
      return (FALSE);
    }
    conf->cache_cbs_sz = dupconf->cache_cbs_sz;
  }

  while (scan < dupconf->cache_cbs_sz)
  {
    ents[scan] = dupconf->cache_cbs_ents[scan];
    ++scan;
  }

  return (TRUE);
}

/* FIXME: changed in 1.0.10 --
 * always allocated now ... needed for ABI compat. however it can't be called */
extern struct Vstr__cache_data_pos *
vstr_extern_inline_make_cache_pos(const Vstr_base *);
struct Vstr__cache_data_pos *
vstr_extern_inline_make_cache_pos(const Vstr_base *base)
{
  (void)base;
  ASSERT(FALSE);
  return (NULL);
}
#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(cache_add);
VSTR__SYM_ALIAS(cache_cb_free);
VSTR__SYM_ALIAS(cache_cb_sub);
VSTR__SYM_ALIAS(cache_set);
VSTR__SYM_ALIAS(cache_srch);
