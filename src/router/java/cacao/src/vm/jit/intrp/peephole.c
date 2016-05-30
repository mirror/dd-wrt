/* Peephole optimization routines and tables

  Copyright (C) 2001,2002,2003 Free Software Foundation, Inc.

  This file is part of Gforth.

  Gforth is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
*/


#include "config.h"

#include <assert.h>
#include <stdlib.h>

#include "vm/jit/intrp/intrp.h"

#include "vm/options.hpp"


/* the numbers in this struct are primitive indices */
typedef struct Combination {
  int prefix;
  int lastprim;
  int combination_prim;
} Combination;

Combination peephole_table[] = {
#include <java-peephole.i>
  {-1,-1,-1} /* unnecessary; just to shut up lcc if the file is empty */
};

int use_super = 1; /* turned off by option -p */

typedef struct Peeptable_entry {
  struct Peeptable_entry *next;
  u4 prefix;
  u4 lastprim;
  u4 combination_prim;
} Peeptable_entry;

#define HASH_SIZE 1024
#define hash(_i1,_i2) (((((Cell)(_i1))+((Cell)(_i2))))&(HASH_SIZE-1))

Cell peeptable;

Cell prepare_peephole_table(Inst insts[])
{
  Cell i;
  Peeptable_entry **pt = (Peeptable_entry **)calloc(HASH_SIZE,sizeof(Peeptable_entry *));
  size_t static_supers = sizeof(peephole_table)/sizeof(peephole_table[0]);

  if (opt_static_supers < static_supers)
    static_supers = opt_static_supers;

  for (i=0; i<static_supers; i++) {
    Combination *c = &peephole_table[i];
    Peeptable_entry *p = (Peeptable_entry *)malloc(sizeof(Peeptable_entry));
    Cell h;
    p->prefix =           c->prefix;
    p->lastprim =         c->lastprim;
    p->combination_prim = c->combination_prim;
    h = hash(p->prefix,p->lastprim);
    p->next = pt[h];
    pt[h] = p;
  }
  return (Cell)pt;
}

void init_peeptable(void)
{
  peeptable = prepare_peephole_table(vm_prim);
}

ptrint peephole_opt(ptrint inst1, ptrint inst2, Cell peeptable)
{
  Peeptable_entry **pt = (Peeptable_entry **)peeptable;
  Peeptable_entry *p;

  if (use_super == 0)
      return -1;
  for (p = pt[hash(inst1,inst2)]; p != NULL; p = p->next)
    if (inst1 == p->prefix && inst2 == p->lastprim)
      return p->combination_prim;
  return -1;
}
