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
#include <stdlib.h> /* free() */

/* OLSRD includes */
#include "olsr.h" /* olsr_printf */
#include "scheduler.h" /* GET_TIMESTAMP, TIMED_OUT */

/* Plugin includes */
#include "Packet.h"

static struct TDupEntry* PacketHistory[HISTORY_HASH_SIZE];

#define CRC_UPTO_NBYTES 256

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
  } /* for */
} /* GenerateCrc32Table */

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
} /* CalcCrc32 */

/* -------------------------------------------------------------------------
 * Function   : PacketCrc32
 * Description: Calculates the CRC-32 value for an IP packet
 * Input      : ipPacket - the IP packet
 *              len - the number of octets in the IP packet
 * Output     : none
 * Return     : 32-bits CRC value
 * Data Used  : none
 * ------------------------------------------------------------------------- */
u_int32_t PacketCrc32(unsigned char* ipPacket, ssize_t len)
{
  struct TSaveTtl sttl;
  struct ip* ipHeader;
  u_int32_t result;

  assert(ipPacket != NULL);

  /* Skip TTL: in a multi-homed OLSR-network, the same multicast packet
   * may enter the network multiple times, each copy differing only in its
   * TTL value. BMF must not calculate a different CRC for packets that
   * differ only in TTL. Skip also the IP-header checksum, because it changes
   * along with TTL. Besides, it is not a good idea to calculate a CRC over
   * data that already contains a checksum.
   *
   * Clip number of bytes over which CRC is calculated to prevent
   * long packets from possibly claiming too much CPU resources. */
  assert(len > 0);
  if (len > CRC_UPTO_NBYTES)
  {
    len = CRC_UPTO_NBYTES;
  }

  SaveTtlAndChecksum(ipPacket, &sttl);

  ipHeader = (struct ip*) ARM_NOWARN_ALIGN(ipPacket);
  ipHeader->ip_ttl = 0xFF; /* fixed value of TTL for CRC-32 calculation */
  ipHeader->ip_sum = 0x5A5A; /* fixed value of IP header checksum for CRC-32 calculation */

  result = CalcCrc32(ipPacket, len);

  RestoreTtlAndChecksum(ipPacket, &sttl);
  return result;
} /* PacketCrc32 */

/* -------------------------------------------------------------------------
 * Function   : Hash
 * Description: Calculates a hash value from a 32-bit value
 * Input      : from32 - 32-bit value
 * Output     : none
 * Return     : hash value
 * Data Used  : none
 * ------------------------------------------------------------------------- */
u_int32_t Hash(u_int32_t from32)
{
  return ((from32 >> N_HASH_BITS) + from32) & ((1u << N_HASH_BITS) - 1);
} /* Hash */

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
  unsigned int i;

  GenerateCrc32Table();

  for(i = 0; i < HISTORY_HASH_SIZE; i++)
  {
    PacketHistory[i] = NULL;
  }
} /* InitPacketHistory */

/* -------------------------------------------------------------------------
 * Function   : CheckAndMarkRecentPacket
 * Description: Check if this packet was seen recently, then record the fact
 *              that this packet was seen recently.
 * Input      : crc32 - 32-bits crc value of the packet
 * Output     : none
 * Return     : not recently seen (0), recently seen (1)
 * Data Used  : PacketHistory
 * ------------------------------------------------------------------------- */
int CheckAndMarkRecentPacket(u_int32_t crc32)
{
  u_int32_t idx;
  struct TDupEntry* walker;
  struct TDupEntry* newEntry;

  idx = Hash(crc32);
  assert(idx < HISTORY_HASH_SIZE);

  for (walker = PacketHistory[idx]; walker != NULL; walker = walker->next)
  {
    if (walker->crc32 == crc32)
    {
      /* Found duplicate entry */

      /* Always mark as "seen recently": refresh time-out */
      walker->timeOut = olsr_getTimestamp(HISTORY_HOLD_TIME);

      return 1;
    } /* if */
  } /* for */

  /* No duplicate entry found: create one */
  newEntry = olsr_malloc(sizeof(struct TDupEntry), "OLSRD P2PD: TDupEntry");
  if (newEntry != NULL)
  {
    newEntry->crc32 = crc32;
    newEntry->timeOut = olsr_getTimestamp(HISTORY_HOLD_TIME);

    /* Add new entry at the front of the list */
    newEntry->next = PacketHistory[idx];
    PacketHistory[idx] = newEntry;
  }

  return 0;
} /* CheckAndMarkRecentPacket */
  
/* -------------------------------------------------------------------------
 * Function   : PrunePacketHistory
 * Description: Prune the packet history table.
 * Input      : useless - not used
 * Output     : none
 * Return     : none
 * Data Used  : PacketHistory
 * ------------------------------------------------------------------------- */
void PrunePacketHistory(void* useless __attribute__ ((unused)))
{
  uint i;
  for (i = 0; i < HISTORY_HASH_SIZE; i++)
  {
    if (PacketHistory[i] != NULL)
    {
      struct TDupEntry* nextEntry = PacketHistory[i];
      struct TDupEntry* prevEntry = NULL;
      while (nextEntry != NULL) 
      {
        struct TDupEntry* entry = nextEntry;
        nextEntry = entry->next;

        if (olsr_isTimedOut(entry->timeOut))
        {
          /* De-queue */
          if (prevEntry != NULL)
          {
            prevEntry->next = entry->next;
          }
          else
          {
            PacketHistory[i] = entry->next;
          } /* if */

          /* De-allocate memory */
          free(entry); 
	      }
	      else
	      {
	        prevEntry = entry;
	      } /* if */
      } /* while */
    } /* if (PacketHistory[i] != NULL) */
  } /* for (i = ...) */
} /* PrunePacketHistory */
