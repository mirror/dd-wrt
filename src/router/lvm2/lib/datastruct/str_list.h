/*
 * Copyright (C) 2003-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2012 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _LVM_STR_LIST_H
#define _LVM_STR_LIST_H

struct dm_list;
struct dm_pool;

struct dm_list *str_list_create(struct dm_pool *mem);
int str_list_add(struct dm_pool *mem, struct dm_list *sll, const char *str);
int str_list_add_list(struct dm_pool *mem, struct dm_list *sll, struct dm_list *sll2);
int str_list_add_no_dup_check(struct dm_pool *mem, struct dm_list *sll, const char *str);
int str_list_add_h_no_dup_check(struct dm_pool *mem, struct dm_list *sll, const char *str);
void str_list_del(struct dm_list *sll, const char *str);
void str_list_wipe(struct dm_list *sll);
int str_list_match_item(const struct dm_list *sll, const char *str);
int str_list_match_list(const struct dm_list *sll, const struct dm_list *sll2, const char **tag_matched);
int str_list_lists_equal(const struct dm_list *sll, const struct dm_list *sll2);
int str_list_dup(struct dm_pool *mem, struct dm_list *sllnew,
		 const struct dm_list *sllold);
char *str_list_to_str(struct dm_pool *mem, const struct dm_list *list, const char *delim);
struct dm_list *str_to_str_list(struct dm_pool *mem, const char *str, const char *delim, int ignore_multiple_delim);

#endif
