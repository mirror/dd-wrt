#define VSTR_C
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
/* master file, contains most of the base functions */
#include "main.h"

MALLOC_CHECK_DECL();

/* validity checking function... only called from asserts */
#ifndef NDEBUG
static void vstr__cache_iovec_check(const Vstr_base *base)
{
  unsigned int count = 0;
  Vstr_node *scan = NULL;

  if (!base->iovec_upto_date || !base->beg)
    return;

  count = VSTR__CACHE(base)->vec->off;
  scan = base->beg;

  assert(scan->len > base->used);
  assert(VSTR__CACHE(base)->vec->sz >= base->num);
  assert(VSTR__CACHE(base)->vec->v[count].iov_len == (size_t)(scan->len - base->used));
  if (scan->type == VSTR_TYPE_NODE_NON)
    assert(!VSTR__CACHE(base)->vec->v[count].iov_base);
  else
    assert(VSTR__CACHE(base)->vec->v[count].iov_base ==
           (vstr_export__node_ptr(scan) + base->used));
  assert(VSTR__CACHE(base)->vec->t[count] == scan->type);

  if (!(scan = scan->next))
    ASSERT(base->beg == base->end);

  ++count;
  while (scan)
  {
    assert(VSTR__CACHE(base)->vec->t[count] == scan->type);
    assert(VSTR__CACHE(base)->vec->v[count].iov_len == scan->len);
    assert(VSTR__CACHE(base)->vec->v[count].iov_base ==
           vstr_export__node_ptr(scan));

    ++count;

    if (!scan->next)
      assert(scan == base->end);

    scan = scan->next;
  }
}

static void vstr__cache_cstr_check(const Vstr_base *base)
{
  Vstr__cache_data_cstr *data = NULL;
  const char *ptr = NULL;

  ASSERT(base->conf->cache_pos_cb_cstr == 3);

  if (!(data = vstr_cache_get(base, base->conf->cache_pos_cb_cstr)))
    return;
  if (!data->ref || !data->len)
    return;
  
  ASSERT(data->len < data->sz);

  ptr  = data->ref->ptr;
  ptr += data->off;

  { /* special case vstr_cmp_buf_eq() ... works with _NON data */
    Vstr_iter iter[1];

    ASSERT(vstr_iter_fwd_beg(base, data->pos, data->len, iter));

    do
    {
      if (iter->node->type != VSTR_TYPE_NODE_NON)
        ASSERT(!memcmp(iter->ptr, ptr, iter->len));
      ptr += iter->len;
    } while (vstr_iter_fwd_nxt(iter));
  }
}

int vstr__check_real_nodes(const Vstr_base *base)
{
  size_t len = 0;
  size_t num = 0;
  unsigned int node_buf_used = FALSE;
  unsigned int node_non_used = FALSE;
  unsigned int node_ptr_used = FALSE;
  unsigned int node_ref_used = FALSE;
  Vstr_node *scan = base->beg;

  MALLOC_CHECK_MEM(base);

  ASSERT(!base->used || (base->used < base->beg->len));
  ASSERT(!base->used || (base->beg->type == VSTR_TYPE_NODE_BUF));
  ASSERT((base->beg == base->end) || (base->num > 1));
  
  while (scan)
  {
    ASSERT(scan->len);

    switch (scan->type)
    {
      case VSTR_TYPE_NODE_BUF: node_buf_used = TRUE; break;
      case VSTR_TYPE_NODE_NON: node_non_used = TRUE; break;
      case VSTR_TYPE_NODE_PTR: node_ptr_used = TRUE; break;
      case VSTR_TYPE_NODE_REF: node_ref_used = TRUE;
        
        ASSERT_NO_SWITCH_DEF();
    }

    len += scan->len;

    ++num;

    scan = scan->next;
  }

  /* it can be TRUE in the base and FALSE in reallity */
  assert(!node_buf_used || base->node_buf_used);
  assert(!node_non_used || base->node_non_used);
  assert(!node_ptr_used || base->node_ptr_used);
  assert(!node_ref_used || base->node_ref_used);

  assert(!!base->beg == !!base->end);
  assert(((len - base->used) == base->len) && (num == base->num));

  vstr__cache_iovec_check(base);
  vstr__cache_cstr_check(base);

  return (TRUE);
}

int vstr__check_spare_nodes(const Vstr_conf *conf)
{
  unsigned int num = 0;
  Vstr_node *scan = NULL;

  MALLOC_CHECK_MEM(conf);
  
  num = 0;
  scan = (Vstr_node *)conf->spare_buf_beg;
  while (scan)
  {
    ++num;

    assert(scan->type == VSTR_TYPE_NODE_BUF);

    scan = scan->next;
  }
  assert(conf->spare_buf_num == num);

  num = 0;
  scan = (Vstr_node *)conf->spare_non_beg;
  while (scan)
  {
    ++num;

    assert(scan->type == VSTR_TYPE_NODE_NON);

    scan = scan->next;
  }
  assert(conf->spare_non_num == num);

  num = 0;
  scan = (Vstr_node *)conf->spare_ptr_beg;
  while (scan)
  {
    ++num;

    assert(scan->type == VSTR_TYPE_NODE_PTR);

    scan = scan->next;
  }
  assert(conf->spare_ptr_num == num);

  num = 0;
  scan = (Vstr_node *)conf->spare_ref_beg;
  while (scan)
  {
    ++num;

    assert(scan->type == VSTR_TYPE_NODE_REF);

    scan = scan->next;
  }
  assert(conf->spare_ref_num == num);

  return (TRUE);
}

static int vstr__check_spare_base(const Vstr_conf *conf)
{
  Vstr_base *scan = NULL;
  unsigned int num = 0;
  
  ASSERT(conf);

  scan = conf->spare_base_beg;
  while (scan)
  { /* FIXME: ... BIG HACK */
    ++num;
    
    scan = (Vstr_base *)scan->beg;
  }  

  ASSERT(num == conf->spare_base_num);
  
  return (TRUE);
}
#endif

/* real functions... */


size_t vstr__loc_thou_grp_strlen(const char *passed_str)
{
  const unsigned char *str = (const unsigned char *)passed_str;
  size_t len = 0;

  while (*str && (*str < SCHAR_MAX))
  {
    ++len;
    ++str;
  }

  if (*str)
    ++len;

  return (len);
}

Vstr_locale_num_base *
vstr__loc_num_srch(Vstr_locale *loc, unsigned int num_base, int clone)
{
  Vstr_locale_num_base *scan = loc->num_beg;
  Vstr_locale_num_base *def = NULL;

  while (scan)
  {
    if (scan->num_base == num_base)
      return (scan);
    
    scan = scan->next;
  }

  ASSERT(num_base);

  def = vstr__loc_num_srch(loc, 0, FALSE);
  
  if (clone)
  {
    if (!(scan = VSTR__MK(sizeof(Vstr_locale_num_base))))
      return (NULL);

    scan->next              = loc->num_beg;

    scan->num_base          = num_base;
    scan->grouping          = vstr_ref_add(def->grouping);
    scan->thousands_sep_ref = vstr_ref_add(def->thousands_sep_ref);
    scan->thousands_sep_len =              def->thousands_sep_len;
    scan->decimal_point_ref = vstr_ref_add(def->decimal_point_ref);
    scan->decimal_point_len =              def->decimal_point_len;
    
    loc->num_beg            = scan;

    return (scan);
  }
  
  return (def);
}

static int vstr__make_conf_loc_vals(Vstr_locale *loc,
                                    const char *name, const char *grp,
                                    const char *thou, const char *deci)
{
  size_t name_len = strlen(name);
  size_t grp_len  = vstr__loc_thou_grp_strlen(grp);
  size_t thou_len = strlen(thou);
  size_t deci_len = strlen(deci);
  Vstr_ref *ref = NULL;
  
  if (!(ref = vstr_ref_make_memdup(name, name_len + 1)))
    goto fail_numeric;
  loc->name_lc_numeric_ref        = ref;
  loc->name_lc_numeric_len        = name_len;
  
  if (!(ref = vstr_ref_make_malloc(grp_len + 1)))
    goto fail_grp;
  loc->num_beg->grouping          = ref;

  /* Grouping is different we have to make sure it is 0 terminated */
  if (grp_len)
    vstr_wrap_memcpy(loc->num_beg->grouping->ptr, grp, grp_len);
  ((char *)loc->num_beg->grouping->ptr)[grp_len] = 0;
  
  if (!(ref = vstr_ref_make_memdup(thou, thou_len + 1)))
    goto fail_thou;
  loc->num_beg->thousands_sep_ref = ref;
  loc->num_beg->thousands_sep_len = thou_len;
  
  if (!(ref = vstr_ref_make_memdup(deci, deci_len + 1)))
    goto fail_deci;
  loc->num_beg->decimal_point_ref = ref;
  loc->num_beg->decimal_point_len = deci_len;
  
  return (TRUE);
  
 fail_deci:
  vstr_ref_del(loc->num_beg->thousands_sep_ref);
 fail_thou:
  vstr_ref_del(loc->num_beg->grouping);
 fail_grp:
  vstr_ref_del(loc->name_lc_numeric_ref);
 fail_numeric:
  return (FALSE);
}

static int vstr__make_conf_loc_def_numeric(Vstr_conf *conf)
{
  ASSERT(conf);
  ASSERT(conf->loc);
  ASSERT(conf->loc->num_beg);
  
  conf->loc->num_beg->num_base                = 0;
  conf->loc->num_beg->next                    = NULL;

  if (!(conf->loc->null_ref                   = vstr_ref_make_strdup("(null)")))
    goto fail_null;
  conf->loc->null_len                         = strlen("(null)");
  
  if (!vstr__make_conf_loc_vals(conf->loc, "C", "", "", "."))
    goto fail_numeric;

  return (TRUE);

  /* vstr_ref_del(conf->loc->null_ref); */
 fail_numeric:
  vstr_ref_del(conf->loc->null_ref);
 fail_null:

  return (FALSE);
}

int vstr__make_conf_loc_numeric(Vstr_conf *conf, const char *name)
{
  struct lconv *sys_loc = NULL;
  const char *tmp = NULL;
  Vstr_locale loc[1];
  Vstr_locale_num_base num_beg[1];

  ASSERT(conf);
  ASSERT(conf->loc);
  ASSERT(conf->loc->num_beg);
  
  loc->num_beg = num_beg;
  
  if (name)
    tmp = setlocale(LC_NUMERIC, name);

  if (!(sys_loc = localeconv()))
    return (FALSE);
  else
  {
    const char *name_numeric = NULL;
    
    if (MALLOC_CHECK_DEC() ||
        !(name_numeric = setlocale(LC_NUMERIC, NULL))) /* name for "name" */
      name_numeric = "C";
    
    if (!vstr__make_conf_loc_vals(loc, name_numeric,
                                  SYS_LOC(grouping), SYS_LOC(thousands_sep),
                                  SYS_LOC(decimal_point)))
      goto fail_numeric;
  }

  while (TRUE)
  {
    Vstr_locale_num_base *next = conf->loc->num_beg->next;
    
    vstr_ref_del(conf->loc->num_beg->grouping);
    vstr_ref_del(conf->loc->num_beg->thousands_sep_ref);
    vstr_ref_del(conf->loc->num_beg->decimal_point_ref);

    if (!next)
      break;

    VSTR__F(conf->loc->num_beg);
    conf->loc->num_beg = next;
  }

  vstr_ref_del(conf->loc->name_lc_numeric_ref);
  conf->loc->name_lc_numeric_ref        = loc->name_lc_numeric_ref;
  conf->loc->name_lc_numeric_len        = loc->name_lc_numeric_len;

  conf->loc->num_beg->num_base          = 0;
  conf->loc->num_beg->grouping          = loc->num_beg->grouping;

  conf->loc->num_beg->thousands_sep_ref = loc->num_beg->thousands_sep_ref;
  conf->loc->num_beg->thousands_sep_len = loc->num_beg->thousands_sep_len;

  conf->loc->num_beg->decimal_point_ref = loc->num_beg->decimal_point_ref;
  conf->loc->num_beg->decimal_point_len = loc->num_beg->decimal_point_len;

  if (tmp)
    setlocale(LC_NUMERIC, tmp);

  return (TRUE);

 fail_numeric:

  if (tmp)
    setlocale(LC_NUMERIC, tmp);

  return (FALSE);
}

Vstr_conf *vstr_make_conf(void)
{
  Vstr_conf *conf = VSTR__MK(sizeof(Vstr_conf));

  if (!conf)
    goto fail_conf;

  if (!vstr__cache_conf_init(conf))
    goto fail_cache;

  if (!vstr__data_conf_init(conf))
    goto fail_data;

  if (!(conf->loc = VSTR__MK(sizeof(Vstr_locale))))
    goto fail_loc;

  if (!(conf->loc->num_beg = VSTR__MK(sizeof(Vstr_locale_num_base))))
    goto fail_loc_num;

  if (!vstr__make_conf_loc_def_numeric(conf))
    goto fail_numeric;

  conf->spare_buf_num = 0;
  conf->spare_buf_beg = NULL;

  conf->spare_non_num = 0;
  conf->spare_non_beg = NULL;

  conf->spare_ptr_num = 0;
  conf->spare_ptr_beg = NULL;

  conf->spare_ref_num = 0;
  conf->spare_ref_beg = NULL;

  conf->iov_min_alloc = 0;
  conf->iov_min_offset = 0;

  conf->buf_sz = 64 - (sizeof(Vstr_node_buf) + 8);

  conf->fmt_usr_names = NULL;
  conf->fmt_usr_escape = 0;
  {
    unsigned int tmp = 0;
    while (tmp < 37)
      conf->fmt_usr_name_hash[tmp++] = NULL;
  }
  
  conf->vstr__fmt_spec_make = NULL;
  conf->vstr__fmt_spec_list_beg = NULL;
  conf->vstr__fmt_spec_list_end = NULL;

  conf->ref = 1;

  conf->ref_link = NULL;

  conf->ref_grp_ptr     = NULL;
  conf->ref_grp_buf2ref = NULL;

  conf->free_do = TRUE;
  conf->malloc_bad = FALSE;
  conf->iovec_auto_update = TRUE;
  conf->split_buf_del = FALSE;

  conf->user_ref = 1;

  conf->no_cache = FALSE;
  conf->fmt_usr_curly_braces = TRUE;
  conf->atomic_ops = TRUE;

  conf->grpalloc_cache = VSTR_TYPE_CNTL_CONF_GRPALLOC_POS;
  
  conf->spare_base_num = 0;
  conf->spare_base_beg = NULL;
  
  return (conf);

 fail_numeric:
  VSTR__F(conf->loc->num_beg);
 fail_loc_num:
  VSTR__F(conf->loc);
 fail_loc:
  VSTR__F(conf->data_usr_ents);
 fail_data:
  VSTR__F(conf->cache_cbs_ents);
 fail_cache:
  VSTR__F(conf);
 fail_conf:

  return (NULL);
}

static void vstr__add_conf(Vstr_conf *conf)
{
  ++conf->ref;
}

void vstr__add_user_conf(Vstr_conf *conf)
{
  assert(conf);
  assert(conf->user_ref <= conf->ref);

  ++conf->user_ref;

  vstr__add_conf(conf);
}

/* NOTE: magic also exists in vstr_add..c (vstr__convert_buf_ref_add)
 * for linked references */

void vstr__add_base_conf(Vstr_base *base, Vstr_conf *conf)
{
  assert(conf->user_ref <= conf->ref);

  base->conf = conf;
  vstr__add_conf(conf);
}

void vstr__del_grpalloc(Vstr_conf *conf, unsigned int num)
{
  Vstr_base *scan = conf->spare_base_beg;
  
  ASSERT(vstr__check_spare_base(conf));
  
  while (scan && num)
  { /* FIXME: ... BIG HACK */
    conf->spare_base_beg = (Vstr_base *)scan->beg;

    if (scan->cache_available)
    {
      if (VSTR__CACHE(scan)->vec)
      {
        VSTR__F(VSTR__CACHE(scan)->vec->v);
        VSTR__F(VSTR__CACHE(scan)->vec->t);          
      }
      VSTR__F(VSTR__CACHE(scan));
    }
    VSTR__F(scan);

    --conf->spare_base_num;
    --num;
    
    scan = conf->spare_base_beg;
  }

  ASSERT(vstr__check_spare_base(conf));
}

void vstr__del_conf(Vstr_conf *conf)
{
  assert(conf->ref > 0);

  if (!--conf->ref)
  {
    ASSERT(!conf->ref_link);

    vstr__ref_grp_free(conf->ref_grp_ptr);
    vstr__ref_grp_free(conf->ref_grp_buf2ref);
    
    vstr_free_spare_nodes(conf, VSTR_TYPE_NODE_BUF, conf->spare_buf_num);
    vstr_free_spare_nodes(conf, VSTR_TYPE_NODE_NON, conf->spare_non_num);
    vstr_free_spare_nodes(conf, VSTR_TYPE_NODE_PTR, conf->spare_ptr_num);
    vstr_free_spare_nodes(conf, VSTR_TYPE_NODE_REF, conf->spare_ref_num);

    MALLOC_CHECK_MEM(conf->loc->name_lc_numeric_ref);
    vstr_ref_del(conf->loc->name_lc_numeric_ref);
    
    while (conf->loc->num_beg)
    {
      Vstr_locale_num_base *next = conf->loc->num_beg->next;
      
      MALLOC_CHECK_MEM(conf->loc->num_beg);
      
      vstr_ref_del(conf->loc->num_beg->grouping);
      vstr_ref_del(conf->loc->num_beg->thousands_sep_ref);
      vstr_ref_del(conf->loc->num_beg->decimal_point_ref);
      
      VSTR__F(conf->loc->num_beg);
      conf->loc->num_beg = next;
    }

    vstr_ref_del(conf->loc->null_ref);

    VSTR__F(conf->loc);

    vstr__data_conf_free(conf);
    
    VSTR__F(conf->cache_cbs_ents);

    vstr__add_fmt_free_conf(conf);

    vstr__del_grpalloc(conf, conf->spare_base_num);
    
    if (conf->free_do)
      VSTR__F(conf);
  }
}

int vstr_swap_conf(Vstr_base *base, Vstr_conf **conf)
{
  int can_change_conf = FALSE;
  
  assert(conf && *conf);
  assert((*conf)->user_ref > 0);
  assert((*conf)->user_ref <= (*conf)->ref);

  if (base->conf == *conf)
    return (TRUE);

  /* there are base string object using this configuration,
   * so we can't change it */
  if ((*conf)->user_ref == (*conf)->ref)
    can_change_conf = TRUE;
  
  if (base->conf->buf_sz != (*conf)->buf_sz)
  {
    if (!can_change_conf)
      return (FALSE);

    vstr_free_spare_nodes(*conf, VSTR_TYPE_NODE_BUF, (*conf)->spare_buf_num);
    (*conf)->buf_sz = base->conf->buf_sz;
  }

  if (!vstr__cache_subset_cbs(base->conf, *conf))
  {
    if (!can_change_conf)
      return (FALSE);

    if (!vstr__cache_dup_cbs(*conf, base->conf))
      return (FALSE);
  }

  --(*conf)->user_ref;
  ++base->conf->user_ref;

  SWAP_TYPE(*conf, base->conf, Vstr_conf *);

  return (TRUE);
}

void vstr_free_conf(Vstr_conf *conf)
{
  if (!conf)
    return;

  assert(conf->user_ref > 0);
  assert(conf->user_ref <= conf->ref);

  --conf->user_ref;

  vstr__del_conf(conf);
}

int vstr_init(void)
{
  if (!vstr__options.def)
  {
    vstr__options.mmap_count = 0;
    
    vstr__options.fd_count = 0;

    MALLOC_CHECK_REINIT();
    
    if (!(vstr__options.def = vstr_make_conf()))
      return (FALSE);
  }

  return (TRUE);
}

void vstr_exit(void)
{
  ASSERT((vstr__options.def->user_ref == 1) &&
         (vstr__options.def->ref == 1));

  vstr_free_conf(vstr__options.def);
  vstr__options.def = NULL;

  ASSERT(!vstr__options.mmap_count);
  ASSERT(!vstr__options.fd_count);
  
  MALLOC_CHECK_EMPTY();
}

int vstr__cache_iovec_valid(Vstr_base *base)
{
  unsigned int count = 0;
  Vstr_node *scan = NULL;

  if (base->iovec_upto_date)
    return (TRUE);

  if (!base->beg)
  {
    if (base->cache_available && VSTR__CACHE(base) && VSTR__CACHE(base)->vec &&
        VSTR__CACHE(base)->vec->sz)
      base->iovec_upto_date = TRUE;

    return (TRUE);
  }

  if (!vstr__cache_iovec_alloc(base, base->num))
    return (FALSE);

  assert(VSTR__CACHE(base)->vec->off == base->conf->iov_min_offset);

  count = base->conf->iov_min_offset;
  scan = base->beg;

  VSTR__CACHE(base)->vec->v[count].iov_len = scan->len - base->used;
  if (scan->type == VSTR_TYPE_NODE_NON)
    VSTR__CACHE(base)->vec->v[count].iov_base = NULL;
  else
    VSTR__CACHE(base)->vec->v[count].iov_base = (vstr_export__node_ptr(scan) +
                                                 base->used);
  VSTR__CACHE(base)->vec->t[count] = scan->type;

  scan = scan->next;
  ++count;
  while (scan)
  {
    VSTR__CACHE(base)->vec->t[count] = scan->type;
    VSTR__CACHE(base)->vec->v[count].iov_len = scan->len;
    VSTR__CACHE(base)->vec->v[count].iov_base = vstr_export__node_ptr(scan);

    ++count;
    scan = scan->next;
  }

  base->iovec_upto_date = TRUE;

  return (TRUE);
}

static void vstr__init_base(Vstr_conf *conf, Vstr_base *base)
{
  base->beg = NULL;
  base->end = NULL;

  base->len = 0;
  base->num = 0;

  vstr__add_base_conf(base, conf);

  base->used = 0;
  base->free_do = FALSE;
  base->iovec_upto_date = FALSE;

  base->node_buf_used = FALSE;
  base->node_non_used = FALSE;
  base->node_ptr_used = FALSE;
  base->node_ref_used = FALSE;

  if (base->cache_available &&
      (base->grpalloc_cache >= VSTR_TYPE_CNTL_CONF_GRPALLOC_IOVEC))
  {
    VSTR__CACHE(base)->vec->off = base->conf->iov_min_offset;
    base->iovec_upto_date = TRUE;
  }
}

void vstr_free_base(Vstr_base *base)
{
  Vstr_conf *conf = NULL;

  if (!base)
    return;

  conf = base->conf;

  ASSERT(vstr__check_spare_base(conf));
  
  vstr__free_cache(base);

  vstr_del(base, 1, base->len);

  if (base->free_do)
  {
    if (base->grpalloc_cache == base->conf->grpalloc_cache)
    { /* FIXME: massive hack */
      base->beg = (Vstr_node *)base->conf->spare_base_beg;
      base->conf->spare_base_beg = base;
      ++base->conf->spare_base_num;
    }
    else
    {
      if (base->cache_available)
      {
        if (VSTR__CACHE(base)->vec)
        {
          VSTR__F(VSTR__CACHE(base)->vec->v);
          VSTR__F(VSTR__CACHE(base)->vec->t);          
        }
        VSTR__F(VSTR__CACHE(base));
      }
      VSTR__F(base);
    }
  }
  
  ASSERT(vstr__check_spare_base(conf));
  
  vstr__del_conf(conf);
}

static Vstr_base *vstr__make_base_cache(Vstr_conf *conf)
{
  Vstr_base *base = NULL;
  size_t sz = 0;
  unsigned int num = 0;
  Vstr__cache *cache = NULL;
  
  switch (conf->grpalloc_cache)
  {
    case VSTR_TYPE_CNTL_CONF_GRPALLOC_NONE:
      num = 0; sz = sizeof(struct Vstr_base);            break;
    case VSTR_TYPE_CNTL_CONF_GRPALLOC_POS:
      num = 1; sz = sizeof(struct Vstr__base_p_cache);   break;
    case VSTR_TYPE_CNTL_CONF_GRPALLOC_IOVEC:
      num = 2; sz = sizeof(struct Vstr__base_pi_cache);  break;
    case VSTR_TYPE_CNTL_CONF_GRPALLOC_CSTR:
      num = 3; sz = sizeof(struct Vstr__base_pic_cache);
      
      ASSERT_NO_SWITCH_DEF();
  }
    
  if (!(base = VSTR__MK(sz)))
    goto malloc_fail_base;
  
  base->grpalloc_cache = conf->grpalloc_cache;
  
  base->cache_internal = TRUE;
  
  /* allocate cache pointers */
  if (!num)
  {
    base->cache_available = FALSE;
    ASSERT(conf->no_cache);
    return (base);
  }

  if (!(cache = VSTR__MK(sizeof(Vstr__cache) + (sizeof(void *) * num))))
    goto malloc_fail_cache;
  
  cache->sz  = num;
  cache->vec = NULL;
  
  vstr_wrap_memset(cache->data, 0, sizeof(void *) * num);
  
  VSTR__CACHE(base)     = cache;
  base->cache_available = TRUE;
  
  /* NOTE: hand init/allocate empty data for each cache type,
   * this should really be in vstr_cache.c ... */
  switch (conf->grpalloc_cache)
  {
    case VSTR_TYPE_CNTL_CONF_GRPALLOC_CSTR:
    {
      struct Vstr__base_pic_cache *ubase = (struct Vstr__base_pic_cache *)base;
      Vstr__cache_data_cstr *data = &ubase->real_cstr;
    
      data->ref = NULL;
      vstr_cache_set(base, 3, data);
      ASSERT(vstr_cache_get(base, 3) == data);
    }
    ASSERT(cache->sz >= 3);
    /* FALLTHOUGH */

    case VSTR_TYPE_CNTL_CONF_GRPALLOC_IOVEC:
    {
      struct Vstr__base_pi_cache *ubase = (struct Vstr__base_pi_cache *)base;
      Vstr__cache_data_iovec *data = &ubase->real_iov;
      size_t alloc_sz = 1 + conf->iov_min_alloc + conf->iov_min_offset;
    
      VSTR__CACHE(base)->vec = data;
    
      data->v = VSTR__MK(sizeof(struct iovec) * alloc_sz);
      data->t = NULL;
      data->off = conf->iov_min_offset;
      data->sz = 0;
    
      if (!data->v)
        goto malloc_fail_vec_v;
    
      assert(!data->t);
      data->t = VSTR__MK(alloc_sz);
    
      if (!data->t)
        goto malloc_fail_vec_t;
    
      data->sz = alloc_sz;
      assert(!data->off);

      /* this is left NULL so that we don't call the cb */
      ASSERT(vstr_cache_get(base, 2) == NULL);
    }
    ASSERT(cache->sz >= 2);
    /* FALLTHOUGH */
    
    case VSTR_TYPE_CNTL_CONF_GRPALLOC_POS:
    {
      struct Vstr__base_p_cache *ubase = (struct Vstr__base_p_cache *)base;
      Vstr__cache_data_pos *data = &ubase->real_pos;
    
      data->node = NULL;
    
      vstr_cache_set(base, 1, data);
      ASSERT(vstr_cache_get(base, 1) == data);
    }
    ASSERT(cache->sz >= 1);
    ASSERT_NO_SWITCH_DEF();
  }

  return (base);
  
 malloc_fail_vec_t:
  VSTR__F(VSTR__CACHE(base)->vec->v);
 malloc_fail_vec_v:
  VSTR__F(VSTR__CACHE(base));
 malloc_fail_cache:
  VSTR__F(base);
 malloc_fail_base:
  return (NULL);
}

Vstr_base *vstr_make_base(Vstr_conf *conf)
{
  Vstr_base *base = NULL;

  if (!conf)
    conf = vstr__options.def;

  ASSERT(vstr__check_spare_nodes(conf));
  ASSERT(vstr__check_spare_base(conf));
  
  if (conf->spare_base_num)
  {
    Vstr__cache *cache = NULL;
  
    base = conf->spare_base_beg;
    conf->spare_base_beg = (Vstr_base *)base->beg;
    --conf->spare_base_num;

    ASSERT(base->grpalloc_cache == conf->grpalloc_cache);
    
    ASSERT(base->cache_internal);

    cache = VSTR__CACHE(base);
    if (base->grpalloc_cache != VSTR_TYPE_CNTL_CONF_GRPALLOC_NONE)
      MALLOC_CHECK_MEM(cache);
    
    switch (conf->grpalloc_cache)
    {
      case VSTR_TYPE_CNTL_CONF_GRPALLOC_CSTR:
      {
        struct Vstr__base_pic_cache *ubase = (struct Vstr__base_pic_cache *)base;
        Vstr__cache_data_cstr *data = &ubase->real_cstr;
        
        data->ref = NULL;
        vstr_cache_set(base, 3, data);
        ASSERT(vstr_cache_get(base, 3) == data);
      }
      ASSERT(cache->sz >= 3);
      /* FALLTHOUGH */
      
      case VSTR_TYPE_CNTL_CONF_GRPALLOC_IOVEC:
        /* this is left NULL so that we don't call the cb */
        ASSERT(vstr_cache_get(base, 2) == NULL);
        ASSERT(cache->sz >= 2);
        /* FALLTHOUGH */
        
      case VSTR_TYPE_CNTL_CONF_GRPALLOC_POS:
      {
        struct Vstr__base_p_cache *ubase = (struct Vstr__base_p_cache *)base;
        Vstr__cache_data_pos *data = &ubase->real_pos;
        
        data->node = NULL;
        
        vstr_cache_set(base, 1, data);
        ASSERT(vstr_cache_get(base, 1) == data);
      }
      ASSERT(cache->sz >= 1);
      /* FALLTHOUGH */
      
      case VSTR_TYPE_CNTL_CONF_GRPALLOC_NONE:
        
        ASSERT_NO_SWITCH_DEF();
    }
  }
  else if (!(base = vstr__make_base_cache(conf)))
    goto malloc_fail_base;

  vstr__init_base(conf, base);

  base->free_do = TRUE;

  ASSERT(base->conf == conf);
  ASSERT(vstr__check_spare_nodes(conf));
  ASSERT(vstr__check_real_nodes(base));
  ASSERT(vstr__check_spare_base(conf));

  return (base);

 malloc_fail_base:
  conf->malloc_bad = TRUE;
  return (NULL);
}

Vstr_node *vstr__base_split_node(Vstr_base *base, Vstr_node *node, size_t pos)
{
  Vstr_node *beg = NULL;

  assert(base && pos && (pos <= node->len));

  switch (node->type)
  {
    case VSTR_TYPE_NODE_BUF:
      if (!vstr_cntl_conf(base->conf,
                          VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_BUF, 1, UINT_MAX))
        return (NULL);

      --base->conf->spare_buf_num;
      beg = (Vstr_node *)base->conf->spare_buf_beg;
      base->conf->spare_buf_beg = (Vstr_node_buf *)beg->next;

      vstr_wrap_memcpy(((Vstr_node_buf *)beg)->buf,
                       ((Vstr_node_buf *)node)->buf + pos, node->len - pos);

      /* poison the split */
      ASSERT(vstr_wrap_memset(((Vstr_node_buf *)node)->buf + pos, 0xFE,
                              node->len - pos));
      break;

    case VSTR_TYPE_NODE_NON:
      if (!vstr_cntl_conf(base->conf,
                          VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_NON, 1, UINT_MAX))
        return (NULL);

      --base->conf->spare_non_num;
      beg = (Vstr_node *)base->conf->spare_non_beg;
      base->conf->spare_non_beg = (Vstr_node_non *)beg->next;
      break;

    case VSTR_TYPE_NODE_PTR:
      if (!vstr_cntl_conf(base->conf,
                          VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_PTR, 1, UINT_MAX))
        return (NULL);

      --base->conf->spare_ptr_num;
      beg = (Vstr_node *)base->conf->spare_ptr_beg;
      base->conf->spare_ptr_beg = (Vstr_node_ptr *)beg->next;

      ((Vstr_node_ptr *)beg)->ptr = ((char *)((Vstr_node_ptr *)node)->ptr) + pos;
      break;

    case VSTR_TYPE_NODE_REF:
    {
      Vstr_ref *ref = NULL;

      if (!vstr_cntl_conf(base->conf,
                          VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_REF, 1, UINT_MAX))
        return (NULL);

      --base->conf->spare_ref_num;
      beg = (Vstr_node *)base->conf->spare_ref_beg;
      base->conf->spare_ref_beg = (Vstr_node_ref *)beg->next;

      ref = ((Vstr_node_ref *)node)->ref;
      ((Vstr_node_ref *)beg)->ref = vstr_ref_add(ref);
      ((Vstr_node_ref *)beg)->off = ((Vstr_node_ref *)node)->off + pos;
    }

    ASSERT_NO_SWITCH_DEF();
  }

  ++base->num;

  base->iovec_upto_date = FALSE;

  beg->len = node->len - pos;

  if (!(beg->next = node->next))
    base->end = beg;
  node->next = beg;

  node->len = pos;

  return (node);
}

static int vstr__make_spare_node(Vstr_conf *conf, unsigned int type)
{
  Vstr_node *node = NULL;

  switch (type)
  {
    case VSTR_TYPE_NODE_BUF:
      node = VSTR__MK(sizeof(Vstr_node_buf) + conf->buf_sz);
      break;
    case VSTR_TYPE_NODE_NON:
      node = VSTR__MK(sizeof(Vstr_node_non));
      break;
    case VSTR_TYPE_NODE_PTR:
      node = VSTR__MK(sizeof(Vstr_node_ptr));
      break;
    case VSTR_TYPE_NODE_REF:
      node = VSTR__MK(sizeof(Vstr_node_ref));
      break;

    default:
      ASSERT_RET(FALSE, FALSE);
  }

  if (!node)
  {
    conf->malloc_bad = TRUE;
    return (FALSE);
  }

  node->len = 0;
  node->type = type;

  switch (type)
  {
    case VSTR_TYPE_NODE_BUF:
      node->next = (Vstr_node *)conf->spare_buf_beg;
      conf->spare_buf_beg = (Vstr_node_buf *)node;
      ++conf->spare_buf_num;
      break;

    case VSTR_TYPE_NODE_NON:
      node->next = (Vstr_node *)conf->spare_non_beg;
      conf->spare_non_beg = (Vstr_node_non *)node;
      ++conf->spare_non_num;
      break;

    case VSTR_TYPE_NODE_PTR:
      node->next = (Vstr_node *)conf->spare_ptr_beg;
      conf->spare_ptr_beg = (Vstr_node_ptr *)node;
      ++conf->spare_ptr_num;
      break;

    case VSTR_TYPE_NODE_REF:
      node->next = (Vstr_node *)conf->spare_ref_beg;
      conf->spare_ref_beg = (Vstr_node_ref *)node;
      ++conf->spare_ref_num;
      break;
  }

  return (TRUE);
}

unsigned int vstr_make_spare_nodes(Vstr_conf *passed_conf, unsigned int type,
                                   unsigned int num)
{
  Vstr_conf *conf = passed_conf ? passed_conf : vstr__options.def;
  unsigned int count = 0;

  assert(vstr__check_spare_nodes(conf));

  while (count < num)
  {
    if (!vstr__make_spare_node(conf, type))
    {
      assert(vstr__check_spare_nodes(conf));

      return (count);
    }

    ++count;
  }
  assert(count == num);

  assert(vstr__check_spare_nodes(conf));

  return (count);
}

static int vstr__free_spare_node(Vstr_conf *conf, unsigned int type)
{
  Vstr_node *scan = NULL;

  switch (type)
  {
    case VSTR_TYPE_NODE_BUF:
      if (!conf->spare_buf_beg)
        return (FALSE);

      scan = (Vstr_node *)conf->spare_buf_beg;

      assert(scan->type == type);

      conf->spare_buf_beg = (Vstr_node_buf *)scan->next;
      --conf->spare_buf_num;
      VSTR__F(scan);
      break;

    case VSTR_TYPE_NODE_NON:
      if (!conf->spare_non_beg)
        return (FALSE);

      scan = (Vstr_node *)conf->spare_non_beg;

      assert(scan->type == VSTR_TYPE_NODE_NON);

      conf->spare_non_beg = (Vstr_node_non *)scan->next;
      --conf->spare_non_num;
      VSTR__F(scan);
      break;

    case VSTR_TYPE_NODE_PTR:
      if (!conf->spare_ptr_beg)
        return (FALSE);

      scan = (Vstr_node *)conf->spare_ptr_beg;

      assert(scan->type == VSTR_TYPE_NODE_PTR);

      conf->spare_ptr_beg = (Vstr_node_ptr *)scan->next;
      --conf->spare_ptr_num;
      VSTR__F(scan);
      break;

    case VSTR_TYPE_NODE_REF:
      if (!conf->spare_ref_beg)
        return (FALSE);

      scan = (Vstr_node *)conf->spare_ref_beg;

      assert(scan->type == type);

      conf->spare_ref_beg = (Vstr_node_ref *)scan->next;
      --conf->spare_ref_num;
      VSTR__F(scan);

      ASSERT_NO_SWITCH_DEF();
  }

  return (TRUE);
}

unsigned int vstr_free_spare_nodes(Vstr_conf *passed_conf, unsigned int type,
                                   unsigned int num)
{
  Vstr_conf *conf = passed_conf ? passed_conf : vstr__options.def;
  unsigned int count = 0;

  assert(vstr__check_spare_nodes(conf));

  while (count < num)
  {
    if (!vstr__free_spare_node(conf, type))
      return (count);

    ++count;
  }
  assert(count == num);

  assert(vstr__check_spare_nodes(conf));

  return (count);
}

/* vstr_base__pos() but returns a (Vstr_node **) instead of (Vstr_node *) */
Vstr_node **vstr__base_ptr_pos(const Vstr_base *base, size_t *pos,
                               unsigned int *num)
{
  Vstr_node *const *scan = &base->beg;

  ASSERT(base && pos && num);
  
  *pos += base->used;
  *num = 1;

  while (*pos > (*scan)->len)
  {
    *pos -= (*scan)->len;

    ASSERT((*scan)->next);
    scan = &(*scan)->next;
    ++*num;
  }

  return ((Vstr_node **) scan);
}

struct Vstr__conf_ref_linked
{
 Vstr_conf *conf;
 unsigned int l_ref;
};

/* put node on reference list */
static int vstr__convert_buf_ref_add(Vstr_conf *conf, Vstr_node *node)
{
  struct Vstr__conf_ref_linked *ln_ref;

  if (!(ln_ref = conf->ref_link))
  {
    if (!(ln_ref = VSTR__MK(sizeof(struct Vstr__conf_ref_linked))))
      return (FALSE);

    ln_ref->conf = conf;
    ln_ref->l_ref = 0;

    conf->ref_link = ln_ref;
    ++conf->ref;
  }
  ASSERT(ln_ref->l_ref < VSTR__CONF_REF_LINKED_SZ);

  ++ln_ref->l_ref;
  /* this cast isn't ISO 9899:1999 compliant ... but fuck it */
  node->next = (Vstr_node *)ln_ref;

  if (ln_ref->l_ref >= VSTR__CONF_REF_LINKED_SZ)
    conf->ref_link = NULL;

  return (TRUE);
}

/* call back ... relink */
static void vstr__ref_cb_relink_bufnode_ref(Vstr_ref *ref)
{
  ASSERT(ref);
  
  if (ref->ptr)
  {
    char *tmp = ref->ptr;
    Vstr_node_buf *node = NULL;
    struct Vstr__conf_ref_linked *ln_ref = NULL;
    Vstr_conf *conf = NULL;

    tmp -= offsetof(Vstr_node_buf, buf);
    node = (Vstr_node_buf *)tmp;
    ln_ref = (struct Vstr__conf_ref_linked *)(node->s.next);

    conf = ln_ref->conf;

    /* manual relink ... Also ISO 9899:1999 violation */
    node->s.next = (Vstr_node *)ln_ref->conf->spare_buf_beg;
    ln_ref->conf->spare_buf_beg = node;
    ++ln_ref->conf->spare_buf_num;

    if (!--ln_ref->l_ref)
    {
      if (ln_ref->conf->ref_link == ln_ref)
        ln_ref->conf->ref_link = NULL;

      VSTR__F(ln_ref);
      vstr__del_conf(conf);
    }
  }
}

void vstr__swap_node_X_X(const Vstr_base *base, size_t pos,
                         Vstr_node *node)
{
  Vstr_node *old_scan = NULL;
  size_t old_scan_len = 0;
  Vstr_node **scan = NULL;
  unsigned int num = 0;
  Vstr__cache_data_pos *data = NULL;

  scan = vstr__base_ptr_pos(base, &pos, &num);

  old_scan = *scan;
  old_scan_len = old_scan->len;

  node->next = (*scan)->next;
  vstr__relink_nodes(base->conf, *scan, &(*scan)->next, 1);
  *scan = node;

  if (!node->next)
    ((Vstr_base *)base)->end = node;

  if (node == base->beg)
    ((Vstr_base *)base)->used = 0;

  if ((data = vstr_cache_get(base, base->conf->cache_pos_cb_pos)) &&
      (data->node == old_scan))
  {
    data->node = NULL;

    if (node->len >= old_scan_len)
      data->node = node;
  }

  switch (node->type)
  {
    /* -- note, should work, but not called like so currently --
      case VSTR_TYPE_NODE_BUF: ((Vstr_base *)base)->node_buf_used = TRUE; break;
      case VSTR_TYPE_NODE_NON: ((Vstr_base *)base)->node_non_used = TRUE; break;
    */
    case VSTR_TYPE_NODE_PTR: ((Vstr_base *)base)->node_ptr_used = TRUE; break;
    case VSTR_TYPE_NODE_REF: ((Vstr_base *)base)->node_ref_used = TRUE;
      ASSERT_NO_SWITCH_DEF();
  }

  vstr__cache_iovec_reset_node(base, node, num);
}


int vstr__chg_node_buf_ref(const Vstr_base *base,
                           Vstr_node **scan, unsigned int num)
{
  Vstr__cache_data_pos *data = NULL;
  Vstr_node *next_node = (*scan)->next; /* must be done now... */
  Vstr_ref *ref = NULL;
  Vstr_node_ref *node_ref = NULL;

  assert((*scan)->type == VSTR_TYPE_NODE_BUF);

  if (!vstr_cntl_conf(base->conf, VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_REF,
                      1, UINT_MAX))
    goto fail_malloc_nodes;

  if (!base->conf->ref_grp_buf2ref)
  {
    Vstr_ref_grp_ptr *tmp = NULL;

    if (!(tmp = vstr__ref_grp_make(vstr__ref_cb_relink_bufnode_ref, 0)))
      goto fail_malloc_ref;
    base->conf->ref_grp_buf2ref = tmp;
  }

  if (!(ref = vstr__ref_grp_add(&base->conf->ref_grp_buf2ref,
                                ((Vstr_node_buf *)(*scan))->buf)))
    goto fail_malloc_ref;
  if (!vstr__convert_buf_ref_add(base->conf, *scan))
    goto fail_malloc_conv_buf;

  --base->conf->spare_ref_num;
  node_ref = base->conf->spare_ref_beg;
  base->conf->spare_ref_beg = (Vstr_node_ref *)node_ref->s.next;

  ((Vstr_base *)base)->node_ref_used = TRUE;

  node_ref->s.len = (*scan)->len;
  node_ref->ref = ref;
  node_ref->off = 0;
  if ((base->beg == *scan) && base->used)
  {
    node_ref->s.len -= base->used;
    node_ref->off = base->used;
    ((Vstr_base *)base)->used = 0;
  }

  if (!(node_ref->s.next = next_node))
    ((Vstr_base *)base)->end = &node_ref->s;

  /* NOTE: we have just changed the type of the node, must update the cache */
  if ((data = vstr_cache_get(base, base->conf->cache_pos_cb_pos)) &&
      (data->node == *scan))
    data->node = &node_ref->s;
  if (base->iovec_upto_date)
  {
    ASSERT(num);
    num += VSTR__CACHE(base)->vec->off - 1;
    assert(VSTR__CACHE(base)->vec->t[num] == VSTR_TYPE_NODE_BUF);
    VSTR__CACHE(base)->vec->t[num] = VSTR_TYPE_NODE_REF;
  }

  *scan = &node_ref->s;

  return (TRUE);

 fail_malloc_conv_buf:
  ref->ptr = NULL;
  vstr_ref_del(ref);
 fail_malloc_ref:
  base->conf->malloc_bad = TRUE;
 fail_malloc_nodes:
  return (FALSE);
}

#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(exit);
VSTR__SYM_ALIAS(free_base);
VSTR__SYM_ALIAS(free_conf);
VSTR__SYM_ALIAS(free_spare_nodes);
VSTR__SYM_ALIAS(init);
VSTR__SYM_ALIAS(make_base);
VSTR__SYM_ALIAS(make_conf);
VSTR__SYM_ALIAS(make_spare_nodes);
VSTR__SYM_ALIAS(swap_conf);
