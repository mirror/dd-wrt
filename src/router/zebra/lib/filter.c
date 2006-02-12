/* Route filtering function.
 * Copyright (C) 1998, 1999 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
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

#include <zebra.h>

#include "prefix.h"
#include "filter.h"
#include "memory.h"
#include "command.h"
#include "sockunion.h"
#include "buffer.h"

/* Filter element of access list */
struct filter
{
  /* For doubly linked list. */
  struct filter *next;
  struct filter *prev;

  /* Filter type information. */
  enum filter_type type;

  /* If this filter is "any" match then this flag is set. */
  int any;

  /* If this filter is "exact" match then this flag is set. */
  int exact;

  /* Prefix information. */
  struct prefix prefix;
};

/* List of access_list. */
struct access_list_list
{
  struct access_list *head;
  struct access_list *tail;
};

/* Master structure of access_list. */
struct access_master
{
  /* List of access_list which name is number. */
  struct access_list_list num;

  /* List of access_list which name is string. */
  struct access_list_list str;

  /* Hook function which is executed when new access_list is added. */
  void (*add_hook) ();

  /* Hook function which is executed when access_list is deleted. */
  void (*delete_hook) ();
};

/* Static structure for IPv4 access_list's master. */
static struct access_master access_master_ipv4 = 
{ 
  {NULL, NULL},
  {NULL, NULL},
  NULL,
  NULL,
};

#ifdef HAVE_IPV6
/* Static structure for IPv6 access_list's master. */
static struct access_master access_master_ipv6 = 
{ 
  {NULL, NULL},
  {NULL, NULL},
  NULL,
  NULL,
};
#endif /* HAVE_IPV6 */

struct access_master *
access_master_get (afi_t afi)
{
  if (afi == AFI_IP)
    return &access_master_ipv4;
#ifdef HAVE_IPV6
  else if (afi == AFI_IP6)
    return &access_master_ipv6;
#endif /* HAVE_IPV6 */
  return NULL;
}

/* Allocate new filter structure. */
struct filter *
filter_new ()
{
  struct filter *new;

  new = XMALLOC (MTYPE_ACCESS_FILTER, sizeof (struct filter));
  bzero (new, sizeof (struct filter));
  return new;
}

void
filter_free (struct filter *filter)
{
  XFREE (MTYPE_ACCESS_FILTER, filter);
}

/* Return string of filter_type. */
static char *
filter_type_str (struct filter *filter)
{
  switch (filter->type)
    {
    case FILTER_PERMIT:
      return "permit";
      break;
    case FILTER_DENY:
      return "deny";
      break;
    case FILTER_DYNAMIC:
      return "dynamic";
      break;
    default:
      return "";
      break;
    }
}

/* Allocate and make new filter. */
struct filter *
filter_make (struct prefix *prefix, enum filter_type type)
{
  struct filter *filter;

  filter = filter_new ();

  /* If prefix is NULL then this is "any" match directive. */
  if (prefix == NULL)
    filter->any = 1;
  else
    prefix_copy (&filter->prefix, prefix);

  filter->type = type;

  return filter;
}

struct filter *
filter_lookup (struct access_list *access, struct prefix *prefix,
	       enum filter_type type, int exact)
{
  struct filter *filter;

  for (filter = access->head; filter; filter = filter->next)
    {
      if (prefix == NULL)
	{
	  if (filter->any == 1 && filter->type == type)
	    return filter;
	}
      else
	{
	  if (prefix_same (&filter->prefix, prefix) &&
	      filter->type == type && filter->exact == exact)
	    return filter;
	}
    }
  return NULL;
}

/* If filter match to the prefix then return 1. */
static int
filter_match (struct filter *filter, struct prefix *p)
{
  if (filter->prefix.family == p->family)
    {
      if (filter->exact)
	{
	  if (filter->prefix.prefixlen == p->prefixlen)
	    return prefix_match (&filter->prefix, p);
	  else
	    return 0;
	}
      else
	return prefix_match (&filter->prefix, p);
    }
  else
    return 0;
}

/* Allocate new access list structure. */
struct access_list *
access_list_new ()
{
  struct access_list *new;

  new = XMALLOC (MTYPE_ACCESS_LIST, sizeof (struct access_list));
  bzero (new, sizeof (struct access_list));
  return new;
}

/* Free allocated access_list. */
void
access_list_free (struct access_list *access)
{
  XFREE (MTYPE_ACCESS_LIST, access);
}

/* Delete access_list from access_master and free it. */
void
access_list_delete (struct access_list *access)
{
  struct filter *filter;
  struct filter *next;
  struct access_list_list *list;
  struct access_master *master;

  for (filter = access->head; filter; filter = next)
    {
      next = filter->next;
      filter_free (filter);
    }

  master = access->master;

  if (access->type == ACCESS_TYPE_NUMBER)
    list = &master->num;
  else
    list = &master->str;

  if (access->next)
    access->next->prev = access->prev;
  else
    list->tail = access->prev;

  if (access->prev)
    access->prev->next = access->next;
  else
    list->head = access->next;

  if (access->name)
    XFREE (MTYPE_ACCESS_LIST_STR, access->name);

  if (access->remark)
    XFREE (MTYPE_TMP, access->remark);

  access_list_free (access);
}

/* Insert new access list to list of access_list.  Each acceess_list
   is sorted by the name. */
struct access_list *
access_list_insert (afi_t afi, char *name)
{
  int i;
  long number;
  struct access_list *access;
  struct access_list *point;
  struct access_list_list *alist;
  struct access_master *master;

  master = access_master_get (afi);
  if (master == NULL)
    return NULL;

  /* Allocate new access_list and copy given name. */
  access = access_list_new ();
  access->name = XSTRDUP (MTYPE_ACCESS_LIST_STR, name);
  access->master = master;

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
      access->type = ACCESS_TYPE_NUMBER;

      /* Set access_list to number list. */
      alist = &master->num;

      for (point = alist->head; point; point = point->next)
	if (atol (point->name) >= number)
	  break;
    }
  else
    {
      access->type = ACCESS_TYPE_STRING;

      /* Set access_list to string list. */
      alist = &master->str;
  
      /* Set point to insertion point. */
      for (point = alist->head; point; point = point->next)
	if (strcmp (point->name, name) >= 0)
	  break;
    }

  /* In case of this is the first element of master. */
  if (alist->head == NULL)
    {
      alist->head = alist->tail = access;
      return access;
    }

  /* In case of insertion is made at the tail of access_list. */
  if (point == NULL)
    {
      access->prev = alist->tail;
      alist->tail->next = access;
      alist->tail = access;
      return access;
    }

  /* In case of insertion is made at the head of access_list. */
  if (point == alist->head)
    {
      access->next = alist->head;
      alist->head->prev = access;
      alist->head = access;
      return access;
    }

  /* Insertion is made at middle of the access_list. */
  access->next = point;
  access->prev = point->prev;

  if (point->prev)
    point->prev->next = access;
  point->prev = access;

  return access;
}

/* Lookup access_list from list of access_list by name. */
struct access_list *
access_list_lookup (afi_t afi, char *name)
{
  struct access_list *access;
  struct access_master *master;

  if (name == NULL)
    return NULL;

  master = access_master_get (afi);
  if (master == NULL)
    return NULL;

  for (access = master->num.head; access; access = access->next)
    if (strcmp (access->name, name) == 0)
      return access;

  for (access = master->str.head; access; access = access->next)
    if (strcmp (access->name, name) == 0)
      return access;

  return NULL;
}

/* Get access list from list of access_list.  If there isn't matched
   access_list create new one and return it. */
struct access_list *
access_list_get (afi_t afi, char *name)
{
  struct access_list *access;

  access = access_list_lookup (afi, name);
  if (access == NULL)
    access = access_list_insert (afi, name);
  return access;
}

/* Print out contents of access list to the terminal. */
void
access_list_print (struct access_list *access)
{
  struct filter *filter;

  printf ("access name %s\n", access->name);

  for (filter = access->head; filter; filter = filter->next)
    {
      if (filter->any)
	printf ("any %s\n", filter_type_str (filter));
      else
	{
	  struct prefix *p;
	  char buf[BUFSIZ];

	  p = &filter->prefix;

	  printf ("%s/%d %s\n", 
		  inet_ntop (p->family, &p->u.prefix, buf, BUFSIZ), 
		  p->prefixlen, filter_type_str (filter));
	}
    }
}

/* Apply access list to object (which should be struct prefix *). */
enum filter_type
access_list_apply (struct access_list *access, void *object)
{
  struct filter *filter;
  struct prefix *p;

  p = (struct prefix *) object;

  if (access == NULL)
    return FILTER_DENY;

  for (filter = access->head; filter; filter = filter->next)
    if (filter->any || filter_match (filter, p))
      return filter->type;

  return FILTER_DENY;
}

/* Add hook function. */
void
access_list_add_hook (void (*func) (struct access_list *access))
{
  access_master_ipv4.add_hook = func;
#ifdef HAVE_IPV6
  access_master_ipv6.add_hook = func;
#endif /* HAVE_IPV6 */
}

/* Delete hook function. */
void
access_list_delete_hook (void (*func) (struct access_list *access))
{
  access_master_ipv4.delete_hook = func;
#ifdef HAVE_IPV6
  access_master_ipv6.delete_hook = func;
#endif /* HAVE_IPV6 */
}

/* Add new filter to the end of specified access_list. */
void
access_list_filter_add (struct access_list *access, struct filter *filter)
{
  filter->next = NULL;
  filter->prev = access->tail;

  if (access->tail)
    access->tail->next = filter;
  else
    access->head = filter;
  access->tail = filter;

  /* Run hook function. */
  if (access->master->add_hook)
    (*access->master->add_hook) (access);
}

/* If access_list has no filter then return 1. */
static int
access_list_empty (struct access_list *access)
{
  if (access->head == NULL && access->tail == NULL)
    return 1;
  else
    return 0;
}

/* Delete filter from specified access_list.  If there is hook
   function execute it. */
void
access_list_filter_delete (struct access_list *access, struct filter *filter)
{
  struct access_master *master;

  master = access->master;

  if (filter->next)
    filter->next->prev = filter->prev;
  else
    access->tail = filter->prev;

  if (filter->prev)
    filter->prev->next = filter->next;
  else
    access->head = filter->next;

  filter_free (filter);

  /* If access_list becomes empty delete it from access_master. */
  if (access_list_empty (access))
    access_list_delete (access);

  /* Run hook function. */
  if (master->delete_hook)
    (*master->delete_hook) (access);
}

/*
  deny    Specify packets to reject
  permit  Specify packets to forward
  dynamic ?
*/

/*
  Hostname or A.B.C.D  Address to match
  any                  Any source host
  host                 A single host address
*/

int
access_list_dup_check (struct access_list *access, struct filter *new)
{
  struct filter *filter;

  for (filter = access->head; filter; filter = filter->next)
    {
      if (filter->any == new->any
	  && filter->exact == new->exact
	  && filter->type == new->type
	  && prefix_same (&filter->prefix, &new->prefix))
	return 1;
    }
  return 0;
}

int
vty_access_list_remark_unset (struct vty *vty, afi_t afi, char *name)
{
  struct access_list *access;

  access = access_list_lookup (afi, name);
  if (! access)
    {
      vty_out (vty, "%% access-list %s doesn't exist%s", name,
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (access->remark)
    {
      XFREE (MTYPE_TMP, access->remark);
      access->remark = NULL;
    }

  if (access->head == NULL && access->tail == NULL && access->remark == NULL)
    access_list_delete (access);

  return CMD_SUCCESS;
}

DEFUN (access_list, access_list_cmd,
       "access-list WORD (deny|permit) (A.B.C.D/M|any)",
       "Add an access list entry\n"
       "Access-list name\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 10.0.0.0/8\n"
       "Any prefix to match\n")
{
  int ret;
  enum filter_type type;
  struct filter *filter;
  struct access_list *access;
  struct prefix p;

  /* Check of filter type. */
  if (strncmp (argv[1], "p", 1) == 0)
    type = FILTER_PERMIT;
  else if (strncmp (argv[1], "d", 1) == 0)
    type = FILTER_DENY;
  else
    {
      vty_out (vty, "filter type must be permit or deny%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* "any" is special token of matching IP addresses.  */
  if (strncmp (argv[2], "a", 1) == 0)
    filter = filter_make (NULL, type);
  else
    {
      /* Check string format of prefix and prefixlen. */
      ret = str2prefix_ipv4 (argv[2], (struct prefix_ipv4 *)&p);
      if (ret <= 0)
	{
	  vty_out (vty, "IP address prefix/prefixlen is malformed%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}
      filter = filter_make (&p, type);
    }

  /* "exact-match" */
  if (argc == 4)
    filter->exact = 1;

  /* Install new filter to the access_list. */
  access = access_list_get (AFI_IP, argv[0]);

  /* Duplicate insertion check. */
  if (access_list_dup_check (access, filter))
    filter_free (filter);
  else
    access_list_filter_add (access, filter);

  return CMD_SUCCESS;
}

ALIAS (access_list, access_list_exact_cmd,
       "access-list WORD (deny|permit) A.B.C.D/M (exact-match|)",
       "Add an access list entry\n"
       "Access-list name\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 10.0.0.0/8\n"
       "Exact match of the prefixes\n")

DEFUN (no_access_list,
       no_access_list_cmd,
       "no access-list WORD (deny|permit) (A.B.C.D/M|any)",
       NO_STR 
       "Add an access list entry\n"
       "Access-list name\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 10.0.0.0/8\n"
       "Any prefix to match\n")
{
  int ret;
  enum filter_type type;
  struct filter *filter;
  struct access_list *access;
  struct prefix p;
  int exact = 0;

  /* Check of filter type. */
  if (strncmp (argv[1], "p", 1) == 0)
    type = FILTER_PERMIT;
  else if (strncmp (argv[1], "d", 1) == 0)
    type = FILTER_DENY;
  else
    {
      vty_out (vty, "filter type must be [permit|deny]%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Looking up access_list. */
  access = access_list_lookup (AFI_IP, argv[0]);
  if (access == NULL)
    {
      vty_out (vty, "%% access-list %s doesn't exist%s", argv[0],
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (argc == 4)
    exact = 1;

  /* Check string format of prefix and prefixlen. */
  if (strncmp (argv[2], "a", 1) == 0)
    filter = filter_lookup (access, NULL, type, exact);
  else
    {
      ret = str2prefix_ipv4 (argv[2], (struct prefix_ipv4 *) &p);
      if (ret <= 0)
	{
	  vty_out (vty, "IP address prefix/prefixlen is malformed%s", VTY_NEWLINE);
	  return CMD_WARNING;
	}
      filter = filter_lookup (access, &p, type, exact);
    }

  /* Looking up filter from access_list. */
  if (filter == NULL)
    {
      vty_out (vty, "%% access-list %s %s %s doesn't exist%s", 
	       argv[0],
	       strncmp (argv[1], "p", 1) == 0 ? "permit" : "deny",
	       strncmp (argv[2], "a", 1) == 0 ? "any" : argv[2],
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Delete filter from access_list. */
  access_list_filter_delete (access, filter);

  return CMD_SUCCESS;
}

ALIAS (no_access_list,
       no_access_list_exact_cmd,
       "no access-list WORD (deny|permit) A.B.C.D/M (exact-match|)",
       NO_STR 
       "Add an access list entry\n"
       "Access-list name\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 10.0.0.0/8\n"
       "Exact match of the prefixes\n")

DEFUN (no_access_list_all,
       no_access_list_all_cmd,
       "no access-list WORD",
       NO_STR
       "Add an access list entry\n"
       "Access-list name\n")
{
  struct access_list *access;

  /* Looking up access_list. */
  access = access_list_lookup (AFI_IP, argv[0]);
  if (access == NULL)
    {
      vty_out (vty, "%% access-list %s doesn't exist%s", argv[0],
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Delete all filter from access-list. */
  access_list_delete (access);
 
  return CMD_SUCCESS;
}

DEFUN (access_list_remark,
       access_list_remark_cmd,
       "access-list WORD remark .LINE",
       "Add an access list entry\n"
       "Access-list name\n"
       "Access list entry comment\n"
       "Comment up to 100 characters\n")
{
  struct access_list *access;
  struct buffer *b;
  int i;

  access = access_list_get (AFI_IP, argv[0]);

  if (access->remark)
    {
      XFREE (MTYPE_TMP, access->remark);
      access->remark = NULL;
    }

  /* Below is remark get codes. */
  b = buffer_new (1024);
  for (i = 1; i < argc; i++)
    {
      buffer_putstr (b, (u_char *)argv[i]);
      buffer_putc (b, ' ');
    }
  buffer_putc (b, '\0');

  access->remark = buffer_getstr (b);

  buffer_free (b);

  return CMD_SUCCESS;
}

DEFUN (no_access_list_remark,
       no_access_list_remark_cmd,
       "no access-list WORD remark",
       NO_STR
       "Add an access list entry\n"
       "Access-list name\n"
       "Access list entry comment\n")
{
  return vty_access_list_remark_unset (vty, AFI_IP, argv[0]);
}
	
ALIAS (no_access_list_remark,
       no_access_list_remark_arg_cmd,
       "no access-list WORD remark .LINE",
       NO_STR
       "Add an access list entry\n"
       "Access-list name\n"
       "Access list entry comment\n"
       "Comment up to 100 characters\n")

#ifdef HAVE_IPV6
DEFUN (ipv6_access_list, ipv6_access_list_cmd,
       "ipv6 access-list WORD (deny|permit) (X:X::X:X/M|any)",
       IPV6_STR
       "Add an access list entry\n"
       "Access-list name\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 3ffe:506::/32\n"
       "Any prefixi to match\n")
{
  int ret;
  enum filter_type type;
  struct filter *filter;
  struct access_list *access;
  struct prefix p;

  /* Check of filter type. */
  if (strncmp (argv[1], "p", 1) == 0)
    type = FILTER_PERMIT;
  else if (strncmp (argv[1], "d", 1) == 0)
    type = FILTER_DENY;
  else
    {
      vty_out (vty, "filter type must be [permit|deny]%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* "any" is special token of matching IP addresses.  */
  if (strncmp (argv[2], "a", 1) == 0)
    filter = filter_make (NULL, type);
  else
    {
      /* Check string format of prefix and prefixlen. */
      ret = str2prefix_ipv6 (argv[2], (struct prefix_ipv6 *) &p);
      if (ret <= 0)
	{
	  vty_out (vty, "IPv6 address prefix/prefixlen is malformed%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}
      filter = filter_make (&p, type);
    }

  /* "exact-match" */
  if (argc == 4)
    filter->exact = 1;

  /* Install new filter to the access_list. */
  access = access_list_get (AFI_IP6, argv[0]);
  access_list_filter_add (access, filter);

  return CMD_SUCCESS;
}

ALIAS (ipv6_access_list, ipv6_access_list_exact_cmd,
       "ipv6 access-list WORD (deny|permit) X:X::X:X/M (exact-match|)",
       IPV6_STR
       "Add an access list entry\n"
       "Access-list name\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 3ffe:506::/32\n"
       "Exact match of the prefixes\n")

DEFUN (no_ipv6_access_list,
       no_ipv6_access_list_cmd,
       "no ipv6 access-list WORD (deny|permit) (X:X::X:X/M|any)",
       NO_STR
       IPV6_STR
       "Add an access list entry\n"
       "Access-list name\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 3ffe:506::/32\n"
       "Any prefixi to match\n")
{
  int ret;
  enum filter_type type;
  struct filter *filter;
  struct access_list *access;
  struct prefix p;
  int exact = 0;

  /* Check of filter type. */
  if (strncmp (argv[1], "p", 1) == 0)
    type = FILTER_PERMIT;
  else if (strncmp (argv[1], "d", 1) == 0)
    type = FILTER_DENY;
  else
    {
      vty_out (vty, "filter type must be [permit|deny]%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Looking up access_list. */
  access = access_list_lookup (AFI_IP6, argv[0]);
  if (access == NULL)
    {
      vty_out (vty, "%% access-list %s doesn't exist%s", argv[0],
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (argc == 4)
    exact = 1;

  /* Check string format of prefix and prefixlen. */
  if (strncmp (argv[2], "a", 1) == 0)
    filter = filter_lookup (access, NULL, type, exact);
  else
    {
      ret = str2prefix_ipv6 (argv[2], (struct prefix_ipv6 *) &p);
      if (ret <= 0)
	{
	  vty_out (vty, "IPv6 address prefix/prefixlen is malformed%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}
      filter = filter_lookup (access, &p, type, exact);
    }

  /* Looking up filter from access_list. */
  if (filter == NULL)
    {
      vty_out (vty, "%% access-list %s %s %s doesn't exist%s", 
	       argv[0],
	       strncmp (argv[1], "p", 1) == 0 ? "permit" : "deny",
	       strncmp (argv[2], "a", 1) == 0 ? "any" : argv[2],
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Delete filter from access_list. */
  access_list_filter_delete (access, filter);

  return CMD_SUCCESS;
}

ALIAS (no_ipv6_access_list,
       no_ipv6_access_list_exact_cmd,
       "no ipv6 access-list WORD (deny|permit) X:X::X:X/M (exact-match|)",
       NO_STR
       IPV6_STR
       "Add an access list entry\n"
       "Access-list name\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 3ffe:506::/32\n"
       "Exact match of the prefixes\n")

DEFUN (no_ipv6_access_list_all,
       no_ipv6_access_list_all_cmd,
       "no ipv6 access-list WORD",
       NO_STR
       IPV6_STR
       "Add an access list entry\n"
       "Access-list name\n")
{
  struct access_list *access;

  /* Looking up access_list. */
  access = access_list_lookup (AFI_IP6, argv[0]);
  if (access == NULL)
    {
      vty_out (vty, "%% access-list %s doesn't exist%s", argv[0],
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Delete filter from access_list. */
  access_list_delete (access);

  return CMD_SUCCESS;
}

DEFUN (ipv6_access_list_remark,
       ipv6_access_list_remark_cmd,
       "ipv6 access-list WORD remark .LINE",
       IPV6_STR
       "Add an access list entry\n"
       "Access-list name\n"
       "Access list entry comment\n"
       "Comment up to 100 characters\n")
{
  struct access_list *access;
  struct buffer *b;
  int i;

  access = access_list_get (AFI_IP6, argv[0]);

  if (access->remark)
    {
      XFREE (MTYPE_TMP, access->remark);
      access->remark = NULL;
    }

  /* Below is remark get codes. */
  b = buffer_new (1024);
  for (i = 1; i < argc; i++)
    {
      buffer_putstr (b, (u_char *)argv[i]);
      buffer_putc (b, ' ');
    }
  buffer_putc (b, '\0');

  access->remark = buffer_getstr (b);

  buffer_free (b);

  return CMD_SUCCESS;
}

DEFUN (no_ipv6_access_list_remark,
       no_ipv6_access_list_remark_cmd,
       "no ipv6 access-list WORD remark",
       NO_STR
       IPV6_STR
       "Add an access list entry\n"
       "Access-list name\n"
       "Access list entry comment\n")
{
  return vty_access_list_remark_unset (vty, AFI_IP6, argv[0]);
}
	
ALIAS (no_ipv6_access_list_remark,
       no_ipv6_access_list_remark_arg_cmd,
       "no ipv6 access-list WORD remark .LINE",
       NO_STR
       IPV6_STR
       "Add an access list entry\n"
       "Access-list name\n"
       "Access list entry comment\n"
       "Comment up to 100 characters\n")
#endif /* HAVE_IPV6 */

/* Configuration write function. */
int
config_write_access_afi (afi_t afi, struct vty *vty)
{
  struct access_list *access;
  struct access_master *master;
  struct filter *filter;
  char buf[BUFSIZ];
  struct prefix *p;
  int write = 0;

  master = access_master_get (afi);
  if (master == NULL)
    return 0;

  for (access = master->num.head; access; access = access->next)
    {
      if (access->remark)
	{
	  vty_out (vty, "%saccess-list %s remark %s%s",
		   afi == AFI_IP ? "" : "ipv6 ",
		   access->name, access->remark,
		   VTY_NEWLINE);
	  write++;
	}

      for (filter = access->head; filter; filter = filter->next)
	{
	  p = &filter->prefix;

	  if (filter->any)
	    vty_out (vty,
	  	     "%saccess-list %s %s any%s", 
		     afi == AFI_IP ? "" : "ipv6 ",
		     access->name,
		     filter_type_str (filter),
		     VTY_NEWLINE);
	  else
	    vty_out (vty,
		     "%saccess-list %s %s %s/%d%s%s", 
		     afi == AFI_IP ? "" : "ipv6 ",
		     access->name,
		     filter_type_str (filter),
		     inet_ntop (p->family, &p->u.prefix, buf, BUFSIZ),
		     p->prefixlen,
		     filter->exact ? " exact-match" : "",
		     VTY_NEWLINE);
	  write++;
	}
    }

  for (access = master->str.head; access; access = access->next)
    {
      if (access->remark)
	{
	  vty_out (vty, "%saccess-list %s remark %s%s",
	  	   afi == AFI_IP ? "" : "ipv6 ",
		   access->name, access->remark,
		   VTY_NEWLINE);
	  write++;
	}

      for (filter = access->head; filter; filter = filter->next)
	{
	  p = &filter->prefix;

	  if (filter->any)
	    vty_out (vty,
	  	     "%saccess-list %s %s any%s", 
		     afi == AFI_IP ? "" : "ipv6 ",
		     access->name,
		     filter_type_str (filter),
		     VTY_NEWLINE);
	  else
	    vty_out (vty, 
	  	     "%saccess-list %s %s %s/%d%s%s", 
		     afi == AFI_IP ? "" : "ipv6 ",
		     access->name,
		     filter_type_str (filter),
		     inet_ntop (p->family, &p->u.prefix, buf, BUFSIZ),
		     p->prefixlen,
		     filter->exact ? " exact-match" : "",
		     VTY_NEWLINE);
	  write++;
        }
    }
  return write;
}

/* Access-list node. */
struct cmd_node access_node =
{
  ACCESS_NODE,
  "",				/* Access list has no interface. */
  1
};

int
config_write_access_ipv4 (struct vty *vty)
{
  return config_write_access_afi (AFI_IP, vty);
}

void
access_list_reset_ipv4 ()
{
  struct access_list *access;
  struct access_list *next;
  struct access_master *master;

  master = access_master_get (AFI_IP);
  if (master == NULL)
    return;

  for (access = master->num.head; access; access = next)
    {
      next = access->next;
      access_list_delete (access);
    }
  for (access = master->str.head; access; access = next)
    {
      next = access->next;
      access_list_delete (access);
    }

  assert (master->num.head == NULL);
  assert (master->num.tail == NULL);

  assert (master->str.head == NULL);
  assert (master->str.tail == NULL);
}

/* Install vty related command. */
void
access_list_init_ipv4 ()
{
  install_node (&access_node, config_write_access_ipv4);

  install_element (CONFIG_NODE, &access_list_exact_cmd);
  install_element (CONFIG_NODE, &access_list_cmd);
  install_element (CONFIG_NODE, &access_list_remark_cmd);
  install_element (CONFIG_NODE, &no_access_list_exact_cmd);
  install_element (CONFIG_NODE, &no_access_list_cmd);
  install_element (CONFIG_NODE, &no_access_list_all_cmd);
  install_element (CONFIG_NODE, &no_access_list_remark_cmd);
  install_element (CONFIG_NODE, &no_access_list_remark_arg_cmd);
}

#ifdef HAVE_IPV6
struct cmd_node access_ipv6_node =
{
  ACCESS_IPV6_NODE,
  "",
  1
};

int
config_write_access_ipv6 (struct vty *vty)
{
  return config_write_access_afi (AFI_IP6, vty);
}

void
access_list_reset_ipv6 ()
{
  struct access_list *access;
  struct access_list *next;
  struct access_master *master;

  master = access_master_get (AFI_IP6);
  if (master == NULL)
    return;

  for (access = master->num.head; access; access = next)
    {
      next = access->next;
      access_list_delete (access);
    }
  for (access = master->str.head; access; access = next)
    {
      next = access->next;
      access_list_delete (access);
    }

  assert (master->num.head == NULL);
  assert (master->num.tail == NULL);

  assert (master->str.head == NULL);
  assert (master->str.tail == NULL);
}

void
access_list_init_ipv6 ()
{
  install_node (&access_ipv6_node, config_write_access_ipv6);

  install_element (CONFIG_NODE, &ipv6_access_list_exact_cmd);
  install_element (CONFIG_NODE, &ipv6_access_list_cmd);
  install_element (CONFIG_NODE, &ipv6_access_list_remark_cmd);
  install_element (CONFIG_NODE, &no_ipv6_access_list_exact_cmd);
  install_element (CONFIG_NODE, &no_ipv6_access_list_cmd);
  install_element (CONFIG_NODE, &no_ipv6_access_list_all_cmd);
  install_element (CONFIG_NODE, &no_ipv6_access_list_remark_cmd);
  install_element (CONFIG_NODE, &no_ipv6_access_list_remark_arg_cmd);
}
#endif /* HAVE_IPV6 */

void
access_list_init ()
{
  access_list_init_ipv4 ();
#ifdef HAVE_IPV6
  access_list_init_ipv6();
#endif /* HAVE_IPV6 */
}

void
access_list_reset ()
{
  access_list_reset_ipv4 ();
#ifdef HAVE_IPV6
  access_list_reset_ipv6();
#endif /* HAVE_IPV6 */
}
