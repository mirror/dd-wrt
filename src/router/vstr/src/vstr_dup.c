#define VSTR_DUP_C
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
/* These are similar to strdup(), for Vstr ... with 1.0.10 it might be
 * useful to create temp. strings with them. */
#include "main.h"

Vstr_base *vstr_dup_buf(Vstr_conf *conf, const void *data, size_t len)
{
  Vstr_base *ret = vstr_make_base(conf);

  if (!ret)
    goto make_base_fail;

  if (len && !vstr_add_buf(ret, 0, data, len))
    goto add_vstr_fail;

  return (ret);

 add_vstr_fail:
  vstr_free_base(ret);
 make_base_fail:

  return (NULL);
}

Vstr_base *vstr_dup_non(Vstr_conf *conf, size_t len)
{
  Vstr_base *ret = vstr_make_base(conf);

  if (!ret)
    goto make_base_fail;

  if (len && !vstr_add_non(ret, 0, len))
    goto add_vstr_fail;

  return (ret);

 add_vstr_fail:
  vstr_free_base(ret);
 make_base_fail:

  return (NULL);
}

Vstr_base *vstr_dup_ptr(Vstr_conf *conf, const void *data, size_t len)
{
  Vstr_base *ret = vstr_make_base(conf);

  if (!ret)
    goto make_base_fail;

  if (len && !vstr_add_ptr(ret, 0, data, len))
    goto add_vstr_fail;

  return (ret);

 add_vstr_fail:
  vstr_free_base(ret);
 make_base_fail:

  return (NULL);
}

Vstr_base *vstr_dup_ref(Vstr_conf *conf,
                        Vstr_ref *ref, size_t off, size_t len)
{
  Vstr_base *ret = vstr_make_base(conf);

  if (!ret)
    goto make_base_fail;

  if (len && !vstr_add_ref(ret, 0, ref, off, len))
    goto add_vstr_fail;

  return (ret);

 add_vstr_fail:
  vstr_free_base(ret);
 make_base_fail:

  return (NULL);
}

Vstr_base *vstr_dup_vstr(Vstr_conf *conf,
                         const Vstr_base *base, size_t pos, size_t len,
                         unsigned int type)
{
  Vstr_base *ret = vstr_make_base(conf);

  if (!ret)
    goto make_base_fail;

  if (len && !vstr_add_vstr(ret, 0, base, pos, len, type))
    goto add_vstr_fail;

  return (ret);

 add_vstr_fail:
  vstr_free_base(ret);
 make_base_fail:
  base->conf->malloc_bad = TRUE;

  return (NULL);
}

Vstr_base *vstr_dup_rep_chr(Vstr_conf *conf, char chr, size_t len)
{
  Vstr_base *ret = vstr_make_base(conf);

  if (!ret)
    goto make_base_fail;

  if (len && !vstr_add_rep_chr(ret, 0, chr, len))
    goto add_vstr_fail;

  return (ret);

 add_vstr_fail:
  vstr_free_base(ret);
 make_base_fail:

  return (NULL);
}
#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(dup_buf);
VSTR__SYM_ALIAS(dup_non);
VSTR__SYM_ALIAS(dup_ptr);
VSTR__SYM_ALIAS(dup_ref);
VSTR__SYM_ALIAS(dup_rep_chr);
VSTR__SYM_ALIAS(dup_vstr);
