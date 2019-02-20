/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2006 Red Hat, Inc. All rights reserved.
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

#ifndef _LVM_UUID_H
#define _LVM_UUID_H

#define ID_LEN 32

#include <sys/types.h>

struct dm_pool;

struct id {
	int8_t uuid[ID_LEN];
};

/*
 * Unique logical volume identifier
 * With format1 this is VG uuid + LV uuid + '\0' + padding
 */
union lvid {
	struct id id[2];
	char s[2 * sizeof(struct id) + 1 + 7];
};

int lvid_from_lvnum(union lvid *lvid, struct id *vgid, uint32_t lv_num);
int lvnum_from_lvid(union lvid *lvid);
int lvid_in_restricted_range(union lvid *lvid);

void uuid_from_num(char *uuid, uint32_t num);

int lvid_create(union lvid *lvid, struct id *vgid);
int id_create(struct id *id);
int id_valid(struct id *id);
int id_equal(const struct id *lhs, const struct id *rhs);

/*
 * Fills 'buffer' with a more human readable form
 * of the uuid.
 */
int id_write_format(const struct id *id, char *buffer, size_t size);

/*
 * Reads a formatted uuid.
 */
int id_read_format(struct id *id, const char *buffer);
/*
 * Tries to read a formatted uuid without logging error for invalid ID
 */
int id_read_format_try(struct id *id, const char *buffer);

char *id_format_and_copy(struct dm_pool *mem, const struct id *id);

#endif
