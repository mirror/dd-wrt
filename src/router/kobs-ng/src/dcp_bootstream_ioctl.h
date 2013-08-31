/*
* Copyright (C) 2010 Freescale Semiconductor, Inc. All Rights Reserved.
*/

/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef DCP_BOOTSTREAM_IOCTL_H
#define DCP_BOOTSTREAM_IOCTL_H

/* remember to have included the proper _IO definition
 * file before hand.
 * For user space it's <sys/ioctl.h>
 */

#define DBS_IOCTL_BASE   'd'

typedef uint8_t stmp3xxx_dcp_bootstream_cipher_block[16];

#define DBS_ENC	_IOW(DBS_IOCTL_BASE, 0x00, stmp3xxx_dcp_bootstream_cipher_block)
#define DBS_DEC _IOW(DBS_IOCTL_BASE, 0x01, stmp3xxx_dcp_bootstream_cipher_block)

#endif
