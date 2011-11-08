/*
 * Copyright (c) 2002-2003,2005 Silicon Graphics, Inc.
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

#ifndef MD_MAJOR
#define MD_MAJOR		9 /* we also check at runtime */
#endif

#define GET_ARRAY_INFO          _IOR (MD_MAJOR, 0x11, struct md_array_info)

#define MD_SB_CLEAN		0
#define MD_SB_ERRORS		1

struct md_array_info {
	/*
	 * Generic constant information
	 */
	__uint32_t major_version;
	__uint32_t minor_version;
	__uint32_t patch_version;
	__uint32_t ctime;
	__uint32_t level;
	__uint32_t size;
	__uint32_t nr_disks;
	__uint32_t raid_disks;
	__uint32_t md_minor;
	__uint32_t not_persistent;

	/*
	 * Generic state information
	 */
	__uint32_t utime;	  /*  0 Superblock update time		  */
	__uint32_t state;	  /*  1 State bits (clean, ...)		  */
	__uint32_t active_disks;  /*  2 Number of currently active disks  */
	__uint32_t working_disks; /*  3 Number of working disks		  */
	__uint32_t failed_disks;  /*  4 Number of failed disks		  */
	__uint32_t spare_disks;	  /*  5 Number of spare disks		  */

	/*
	 * Personality information
	 */
	__uint32_t layout;	  /*  0 the array's physical layout	  */
	__uint32_t chunk_size;	  /*  1 chunk size in bytes		  */

};

/*
 * MDP = partitionable RAID arrays
 */
enum md_type {
	MD_TYPE_MD,
	MD_TYPE_MDP
};
