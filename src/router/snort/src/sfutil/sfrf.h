/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2009-2013 Sourcefire, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/

#ifndef _SFRF_H_
#define _SFRF_H_
/* @file  sfrf.h
 * @brief rate filter implementation for Snort
 * @ingroup rate_filter
 * @author Dilbagh Chahal
 */
/* @defgroup rate_filter sourcefire.rate_filter
 * Implements rate_filter feature for snort
 * @{
 */

#include "ipv6_port.h"
#include "parser/IpAddrSet.h"

#include "sflsq.h"
#include "sfghash.h"
#include "sfxhash.h"
#include "sfPolicy.h"

// define to use over rate threshold
#define SFRF_OVER_RATE

// used for the dimensions of the gid lookup array.
#define SFRF_MAX_GENID 8129

// rate_filter tracking by src, by dst, or by rule
typedef enum
{
    SFRF_TRACK_BY_SRC = 1,
    SFRF_TRACK_BY_DST,
    SFRF_TRACK_BY_RULE,
    SFRF_TRACK_BY_MAX
} SFRF_TRACK;

/* Type of operation for threshold tracking nodes.
 */
typedef enum
{
    SFRF_COUNT_NOP,
    SFRF_COUNT_RESET,
    SFRF_COUNT_INCREMENT,
    SFRF_COUNT_DECREMENT,
    SFRF_COUNT_MAX
} SFRF_COUNT_OPERATION;

typedef enum {
    FS_NEW = 0, FS_OFF, FS_ON, FS_MAX
} FilterState;

/* A threshold configuration object, created for each configured rate_filter.
 * These are created at initialization, and remain static.
 */
typedef struct
{
    // Internally generated unique threshold identity
    int      tid;

    // Generator id from configured threshold
    unsigned gid;

    // Signature id from configured threshold
    unsigned sid;

    // Signature id from configured threshold
    tSfPolicyId policyId;

    // Threshold tracking by src, dst or rule
    SFRF_TRACK tracking;

    // Number of rule matching before rate limit is reached.
    unsigned count;

    // Duration in seconds for determining rate of rule matching
    unsigned seconds;

    // Action that replaces original rule action on reaching threshold
    RuleType newAction;

    // Threshold action duration in seconds before reverting to original rule action
    unsigned timeout;

    // ip set to restrict rate_filter
    IpAddrSet* applyTo;

} tSFRFConfigNode;

/* tSFRFSidNode acts as a container of gid+sid based threshold objects,
 * this allows multiple threshold objects to be applied to a single
 * gid+sid pair. This is static data elements, built at initialization.
 */
typedef struct
{
    // List of threshold configuration nodes of type tSFRFConfigNode
    tSfPolicyId policyId;

    // Generator id from configured threshold
    unsigned gid;

    // Signature id from configured threshold
    unsigned sid;

    // List of threshold configuration nodes of type tSFRFConfigNode
    SF_LIST* configNodeList;

} tSFRFSidNode;

typedef struct
{
    ///policy identifier
    tSfPolicyId policyId;

    // Signature id from configured threshold
    unsigned sid;

} tSFRFGenHashKey;


/* Single global context containing rate_filter configuration nodes.
 */
typedef struct _RateFilterConfig
{
    /* Array of hash, indexed by gid. Each array element is a hash, which
     * is keyed on sid/policyId and data is a tSFRFSidNode node.
     */
    SFGHASH* genHash [SFRF_MAX_GENID];

    // Number of DOS thresholds added.
    int count;

    // count of no revert DOS thresholds
    unsigned noRevertCount;

    int memcap;

    int internal_event_mask;

} RateFilterConfig;

/*
 * Prototypes
 */
void SFRF_Delete(void);
void SFRF_Flush(void);

struct _SnortConfig;
int SFRF_ConfigAdd(struct _SnortConfig *, RateFilterConfig *, tSFRFConfigNode* );

int SFRF_TestThreshold(
    RateFilterConfig *config,
    unsigned gid,
    unsigned sid,
    sfaddr_t* sip,
    sfaddr_t* dip,
    time_t curTime,
    SFRF_COUNT_OPERATION
);

void SFRF_ShowObjects(RateFilterConfig *);
int SFRF_InternalSynRecdEvent(Packet* p);
/*@}*/
#endif // _SFRF_H_

