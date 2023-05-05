/* $Id$ */
/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2002-2013 Sourcefire, Inc.
 ** Author: Martin Roesch
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

/* sp_clientserver
 *
 * Purpose:
 *
 * Wouldn't be nice if we could tell a TCP rule to only apply if it's going
 * to or from the client or server side of a connection?  Think of all the
 * false alarms we could elminate!  That's what we're doing with this one,
 * it allows you to write rules that only apply to client or server packets.
 * One thing though, you *must* have stream4 enabled for it to work!
 *
 * Arguments:
 *
 *   None.
 *
 * Effect:
 *
 * Test the packet to see if it's coming from the client or the server side
 * of a connection.
 *
 * Comments:
 *
 * None.
 *
 */

/* put the name of your pluging header file here */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "sf_types.h"
#include "rules.h"
#include "treenodes.h"
#include "decode.h"
#include "plugbase.h"
#include "parser.h"
#include "snort_debug.h"
#include "util.h"
#include "plugin_enum.h"
#include "snort.h"
//#include "signature.h"
#include "sfhashfcn.h"
#include "sp_clientserver.h"

#include "stream_api.h"

#include "snort.h"
#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats flowCheckPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "sfhashfcn.h"
#include "detection_options.h"

void FlowInit(struct _SnortConfig *, char *, OptTreeNode *, int);
void ParseFlowArgs(struct _SnortConfig *, char *, OptTreeNode *);
void InitFlowData(OptTreeNode *);
int CheckFlow(void *option_data, Packet *p);

uint32_t FlowHash(void *d)
{
    uint32_t a,b,c;
    ClientServerData *data = (ClientServerData *)d;

    a = data->from_server || data->from_client << 16;
    b = data->ignore_reassembled || data->only_reassembled << 16;
    c = data->stateless || data->established << 16;

    mix(a,b,c);

    a += data->unestablished;
    b += RULE_OPTION_TYPE_FLOW;

    final(a,b,c);

    return c;
}

int FlowCompare(void *l, void *r)
{
    ClientServerData *left = (ClientServerData *)l;
    ClientServerData *right = (ClientServerData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if (( left->from_server == right->from_server) &&
        ( left->from_client == right->from_client) &&
        ( left->ignore_reassembled == right->ignore_reassembled) &&
        ( left->only_reassembled == right->only_reassembled) &&
        ( left->stateless == right->stateless) &&
        ( left->established == right->established) &&
        ( left->unestablished == right->unestablished))
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}

int OtnFlowFromServer( OptTreeNode * otn )
{
    ClientServerData *csd;

    csd = (ClientServerData *)otn->ds_list[PLUGIN_CLIENTSERVER];
    if(csd )
    {
        if( csd->from_server ) return 1;
    }
    return 0;
}
int OtnFlowFromClient( OptTreeNode * otn )
{
    ClientServerData *csd;

    csd = (ClientServerData *)otn->ds_list[PLUGIN_CLIENTSERVER];
    if(csd )
    {
        if( csd->from_client ) return 1;
    }
    return 0;
}
int OtnFlowIgnoreReassembled( OptTreeNode * otn )
{
    ClientServerData *csd;

    csd = (ClientServerData *)otn->ds_list[PLUGIN_CLIENTSERVER];
    if( csd )
    {
        if( csd->ignore_reassembled ) return 1;
    }
    return 0;
}
int OtnFlowOnlyReassembled( OptTreeNode * otn )
{
    ClientServerData *csd;

    csd = (ClientServerData *)otn->ds_list[PLUGIN_CLIENTSERVER];
    if( csd )
    {
        if( csd->only_reassembled ) return 1;
    }
    return 0;
}

/****************************************************************************
 *
 * Function: SetupClientServer()
 *
 * Purpose: Generic detection engine plugin template.  Registers the
 *          configuration function and links it to a rule keyword.  This is
 *          the function that gets called from InitPlugins in plugbase.c.
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupClientServer(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("flow", FlowInit, NULL, OPT_TYPE_DETECTION, NULL);

#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("flow", &flowCheckPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,
                            "Plugin: ClientServerName(Flow) Setup\n"););
}


/****************************************************************************
 *
 * Function: FlowInit(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Configure the flow init option to register the appropriate checks
 *
 * Arguments: data => rule arguments/data
 *            otn => pointer to the current rule option list node
 *
 * Returns: void function
 *
 ****************************************************************************/
void FlowInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    ClientServerData *csd;
    /* multiple declaration check */
    if(otn->ds_list[PLUGIN_CLIENTSERVER])
    {
        FatalError("%s(%d): Multiple flow options in rule\n", file_name,
                file_line);
    }


    InitFlowData(otn);
    ParseFlowArgs(sc, data, otn);
    csd = (ClientServerData *)otn->ds_list[PLUGIN_CLIENTSERVER];

    if(protocol == IPPROTO_UDP)
    {
        if (!stream_api || (stream_api->version != STREAM_API_VERSION5))
        {
            FatalError("%s(%d): Cannot check flow connection "
                   "for UDP traffic\n", file_name, file_line);
        }
    }

    if (protocol == IPPROTO_ICMP)
    {
        if ((csd->only_reassembled != ONLY_FRAG) && (csd->ignore_reassembled != IGNORE_FRAG))
        {
            FatalError("%s(%d): Cannot check flow connection "
                   "for ICMP traffic\n", file_name, file_line);
        }
    }
}


static inline void CheckStream(char *token)
{
    if (!stream_api)
    {
        FatalError("%s(%d): Stream must be enabled to use the '%s' option.\n",
            file_name, file_line, token);
    }
}

/****************************************************************************
 *
 * Function: ParseFlowArgs(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: parse the arguments to the flow plugin and alter the otn
 *          accordingly
 *
 * Arguments: otn => pointer to the current rule option list node
 *
 * Returns: void function
 *
 ****************************************************************************/
void ParseFlowArgs(struct _SnortConfig *sc, char *data, OptTreeNode *otn)
{
    char *token, *str, *p;
    ClientServerData *csd;
    void *idx_dup;
    OptFpList *fpl = NULL;

    csd = (ClientServerData *)otn->ds_list[PLUGIN_CLIENTSERVER];

    str = SnortStrdup(data);

    p = str;

    /* nuke leading whitespace */
    while(isspace((int)*p)) p++;

    token = strtok(p, ",");

    while(token)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,
                    "parsed %s,(%d)\n", token,strlen(token)););

        while(isspace((int)*token))
            token++;

        if(!strcasecmp(token, "to_server"))
        {
            CheckStream(token);
            csd->from_client = 1;
        }
        else if(!strcasecmp(token, "to_client"))
        {
            CheckStream(token);
            csd->from_server = 1;
        }
        else if(!strcasecmp(token, "from_server"))
        {
            CheckStream(token);
            csd->from_server = 1;
        }
        else if(!strcasecmp(token, "from_client"))
        {
            CheckStream(token);
            csd->from_client = 1;
        }
        else if(!strcasecmp(token, "stateless"))
        {
            csd->stateless = 1;
            otn->stateless = 1;
        }
        else if(!strcasecmp(token, "established"))
        {
            CheckStream(token);
            csd->established = 1;
            otn->established = 1;
        }
        else if(!strcasecmp(token, "not_established"))
        {
            CheckStream(token);
            csd->unestablished = 1;
            otn->unestablished = 1;
        }
        else if(!strcasecmp(token, "no_stream"))
        {
            CheckStream(token);
            csd->ignore_reassembled |= IGNORE_STREAM;
        }
        else if(!strcasecmp(token, "only_stream"))
        {
            CheckStream(token);
            csd->only_reassembled |= ONLY_STREAM;
        }
        else if(!strcasecmp(token, "no_frag"))
        {
            csd->ignore_reassembled |= IGNORE_FRAG;
        }
        else if(!strcasecmp(token, "only_frag"))
        {
            csd->only_reassembled |= ONLY_FRAG;
        }
        else
        {
            FatalError("%s:%d: Unknown Flow Option: '%s'\n",
                       file_name,file_line,token);

        }


        token = strtok(NULL, ",");
    }

    if(csd->from_client && csd->from_server)
    {
        FatalError("%s:%d: Can't use both from_client"
                   "and flow_from server", file_name, file_line);
    }

    if((csd->ignore_reassembled & IGNORE_STREAM) && (csd->only_reassembled & ONLY_STREAM))
    {
        FatalError("%s:%d: Can't use no_stream and"
                   " only_stream", file_name,file_line);
    }

    if((csd->ignore_reassembled & IGNORE_FRAG) && (csd->only_reassembled & ONLY_FRAG))
    {
        FatalError("%s:%d: Can't use no_frag and"
                   " only_frag", file_name,file_line);
    }

    if(otn->stateless && (csd->from_client || csd->from_server))
    {
        FatalError("%s:%d: Can't use flow: stateless option with"
                   " other options", file_name, file_line);
    }

    if(otn->stateless && otn->established)
    {
        FatalError("%s:%d: Can't specify established and stateless "
                   "options in same rule\n", file_name, file_line);
    }

    if(otn->stateless && otn->unestablished)
    {
        FatalError("%s:%d: Can't specify unestablished and stateless "
                   "options in same rule\n", file_name, file_line);
    }

    if(otn->established && otn->unestablished)
    {
        FatalError("%s:%d: Can't specify unestablished and established "
                   "options in same rule\n", file_name, file_line);
    }

    if (add_detection_option(sc, RULE_OPTION_TYPE_FLOW, (void *)csd, &idx_dup) == DETECTION_OPTION_EQUAL)
    {
#ifdef DEBUG_RULE_OPTION_TREE
        LogMessage("Duplicate Flow:\n%c %c %c %c\n%c %c %c %c\n\n",
            csd->from_client,
            csd->from_server,
            csd->ignore_reassembled,
            csd->only_reassembled,
            ((ClientServerData *)idx_dup)->from_client,
            ((ClientServerData *)idx_dup)->from_server,
            ((ClientServerData *)idx_dup)->ignore_reassembled,
            ((ClientServerData *)idx_dup)->only_reassembled);
#endif
        free(csd);
        csd = otn->ds_list[PLUGIN_CLIENTSERVER] = (ClientServerData *)idx_dup;
    }

    fpl = AddOptFuncToList(CheckFlow, otn);
    if (fpl)
    {
        fpl->type = RULE_OPTION_TYPE_FLOW;
        fpl->context = (void *)csd;
    }

    free(str);
}

/****************************************************************************
 *
 * Function: InitFlowData(OptTreeNode *)
 *
 * Purpose: calloc the clientserver data node
 *
 * Arguments: otn => pointer to the current rule option list node
 *
 * Returns: void function
 *
 ****************************************************************************/
void InitFlowData(OptTreeNode * otn)
{

    /* allocate the data structure and attach it to the
       rule's data struct list */
    otn->ds_list[PLUGIN_CLIENTSERVER] = (ClientServerData *)
        calloc(sizeof(ClientServerData), sizeof(char));

    if(otn->ds_list[PLUGIN_CLIENTSERVER] == NULL)
    {
        FatalError("FlowData calloc Failed!\n");
    }
}

int CheckFlow(void *option_data, Packet *p)
{
    ClientServerData *csd = (ClientServerData *)option_data;
    PROFILE_VARS;

    PREPROC_PROFILE_START(flowCheckPerfStats);

    /* Check established/unestablished first */
    if (ScStateful())
    {
        if ((csd->established == 1) && !(p->packet_flags & PKT_STREAM_EST))
        {
            /*
            ** This option requires an established connection and it isn't
            ** in that state yet, so no match.
            */
            PREPROC_PROFILE_END(flowCheckPerfStats);
            return DETECTION_OPTION_NO_MATCH;
        }
        else if ((csd->unestablished == 1) && (p->packet_flags & PKT_STREAM_EST))
        {
            /*
            **  We're looking for an unestablished stream, and this is
            **  established, so don't continue processing.
            */
            PREPROC_PROFILE_END(flowCheckPerfStats);
            return DETECTION_OPTION_NO_MATCH;
        }
    }

    /* Now check from client */
    if (csd->from_client)
    {
        if (ScStateful())
        {
            if (!(p->packet_flags & PKT_FROM_CLIENT) &&
                (p->packet_flags & PKT_FROM_SERVER))
            {
                /* No match on from_client */
                PREPROC_PROFILE_END(flowCheckPerfStats);
                return DETECTION_OPTION_NO_MATCH;
            }
        }
    }

    /* And from server */
    if (csd->from_server)
    {
        if (ScStateful())
        {
            if (!(p->packet_flags & PKT_FROM_SERVER) &&
                (p->packet_flags & PKT_FROM_CLIENT))
            {
                /* No match on from_server */
                PREPROC_PROFILE_END(flowCheckPerfStats);
                return DETECTION_OPTION_NO_MATCH;
            }
        }
    }

    /* ...ignore_reassembled */
    if (csd->ignore_reassembled & IGNORE_STREAM)
    {
        if (p->packet_flags & PKT_REBUILT_STREAM)
        {
            PREPROC_PROFILE_END(flowCheckPerfStats);
            return DETECTION_OPTION_NO_MATCH;
        }
    }

    if (csd->ignore_reassembled & IGNORE_FRAG)
    {
        if (p->packet_flags & PKT_REBUILT_FRAG)
        {
            PREPROC_PROFILE_END(flowCheckPerfStats);
            return DETECTION_OPTION_NO_MATCH;
        }
    }

    /* ...only_reassembled */
    if (csd->only_reassembled & ONLY_STREAM)
    {
        if ( !PacketHasPAFPayload(p)) 
        {
            PREPROC_PROFILE_END(flowCheckPerfStats);
            return DETECTION_OPTION_NO_MATCH;
        }
    }

    if (csd->only_reassembled & ONLY_FRAG)
    {
        if (!(p->packet_flags & PKT_REBUILT_FRAG))
        {
            PREPROC_PROFILE_END(flowCheckPerfStats);
            return DETECTION_OPTION_NO_MATCH;
        }
    }

    PREPROC_PROFILE_END(flowCheckPerfStats);
    return DETECTION_OPTION_MATCH;
}
