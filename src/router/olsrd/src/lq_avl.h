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
 * $Id: lq_avl.h,v 1.8 2007/03/29 00:05:50 tlopatic Exp $
 */

#ifndef _LQ_AVL_H
#define _LQ_AVL_H

struct avl_node
{
  struct avl_node *parent;
  struct avl_node *left;
  struct avl_node *right;
  struct avl_node *next;
  struct avl_node *prev;
  void *key;
  void *data;
  char balance;
  char leader;
};

struct avl_tree
{
  struct avl_node *root;
  int (*comp)(void *, void *);
};

void avl_init(struct avl_tree *, int (*)(void *, void *));
struct avl_node *avl_find(struct avl_tree *, void *);
int avl_insert(struct avl_tree *, struct avl_node *, int);
void avl_delete(struct avl_tree *, struct avl_node *);
struct avl_node *avl_walk_first(struct avl_tree *);
struct avl_node *avl_walk_next(struct avl_node *);

#define inline_avl_comp_ipv4(ip1, ip2) \
  (*(unsigned int *)ip1 == *(unsigned int *)ip2 ? 0 : \
  *(unsigned int *)ip1 < *(unsigned int *)ip2 ? -1 : +1)

#endif
