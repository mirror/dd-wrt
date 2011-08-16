/*
** $Id$
**
** sp_flowbits
** 
** Purpose:
**
** Wouldn't it be nice if we could do some simple state tracking 
** across multiple packets?  Well, this allows you to do just that.
**
** Effect:
**
** - [Un]set a bitmask stored with the session
** - Check the value of the bitmask
**
** Copyright (C) 2003-2011 Sourcefire, Inc.
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include "rules.h"
#include "treenodes.h"
#include "decode.h"
#include "plugbase.h"
#include "parser.h"
#include "debug.h"
#include "util.h"
#include "plugin_enum.h"
#include "snort.h"
#include "bitop_funcs.h"
#include "sfghash.h"
#include "sp_flowbits.h"
#include "sf_types.h"
#include "mstring.h"

#include "stream_api.h"

#include "snort.h"
#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats flowBitsPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "sfhashfcn.h"
#include "detection_options.h"
#define DEFAULT_FLOWBIT_GROUP  "default"

SFGHASH *flowbits_hash = NULL;
SFGHASH *flowbits_grp_hash = NULL;
SF_QUEUE *flowbits_bit_queue = NULL;
uint32_t flowbits_count = 0;
int flowbits_toggle = 1;

extern const unsigned int giFlowbitSize;

void FlowItemFree(void *);
void FlowBitsGrpFree(void *);
static void FlowBitsInit(char *, OptTreeNode *, int);
static void FlowBitsParse(char *, FLOWBITS_OP *, OptTreeNode *);
static void FlowBitsCleanExit(int, void *);


/****************************************************************************
 * 
 * Function: FlowBitsHashInit(void)
 *
 * Purpose: Initialize the hash table and queue storage for flowbits IDs
 *
 * Arguments: None
 *
 * Returns: void function
 *
 ****************************************************************************/
void FlowBitsHashInit(void)
{
    if (flowbits_hash != NULL)
        return;

    flowbits_hash = sfghash_new(10000, 0, 0, FlowItemFree);
    if (flowbits_hash == NULL)
    {
        FatalError("%s(%d) Could not create flowbits hash.\n",
                   __FILE__, __LINE__);
    }


    flowbits_bit_queue = sfqueue_new();
    if (flowbits_bit_queue == NULL)
    {
        FatalError("%s(%d) Could not create flowbits bit queue.\n",
                   __FILE__, __LINE__);
    }
}

void FlowBitsGrpHashInit(void)
{
    if (flowbits_grp_hash != NULL)
        return;

    flowbits_grp_hash = sfghash_new(10000, 0, 0, FlowBitsGrpFree);
    if (flowbits_grp_hash == NULL)
    {
        FatalError("%s(%d) Could not create flowbits group hash.\n",
                   __FILE__, __LINE__);
    }
    
}

void FlowItemFree(void *d)
{
    FLOWBITS_OBJECT *data = (FLOWBITS_OBJECT *)d;
    free(data->group);
    free(data);
}

void FlowBitsGrpFree(void *d)
{
    FLOWBITS_GRP *data = (FLOWBITS_GRP *)d;
    boFreeBITOP(&(data->GrpBitOp));
    free(data);
}

void FlowBitsFree(void *d)
{
    FLOWBITS_OP *data = (FLOWBITS_OP *)d;

    free(data->name);
    free(data->group);
    free(data);
}

uint32_t FlowBitsHash(void *d)
{
    uint32_t a,b,c;
    FLOWBITS_OP *data = (FLOWBITS_OP *)d;

    a = data->id;
    b = data->type;
    c= RULE_OPTION_TYPE_FLOWBIT;

    final(a,b,c);

    return c;
}

int FlowBitsCompare(void *l, void *r)
{
    FLOWBITS_OP *left = (FLOWBITS_OP *)l;
    FLOWBITS_OP *right = (FLOWBITS_OP *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;
                                
    if (( left->id == right->id) &&
        ( left->type == right->type))
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}

/****************************************************************************
 * 
 * Function: SetupFlowBits()
 *
 * Purpose: Generic detection engine plugin template.  Registers the
 *          configuration function and links it to a rule keyword.  This is
 *          the function that gets called from InitPlugins in plugbase.c.
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 * 3/4/05 - man beefed up the hash table size from 100 -> 10000
 *
 ****************************************************************************/
void SetupFlowBits(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("flowbits", FlowBitsInit, NULL, OPT_TYPE_DETECTION, NULL);

#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("flowbits", &flowBitsPerfStats, 3, &ruleOTNEvalPerfStats);
#endif

    AddFuncToCleanExitList(FlowBitsCleanExit, NULL);

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Plugin: FlowBits Setup\n"););
}


/****************************************************************************
 * 
 * Function: FlowBitsInit(char *, OptTreeNode *)
 *
 * Purpose: Configure the flow init option to register the appropriate checks
 *
 * Arguments: data => rule arguments/data
 *            otn => pointer to the current rule option list node
 *
 * Returns: void function
 *
 ****************************************************************************/
static void FlowBitsInit(char *data, OptTreeNode *otn, int protocol)
{
    FLOWBITS_OP *flowbits;
    OptFpList *fpl;
    void *idx_dup;

    /* Flowbits are now part of the rule stub for .so rules.
     * We avoid adding the flowbit twice by skipping it here. */
    if (otn->sigInfo.generator == 3)
        return;
 
    /* Flow bits are handled by Stream5 if its enabled */
    if( stream_api && stream_api->version != STREAM_API_VERSION5)
    {
        if (ScConfErrorOut())
        {
            FatalError("Warning: %s (%d) => flowbits without Stream5. "
                "Stream5 must be enabled for this plugin.\n",
                file_name,file_line);
        }
        else
        {
            LogMessage("Warning: %s (%d) => flowbits without Stream5. "
                "Stream5 must be enabled for this plugin.\n",
                file_name,file_line);
        }
    }

    /* Auto init hash table and queue */
    if (flowbits_hash == NULL)
        FlowBitsHashInit();

    if (flowbits_grp_hash == NULL )
        FlowBitsGrpHashInit();

    flowbits = (FLOWBITS_OP *) SnortAlloc(sizeof(FLOWBITS_OP));
    if (!flowbits) {
        FatalError("%s (%d): Unable to allocate flowbits node\n", file_name,
                file_line);
    }

    /* Set the ds_list value to 1 (yes, we have flowbits for this rule) */
    otn->ds_list[PLUGIN_FLOWBIT] = (void *)1;

    FlowBitsParse(data, flowbits, otn);
    if (add_detection_option(RULE_OPTION_TYPE_FLOWBIT, (void *)flowbits, &idx_dup) == DETECTION_OPTION_EQUAL)
    {
#ifdef DEBUG_RULE_OPTION_TREE
        LogMessage("Duplicate FlowBit:\n%d %c\n%d %c\n\n",
            flowbits->id,
            flowbits->type,
            ((FLOWBITS_OP *)idx_dup)->id,
            ((FLOWBITS_OP *)idx_dup)->type);
#endif
        free(flowbits->name);
        free(flowbits->group);
        free(flowbits);
        flowbits = idx_dup;
     }

    fpl = AddOptFuncToList(FlowBitsCheck, otn);
    fpl->type = RULE_OPTION_TYPE_FLOWBIT;

    /*
     * attach it to the context node so that we can call each instance 
     * individually
     */
    
    fpl->context = (void *) flowbits;
    return;
}


/****************************************************************************
 * 
 * Function: FlowBitsParse(char *, FlowBits *flowbits, OptTreeNode *)
 *
 * Purpose: parse the arguments to the flow plugin and alter the otn
 *          accordingly
 *
 * Arguments: otn => pointer to the current rule option list node
 *
 * Returns: void function
 *
 ****************************************************************************/
static void FlowBitsParse(char *data, FLOWBITS_OP *flowbits, OptTreeNode *otn)
{
    FLOWBITS_OBJECT *flowbits_item;
    FLOWBITS_GRP *flowbits_grp;
    char **toks;
    int num_toks;
    char *token;
    char *pch= NULL;
    uint32_t id = 0;
    int hstatus;
    int found = 0;
    int default_grp = 0;
    SnortConfig *sc = snort_conf_for_parsing;

    if (sc == NULL)
    {
        FatalError("%s(%d) Snort config for parsing is NULL.\n",
                   __FILE__, __LINE__);
    }

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "flowbits parsing %s\n",data););

    toks = mSplit(data, ",", 0, &num_toks, 0);
    if(num_toks < 1)
    {
        FatalError("%s (%d): ParseFlowArgs: Must specify flowbits operation.\n", 
                file_name, file_line);
    }
    token = toks[0];

    if(!strcasecmp("set",token))
    {
        flowbits->type = FLOWBITS_SET;
    } 
    else if(!strcasecmp("unset",token))
    {
        flowbits->type = FLOWBITS_UNSET;
    }
    else if(!strcasecmp("toggle",token))
    {
        flowbits->type = FLOWBITS_TOGGLE;
    }
    else if(!strcasecmp("isset",token))
    {
        flowbits->type = FLOWBITS_ISSET;
    }
    else if(!strcasecmp("isnotset",token))
    {
        flowbits->type = FLOWBITS_ISNOTSET;
    } 
    else if(!strcasecmp("noalert", token))
    {
        if(num_toks > 1)
        {
            FatalError("%s (%d): Do not specify a flowbits tag id for the "
                       "keyword 'noalert'.\n", file_name, file_line);
        }

        flowbits->type = FLOWBITS_NOALERT;
        flowbits->id   = 0;

        mSplitFree(&toks, num_toks);
        return;
    }
    else if(!strcasecmp("reset",token))
    {
        if(num_toks > 1)
        {
            FatalError("%s (%d): Do not specify a flowbits tag id for the "
                       "keyword 'reset'.\n", file_name, file_line);
        }

        flowbits->type = FLOWBITS_RESET;
        flowbits->id   = 0;
        mSplitFree(&toks, num_toks);
        return;
    } 
    else
    {
        FatalError("%s(%d) ParseFlowArgs: Invalid token %s\n",
                file_name, file_line, token);
    }

    /*
    **  Let's parse the flowbits name
    */
    if( num_toks < 2 )
    {
        FatalError("%s (%d): flowbits tag id must be provided\n",
                file_name, file_line);
    }
    token = toks[1];
    pch = strtok(token, " ,\t" );
    
    if(pch && (pch != token ))
    {
        FatalError("%s (%d): flowbits tag id cannot include spaces or "
                   "commas.\n", file_name, file_line);
    }

    flowbits_item = (FLOWBITS_OBJECT *)sfghash_find(flowbits_hash, token);

    if (flowbits_item != NULL) 
    {
        id = flowbits_item->id;
        found = 1;
    }
    else
    {
        flowbits_item = 
            (FLOWBITS_OBJECT *)SnortAlloc(sizeof(FLOWBITS_OBJECT));

        if (sfqueue_count(flowbits_bit_queue) > 0)
        {
            id = (uint32_t)(uintptr_t)sfqueue_remove(flowbits_bit_queue);
            flowbits_item->id = id;
        }
        else
        {
            id = flowbits_count;
            flowbits_item->id = flowbits_count;

            flowbits_count++;

            if(flowbits_count > (giFlowbitSize<<3) )
            {
                FatalError("FLOWBITS: The number of flowbit IDs in the "
                           "current ruleset (%d) exceed the maximum number of IDs "
                           "that are allowed (%d).\n", flowbits_count,giFlowbitSize<<3);
            }
        }

        hstatus = sfghash_add(flowbits_hash, token, flowbits_item);
        if(hstatus != SFGHASH_OK) 
        {
            FatalError("Could not add flowbits key (%s) to hash.\n",token);
        }
    }

    flowbits_item->toggle = flowbits_toggle;
    flowbits_item->types |= flowbits->type;

    switch (flowbits->type)
    {
        case FLOWBITS_SET:
        case FLOWBITS_UNSET:
        case FLOWBITS_TOGGLE:
        case FLOWBITS_RESET:
            flowbits_item->set++;
            break;
        case FLOWBITS_ISSET:
        case FLOWBITS_ISNOTSET:
            flowbits_item->isset++;
            break;
        default:
            break;
    }

    flowbits->id = id;
    flowbits->name = SnortStrdup(token);
    
    if(num_toks < 3)
    {
        if (flowbits->type == FLOWBITS_SET || flowbits->type == FLOWBITS_TOGGLE)
            token = DEFAULT_FLOWBIT_GROUP;
        else
        {
            mSplitFree(&toks, num_toks);
            return;
        }
    }
    else
    {
        if (flowbits->type == FLOWBITS_SET || flowbits->type == FLOWBITS_TOGGLE)
            token = toks[2];
        else
        {
            FatalError("%s (%d): Group name can be specified only with set and toggle "
                    "operations \n", file_name, file_line);
        }
    }
    if (strcmp(token, DEFAULT_FLOWBIT_GROUP)  == 0)
        default_grp = 1;
    flowbits_grp = (FLOWBITS_GRP *)sfghash_find(flowbits_grp_hash, token);

    if (flowbits_grp == NULL && !default_grp )
    {
        flowbits_grp = (FLOWBITS_GRP *)SnortAlloc(sizeof(FLOWBITS_GRP));
        boInitBITOP(&(flowbits_grp->GrpBitOp), giFlowbitSize);
        boSetAllBits(&(flowbits_grp->GrpBitOp));
        hstatus = sfghash_add(flowbits_grp_hash, token, flowbits_grp);
        if(hstatus != SFGHASH_OK)
        {
            FatalError("Could not add flowbits group (%s) to hash.\n",token);
        }
    }
    if ( found && flowbits_item->group)
    {
        if (strcmp((flowbits_item->group), token) != 0)
        {
            FatalError("%s(%d) Flowbits already belongs to a group\n",
                       __FILE__, __LINE__);
        }
    }
    else
    {
        if ( !default_grp)
        {
            flowbits_grp->count++;
            if ( flowbits_grp->max_id < id )
                flowbits_grp->max_id = id;
            boClearBit(&(flowbits_grp->GrpBitOp),flowbits->id);
        }
        flowbits_item->group = SnortStrdup(token);
    }
    flowbits->group = SnortStrdup(token);
    mSplitFree(&toks, num_toks);

}

static INLINE int boSetGrpBit(BITOP *BitOp, char *group, unsigned int uiPos)
{
    FLOWBITS_GRP *flowbits_grp;
    BITOP *GrpBitOp;
    unsigned int i;
    flowbits_grp = (FLOWBITS_GRP *)sfghash_find(flowbits_grp_hash, group);
    if( flowbits_grp == NULL )
        return 0;
    if((BitOp == NULL) || (BitOp->uiMaxBits <= uiPos) || (BitOp->uiMaxBits <= flowbits_grp->max_id) || flowbits_grp->count <= 1)
        return 0;
    GrpBitOp = &(flowbits_grp->GrpBitOp);
    BitOp->pucBitBuffer[uiPos >> 3] = 0;
    for ( i = 0; i <= flowbits_grp->max_id ; i++ )
    {
        BitOp->pucBitBuffer[i >> 3] &= GrpBitOp->pucBitBuffer[i >> 3];
    }
    return 1;
}

/****************************************************************************
 * 
 * Function: FlowBitsCheck(Packet *, struct _OptTreeNode *, OptFpList *)
 *
 * Purpose: Check flow bits foo 
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: 0 on failure
 *
 ****************************************************************************/
int FlowBitsCheck(void *option_data, Packet *p)
{
    FLOWBITS_OP *flowbits = (FLOWBITS_OP*)option_data;
    int rval = DETECTION_OPTION_NO_MATCH;
    StreamFlowData *flowdata;
    int result = 0;
    int iRet = 0;
    PROFILE_VARS;

    /* Need session pointer to get flowbits */
    if ((stream_api == NULL) || (p->ssnptr == NULL))
        return rval;

    PREPROC_PROFILE_START(flowBitsPerfStats);

    flowdata = stream_api->get_flow_data(p);
    if(!flowdata)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "No FLOWBITS_DATA"););
        PREPROC_PROFILE_END(flowBitsPerfStats);
        return rval;
    }

    DEBUG_WRAP
    (
        DebugMessage(DEBUG_PLUGIN,"flowbits: type = %d\n",flowbits->type);
        DebugMessage(DEBUG_PLUGIN,"flowbits: value = %d\n",flowbits->id);
    );

    if(((flowbits->type == FLOWBITS_SET) || (((flowbits->type == FLOWBITS_TOGGLE) &&
            (!boIsBitSet(&(flowdata->boFlowbits),flowbits->id))))) && 
            (flowbits->group && (strcasecmp(flowbits->group, DEFAULT_FLOWBIT_GROUP) != 0 )))
    {
        if ( (iRet = boSetGrpBit(&(flowdata->boFlowbits), flowbits->group, flowbits->id)) == 0 )
            return rval;

    }

    switch(flowbits->type)
    {
        case FLOWBITS_SET:
            boSetBit(&(flowdata->boFlowbits),flowbits->id);
            result = 1;
            break;

        case FLOWBITS_UNSET:
            boClearBit(&(flowdata->boFlowbits),flowbits->id);
            result = 1;
            break;

        case FLOWBITS_RESET:
            boResetBITOP(&(flowdata->boFlowbits));
            result = 1;
            break;

        case FLOWBITS_ISSET:
            if(boIsBitSet(&(flowdata->boFlowbits),flowbits->id))
            {
                result = 1;
            }
            else
            {
                rval = DETECTION_OPTION_FAILED_BIT;
            }
            break;

        case FLOWBITS_ISNOTSET:
            if (boIsBitSet(&(flowdata->boFlowbits),flowbits->id))
            {
                result = 0;
            }
            else
            {
                result = 1;
            }
            break;

        case FLOWBITS_TOGGLE:
            if (boIsBitSet(&(flowdata->boFlowbits),flowbits->id))
            {
                boClearBit(&(flowdata->boFlowbits),flowbits->id);
            }
            else
            {
                boSetBit(&(flowdata->boFlowbits),flowbits->id);
            }

            result = 1;

            break;

        case FLOWBITS_NOALERT:
            /*
            **  This logic allows us to put flowbits: noalert any where
            **  in the detection chain, and still do bit ops after this
            **  option.
            */
            PREPROC_PROFILE_END(flowBitsPerfStats);
            return DETECTION_OPTION_NO_ALERT;

        default:
            /*
            **  Always return failure here.
            */
            PREPROC_PROFILE_END(flowBitsPerfStats);
            return rval;
    }
    
    /*
    **  Now return what we found
    */
    if (result == 1)
    {
        rval = DETECTION_OPTION_MATCH;
    }

    PREPROC_PROFILE_END(flowBitsPerfStats);
    return rval;
}

/****************************************************************************
 * 
 * Function: FlowBitsVerify()
 *
 * Purpose: Check flow bits foo to make sure its valid
 *
 * Arguments: 
 *
 * Returns: 0 on failure
 *
 ****************************************************************************/
void FlowBitsVerify(void)
{
    SFGHASH_NODE * n;
    FLOWBITS_OBJECT *fb;
    int num_flowbits = 0;

    if (flowbits_hash == NULL)
        return;

    for (n = sfghash_findfirst(flowbits_hash); 
         n != NULL; 
         n= sfghash_findnext(flowbits_hash))
    {
        fb = (FLOWBITS_OBJECT *)n->data;

        if (fb->toggle != flowbits_toggle)
        {
            if (sfqueue_add(flowbits_bit_queue, (NODE_DATA)(uintptr_t)fb->id) == -1)
            {
                FatalError("%s(%d) Failed to add flow bit id to queue.\n",
                           __FILE__, __LINE__);
            }

            sfghash_remove(flowbits_hash, n->key);
            continue;
        }

        if ((fb->set > 0) && (fb->isset == 0))
        {
            LogMessage("Warning: flowbits key '%s' is set but not ever checked.\n",
                    (char*)n->key);
        }
        else if ((fb->isset > 0) && (fb->set == 0))
        {
            LogMessage("Warning: flowbits key '%s' is checked but not ever set.\n",
                    (char*)n->key);
        }
        else if ((fb->set == 0) && (fb->isset == 0))
        {
            continue; /* don't count this bit as used */
        }

        num_flowbits++;
    }

    flowbits_toggle ^= 1;

    LogMessage("%d out of %d flowbits in use.\n", 
               num_flowbits, giFlowbitSize<<3);
}

static void FlowBitsCleanExit(int signal, void *data)
{
    if (flowbits_hash != NULL)
    {
        sfghash_delete(flowbits_hash);
        flowbits_hash = NULL;
    }

    if (flowbits_grp_hash != NULL)
    {
        sfghash_delete(flowbits_grp_hash);
        flowbits_grp_hash = NULL;
    }

    if (flowbits_bit_queue != NULL)
    {
        sfqueue_free_all(flowbits_bit_queue, NULL);
        flowbits_bit_queue = NULL;
    }
}

