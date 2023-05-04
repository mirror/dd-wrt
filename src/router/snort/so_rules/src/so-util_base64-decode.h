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


#ifndef SO_UTIL_BASE64_DECODE_H
#define SO_UTIL_BASE64_DECODE_H

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"

/*
 * Base64 Decoding Routines
 */

int unfold_header(const u_int8_t*, u_int32_t, u_int8_t*, u_int32_t, u_int32_t*);
int base64decode(const u_int8_t*, u_int32_t, u_int8_t*, u_int32_t, u_int32_t*); 

#endif /* SO_UTIL_BASE64_DECODE_H */
