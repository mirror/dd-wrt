/* $Id$ */

/*
 ** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2005-2013 Sourcefire, Inc.
 ** AUTHOR: Steven Sturges
 **
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License Version 2 as
 ** published by the Free Software Foundation.  You may not use, modify or
 ** distribute this program under any other version of the GNU General
 ** Public License.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/* session_expect.c
 *
 * Purpose: Handle hash table storage and lookups for ignoring
 *          entire data streams.
 *
 * Arguments:
 *
 * Effect:
 *
 * Comments:
 *
 * Any comments?
 *
 */
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif /* WIN32 */
#include <time.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "snort.h"
#include "session_api.h"
#include "stream_api.h"
#include "session_expect.h"
#include "sf_types.h"
#include "snort_debug.h"
#include "decode.h"
#include "sfxhash.h"
#include "util.h"
#include "ipv6_port.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"
#include "sfdaq.h"
#include "stream5_ha.h"

/* Reasonably small, and prime */
#define EXPECT_HASH_SIZE 1021

/* Number of unique ExpectSessionData stored in each hash entry. */
#define NUM_SESSION_DATA_MAX 8
#define STREAM_EXPECT_CLEAN_LIMIT   5

/* In spp_stream5.c, stream callback ID (stream_cb_idx) is initially 1
 * and only increments. */
#define INVALIDCBID 0

typedef struct _ExpectedSessionData
{
    uint32_t preprocId;
    unsigned cbId;
    Stream_Event se;
    void *appData;
    void (*appDataFreeFn)(void *);
    struct _ExpectedSessionData *next;
} ExpectedSessionData;

typedef struct _ExpectedSessionDataList
{
    ExpectedSessionData *data;
    struct _ExpectedSessionDataList *next;
} ExpectedSessionDataList;

typedef struct _ExpectHashKey
{
    snort_ip ip1;
    snort_ip ip2;
    uint16_t port1;
    uint16_t port2;
    uint32_t protocol;
} ExpectHashKey;

typedef struct _ExpectNode
{
    int reversed_key;
    time_t expires;
    int direction;
    unsigned data_list_count;
    int16_t appId;
    ExpectedSessionDataList *data_list;
    ExpectedSessionDataList *data_list_tail;
} ExpectNode;

/* The hash table of expected channels */
static SFXHASH *channelHash = NULL;

static void freeExpectedSessionData(ExpectedSessionData *data)
{
    ExpectedSessionData *tmp;

    while ((tmp = data))
    {
        data = tmp->next;
        if (tmp->appData && tmp->appDataFreeFn)
            tmp->appDataFreeFn(tmp->appData);
        free(tmp);
    }
}

static void freeNodeAppData(ExpectNode *node)
{
    ExpectedSessionDataList *data_list;

    while ((data_list = node->data_list))
    {
        node->data_list = data_list->next;
        freeExpectedSessionData(data_list->data);
        free(data_list);
    }
    node->data_list_tail = NULL;
    node->data_list_count = 0;
}

static int freeHashNode(void *k, void *p)
{
    freeNodeAppData((ExpectNode*)p);
    return 0;
}

static snort_ip zeroed;

/**Either expect or expect future session.
 *
 * Preprocessors may add sessions to be expected altogether or to be associated with some data. For example,
 * FTP preprocessor may add data channel that should be expected. Alternatively, FTP preprocessor may add
 * session with appId FTP-DATA.
 *
 * It is assumed that only one of cliPort or srvPort should be known (!0). This violation of this assumption
 * will cause hash collision that will cause some session to be not expected and expected. This will occur only
 * rarely and therefore acceptable design optimization.
 *
 * Also, appId is assumed to be consistent between different preprocessors. Each session can be assigned only
 * one AppId. When new appId mismatches existing appId, new appId and associated data is not stored.
 *
 * @param cliIP - client IP address. All preprocessors must have consistent view of client side of a session.
 * @param cliPort - client port number
 * @param srvIP - server IP address. All preprocessors must have consisten view of server side of a session.
 * @param srcPort - server port number
 * @param protocol - IPPROTO_TCP or IPPROTO_UDP.
 * @param direction - direction of session. Assumed that direction value for session being expected or expected will
 * remain same across different calls to this function.
 * @param expiry - session expiry in seconds.
 */
int StreamExpectAddChannel(const Packet *ctrlPkt, snort_ip_p cliIP, uint16_t cliPort,
        snort_ip_p srvIP, uint16_t srvPort, char direction, uint8_t flags,
        uint8_t protocol, uint32_t timeout, int16_t appId, uint32_t preprocId,
        void *appData, void (*appDataFreeFn)(void*))
{
    return StreamExpectAddChannelPreassignCallback(ctrlPkt, cliIP, cliPort, srvIP, srvPort, 
            direction, flags, protocol, timeout, appId, preprocId,
            appData, appDataFreeFn, INVALIDCBID, SE_REXMIT);
}

int StreamExpectAddChannelPreassignCallback(const Packet *ctrlPkt, snort_ip_p cliIP, uint16_t cliPort,
        snort_ip_p srvIP, uint16_t srvPort, char direction, uint8_t flags,
        uint8_t protocol, uint32_t timeout, int16_t appId, uint32_t preprocId,
        void *appData, void (*appDataFreeFn)(void*), unsigned cbId,
        Stream_Event se)
{ 
    ExpectHashKey hashKey;
    SFXHASH_NODE *hash_node;
    ExpectNode new_node;
    ExpectNode *node;
    ExpectedSessionDataList *data_list;
    ExpectedSessionData *data;
    int reversed_key;
    SFIP_RET rval;
    time_t now =  ctrlPkt->pkth->ts.tv_sec;

    if (cliPort != UNKNOWN_PORT)
        srvPort = UNKNOWN_PORT;

#if defined(DEBUG_MSGS)
    {
        char src_ip[INET6_ADDRSTRLEN];
        char dst_ip[INET6_ADDRSTRLEN];

        sfip_ntop(cliIP, src_ip, sizeof(src_ip));
        sfip_ntop(srvIP, dst_ip, sizeof(dst_ip));
        DebugMessage(DEBUG_STREAM, "Creating expected %s-%u -> %s-%u %u appid %d  preproc %u\n", src_ip,
                cliPort, dst_ip, srvPort, protocol, appId, preprocId);
    }
#endif

    /* Add the info to a tree that marks this channel as one to expect.
     * Only one of the port values may be UNKNOWN_PORT.
     * As a sanity check, the IP addresses may not be 0 or 255.255.255.255.
     */
    if ((cliPort == UNKNOWN_PORT) && (srvPort == UNKNOWN_PORT))
        return -1;

    if (cliIP->family == AF_INET)
    {
        if (!cliIP->ip.u6_addr32[0] || cliIP->ip.u6_addr32[0] == 0xFFFFFFFF ||
                !srvIP->ip.u6_addr32[0] || srvIP->ip.u6_addr32[0] == 0xFFFFFFFF)
        {
            return -1;
        }
    }
    else if (sfip_fast_eq6(cliIP, IP_ARG(zeroed)) || sfip_fast_eq6(srvIP, IP_ARG(zeroed)))
    {
        return -1;
    }

    rval = sfip_compare(cliIP, srvIP);
    if (rval == SFIP_LESSER || (rval == SFIP_EQUAL && cliPort < srvPort))
    {
        IP_COPY_VALUE(hashKey.ip1, cliIP);
        hashKey.port1 = cliPort;
        IP_COPY_VALUE(hashKey.ip2, srvIP);
        hashKey.port2 = srvPort;
        reversed_key = 0;
    }
    else
    {
        IP_COPY_VALUE(hashKey.ip1, srvIP);
        hashKey.port1 = srvPort;
        IP_COPY_VALUE(hashKey.ip2, cliIP);
        hashKey.port2 = cliPort;
        reversed_key = 1;
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "reversed\n"););
    }

    /* Actually add it to the hash table with a timestamp of now.
     * so we can expire entries that are older than a configurable
     * time.  Those entries will be for sessions that we missed or
     * never occured.  Should not keep the entry around indefinitely.
     */
    hashKey.protocol = (uint32_t)protocol;

    hash_node = sfxhash_find_node(channelHash, &hashKey);
    if( hash_node )
    {
        if( !( node = hash_node->data ) )
            sfxhash_free_node( channelHash, hash_node );
    }
    else
        node = NULL;

    if( node )
    {
        int expired;

        DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "exists\n"););
        /*
         * This handles the case where there is already an entry
         * for this key (IP addresses/port).  It could occur when
         * multiple users from behind a NAT'd Firewall all go to the
         * same site when in FTP Port mode.  To get around this issue,
         * we keep a counter of the number of pending open channels
         * with the same known endpoints (2 IPs & a port).  When that
         * channel is actually opened, the counter is decremented, and
         * the entry is removed when the counter hits 0.
         * Because all of this is single threaded, there is no potential
         * for a race condition.
         */
        expired = ( node->expires != 0 ) && ( now > node->expires );
        if( expired )
        {
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "expected session is expired\n"););
            //free older data
            freeNodeAppData( node );
            node->appId = appId;
        }

        if( node->appId != appId )
        {
            if( node->appId && appId )
            {
                DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "expected session has different appId %d != %d\n", 
                            node->appId, appId););
                return -1;
            }
            node->appId = appId;
        }

        if( node->data_list_count >= NUM_SESSION_DATA_MAX )
        {
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "expected session has maximum data slots used\n"););
            return -1;
        }

        if( ( data_list = node->data_list_tail ) )
        {
            for( data = data_list->data; data && data->preprocId != preprocId; data = data->next );
            if( data )
            {
                DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Found an existing occurance\n"););
                data_list = NULL;
            }
        }

        data = malloc( sizeof( *data ) );
        if( !data )
        {
            if( !node->data_list_count )
                sfxhash_free_node( channelHash, hash_node );

            DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Memory alloc error\n"););
            return -1;
        }
        data->appData = appData;
        data->cbId = cbId;
        data->se = se;
        data->appDataFreeFn = appDataFreeFn;
        data->preprocId = preprocId;

        if( !data_list )
        {
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Adding new occurance\n"););
            data_list = calloc( 1, sizeof( *data_list ) );
            if( !data_list )
            {
                if( !node->data_list_count )
                    sfxhash_free_node( channelHash, hash_node );

                free( data );
                DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Memory alloc error\n"););
                return -1;
            }

            if( node->data_list_tail )
            {
                node->data_list_tail->next = data_list;
                node->data_list_tail = data_list;
            }
            else
                node->data_list = node->data_list_tail = data_list;

            node->data_list_count++;
        }
#ifdef DEBUG_MSGS
        else
        {
            DebugMessage(DEBUG_STREAM, "Using an existing occurance\n");
        }
#endif
        data->next = data_list->data;
        data_list->data = data;

        if( flags & EXPECT_FLAG_ALWAYS )
            node->expires = 0;
        else
            node->expires = now + timeout;
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Updating expect channel node with %u occurances\n", node->data_list_count););
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Adding expect channel node\n"););

        new_node.appId = appId;
        new_node.reversed_key = reversed_key;
        new_node.direction = direction;
        /* now + 5 minutes (configurable?)
         *
         * use the time that we keep sessions around
         * since this info would effectively be invalid
         * after that anyway because the session that
         * caused this will be gone.
         */
        if( flags & EXPECT_FLAG_ALWAYS )
            new_node.expires = 0;
        else
            new_node.expires = now + timeout;

        data = malloc( sizeof( *data ) );
        if( !data )
        {
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Memory alloc error\n"););
            return -1;
        }
        data->appData = appData;
        data->cbId = cbId;
        data->se = se;
        data->appDataFreeFn = appDataFreeFn;
        data->preprocId = preprocId;
        data->next = NULL;
        data_list = malloc( sizeof( *data_list ) );
        if( !data_list )
        {
            free( data );
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Memory alloc error\n"););
            return -1;
        }
        data_list->next = NULL;
        data_list->data = data;
        new_node.data_list_count = 1;
        new_node.data_list = new_node.data_list_tail = data_list;

        /* Add it to the table */
        if( sfxhash_add( channelHash, &hashKey, &new_node ) != SFXHASH_OK )
        {
            /* Uh, shouldn't get here...
             * There is already a node or couldn't alloc space
             * for key.  This means bigger problems, but fail
             * gracefully.
             */
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM,
                        "Failed to add channel node to expected hash table\n"););
            free( data_list );
            free( data );
            return -1;
        }

#ifdef HAVE_DAQ_DP_ADD_DC
        // when adding expected channel send expected flow parameters to the DAQ
        // for forwarding to firmware...
        if( cliPort == UNKNOWN_PORT )
           DAQ_Add_Dynamic_Protocol_Channel( ctrlPkt, cliIP, cliPort, srvIP, srvPort, protocol );
        else
           DAQ_Add_Dynamic_Protocol_Channel( ctrlPkt, srvIP, srvPort, cliIP, cliPort, protocol );
#endif

    }

    return 0;
}

int StreamExpectIsExpected(Packet *p, SFXHASH_NODE **expected_hash_node)
{
    snort_ip_p srcIP, dstIP;
    SFXHASH_NODE *hash_node;
    ExpectHashKey hashKey;
    ExpectNode *node;
    SFIP_RET rval;
    uint16_t port1;
    uint16_t port2;
    int reversed_key;

    /* No hash table, or its empty?  Get out of dodge.  */
    if (!sfxhash_count(channelHash))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "No expected sessions\n"););
        return 0;
    }

    srcIP = GET_SRC_IP(p);
    dstIP = GET_DST_IP(p);

#if defined(DEBUG_MSGS)
    {
        char src_ip[INET6_ADDRSTRLEN];
        char dst_ip[INET6_ADDRSTRLEN];

        sfip_ntop(srcIP, src_ip, sizeof(src_ip));
        sfip_ntop(dstIP, dst_ip, sizeof(dst_ip));
        DebugMessage(DEBUG_STREAM, "Checking isExpected %s-%u -> %s-%u %u\n", src_ip,
                p->sp, dst_ip, p->dp, GET_IPH_PROTO(p));
    }
#endif

    rval = sfip_compare(dstIP, srcIP);
    if (rval == SFIP_LESSER || (rval == SFIP_EQUAL && p->dp < p->sp))
    {
        IP_COPY_VALUE(hashKey.ip1, dstIP);
        IP_COPY_VALUE(hashKey.ip2, srcIP);
        hashKey.port1 = p->dp;
        hashKey.port2 = 0;
        port1 = 0;
        port2 = p->sp;
        reversed_key = 1;
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "reversed\n"););
    }
    else
    {
        IP_COPY_VALUE(hashKey.ip1, srcIP);
        IP_COPY_VALUE(hashKey.ip2, dstIP);
        hashKey.port1 = 0;
        hashKey.port2 = p->dp;
        port1 = p->sp;
        port2 = 0;
        reversed_key = 0;
    }
    hashKey.protocol = (uint32_t)GET_IPH_PROTO(p);

    hash_node = sfxhash_find_node(channelHash, &hashKey);
    if (hash_node)
    {
        if (!(node = hash_node->data))
            sfxhash_free_node(channelHash, hash_node);
    }
    else
        node = NULL;
    if (!node)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Could not find with dp\n"););
        hashKey.port1 = port1;
        hashKey.port2 = port2;
        hash_node = sfxhash_find_node(channelHash, &hashKey);
        if (hash_node)
        {
            if (!(node = hash_node->data))
                sfxhash_free_node(channelHash, hash_node);
        }
    }
    if (node)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Found expected\n"););
        if (node->expires && p->pkth->ts.tv_sec > node->expires)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Expected expired\n"););
            sfxhash_free_node(channelHash, hash_node);
            return 0;
        }
        if (!node->data_list)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Expected session has no data???\n"););
            sfxhash_free_node(channelHash, hash_node);
            return 0;
        }
        /* Make sure the packet direction is correct */
        switch (node->direction)
        {
            case SSN_DIR_BOTH:
                break;
            case SSN_DIR_FROM_CLIENT:
            case SSN_DIR_FROM_SERVER:
                if (node->reversed_key != reversed_key)
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Expected is the wrong direction\n"););
                    return 0;
                }
                break;
        }
        *expected_hash_node = hash_node;
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "using expected\n"););
        return 1;
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Could not find with sp\n"););
    }
    return 0;
}

char StreamExpectProcessNode(Packet *p, SessionControlBlock* lws, SFXHASH_NODE *expected_hash_node)
{
    SFXHASH_NODE *hash_node;
    ExpectNode *node;
    ExpectedSessionDataList *data_list;
    ExpectedSessionData *data;
    time_t now;
    int retVal = SSN_DIR_NONE;
    unsigned i;

    node = expected_hash_node->data;
    node->data_list_count--;
    data_list = node->data_list;
    node->data_list = data_list->next;

    while ((data = data_list->data))
    {
        data_list->data = data->next;
        if (data->appData && 
                session_api->set_application_data(lws, data->preprocId, data->appData, data->appDataFreeFn)
                && data->appDataFreeFn)
        {
            data->appDataFreeFn(data->appData);
        }

        if(data->cbId != INVALIDCBID)
        {
            stream_api->set_event_handler(lws, data->cbId, data->se);
            p->packet_flags |= PKT_EARLY_REASSEMBLY;
        }

        free(data);
    }
    free(data_list);

    /* If this is 0, we're ignoring, otherwise setting id of new session */
    if (!node->appId)
        retVal = node->direction;
#ifdef TARGET_BASED
    else if (lws->ha_state.application_protocol != node->appId)
    {
        lws->ha_state.application_protocol = node->appId;
#ifdef ENABLE_HA
        lws->ha_flags |= HA_FLAG_MODIFIED;
#endif
        session_api->set_application_protocol_id( lws, node->appId );
    }
#endif

#if defined(DEBUG_MSGS)
    {
        snort_ip_p srcIP, dstIP;
        char src_ip[INET6_ADDRSTRLEN];
        char dst_ip[INET6_ADDRSTRLEN];

        srcIP = GET_SRC_IP(p);
        dstIP = GET_DST_IP(p);
        sfip_ntop(srcIP, src_ip, sizeof(src_ip));
        sfip_ntop(dstIP, dst_ip, sizeof(dst_ip));
        DebugMessage(DEBUG_STREAM,
                "Ignoring channel %s:%d --> %s:%d, policyId %d\n",
                src_ip, p->sp,
                dst_ip, p->dp, getNapRuntimePolicy());
    }
#endif

    if (!node->data_list)
        sfxhash_free_node(channelHash, expected_hash_node);

    now = p->pkth->ts.tv_sec;
    /* Clean the hash table of at most STREAM_EXPECT_CLEAN_LIMIT expired nodes */
    for (i = 0; i < STREAM_EXPECT_CLEAN_LIMIT && (hash_node = sfxhash_lru_node(channelHash)); i++)
    {
        node = hash_node->data;
        if (node)
        {
            if (node->expires && now > node->expires)
            {
                /* sayonara baby... */
                sfxhash_free_node(channelHash, hash_node);
            }
            else
            {
                /* This one's not expired, fine...
                 * no need to prune further.
                 */
                break;
            }
        }
    }

    return retVal;
}

char StreamExpectCheck(Packet *p, SessionControlBlock* lws)
{
    SFXHASH_NODE *hash_node;

    if (!StreamExpectIsExpected(p, &hash_node))
        return SSN_DIR_NONE;

    return StreamExpectProcessNode(p, lws, hash_node);
}

void StreamExpectInit (uint32_t max)
{
    // number of entries * overhead per entry
    max *= (sizeof(SFXHASH_NODE) + sizeof(long) + 
            sizeof(ExpectHashKey) + sizeof(ExpectNode));

    // add in fixed cost of hash table
    max += (sizeof(SFXHASH_NODE**) * EXPECT_HASH_SIZE) + sizeof(long);

    channelHash = sfxhash_new(
            -EXPECT_HASH_SIZE, sizeof(ExpectHashKey), 
            sizeof(ExpectNode), max, 1, freeHashNode, freeHashNode, 1);

    if (!channelHash)
        FatalError("Failed to create the expected channel hash table.\n");

    memset(&zeroed, 0, sizeof(zeroed));
}

void StreamExpectCleanup(void)
{
    if (channelHash)
    {
        sfxhash_delete(channelHash);
        channelHash = NULL;
    }
}

