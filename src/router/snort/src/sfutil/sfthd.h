/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2003-2013 Sourcefire, Inc.
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

/*!
    \file sfthd.h
*/
#ifndef _SF_THRESHOLDING_
#define _SF_THRESHOLDING_

#include "ipv6_port.h"
#include "parser/IpAddrSet.h"

#include "sflsq.h"
#include "sfghash.h"
#include "sfxhash.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"

/*!
    Max GEN_ID value - Set this to the Max Used by Snort, this is used for the
    dimensions of the gen_id lookup array.

    Rows in each hash table, by gen_id.
*/
#define THD_MAX_GENID     8129
#define THD_GEN_ID_1_ROWS 4096
#define THD_GEN_ID_ROWS   512

#define THD_NO_THRESHOLD -1

#define THD_TOO_MANY_THDOBJ -15

/*!
   Type of Thresholding
*/
enum
{
    THD_TYPE_LIMIT,
    THD_TYPE_THRESHOLD,
    THD_TYPE_BOTH,
    THD_TYPE_SUPPRESS,
    THD_TYPE_DETECT
};

/*
   Very high priority for suppression objects
   users priorities are limited to this minus one
*/
#define THD_PRIORITY_SUPPRESS 1000000

/*!
   Tracking by src, or by dst
*/
enum
{
    THD_TRK_NONE,  // suppress only
    THD_TRK_SRC,
    THD_TRK_DST
};


/*!
    THD_IP_NODE

    Dynamic hashed node data - added and deleted during runtime
    These are added during run-time, and recycled if we max out memory usage.
*/
typedef struct {

    unsigned count;
    unsigned prev;
    time_t tstart;
    time_t tlast;

} THD_IP_NODE;


/*!
    THD_IP_NODE_KEY

    HASH Key to lookup and store Ip nodes. The structure now tracks thresholds for different
    policies. This destroys locality of reference and may cause poor performance.
*/
typedef struct{

    int thd_id;
    struct in6_addr ip;
    tSfPolicyId policyId;

} THD_IP_NODE_KEY ;

typedef struct{

    unsigned gen_id;
    unsigned sig_id;
    struct in6_addr ip;
    tSfPolicyId policyId;

} THD_IP_GNODE_KEY ;


/*!
    THD_NODE

    A Thresholding Object
    These are created at program startup, and remain static.
    The THD_IP_NODE elements are dynamic.
*/
typedef struct {

    int      thd_id;   /* Id of this node */
    unsigned gen_id;   /* Keep these around if needed */
    unsigned sig_id;
    int      tracking; /* by_src, by_dst */
    int      type;
    int      priority;
    int      count;
    unsigned seconds;
    uint64_t filtered;
    IpAddrSet* ip_address;

} THD_NODE;


/*!
    THD_ITEM

    The THD_ITEM acts as a container of gen_id+sig_id based threshold objects,
    this allows multiple threshold objects to be applied to a single
    gen_id+sig_id pair. The sflist is created using the priority field,
    so highest priority objects are first in the list. When processing the
    highest priority object will trigger first.

    These are static data elements, built at program startup.
*/
typedef struct {

    tSfPolicyId policyId;
    unsigned gen_id; /* just so we know what gen_id we are */
    unsigned sig_id;
    /*
     * List of THD_NODE's - walk this list and hash the
     * 'THD_NODE->sfthd_id + src_ip or dst_ip' to get the correct THD_IP_NODE.
     */
    SF_LIST* sfthd_node_list;

} THD_ITEM;

// Temporary structure useful when parsing the Snort rules
typedef struct _THDX_STRUCT
{
    unsigned gen_id;
    unsigned sig_id;
    int  type;
    int  tracking;
    int  priority;
    int  count;
    unsigned int  seconds;
    IpAddrSet* ip_address;

} THDX_STRUCT;

typedef struct {

    tSfPolicyId policyId;
    unsigned sig_id;

} tThdItemKey;

/*!
    THD_STRUCT

    The main thresholding data structure.

    Local and global threshold thd_id's are all unqiue, so we use just one
    ip_nodes lookup table
 */
typedef struct _THD_STRUCT
{
    SFXHASH *ip_nodes;   /* Global hash of active IP's key=THD_IP_NODE_KEY, data=THD_IP_NODE */
    SFXHASH *ip_gnodes;  /* Global hash of active IP's key=THD_IP_GNODE_KEY, data=THD_IP_GNODE */

} THD_STRUCT;

typedef struct _ThresholdObjects
{
    int count;  /* Total number of thresholding/suppression objects */
    SFGHASH *sfthd_array[THD_MAX_GENID];    /* Local Hash of THD_ITEM nodes,  lookup by key=sig_id */

    /* Double array of THD_NODE pointers. First index is policyId and therefore variable length.
     * Second index is genId and of fixed length, A simpler definition could be
     * THD_NODE * sfthd_garray[0][THD_MAX_GENID] and we could intentionally overflow first index,
     * but we may have to deal with compiler warnings if the array is indexed by value known at
     * compile time.
     */
    //THD_NODE * (*sfthd_garray)[THD_MAX_GENID];
    THD_NODE* **sfthd_garray;
    tSfPolicyId numPoliciesAllocated;

} ThresholdObjects;


/*
 * Prototypes
 */
// lbytes = local threshold memcap
// gbytes = global threshold memcap (0 to disable global)
THD_STRUCT * sfthd_new(unsigned lbytes, unsigned gbytes);
SFXHASH * sfthd_local_new(unsigned bytes);
SFXHASH * sfthd_global_new(unsigned bytes);
void sfthd_free(THD_STRUCT *);
ThresholdObjects * sfthd_objs_new(void);
void sfthd_objs_free(ThresholdObjects *);

int sfthd_test_rule(SFXHASH *rule_hash, THD_NODE *sfthd_node,
                    sfaddr_t* sip, sfaddr_t* dip, long curtime,
                    detection_option_eval_data_t *eval_data);

void * sfthd_create_rule_threshold(
   int id,
   int tracking,
   int type,
   int count,
   unsigned int seconds
);

struct _SnortConfig;
int sfthd_create_threshold(
    struct _SnortConfig *,
    ThresholdObjects *,
    unsigned     gen_id,
    unsigned     sig_id,
    int          tracking,
    int          type,
    int          priority,
    int          count,
    int          seconds,
    IpAddrSet*   ip_address
);

//  1: don't log due to event_filter
//  0: log
// -1: don't log due to suppress
int sfthd_test_threshold(
    ThresholdObjects *,
    THD_STRUCT *,
    unsigned     gen_id,
    unsigned     sig_id,
    sfaddr_t*    sip,
    sfaddr_t*    dip,
    long         curtime ) ;


SFXHASH * sfthd_new_hash(unsigned, size_t, size_t);

int sfthd_test_local(
    SFXHASH *local_hash,
    THD_NODE   * sfthd_node,
    sfaddr_t*    sip,
    sfaddr_t*    dip,
    time_t       curtime,
    detection_option_eval_data_t *eval_data);

#ifdef THD_DEBUG
int sfthd_show_objects( THD_STRUCT * thd );
#endif

#endif
