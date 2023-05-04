/* $Id$ */
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

/* sp_ip_proto
 *
 * Purpose:
 *
 * Check the IP header's protocol field value.
 *
 * Arguments:
 *
 *   Number, protocol name, ! for negation
 *
 * Effect:
 *
 *  Success on protocol match, failure otherwise
 *
 * Comments:
 *
 * None.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#ifndef WIN32
#include <netdb.h>
#endif /* !WIN32 */

#include "rules.h"
#include "treenodes.h"
#include "decode.h"
#include "plugbase.h"
#include "parser.h"
#include "snort_debug.h"
#include "util.h"
#include "plugin_enum.h"
#include "sp_ip_proto.h"

#include "snort.h"
#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats ipProtoPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "sfhashfcn.h"
#include "detection_options.h"

#define IP_PROTO__EQUAL         0
#define IP_PROTO__NOT_EQUAL     1
#define IP_PROTO__GREATER_THAN  2
#define IP_PROTO__LESS_THAN     3

typedef struct _IpProtoData
{
    uint8_t protocol;
    uint8_t comparison_flag;

} IpProtoData;


void IpProtoInit(struct _SnortConfig *, char *, OptTreeNode *, int);
void IpProtoRuleParseFunction(char *, IpProtoData *);
int IpProtoDetectorFunction(void *option_data, Packet *p);

uint32_t IpProtoCheckHash(void *d)
{
    uint32_t a,b,c;
    IpProtoData *data = (IpProtoData *)d;

    a = data->protocol;
    b = data->comparison_flag;
    c = RULE_OPTION_TYPE_IP_PROTO;

    final(a,b,c);

    return c;
}

int IpProtoCheckCompare(void *l, void *r)
{
    IpProtoData *left = (IpProtoData *)l;
    IpProtoData *right = (IpProtoData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if ((left->protocol == right->protocol) &&
        (left->comparison_flag == right->comparison_flag))
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}


/****************************************************************************
 *
 * Function: SetupIpProto()
 *
 * Purpose: Generic detection engine plugin ip_proto.  Registers the
 *          configuration function and links it to a rule keyword.  This is
 *          the function that gets called from InitPlugins in plugbase.c.
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupIpProto(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("ip_proto", IpProtoInit, NULL, OPT_TYPE_DETECTION, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("ip_proto", &ipProtoPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif
    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Plugin: IpProto Setup\n"););
}


/****************************************************************************
 *
 * Function: IpProtoInit(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Generic rule configuration function.  Handles parsing the rule
 *          information and attaching the associated detection function to
 *          the OTN.
 *
 * Arguments: data => rule arguments/data
 *            otn => pointer to the current rule option list node
 *
 * Returns: void function
 *
 ****************************************************************************/
void IpProtoInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    OptFpList *ofl;
    IpProtoData *ipd;
    void *ds_ptr_dup;

    /* Rule must be an "ip" rule to use this rule option */
    if (protocol != ETHERNET_TYPE_IP)
    {
        FatalError("%s(%d): \"ip_proto\" rule option can only be used in an "
                   "\"ip\" rule.\n", file_name, file_line);
    }

    /* multiple declaration check */
    /*if(otn->ds_list[PLUGIN_IP_PROTO_CHECK])
    {
        FatalError("%s(%d): Multiple ip_proto options in rule\n", file_name,
                file_line);
    }*/

    ipd = (IpProtoData *) SnortAlloc(sizeof(IpProtoData));

    /* allocate the data structure and attach it to the
       rule's data struct list */
    //otn->ds_list[PLUGIN_IP_PROTO_CHECK] = (IpProtoData *) calloc(sizeof(IpProtoData), sizeof(char));

    /* this is where the keyword arguments are processed and placed into the
       rule option's data structure */
    IpProtoRuleParseFunction(data, ipd);

    /* finally, attach the option's detection function to the rule's
       detect function pointer list */
    ofl = AddOptFuncToList(IpProtoDetectorFunction, otn);
    ofl->type = RULE_OPTION_TYPE_IP_PROTO;

    /*
    **  Set the ds_list for the first ip_proto check for a rule.  This
    **  is needed for the high-speed rule optimization.
    */
    if(!otn->ds_list[PLUGIN_IP_PROTO_CHECK])
        otn->ds_list[PLUGIN_IP_PROTO_CHECK] = ipd;

    if (add_detection_option(sc, RULE_OPTION_TYPE_IP_PROTO, (void *)ipd, &ds_ptr_dup) == DETECTION_OPTION_EQUAL)
    {
        free(ipd);
        ipd = otn->ds_list[PLUGIN_IP_PROTO_CHECK] = ds_ptr_dup;
    }
    ofl->context = ipd;
}



/****************************************************************************
 *
 * Function: IpProtoRuleParseFunction(char *, OptTreeNode *)
 *
 * Purpose: This is the function that is used to process the option keyword's
 *          arguments and attach them to the rule's data structures.
 *
 * Arguments: data => argument data
 *            ds_ptr => pointer to the IpProtoData struct
 *
 * Returns: void function
 *
 ****************************************************************************/
void IpProtoRuleParseFunction(char *data, IpProtoData *ds_ptr)
{
    /* set the ds pointer to make it easier to reference the option's
       particular data struct */
    //ds_ptr = otn->ds_list[PLUGIN_IP_PROTO_CHECK];

    while(isspace((int)*data)) data++;

    if (*data == '!')
    {
        ds_ptr->comparison_flag = IP_PROTO__NOT_EQUAL;
        data++;
    }
    else if (*data == '>')
    {
        ds_ptr->comparison_flag = IP_PROTO__GREATER_THAN;
        data++;
    }
    else if (*data == '<')
    {
        ds_ptr->comparison_flag = IP_PROTO__LESS_THAN;
        data++;
    }
    else
    {
        ds_ptr->comparison_flag = IP_PROTO__EQUAL;
    }

    /* check for a number or a protocol name */
    if(isdigit((int)*data))
    {
        unsigned long ip_proto;
        char *endptr;

        ip_proto = SnortStrtoul(data, &endptr, 10);
        if ((errno == ERANGE) || (ip_proto >= NUM_IP_PROTOS))
        {
            FatalError("%s(%d) Invalid protocol number for \"ip_proto\" "
                       "rule option.  Value must be between 0 and 255.\n",
                       file_name, file_line);
        }

        ds_ptr->protocol = (uint8_t)ip_proto;
    }
    else
    {
        struct protoent *pt = getprotobyname(data);

        if (pt != NULL)
        {
            /* p_proto should be a number less than 256 */
            ds_ptr->protocol = (uint8_t)pt->p_proto;
        }
        else
        {
            FatalError("%s(%d) Invalid protocol name for \"ip_proto\" "
                       "rule option: \"%s\".\n", file_name, file_line, data);
        }
    }
}


/****************************************************************************
 *
 * Function: IpProtoDetectorFunction(char *, OptTreeNode *)
 *
 * Purpose: Use this function to perform the particular detection routine
 *          that this rule keyword is supposed to encompass.
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: If the detection test fails, this function *must* return a zero!
 *          On success, it calls the next function in the detection list
 *
 ****************************************************************************/
int IpProtoDetectorFunction(void *option_data, Packet *p)
{
    IpProtoData *ipd = (IpProtoData *)option_data;  /* data struct pointer */
    int rval = DETECTION_OPTION_NO_MATCH;
    PROFILE_VARS;

    if(!IPH_IS_VALID(p))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"Not IP\n"););
        return rval;
    }

    PREPROC_PROFILE_START(ipProtoPerfStats);

    switch (ipd->comparison_flag)
    {
        case IP_PROTO__EQUAL:
            if (GET_IPH_PROTO(p) == ipd->protocol)
                rval = DETECTION_OPTION_MATCH;
            break;

        case IP_PROTO__NOT_EQUAL:
            if (GET_IPH_PROTO(p) != ipd->protocol)
                rval = DETECTION_OPTION_MATCH;
            break;

        case IP_PROTO__GREATER_THAN:
            if (GET_IPH_PROTO(p) > ipd->protocol)
                rval = DETECTION_OPTION_MATCH;
            break;

        case IP_PROTO__LESS_THAN:
            if (GET_IPH_PROTO(p) < ipd->protocol)
                rval = DETECTION_OPTION_MATCH;
            break;

        default:
            ErrorMessage("%s(%d) Invalid comparison flag.\n",
                         __FILE__, __LINE__);
            break;
    }

    /* if the test isn't successful, this function *must* return 0 */
    PREPROC_PROFILE_END(ipProtoPerfStats);
    return rval;
}

int GetIpProtos(void *option_data, uint8_t *proto_array, int pa_size)
{
    IpProtoData *ipd = (IpProtoData *)option_data;
    int i;

    if ((proto_array == NULL) || (pa_size < NUM_IP_PROTOS))
        return -1;

    /* IP proto not set.  Include them all */
    if (option_data == NULL)
    {
        memset(proto_array, 1, pa_size);
        return 0;
    }

    memset(proto_array, 0, pa_size);

    switch (ipd->comparison_flag)
    {
        case IP_PROTO__EQUAL:
            proto_array[ipd->protocol] = 1;
            break;

        case IP_PROTO__NOT_EQUAL:
            for (i = 0; i < ipd->protocol; i++)
                proto_array[i] = 1;
            for (i = i + 1; i < NUM_IP_PROTOS; i++)
                proto_array[i] = 1;
            break;

        case IP_PROTO__GREATER_THAN:
            for (i = ipd->protocol + 1; i < NUM_IP_PROTOS; i++)
                proto_array[i] = 1;
            break;

        case IP_PROTO__LESS_THAN:
            for (i = 0; i < ipd->protocol; i++)
                proto_array[i] = 1;
            break;

        default:
            ErrorMessage("%s(%d) Invalid comparison flag.\n",
                         __FILE__, __LINE__);
            return -1;
    }

    return 0;
}

/*
 * Extract the IP Protocol field.
*/
int GetOtnIpProto(OptTreeNode *otn)
{
   IpProtoData *ipd;

   if (otn == NULL)
       return -1;

   ipd = (IpProtoData *)otn->ds_list[PLUGIN_IP_PROTO_CHECK];

   if ((ipd != NULL) && (ipd->comparison_flag == IP_PROTO__EQUAL))
       return (int)ipd->protocol;

   return -1;
}

