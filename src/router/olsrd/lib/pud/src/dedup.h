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
