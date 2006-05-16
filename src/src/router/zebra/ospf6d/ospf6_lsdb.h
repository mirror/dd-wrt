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

#ifndef OSPF6_LSDB_H
#define OSPF6_LSDB_H

#include "ospf6_lsa.h"

#define MAXLSASIZE   1024

struct ospf6_lsdb_node
{
  struct ospf6_lsdb *lsdb;
  unsigned int lock;

  struct ospf6_lsdb_node *next;
  struct ospf6_lsdb_node *prev;
  struct ospf6_lsa *lsa;
};

struct ospf6_lsdb
{
  struct ospf6_lsdb_node *head;
  struct ospf6_lsdb_node *tail;
  unsigned int count;
};

/* Function Prototypes */
void ospf6_lsdb_init ();
struct ospf6_lsdb *ospf6_lsdb_create ();
void ospf6_lsdb_delete (struct ospf6_lsdb *lsdb);
void ospf6_lsdb_remove_maxage (struct ospf6_lsdb *lsdb);

struct ospf6_lsa *
  ospf6_lsdb_lookup (u_int16_t type, u_int32_t id, u_int32_t adv_router);

void ospf6_lsdb_install (struct ospf6_lsa *new);

struct ospf6_lsdb_node *ospf6_lsdb_head (struct ospf6_lsdb *lsdb);
struct ospf6_lsdb_node *ospf6_lsdb_next (struct ospf6_lsdb_node *node);
void ospf6_lsdb_end (struct ospf6_lsdb_node *node);
void ospf6_lsdb_add (struct ospf6_lsa *lsa, struct ospf6_lsdb *lsdb);
void ospf6_lsdb_remove (struct ospf6_lsa *lsa, struct ospf6_lsdb *lsdb);
void ospf6_lsdb_remove_all (struct ospf6_lsdb *lsdb);

struct ospf6_lsa *
ospf6_lsdb_lookup_lsdb (u_int16_t type, u_int32_t id, u_int32_t adv_router,
                        struct ospf6_lsdb *lsdb);

#endif /* OSPF6_LSDB_H */

