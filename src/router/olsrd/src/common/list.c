/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
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

#include <stdlib.h>
#include "common/list.h"

/* init a circular list  */
void
list_head_init(struct list_node *node)
{
  node->prev = node;
  node->next = node;
}

void
list_node_init(struct list_node *node)
{
  node->prev = NULL;
  node->next = NULL;
}

int
list_node_on_list(struct list_node *node)
{
  if (node->prev || node->next) {
    return 1;
  }

  return 0;
}

int
list_is_empty(struct list_node *node)
{
  if (node->prev == node && node->next == node) {
    return 1;
  }

  return 0;
}

void
list_add_after(struct list_node *pos_node, struct list_node *new_node)
{
  new_node->next = pos_node->next;
  new_node->prev = pos_node;

  pos_node->next->prev = new_node;
  pos_node->next = new_node;
}

void
list_add_before(struct list_node *pos_node, struct list_node *new_node)
{
  new_node->prev = pos_node->prev;
  new_node->next = pos_node;

  pos_node->prev->next = new_node;
  pos_node->prev = new_node;
}

void
list_remove(struct list_node *del_node)
{
  del_node->next->prev = del_node->prev;
  del_node->prev->next = del_node->next;

  list_node_init(del_node);
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
