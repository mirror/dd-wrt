/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
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
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#ifndef _P2PD_PACKETHISTORY_H
#define _P2PD_PACKETHISTORY_H

/* -------------------------------------------------------------------------
 * File       : PacketHistory.h
 * Description: Functions for keeping and accessing the history of processed
 *              multicast IP packets.
 * Created    : 29 Jun 2006
 *
 * ------------------------------------------------------------------------- */

/* System includes */
#include <sys/types.h> /* ssize_t */
#include <sys/times.h> /* clock_t */

#define N_HASH_BITS 15
#define HISTORY_HASH_SIZE (1u << N_HASH_BITS)

/* Time-out of duplicate entries, in milliseconds */
#define HISTORY_HOLD_TIME 3000

struct TDupEntry
{
  u_int32_t crc32;
  clock_t timeOut;
  struct TDupEntry* next;
};

void InitPacketHistory(void);
u_int32_t PacketCrc32(unsigned char* ipPkt, ssize_t len);
u_int32_t Hash(u_int32_t from32);
void MarkRecentPacket(u_int32_t crc32);
int CheckAndMarkRecentPacket(u_int32_t crc32);
void PrunePacketHistory(void*);

#endif /* _P2PD_PACKETHISTORY_H */
