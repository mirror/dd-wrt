/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Thomas Lopatic (thomas@lopatic.de)
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
 * $Id: lq_list.c,v 1.3 2004/12/04 17:06:57 tlopatic Exp $
 */

#include <stdlib.h>
#include "lq_list.h"

void list_init(struct list *list)
{
  list->head = NULL;
  list->tail = NULL;
}

struct list_node *list_get_head(struct list *list)
{
  return list->head;
}

struct list_node *list_get_tail(struct list *list)
{
  return list->tail;
}

struct list_node *list_get_next(struct list_node *node)
{
  return node->next;
}

struct list_node *list_get_prev(struct list_node *node)
{
  return node->prev;
}

void list_add_head(struct list *list, struct list_node *node)
{
  if (list->head != NULL)
    list->head->prev = node;

  else
    list->tail = node;

  node->prev = NULL;
  node->next = list->head;

  list->head = node;
}

void list_add_tail(struct list *list, struct list_node *node)
{
  if (list->tail != NULL)
    list->tail->next = node;

  else
    list->head = node;

  node->prev = list->tail;
  node->next = NULL;

  list->tail = node;
}

void list_add_before(struct list *list, struct list_node *pos_node,
                     struct list_node *node)
{
  if (pos_node->prev != NULL)
    pos_node->prev->next = node;

  else
    list->head = node;

  node->prev = pos_node->prev;
  node->next = pos_node;

  pos_node->prev = node;
}

void list_add_after(struct list *list, struct list_node *pos_node,
                    struct list_node *node)
{
  if (pos_node->next != NULL)
    pos_node->next->prev = node;

  else
    list->tail = node;

  node->prev = pos_node;
  node->next = pos_node->next;

  pos_node->next = node;
}

void list_remove(struct list *list, struct list_node *node)
{
  if (node == list->head)
    list->head = node->next;

  else
    node->prev->next = node->next;

  if (node == list->tail)
    list->tail = node->prev;

  else
    node->next->prev = node->prev;
}
