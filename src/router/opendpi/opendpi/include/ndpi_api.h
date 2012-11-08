/*
 * ndpi_api.h
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


#ifndef __NDPI_API_INCLUDE_FILE__
#define __NDPI_API_INCLUDE_FILE__


#if defined(WIN32)
#include <winsock2.h>
/* Windows is little endian */ 
#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN    4321
#define __BYTE_ORDER __LITTLE_ENDIAN
#define __FLOAT_WORD_ORDER __BYTE_ORDER
#endif /*  defined(WIN32) */


#ifdef __cplusplus
extern "C" {
#endif

/* basic definitions (u_int64_t, u_int32_t, timestamp size,...) */
#include "ndpi_protocols_osdpi.h"
/* macros for protocol / bitmask conversation if needed */
#include "ndpi_macros.h"
#include "ndpi_public_functions.h"
#include "ndpi_debug_functions.h"
#ifdef __cplusplus
}
#endif
#endif							/* __NDPI_API_INCLUDE_FILE__ */
