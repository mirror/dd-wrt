/*
 * BGP Community list.
 * Copyright (C) 1999 Kunihiro Ishiguro
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
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */
enum community_list_sort
{
  COMMUNITY_LIST_STRING,
  COMMUNITY_LIST_NUMBER
};

struct community_list
{
  char *name;

  enum community_list_sort sort;

  struct community_list *next;
  struct community_list *prev;

  struct community_entry *head;
  struct community_entry *tail;
};

struct community_list *community_list_lookup (char *);
int community_list_match (struct community *, struct community_list *);
int community_list_match_exact (struct community *, struct community_list *);
struct community *community_list_delete_entries (struct community *, struct community_list *);
void community_list_init ();
