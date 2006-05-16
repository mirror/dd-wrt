/*
 * Hash routine.
 * Copyright (C) 1998 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <zebra.h>

#include "hash.h"
#include "memory.h"
#include "log.h"

/* for hash */
HashBacket *hash[HASHTABSIZE];

/* Allocate new hash. */
struct Hash *
hash_new (int size)
{
  struct Hash *new;

  new = XMALLOC (MTYPE_HASH, sizeof (struct Hash));
  bzero (new, sizeof (struct Hash));

  new->index = XMALLOC (MTYPE_HASH, sizeof (HashBacket *) * size);
  bzero (new->index, sizeof (HashBacket *) * size);

  new->size = size;

  return new;
}

/* allocate new hash backet */
static HashBacket *
hash_backet_new (void *data)
{
  HashBacket *new;

  new = XMALLOC (MTYPE_HASH_BACKET, sizeof (struct HashBacket));
  new->data = data;
  new->next = NULL;

  return new;
}

HashBacket *
hash_head (struct Hash *hash, int index)
{
  return hash->index[index];
}

/* Hash search */
void *
hash_search (struct Hash *hash, void *data)
{
  unsigned int key;
  HashBacket *backet;

  key = (*hash->hash_key) (data);

  if (hash->index[key] == NULL)
    return NULL;

  for (backet = hash->index[key]; backet != NULL; backet = backet->next) 
    if ((*hash->hash_cmp) (backet->data, data) == 1)
      return backet->data;

  return NULL;
}

/* Push data into hash. */
HashBacket *
hash_push (struct Hash *hash, void *data)
{
  unsigned int key;
  HashBacket  *backet, *mp;

  key = (*hash->hash_key) (data);
  backet = hash_backet_new (data);

  hash->count++;

  if (hash->index[key] == NULL)
    hash->index[key] = backet;
  else
    {
      for (mp = hash->index[key]; mp->next != NULL; mp = mp->next) 
	if ((*hash->hash_cmp) (data, mp->data) == 1) 
	  {
	    zlog_info ("hash data [%p] was duplicated!", data);
	    XFREE (MTYPE_HASH_BACKET, backet);
	    return NULL;
	  }
      
      if ((*hash->hash_cmp) (data, mp->data) == 1) 
	{
	  zlog_info ("hash account name [%p] was duplicated!", data);
	  XFREE (MTYPE_HASH_BACKET, backet);
	  return NULL;
	}
      mp->next = backet;
    }
  return backet;
}

/* When deletion is finished successfully return data of delete
   backet. */
void *
hash_pull (struct Hash *hash, void *data)
{
  void *ret;
  unsigned int key;
  HashBacket *mp;
  HashBacket *mpp;

  key = (*hash->hash_key) (data);

  if(hash->index[key] == NULL) 
    return NULL;

  mp = mpp = hash->index[key];
  while (mp) 
    {
      if((*hash->hash_cmp) (mp->data, data) == 1) 
	{
	  if (mp == mpp) 
	      hash->index[key] = mp->next;
	  else 
	      mpp->next = mp->next;

	  ret = mp->data;
	  XFREE (MTYPE_HASH_BACKET, mp);
	  hash->count--;
	  return ret;
	}
      mpp = mp;
      mp = mp->next;
    }
  return NULL;
}

void
hash_clean (struct Hash *hash, void (* func) (void *))
{
  int i;
  HashBacket *mp;
  HashBacket *next;

  for (i = 0; i < HASHTABSIZE; i++)
    {
      for (mp = hash_head (hash, i); mp; mp = next)
	{
	  next = mp->next;
	      
	  if (func)
	    (*func) (mp->data);
	  XFREE (MTYPE_HASH_BACKET, mp);
	}
      hash->index[i] = NULL;
    }
}

void
hash_free (struct Hash *hash)
{
 hash_clean (hash, NULL);
 XFREE(MTYPE_HASH, hash->index);
 XFREE(MTYPE_HASH, hash);
}
