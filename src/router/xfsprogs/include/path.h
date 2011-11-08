/*
 * Copyright (c) 2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef __PATH_H__
#define __PATH_H__

#include <xfs/xfs.h>

/*
 * XFS Filesystem Paths
 *
 * Utility routines for iterating and searching through the list
 * of known mounted filesystems and project paths.
 */

#define FS_MOUNT_POINT	(1<<0)
#define FS_PROJECT_PATH	(1<<1)

typedef struct fs_path {
	char		*fs_name;	/* Data device for filesystem 	*/
	dev_t		fs_datadev;
	char		*fs_log;	/* External log device, if any	*/
	dev_t		fs_logdev;
	char		*fs_rt;		/* Realtime device, if any	*/
	dev_t		fs_rtdev;
	char		*fs_dir;	/* Directory / mount point	*/
	uint		fs_flags;	/* FS_{MOUNT_POINT,PROJECT_PATH}*/
	uint		fs_prid;	/* Project ID for tree root	*/
} fs_path_t;

extern int fs_count;		/* number of entries in fs table */
extern fs_path_t *fs_table;	/* array of entries in fs table  */
extern fs_path_t *fs_path;	/* current entry in the fs table */
extern char *mtab_file;

extern void fs_table_initialise(int, char *[], int, char *[]);
extern void fs_table_destroy(void);

extern void fs_table_insert_project_path(char *__dir, uint __projid);


extern fs_path_t *fs_table_lookup(const char *__dir, uint __flags);

typedef struct fs_cursor {
	uint		count;		/* total count of mount entries	*/
	uint		index;		/* current position in table	*/
	uint		flags;		/* iterator flags: mounts/trees */
	fs_path_t	*table;		/* local/global table pointer	*/
	fs_path_t	local;		/* space for single-entry table	*/
} fs_cursor_t;

extern void fs_cursor_initialise(char *__dir, uint __flags, fs_cursor_t *__cp);
extern fs_path_t *fs_cursor_next_entry(fs_cursor_t *__cp);

#endif	/* __PATH_H__ */
