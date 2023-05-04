/*
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
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

/* sp_appid
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <errno.h>

#include "sf_types.h"
#include "snort_bounds.h"
#include "rules.h"
#include "decode.h"
#include "plugbase.h"
#include "parser.h"
#include "snort_debug.h"
#include "util.h"
#include "mstring.h"

#include "snort.h"
#include "profiler.h"
#include "sp_appid.h"
#include "appIdApi.h"
#include "preprocessors/stream_api.h"
#ifdef PERF_PROFILING
PreprocStats appIdPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#define CONF_SEPARATORS         ",\t\n\r"

#include "detection_options.h"
#include "detection_util.h"
#include "sftarget_protocol_reference.h" 
#include "sp_appid.h" 
//#include "sf_dynamic_preprocessor.h"

extern char *file_name;  /* this is the file name from rules.c, generally used
                            for error messages */

extern int file_line;    /* this is the file line number from rules.c that is
                            used to indicate file lines for error messages */


#define NUM_APPID_IN_RULE_STEP 5
#define MAX_NUM_APPID_IN_RULE 10

static void ParseAppIdData(struct _SnortConfig *sc, char *data, OptTreeNode *otn);
static void AppIdInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol);
static int CheckAppId (void *option_data, Packet *p);
AppIdOptionData* createOptionData(void);
void optAppIdFree(AppIdOptionData *optData);
extern int32_t getApplicationId(const char * appName);

/****************************************************************************
 *
 * Function: SetupAppId()
 *
 * Purpose: Load 'er up
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupAppId(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("appid", AppIdInit, NULL, OPT_TYPE_DETECTION, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("appid", &appIdPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Plugin: appid Setup\n"););
}

AppIdOptionData* optionAppIdCreate()
{
    AppIdOptionData *optData = (AppIdOptionData *) SnortAlloc(sizeof(AppIdOptionData));
    optData->appid_table = SnortAlloc(sizeof(AppIdInfo) * NUM_APPID_IN_RULE_STEP);
    optData->num_appid_allocated = NUM_APPID_IN_RULE_STEP;
    return optData;
}

// Generate a unique digest of the rule option. 
uint32_t optionAppIdHash(const void *option)
{
    uint32_t abc[3];
    int i;
    const AppIdOptionData *optData = (AppIdOptionData*)option;

    abc[0] = optData->num_appid;
    abc[1] = 0;
    abc[2] = 0;

    mix(abc[0], abc[1], abc[2]);

    //TBD do I need to keep is sorted??
    for ( i = 0; i < optData->num_appid; ++i )
        abc[i % 3] += optData->appid_table[i].appid_ordinal;

    final(abc[0], abc[1], abc[2]);

    return abc[2];
}

// Compare 2 rule options for duplicity 
int optionAppIdCompare(const void * _left, const void * _right)
{
    int i;
    const AppIdOptionData *left = (AppIdOptionData*)_left;
    const AppIdOptionData *right = (AppIdOptionData*)_right;

    if ( !left || !right )
        return DETECTION_OPTION_NOT_EQUAL;

    if ( left->num_appid != right->num_appid )
        return DETECTION_OPTION_NOT_EQUAL;

    for ( i = 0; i < left->num_appid; ++i )
        if ( left->appid_table[i].appid_ordinal != right->appid_table[i].appid_ordinal )
            return DETECTION_OPTION_NOT_EQUAL;

    return DETECTION_OPTION_EQUAL;
}
void optionAppIdFree(AppIdOptionData *optData)
{
    unsigned i;

    for (i=0; i < optData->num_appid; i++)
    {
        free(optData->appid_table[i].appName);
    }
    free(optData->appid_table);
    free(optData);
}

/****************************************************************************
 *
 * Function: AppIdInit(struct _SnortConfig *, char *, OptTreeNode *, int protocol)
 *
 * Purpose: Generic rule configuration function.  Handles parsing the rule
 *          information and attaching the associated detection function to
 *          the OTN.
 *
 * Arguments: data => rule arguments/data
 *            otn => pointer to the current rule option list node
 *            protocol => protocol the rule is on (we don't care in this case)
 *
 * Returns: void function
 *
 ****************************************************************************/

static void AppIdInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    OptFpList *fpl;
    void *ds_ptr_dup;
    AppIdOptionData *optData;

    /* multiple declaration check */
    if(otn->ds_list[PLUGIN_APPID])
    {
        FatalError("%s(%d): Multiple appid options in rule\n", file_name, file_line);
    }

    /* allocate the data structure and attach it to the
       rule's data struct list */
    optData = optionAppIdCreate();
    otn->ds_list[PLUGIN_APPID] = optData;

    /* this is where the keyword arguments are processed and placed into the
       rule option's data structure */
    ParseAppIdData(sc, data, otn);

    if (add_detection_option(sc, RULE_OPTION_TYPE_APPID, (void *)optData, &ds_ptr_dup) == DETECTION_OPTION_EQUAL)
    {
        optionAppIdFree(optData);
        otn->ds_list[PLUGIN_APPID] = ds_ptr_dup;
    }

    /* finally, attach the option's detection function to the rule's
       detect function pointer list */
    fpl = AddOptFuncToList(CheckAppId, otn);
    fpl->type = RULE_OPTION_TYPE_APPID;
    fpl->context = otn->ds_list[PLUGIN_APPID];

    return;
}
static int compareIds(const void *p1, const void *p2)
{
    return (((AppIdInfo *)p1)->appid_ordinal - ((AppIdInfo *)p2)->appid_ordinal);
}

/****************************************************************************
 *
 * Function: ParseIpOptionData(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: This is the function that is used to process the option keyword's
 *          arguments and attach them to the rule's data structures.
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: void function
 *
 ****************************************************************************/
static void ParseAppIdData(struct _SnortConfig *sc, char *data, OptTreeNode *otn)
{
    AppIdOptionData *ds_ptr;  /* data struct pointer */
    char * appName;
    boolean noapp = true;
    boolean found = false;
    int32_t ordinal;
    unsigned i;
    char  *appNameEnd;

    /* set the ds pointer to make it easier to reference the option's
    particular data struct */
    ds_ptr = otn->ds_list[PLUGIN_APPID];

    if(data == NULL)
    {
        FatalError("%s(%d): appid Option keyword missing argument!\n", file_name, file_line);
    }

    while(isspace((u_char)*data))
        data++;

    for (appName = strtok(data, CONF_SEPARATORS);
            appName;
            appName = strtok(NULL, CONF_SEPARATORS))
    {
        //strip spaces at beging and end.
        while (*appName == ' ')
            appName++;

        //strip spaces at the end
        appNameEnd = appName + strlen(appName);
        
        while(isspace(*--appNameEnd) && appNameEnd > appName); 

        *(appNameEnd+1) = '\0'; 

        if (ds_ptr->num_appid == ds_ptr->num_appid_allocated)
        {
            AppIdInfo* tmp = realloc(ds_ptr->appid_table, (ds_ptr->num_appid_allocated + NUM_APPID_IN_RULE_STEP)*sizeof(*tmp));
            if (!tmp)
            {
                ParseWarning("Malloc failure, too many appids in ruled %u\n");
                break;
            }
            ds_ptr->appid_table = tmp;
            ds_ptr->num_appid_allocated += NUM_APPID_IN_RULE_STEP;
        }

        ordinal = appIdApi.getApplicationId(appName);
        if (!ordinal)
        {
            ParseWarning(" appid metadata \"%s\" unknown.\n", appName);
            return;
        }

        for (i=0; i < ds_ptr->num_appid; i++)
        {
            if (ds_ptr->appid_table[i].appid_ordinal == ordinal)
            {
                found = true;
                ParseWarning("Duplicate appid metadata \"%s\" found.\n", appName);
                break;
            }
        }
        if (!found)
        {
            noapp = false;
            ds_ptr->appid_table[ds_ptr->num_appid].appName = SnortStrdup(appName);
            ds_ptr->appid_table[ds_ptr->num_appid].appid_ordinal = ordinal;
            ds_ptr->num_appid++;
        }
    }

    if (noapp)
    {
        FatalError("%s(%d) => appid must be followed by common separate list of apps: %s!\n",
                   file_name, file_line, data);
    }

    qsort(ds_ptr->appid_table, ds_ptr->num_appid, sizeof(*ds_ptr->appid_table), compareIds);
}

static int matchRuleProtoId( int16_t appProtoId, AppIdOptionData *app_data)
{
    unsigned i;
    if (appProtoId == 0)
        return 0;

    for (i = 0;
            i < app_data->num_appid && (app_data->appid_table[i].appid_ordinal);
            i++)
    {
        if (appProtoId == app_data->appid_table[i].appid_ordinal)
            return appProtoId;
    }
    return 0;
}

static int CheckAppId (void *option_data, Packet *p)
{
    AppIdOptionData *app_data = (AppIdOptionData *)option_data; 
    PROFILE_VARS;
    int16_t serviceProtoId, clientProtoId, payloadProtoId, miscProtoId, matchedProtoId;

    PREPROC_PROFILE_START(appIdPerfStats);

    app_data->matched_appid = 0;
    if (!p->ssnptr)
        return DETECTION_OPTION_NO_MATCH;

    stream_api->get_application_id(p->ssnptr, &serviceProtoId, &clientProtoId, &payloadProtoId, &miscProtoId);

    matchedProtoId = matchRuleProtoId(payloadProtoId, app_data);
    if (!matchedProtoId)
        matchedProtoId = matchRuleProtoId(miscProtoId, app_data); 

    if (!matchedProtoId)
    {
        if ((p->packet_flags & PKT_FROM_CLIENT))
        {
            matchedProtoId = matchRuleProtoId(clientProtoId, app_data);
            if (!matchedProtoId)
               matchedProtoId =  matchRuleProtoId(serviceProtoId, app_data);
        }
        else
        {
            matchedProtoId = matchRuleProtoId(serviceProtoId, app_data);
            if (!matchedProtoId)
                matchedProtoId = matchRuleProtoId(clientProtoId, app_data);
        }
    }

    if (matchedProtoId)
    {
        app_data->matched_appid = matchedProtoId;
        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Got appid %d matched!\n", matchedProtoId););
        PREPROC_PROFILE_END(appIdPerfStats);
        return DETECTION_OPTION_MATCH;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"No match\n"););
    PREPROC_PROFILE_END(appIdPerfStats);
    return DETECTION_OPTION_NO_MATCH;
}

