/*
 * include/asm-armnommu/arch-netarm/netarm_nvram.h
 *
 * Copyright (C) 2000 NETsilicon, Inc.
 * Copyright (C) 2000 WireSpeed Communications Corporation
 *
 * This software is copyrighted by WireSpeed. LICENSEE agrees that
 * it will not delete this copyright notice, trademarks or protective
 * notices from any copy made by LICENSEE.
 *
 * This software is provided "AS-IS" and any express or implied 
 * warranties or conditions, including but not limited to any
 * implied warranties of merchantability and fitness for a particular
 * purpose regarding this software. In no event shall WireSpeed
 * be liable for any indirect, consequential, or incidental damages,
 * loss of profits or revenue, loss of use or data, or interruption
 * of business, whether the alleged damages are labeled in contract,
 * tort, or indemnity.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * author(s) : Joe deBlaquiere
 */

#ifndef __NETARM_NON_VOLATILE_MEMORY_H
#define __NETARM_NON_VOLATILE_MEMORY_H

/* structure definitions for per-unit configuration data */

/* count determines how many dwords are used to calculate checksum */
#define	NETARM_NVRAM_DWORD_COUNT	(11)

typedef struct
{
  unsigned char serialNumber[8];
  unsigned long ipAddress;
  unsigned long subnetMask;
  unsigned long gateway;
  long		useDhcp;
  unsigned long waitTime;
  unsigned long baudrate;
  unsigned long telnetMagic;
  unsigned long reserved[1];
  unsigned long checksum;
} NA_dev_board_params_t;


/* auxiliary functions, moved here from setup.c */
static inline int
parse_decimal(char ch)
{
	if ('0' >  ch) return -1;
	if ('9' >= ch) return ch - '0' ;
	return -1;
}

static inline unsigned int
compute_checksum(NA_dev_board_params_t *pParams)
{
	int i;
	unsigned int *pInt = (unsigned int *)pParams ;
	unsigned int sum = 0;
	for ( i = 0 ; i < NETARM_NVRAM_DWORD_COUNT ; i++ )
	{
		sum += pInt[i];
	}
	
	if ( sum != 0 ) return -1 ;
	return 0 ;
}

#endif

