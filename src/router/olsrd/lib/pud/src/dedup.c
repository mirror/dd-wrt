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

#include "dedup.h"

/* Plugin includes */

/* OLSR includes */
#include "olsr.h"

/* System includes */
#include <assert.h>

/* Defines */

#define LISTSIZE(x)			((x)->entriesMaxCount) /* always valid */
#define NEWESTINDEX(x)		((x)->newestEntryIndex) /* always valid */
#define WRAPINDEX(x, i)		((i) % LISTSIZE(x)) /* always valid for i>=0 */
#define INCOMINGINDEX(x)	WRAPINDEX(x, (NEWESTINDEX(x) + LISTSIZE(x) - 1)) /* always valid */

/**
 Initialise the de-duplication list: allocate memory for the entries and
 reset fields.

 @param deDupList
 The de-duplication list
 @param maxEntries
 The maximum number of entries in the list (the number of messages that should
 be tracked)

 @return
 - false on failure
 - true otherwise
 */
bool initDeDupList(DeDupList * deDupList, unsigned long long maxEntries) {
	void * p;

	if (deDupList == NULL) {
		return false;
	}
	if (maxEntries < 1) {
		return false;
	}

	p = olsr_malloc(maxEntries * sizeof(DeDupEntry),
			"DeDupEntry entries for DeDupList (PUD)");
	if (p == NULL) {
		return false;
	}

	deDupList->entriesMaxCount = maxEntries;
	deDupList->entries = p;

	deDupList->entriesCount = 0;
	deDupList->newestEntryIndex = 0;

	return true;
}

/**
 Clean up the de-duplication list: free memory and reset fields.

 @param deDupList
 The de-duplication list
 */
void destroyDeDupList(DeDupList * deDupList) {
	assert (deDupList != NULL);

	if (deDupList->entries != NULL) {
		free(deDupList->entries);
		deDupList->entries = NULL;
	}

	deDupList->entriesMaxCount = 0;

	deDupList->entriesCount = 0;
	deDupList->newestEntryIndex = 0;
}

/**
 Add a new (incoming) message to the de-duplication list

 @param deDupList
 The de-duplication list
 @param olsrMessage
 The message
 */
void addToDeDup(DeDupList * deDupList, union olsr_message *olsrMessage) {
	unsigned long long incomingIndex;
	DeDupEntry * newEntry;

	assert (deDupList != NULL);

	incomingIndex = INCOMINGINDEX(deDupList);
	newEntry = &deDupList->entries[incomingIndex];

	memset(newEntry, 0, sizeof(DeDupEntry));
	if (olsr_cnf->ip_version == AF_INET) {
		newEntry->seqno = olsrMessage->v4.seqno;
		newEntry->originator.v4.s_addr = olsrMessage->v4.originator;
	} else {
		newEntry->seqno = olsrMessage->v6.seqno;
		newEntry->originator.v6 = olsrMessage->v6.originator;
	}

	deDupList->newestEntryIndex = incomingIndex;
	if (deDupList->entriesCount < deDupList->entriesMaxCount) {
		deDupList ->entriesCount++;
	}
}

/**
 Determines whether a new (incoming) message is already in the de-duplication
 list

 @param deDupList
 The de-duplication list
 @param olsrMessage
 The message

 @return
 - true when the message is already in the list
 - false otherwise
 */
bool isInDeDupList(DeDupList * deDupList, union olsr_message *olsrMessage) {
	bool retval = false;
	unsigned long long iteratedIndex;
	unsigned long long count;

	iteratedIndex = NEWESTINDEX(deDupList);
	count = deDupList->entriesCount;

	/* we iterate from newest until oldest: we have a higher probability to
	 * match on the newest entries */

	while (count > 0) {
		DeDupEntry * iteratedEntry = &deDupList->entries[iteratedIndex];
		if (olsr_cnf->ip_version == AF_INET) {
			if ((iteratedEntry->seqno == olsrMessage->v4.seqno) && (memcmp(
					&iteratedEntry->originator.v4, &olsrMessage->v4.originator,
					sizeof(iteratedEntry->originator.v4))) == 0) {
				retval = true;
				break;
			}
		} else {
			if ((iteratedEntry->seqno == olsrMessage->v6.seqno) && (memcmp(
					&iteratedEntry->originator.v6, &olsrMessage->v6.originator,
					sizeof(iteratedEntry->originator.v6)) == 0)) {
				retval = true;
				break;
			}
		}

		iteratedIndex = WRAPINDEX(deDupList, iteratedIndex + 1); /* go the the next older entry */
		count--;
	}

	return retval;
}
