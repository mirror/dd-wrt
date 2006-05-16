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

#include <zebra.h>

#include "command.h"
#include "memory.h"
#include "log.h"
#include "buffer.h"

#include "bgpd/bgpd.h"
#include "bgpd/bgp_community.h"
#include "bgpd/bgp_clist.h"
#include "bgpd/bgp_aspath.h"
#include "bgpd/bgp_regex.h"

enum community_entry_type
{
  COMMUNITY_DENY,
  COMMUNITY_PERMIT
};

enum community_entry_style
{
  COMMUNITY_LIST,
  COMMUNITY_REGEXP,
};

struct community_list_list
{
  struct community_list *head;
  struct community_list *tail;
};

struct community_list_master
{
  struct community_list_list num;
  struct community_list_list str;
};

struct community_entry
{
  struct community_entry *next;
  struct community_entry *prev;

  enum community_entry_type type;

  enum community_entry_style style;

  struct community *com;
  char *regexp;
  regex_t *reg;
};

static struct community_list_master community_list_master =
{
  {NULL, NULL},
  {NULL, NULL}
};

struct community_entry *
community_entry_new ()
{
  struct community_entry *new;

  new = XMALLOC (MTYPE_COMMUNITY_ENTRY, sizeof (struct community_entry));
  memset (new, 0, sizeof (struct community_entry));
  return new;
}

void
community_entry_free (struct community_entry *entry)
{
  if (entry->com)
    community_free (entry->com);
  if (entry->regexp)
    {
      XFREE (MTYPE_COMMUNITY_REGEXP, entry->regexp);
      bgp_regex_free (entry->reg);
    }
  XFREE (MTYPE_COMMUNITY_ENTRY, entry);
}

struct community_entry *
community_entry_make (struct community *com, enum community_entry_type type)
{
  struct community_entry *entry;

  entry = community_entry_new ();
  entry->com = com;
  entry->type = type;
  entry->style = COMMUNITY_LIST;

  return entry;
}

struct community_list *
community_list_new ()
{
  struct community_list *new;

  new = XMALLOC (MTYPE_COMMUNITY_LIST, sizeof (struct community_list));
  memset (new, 0, sizeof (struct community_list));
  return new;
}

void
community_list_free (struct community_list *list)
{
  if (list->name)
    XFREE (0, list->name);
  XFREE (MTYPE_COMMUNITY_LIST, list);
}

struct community_list *
community_list_insert (char *name)
{
  int i;
  long number;
  struct community_list *new;
  struct community_list *point;
  struct community_list_list *list;

  /* Allocate new community_list and copy given name. */
  new = community_list_new ();
  new->name = strdup (name);

  /* If name is made by all digit character.  We treat it as
     number. */
  for (number = 0, i = 0; i < strlen (name); i++)
    {
      if (isdigit ((int) name[i]))
	number = (number * 10) + (name[i] - '0');
      else
	break;
    }

  /* In case of name is all digit character */
  if (i == strlen (name))
    {
      new->sort = COMMUNITY_LIST_NUMBER;

      /* Set access_list to number list. */
      list = &community_list_master.num;

      for (point = list->head; point; point = point->next)
	if (atol (point->name) >= number)
	  break;
    }
  else
    {
      new->sort = COMMUNITY_LIST_STRING;

      /* Set access_list to string list. */
      list = &community_list_master.str;
  
      /* Set point to insertion point. */
      for (point = list->head; point; point = point->next)
	if (strcmp (point->name, name) >= 0)
	  break;
    }

  /* In case of this is the first element of master. */
  if (list->head == NULL)
    {
      list->head = list->tail = new;
      return new;
    }

  /* In case of insertion is made at the tail of access_list. */
  if (point == NULL)
    {
      new->prev = list->tail;
      list->tail->next = new;
      list->tail = new;
      return new;
    }

  /* In case of insertion is made at the head of access_list. */
  if (point == list->head)
    {
      new->next = list->head;
      list->head->prev = new;
      list->head = new;
      return new;
    }

  /* Insertion is made at middle of the access_list. */
  new->next = point;
  new->prev = point->prev;

  if (point->prev)
    point->prev->next = new;
  point->prev = new;

  return new;
}

struct community_list *
community_list_lookup (char *name)
{
  struct community_list *list;

  if (name == NULL)
    return NULL;

  for (list = community_list_master.num.head; list; list = list->next)
    if (strcmp (list->name, name) == 0)
      return list;

  for (list = community_list_master.str.head; list; list = list->next)
    if (strcmp (list->name, name) == 0)
      return list;

  return NULL;
}

struct community_list *
community_list_get (char *name)
{
  struct community_list *list;

  list = community_list_lookup (name);
  if (list == NULL)
    list = community_list_insert (name);
  return list;
}

struct community_entry *
community_entry_lookup (struct community_list *list, 
			struct community *com, enum community_entry_type type)
{
  struct community_entry *entry;

  for (entry = list->head; entry; entry = entry->next)
    if (entry->style == COMMUNITY_LIST)
      if (community_cmp (entry->com, com))
	return entry;
  return NULL;
}

struct community_entry *
community_entry_regexp_lookup (struct community_list *list,
			       char *str, enum community_entry_type type)
{
  struct community_entry *entry;

  for (entry = list->head; entry; entry = entry->next)
    if (entry->style == COMMUNITY_REGEXP)
      if (strcmp (entry->regexp, str) == 0)
	return entry;
  return NULL;
}

void
community_list_entry_add (struct community_list *list, 
			  struct community_entry *entry)
{
  entry->next = NULL;
  entry->prev = list->tail;

  if (list->tail)
    list->tail->next = entry;
  else
    list->head = entry;
  list->tail = entry;
}

void
community_list_delete (struct community_list *list)
{
  struct community_list_list *clist;
  struct community_entry *entry, *next;

  for (entry = list->head; entry; entry = next)
    {
      next = entry->next;
      community_entry_free (entry);
    }

  if (list->sort == COMMUNITY_LIST_NUMBER)
    clist = &community_list_master.num;
  else
    clist = &community_list_master.str;

  if (list->next)
    list->next->prev = list->prev;
  else
    clist->tail = list->prev;

  if (list->prev)
    list->prev->next = list->next;
  else
    clist->head = list->next;

  community_list_free (list);
}

int 
community_list_empty (struct community_list *list)
{
  if (list->head == NULL && list->tail == NULL)
    return 1;
  else
    return 0;
}

void
community_list_entry_delete (struct community_list *list,
			     struct community_entry *entry)
{
  if (entry->next)
    entry->next->prev = entry->prev;
  else
    list->tail = entry->prev;

  if (entry->prev)
    entry->prev->next = entry->next;
  else
    list->head = entry->next;

  community_entry_free (entry);

  if (community_list_empty (list))
    community_list_delete (list);
}

int
community_match_regexp (struct community_entry *entry,
			struct community *com)
{
  int i;
  char c[12];
  u_int32_t comval;

  /* This is an evil special case, sorry */
  if(strcmp(entry->regexp, "^$") == 0
     && (com == NULL || com->size == 0))
    {
      return 1;
    }

  for (i = 0; i < com->size; i++)
    {
      memcpy (&comval, com_nthval (com, i), sizeof (u_int32_t));
      comval = ntohl (comval);

      sprintf(c, "%d:%d", (comval >> 16) & 0xFFFF, comval & 0xFFFF);
      
      if (regexec (entry->reg, c, 0, NULL, 0) == 0)
	return 1;
    }
  return 0;
}

struct community *
community_delete_regexp (struct community *com, regex_t *reg)
{
  int i;
  char c[12];
  u_int32_t comval;

  i = 0;
  while (i < com->size)
    {
      memcpy (&comval, com_nthval (com, i), sizeof (u_int32_t));
      comval = ntohl (comval);

      sprintf(c, "%d:%d", (comval >> 16) & 0xFFFF, comval & 0xFFFF);

      if (regexec (reg, c, 0, NULL, 0) == 0)
	{
	  /* Matched - delete! */
	  community_del_val (com, com_nthval (com, i));
	}
      else
	i++;
    }

  return com;
}

/* Delete all permitted communities in the list from com1 */
struct community *
community_list_delete_entries (struct community *com1,
			       struct community_list *list)
{
  struct community_entry *entry;

  for (entry = list->head; entry; entry = entry->next)
    {
      if (entry->style == COMMUNITY_LIST)
	{
	  if (entry->type == COMMUNITY_PERMIT)
	    community_delete (com1, entry->com);
	}
      else if (entry->style == COMMUNITY_REGEXP)
	{
	  if (entry->type == COMMUNITY_PERMIT)
	    community_delete_regexp (com1, entry->reg);
	}
    }

  return com1;
}


int
community_list_match (struct community *com, struct community_list *list)
{
  struct community_entry *entry;

  for (entry = list->head; entry; entry = entry->next)
    {
      if (entry->style == COMMUNITY_LIST)
	{
	  if (community_match (com, entry->com))
	    return (entry->type == COMMUNITY_PERMIT ? 1 : 0);
	}
      else if (entry->style == COMMUNITY_REGEXP)
	{
	  if (community_match_regexp (entry, com))
	    return (entry->type == COMMUNITY_PERMIT ? 1 : 0);
	}
    }
  return 0;
}

int
community_list_match_exact (struct community *com, struct community_list *list)
{
  struct community_entry *entry;

  for (entry = list->head; entry; entry = entry->next)
    if (entry->style == COMMUNITY_LIST)
      if (community_cmp (com, entry->com))
	return (entry->type == COMMUNITY_PERMIT ? 1 : 0);
  return 0;
}

char *
community_type_str (enum community_entry_type type)
{
  switch (type)
    {
    case COMMUNITY_DENY:
      return "deny";
      break;
    case COMMUNITY_PERMIT:
      return "permit";
      break;
    default:
      return "";
      break;
    }
}

void
community_list_print (struct community_list *list)
{
  struct community_entry *entry;

  for (entry = list->head; entry; entry = entry->next)
    printf ("ip community-list %s %s%s\n", list->name, 
	    community_type_str (entry->type), community_print (entry->com));
}

int
community_list_dup_check (struct community_list *list, 
			  struct community_entry *new)
{
  struct community_entry *entry;
  
  for (entry = list->head; entry; entry = entry->next)
    {
      if (entry->style != new->style)
        continue;
      if (entry->style == COMMUNITY_LIST)
	{
	  if (entry->type == new->type
	      && community_cmp (entry->com, new->com))
	    return 1;
	}
      else if (entry->style == COMMUNITY_REGEXP)
	{
	  if (entry->type == new->type
	      && (strcmp(entry->regexp, new->regexp) == 0))
	    return 1;
	}
    }
  return 0;
}

DEFUN (ip_community_list, ip_community_list_cmd,
       "ip community-list WORD (deny|permit) .AA:NN",
       IP_STR
       "Add a community list entry\n"
       "Community list name\n"
       "Specify community to reject\n"
       "Specify community to accept\n"
       "Community number in aa:nn format or local-AS|no-advertise|no-export\n")
{
  enum community_entry_type type;
  struct community_entry *entry;
  struct community_list *list;
  struct community *com;
  struct buffer *b;
  regex_t *regex;
  int i;
  char *str;
  int first = 0;

  /* Check the list type. */
  if (strncmp (argv[1], "p", 1) == 0)
    type = COMMUNITY_PERMIT;
  else if (strncmp (argv[1], "d", 1) == 0)
    type = COMMUNITY_DENY;
  else
    {
      vty_out (vty, "community-list type must be [permit|deny]%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  b = buffer_new (1024);
  for (i = 2; i < argc; i++)
    {
      if (first)
	buffer_putc (b, ' ');
      else
	first = 1;

      buffer_putstr (b, argv[i]);
    }
  buffer_putc (b, '\0');

  str = buffer_getstr (b);
  buffer_free (b);

  com = community_str2com (str);
  if (com)
    {
      entry = community_entry_make (com, type);
      free (str);
    }
  else
    {
      regex = bgp_regcomp (str);
      if (regex)
	{
	  entry = community_entry_new ();
	  entry->reg = regex;
	  entry->regexp = XSTRDUP (MTYPE_COMMUNITY_REGEXP, str);
	  entry->type = type;
	  entry->style = COMMUNITY_REGEXP;
	  free(str);
	}
      else
	{
	  vty_out (vty, "Community-list malformed: %s%s", str,
		   VTY_NEWLINE);
	  free (str);
	  return CMD_WARNING;
	}
    }

  /* Install new community list to the community_list. */
  list = community_list_get (argv[0]);

  if (community_list_dup_check (list, entry))
    community_entry_free (entry);
  else
    community_list_entry_add (list, entry);

  return CMD_SUCCESS;
}

DEFUN (no_ip_community_list,
       no_ip_community_list_cmd,
       "no ip community-list WORD (deny|permit) .AA:NN",
       NO_STR
       IP_STR
       "Add a community list entry\n"
       "Community list name\n"
       "Specify community to reject\n"
       "Specify community to accept\n"
       "Community number in aa:nn format or local-AS|no-advertise|no-export\n")
{
  enum community_entry_type type;
  struct community_entry *entry;
  struct community_list *list;
  struct community *com;
  struct buffer *b;
  regex_t *regex;
  int i;
  char *str;
  int first = 0;

  list = community_list_lookup (argv[0]);
  if (list == NULL)
    {
      vty_out (vty, "ip community-list %s doesn't exist.%s", argv[0],
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Check the list type. */
  if (strncmp (argv[1], "p", 1) == 0)
    type = COMMUNITY_PERMIT;
  else if (strncmp (argv[1], "d", 1) == 0)
    type = COMMUNITY_DENY;
  else
    {
      vty_out (vty, "community-list type must be [permit|deny]%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  b = buffer_new (1024);
  for (i = 2; i < argc; i++)
    {
      if (first)
	buffer_putc (b, ' ');
      else
	first = 1;

      buffer_putstr (b, argv[i]);
    }
  buffer_putc (b, '\0');

  str = buffer_getstr (b);
  buffer_free (b);

  com = community_str2com (str);

  if (com)
    {
      free (str);
      entry = community_entry_lookup (list, com, type);
    }
  else
    {
      regex = bgp_regcomp (str);
      if (regex)
	{
	  entry = community_entry_regexp_lookup (list, str, type);
	  free(str);
	}
      else
	{
 	  vty_out (vty, "Community-list malformed: %s%s", str,
 		   VTY_NEWLINE);
 	  free (str);
	  return CMD_WARNING;
 	}
    }

  if (entry == NULL)
    {
      vty_out (vty, "Can't find specified community list.%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  community_list_entry_delete (list, entry);

  return CMD_SUCCESS;
}

DEFUN (no_ip_community_list_all,
       no_ip_community_list_all_cmd,
       "no ip community-list WORD",
       NO_STR
       IP_STR
       "Add a community list entry\n"
       "Community list name\n")
{
  struct community_list *list;

  list = community_list_lookup (argv[0]);
  if (list == NULL)
    {
      vty_out (vty, "ip community-list %s doesn't exist.%s", argv[0],
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  community_list_delete (list);

  return CMD_SUCCESS;
}

int
config_write_community (struct vty *vty)
{
  struct community_list *list;
  struct community_entry *entry;
  int write = 0;

  for (list = community_list_master.num.head; list; list = list->next)
    for (entry = list->head; entry; entry = entry->next)
      {
	vty_out (vty, "ip community-list %s %s%s%s%s",
		 list->name,
		 community_type_str (entry->type),
		 entry->style == COMMUNITY_LIST ? "" : " ",
		 entry->style == COMMUNITY_LIST
		 ? community_print (entry->com) : entry->regexp,
  		 VTY_NEWLINE);
	write++;
      }

  for (list = community_list_master.str.head; list; list = list->next)
    for (entry = list->head; entry; entry = entry->next)
      {
 	vty_out (vty, "ip community-list %s %s%s%s%s",
 		 list->name,
 		 community_type_str (entry->type),
 		 entry->style == COMMUNITY_LIST ? "" : " ",
 		 entry->style == COMMUNITY_LIST
 		 ? community_print (entry->com) : entry->regexp,
  		 VTY_NEWLINE);
	write++;
      }
  return write;
}

struct cmd_node community_list_node =
{
  COMMUNITY_LIST_NODE,
  "",
  1
};

void
community_list_init ()
{
  install_node (&community_list_node, config_write_community);

  install_element (CONFIG_NODE, &ip_community_list_cmd);
  install_element (CONFIG_NODE, &no_ip_community_list_cmd);
  install_element (CONFIG_NODE, &no_ip_community_list_all_cmd);
}

void
community_list_test ()
{
  struct community *com1;
  struct community *com2;
  struct community_list *list;
  struct community_entry *entry;

  /* ip community-list 10 permit no-export. */
  list = community_list_get ("10");

  com1 = community_str2com ("7675:70");
  entry = community_entry_make (com1, COMMUNITY_PERMIT);
  community_list_entry_add (list, entry);

  com1 = community_str2com ("7675:60");
  entry = community_entry_make (com1, COMMUNITY_PERMIT);
  community_list_entry_add (list, entry);

  com2 = community_str2com ("7675:60");
  entry = community_entry_lookup (list, com2, COMMUNITY_PERMIT);
  community_list_entry_delete (list, entry);

  community_list_print (list);
}
