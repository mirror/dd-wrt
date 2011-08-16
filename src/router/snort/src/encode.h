/* $Id$ */
/****************************************************************************
 *
 * Copyright (C) 2005-2011 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/

#ifndef __ENCODE_H__
#define __ENCODE_H__

#include "decode.h"

void Encode_Init(void);
void Encode_Term(void);

typedef enum {
    ENC_TCP_FIN, ENC_TCP_RST,
    ENC_UNR_NET, ENC_UNR_HOST, ENC_UNR_PORT,
    ENC_MAX
} EncodeType;

#define ENC_FLAG_FWD 0x80000000  // send in forward direction
#define ENC_FLAG_SEQ 0x40000000  // VAL bits contain seq adj
#define ENC_FLAG_ID  0x20000000  // use randomized IP ID
#define ENC_FLAG_NET 0x10000000  // stop after innermost network (ip4/6) layer
#define ENC_FLAG_DEF 0x08000000  // stop before innermost ip4 opts or ip6 frag header
#define ENC_FLAG_RAW 0x04000000  // stop before innermost ip4 opts or ip6 frag header
#define ENC_FLAG_RES 0x03000000  // bits reserved for future use
#define ENC_FLAG_VAL 0x00FFFFFF  // bits for adjusting seq and/or ack

typedef uint32_t EncodeFlags;

// orig must be the current packet from the interface to
//   ensure proper encoding (not the reassembled packet).
// len is number of bytes in the encoded packet upon return
//   (or 0 if the returned pointer is null).
const uint8_t* Encode_Reject(
    EncodeType, EncodeFlags, const Packet* orig, uint32_t* len);

const uint8_t* Encode_Response(
    EncodeType, EncodeFlags, const Packet* orig, uint32_t* len,
    const uint8_t* payLoad, uint32_t payLen);

// allocate a Packet for later formatting (cloning)
Packet* Encode_New(void);

// release the allocated Packet
void Encode_Delete(Packet*);

// orig is the wire pkt; clone was obtained with New()
int Encode_Format(EncodeFlags, const Packet* orig, Packet* clone);

// update length and checksum fields in layers and caplen, etc.
void Encode_Update(Packet*);

#endif // __ENCODE_H__

