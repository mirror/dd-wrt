/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2007 Red Hat, Inc. All rights reserved.
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

#ifndef _LVM_STRING_H
#define _LVM_STRING_H

#define NAME_LEN 128
#define UUID_PREFIX "LVM-"

#include <sys/types.h>

struct dm_pool;
struct pool;
struct logical_volume;

typedef enum name_error {
	NAME_VALID = 0,
	NAME_INVALID_EMPTY = -1,
	NAME_INVALID_HYPHEN = -2,
	NAME_INVALID_DOTS = -3,
	NAME_INVALID_CHARSET = -4,
	NAME_INVALID_LENGTH = -5
} name_error_t;

int emit_to_buffer(char **buffer, size_t *size, const char *fmt, ...)
  __attribute__ ((format(printf, 3, 4)));

char *build_dm_uuid(struct dm_pool *mem, const struct logical_volume *lvid,
		    const char *layer);

int validate_name(const char *n);
name_error_t validate_name_detailed(const char *n);
int validate_tag(const char *n);

void copy_systemid_chars(const char *src, char *dst);

int apply_lvname_restrictions(const char *name);
int is_component_lvname(const char *name);
int is_reserved_lvname(const char *name);

/*
 * Provided with a NULL-terminated argument list of const char *
 * substrings that might be contained within the string str, use
 * strstr() to search str for each in turn and return a pointer to the
 * first match or else NULL.
 */
char *first_substring(const char *str, ...);

#endif
