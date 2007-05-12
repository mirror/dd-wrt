#ifndef _BMF_PACKETHISTORY_H
#define _BMF_PACKETHISTORY_H

/*
 * OLSR Basic Multicast Forwarding (BMF) plugin.
 * Copyright (c) 2005, 2006, Thales Communications, Huizen, The Netherlands.
 * Written by Erik Tromp.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 *
 * * Redistributions of source code must retain the above copyright 
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimer in 
 *   the documentation and/or other materials provided with the 
 *   distribution.
 * * Neither the name of Thales, BMF nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* -------------------------------------------------------------------------
 * File       : PacketHistory.h
 * Description: Functions for keeping and accessing the history of processed
 *              multicast IP packets.
 * Created    : 29 Jun 2006
 *
 * ------------------------------------------------------------------------- */

#include <sys/types.h> /* ssize_t */

/* 2 bits per seen packet: 
 * 11 = "seen recently",
 * 01 = "timing out"
 * 00 = "not seen recently"
 * Note that 10 is unused */
#define NBITS_PER_PACKET 2
#define NBITS_IN_UINT16 (sizeof(u_int16_t) * 8)
#define NBITS_IN_UINT32 (sizeof(u_int32_t) * 8)
#define NPACKETS_PER_ENTRY (NBITS_IN_UINT32 / NBITS_PER_PACKET)
#define HISTORY_TABLE_SIZE ((1 << NBITS_IN_UINT16) / NPACKETS_PER_ENTRY)

void InitPacketHistory(void);
u_int32_t PacketCrc32(unsigned char* ethPkt, ssize_t len);
u_int16_t Hash16(u_int32_t hash32);
void MarkRecentPacket(u_int16_t hash16);
int CheckAndMarkRecentPacket(u_int16_t hash16);
void PrunePacketHistory(void*);

#endif /* _BMF_PACKETHISTORY_H */
