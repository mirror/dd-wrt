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
 * File       : PacketHistory.c
 * Description: Functions for keeping and accessing the history of processed
 *              multicast IP packets.
 * Created    : 29 Jun 2006
 *
 * ------------------------------------------------------------------------- */

#include "PacketHistory.h"

/* System includes */
#include <stddef.h> /* NULL */
#include <assert.h> /* assert() */
#include <string.h> /* memset */
#include <sys/types.h> /* u_int16_t, u_int32_t */
#include <netinet/ip.h> /* struct iphdr */

/* OLSRD includes */
#include "olsr.h" /* olsr_printf */

/* NULLPlugin includes */
#include "Packet.h"

static u_int32_t PacketHistory[HISTORY_TABLE_SIZE];

#define CRC_UPTO_NBYTES 256

#if 0
/* -------------------------------------------------------------------------
 * Function   : CalcCrcCcitt
 * Description: Calculate 16-bits CRC according to CRC-CCITT specification
 * Input      : buffer - the bytes to calculate the CRC value over
 *              len - the number of bytes to calculate the CRC value over
 * Output     : none
 * Return     : CRC-16 value
 * Data Used  : none
 * ------------------------------------------------------------------------- */
static u_int16_t CalcCrcCcitt(unsigned char* buffer, ssize_t len)
{
  /* Initial value of 0xFFFF should be 0x1D0F according to
   * www.joegeluso.com/software/articles/ccitt.htm */
  u_int16_t crc = 0xFFFF; 
  int i;

  assert(buffer != NULL);

  for (i = 0; i < len; i++)
  {
    crc  = (unsigned char)(crc >> 8) | (crc << 8);
    crc ^= buffer[i];
    crc ^= (unsigned char)(crc & 0xff) >> 4;
    crc ^= (crc << 8) << 4;
    crc ^= ((crc & 0xff) << 4) << 1;
  }
  return crc;
}
#endif

/* -------------------------------------------------------------------------
 * Function   : GenerateCrc32Table
 * Description: Generate the table of CRC remainders for all possible bytes,
 *              according to CRC-32-IEEE 802.3
 * Input      : none
 * Output     : none
 * Return     : none
 * Data Used  : none
 * ------------------------------------------------------------------------- */
#define CRC32_POLYNOMIAL 0xedb88320UL /* bit-inverse of 0x04c11db7UL */

static unsigned long CrcTable[256];

static void GenerateCrc32Table(void)
{
  int i, j;
  u_int32_t crc;
  for (i = 0; i < 256; i++)
  {
    crc = (u_int32_t) i;
    for (j = 0; j < 8; j++)
    {
      if (crc & 1)
      {
        crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
      }
      else
      {
        crc = (crc >> 1);
      }
    }
    CrcTable[i] = crc;
  }
}

/* -------------------------------------------------------------------------
 * Function   : CalcCrc32
 * Description: Calculate CRC-32 according to CRC-32-IEEE 802.3
 * Input      : buffer - the bytes to calculate the CRC value over
 *              len - the number of bytes to calculate the CRC value over
 * Output     : none
 * Return     : CRC-32 value
 * Data Used  : none
 * ------------------------------------------------------------------------- */
static u_int32_t CalcCrc32(unsigned char* buffer, ssize_t len)
{
  int i, j;
  u_int32_t crc = 0xffffffffUL;
  for (i = 0; i < len; i++)
  {
    j = ((int) (crc & 0xFF) ^ *buffer++);
    crc = (crc >> 8) ^ CrcTable[j];
  }
  return crc ^ 0xffffffffUL;
}

/* -------------------------------------------------------------------------
 * Function   : PacketCrc32
 * Description: Calculates the CRC-32 value for an Ethernet frame
 * Input      : ethernetFrame - the Ethernet frame
 *              len - the number of octets in the Ethernet frame
 * Output     : none
 * Return     : 32-bits hash value
 * Data Used  : none
 * Notes      : The source and destination MAC address are not taken into account
 *              in the CRC calculation.
 * ------------------------------------------------------------------------- */
u_int32_t PacketCrc32(unsigned char* ethernetFrame, ssize_t len)
{
  ssize_t nCrcBytes;
  struct TSaveTtl sttl;
  struct ip* ipHeader;
  u_int32_t result;

  assert(ethernetFrame != NULL);

  /* Start CRC calculation at ethertype; skip source and destination MAC 
   * addresses, and ethertype.
   *
   * Also skip TTL: in a multi-homed OLSR-network, the same multicast packet
   * may enter the network multiple times, each copy differing only in its
   * TTL value. BMF must not calculate a different CRC for packets that
   * differ only in TTL. Skip also the IP-header checksum, because it changes
   * along with TTL. Besides, it is not a good idea to calculate a CRC over
   * data that already contains a checksum.
   *
   * Clip number of bytes over which CRC is calculated to prevent
   * long packets from possibly claiming too much CPU resources. */
  nCrcBytes = len - IP_HDR_OFFSET;
  assert(nCrcBytes > 0);
  if (nCrcBytes > CRC_UPTO_NBYTES)
  {
    nCrcBytes = CRC_UPTO_NBYTES;
  }

  SaveTtlAndChecksum(GetIpPacket(ethernetFrame), &sttl);

  ipHeader = GetIpHeader(ethernetFrame);
  ipHeader->ip_ttl = 0xFF; /* fixed value of TTL for CRC-32 calculation */
  ipHeader->ip_sum = 0x5A5A; /* fixed value of IP header checksum for CRC-32 calculation */

  result = CalcCrc32(ethernetFrame + IP_HDR_OFFSET, nCrcBytes);

  RestoreTtlAndChecksum(GetIpPacket(ethernetFrame), &sttl);
  return result;
}

/* Calculates a 16-bit hash value from a 32-bit hash value */
u_int16_t Hash16(u_int32_t hash32)
{
  return ((hash32 >> 16) + hash32) & 0xFFFFU;
}

/* -------------------------------------------------------------------------
 * Function   : InitPacketHistory
 * Description: Initialize the packet history table and CRC-32 table
 * Input      : none
 * Output     : none
 * Return     : none
 * Data Used  : PacketHistory
 * ------------------------------------------------------------------------- */
void InitPacketHistory(void)
{
  memset(PacketHistory, 0, sizeof(PacketHistory));
  GenerateCrc32Table();
}

/* -------------------------------------------------------------------------
 * Function   : MarkRecentPacket
 * Description: Record the fact that this packet was seen recently
 * Input      : hash16
 * Output     : none
 * Return     : none
 * Data Used  : PacketHistory
 * ------------------------------------------------------------------------- */
void MarkRecentPacket(u_int16_t hash16)
{
  u_int32_t index;
  uint offset;

  index = hash16 / NPACKETS_PER_ENTRY;
  assert(index < HISTORY_TABLE_SIZE);

  offset = (hash16 % NPACKETS_PER_ENTRY) * NBITS_PER_PACKET;
  assert(offset <= NBITS_IN_UINT32 - NBITS_PER_PACKET);

  /* Mark as "seen recently" */
  PacketHistory[index] = PacketHistory[index] | (0x3u << offset);
}

/* -------------------------------------------------------------------------
 * Function   : CheckAndMarkRecentPacket
 * Description: Check if this packet was seen recently, then record the fact
 *              that this packet was seen recently.
 * Input      : hash16
 * Output     : none
 * Return     : not recently seen (0), recently seen (1)
 * Data Used  : PacketHistory
 * ------------------------------------------------------------------------- */
int CheckAndMarkRecentPacket(u_int16_t hash16)
{
  u_int32_t index;
  uint offset;
  u_int32_t bitMask;
  int result;

  index = hash16 / NPACKETS_PER_ENTRY;
  assert(index < HISTORY_TABLE_SIZE);

  offset =  (hash16 % NPACKETS_PER_ENTRY) * NBITS_PER_PACKET;
  assert(offset <= NBITS_IN_UINT32 - NBITS_PER_PACKET);

  bitMask = 0x1u << offset;
  result = ((PacketHistory[index] & bitMask) == bitMask);
  
  /* Always mark as "seen recently" */
  PacketHistory[index] = PacketHistory[index] | (0x3u << offset);

  return result;
}
  
/* -------------------------------------------------------------------------
 * Function   : PrunePacketHistory
 * Description: Prune the packet history table.
 * Input      : useless - not used
 * Output     : none
 * Return     : none
 * Data Used  : PacketHistory
 * ------------------------------------------------------------------------- */
void PrunePacketHistory(void* useless __attribute__((unused)))
{
  uint i;
  for (i = 0; i < HISTORY_TABLE_SIZE; i++)
  {
    if (PacketHistory[i] > 0)
    {
      uint j;
      for (j = 0; j < NPACKETS_PER_ENTRY; j++)
      {
        uint offset = j * NBITS_PER_PACKET;

        u_int32_t bitMask = 0x3u << offset;
        u_int32_t bitsSeenRecenty = 0x3u << offset;
        u_int32_t bitsTimingOut = 0x1u << offset;

        /* 10 should never occur */
        assert ((PacketHistory[i] & bitMask) != (0x2u << offset));
        
        if ((PacketHistory[i] & bitMask) == bitsSeenRecenty)
        {
          /* 11 -> 01 */
          PacketHistory[i] &= ~bitMask | bitsTimingOut;
        }
        else if ((PacketHistory[i] & bitMask) == bitsTimingOut)
        {
          /* 01 -> 00 */
          PacketHistory[i] &= ~bitMask;
        }
      } /* for (j = ...) */
    } /* if (PacketHistory[i] > 0) */
  } /* for (i = ...) */
}
