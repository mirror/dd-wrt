/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.
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
#ifndef __VOLUME_H__
#define __VOLUME_H__

/*
 * Subvolume Types for all volume managers.
 *
 * There is a maximum of 255 subvolumes. 0 is reserved.
 *	Note:  SVTYPE_LOG, SVTYPE_DATA, SVTYPE_RT values matches XLV.
 *	       Do not change - Colin Ngam
 */
typedef enum sv_type_e {
	SVTYPE_ALL		=0,	 /* special: denotes all sv's */
	SVTYPE_LOG		=1,	 /* XVM Log subvol type */
	SVTYPE_DATA,			 /* XVM Data subvol type */
	SVTYPE_RT,			 /* XVM Real Time subvol type */
	SVTYPE_SWAP,			 /* swap area */
	SVTYPE_RSVD5,			 /* reserved 5 */
	SVTYPE_RSVD6,			 /* reserved 6 */
	SVTYPE_RSVD7,			 /* reserved 7 */
	SVTYPE_RSVD8,			 /* reserved 8 */
	SVTYPE_RSVD9,			 /* reserved 9 */
	SVTYPE_RSVD10,			 /* reserved 10 */
	SVTYPE_RSVD11,			 /* reserved 11 */
	SVTYPE_RSVD12,			 /* reserved 12 */
	SVTYPE_RSVD13,			 /* reserved 13 */
	SVTYPE_RSVD14,			 /* reserved 14 */
	SVTYPE_RSVD15,			 /* reserved 15 */
	SVTYPE_USER1,			 /* First User Defined Subvol Type */
	SVTYPE_LAST		=255
} sv_type_t;

extern void get_subvol_stripe_wrapper (char *, sv_type_t, int *, int *, int *);
extern int  get_driver_block_major (const char *, int);

#endif /* __VOLUME_H__ */
