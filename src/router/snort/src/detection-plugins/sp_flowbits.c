/*
 ** $Id$

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
 *
 *
 *  Major rewrite: Hui Cao <hcao@sourcefire.com>
 *
 *  Add flowbits OR support
 *
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
 *
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
#include "bitop_funcs.h"
#include "sfghash.h"
#include "sp_flowbits.h"
#include "sf_types.h"
#include "mstring.h"

#include "session_api.h"
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

#define ALLOWED_SPECIAL_CHARS       ".-_"

SFGHASH *flowbits_hash = NULL;
SFGHASH *flowbits_grp_hash = NULL;
SF_QUEUE *flowbits_bit_queue = NULL;
uint32_t flowbits_count = 0;
uint32_t flowbits_grp_count = 0;
int flowbits_toggle = 1;

#define DEFAULT_FLOWBIT_SIZE  1024
#define MAX_FLOWBIT_SIZE      2048
#define CONVERT_BITS_TO_BYTES(size)    ( (size > 1)?(((size -1) >> 3)+ 1):0)
static unsigned int giFlowbitSizeInBytes = CONVERT_BITS_TO_BYTES(DEFAULT_FLOWBIT_SIZE);
static unsigned int giFlowbitSize = DEFAULT_FLOWBIT_SIZE;

void FlowItemFree(void *);
void FlowBitsGrpFree(void *);
static void FlowBitsInit(struct _SnortConfig *, char *, OptTreeNode *, int);
static void FlowBitsParse(struct _SnortConfig *, char *, FLOWBITS_OP *, OptTreeNode *);
static void FlowBitsCleanExit(int, void *);

#ifdef SNORT_RELOAD
extern volatile bool reloadInProgress;
#endif

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
    free(data);
}

void FlowBitsGrpFree(void *d)
{
    FLOWBITS_GRP *data = (FLOWBITS_GRP *)d;
    boFreeBITOP(&(data->GrpBitOp));
    if (data->name)
        free(data->name);
    free(data);
}

void FlowBitsFree(void *d)
{
    FLOWBITS_OP *data = (FLOWBITS_OP *)d;
    if (data->ids)
        free(data->ids);
    if (data->name)
        free(data->name);
    if (data->group)
        free(data->group);
    free(data);
}

uint32_t FlowBitsHash(void *d)
{
    uint32_t a,b,c;
    FLOWBITS_OP *data = (FLOWBITS_OP *)d;
    int i;
    int j = 0;

    a = data->eval;
    b = data->type;
    c = RULE_OPTION_TYPE_FLOWBIT;

    mix(a,b,c);

    for (i = 0, j = 0; i < data->num_ids; i++, j++)
    {
        if (j >= 3)
        {
            a += data->ids[i - 2];
            b += data->ids[i - 1];
            c += data->ids[i];
            mix(a,b,c);
            j -= 3;
        }
    }
    if (1 == j)
    {
        a += data->ids[data->num_ids - 1];
        b += data->num_ids;
    }
    else if (2 == j)
    {
        a += data->ids[data->num_ids - 2];
        b += data->ids[data->num_ids - 1]|data->num_ids << 16;
    }

    c += data->group_id;

    final(a,b,c);

    return c;
}

int FlowBitsCompare(void *l, void *r)
{
    FLOWBITS_OP *left = (FLOWBITS_OP *)l;
    FLOWBITS_OP *right = (FLOWBITS_OP *)r;
    int i;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if ((left->num_ids != right->num_ids)||
            (left->eval != right->eval)||
            (left->type != right->type)||
            (left->group_id != right->group_id))
        return DETECTION_OPTION_NOT_EQUAL;


    for (i = 0; i < left->num_ids; i++)
    {
        if (left->ids[i] != right->ids[i])

            return DETECTION_OPTION_NOT_EQUAL;
    }

    return DETECTION_OPTION_EQUAL;
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
    RegisterPreprocessorProfile("flowbits", &flowBitsPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif

    AddFuncToCleanExitList(FlowBitsCleanExit, NULL);

    DEBUG_WRAP(DebugMessage(DEBUG_FLOWBITS, "Plugin: FlowBits Setup\n"););
}


/****************************************************************************
 *
 * Function: FlowBitsInit(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Configure the flow init option to register the appropriate checks
 *
 * Arguments: data => rule arguments/data
 *            otn => pointer to the current rule option list node
 *
 * Returns: void function
 *
 ****************************************************************************/
static void FlowBitsInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    FLOWBITS_OP *flowbits;
    OptFpList *fpl;
    void *idx_dup;

    /* Flowbits are now part of the rule stub for .so rules.
     * We avoid adding the flowbit twice by skipping it here. */
    if (otn->sigInfo.generator == 3)
        return;

    /* Flow bits are handled by Stream if its enabled */
    if( stream_api && stream_api->version != STREAM_API_VERSION5)
    {
        if (ScConfErrorOut())
        {
            FatalError("WARNING: %s (%d) => flowbits without Stream. "
                    "Stream must be enabled for this plugin.\n",
                    file_name,file_line);
        }
        else
        {
            LogMessage("WARNING: %s (%d) => flowbits without Stream. "
                    "Stream must be enabled for this plugin.\n",
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

    FlowBitsParse(sc, data, flowbits, otn);
    if (add_detection_option(sc, RULE_OPTION_TYPE_FLOWBIT, (void *)flowbits, &idx_dup) == DETECTION_OPTION_EQUAL)
    {
        char *group_name =  ((FLOWBITS_OP *)idx_dup)->group;

#ifdef DEBUG_RULE_OPTION_TREE
        LogMessage("Duplicate FlowBit:\n%d %c\n%d %c\n\n",
                flowbits->id,
                flowbits->type,
                ((FLOWBITS_OP *)idx_dup)->id,
                ((FLOWBITS_OP *)idx_dup)->type);
#endif
        if (flowbits->group)
        {
            if (group_name && strcmp(group_name, flowbits->group))
                free(group_name);
            ((FLOWBITS_OP *)idx_dup)->group = SnortStrdup(flowbits->group);
        }

        free(flowbits->ids);
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

static void udpateFlowBitGroupInfo(FLOWBITS_GRP *flowbits_grp, char *flowbitName,
        FLOWBITS_OBJECT *flowbits_item)
{

    char *groupName;

    if (!flowbits_grp)
        return;

    groupName = flowbits_grp->name;

    if (!groupName)
        return;

    flowbits_grp->count++;
    if ( flowbits_grp->max_id < flowbits_item->id )
        flowbits_grp->max_id = flowbits_item->id;
    boSetBit(&(flowbits_grp->GrpBitOp),flowbits_item->id);

}

static bool validateName(char *name)
{
    unsigned i;

    if (!name)
        return false;

    for (i=0; i<strlen(name); i++)
    {
        if (!isalnum(name[i])&&(NULL == strchr(ALLOWED_SPECIAL_CHARS,name[i])))
            return false;
    }
    return true;
}
/****************************************************************************
 *
 * Function: getFlowBitItem(char *, FLOWBITS_OP *, FLOWBITS_GRP *)
 *
 * Purpose: update the flowbits
 *
 * Arguments:
 *   char *flowbit --> flowbits name,
 *   FLOWBITS_OP *flowbits
 *   FLOWBITS_GRP *flowbits_grp
 *
 * Returns: FLOWBITS_OBJECT*, whether exist before
 *
 ****************************************************************************/
static  FLOWBITS_OBJECT* getFlowBitItem(char *flowbitName, FLOWBITS_OP *flowbits, FLOWBITS_GRP *flowbits_grp)
{
    FLOWBITS_OBJECT *flowbits_item;
    int hstatus;

    if (!validateName(flowbitName))
    {
        ParseError("Flowbits: flowbits name is limited to any alphanumeric string including %s"
        , ALLOWED_SPECIAL_CHARS);
    }

    flowbits_item = (FLOWBITS_OBJECT *)sfghash_find(flowbits_hash, flowbitName);

    if (flowbits_item == NULL)
    {
        flowbits_item = (FLOWBITS_OBJECT *)SnortAlloc(sizeof(FLOWBITS_OBJECT));

        if (sfqueue_count(flowbits_bit_queue) > 0)
        {
            flowbits_item->id = (uint16_t)(uintptr_t)sfqueue_remove(flowbits_bit_queue);

        }
        else
        {
            flowbits_item->id = (uint16_t)flowbits_count;

            flowbits_count++;

            if(flowbits_count > giFlowbitSize)
            {
                ParseError("The number of flowbit IDs in the "
                        "current ruleset exceeds the maximum number of IDs "
                        "that are allowed (%d).", giFlowbitSize);
            }
        }

        hstatus = sfghash_add(flowbits_hash, flowbitName, flowbits_item);
        if(hstatus != SFGHASH_OK)
        {
            FatalError("Could not add flowbits key (%s) to hash.\n",flowbitName);
        }
    }

    flowbits_item->toggle = flowbits_toggle;
    flowbits_item->types |= flowbits->type;

    switch (flowbits->type)
    {
    case FLOWBITS_SET:
    case FLOWBITS_SETX:
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
    udpateFlowBitGroupInfo(flowbits_grp, flowbitName, flowbits_item);

    return flowbits_item;
}

static FLOWBITS_GRP *getFlowBitGroup(char *groupName)
{
    int hstatus;
    FLOWBITS_GRP *flowbits_grp = NULL;

    if(!groupName)
        return NULL;

    if (!validateName(groupName))
    {
        ParseError("Flowbits: flowbits group name is limited to any alphanumeric string including %s",
         ALLOWED_SPECIAL_CHARS);
    }

    flowbits_grp = (FLOWBITS_GRP *)sfghash_find(flowbits_grp_hash, groupName);

    /*New group defined, add*/
    if (flowbits_grp == NULL)
    {
        flowbits_grp = (FLOWBITS_GRP *)SnortAlloc(sizeof(FLOWBITS_GRP));
        boInitBITOP(&(flowbits_grp->GrpBitOp), giFlowbitSizeInBytes);
        boResetBITOP(&(flowbits_grp->GrpBitOp));
        hstatus = sfghash_add(flowbits_grp_hash, groupName, flowbits_grp);
        if(hstatus != SFGHASH_OK)
        {
            FatalError("Could not add flowbits group (%s) to hash.\n",groupName);
        }
        flowbits_grp_count++;
        flowbits_grp->group_id = flowbits_grp_count;
        flowbits_grp->name = SnortStrdup(groupName);

    }

    return flowbits_grp;
}

#ifdef DEBUG_MSGS
static void printOutFlowbits(FLOWBITS_OP *flowbits)
{
    int i;

    DebugMessage(DEBUG_FLOWBITS,"flowbits: type = %d\n",flowbits->type);
    DebugMessage(DEBUG_FLOWBITS,"flowbits: name = %s\n",flowbits->name);
    DebugMessage(DEBUG_FLOWBITS,"flowbits: eval = %d\n",flowbits->eval);
    DebugMessage(DEBUG_FLOWBITS,"flowbits: num_ids = %d\n",flowbits->num_ids);
    DebugMessage(DEBUG_FLOWBITS,"flowbits: grp_id = %d\n",flowbits->group_id);
    DebugMessage(DEBUG_FLOWBITS,"flowbits: group_name = %s\n",flowbits->group);
    for (i = 0; i < flowbits->num_ids; i++)
    {
        DebugMessage(DEBUG_FLOWBITS,"flowbits: value = %d\n",flowbits->ids[i]);
    }
}
#endif

/****************************************************************************
 *
 * Function: processFlowbits(char *, FlowBits *flowbits, OptTreeNode *)
 *
 * Purpose: parse the arguments to the flow plugin and alter the otn
 *          accordingly
 *
 * Arguments: flowbits => pointer to the current flowbits op
 *
 * Returns: void function
 *
 ****************************************************************************/
static void processFlowbits(char *flowbits_names, FLOWBITS_GRP *flowbits_grp, FLOWBITS_OP *flowbits)
{
    char **toks;
    int num_toks;
    int i;
    char *flowbits_name;

    FLOWBITS_OBJECT *flowbits_item;

    if (!flowbits_names || ((*flowbits_names) == 0))
    {
       return;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_FLOWBITS, "flowbits tag id parsing %s\n",flowbits_names););

    flowbits_name = SnortStrdup(flowbits_names);

    if (NULL != strchr(flowbits_name, '|'))
    {
        if(NULL != strchr(flowbits_name, '&'))
        {
            ParseError("Flowbits: flowbits tag id operator '|' and '&' are used together.");
        }
        toks = mSplit(flowbits_name, "|", 0, &num_toks, 0);
        flowbits->ids = SnortAlloc(num_toks*sizeof(*(flowbits->ids)));
        flowbits->num_ids = num_toks;
        for (i = 0; i < num_toks; i++)
        {
            flowbits_item = getFlowBitItem(toks[i], flowbits, flowbits_grp);
            flowbits->ids[i] = flowbits_item->id;
        }
        flowbits->eval = FLOWBITS_OR;
        mSplitFree(&toks, num_toks);
    }
    else if (NULL != strchr(flowbits_name, '&'))
    {
        toks = mSplit(flowbits_name, "&", 0, &num_toks, 0);
        flowbits->ids = SnortAlloc(num_toks*sizeof(*(flowbits->ids)));
        flowbits->num_ids = num_toks;
        for (i = 0; i < num_toks; i++)
        {
            flowbits_item = getFlowBitItem(toks[i], flowbits, flowbits_grp);
            flowbits->ids[i] = flowbits_item->id;
        }
        flowbits->eval = FLOWBITS_AND;
        mSplitFree(&toks, num_toks);
    }
    else if (!strcasecmp(flowbits_name,"all"))
    {
        flowbits->eval = FLOWBITS_ALL;
    }
    else if (!strcasecmp(flowbits_name,"any"))
    {
        flowbits->eval = FLOWBITS_ANY;
    }
    else
    {
        flowbits_item = getFlowBitItem(flowbits_name, flowbits, flowbits_grp);
        flowbits->ids = SnortAlloc(sizeof(*(flowbits->ids)));
        flowbits->num_ids = 1;
        flowbits->ids[0] = flowbits_item->id;
    }

    free(flowbits_name);

}

void validateFlowbitsSyntax(FLOWBITS_OP *flowbits)
{
    switch (flowbits->type)
    {
    case FLOWBITS_SET:
        if ((flowbits->eval == FLOWBITS_AND) && (flowbits->ids))
            break;

        ParseError("Flowbits: operation set uses syntax: flowbits:set,bit[&bit],[group].");
        break;
    case FLOWBITS_SETX:
        if ((flowbits->eval == FLOWBITS_AND)&&(flowbits->group) && (flowbits->ids) )
            break;

        ParseError("Flowbits: operation setx uses syntax: flowbits:setx,bit[&bit],group.");

        break;
    case FLOWBITS_UNSET:
        if (((flowbits->eval == FLOWBITS_AND) && (!flowbits->group) && (flowbits->ids))
                ||((flowbits->eval == FLOWBITS_ALL) && (flowbits->group)))
            break;

        ParseError("Flowbits: operation unset uses syntax: flowbits:unset,bit[&bit] OR"
                " flowbits:unset, all, group.");

        break;
    case FLOWBITS_TOGGLE:
        if (((flowbits->eval == FLOWBITS_AND) && (!flowbits->group) &&(flowbits->ids))
                ||((flowbits->eval == FLOWBITS_ALL) && (flowbits->group)))
            break;

        ParseError("Flowbits: operation toggle uses syntax: flowbits:toggle,bit[&bit] OR"
                " flowbits:toggle,all,group.");
        break;
    case FLOWBITS_ISSET:
        if ((((flowbits->eval == FLOWBITS_AND) || (flowbits->eval == FLOWBITS_OR)) && (!flowbits->group) && flowbits->ids)
                ||((((flowbits->eval == FLOWBITS_ANY))||(flowbits->eval == FLOWBITS_ALL)) && (flowbits->group)))
            break;

        ParseError("Flowbits: operation isset uses syntax: flowbits:isset,bit[&bit] OR "
                "flowbits:isset,bit[|bit] OR flowbits:isset,all,group OR flowbits:isset,any,group.");

        break;
    case FLOWBITS_ISNOTSET:
        if ((((flowbits->eval == FLOWBITS_AND) || (flowbits->eval == FLOWBITS_OR)) && (!flowbits->group) && flowbits->ids)
                ||((((flowbits->eval == FLOWBITS_ANY))||(flowbits->eval == FLOWBITS_ALL)) && (flowbits->group)))
            break;

        ParseError("Flowbits: operation isnotset uses syntax: flowbits:isnotset,bit[&bit] OR "
                "flowbits:isnotset,bit[|bit] OR flowbits:isnotset,all,group OR flowbits:isnotset,any,group.");

        break;
    case FLOWBITS_RESET:
        if (flowbits->ids == NULL)
            break;
        ParseError("Flowbits: operation unset uses syntax: flowbits:reset OR flowbits:reset, group." );
        break;

    case FLOWBITS_NOALERT:
        if ((flowbits->ids == NULL) && (flowbits->group == NULL))
            break;
        ParseError("Flowbits: operation noalert uses syntax: flowbits:noalert." );
        break;

    default:
        ParseError("Flowbits: unknown operator.\n"
                , file_name, file_line);
        break;
    }
}

void processFlowBitsWithGroup(char *flowbitsName, char *groupName, FLOWBITS_OP *flowbits)
{
    FLOWBITS_GRP *flowbits_grp;

    if (!flowbits_hash)
        FlowBitsHashInit();

    if ((groupName)&& (!flowbits_grp_hash))
        FlowBitsGrpHashInit();

    flowbits_grp = getFlowBitGroup(groupName);
    processFlowbits(flowbitsName,flowbits_grp, flowbits);

    if (groupName && !(flowbits->group))
    {
        flowbits->group = SnortStrdup(groupName);
        flowbits->group_id = flowbits_grp->group_id;
    }
    validateFlowbitsSyntax(flowbits);
    DEBUG_WRAP( printOutFlowbits(flowbits));

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
static void FlowBitsParse(struct _SnortConfig *sc, char *data, FLOWBITS_OP *flowbits, OptTreeNode *otn)
{
    char **toks;
    int num_toks;
    char *typeName = NULL;
    char *groupName = NULL;
    char *flowbitsName = NULL;
    FLOWBITS_GRP *flowbits_grp;

    if (sc == NULL)
    {
        FatalError("%s(%d) Snort config for parsing is NULL.\n",
                __FILE__, __LINE__);
    }

    DEBUG_WRAP(DebugMessage(DEBUG_FLOWBITS, "flowbits parsing %s\n",data););

    toks = mSplit(data, ",", 0, &num_toks, 0);

    if(num_toks < 1)
    {
        ParseError("ParseFlowArgs: Must specify flowbits operation.");
    }
    else if (num_toks > 3)
    {
        ParseError("ParseFlowArgs: Too many arguments.");
    }

    typeName = toks[0];

    if(!strcasecmp("set",typeName))
    {
        flowbits->type = FLOWBITS_SET;
    }
    else if(!strcasecmp("setx",typeName))
    {
        flowbits->type = FLOWBITS_SETX;
    }
    else if(!strcasecmp("unset",typeName))
    {
        flowbits->type = FLOWBITS_UNSET;
    }
    else if(!strcasecmp("toggle",typeName))
    {
        flowbits->type = FLOWBITS_TOGGLE;
    }
    else if(!strcasecmp("isset",typeName))
    {
        flowbits->type = FLOWBITS_ISSET;
    }
    else if(!strcasecmp("isnotset",typeName))
    {
        flowbits->type = FLOWBITS_ISNOTSET;
    }
    else if(!strcasecmp("noalert", typeName))
    {
        if(num_toks > 1)
        {
            ParseError("Flowbits: Do not specify a flowbits tag id for the keyword 'noalert'.");
        }

        flowbits->type = FLOWBITS_NOALERT;
        flowbits->ids   = NULL;
        flowbits->num_ids  = 0;
        flowbits->name  = SnortStrdup(typeName);

        mSplitFree(&toks, num_toks);
        return;
    }
    else if(!strcasecmp("reset",typeName))
    {

        if(num_toks > 2)
        {
            ParseError("Flowbits: Too many arguments for the keyword 'reset'.");
        }

        if (num_toks == 2)
        {
            /*Save the group name*/
            groupName = SnortStrdup(toks[1]);
            flowbits_grp = getFlowBitGroup(groupName);
            flowbits->group = groupName;
            flowbits->group_id = flowbits_grp->group_id;
        }
        flowbits->type = FLOWBITS_RESET;
        flowbits->ids   = NULL;
        flowbits->num_ids   = 0;
        flowbits->name  = SnortStrdup(typeName);
        mSplitFree(&toks, num_toks);
        return;

    }
    else
    {
        ParseError("Flowbits: Invalid token %s.", typeName);
    }

    flowbits->name = SnortStrdup(typeName);
    /*
     **  Let's parse the flowbits name
     */
    if( num_toks < 2 )
    {
        ParseError("Flowbit: flowbits tag id must be provided.",
                file_name, file_line);
    }

    flowbitsName = toks[1];

    if(num_toks == 3)
    {
        groupName = toks[2];
    }
    processFlowBitsWithGroup(flowbitsName, groupName, flowbits);

    mSplitFree(&toks, num_toks);
}

static inline int boUnSetGrpBit(BITOP *BitOp, char *group)
{
    FLOWBITS_GRP *flowbits_grp;
    BITOP *GrpBitOp;
    unsigned int i, max_bytes;

    if( group == NULL )
        return 0;
    flowbits_grp = (FLOWBITS_GRP *)sfghash_find(flowbits_grp_hash, group);
    if( flowbits_grp == NULL )
        return 0;
    if((BitOp == NULL) || (BitOp->uiMaxBits <= flowbits_grp->max_id) || flowbits_grp->count == 0)
        return 0;
    GrpBitOp = &(flowbits_grp->GrpBitOp);

    /* note, max_id is an index, not a count.
     * Calculate max_bytes by adding 8 to max_id, then dividing by 8.  */
    max_bytes = (flowbits_grp->max_id + 8) >> 3;
    for ( i = 0; i < max_bytes; i++ )
    {
        BitOp->pucBitBuffer[i] &= ~GrpBitOp->pucBitBuffer[i];
    }
    return 1;
}

static inline int boToggleGrpBit(BITOP *BitOp, char *group)
{
    FLOWBITS_GRP *flowbits_grp;
    BITOP *GrpBitOp;
    unsigned int i, max_bytes;

    if( group == NULL )
        return 0;
    flowbits_grp = (FLOWBITS_GRP *)sfghash_find(flowbits_grp_hash, group);
    if( flowbits_grp == NULL )
        return 0;
    if((BitOp == NULL) || (BitOp->uiMaxBits <= flowbits_grp->max_id) || flowbits_grp->count == 0)
        return 0;
    GrpBitOp = &(flowbits_grp->GrpBitOp);

    /* note, max_id is an index, not a count.
     * Calculate max_bytes by adding 8 to max_id, then dividing by 8.  */
    max_bytes = (flowbits_grp->max_id + 8) >> 3;
    for ( i = 0; i < max_bytes; i++ )
    {
        BitOp->pucBitBuffer[i] ^= GrpBitOp->pucBitBuffer[i];
    }
    return 1;
}

static inline int boSetxBitsToGrp(BITOP *BitOp, uint16_t *ids, uint16_t num_ids, char *group)
{
    unsigned int i;
    if (!boUnSetGrpBit(BitOp, group))
        return 0;
    for(i = 0; i < num_ids; i++)
        boSetBit(BitOp,ids[i]);
    return 1;
}


static inline int issetFlowbits(StreamFlowData *flowdata, uint8_t eval, uint16_t *ids,
        uint16_t num_ids, char *group )
{
    unsigned int i;
    FLOWBITS_GRP *flowbits_grp;
    Flowbits_eval  evalType = (Flowbits_eval)eval;

    switch (evalType)
    {
    case FLOWBITS_AND:
        for(i = 0; i < num_ids; i++)
        {
            if(!boIsBitSet(&(flowdata->boFlowbits), ids[i]))
                return 0;
        }
        return 1;
        break;
    case FLOWBITS_OR:
        for(i = 0; i < num_ids; i++)
        {
            if(boIsBitSet(&(flowdata->boFlowbits), ids[i]))
                return 1;
        }
        return 0;
        break;
    case FLOWBITS_ALL:
        flowbits_grp = (FLOWBITS_GRP *)sfghash_find(flowbits_grp_hash, group);
        if( flowbits_grp == NULL )
            return 0;
        for ( i = 0; i <= (unsigned int)(flowbits_grp->max_id >>3) ; i++ )
        {
            uint8_t val = flowdata->boFlowbits.pucBitBuffer[i] & flowbits_grp->GrpBitOp.pucBitBuffer[i];
            if (val != flowbits_grp->GrpBitOp.pucBitBuffer[i])
                return 0;
        }
        return 1;
        break;
    case FLOWBITS_ANY:
        flowbits_grp = (FLOWBITS_GRP *)sfghash_find(flowbits_grp_hash, group);
        if( flowbits_grp == NULL )
            return 0;
        for ( i = 0; i <= (unsigned int)(flowbits_grp->max_id >>3) ; i++ )
        {
            uint8_t val = flowdata->boFlowbits.pucBitBuffer[i] & flowbits_grp->GrpBitOp.pucBitBuffer[i];
            if (val)
                return 1;
        }
        return 0;
        break;
    default:
        return 0;
    }
}

/****************************************************************************
 *
 * Function: checkFlowBits
 *
 * Purpose: Check flow bits
 *
 * Arguments: Packet *p => packet data
 *            uint8_t type,
 *            uint8_t evalType,
 *            uint16_t *ids,
 *            uint16_t num_ids,
 *            char *group
 *
 * Returns: 0 on failure
 *
 ****************************************************************************/
int checkFlowBits( uint8_t type, uint8_t evalType, uint16_t *ids, uint16_t num_ids, char *group, Packet *p)
{
    int rval = DETECTION_OPTION_NO_MATCH;
    StreamFlowData *flowdata;
    Flowbits_eval eval = (Flowbits_eval) evalType;
    int result = 0;
    int i;

    /* Need session pointer to get flowbits */
    if ((stream_api == NULL) || (p->ssnptr == NULL))
        return rval;


    flowdata = session_api->get_flow_data(p);
    if(!flowdata)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FLOWBITS, "No FLOWBITS_DATA"););
        return rval;
    }

    switch(type)
    {
    case FLOWBITS_SET:
        for(i = 0; i < num_ids; i++)
            boSetBit(&(flowdata->boFlowbits),ids[i]);
        result = 1;
        break;

    case FLOWBITS_SETX:
        result = boSetxBitsToGrp(&(flowdata->boFlowbits), ids, num_ids, group);
        break;

    case FLOWBITS_UNSET:
        if (eval == FLOWBITS_ALL )
            boUnSetGrpBit(&(flowdata->boFlowbits), group);
        else
        {
            for(i = 0; i < num_ids; i++)
                boClearBit(&(flowdata->boFlowbits),ids[i]);
        }
        result = 1;
        break;

    case FLOWBITS_RESET:
        if (!group)
            boResetBITOP(&(flowdata->boFlowbits));
        else
            boUnSetGrpBit(&(flowdata->boFlowbits), group);
        result = 1;
        break;

    case FLOWBITS_ISSET:

        if(issetFlowbits(flowdata,(uint8_t)eval, ids, num_ids, group))
        {
            result = 1;
        }
        else
        {
            rval = DETECTION_OPTION_FAILED_BIT;
        }

        break;

    case FLOWBITS_ISNOTSET:
        if(!issetFlowbits(flowdata, (uint8_t)eval, ids, num_ids, group))
        {
            result = 1;
        }
        else
        {
            rval = DETECTION_OPTION_FAILED_BIT;
        }
        break;

    case FLOWBITS_TOGGLE:
        if (group)
            boToggleGrpBit(&(flowdata->boFlowbits),group);
        else
        {
            for(i = 0; i < num_ids; i++)
            {
                if (boIsBitSet(&(flowdata->boFlowbits),ids[i]))
                {
                    boClearBit(&(flowdata->boFlowbits),ids[i]);
                }
                else
                {
                    boSetBit(&(flowdata->boFlowbits),ids[i]);
                }
            }
        }
        result = 1;

        break;

    case FLOWBITS_NOALERT:
        /*
         **  This logic allows us to put flowbits: noalert any where
         **  in the detection chain, and still do bit ops after this
         **  option.
         */
        return DETECTION_OPTION_NO_ALERT;

    default:
        /*
         **  Always return failure here.
         */
        return rval;
    }

    /*
     **  Now return what we found
     */
    if (result == 1)
    {
        rval = DETECTION_OPTION_MATCH;
    }

    return rval;
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

    PROFILE_VARS;

    if (!flowbits)
        return rval;

    PREPROC_PROFILE_START(flowBitsPerfStats);

    rval = checkFlowBits( flowbits->type, (uint8_t)flowbits->eval,
            flowbits->ids, flowbits->num_ids, flowbits->group, p);

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
            LogMessage("WARNING: flowbits key '%s' is set but not ever checked.\n",
                    (char*)n->key);
        }
        else if ((fb->isset > 0) && (fb->set == 0))
        {
            LogMessage("WARNING: flowbits key '%s' is checked but not ever set.\n",
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
            num_flowbits, giFlowbitSize);
}

static void FlowBitsCleanExit(int signal, void *data)
{
#ifdef SNORT_RELOAD
    if (reloadInProgress)
    {
        return;
    }
#endif

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
void setFlowbitSize(char *args)
{
    char *endptr;
    long int size;
    size = SnortStrtol(args, &endptr, 0);
    if ((errno == ERANGE) || (*endptr != '\0') ||
            (size < 0) || (size > MAX_FLOWBIT_SIZE))
    {
        ParseError("Invalid argument to 'flowbits_size': %s.  Must be a "
                "positive integer and less than %d.", args, MAX_FLOWBIT_SIZE);
    }

    giFlowbitSize = size;
    giFlowbitSizeInBytes = CONVERT_BITS_TO_BYTES(giFlowbitSize);
}

unsigned int getFlowbitSize(void)
{
    return giFlowbitSize;
}
unsigned int getFlowbitSizeInBytes(void)
{
    return giFlowbitSizeInBytes;
}

void FlowbitResetCounts(void)
{
    SFGHASH_NODE *n;
    FLOWBITS_OBJECT *fb;

    if (flowbits_hash == NULL)
        return;

    for (n = sfghash_findfirst(flowbits_hash);
            n != NULL;
            n = sfghash_findnext(flowbits_hash))
    {
        fb = (FLOWBITS_OBJECT *)n->data;
        fb->set = 0;
        fb->isset = 0;
    }
}
