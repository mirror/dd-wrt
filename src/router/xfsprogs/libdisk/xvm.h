/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
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

#define _DIOC_(x)        (('d'<<8) | x)
#define DIOCGETVOLDEV    _DIOC_(36) /* subvolume devices */
#define DIOCGETVOLSTRIPE _DIOC_(47) /* subvolume stripe info */

#ifndef __UCLIBC__
/* Convenience types.  */
typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;

/* Fixed-size types, underlying types depend on word size and compiler.  */
typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;
#if __WORDSIZE == 64
typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;
#elif defined(__GNUC__)
__extension__ typedef signed long long int __int64_t;
__extension__ typedef unsigned long long int __uint64_t;
#endif

#endif

/*
 * Structure returned by the DIOCGETVOLDEV ioctl to list the
 * subvolume device nodes in a volume.  These are external device
 * numbers.
 */
#define XVM_GETDEV_VERS 1

typedef __uint32_t xvm_dev_t;

typedef struct {
	__uint32_t              version;
	xvm_dev_t		data_subvol_dev;

	xvm_dev_t		log_subvol_dev;
	xvm_dev_t		rt_subvol_dev;

	xvm_dev_t		sp_subvol_dev;
} xvm_getdev_t;

/*
 * Structure returned by the DIOCGETVOLSTRIPE ioctl to describe
 * the subvolume stripe units and width.
 */
#define XVM_SUBVOL_GEOMETRY_VERS  1
typedef struct xvm_subvol_stripe_s {
	__uint32_t              version;
	__uint32_t              unit_size;      /* in blocks */
	__uint32_t              width_size;     /* in blocks */
	__uint32_t		pad1;		/* padding */
	xvm_dev_t		dev;
} xvm_subvol_stripe_t;
