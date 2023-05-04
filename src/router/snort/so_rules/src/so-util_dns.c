/*
 * DNS support functions
 * 
 * Copyright (C) 2013 Sourcefire, Inc. All Rights Reserved
 * 
 * Written by Patrick Mullen <pmullen@sourcefire.com> 
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

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"

#include "so-util_dns.h"

//#define DEBUG
#ifdef DEBUG
#define DEBUG_SO(code) code
#else
#define DEBUG_SO(code)
#endif

/* dns_skip_name

   Given pointers to a cursor and the end of the payload, will process a dns name
   for the sole purpose of skipping over it.  This means it doesn't actually follow
   name compression; it just jumps to the end of it all and modifies the cursor to
   point to the byte after the dns name.
*/
int dns_skip_name(const uint8_t **cursor_in, const uint8_t *end_of_payload) {
   const uint8_t *cursor_raw = *cursor_in;

   while(cursor_raw < end_of_payload && *cursor_raw != 0 && !((*cursor_raw & 0xc0) == 0xc0))
      cursor_raw += *cursor_raw + 1;

   if(cursor_raw >= end_of_payload)
      return(DNS_FAIL);

   // two bytes for pointer or null byte
   cursor_raw += ((*cursor_raw & 0xc0) == 0xc0) ? 2 : 1;

   *cursor_in = cursor_raw;
   return(DNS_SUCCESS);
}

