/*
 * Copyright (c) 2000-2006 Silicon Graphics, Inc.
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
#ifndef LIBXFS_INIT_H
#define LIBXFS_INIT_H

struct stat64;

extern int platform_check_ismounted (char *path, char *block,
					struct stat64 *sptr, int verbose);
extern int platform_check_iswritable (char *path, char *block,
					struct stat64 *sptr, int fatal);
extern int platform_set_blocksize (int fd, char *path, dev_t device, int bsz, int fatal);
extern void platform_flush_device (int fd, dev_t device);
extern char *platform_findrawpath(char *path);
extern char *platform_findrawpath (char *path);
extern char *platform_findblockpath (char *path);
extern int platform_direct_blockdev (void);
extern int platform_align_blockdev (void);
extern int platform_nproc(void);
extern unsigned long platform_physmem(void);	/* in kilobytes */
extern int platform_has_uuid;

#endif	/* LIBXFS_INIT_H */
