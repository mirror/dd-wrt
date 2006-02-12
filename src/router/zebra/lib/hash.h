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

#ifndef _ZEBRA_HASH_H
#define _ZEBRA_HASH_H

/* for Hash tables */ 
#define HASHTABSIZE     2048

typedef struct HashBacket
{
  void *data;
  struct HashBacket *next;
} HashBacket;

struct Hash
{
  /* Hash backet. */
  HashBacket **index;

  /* Hash size. */
  int size;

  /* Key make function. */
  unsigned int (*hash_key)();

  /* Data compare function. */
  int (*hash_cmp)();

  /* Backet alloc. */
  unsigned long count;
};

struct Hash *hash_new (int size);
HashBacket *hash_head (struct Hash *, int);
HashBacket *hash_push (struct Hash *, void *);
void *hash_pull (struct Hash *, void *);
void *hash_search (struct Hash *, void *);
void hash_clean (struct Hash *hash, void (* func) (void *));
void hash_free (struct Hash *hash);

#endif /* _ZEBRA_HASH_H */
