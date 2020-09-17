// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (c) 2005 Silicon Graphics, Inc.
 * All Rights Reserved.
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
