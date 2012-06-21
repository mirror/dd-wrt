/*
 * ipq_utils.h
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



#ifndef _IPQ_UTILS_H_
#define _IPQ_UTILS_H_

#include "ipq_protocols.h"


/**
 * macro for getting the string len of a static string
 *
 * use it instead of strlen to avoid runtime calculations
 */
#define IPQ_STATICSTRING_LEN( s ) ( sizeof( s ) - 1 )



/** macro to compare 2 IPv6 addresses with each other to identify the "smaller" IPv6 address  */
#define IPOQUE_COMPARE_IPV6_ADDRESS_STRUCTS(x,y)  \
  ((((u64 *)(x))[0]) < (((u64 *)(y))[0]) || ( (((u64 *)(x))[0]) == (((u64 *)(y))[0]) && (((u64 *)(x))[1]) < (((u64 *)(y))[1])) )



#ifdef HAVE_NTOP
/* http.c */
extern char* ntop_strnstr(const char *s, const char *find, size_t slen);
#endif

#endif							/* _IPQ_UTILS_H_ */

