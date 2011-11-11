/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2010-2011 The ProFTPD Project team
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
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 *
 * As a special exemption, The ProFTPD Project and other respective copyright
 * holders give permission to link this program with OpenSSL, and distribute
 * the resulting executable, without including the source code for OpenSSL in
 * the source distribution.
 */

/* ProFTPD symbol table hash ("stash")
 * $Id: stash.h,v 1.2 2011/05/23 20:35:35 castaglia Exp $
 */

#ifndef PR_STASH_H
#define PR_STASH_H

typedef enum {
  PR_SYM_CONF = 1,
  PR_SYM_CMD,
  PR_SYM_AUTH,
  PR_SYM_HOOK
} pr_stash_type_t;

int init_stash(void);
int pr_stash_add_symbol(pr_stash_type_t, void *);
void *pr_stash_get_symbol(pr_stash_type_t, const char *, void *, int *);
int pr_stash_remove_symbol(pr_stash_type_t, const char *, module *);

#endif /* PR_STASH_H */
