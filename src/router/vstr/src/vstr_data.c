#define VSTR_C
/*
 *  Copyright (C) 2004  James Antill
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
/* functions for assigning constant data from the "user" to a configuration */
#include "main.h"

int vstr__data_conf_init(Vstr_conf *conf)
{
  if (!(conf->data_usr_ents = VSTR__MK(sizeof(struct Vstr_data_usr))))
    return (FALSE);
  
  conf->data_usr_sz   = 1;
  conf->data_usr_len  = 0;

  return (TRUE);
}

void vstr__data_conf_free(Vstr_conf *conf)
{
  unsigned int pos = 0;

  while (pos < conf->data_usr_len)
  {
    if (conf->data_usr_ents[pos].name)
      vstr_ref_del(conf->data_usr_ents[pos].data);
    
    ++pos;
  }

  VSTR__F(conf->data_usr_ents);
}

/* add / srch / del / get / set */
unsigned int vstr_data_srch(Vstr_conf *passed_conf, const char *name)
{
  Vstr_conf *conf = passed_conf ? passed_conf : vstr__options.def;
  unsigned int pos = 0;

  ASSERT_RET(name, 0);
  ASSERT(conf->data_usr_len <= conf->data_usr_sz);

  while (pos < conf->data_usr_len)
  {
    if (conf->data_usr_ents[pos].name &&
        !strcmp(name, conf->data_usr_ents[pos++].name))
      return (pos);
  }

  return (0);
}

unsigned int vstr_data_add(Vstr_conf *passed_conf,
                           const char *name, Vstr_ref *data)
{
  Vstr_conf *conf = passed_conf ? passed_conf : vstr__options.def;
  unsigned int len = conf->data_usr_len;
  unsigned int sz  = conf->data_usr_sz;
  struct Vstr_data_usr *ents = NULL;

  ASSERT_RET(name, 0);
  
  ASSERT(!vstr_data_srch(conf, name));
  
  ASSERT(len <= sz);

  if (len == sz)
    len = 0; /* look for deleted enteries */
  
  while ((len < conf->data_usr_len) && conf->data_usr_ents[len].name)
    ++len;
  
  if (len == sz)
  {
    sz *= 2;
  
    if (!VSTR__MV(conf->data_usr_ents, ents, sizeof(struct Vstr_data_usr) * sz))
    {
      conf->malloc_bad = TRUE;
      return (0);
    }
    conf->data_usr_sz = sz;
  }
  
  conf->data_usr_ents[len].name = name;
  conf->data_usr_ents[len].data = data ? vstr_ref_add(data) : NULL;

  ++len;
  if (conf->data_usr_len < len)
    conf->data_usr_len = len;
  
  ASSERT(vstr_data_srch(conf, name));
  
  return (len);
}

void vstr_data_del(Vstr_conf *passed_conf, unsigned int pos)
{
  Vstr_conf *conf = passed_conf ? passed_conf : vstr__options.def;
  
  ASSERT_RET_VOID(pos && (pos <= conf->data_usr_len));
  
  vstr_ref_del(conf->data_usr_ents[pos - 1].data);
  
  conf->data_usr_ents[pos - 1].name = NULL;
  conf->data_usr_ents[pos - 1].data = NULL;
  
  if (pos == conf->data_usr_len)
  {
    while (pos && !conf->data_usr_ents[pos - 1].name)
    { --pos; }
    
    conf->data_usr_len = pos;
  }
}

void *vstr_extern_inline_data_get(unsigned int pos)
{ /* global isn't exported */
  Vstr_conf *conf = vstr__options.def;
  return (vstr_data_get(conf, pos));
}

void  vstr_extern_inline_data_set(unsigned int pos, Vstr_ref *ref)
{ /* global isn't exported */
  Vstr_conf *conf = vstr__options.def;
  return (vstr_data_set(conf, pos, ref));
}
#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(data_add);
VSTR__SYM_ALIAS(data_del);
VSTR__SYM_ALIAS(data_srch);
