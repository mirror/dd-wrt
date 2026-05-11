/*
 * mmcimage.h
 *
 * Copyright (C) 2026 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */


#ifndef MMCIMAGE_H
#define MMCIMAGE_H

#pragma pack(push)
#pragma pack(1)

typedef struct FWPART {
	char name[32]; // name of partition. must be 0 terminated
	unsigned long long partsize; // partition size
} fwpart;

typedef struct FWHEADER {
	char devname[64]; // devicename. must be 0 terminated
	unsigned int flags; // flags (not used yet);
	unsigned char partnum; // number of partitions;
	fwpart partitions[2];
} fwheader;

#pragma pack(pop)

#endif