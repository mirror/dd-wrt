/* $Id$ */
/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2003-2013 Sourcefire, Inc.
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
/*
   sfthreshold.c

   This file contains functions that glue the generic thresholding2 code to
   snort.

   dependent files:  sfthd sfxghash sfghash sflsq
                     util mstring

   Marc Norton

   2003-05-29:
     cmg: Added s_checked variable  --
       when this is 1, the sfthreshold_test will always return the same
       answer until
       sfthreshold_reset is called

   2003-11-3:
     man: cleaned up and added more startup printout.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mstring.h"
#include "util.h"
#include "parser.h"

#include "sfthd.h"
#include "sfthreshold.h"
#include "snort.h"

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <errno.h>

/* Data */
THD_STRUCT *thd_runtime = NULL;

static int thd_checked = 0;
static int thd_answer = 0;

typedef enum { PRINT_GLOBAL, PRINT_LOCAL, PRINT_SUPPRESS } PrintFormat;

ThresholdConfig * ThresholdConfigNew(void)
{
    ThresholdConfig *tc =
        (ThresholdConfig *)SnortAlloc(sizeof(ThresholdConfig));

    /* sfthd_objs_new will handle fatal */
    tc->thd_objs = sfthd_objs_new();
    tc->memcap = 1024 * 1024;
    tc->enabled = 1;

    return tc;
}

void ThresholdConfigFree(ThresholdConfig *tc)
{
    if (tc == NULL)
        return;

    if (tc->thd_objs != NULL)
    {
        sfthd_objs_free(tc->thd_objs);
        tc->thd_objs = NULL;
    }

    free(tc);
}

// prnMode = 0: init output format
// prnMode = 1: term output format (with header and count of filtered events)
// prnMode = 2: term output format (count only)
static int print_thd_node( THD_NODE *p , PrintFormat type, unsigned* prnMode )
{
    char buf[STD_BUF+1];
    memset(buf, 0, STD_BUF+1);

    switch( type )
    {
    case PRINT_GLOBAL:
           if(p->type == THD_TYPE_SUPPRESS ) return 0;
           if(p->sig_id != 0 ) return 0;
           break;

    case PRINT_LOCAL:
           if(p->type == THD_TYPE_SUPPRESS ) return 0;
           if(p->sig_id == 0 || p->gen_id == 0 ) return 0;
           break;

    case PRINT_SUPPRESS:
           if(p->type != THD_TYPE_SUPPRESS ) return 0;
           break;
    }

    /* SnortSnprintfAppend(buf, STD_BUF, "| thd-id=%d", p->thd_id ); */


    if ( *prnMode && !p->filtered )
        return 1;

    if( p->gen_id == 0 )
    {
        SnortSnprintfAppend(buf, STD_BUF, "| gen-id=global");
    }
    else
    {
        SnortSnprintfAppend(buf, STD_BUF, "| gen-id=%-6d", p->gen_id );
    }
    if( p->sig_id == 0 )
    {
        SnortSnprintfAppend(buf, STD_BUF, " sig-id=global" );
    }
    else
    {
        SnortSnprintfAppend(buf, STD_BUF, " sig-id=%-10d", p->sig_id );
    }

    switch ( p->type )
    {
        case THD_TYPE_LIMIT:
            SnortSnprintfAppend(buf, STD_BUF, " type=Limit    ");
            break;

        case THD_TYPE_THRESHOLD:
            SnortSnprintfAppend(buf, STD_BUF, " type=Threshold");
            break;

        case THD_TYPE_BOTH:
            SnortSnprintfAppend(buf, STD_BUF, " type=Both     ");
            break;

        case THD_TYPE_SUPPRESS:
            if ( *prnMode )
                SnortSnprintfAppend(buf, STD_BUF, " type=Suppress ");
            break;
    }

    switch ( p->tracking )
    {
        case THD_TRK_NONE:
            SnortSnprintfAppend(buf, STD_BUF, " tracking=none");
            break;

        case THD_TRK_SRC:
            SnortSnprintfAppend(buf, STD_BUF, " tracking=src");
            break;

        case THD_TRK_DST:
            SnortSnprintfAppend(buf, STD_BUF, " tracking=dst");
            break;
    }

    if( p->type == THD_TYPE_SUPPRESS )
    {
        if ( p->tracking != THD_TRK_NONE )
        {
            // TBD output suppress node ip addr set
            SnortSnprintfAppend(buf, STD_BUF, "-ip=%-16s", "<list>");
        }
    }
    else
    {
        SnortSnprintfAppend(buf, STD_BUF, " count=%-3d", p->count);
        SnortSnprintfAppend(buf, STD_BUF, " seconds=%-3d", p->seconds);
    }

    if ( *prnMode )
    {
        if ( *prnMode == 1 )
        {
            LogMessage("+-----------------------[filtered events]--------------------------------------\n");
            *prnMode = 2;
        }
        SnortSnprintfAppend(buf, STD_BUF, " filtered=" STDu64, p->filtered);
    }
    LogMessage("%s\n", buf);

    return 1;
}
/*
 *
 */
static int print_thd_local(ThresholdObjects *thd_objs, PrintFormat type, unsigned* prnMode)
{
    SFGHASH  * sfthd_hash;
    THD_ITEM * sfthd_item;
    THD_NODE * sfthd_node;
    int        gen_id;
    SFGHASH_NODE * item_hash_node;
    int        lcnt=0;
    tSfPolicyId policyId;

    for (policyId = 0; policyId < thd_objs->numPoliciesAllocated; policyId++)
    {
        for(gen_id=0;gen_id < THD_MAX_GENID ; gen_id++ )
        {
            sfthd_hash = thd_objs->sfthd_array[gen_id];
            if( !sfthd_hash )
            {
                continue;
            }

            for(item_hash_node  = sfghash_findfirst( sfthd_hash );
                    item_hash_node != 0;
                    item_hash_node  = sfghash_findnext( sfthd_hash ) )
            {
                /* Check for any Permanent sig_id objects for this gen_id */
                sfthd_item = (THD_ITEM*)item_hash_node->data;

                if (sfthd_item->policyId != policyId)
                {
                    continue;
                }

                for( sfthd_node  = (THD_NODE*)sflist_first(sfthd_item->sfthd_node_list);
                        sfthd_node != 0;
                        sfthd_node = (THD_NODE*)sflist_next(sfthd_item->sfthd_node_list) )
                {
                    if (print_thd_node(sfthd_node, type, prnMode) != 0)
                        lcnt++;
                }
            }
        }
    }

    if( !lcnt && !*prnMode )
        LogMessage("| none\n");

    return 0;
}


/*
 *  Startup/Shutdown Display Of Thresholding
 */
void print_thresholding(ThresholdConfig *thd_config, unsigned shutdown)
{
    int i;
    THD_NODE * thd;

    if (thd_config == NULL)
        return;

    if ( !shutdown )
    {
        LogMessage("\n");
        LogMessage("+-----------------------[event-filter-config]----------------------------------\n");
        LogMessage("| memory-cap : %d bytes\n",thd_config->memcap);
        LogMessage("+-----------------------[event-filter-global]----------------------------------\n");
    }

    if (thd_config->thd_objs == NULL)
    {
        if ( !shutdown )
            LogMessage("| none\n");
    }
    else
    {
        tSfPolicyId policyId;
        int gcnt=0;

        for (policyId = 0; policyId < thd_config->thd_objs->numPoliciesAllocated; policyId++)
        {
            if (thd_config->thd_objs->sfthd_garray[policyId] == NULL)
            {
                continue;
            }

            for(i=0;i<THD_MAX_GENID;i++)
            {
                thd = thd_config->thd_objs->sfthd_garray[policyId][i];
                if( !thd ) continue;
                gcnt++;
            }

            if( !gcnt )
            {
                if ( !shutdown )
                    LogMessage("| none\n");
            }
            /* display gen_id=global  and sig_id=global rules */
            if( gcnt )
            {
                for(i=0;i<THD_MAX_GENID;i++)
                {
                    thd = thd_config->thd_objs->sfthd_garray[policyId][i];
                    if( !thd ) continue;

                    if( thd->gen_id == 0 && thd->sig_id == 0 )
                    {
                        print_thd_node( thd, PRINT_GLOBAL, &shutdown );
                        break;
                    }
                }
            }

            /* display gen_id!=global and sig_id=global rules */
            if( gcnt )
            {
                for(i=0;i<THD_MAX_GENID;i++)
                {
                    thd = thd_config->thd_objs->sfthd_garray[policyId][i];
                    if( !thd ) continue;

                    if( thd->gen_id !=0 ||  thd->sig_id != 0 )
                    {
                        print_thd_node( thd, PRINT_GLOBAL, &shutdown );
                    }
                }
            }
        }
    }

    if ( !shutdown )
        LogMessage("+-----------------------[event-filter-local]-----------------------------------\n");
    if (thd_config->thd_objs == NULL)
    {
        if ( !shutdown )
            LogMessage("| none\n");
    }
    else
    {
        print_thd_local(thd_config->thd_objs, PRINT_LOCAL, &shutdown);
    }

    if ( !shutdown )
        LogMessage("+-----------------------[suppression]------------------------------------------\n");
    if (thd_config->thd_objs == NULL)
    {
        if ( !shutdown )
            LogMessage("| none\n");
    }
    else
    {
        print_thd_local(thd_config->thd_objs, PRINT_SUPPRESS, &shutdown);
    }

    if ( !shutdown )
        LogMessage("--------------------------------------------"
               "-----------------------------------\n");

}

void sfthreshold_free(void)
{
    if (thd_runtime != NULL)
        sfthd_free(thd_runtime);

    thd_runtime = NULL;
}

/*

    Create and Add a Thresholding Event Object

*/
int sfthreshold_create(struct _SnortConfig *sc, ThresholdConfig *thd_config, THDX_STRUCT *thdx)
{
    if (thd_config == NULL)
        return -1;

    if (!thd_config->enabled)
        return 0;

    /* Auto init - memcap must be set 1st, which is not really a problem */
    if (thd_runtime == NULL)
    {
        thd_runtime = sfthd_new(thd_config->memcap, thd_config->memcap);
        if (thd_runtime == NULL)
            return -1;
    }

    /* print_thdx( thdx ); */

    /* Add the object to the table - */
    return sfthd_create_threshold(sc,
                                  thd_config->thd_objs,
                                  thdx->gen_id,
                                  thdx->sig_id,
                                  thdx->tracking,
                                  thdx->type,
                                  thdx->priority,
                                  thdx->count,
                                  thdx->seconds,
                                  thdx->ip_address);
}

/*
    Test an event against the threshold object table
    to determine if it should be logged.

    It will always return the same answer until sfthreshold_reset is
    called

    gen_id:
    sig_id:
    sip:    host ordered sip
    dip:    host ordered dip
    curtime:

    2003-05-29 cmg:

     This code is in use in fpLogEvent, CallAlertFuncs, CallLogFuncs
     and the reset function is called in ProcessPacket


    returns 0 - log
           !0 - don't log
*/
int sfthreshold_test(
    unsigned gen_id, unsigned  sig_id,
    sfaddr_t* sip, sfaddr_t* dip,
    long curtime )
{
    if ((snort_conf->threshold_config == NULL) ||
        !snort_conf->threshold_config->enabled)
    {
        return 0;
    }

    if (!thd_checked)
    {
       thd_checked = 1;
       thd_answer = sfthd_test_threshold(snort_conf->threshold_config->thd_objs,
                                         thd_runtime, gen_id, sig_id, sip, dip, curtime);
    }

    return thd_answer;
}

/**
 * Reset the thresholding system so that subsequent calls to
 * sfthreshold_test will indeed try to alter the thresholding system
 *
 */
void sfthreshold_reset(void)
{
    thd_checked = 0;
}

/* empty out active entries */
void sfthreshold_reset_active(void)
{
    if (thd_runtime == NULL)
        return;

    if (thd_runtime->ip_nodes != NULL)
        sfxhash_make_empty(thd_runtime->ip_nodes);

    if (thd_runtime->ip_gnodes != NULL)
        sfxhash_make_empty(thd_runtime->ip_gnodes);
}

