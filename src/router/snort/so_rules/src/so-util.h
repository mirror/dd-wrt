/*
 * Utility functions shared by custom rules
 * 
 * Copyright (C) 2007 Sourcefire, Inc. All Rights Reserved
 * 
 * Writen by Patrick Mullen <pmullen@sourcefire.com>
 *           Monica Sojeong Hong <shong@sourcefire.com>
 *
 * This file may contain proprietary rules that were created, tested and
 * certified by Sourcefire, Inc. (the "VRT Certified Rules") as well as
 * rules that were created by Sourcefire and other third parties and
 * distributed under the GNU General Public License (the "GPL Rules").  The
 * VRT Certified Rules contained in this file are the property of
 * Sourcefire, Inc. Copyright 2005 Sourcefire, Inc. All Rights Reserved.
 * The GPL Rules created by Sourcefire, Inc. are the property of
 * Sourcefire, Inc. Copyright 2002-2005 Sourcefire, Inc. All Rights
 * Reserved.  All other GPL Rules are owned and copyrighted by their
 * respective owners (please see www.snort.org/contributors for a list of
 * owners and their respective copyrights).  In order to determine what
 * rules are VRT Certified Rules or GPL Rules, please refer to the VRT
 * Certified Rules License Agreement.
 */


#ifndef SO_UTIL_H
#define SO_UTIL_H

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"

/*
 * Support functions to read little-endian or big-endian binary data 
 */

u_int64_t read_big_64(const u_int8_t *p);
u_int32_t read_big_32(const u_int8_t *p);
u_int16_t read_big_16(const u_int8_t *p);
u_int64_t read_little_64(const u_int8_t *p);
u_int32_t read_little_32(const u_int8_t *p);
u_int16_t read_little_16(const u_int8_t *p);

// Macros that convert the above functions to auto-incrementing versions
#define read_big_64_inc(p) read_big_64(p); p += 8
#define read_big_32_inc(p) read_big_32(p); p += 4
#define read_big_16_inc(p) read_big_16(p); p += 2
#define read_little_64_inc(p) read_little_64(p); p += 8
#define read_little_32_inc(p) read_little_32(p); p += 4
#define read_little_16_inc(p) read_little_16(p); p += 2

#endif /* SO_UTIL_H */
