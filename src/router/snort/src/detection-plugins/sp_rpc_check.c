/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
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

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#ifndef WIN32
#include <rpc/rpc.h>
#endif /* !WIN32 */

#include "rules.h"
#include "treenodes.h"
#include "decode.h"
#include "plugbase.h"
#include "parser.h"
#include "snort_debug.h"
#include "util.h"
#include "plugin_enum.h"
#include "sfhashfcn.h"

#include "snort.h"
#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats rpcCheckPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "sfhashfcn.h"
#include "detection_options.h"

/*
 * This is driven by 64-bit Solaris which doesn't
 * define _LONG
 *
 */

#ifndef IXDR_GET_LONG
    #define IXDR_GET_LONG IXDR_GET_INT32
#endif

#define GET_32BITS(buf) (ntohl(*(*(uint32_t **)&(buf))++))

typedef struct _RpcCheckData
{
    u_long program; /* RPC program number */
    u_long vers; /* RPC program version */
    u_long proc; /* RPC procedure number */
    int flags; /* Which of the above fields have been specified */

} RpcCheckData;

#define RPC_CHECK_PROG 1
#define RPC_CHECK_VERS 2
#define RPC_CHECK_PROC 4

void RpcCheckInit(struct _SnortConfig *, char *, OptTreeNode *, int);
void ParseRpc(struct _SnortConfig *, char *, OptTreeNode *);
int CheckRpc(void *option_data, Packet *p);

uint32_t RpcCheckHash(void *d)
{
    uint32_t a,b,c;
    RpcCheckData *data = (RpcCheckData *)d;

    a = data->program;
    b = data->vers;
    c = data->proc;

    mix(a,b,c);

    a += data->flags;
    b += RULE_OPTION_TYPE_RPC_CHECK;

    final(a,b,c);

    return c;
}

int RpcCheckCompare(void *l, void *r)
{
    RpcCheckData *left = (RpcCheckData *)l;
    RpcCheckData *right = (RpcCheckData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if ((left->program == right->program) &&
        (left->vers == right->vers) &&
        (left->proc == right->proc) &&
        (left->flags == right->flags))
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}


/****************************************************************************
 *
 * Function: SetupRpcCheck()
 *
 * Purpose: Register the rpc option keyword with its setup function
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupRpcCheck(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("rpc", RpcCheckInit, NULL, OPT_TYPE_DETECTION, NULL);

#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("rpc", &rpcCheckPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Plugin: RPCCheck Initialized\n"););
}


/****************************************************************************
 *
 * Function: RpcCheckInit(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Parse the rpc keyword arguments and link the detection module
 *          into the function list
 *
 * Arguments: data => rule arguments/data
 *            otn => pointer to the current rule option list node
 *
 * Returns: void function
 *
 ****************************************************************************/
void RpcCheckInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    OptFpList *fpl;
    if(protocol != IPPROTO_TCP && protocol != IPPROTO_UDP)
    {
        FatalError("%s(%d) => Bad protocol in RPC Check rule...\n",
                   file_name, file_line);
    }

    /* multiple declaration check */
    if(otn->ds_list[PLUGIN_RPC_CHECK])
    {
        FatalError("%s(%d): Multiple rpc options in rule\n", file_name,
                file_line);
    }

    /* allocate the data structure and attach it to the
       rule's data struct list */
    otn->ds_list[PLUGIN_RPC_CHECK] = (RpcCheckData *)
            SnortAlloc(sizeof(RpcCheckData));

    /* this is where the keyword arguments are processed and placed into the
       rule option's data structure */
    ParseRpc(sc, data, otn);

    /* finally, attach the option's detection function to the rule's
       detect function pointer list */
    fpl = AddOptFuncToList(CheckRpc, otn);
    fpl->type = RULE_OPTION_TYPE_RPC_CHECK;
    fpl->context = otn->ds_list[PLUGIN_RPC_CHECK];
}



/****************************************************************************
 *
 * Function: ParseRpc(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Parse the RPC keyword's arguments
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: void function
 *
 ****************************************************************************/
void ParseRpc(struct _SnortConfig *sc, char *data, OptTreeNode *otn)
{
    RpcCheckData *ds_ptr;  /* data struct pointer */
    void *ds_ptr_dup;
    char *tmp = NULL;

    /* set the ds pointer to make it easier to reference the option's
       particular data struct */
    ds_ptr = otn->ds_list[PLUGIN_RPC_CHECK];
    ds_ptr->flags=0;

    /* advance past whitespace */
    while(isspace((int)*data)) data++;

    if(*data != '*')
    {
        ds_ptr->program = strtoul(data,&tmp,0);
        ds_ptr->flags|=RPC_CHECK_PROG;
        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Set RPC program to %lu\n", ds_ptr->program););
    }
    else
    {
        FatalError("%s(%d): Invalid applicaion number in rpc rule option\n",file_name,file_line);
    }

    if(*tmp == '\0') return;

    data=++tmp;
    if(*data != '*')
    {
        ds_ptr->vers = strtoul(data,&tmp,0);
        ds_ptr->flags|=RPC_CHECK_VERS;
        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Set RPC vers to %lu\n", ds_ptr->vers););
    }
    else
    {
        tmp++;
    }
    if(*tmp == '\0') return;
    data=++tmp;
    if(*data != '*')
    {
        ds_ptr->proc = strtoul(data,&tmp,0);
        ds_ptr->flags|=RPC_CHECK_PROC;
        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Set RPC proc to %lu\n", ds_ptr->proc););
    }

    if (add_detection_option(sc, RULE_OPTION_TYPE_RPC_CHECK, (void *)ds_ptr, &ds_ptr_dup) == DETECTION_OPTION_EQUAL)
    {
        free(ds_ptr);
        ds_ptr = otn->ds_list[PLUGIN_RPC_CHECK] = ds_ptr_dup;
     }

}

/****************************************************************************
 *
 * Function: CheckRpc(char *, OptTreeNode *)
 *
 * Purpose: Test if the packet RPC equals the rule option's rpc
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: 0 on failure, return value of next list function on success
 *
 ****************************************************************************/
int CheckRpc(void *option_data, Packet *p)
{
    RpcCheckData *ds_ptr = (RpcCheckData *)option_data;
    unsigned char* c=(unsigned char*)p->data;
    u_long rpcvers, prog, vers, proc;
    enum msg_type direction;
    int rval = DETECTION_OPTION_NO_MATCH;
#ifdef DEBUG_MSGS
    int i;
#endif
    PROFILE_VARS;

    if(!p->iph_api || (IsTCP(p) && !p->tcph)
       || (IsUDP(p) && !p->udph))
        return 0; /* if error occured while ip header
                   * was processed, return 0 automagically.  */

    PREPROC_PROFILE_START(rpcCheckPerfStats);

    if( IsTCP(p) )
    {
        /* offset to rpc_msg */
        c+=4;
        /* Fail if the packet is too short to match */
        if(p->dsize<28)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "RPC packet too small"););
            PREPROC_PROFILE_END(rpcCheckPerfStats);
            return rval;
        }
    }
    else
    { /* must be UDP */
        /* Fail if the packet is too short to match */
        if(p->dsize<24)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "RPC packet too small"););
            PREPROC_PROFILE_END(rpcCheckPerfStats);
            return rval;
        }
    }

#ifdef DEBUG_MSGS
    DebugMessage(DEBUG_PLUGIN,"<---xid---> <---dir---> <---rpc--->"
                              " <---prog--> <---vers--> <---proc-->\n");
    for(i=0; i<24; i++)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "%02X ",c[i]););
    }
    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"\n"););
#endif

    /* Read xid */
    GET_32BITS(c);

    /* Read direction : CALL or REPLY */
    direction = (enum msg_type)GET_32BITS(c);

    /* We only look at calls */
    if(direction != CALL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "RPC packet not a call"););
        PREPROC_PROFILE_END(rpcCheckPerfStats);
        return rval;
    }

    /* Read the RPC message version */
      rpcvers = GET_32BITS(c);

    /* Fail if it is not right */
    if(rpcvers != RPC_MSG_VERSION)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"RPC msg version invalid"););
        PREPROC_PROFILE_END(rpcCheckPerfStats);
        return rval;
    }

    /* Read the program number, version, and procedure */
    prog = GET_32BITS(c);
    vers = GET_32BITS(c);
    proc = GET_32BITS(c); 

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"RPC decoded to: %lu %lu %lu\n",
                            prog,vers,proc););

    DEBUG_WRAP(
           DebugMessage(DEBUG_PLUGIN, "RPC matching on: %d %d %d\n",
                ds_ptr->flags & RPC_CHECK_PROG,ds_ptr->flags & RPC_CHECK_VERS,
                ds_ptr->flags & RPC_CHECK_PROC););
    if(!(ds_ptr->flags & RPC_CHECK_PROG) ||
       ds_ptr->program == prog)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"RPC program matches"););
        if(!(ds_ptr->flags & RPC_CHECK_VERS) ||
           ds_ptr->vers == vers)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"RPC version matches"););
            if(!(ds_ptr->flags & RPC_CHECK_PROC) ||
               ds_ptr->proc == proc)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"RPC proc matches"););
                DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Yippee! Found one!"););
                rval = DETECTION_OPTION_MATCH;
            }
        }
    }
    else
    {
        /* you can put debug comments here or not */
        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"RPC not equal\n"););
    }

    /* if the test isn't successful, return 0 */
    PREPROC_PROFILE_END(rpcCheckPerfStats);
    return rval;
}
