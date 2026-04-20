/*
 * Copyright (c) 2014 - 2015, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * rfs.h
 *	Receiving Flow Streering
 */

#ifndef __RFS_H
#define __RFS_H
#define DBG_LVL_ERROR    0
#define DBG_LVL_WARN     1
#define DBG_LVL_INFO     2
#define DBG_LVL_DEBUG    3
#define DBG_LVL_TRACE    4
#define DBG_LVL_DEFAULT  DBG_LVL_INFO

extern int rfs_dbg_level;
extern struct proc_dir_entry *rfs_proc_entry;

#define __DBG_FUN(xyz, fmt, ...) \
	do { \
		if (DBG_LVL_##xyz <= rfs_dbg_level) { \
			printk("%s[%u]:"#xyz":"fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);\
		} \
	} while(0)

#define RFS_ERROR(fmt, ...) __DBG_FUN(ERROR, fmt, ##__VA_ARGS__)
#define RFS_WARN(fmt, ...)  __DBG_FUN(WARN, fmt, ##__VA_ARGS__)
#define RFS_INFO(fmt, ...)  __DBG_FUN(INFO, fmt, ##__VA_ARGS__)
#define RFS_DEBUG(fmt, ...) __DBG_FUN(DEBUG, fmt, ##__VA_ARGS__)
#define RFS_TRACE(fmt, ...) __DBG_FUN(TRACE, fmt, ##__VA_ARGS__)
#endif
