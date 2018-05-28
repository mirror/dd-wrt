/*
 * BER support functions
 *
 * Copyright (C) 2007 Sourcefire, Inc. All Rights Reserved
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

#ifndef SO_UTIL_BER_H
#define SO_UTIL_BER_H

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"


typedef struct {
   u_int32_t type;                // Type of element
   u_int32_t specified_total_len; // Len of type byte + size field + data_len from packet
   u_int32_t total_len;           // The actual number of bytes available for the entire element
   u_int32_t data_len;            // Size specified in size field of element
   struct {
      const u_int8_t *data_ptr;   // Pointer to the element's data
      u_int32_t int_val;          // If int type, value of element after ber_extract_int() is called
   } data;
} BER_ELEMENT;

int ber_get_size(SFSnortPacket *, const u_int8_t *, u_int32_t *, u_int32_t *);
int ber_get_int(const u_int8_t *, u_int32_t, u_int32_t *);
int ber_get_element(SFSnortPacket *, const u_int8_t *, BER_ELEMENT *);
int ber_extract_int_val(BER_ELEMENT *);
int ber_skip_element(SFSnortPacket *, const u_int8_t **, u_int32_t);
int ber_point_to_data(SFSnortPacket *, const u_int8_t **, u_int32_t);
int ber_extract_int(SFSnortPacket *, const u_int8_t **, BER_ELEMENT *);

#define BER_SUCCESS 0
#define BER_FAIL -1         // Complete failure
#define BER_PARTIAL_DATA -2 // Data may not be useful

#ifndef BER_MACROS
#define BER_MACROS
#define BER_DATA(type) if(ber_point_to_data(sp, &cursor_normal, type) != BER_SUCCESS) return RULE_NOMATCH
#define BER_SKIP(type) if(ber_skip_element(sp, &cursor_normal, type) != BER_SUCCESS) return RULE_NOMATCH
#define BER_EXTRACT_INT(BE) if(ber_extract_int(sp, &cursor_normal, &BE) != BER_SUCCESS) return RULE_NOMATCH
#endif

#endif /* SO_UTIL_BER_H */

