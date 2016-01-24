#define VSTR_REF_C
/*
 *  Copyright (C) 1999, 2000, 2001, 2002, 2003  James Antill
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
/* contains all the base functions related to vstr_ref objects
 * including callbacks */
#include "main.h"

void vstr_ref_cb_free_nothing(Vstr_ref *VSTR__ATTR_UNUSED(ref))
{
}

void vstr_ref_cb_free_ref(Vstr_ref *ref)
{
  free(ref);
}

void vstr_ref_cb_free_ptr(Vstr_ref *ref)
{
  if (ref)
    free(ref->ptr);
}

void vstr_ref_cb_free_ptr_ref(Vstr_ref *ref)
{
  vstr_ref_cb_free_ptr(ref);
  vstr_ref_cb_free_ref(ref);
}

Vstr_ref *vstr_ref_make_ptr(const void *ptr, void (*func)(struct Vstr_ref *))
{
  Vstr_ref *ref = malloc(sizeof(Vstr_ref));

  if (!ref)
    return (NULL);

  ref->ref = 1;
  ref->ptr = (void *)ptr;
  ref->func = func;

  return (ref);
}

#ifndef NDEBUG
static void vstr__ref_cb_free_ref(Vstr_ref *ref)
{
  VSTR__F(ref);
}
#else
#define     vstr__ref_cb_free_ref vstr_ref_cb_free_ref
#endif

Vstr_ref *vstr_ref_make_malloc(size_t len)
{
  struct Vstr__buf_ref *ref = VSTR__MK(sizeof(Vstr__buf_ref) + len);

  if (!ref)
    return (NULL);

  ref->ref.ref = 1;
  ref->ref.ptr = ref->buf;
  ref->ref.func = vstr__ref_cb_free_ref;

  assert(&ref->ref == (Vstr_ref *)ref);

  return (&ref->ref);
}

Vstr_ref *vstr_ref_make_memdup(const void *ptr, size_t len)
{
  Vstr_ref *ref = vstr_ref_make_malloc(len);

  if (ref)
    vstr_wrap_memcpy(ref->ptr, ptr, len);

  return (ref);
}

static void vstr__ref_cb_free_vstr_base(Vstr_ref *ref)
{
  vstr_free_base(ref->ptr);
  free(ref);
}

Vstr_ref *vstr_ref_make_vstr_base(Vstr_base *base)
{
  return (vstr_ref_make_ptr(base, vstr__ref_cb_free_vstr_base));
}

static void vstr__ref_cb_free_vstr_conf(Vstr_ref *ref)
{
  vstr_free_conf(ref->ptr);
  free(ref);
}

Vstr_ref *vstr_ref_make_vstr_conf(Vstr_conf *conf)
{
  return (vstr_ref_make_ptr(conf, vstr__ref_cb_free_vstr_conf));
}

static void vstr__ref_cb_free_vstr_sects(Vstr_ref *ref)
{
  vstr_sects_free(ref->ptr);
  free(ref);
}

Vstr_ref *vstr_ref_make_vstr_sects(Vstr_sects *sects)
{
  return (vstr_ref_make_ptr(sects, vstr__ref_cb_free_vstr_sects));
}

static void vstr__ref_cb_free_grp_main(Vstr_ref_grp_ptr *parent,
                                       Vstr_ref *ref, unsigned int off)
{
  ASSERT(&parent->refs[off] == ref);
  
  parent->func(ref);

  ref->func = NULL;

  ASSERT(parent->free_num <  parent->make_num);
  ASSERT(parent->make_num <= VSTR__REF_GRP_MAKE_SZ);
  
  if (++parent->free_num == parent->make_num)
  {
    if (!(parent->flags & VSTR__FLAG_REF_GRP_REF))
      VSTR__F(parent);
    else
    {
      parent->free_num = 0;
      parent->make_num = 0;
    }
  }
  else
  {
    unsigned int scan = parent->make_num - 1;
  
    ASSERT(parent->free_num <  parent->make_num);
    
    while (!parent->refs[scan].func)
    {
      ASSERT(parent->free_num > 0);
      
      --parent->make_num;
      --parent->free_num;
      --scan;
    }
  }
}

#define VSTR__REF_GRP_CB(x)                                             \
    static void vstr__ref_cb_free_grp_ref_ ## x (Vstr_ref *ref)         \
    {                                                                   \
      unsigned int off = (x);                                           \
      Vstr_ref_grp_ptr *parent = NULL;                                  \
      char *ptr = (char *)(ref - off);                                  \
                                                                        \
      ptr -= offsetof(Vstr_ref_grp_ptr, refs);                          \
      parent = (Vstr_ref_grp_ptr *)ptr;                                 \
                                                                        \
      vstr__ref_cb_free_grp_main(parent, ref, off);                     \
    }

#include "vstr-ref_grp-data.h"

#undef VSTR__REF_GRP_CB

Vstr_ref_grp_ptr *vstr__ref_grp_make(void (*func) (Vstr_ref *),
                                     unsigned int flags)
{
  Vstr_ref_grp_ptr *parent = NULL;

  if (!(parent = VSTR__MK(sizeof(Vstr_ref_grp_ptr) +
                          (VSTR__REF_GRP_MAKE_SZ * sizeof(Vstr_ref)))))
    return (NULL);

  parent->make_num = 0;
  parent->free_num = 0;

  parent->func     = func;
  parent->flags    = flags | VSTR__FLAG_REF_GRP_REF;
  
  return (parent);
}

void vstr__ref_grp_free(Vstr_ref_grp_ptr *parent)
{
  if (!parent)
    return;

  if (!parent->make_num)
  {
    ASSERT(!parent->free_num);
    VSTR__F(parent);
    return;
  }
  
  parent->flags &= ~VSTR__FLAG_REF_GRP_REF;
  
  ASSERT(parent->make_num == VSTR__REF_GRP_MAKE_SZ);
  ASSERT(parent->free_num <  VSTR__REF_GRP_MAKE_SZ);
}

/* needs to be reset each time, so we can tell which are free */
#define VSTR__REF_GRP_CB(x)                                             \
    case (x): ref->func = vstr__ref_cb_free_grp_ref_ ## x; break;

Vstr_ref *vstr__ref_grp_add(Vstr_ref_grp_ptr **parent, const void *ptr)
{
  Vstr_ref *ref = NULL;
  
  ASSERT(parent && *parent);
  
  if ((*parent)->make_num == VSTR__REF_GRP_MAKE_SZ)
  {
    Vstr_ref_grp_ptr *tmp = NULL;
    
    if (!(tmp = vstr__ref_grp_make((*parent)->func, (*parent)->flags)))
      return (NULL);
    
    vstr__ref_grp_free(*parent);
    *parent = tmp;
  }
  
  ref = &(*parent)->refs[(*parent)->make_num];
  
  ref->ref = 1;
  ref->ptr = (void *)ptr; /* get rid of const */
  switch ((*parent)->make_num++)
  {
#include "vstr-ref_grp-data.h"
  }
  
  return (ref);
}
#undef VSTR__REF_GRP_CB

#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(ref_cb_free_nothing);
VSTR__SYM_ALIAS(ref_cb_free_ptr);
VSTR__SYM_ALIAS(ref_cb_free_ptr_ref);
VSTR__SYM_ALIAS(ref_cb_free_ref);
VSTR__SYM_ALIAS(ref_make_malloc);
VSTR__SYM_ALIAS(ref_make_memdup);
VSTR__SYM_ALIAS(ref_make_ptr);
VSTR__SYM_ALIAS(ref_make_vstr_base);
VSTR__SYM_ALIAS(ref_make_vstr_conf);
VSTR__SYM_ALIAS(ref_make_vstr_sects);
