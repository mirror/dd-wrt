/*
 * Copyright (C) 1999 Yasuhiro Ohara
 *
 * This file is part of GNU Zebra.
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

#ifndef OSPF6_LIST_H
#define OSPF6_LIST_H

struct ospf6_list_node {
  int lock;
  struct ospf6_list *list;
  struct ospf6_list_node *prev;
  struct ospf6_list_node *next;

  void *data;
};

struct ospf6_list {
  int count;
  struct ospf6_list_node *head;
  struct ospf6_list_node *tail;
};

/* function definition */
struct ospf6_list_node *
  ospf6_list_head (struct ospf6_list *);
struct ospf6_list_node *
  ospf6_list_next (struct ospf6_list_node *);

struct ospf6_list_node *
  ospf6_list_add (void *, struct ospf6_list *);
struct ospf6_list_node *
  ospf6_list_add_index (void *, struct ospf6_list *, int);

struct ospf6_list_node *
  ospf6_list_lookup (void *, struct ospf6_list *);

void
  ospf6_list_remove_node (struct ospf6_list_node *);
void
  ospf6_list_remove (void *, struct ospf6_list *);
void
  ospf6_list_remove_index (struct ospf6_list *, int);

struct ospf6_list *
  ospf6_list_create ();
void
  ospf6_list_delete (struct ospf6_list *);

#endif /*OSPF6_LIST_H*/


