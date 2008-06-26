/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2003 The ProFTPD Project team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, the ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 */

/* Class definitions
 * $Id: class.h,v 1.3 2004/05/03 17:25:23 castaglia Exp $
 */

#ifndef PR_CLASS_H
#define PR_CLASS_H

#include "netacl.h"

typedef struct pr_class_t {
  pool *cls_pool;
  char *cls_name;
  unsigned int cls_satisfy;
  array_header *cls_acls;

  struct pr_class_t *cls_next;
} pr_class_t;

#define PR_CLASS_SATISFY_ANY	0
#define PR_CLASS_SATISFY_ALL	1

/* Returns the class object associated with the given name, or NULL if
 * there is no matching class object.
 */
pr_class_t *pr_class_find(const char *);

/* Iterate through the Class list, returning the next class.  Returns NULL
 * once the end of the list is reached.  If prev is NULL, the iterator
 * restarts at the beginning of the list.
 */
pr_class_t *pr_class_get(pr_class_t *prev);

/* Returns the class object for which the given address matches every rule.
 * If multiple classes exist that might match the given address, the first
 * defined class matches.
 */
pr_class_t *pr_class_match_addr(pr_netaddr_t *);

/* Start a new class object, allocated from the given pool, with the given
 * name.
 */
int pr_class_open(pool *, const char *);

/* Close the current class object.
 *
 * Note that -1 may be returned.  This can happen, for example, if the
 * current class object has no associated rules, i.e. is empty.
 */
int pr_class_close(void);

/* Add the given ACL object to the currently opened class object.
 */
int pr_class_add_acl(pr_netacl_t *);

/* Set the Satisfy flag on the currently opened class object.
 */
int pr_class_set_satisfy(int);

void init_class(void);

#endif /* PR_CLASS_H */
