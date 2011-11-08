/*
 *   Copyright (c) International Business Machines  Corp., 2002
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#define EVMS_MAJOR			117
#define EVMS_GET_VOL_STRIPE_INFO	\
		_IOR(EVMS_MAJOR, 0xF0, struct evms_vol_stripe_info_s)

/*
 * struct evms_vol_stripe_info_s - contains stripe information for a volume
 *
 * unit: the stripe unit specified in 512 byte block units
 * width: the number of stripe members or RAID data disks
 */
typedef struct evms_vol_stripe_info_s {
	u_int32_t	size;
	u_int32_t	width;
} evms_vol_stripe_info_t;
