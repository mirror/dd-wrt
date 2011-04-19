/*
 * ipq_protocols.h
 * Copyright (C) 2009-2010 by ipoque GmbH
 * 
 * This file is part of OpenDPI, an open source deep packet inspection
 * library based on the PACE technology by ipoque GmbH
 * 
 * OpenDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * OpenDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenDPI.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */


#ifndef __IPOQUE_PROTOCOLS_INCLUDE_FILE__
#define __IPOQUE_PROTOCOLS_INCLUDE_FILE__
#include "ipq_main.h"

#define get_u8(X,O)  (*(u8 *)(((u8 *)X) + O))

#if defined(OPENDPI_NETFILTER_MODULE)
#include <asm/unaligned.h>
#include <linux/unaligned/packed_struct.h>

#define get_u16(X, O)	__get_unaligned_cpu16(((const u8 *) (X)) + O)
#define get_u32(X, O)	__get_unaligned_cpu32(((const u8 *) (X)) + O)
#define get_u64(X, O)	__get_unaligned_cpu64(((const u8 *) (X)) + O)

#define get_l16(X, O)	get_unaligned_le16(((const u8 *) (X)) + O)
#define get_l32(X, O)	get_unaligned_le32(((const u8 *) (X)) + O)

#else

/* the get_uXX will return raw network packet bytes !! */
#define get_u16(X,O)  (*(u16 *)(((u8 *)X) + O))
#define get_u32(X,O)  (*(u32 *)(((u8 *)X) + O))
#define get_u64(X,O)  (*(u64 *)(((u8 *)X) + O))

/* new definitions to get little endian from network bytes */
#define get_ul8(X,O) get_u8(X,O)

#ifndef __BYTE_ORDER
# define __BYTE_ORDER BYTE_ORDER
# define __LITTLE_ENDIAN LITTLE_ENDIAN
# define __BIG_ENDIAN BIG_ENDIAN
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN

#define get_l16(X,O)  get_u16(X,O)
#define get_l32(X,O)  get_u32(X,O)

#elif __BYTE_ORDER == __BIG_ENDIAN

/* convert the bytes from big to little endian */
# define get_l16(X,O) bswap_16(get_u16(X,O))
# define get_l32(X,O) bswap_32(get_u32(X,O))

#else

#error "__BYTE_ORDER MUST BE DEFINED !"

#endif							/* __BYTE_ORDER */

#endif /* OPENDPI_NETFILTER_MODULE */


/* define memory callback function */
#define ipq_mem_cmp memcmp
#define ipq_mem_cpy memcpy

bool get_next_line(const u8 **p, const u8 *end, const u8 **line, int *len);

#endif							/* __IPOQUE_PROTOCOLS_INCLUDE_FILE__ */
