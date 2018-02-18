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

#ifndef _PUD_DEDUP_H_
#define _PUD_DEDUP_H_

/* Plugin includes */

/* OLSR includes */
#include "olsr_types.h"
#include "olsr_protocol.h"

/* System includes */
#include <stdint.h>
#include <stdbool.h>

/* A de-duplication entry holding the information to compare, 18 bytes */
typedef struct _DeDupEntry {
		uint16_t seqno;
		union olsr_ip_addr originator;
} DeDupEntry;

/**
 A list of de-duplication entries that are used to determine whether a received
 OLSR message was already seen.

 The list is a circular list.
 */
typedef struct _DeDupList {
	unsigned long long entriesMaxCount; /**< the maximum number of entries in the list */
	DeDupEntry * entries; /**< the list entries */

	unsigned long long entriesCount; /**< the number of entries in the list */
	unsigned long long newestEntryIndex; /**< index of the newest entry in the list (zero-based) */
} DeDupList;

bool initDeDupList(DeDupList * deDupList, unsigned long long maxEntries);
void destroyDeDupList(DeDupList * deDupList);

void addToDeDup(DeDupList * deDupList, union olsr_message *olsrMessage);

bool isInDeDupList(DeDupList * deDupList, union olsr_message *olsrMessage);

#endif /* _PUD_DEDUP_H_ */
