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
 *
 * \file sfthd.c
 *
 * An Abstracted Event Thresholding System
 *
 * Marc Norton
 *
 * 3/5/07 - man - fixed memory leak in global config to limit
 * of one gid=0, or multiple gid!=0 but not both.
 * Boris Lytochkin found it.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef WIN32
#include <netinet/in.h>
#endif

#include "parser/IpAddrSet.h"
#include "sflsq.h"
#include "sfghash.h"
#include "sfxhash.h"

#include "snort.h"
#include "sfthd.h"
#include "util.h"
#include "sfPolicy.h"

//  Debug Printing
//#define THD_DEBUG

// This disables adding and testing of Threshold objects
//#define CRIPPLE

SFXHASH * sfthd_new_hash(unsigned nbytes, size_t key, size_t data)
{
    size_t size = key + data;
    int nrows;

    /* Calc max ip nodes for this memory */
    if ( nbytes < size )
    {
        nbytes = size;
    }
    nrows = nbytes / (size);

    return sfxhash_new(
        nrows,  /* try one node per row - for speed */
        key,    /* keys size */
        data,   /* data size */
        nbytes, /* memcap **/
        1,      /* ANR flag - true ?- Automatic Node Recovery=ANR */
        0,      /* ANR callback - none */
        0,      /* user freemem callback - none */
        1 ) ;   /* Recycle nodes ?*/
}

/*!
  Create a threshold table, initialize the threshold system,
  and optionally limit it's memory usage.

  @param nbytes maximum memory to use for thresholding objects, in bytes.

  @return  THD_STRUCT*
  @retval  0 error
  @retval !0 valid THD_STRUCT
*/

SFXHASH * sfthd_local_new(unsigned bytes)
{
    SFXHASH *local_hash =
        sfthd_new_hash(bytes,
                       sizeof(THD_IP_NODE_KEY),
                       sizeof(THD_IP_NODE));

#ifdef THD_DEBUG
    if (local_hash == NULL)
        printf("Could not allocate the sfxhash table\n");
#endif

    return local_hash;
}

SFXHASH * sfthd_global_new(unsigned bytes)
{
    SFXHASH *global_hash =
        sfthd_new_hash(bytes,
                       sizeof(THD_IP_GNODE_KEY),
                       sizeof(THD_IP_NODE));

#ifdef THD_DEBUG
    if (global_hash == NULL)
        printf("Could not allocate the sfxhash table\n");
#endif

    return global_hash;
}

THD_STRUCT * sfthd_new(unsigned lbytes, unsigned gbytes)
{
    THD_STRUCT * thd;

    /* Create the THD struct */
    thd = (THD_STRUCT *)SnortAlloc(sizeof(THD_STRUCT));

#ifndef CRIPPLE
    /* Create hash table for all of the local IP Nodes */
    thd->ip_nodes = sfthd_local_new(lbytes);
    if( !thd->ip_nodes )
    {
#ifdef THD_DEBUG
        printf("Could not allocate the sfxhash table\n");
#endif
        free(thd);
        return NULL;
    }

    if ( gbytes == 0 )
        return thd;

    /* Create hash table for all of the global IP Nodes */
    thd->ip_gnodes = sfthd_global_new(gbytes);
    if( !thd->ip_gnodes )
    {
#ifdef THD_DEBUG
        printf("Could not allocate the sfxhash table\n");
#endif
        sfxhash_delete(thd->ip_nodes);
        free(thd);
        return NULL;
    }
#endif

    return thd;
}

ThresholdObjects * sfthd_objs_new(void)
{
    return (ThresholdObjects *)SnortAlloc(sizeof(ThresholdObjects));
}

static void sfthd_node_free(void *node)
{
    THD_NODE *sfthd_node = (THD_NODE *)node;

    if (sfthd_node == NULL)
        return;

    if (sfthd_node->ip_address != NULL)
    {
        IpAddrSetDestroy(sfthd_node->ip_address);
    }

    free(sfthd_node);
}

void sfthd_objs_free(ThresholdObjects *thd_objs)
{
    int i;
    tSfPolicyId policyId;

    if (thd_objs == NULL)
        return;

    for (i = 0; i < THD_MAX_GENID; i++)
    {
        if (thd_objs->sfthd_array[i])
            sfghash_delete(thd_objs->sfthd_array[i]);
    }

    for (policyId = 0; policyId < thd_objs->numPoliciesAllocated; policyId++)
    {
        if (thd_objs->sfthd_garray[policyId] == NULL)
            continue;

        if (thd_objs->sfthd_garray[policyId][0] != NULL)
        {
            sfthd_node_free((void *)thd_objs->sfthd_garray[policyId][0]);

            /* Free any individuals */
            for (i = 0; i < THD_MAX_GENID; i++)
            {
                if (thd_objs->sfthd_garray[policyId][i] !=
                    thd_objs->sfthd_garray[policyId][0])
                {
                    sfthd_node_free(
                        (void *)thd_objs->sfthd_garray[policyId][i]);
                }
            }

        }
        else
        {
            /* Anything other GID will be allocated individually */
            for (i = 1; i < THD_MAX_GENID; i++)
            {
                if (thd_objs->sfthd_garray[policyId][i])
                {
                    sfthd_node_free((void *)thd_objs->sfthd_garray[policyId][i]);
                }
            }
        }

        free(thd_objs->sfthd_garray[policyId]);
    }

    if (thd_objs->sfthd_garray != NULL)
        free(thd_objs->sfthd_garray);

    free(thd_objs);
}

void sfthd_item_free(void *item)
{
    THD_ITEM *sfthd_item = (THD_ITEM*)item;
    sflist_free_all(sfthd_item->sfthd_node_list, sfthd_node_free);
    free(sfthd_item);
}

void sfthd_free(THD_STRUCT *thd)
{
    if (thd == NULL)
        return;

#ifndef CRIPPLE
    if (thd->ip_nodes != NULL)
        sfxhash_delete(thd->ip_nodes);

    if (thd->ip_gnodes != NULL)
        sfxhash_delete(thd->ip_gnodes);
#endif

    free(thd);
}

void * sfthd_create_rule_threshold(int id,
                                   int tracking,
                                   int type,
                                   int count,
                                   unsigned int seconds)
{
    THD_NODE *sfthd_node = (THD_NODE *)calloc(1, sizeof(THD_NODE));

    if (sfthd_node == NULL)
        return NULL;

    sfthd_node->thd_id    = id;
    sfthd_node->tracking  = tracking;
    sfthd_node->type      = type;
    sfthd_node->count     = count;
    sfthd_node->seconds   = seconds;

    return (void *)sfthd_node;
}

/*!
Add a permanent threshold object to the threshold table. Multiple
objects may be defined for each gen_id and sig_id pair. Internally
a unique threshold id is generated for each pair.

Threshold objects track the number of events seen during the time
interval specified by seconds. Depending on the type of threshold
object and the count value, the thresholding object determines if
the current event should be logged or dropped.

@param thd Threshold object from sfthd_new()
@param gen_id Generator id
@param sig_id Signauture id
@param tracking Selects tracking by src ip or by dst ip
@param type  Thresholding type: Limit, Threshold, or Limt+Threshold, Suppress
@param priority Assigns a relative priority to this object, higher numbers imply higher priority

@param count Number of events
@param seconds Time duration over which this threshold object acts.
@param ip      IP address, for supression
@param ip-mask IP mask, applied with ip_mask, for supression

@return integer
@retval  0 successfully added the thresholding object
@retval !0 failed

*/
static int sfthd_create_threshold_local(SnortConfig *sc, ThresholdObjects *thd_objs,
                                        THD_NODE* config)
{
    SFGHASH  * sfthd_hash;
    THD_ITEM * sfthd_item;
    THD_NODE * sfthd_node;
    tThdItemKey key;
    int nrows;
    int hstatus;
    tSfPolicyId policy_id = getParserPolicy(sc);

    if (thd_objs == NULL )
        return -1;

    if( config->gen_id >= THD_MAX_GENID )
        return -1;

#ifdef CRIPPLE
    return 0;
#endif

    /* Check for an existing 'gen_id' entry, if none found create one. */
    if (thd_objs->sfthd_array[config->gen_id] == NULL)
    {
        if( config->gen_id == 1 )/* patmatch rules gen_id, many rules */
        {
            nrows= THD_GEN_ID_1_ROWS;
        }
        else  /* other gen_id's */
        {
            nrows= THD_GEN_ID_ROWS;
        }

        /* Create the hash table for this gen_id */
        sfthd_hash = sfghash_new( nrows, sizeof(tThdItemKey), 0, sfthd_item_free );
        if( !sfthd_hash )
        {
            return -2;
        }

        thd_objs->sfthd_array[config->gen_id] = sfthd_hash;
    }
    else
    {
        /* Get the hash table for this gen_id */
        sfthd_hash = thd_objs->sfthd_array[config->gen_id];
    }

    if (sfthd_hash == NULL)
         return -2;

    key.sig_id = config->sig_id;
    key.policyId = policy_id;

    /* Check if sig_id is already in the table - if not allocate and add it */
    sfthd_item = (THD_ITEM*)sfghash_find( sfthd_hash, (void*)&key );
    if( !sfthd_item )
    {
        /* Create the sfthd_item hash node data */
        sfthd_item = (THD_ITEM*)calloc(1,sizeof(THD_ITEM));
        if( !sfthd_item )
        {
            return -3;
        }

        sfthd_item->gen_id = config->gen_id;
        sfthd_item->sig_id = config->sig_id;
        sfthd_item->policyId = policy_id;
        sfthd_item->sfthd_node_list = sflist_new();

        if(!sfthd_item->sfthd_node_list)
        {
            free(sfthd_item);
            return -4;
        }

        /* Add the sfthd_item to the hash table */
        hstatus = sfghash_add( sfthd_hash, (void*)&key, sfthd_item );
        if( hstatus )
        {
            sflist_free(sfthd_item->sfthd_node_list);
            free(sfthd_item);
            return -5;
        }
    }

    /*
     * Test that we only have one Limit/Threshold/Both Object at the tail,
     * we can have multiple suppression nodes at the head
     */
    if( sfthd_item->sfthd_node_list->count > 0  )
    {
        THD_NODE * p;
        if( !sfthd_item->sfthd_node_list->tail)
        {
            /* can you say paranoid- if there is a count, there should be a tail */
            return -10;
        }
        p = (THD_NODE*)sfthd_item->sfthd_node_list->tail->ndata;
        if(p) /* just to be safe- if thers a tail, there is is node data */
        {
            if( p->type != THD_TYPE_SUPPRESS && config->type != THD_TYPE_SUPPRESS )
            {
#ifdef THD_DEBUG
                printf("THD_DEBUG: Could not add a 2nd Threshold object, "
                       "you can only have 1 per sid: gid=%u, sid=%u\n",
                       config->gen_id, config->sig_id);
#endif
                /* cannot add more than one threshold per sid in
                   version 3.0, wait for 3.2 and CIDR blocks */
                return THD_TOO_MANY_THDOBJ;
            }
        }
    }

    /* Create a THD_NODE for this THD_ITEM (Object) */
    sfthd_node = (THD_NODE*)calloc(1,sizeof(THD_NODE));
    if( !sfthd_node )
    {
        return -6;
    }

    /* Limit priorities to force supression nodes to highest priority */
    if( config->priority >= THD_PRIORITY_SUPPRESS )
    {
        config->priority  = THD_PRIORITY_SUPPRESS - 1;
    }

    /* Copy the node parameters */
    sfthd_node->thd_id    = config->thd_id;
    sfthd_node->gen_id    = config->gen_id;
    sfthd_node->sig_id    = config->sig_id;
    sfthd_node->tracking  = config->tracking; /* by_src, by_dst */
    sfthd_node->type      = config->type;
    sfthd_node->priority  = config->priority;
    sfthd_node->count     = config->count;
    sfthd_node->seconds   = config->seconds;
    sfthd_node->ip_address= config->ip_address;

    if( config->type == THD_TYPE_SUPPRESS )
    {
        sfthd_node->priority = THD_PRIORITY_SUPPRESS;
    }

    /*
      If sfthd_node list is empty - add as head node
    */
    if( !sfthd_item->sfthd_node_list->count )
    {
#ifdef THD_DEBUG
            printf("Threshold node added to head of list\n");fflush(stdout);
#endif
        sflist_add_head(sfthd_item->sfthd_node_list,sfthd_node);
    }

    /*
      else add the sfthd_node using priority to determine where in the list
      it belongs

      3.0 we can have only 1 threshold object but several suppression objects
      plus a single threshold object is ok.  Blocking multiple threshold
      objects is done above.

      Suppressions have the highest priority and are at the front of the
      list, the tail node is either a supprssion node or the only pure
      thresholding node.
    */
    else
    {
        SF_LNODE* lnode;

        /* Walk the list and insert based on priorities if suppress */
        for( lnode = sflist_first_node(sfthd_item->sfthd_node_list);
             lnode;
             lnode = sflist_next_node(sfthd_item->sfthd_node_list) )
        {
            THD_NODE* sfthd_n = (THD_NODE*)lnode->ndata;

            /* check if the new node is higher priority */
            if( sfthd_node->priority > sfthd_n->priority  )
            {
                /* insert before current node */
#ifdef THD_DEBUG
                printf("Threshold node added after based on priority\n");fflush(stdout);
#endif
                sflist_add_before(sfthd_item->sfthd_node_list,lnode,sfthd_node);
                return 0;
            }

            /* last node, just insert it here */
            if( !lnode->next  )
            {
                /* if last node, insert at end of list */
#ifdef THD_DEBUG
            printf("Threshold node added to tail\n");fflush(stdout);
#endif
                sflist_add_tail(sfthd_item->sfthd_node_list,sfthd_node);
                return 0;
            }
        }
    }

    return 0;
}


/*
 */
static int sfthd_create_threshold_global(SnortConfig *sc, ThresholdObjects *thd_objs,
                                         THD_NODE* config)
{
    THD_NODE *sfthd_node;
    tSfPolicyId policy_id = getParserPolicy(sc);

    if (thd_objs == NULL)
        return -1;

    if (thd_objs->sfthd_garray[policy_id] == NULL)
    {
        thd_objs->sfthd_garray[policy_id] = (THD_NODE **)(calloc(THD_MAX_GENID, sizeof(THD_NODE *)));
        if (thd_objs->sfthd_garray[policy_id] == NULL)
        {
            return -1;
        }
    }

    if ((config->gen_id == 0) &&
        (thd_objs->sfthd_garray[policy_id][config->gen_id] != NULL))
    {
        return THD_TOO_MANY_THDOBJ;
    }
    /* Reset the current threshold */
    if (thd_objs->sfthd_garray[policy_id][config->gen_id] ==
        thd_objs->sfthd_garray[policy_id][0])
    {
        thd_objs->sfthd_garray[policy_id][config->gen_id] = NULL;
    }
    else if(thd_objs->sfthd_garray[policy_id][config->gen_id])
    {
        return THD_TOO_MANY_THDOBJ;
    }

    sfthd_node = (THD_NODE*)calloc(1,sizeof(THD_NODE));
    if( !sfthd_node )
    {
        return -2;
    }

    /* Copy the node parameters */
    sfthd_node->thd_id = config->thd_id;
    sfthd_node->gen_id = config->gen_id;
    sfthd_node->sig_id = config->sig_id;      /* -1 for global thresholds */
    sfthd_node->tracking = config->tracking;  /* by_src, by_dst */
    sfthd_node->type = config->type;
    sfthd_node->priority = config->priority;
    sfthd_node->count = config->count;
    sfthd_node->seconds = config->seconds;
    sfthd_node->ip_address = config->ip_address;

    /* need a hash of these where
     * key=[gen_id,sig_id] => THD_GNODE_KEY
     * data = THD_NODE's
     */
    if (config->gen_id == 0) /* do em all */
    {
        int i;

        for (i = 0; i < THD_MAX_GENID; i++)
        {
            /* only assign if there wasn't a value */
            if (thd_objs->sfthd_garray[policy_id][i] == NULL)
            {
                thd_objs->sfthd_garray[policy_id][i] = sfthd_node;
            }
        }
    }
    else
    {
        thd_objs->sfthd_garray[policy_id][config->gen_id] = sfthd_node;
    }

#ifdef THD_DEBUG
    printf("THD_DEBUG-GLOBAL: created global threshold object "
           "for gen_id=%d\n",config->gen_id);
    fflush(stdout);
#endif

    return 0;
}

/*!
Add a permanent threshold object to the threshold table. Multiple
objects may be defined for each gen_id and sig_id pair. Internally
a unique threshold id is generated for each pair.

Threshold objects track the number of events seen during the time
interval specified by seconds. Depending on the type of threshold
object and the count value, the thresholding object determines if
the current event should be logged or dropped.

@param thd Threshold object from sfthd_new()
@param gen_id Generator id
@param sig_id Signauture id
@param tracking Selects tracking by src ip or by dst ip
@param type  Thresholding type: Limit, Threshold, or Limt+Threshold, Suppress
@param priority Assigns a relative priority to this object, higher numbers imply higher priority

@param count Number of events
@param seconds Time duration over which this threshold object acts.
@param ip      IP address, for supression
@param ip-mask IP mask, applied with ip_mask, for supression

@return integer
@retval  0 successfully added the thresholding object
@retval !0 failed

 --- Local and Global Thresholding is setup here  ---

*/
int sfthd_create_threshold(SnortConfig *sc,
                           ThresholdObjects *thd_objs,
                           unsigned gen_id,
                           unsigned sig_id,
                           int tracking,
                           int type,
                           int priority,
                           int count,
                           int seconds,
                           IpAddrSet* ip_address)
{
    //allocate memory fpr sfthd_array if needed.
    tSfPolicyId policyId = getParserPolicy(sc);
    THD_NODE sfthd_node;
    memset(&sfthd_node, 0, sizeof(sfthd_node));

    thd_objs->count++;

    sfthd_node.thd_id    = thd_objs->count;  /* produce a unique thd_id for this node */
    sfthd_node.gen_id    = gen_id;
    sfthd_node.sig_id    = sig_id;
    sfthd_node.tracking  = tracking; /* by_src, by_dst */
    sfthd_node.type      = type;
    sfthd_node.priority  = priority;
    sfthd_node.count     = count;
    sfthd_node.seconds   = seconds;
    sfthd_node.ip_address= ip_address;

    sfDynArrayCheckBounds ((void **)&thd_objs->sfthd_garray, policyId, &thd_objs->numPoliciesAllocated);
    if (thd_objs->sfthd_garray[policyId] == NULL)
    {
        thd_objs->sfthd_garray[policyId] = SnortAlloc(THD_MAX_GENID * sizeof(THD_NODE *));
        if (thd_objs->sfthd_garray[policyId] == NULL)
        {
            return -1;
        }
    }

    if( sig_id == 0 )
    {
        return sfthd_create_threshold_global(sc, thd_objs, &sfthd_node);
    }

    if( gen_id == 0 )
        return -1;

    return sfthd_create_threshold_local(sc, thd_objs, &sfthd_node);
}

#ifdef THD_DEBUG
static char * printIP(unsigned u )
{
    static char s[80];
    SnortSnprintf(s,80,"%d.%d.%d.%d", (u>>24)&0xff, (u>>16)&0xff, (u>>8)&0xff, u&0xff );
    return s;
}
#endif

int sfthd_test_rule(SFXHASH *rule_hash, THD_NODE *sfthd_node,
                    sfaddr_t* sip, sfaddr_t* dip, long curtime,
                    detection_option_eval_data_t *eval_data)
{
    int status;

    if ((rule_hash == NULL) || (sfthd_node == NULL))
        return 0;

    status = sfthd_test_local(rule_hash, sfthd_node, sip, dip, curtime, eval_data);

    return (status < -1) ? 1 : status;
}

static inline int sfthd_test_suppress (
    THD_NODE* sfthd_node,
    sfaddr_t* ip)
{
    if ( !sfthd_node->ip_address ||
         IpAddrSetContains(sfthd_node->ip_address, ip) )
    {
#ifdef THD_DEBUG
        printf("THD_DEBUG: SUPPRESS NODE, do not log events with this IP\n");
        fflush(stdout);
#endif
        /* Don't log, and stop looking( event's to this address
         * for this gen_id+sig_id) */
        sfthd_node->filtered++;
        return -1;
    }
    return 1; /* Keep looking for other suppressors */
}

/*
 *  Do the appropriate test for the Threshold Object Type
 */
static inline int sfthd_test_non_suppress(
    THD_NODE* sfthd_node,
    THD_IP_NODE* sfthd_ip_node,
    time_t curtime)
{
    unsigned dt;

    if( sfthd_node->type == THD_TYPE_DETECT )
    {
#ifdef THD_DEBUG
        printf("\n...Detect Test\n");
        fflush(stdout);
#endif
        dt = (unsigned)(curtime - sfthd_ip_node->tstart);

        if( dt >= sfthd_node->seconds )
        {   /* reset */
            sfthd_ip_node->tstart = curtime;
            if ( (unsigned)(curtime - sfthd_ip_node->tlast) > sfthd_node->seconds )
                sfthd_ip_node->prev = 0;
            else
                sfthd_ip_node->prev = sfthd_ip_node->count - 1;
            sfthd_ip_node->count = 1;
        }
        sfthd_ip_node->tlast = curtime;

#ifdef THD_DEBUG
        printf("...dt=%d, sfthd_node->seconds=%d\n",dt, sfthd_node->seconds );
        printf("...sfthd_ip_node->count=%d, sfthd_node->count=%d\n",
            sfthd_ip_node->count,sfthd_node->count );
        fflush(stdout);
#endif
        if( (int)sfthd_ip_node->count > sfthd_node->count ||
            (int)sfthd_ip_node->prev > sfthd_node->count )
        {
            return 0; /* Log it, stop looking: log all > 'count' events */
        }

        /* Don't Log yet, don't keep looking:
         * already logged our limit, don't log this sid  */
        sfthd_node->filtered++;
        return -2;
    }
    if( sfthd_node->type == THD_TYPE_LIMIT )
    {
#ifdef THD_DEBUG
        printf("\n...Limit Test\n");
        fflush(stdout);
#endif
        dt = (unsigned)(curtime - sfthd_ip_node->tstart);

        if( dt >= sfthd_node->seconds )
        {   /* reset */
            sfthd_ip_node->tstart = curtime;
            sfthd_ip_node->count  = 1;
        }

#ifdef THD_DEBUG
        printf("...dt=%d, sfthd_node->seconds=%d\n",dt, sfthd_node->seconds );
        printf("...sfthd_ip_node->count=%d, sfthd_node->count=%d\n",
            sfthd_ip_node->count,sfthd_node->count );
        fflush(stdout);
#endif
        if( (int)sfthd_ip_node->count <= sfthd_node->count )
        {
            return 0; /* Log it, stop looking: only log the 1st 'count' events */
        }

        /* Don't Log yet, don't keep looking:
         * already logged our limit, don't log this sid  */
        sfthd_node->filtered++;
        return -2;
    }

    else if( sfthd_node->type == THD_TYPE_THRESHOLD )
    {
#ifdef THD_DEBUG
        printf("\n...Threshold Test\n");
        fflush(stdout);
#endif
        dt = (unsigned)(curtime - sfthd_ip_node->tstart);
        if( dt >= sfthd_node->seconds )
        {
            sfthd_ip_node->tstart = curtime;
            sfthd_ip_node->count  = 1;
        }
        if( (int)sfthd_ip_node->count >= sfthd_node->count )
        {
            /* reset */
            sfthd_ip_node->count = 0;
            sfthd_ip_node->tstart= curtime;
            return 0; /* Log it, stop looking */
        }
        sfthd_node->filtered++;
        return -2; /* don't log yet */
    }

    else if( sfthd_node->type == THD_TYPE_BOTH )
    {
#ifdef THD_DEBUG
        printf("\n...Threshold+Limit Test\n");
        fflush(stdout);
#endif
        dt = (unsigned)(curtime - sfthd_ip_node->tstart);
        if( dt >= sfthd_node->seconds )
        {
            sfthd_ip_node->tstart = curtime;
            sfthd_ip_node->count  = 1;

            /* Don't Log yet, keep looking:
             * only log after we reach count, which must be > '1' */
            sfthd_node->filtered++;
            return -2;
        }
        else
        {
            if( (int)sfthd_ip_node->count >= sfthd_node->count )
            {
                if( (int)sfthd_ip_node->count >  sfthd_node->count )
                {
                    /* don't log it, stop looking:
                     * log once per time interval - than block it */
                    sfthd_node->filtered++;
                    return -2;
                }
                /* Log it, stop looking:
                 * log the 1st event we see past 'count' events */
                return 0;
            }
            else  /* Block it from logging */
            {
                /* don't log it, stop looking:
                 * we must see at least count events 1st */
                sfthd_node->filtered++;
                return -2;
            }
        }
    }

#ifdef THD_DEBUG
    printf("THD_DEBUG: You should not be here...\n");
    fflush(stdout);
#endif

    return 0;  /* should not get here, so log it just to be safe */
}

/*!
 *
 *  Find/Test/Add an event against a single threshold object.
 *  Events without thresholding objects are automatically loggable.
 *
 *  @param thd     Threshold table pointer
 *  @param sfthd_node Permanent Thresholding Object
 *  @param sip     Event/Packet Src IP address- should be host ordered for comparison
 *  @param dip     Event/Packet Dst IP address
 *  @param curtime Current Event/Packet time in seconds
 *
 *  @return  integer
 *  @retval   0 : Event is loggable
 *  @retval  >0 : Event should not be logged, try next thd object
 *  @retval  <0 : Event should never be logged to this user! Suppressed Event+IP
 *
 */
int sfthd_test_local(
    SFXHASH *local_hash,
    THD_NODE   * sfthd_node,
    sfaddr_t*    sip,
    sfaddr_t*    dip,
    time_t       curtime,
    detection_option_eval_data_t *eval_data)
{
    THD_IP_NODE_KEY key;
    THD_IP_NODE     data,*sfthd_ip_node;
    int             status=0;
    sfaddr_t*       ip;
    tSfPolicyId policy_id = getIpsRuntimePolicy();

#ifdef THD_DEBUG
    printf("THD_DEBUG: Key THD_NODE IP=%s,",printIP((unsigned)sfthd_node->ip_address) );
    printf(" MASK=%s\n",printIP((unsigned)sfthd_node->ip_mask) );
    printf("THD_DEBUG:        PKT  SIP=%s\n",printIP((unsigned)sip) );
    printf("THD_DEBUG:        PKT  DIP=%s\n",printIP((unsigned)dip) );
    fflush(stdout);
#endif

    /* -1 means don't do any limit or thresholding */
    if ( sfthd_node->count == THD_NO_THRESHOLD)
    {
#ifdef THD_DEBUG
        printf("\n...No Threshold applied for this object\n");
        fflush(stdout);
#endif
        return 0;
    }

    /*
     *  Get The correct IP
     */
    if (sfthd_node->tracking == THD_TRK_SRC)
       ip = sip;
    else
       ip = dip;

    /*
     *  Check for and test Suppression of this event to this IP
     */
    if( sfthd_node->type == THD_TYPE_SUPPRESS )
    {
#ifdef THD_DEBUG
        printf("THD_DEBUG: SUPPRESS NODE Testing...\n");fflush(stdout);
#endif
        return sfthd_test_suppress(sfthd_node, ip);
    }

    /*
    *  Go on and do standard thresholding
    */

    /* Set up the key */
    key.policyId = policy_id;
    sfaddr_copy_to_raw(&key.ip, ip);
    key.thd_id = sfthd_node->thd_id;

    /* Set up a new data element */
    data.count  = 0;
    data.prev   = 0;
    data.tstart = data.tlast = curtime; /* Event time */

    /*
     * Check for any Permanent sig_id objects for this gen_id  or add this one ...
     */
    status = sfxhash_add(local_hash, (void*)&key, &data);
    if (status == SFXHASH_INTABLE || status == SFXHASH_OK)
    {
        /* Already in the table */
        sfthd_ip_node = local_hash->cnode->data;

        /* Increment the event count */
        if(eval_data)
        {
             if(eval_data->detection_filter_count == 0)
             {
                 eval_data->detection_filter_count = 1;
                 sfthd_ip_node->count++;
             }
        }
        else
        {
            sfthd_ip_node->count++;
        }
    }
    else if (status != SFXHASH_OK)
    {
        /* hash error */
        return 1; /*  check the next threshold object */
    }
    else
    {  
        /* Was not in the table - it was added - work with our copy of the data */
        sfthd_ip_node = &data;
        if(eval_data)
        {
            eval_data->detection_filter_count = 1;
        }
    }
    return sfthd_test_non_suppress(sfthd_node, sfthd_ip_node, curtime);

}

/*
 *   Test a global thresholding object
 */
static inline int sfthd_test_global(
    SFXHASH *global_hash,
    THD_NODE   * sfthd_node,
    unsigned     gen_id,     /* from current event */
    unsigned     sig_id,     /* from current event */
    sfaddr_t*    sip,        /* " */
    sfaddr_t*    dip,        /* " */
    time_t       curtime )
{
    THD_IP_GNODE_KEY key;
    THD_IP_NODE      data, *sfthd_ip_node;
    int              status=0;
    sfaddr_t*        ip;
    tSfPolicyId policy_id = getIpsRuntimePolicy();

#ifdef THD_DEBUG
    printf("THD_DEBUG-GLOBAL:  gen_id=%u, sig_id=%u\n",gen_id,sig_id);
    printf("THD_DEBUG: Global THD_NODE IP=%s,",printIP((unsigned)sfthd_node->ip_address) );
    printf(" MASK=%s\n",printIP((unsigned)sfthd_node->ip_mask) );
    printf("THD_DEBUG:        PKT  SIP=%s\n",printIP((unsigned)sip) );
    printf("THD_DEBUG:        PKT  DIP=%s\n",printIP((unsigned)dip) );
    fflush(stdout);
#endif

    /* -1 means don't do any limit or thresholding */
    if ( sfthd_node->count == THD_NO_THRESHOLD)
    {
#ifdef THD_DEBUG
        printf("\n...No Threshold applied for this object\n");
        fflush(stdout);
#endif
        return 0;
    }

    /* Get The correct IP */
    if (sfthd_node->tracking == THD_TRK_SRC)
       ip = sip;
    else
       ip = dip;

    /* Check for and test Suppression of this event to this IP */
    if( sfthd_node->type == THD_TYPE_SUPPRESS )
    {
#ifdef THD_DEBUG
        printf("THD_DEBUG: G-SUPPRESS NODE Testing...\n");fflush(stdout);
#endif
        return sfthd_test_suppress(sfthd_node, ip);
    }

    /*
    *  Go on and do standard thresholding
    */

    /* Set up the key */
    sfaddr_copy_to_raw(&key.ip, ip);
    key.gen_id = sfthd_node->gen_id;
    key.sig_id = sig_id;
    key.policyId = policy_id;

    /* Set up a new data element */
    data.count  = 1;
    data.prev  = 0;
    data.tstart = data.tlast = curtime; /* Event time */

    /* Check for any Permanent sig_id objects for this gen_id  or add this one ...  */
    status = sfxhash_add(global_hash, (void*)&key, &data);
    if (status == SFXHASH_INTABLE)
    {
        /* Already in the table */
        sfthd_ip_node = global_hash->cnode->data;

        /* Increment the event count */
        sfthd_ip_node->count++;
    }
    else if (status != SFXHASH_OK)
    {
        /* hash error */
        return 1; /*  check the next threshold object */
    }
    else
    {
        /* Was not in the table - it was added - work with our copy of the data */
        sfthd_ip_node = &data;
    }

    return sfthd_test_non_suppress(sfthd_node, sfthd_ip_node, curtime);
}


/*!
 *
 *  Test a an event against the threshold database.
 *  Events without thresholding objects are automatically
 *  loggable.
 *
 *  @param thd     Threshold table pointer
 *  @param gen_id  Generator Id from the event
 *  @param sig_id  Signature Id from the event
 *  @param sip     Event/Packet Src IP address
 *  @param dip     Event/Packet Dst IP address
 *  @param curtime Current Event/Packet time
 *
 *  @return  integer
 *  @retval  0 : Event is loggable
 *  @retval !0 : Event should not be logged (-1 suppressed, 1 filtered)
 *
 */
int sfthd_test_threshold(
    ThresholdObjects *thd_objs,
    THD_STRUCT *thd,
    unsigned   gen_id,
    unsigned   sig_id,
    sfaddr_t*  sip,
    sfaddr_t*  dip,
    long       curtime )
{
    tThdItemKey key;
    SFGHASH  * sfthd_hash;
    THD_ITEM * sfthd_item;
    THD_NODE * sfthd_node;
    THD_NODE * g_thd_node = NULL;
#ifdef THD_DEBUG
    int cnt;
#endif
    int status=0;
    tSfPolicyId policy_id = getIpsRuntimePolicy();

    if ((thd_objs == NULL) || (thd == NULL))
        return 0;

#ifdef CRIPPLE
    return 0;
#endif

#ifdef THD_DEBUG
    printf("sfthd_test_threshold...\n");fflush(stdout);
#endif

    if( gen_id >= THD_MAX_GENID )
    {
#ifdef THD_DEBUG
        printf("THD_DEBUG: invalid gen_id=%u\n",gen_id);
        fflush(stdout);
#endif
        return 0; /* bogus gen_id */
    }

    /*
     *  Get the hash table for this gen_id
     */
    sfthd_hash = thd_objs->sfthd_array[gen_id];
    if (sfthd_hash == NULL)
    {
#ifdef THD_DEBUG
        printf("THD_DEBUG: no hash table entry for gen_id=%u\n",gen_id);
        fflush(stdout);
#endif
        goto global_test;
        /* return 0; */ /* no threshold objects for this gen_id, log it ! */
    }

    key.sig_id = sig_id;
    key.policyId = policy_id;
    /*
     * Check for any Permanent sig_id objects for this gen_id
     */
    sfthd_item = (THD_ITEM *)sfghash_find(sfthd_hash, (void*)&key);
    if (sfthd_item == NULL)
    {
#ifdef THD_DEBUG
        printf("THD_DEBUG: no THD objects for gen_id=%u, sig_id=%u\n",gen_id,sig_id);
        fflush(stdout);
#endif
        /* no matching permanent sig_id objects so, log it ! */
        goto global_test;
    }

    /* No List of Threshold objects - bail and log it */
    if (sfthd_item->sfthd_node_list == NULL)
    {
          goto global_test;
    }

    /* For each permanent thresholding object, test/add/update the thd object */
    /* We maintain a list of thd objects for each gen_id+sig_id */
    /* each object has it's own unique thd_id */
    /* Suppression nodes have a very high priority, so they are tested 1st */
#ifdef THD_DEBUG
    cnt=0;
#endif
    for (sfthd_node = (THD_NODE *)sflist_first(sfthd_item->sfthd_node_list);
         sfthd_node != NULL;
         sfthd_node = (THD_NODE *)sflist_next(sfthd_item->sfthd_node_list))
    {
#ifdef THD_DEBUG
        cnt++;
        printf("THD_DEBUG: gen_id=%u sig_id=%u testing thd_id=%d thd_type=%d\n",
                        gen_id, sig_id, sfthd_node->thd_id, sfthd_node->type);
        fflush(stdout);
#endif
        /*
         *   Test SUPPRESSION and THRESHOLDING
         */
        status = sfthd_test_local(thd->ip_nodes, sfthd_node, sip, dip, curtime, NULL);

        if( status < 0 ) /* -1 == Don't log and stop looking */
        {
#ifdef THD_DEBUG
            printf("THD_DEBUG: gen_id=%u sig_id=%u, UnLoggable\n\n",gen_id, sig_id,cnt);
            fflush(stdout);
#endif
            return (status < -1) ? 1 : -1;  /* !0 == Don't log it*/
        }
        else if( status == 0 )  /* Log it and stop looking */
        {
#ifdef THD_DEBUG
            printf("THD_DEBUG: gen_id=%u sig_id=%u tested %d THD_NODE's, "
                   "Loggable\n\n",sfthd_item->gen_id, sfthd_item->sig_id,cnt);
            fflush(stdout);
#endif
            return 0; /* 0 == Log the event */
        }
        /* status > 0 : Log it later but Keep looking
         * check the next threshold object for a blocking action ...
         */
    }


    /*
     * Test for a global threshold object
     * we're here cause ther were no threshold objects for this gen_id/sig_id pair
     */
global_test:

#ifdef THD_DEBUG
    printf("THD_DEBUG-GLOBAL: doing global object test\n");
#endif

     if (thd_objs->sfthd_garray && thd_objs->sfthd_garray[policy_id])
     {
         g_thd_node = thd_objs->sfthd_garray[policy_id][gen_id];
     }

     if( g_thd_node )
     {
         status = sfthd_test_global(
             thd->ip_gnodes, g_thd_node, gen_id, sig_id, sip, dip, curtime );

         if( status < 0 ) /* -1 == Don't log and stop looking */
         {
#ifdef THD_DEBUG
            printf("THD_DEBUG-GLOBAL: gen_id=%u sig_id=%u THD_NODE's, "
                   "UnLoggable\n\n",gen_id, sig_id);
            fflush(stdout);
#endif
            return (status < -1) ? 1 : -1;  /* !0 == Don't log it*/
         }

         /* Log it ! */
#ifdef THD_DEBUG
        printf("THD_DEBUG-GLOBAL: gen_id=%u sig_id=%u  THD_NODE's, "
               "Loggable\n\n",gen_id, sig_id);
        fflush(stdout);
#endif
     }
     else
     {
#ifdef THD_DEBUG
        printf("THD_DEBUG-GLOBAL: no Global THD Object for gen_id=%u, "
               "sig_id=%u\n\n",gen_id, sig_id);
        fflush(stdout);
#endif
     }

    return 0; /* Default: Log it if we did not block the logging action */
}

#ifdef THD_DEBUG
/*!
 *   A function to print the thresholding objects to stdout.
 *
 */
int sfthd_show_objects(ThresholdObjects *thd_objs)
{
    SFGHASH  * sfthd_hash;
    THD_ITEM * sfthd_item;
    THD_NODE * sfthd_node;
    int        gen_id;
    SFGHASH_NODE * item_hash_node;
    tSfPolicyId policyId;

    for(gen_id=0;gen_id < THD_MAX_GENID ; gen_id++ )
    {
        sfthd_hash = thd_objs->sfthd_array[gen_id];
        if (sfthd_hash == NULL)
            continue;

        printf("...GEN_ID = %u\n",gen_id);

        for(item_hash_node  = sfghash_findfirst( sfthd_hash );
            item_hash_node != 0;
            item_hash_node  = sfghash_findnext( sfthd_hash ) )
        {
            /* Check for any Permanent sig_id objects for this gen_id */
            sfthd_item = (THD_ITEM*)item_hash_node->data;

            printf(".....GEN_ID = %u, SIG_ID = %u, Policy = %u\n",gen_id,sfthd_item->sig_id, sfthd_item->policyId);

            /* For each permanent thresholding object, test/add/update the thd object */
            /* We maintain a list of thd objects for each gen_id+sig_id */
            /* each object has it's own unique thd_id */

            for( sfthd_node  = (THD_NODE*)sflist_first(sfthd_item->sfthd_node_list);
                 sfthd_node != 0;
                 sfthd_node = (THD_NODE*)sflist_next(sfthd_item->sfthd_node_list) )
            {
                printf(".........THD_ID  =%d\n",sfthd_node->thd_id );

                if( sfthd_node->type == THD_TYPE_SUPPRESS )
                    printf(".........type    =Suppress\n");
                if( sfthd_node->type == THD_TYPE_LIMIT )
                    printf(".........type    =Limit\n");
                if( sfthd_node->type == THD_TYPE_THRESHOLD )
                    printf(".........type    =Threshold\n");
                if( sfthd_node->type == THD_TYPE_BOTH )
                    printf(".........type    =Both\n");

                printf(".........tracking=%d\n",sfthd_node->tracking);
                printf(".........priority=%d\n",sfthd_node->priority);

                if( sfthd_node->type == THD_TYPE_SUPPRESS )
                {
                    printf(".........ip      =%s\n",
                           sfip_to_str(&sfthd_node->ip_address));
                    printf(".........mask    =%d\n",
                           sfip_bits(&sfthd_node->ip_address));
                    printf(".........not_flag=%d\n",sfthd_node->ip_mask);
                }
                else
                {
                    printf(".........count   =%d\n",sfthd_node->count);
                    printf(".........seconds =%d\n",sfthd_node->seconds);
                }
            }
        }
    }

    return 0;
}
#endif // THD_DEBUG

