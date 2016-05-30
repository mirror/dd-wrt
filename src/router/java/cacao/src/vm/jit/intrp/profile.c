/* VM profiling support stuff

  Copyright (C) 2001,2003 Free Software Foundation, Inc.

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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "vm/jit/intrp/intrp.h"

/* data structure: simple hash table with external chaining */

#define HASH_SIZE (1<<20)
#define hash(p) ((((Cell)(p))/sizeof(Inst))&(HASH_SIZE-1))

#ifdef __GNUC__
typedef long long long_long;
#else
typedef long long_long;
#endif

struct block_count {
  struct block_count *next; /* next in hash table */
  struct block_count *fallthrough; /* the block that this one falls
                                       through to without SUPER_END */
  Inst *ip;
  long_long count;
  char **insts;
  size_t ninsts;
};

block_count *blocks[HASH_SIZE];

block_count *block_lookup(Inst *ip)
{
  block_count *b = blocks[hash(ip)];

  while (b!=NULL && b->ip!=ip)
    b = b->next;
  return b;
}

/* looks up present elements, inserts absent elements */
block_count *vm_block_insert(Inst *ip)
{ 
  block_count *b = block_lookup(ip);
  block_count *new;

  if (b != NULL)
    return b;
  new = (block_count *)malloc(sizeof(block_count));
  new->next = blocks[hash(ip)];
  new->fallthrough = NULL;
  new->ip = ip;
  new->count = (long_long)0;
  new->insts = malloc(1);
  assert(new->insts != NULL);
  new->ninsts = 0;
  blocks[hash(ip)] = new;
  return new;
}

void add_inst(block_count *b, char *inst)
{
  b->insts = realloc(b->insts, (b->ninsts+1) * sizeof(char *));
  b->insts[b->ninsts++] = inst;
}

void vm_count_block(Inst *ip)
{
  vm_block_insert(ip)->count++;
}

void vm_uncount_block(Inst *ip)
{
  vm_block_insert(ip)->count--;
}

void postprocess_block(block_count *b)
{
  Inst *ip = b->ip;
  block_count *next_block=NULL;

  if (b->count == 0)
    return;
  while (next_block == NULL) {
#include "java-profile.i"
    /* else */
    {
      add_inst(b,"unknown");
      ip++;
    }
  _endif_:
    next_block = block_lookup(ip);
  }
  /* we fell through, so set fallthrough and update the count */
  b->fallthrough = next_block;
  /* also update the counts of all following fallthrough blocks that
     have already been processed */
  while (next_block != NULL) {
    next_block->count += b->count;
    next_block = next_block->fallthrough;
  }
}

/* Deal with block entry by falling through from non-SUPER_END
   instructions.  And fill the insts and ninsts fields. */
void postprocess(void)
{
  size_t i;

  for (i=0; i<HASH_SIZE; i++) {
    block_count *b = blocks[i];
    for (; b!=0; b = b->next)
      postprocess_block(b);
   }
}

#if 0
void print_block(FILE *file, block_count *b)
{
  size_t i;

  fprintf(file,"%14lld\t",b->count);
  for (i=0; i<b->ninsts; i++)
    fprintf(file, "%s ", b->insts[i]);
  putc('\n', file);
}
#endif

void print_block(FILE *file, block_count *b)
{
  size_t i,j,k;

  for (i=2; i<12; i++)
    for (j=0; i+j<=b->ninsts; j++) {
      fprintf(file,"%14lld\t",b->count);
      for (k=j; k<i+j; k++)
	fprintf(file, "%s ", b->insts[k]);
      putc('\n', file);
    }
}

void vm_print_profile(FILE *file)
{
  size_t i;

  postprocess();
  for (i=0; i<HASH_SIZE; i++) {
    block_count *b = blocks[i];
    for (; b!=0; b = b->next)
      print_block(file, b);
   }
}
