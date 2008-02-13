/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Thomas Lopatic (thomas@lopatic.de)
 * IPv4 performance optimization (c) 2006, sven-ola(gmx.de)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 *
 * * Redistributions of source code must retain the above copyright 
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimer in 
 *   the documentation and/or other materials provided with the 
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#ifndef _LQ_AVL_H
#define _LQ_AVL_H

#include "defs.h"

struct avl_node
{
  struct avl_node *parent;
  struct avl_node *left;
  struct avl_node *right;
  struct avl_node *next;
  struct avl_node *prev;
  void *key;
  void *data;
  signed char balance;
  unsigned char leader;
};

typedef int (*avl_tree_comp)(const void *, const void *);

struct avl_tree
{
  struct avl_node *root;
  struct avl_node *first;
  struct avl_node *last;
  unsigned int count;
  avl_tree_comp comp;
};

#define AVL_DUP    1
#define AVL_DUP_NO 0

void avl_init(struct avl_tree *, avl_tree_comp);
struct avl_node *avl_find(struct avl_tree *, const void *);
int avl_insert(struct avl_tree *, struct avl_node *, int);
void avl_delete(struct avl_tree *, struct avl_node *);

static INLINE struct avl_node *avl_walk_first(struct avl_tree *tree) { return tree->first; }
static INLINE struct avl_node *avl_walk_last(struct avl_tree *tree) { return tree->last; }
static INLINE struct avl_node *avl_walk_next(struct avl_node *node) { return node->next; }
static INLINE struct avl_node *avl_walk_prev(struct avl_node *node) { return node->prev; }
/* and const versions*/
static INLINE const struct avl_node *avl_walk_first_c(const struct avl_tree *tree) { return tree->first; }
static INLINE const struct avl_node *avl_walk_last_c(const struct avl_tree *tree) { return tree->last; }
static INLINE const struct avl_node *avl_walk_next_c(const struct avl_node *node) { return node->next; }
static INLINE const struct avl_node *avl_walk_prev_c(const struct avl_node *node) { return node->prev; }

extern avl_tree_comp avl_comp_default;
extern avl_tree_comp avl_comp_prefix_default;
extern int avl_comp_ipv4(const void *, const void *);
extern int avl_comp_ipv6(const void *, const void *);

#endif

/*
 * Local Variables:
 * c-basic-offset: 2
 * End:
 */
