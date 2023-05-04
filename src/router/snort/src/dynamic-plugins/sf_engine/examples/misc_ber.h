/*
 * BER support functions
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2007-2013 Sourcefire, Inc. All Rights Reserved
 *
 * Writen by Patrick Mullen <pmullen@sourcefire.com>
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

#ifndef MISC_BER_H
#define MISC_BER_H

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"


typedef struct {
   u_int8_t type;
   u_int32_t total_len;
   u_int32_t data_len;
   union {
      const u_int8_t *data_ptr;
      u_int32_t int_val;
   } data;
} BER_ELEMENT;

int ber_get_size(SFSnortPacket *, const u_int8_t *, u_int32_t *, u_int32_t *);
int ber_get_int(const u_int8_t *, u_int32_t, u_int32_t *);
int ber_get_element(SFSnortPacket *, const u_int8_t *, BER_ELEMENT *);

#endif /* MISC_BER_H */

