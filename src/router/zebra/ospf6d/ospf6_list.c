/*
 * Copyright (C) 1999 Yasuhiro Ohara
 * * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
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

/*
 * OSPF6 List:
 *   - don't use "while" iteration
 *   - use ospf6_list_head () & ospf6_list_next () for iteration.
 *   - don't forget to unlock current node
 *     before "break"ing "for" iteration, because lock will be broken
 */

#include <zebra.h>

#include "memory.h"
#include "log.h"

#include "ospf6_list.h"

#ifdef TEST
void _zlog_warn (const char *format, ...) {}
void _zlog_info (const char *format, ...) {}
#define zlog_warn _zlog_warn
#define zlog_info _zlog_info
#endif /*TEST*/

static struct ospf6_list *
ospf6_list_new ()
{
  struct ospf6_list *list;

  list = (struct ospf6_list *) XMALLOC
           (MTYPE_LINK_LIST, sizeof (struct ospf6_list));
  if ( ! list)
    zlog_err ("ospf6_list: list malloc failure");
  else
    memset (list, 0, sizeof (struct ospf6_list));

  return list;
}

static void
ospf6_list_free (struct ospf6_list *list)
{
  if (list->count)
    zlog_warn ("ospf6_list: delete list not empty");

  XFREE (MTYPE_LINK_LIST, list);
}

static struct ospf6_list_node *
ospf6_list_node_new (struct ospf6_list *list)
{
  struct ospf6_list_node *node;

  node = (struct ospf6_list_node *) XMALLOC
           (MTYPE_LINK_NODE, sizeof (struct ospf6_list_node));

  if (! node)
    {
      zlog_err ("ospf6_list: node malloc failure");
      return NULL;
    }

  memset (node, 0, sizeof (struct ospf6_list_node));
  node->list = list;

  return node;
}

static void
ospf6_list_node_free (struct ospf6_list_node *node)
{
  XFREE (MTYPE_LINK_NODE, node);
}

static void
ospf6_list_node_lock (struct ospf6_list_node *node)
{
  node->lock ++;
}

static void
ospf6_list_node_unlock (struct ospf6_list_node *node)
{
  node->lock --;

  if (node->lock == 0)
    ospf6_list_node_free (node);
}

static void
ospf6_list_add_node_head (struct ospf6_list_node *node)
{
  if (! node->list->count)
    {
      node->list->head = node->list->tail = node;
    }
  else
    {
      node->list->head->prev = node;
      node->next = node->list->head;
      node->list->head = node;
    }

  ospf6_list_node_lock (node);
  node->list->count ++;
}

static void
ospf6_list_add_node_tail (struct ospf6_list_node *node)
{
  if (! node->list->count)
    {
      node->list->head = node->list->tail = node;
    }
  else
    {
      node->list->tail->next = node;
      node->prev = node->list->tail;
      node->list->tail = node;
    }

  ospf6_list_node_lock (node);
  node->list->count ++;
}

void
ospf6_list_remove_node (struct ospf6_list_node *node)
{
  if (node->prev)
    node->prev->next = node->next;
  if (node->next)
    node->next->prev = node->prev;

  if (node->list->head == node)
    node->list->head = node->next;
  if (node->list->tail == node)
    node->list->tail = node->prev;

  ospf6_list_node_unlock (node);
  node->list->count --;
}

struct ospf6_list_node *
ospf6_list_head (struct ospf6_list *list)
{
  if (! list->head)
    return NULL;

  ospf6_list_node_lock (list->head);
  return list->head;
}

struct ospf6_list_node *
ospf6_list_next (struct ospf6_list_node *node)
{
  ospf6_list_node_unlock (node);
  if (node->next)
    ospf6_list_node_lock (node->next);
  return node->next;
}

struct ospf6_list_node *
ospf6_list_add (void *data, struct ospf6_list *list)
{
  struct ospf6_list_node *node;

  node = ospf6_list_node_new (list);
  if (! node)
    {
      zlog_warn ("list: can't add node");
      return NULL;
    }

  node->data = data;

  ospf6_list_add_node_tail (node);
  return node;
}

static void
ospf6_list_add_node_index (struct ospf6_list_node *node, int index)
{
  int i;
  struct ospf6_list_node *n;

  i = index;

  if (i == 0)
    {
      ospf6_list_add_node_head (node);
      return;
    }
  else if (i >= node->list->count)
    {
      ospf6_list_add_node_tail (node);
      return;
    }

  for (n = ospf6_list_head (node->list); n; n = ospf6_list_next (n))
    {
      if (i == 1)
        {
          node->prev = n;
          node->next = n->next;
          n->next = node;

          if (node->next == NULL) /* This is tail */
            node->list->tail = node;

          ospf6_list_node_lock (node);
          node->list->count ++;
        }

      if (i > 0)
        i --;
    }

  return;
}

struct ospf6_list_node *
ospf6_list_add_index (void *data, struct ospf6_list *list, int index)
{
  struct ospf6_list_node *node;

  node = ospf6_list_node_new (list);
  if (! node)
    {
      zlog_warn ("list: can't add node");
      return NULL;
    }

  node->data = data;

  ospf6_list_add_node_index (node, index);
  return node;
}


struct ospf6_list_node *
ospf6_list_lookup (void *data, struct ospf6_list *list)
{
  struct ospf6_list_node *node;
  struct ospf6_list_node *found;

  found = (struct ospf6_list_node *) NULL;
  for (node = ospf6_list_head (list); node; node = ospf6_list_next (node))
    {
      if (node->data == data)
        found = node;
    }
  return found;
}

void
ospf6_list_remove (void *data, struct ospf6_list *list)
{
  struct ospf6_list_node *node;

  node = ospf6_list_lookup (data, list);
  if (node)
    ospf6_list_remove_node (node);
  else
    zlog_warn ("list: can't delete");
}

void
ospf6_list_remove_index (struct ospf6_list *list, int index)
{
  int i;
  struct ospf6_list_node *node;

  if (index >= list->count)
    {
      zlog_warn ("list: index exceeds list size");
      return;
    }

  i = 0;
  for (node = ospf6_list_head (list); node; node = ospf6_list_next (node))
    {
      if (i == index)
        ospf6_list_remove_node (node);

      i++;
    }
}

struct ospf6_list *
ospf6_list_create ()
{
  return ospf6_list_new ();
}

void
ospf6_list_delete (struct ospf6_list *list)
{
  struct ospf6_list_node *node;

  for (node = ospf6_list_head (list); node; node = ospf6_list_next (node))
    ospf6_list_remove_node (node);

  ospf6_list_free (list);
}

void
  ospf6_list_debug (struct ospf6_list *list)
{
  struct ospf6_list_node *node;

  printf ("list(%#x):\n", (u_int)list);
  printf ("  count: %d head: %#x tail: %#x\n",
          list->count, (u_int)list->head, (u_int)list->tail);
  for (node = ospf6_list_head (list); node; node = ospf6_list_next (node))
    {
      printf ("    node(%#x):  data: %#x\n", (u_int)node, (u_int)node->data);
      printf ("      prev:%#x,next:%#x,list:%#x,lock:%#x\n",
              (u_int)node->prev, (u_int)node->next,
              (u_int)node->list, node->lock);
    }
}

#ifdef TEST

#include "command.h"

extern struct 
{
  char *name;
  unsigned long alloc;
  unsigned long t_malloc;
  unsigned long c_malloc;
  unsigned long t_calloc;
  unsigned long c_calloc;
  unsigned long t_realloc;
  unsigned long t_free;
  unsigned long c_strdup;
} mstat [MTYPE_MAX];

int main ()
{
  struct ospf6_list *list;
  struct ospf6_list_node *node;
 
  int flag;
  flag |= ZLOG_STDOUT;
  zlog_default = openzlog ("test", flag, ZLOG_OSPF6,
                 LOG_CONS|LOG_NDELAY|LOG_PERROR|LOG_PID,
                 LOG_DAEMON);

  cmd_init (1);
  memory_init ();

  list = ospf6_list_create ();

  ospf6_list_add_index ("This", list, 0);
  ospf6_list_add_index ("is", list, 1);
  ospf6_list_add_index ("a", list, 2);
  ospf6_list_add_index ("test", list, 3);
  ospf6_list_add_index ("for", list, 4);
  ospf6_list_add_index ("list", list, 5);
  ospf6_list_add_index ("of", list, 6);
  ospf6_list_add_index ("ospf6", list, 7);
  ospf6_list_add_index ("!!", list, 8);
  ospf6_list_add_index ("!!", list, 0);

  ospf6_list_debug (list);
  for (node = ospf6_list_head (list); node; node = ospf6_list_next (node))
    {
      printf ("%s ", (char *)node->data);
    }
  printf ("\n");
  printf ("LIST: %ld LISTNODE: %ld\n",
          mstat[MTYPE_LINK_LIST].alloc, mstat[MTYPE_LINK_NODE].alloc);

  for (node = ospf6_list_head (list); node; node = ospf6_list_next (node))
    {
      if (strcmp ((char *)node->data, "test") == 0)
        ospf6_list_remove (node->data, list);
    }

  for (node = ospf6_list_head (list); node; node = ospf6_list_next (node))
    {
      printf ("%s ", (char *)node->data);
    }
  printf ("\n");
  printf ("LIST: %ld LISTNODE: %ld\n",
          mstat[MTYPE_LINK_LIST].alloc, mstat[MTYPE_LINK_NODE].alloc);

  ospf6_list_add_index ("test", list, 3);
  for (node = ospf6_list_head (list); node; node = ospf6_list_next (node))
    {
      printf ("%s ", (char *)node->data);
    }
  printf ("\n");
  printf ("LIST: %ld LISTNODE: %ld\n",
          mstat[MTYPE_LINK_LIST].alloc, mstat[MTYPE_LINK_NODE].alloc);

  ospf6_list_remove_index (list, 3);
  for (node = ospf6_list_head (list); node; node = ospf6_list_next (node))
    {
      printf ("%s ", (char *)node->data);
    }
  printf ("\n");
  printf ("LIST: %ld LISTNODE: %ld\n",
          mstat[MTYPE_LINK_LIST].alloc, mstat[MTYPE_LINK_NODE].alloc);

  ospf6_list_delete (list);

  printf ("LIST: %ld LISTNODE: %ld\n",
          mstat[MTYPE_LINK_LIST].alloc, mstat[MTYPE_LINK_NODE].alloc);

  return 0;
}

#endif /*TEST*/


