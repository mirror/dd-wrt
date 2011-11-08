/*
 * Copyright (c) 2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef __PARENT_H__
#define	__PARENT_H__

typedef struct parent {
	__u64	p_ino;
	__u32	p_gen;
	__u16	p_reclen;
	char	p_name[1];
} parent_t;

typedef struct parent_cursor {
	__u32	opaque[4];      /* an opaque cookie */
} parent_cursor_t;

#endif
