/*
 * ipq_basic_def.h
 * Copyright (C) 2009-2011 by ipoque GmbH
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


#ifndef __IPOQUE_API_INCLUDE_FILE__
#error CANNOT INCLUDE THIS .H FILE, INCLUDE IPQ_API.H
#endif

#ifndef __IPQ_BASIC_DEF_H__
#define __IPQ_BASIC_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

/* define u64, u32, here
 * this is used for compatibility restrictions on different platforms
 *
 */

#ifndef u64
#define u64 	unsigned long long
#endif
#ifndef u32
#define u32 	unsigned int
#endif
#ifndef u16
#define u16 	unsigned short
#endif
#ifndef u8
#define u8 	unsigned char
#endif


/* generic timestamp counter size */
#define IPOQUE_TIMESTAMP_COUNTER_SIZE		u32
#ifdef __cplusplus
}
#endif
#endif
