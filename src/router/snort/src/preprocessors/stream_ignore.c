/* $Id$ */

/*
** Copyright (C) 2005-2011 Sourcefire, Inc.
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* stream_ignore.c
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

#include "debug.h"
#include "decode.h"
#include "stream_api.h"
#include "sfghash.h"
#include "util.h"
#include "ipv6_port.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"

/* Reasonably small, and prime */
#define IGNORE_HASH_SIZE 1021
typedef struct _IgnoreNode
{
    snort_ip ip1;
    short port1;
    snort_ip ip2;
    short port2;
    char protocol;
    time_t expires;
    int direction;
    int numOccurances;
    tSfPolicyId policyId;
    int16_t appId;
} IgnoreNode;

typedef struct _IgnoreHashKey
{
    snort_ip ip1;
    snort_ip ip2;
    tSfPolicyId policyId;
    short port;
    char protocol;
    char pad;
} IgnoreHashKey;

/* The hash table of ignored channels */
static SFGHASH *channelHash = NULL;

int IgnoreChannel(snort_ip_p cliIP, uint16_t cliPort,
                  snort_ip_p srvIP, uint16_t srvPort,
                  char protocol, char direction, char flags,
                  uint32_t timeout, int16_t appId)
{
    IgnoreHashKey hashKey;
    time_t now;
    IgnoreNode *node = NULL;
    short portToHash = cliPort != UNKNOWN_PORT ? cliPort : srvPort;
    snort_ip_p ip1, ip2;
    snort_ip zeroed, oned;
    IP_CLEAR(zeroed);
#ifdef SUP_IP6
    memset(oned.ip8, 1, 16);
    zeroed.family = oned.family = cliIP->family;
#else
    oned = 0xffffffff;
#endif

    if (!channelHash)
    {
        /* Create the hash table */
        channelHash = sfghash_new(IGNORE_HASH_SIZE,
                                  sizeof(IgnoreHashKey), 0, free);
    }
   
    time(&now);

    /* Add the info to a tree that marks this channel as one to ignore.
     * Only one of the port values may be UNKNOWN_PORT.  
     * As a sanity check, the IP addresses may not be 0 or 255.255.255.255.
     */
    if ((cliPort == UNKNOWN_PORT) && (srvPort == UNKNOWN_PORT))
        return -1;

    if (sfip_equal(cliIP, IP_ARG(zeroed)) || sfip_equal(cliIP, IP_ARG(oned)) ||
        sfip_equal(srvIP, IP_ARG(zeroed)) || sfip_equal(srvIP, IP_ARG(oned)) )
        return -1;

    if (IP_LESSER(cliIP, srvIP))
    {
        ip1 = cliIP;
        ip2 = srvIP;
    }
    else
    {
        ip1 = srvIP;
        ip2 = cliIP;
    }

    /* Actually add it to the hash table with a timestamp of now.
     * so we can expire entries that are older than a configurable
     * time.  Those entries will be for sessions that we missed or
     * never occured.  Should not keep the entry around indefinitely.
     */
    IP_COPY_VALUE(hashKey.ip1, ip1);
    IP_COPY_VALUE(hashKey.ip2, ip2);
    hashKey.port = portToHash;
    hashKey.protocol = protocol;
    hashKey.policyId = getRuntimePolicy();
    hashKey.pad = 0;

    node = sfghash_find(channelHash, &hashKey);
    if (node)
    {
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
        int expired = (node->expires != 0) && (now > node->expires);
        if (expired)
        {
            IP_COPY_VALUE(node->ip1, cliIP);
            node->port1 = cliPort;
            IP_COPY_VALUE(node->ip2, srvIP);
            node->port2 = srvPort;
            node->direction = direction;
            node->protocol = protocol;
            node->policyId = getRuntimePolicy();
            node->appId = appId;
        }
        else
        {
            node->numOccurances++;
        }
        if (flags & IGNORE_FLAG_ALWAYS)
            node->expires = 0;
        else
            node->expires = now + timeout;
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM,
                   "Updating ignore channel node\n"););
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM,
                   "Adding ignore channel node\n"););

        node = SnortAlloc(sizeof(IgnoreNode));
        if (!node)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Memory alloc error\n"););
            return -1;
        }
        IP_COPY_VALUE(node->ip1, cliIP);
        node->port1 = cliPort;
        IP_COPY_VALUE(node->ip2, srvIP);
        node->port2 = srvPort;
        node->direction = direction;
        node->protocol = protocol;
        node->policyId = getRuntimePolicy();
        /* now + 5 minutes (configurable?)
         *
         * use the time that we keep sessions around
         * since this info would effectively be invalid
         * after that anyway because the session that
         * caused this will be gone.
         */
        if (flags & IGNORE_FLAG_ALWAYS)
            node->expires = 0;
        else
            node->expires = now + timeout;
        node->numOccurances = 1;
        node->appId = appId;

        /* Add it to the table */
        if (sfghash_add(channelHash, &hashKey, (void *)node)
            != SFGHASH_OK)
        {
            /* Uh, shouldn't get here...
             * There is already a node or couldn't alloc space
             * for key.  This means bigger problems, but fail
             * gracefully.
             */
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM,
                       "Failed to add channel node to hash table\n"););
            free(node);
            return -1;
        }
    }

    return 0;
}

char CheckIgnoreChannel(Packet *p, int16_t *appId)
{
    snort_ip_p srcIP, dstIP;
    short srcPort, dstPort;
    char protocol;

    IgnoreHashKey hashKey;
    time_t now;
    int match = 0;
    int retVal = 0;
    IgnoreNode *node = NULL;
    int expired = 0;
    int i;

    /* No hash table, or its empty?  Get out of dodge.  */
    if (!channelHash || channelHash->count == 0)
        return retVal;

    srcIP = GET_SRC_IP(p);
    dstIP = GET_DST_IP(p);
    srcPort = p->sp;
    dstPort = p->dp;
    protocol = GET_IPH_PROTO(p);
    
    /* First try the hash table using the dstPort.
     * For FTP data channel this would be the client's port when the PORT
     * command is used and the server is initiating the connection.
     * This is done first because it is the most common case for FTP clients.
     */
    if (IP_LESSER(dstIP,srcIP))
    {
        IP_COPY_VALUE(hashKey.ip1, dstIP);
        IP_COPY_VALUE(hashKey.ip2, srcIP);
    }
    else
    {
        IP_COPY_VALUE(hashKey.ip1, srcIP);
        IP_COPY_VALUE(hashKey.ip2, dstIP);
    }
    hashKey.port = dstPort;
    hashKey.protocol = protocol;
    hashKey.pad = 0;
    hashKey.policyId = getRuntimePolicy();

    node = sfghash_find(channelHash, &hashKey);

    if (!node)
    {
        /* Okay, next try the hash table using the srcPort.
         * For FTP data channel this would be the servers's port when the
         * PASV command is used and the client is initiating the connection.
         */
        hashKey.port = srcPort;
        node = sfghash_find(channelHash, &hashKey);

        /* We could also check the reverses of these, ie. use 
         * srcIP then dstIP in the hashKey.  Don't need to, though.
         *
         * Here's why:
         * 
         * Since there will be an ACK that comes back from the server
         * side, we don't need to look for the hash entry the other
         * way -- it will be found when we get the ACK.  This approach
         * results in 2 checks per packet -- and 2 checks on the ACK.
         * If we find a match, cool.  If not we've done at most 4 checks
         * between the packet and the ACK.
         * 
         * Whereas, if we check the reverses, we do 4 checks on each
         * side, or 8 checks between the packet and the ACK.  While
         * this would more quickly find the channel to ignore, it is
         * a performance hit when we the session in question is
         * NOT being ignored.  Err on the side of performance here.
         */
    }


    /* Okay, found the key --> verify that the info in the node
     * does in fact match and has not expired.
     */
    time(&now);
    if (node)
    {
        /* If the IPs match and if the ports match (or the port is
         * "unknown"), we should ignore this channel.
         */
        if(
#ifdef SUP_IP6
        IP_EQUALITY(&node->ip1, srcIP) && IP_EQUALITY(&node->ip2, dstIP) &&
#else
        IP_EQUALITY(node->ip1, srcIP) && IP_EQUALITY(node->ip2, dstIP) &&
#endif
            (node->policyId == getRuntimePolicy()) &&
            (node->port1 == srcPort || node->port1 == UNKNOWN_PORT) &&
            (node->port2 == dstPort || node->port2 == UNKNOWN_PORT) )
        {
            match = 1;
        }
        else if (
#ifdef SUP_IP6
        IP_EQUALITY(&node->ip2, srcIP) && IP_EQUALITY(&node->ip1, dstIP) &&
#else
        IP_EQUALITY(node->ip2, srcIP) && IP_EQUALITY(node->ip1, dstIP) &&
#endif
                 (node->policyId == getRuntimePolicy()) &&
                 (node->port2 == srcPort || node->port2 == UNKNOWN_PORT) &&
                 (node->port1 == dstPort || node->port1 == UNKNOWN_PORT) )
        {
            match = 1;
        }

        /* Make sure the packet direction is correct */
        switch (node->direction)
        {
            case SSN_DIR_BOTH:
                break;
            case SSN_DIR_CLIENT:
                if (!(p->packet_flags & PKT_FROM_CLIENT))
                    match = 0;
                break;
            case SSN_DIR_SERVER:
                if (!(p->packet_flags & PKT_FROM_SERVER))
                    match = 0;
                break;
        }

        if (node->expires)
            expired = (now > node->expires);
        if (match)
        {
            /* Uh, just check to be sure it hasn't expired,
             * in case we missed a packet and this is a
             * different connection.  */
            if ((node->numOccurances > 0) && (!expired))
            {
                node->numOccurances--;
                /* Matched & Still valid --> ignore it! */

                if (node->appId) /* If this is 0, we're ignoring, otherwise setting id of new session */
                    *appId = node->appId;
                else
                    retVal = node->direction;
#ifdef DEBUG
                {
                    /* Have to allocate & copy one of these since inet_ntoa
                     * clobbers the info from the previous call. */

#ifdef SUP_IP6
                    sfip_t *tmpAddr;
                    char srcAddr[40];
                    tmpAddr = srcIP;
                    SnortStrncpy(srcAddr, sfip_ntoa(tmpAddr), sizeof(srcAddr));
                    tmpAddr = dstIP;
#else

                    struct in_addr tmpAddr;
                    char srcAddr[17];
                    tmpAddr.s_addr = srcIP;
                    SnortStrncpy(srcAddr, inet_ntoa(tmpAddr), sizeof(srcAddr));
                    tmpAddr.s_addr = dstIP;
#endif

                    DEBUG_WRAP(DebugMessage(DEBUG_STREAM,
                           "Ignoring channel %s:%d --> %s:%d, policyId %d\n",
                           srcAddr, srcPort,
                           inet_ntoa(tmpAddr), dstPort, getRuntimePolicy()););
                }
#endif
            }
        }

        if (((node->numOccurances <= 0) || (expired)) &&
                (node->expires != 0))
        {
            /* Either expired or was the only one in the hash
             * table.  Remove this node.  */
            sfghash_remove(channelHash, &hashKey);
        }
    }

    /* Clean the hash table of at most 5 expired nodes */
    for (i=0;i<5 && channelHash->count>0;i++)
    {
        SFGHASH_NODE *hash_node = sfghash_findfirst(channelHash);
        if (hash_node)
        {
            node = hash_node->data;
            if (node)
            {
                expired = (node->expires != 0) && (now > node->expires);
                if (expired)
                {
                    /* sayonara baby... */
                    sfghash_remove(channelHash, hash_node->key);
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
    }

    return retVal;
}

void CleanupIgnore(void)
{
    if (channelHash)
    {
        sfghash_delete(channelHash);
        channelHash = NULL;
    }
}
