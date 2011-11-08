/*
 * Copyright (c) 2000-2001, 2005 Silicon Graphics, Inc.
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
#ifndef __FSTYP_H__
#define __FSTYP_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * fstype allows the user to determine the filesystem identifier of
 * mounted or unmounted filesystems, using heuristics.
 * The filesystem type is required by mount(2) and sometimes by mount(8)
 * to mount filesystems of different types.
 */
extern char *fstype (const char * __device);

/*
 * ptabtype allows one to determine the type of partition table in
 * use on a given volume, using heuristics.
 */
extern char *pttype (const char *__device);

#ifdef __cplusplus
}
#endif

#endif	/* __FSTYP_H__ */
