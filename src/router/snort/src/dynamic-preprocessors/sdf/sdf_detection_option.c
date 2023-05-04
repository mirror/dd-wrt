/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2009-2013 Sourcefire, Inc.
**
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

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "spp_sdf.h"
#include "sdf_pattern_match.h"
#include "sdf_detection_option.h"
#include "sf_snort_plugin_api.h"
#include "sdf_us_ssn.h"
#include "sdf_credit_card.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"
#include "treenodes.h"

#ifdef SNORT_RELOAD
extern sdf_tree_node *swap_head_node;
extern uint32_t swap_num_patterns;
#endif

void AddPortsToConf(struct _SnortConfig *sc, SDFConfig *config, OptTreeNode *otn);
void AddProtocolsToConf(struct _SnortConfig *sc, SDFConfig *config, OptTreeNode *otn);

/* Function: SDFOptionInit
 * Purpose:  Parses a SDF rule option.
 * Arguments:
 *  name => Name of rule option
 *  args => Arguments to rule option
 *  data => Variable to save option data
 * Returns: 1 if successful
 *          0 if name is incorrect
 *          Fatal Error if invalid arguments
 */
int SDFOptionInit(struct _SnortConfig *sc, char *name, char *args, void **data)
{
    char *token, *endptr;
    unsigned long int tmpcount;
    SDFOptionData *sdf_data;

    if (name == NULL || args == NULL || data == NULL)
        return 0;

    if (strcasecmp(name, SDF_OPTION_NAME) != 0)
        return 0;

    sdf_data = (SDFOptionData *)calloc(1, sizeof(SDFOptionData));
    if (sdf_data == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) Failed to allocate memory for "
                "SDF pattern data structure.", __FILE__, __LINE__);
    }

    /* Parse the count */
    if (*args == '-')
    {
        free(sdf_data);
        DynamicPreprocessorFatalMessage("SDF rule cannot have a negative count:"
                " %s\n", args);
    }

    tmpcount = _dpd.SnortStrtoul(args, &endptr, 10);

    if (*endptr != ',')
    {
        free(sdf_data);
        DynamicPreprocessorFatalMessage("SDF rule configured with invalid "
                "arguments: %s\n", args);
    }

    if (tmpcount == 0 || tmpcount > 255)
    {
        free(sdf_data);
        DynamicPreprocessorFatalMessage("SDF rule needs to have a count between "
                " 1 - 255: %s\n", args);
    }

    sdf_data->count = (uint8_t)tmpcount;

    /* Take everything after the comma as a pattern. */
    token = endptr + 1;
    if (*token == '\0')
    {
        free(sdf_data);
        DynamicPreprocessorFatalMessage("SDF rule missing pattern: %s ", args);
    }
    if (strcasecmp(token, SDF_CREDIT_KEYWORD) == 0)
    {
        sdf_data->pii = strdup(SDF_CREDIT_PATTERN_ALL);
        sdf_data->validate_func = SDFLuhnAlgorithm;
    }
    else if (strcasecmp(token, SDF_SOCIAL_KEYWORD) == 0)
    {
        sdf_data->pii = strdup(SDF_SOCIAL_PATTERN);
        sdf_data->validate_func = SDFSocialCheck;
    }
    else if (strcasecmp(token, SDF_SOCIAL_NODASHES_KEYWORD) == 0)
    {
        sdf_data->pii = strdup(SDF_SOCIAL_NODASHES_PATTERN);
        sdf_data->validate_func = SDFSocialCheck;
    }
    else if (strcasecmp(token, SDF_EMAIL_KEYWORD) == 0)
    {
        sdf_data->pii = strdup(SDF_EMAIL_PATTERN);
    }
    else
    {
        sdf_data->pii = strdup(token);
        sdf_data->validate_func = NULL;
    }
    if (!sdf_data->pii)
    {
        free(sdf_data);
        DynamicPreprocessorFatalMessage("%s(%d) Failed to allocate memory for "
                "SDF pattern data.", __FILE__, __LINE__);
    }

    *data = (void *)sdf_data;
    return 1;
}

/* This function receives the OTN of a fully-parsed rule, checks that it is a
   SDF rule, then adds pattern & OTN to the SDF pattern-matching tree. */
int SDFOtnHandler(struct _SnortConfig *sc, void *potn)
{
    OptTreeNode *otn = (OptTreeNode *)potn;
    SDFConfig *config;
    tSfPolicyId policy_id;
    SDFOptionData *sdf_data;
    OptFpList *tmp = otn->opt_func;
    PreprocessorOptionInfo *preproc_info = NULL;
    tSfPolicyUserContextId context_to_use = sdf_context->context_id;
    sdf_tree_node *head_node_to_use = sdf_context->head_node;
    uint32_t *num_patterns_to_use = &sdf_context->num_patterns;
    int sdf_option_added = 0;

#ifdef SNORT_RELOAD
    /* If we are reloading, use that context instead.
       This should work since preprocessors get configured before rule parsing */
    SDFContext *sdf_swap_context;
    sdf_swap_context = (SDFContext *)_dpd.getRelatedReloadData(sc, "sensitive_data");
    if (sdf_swap_context != NULL)
    {
        context_to_use = sdf_swap_context->context_id;
        head_node_to_use = sdf_swap_context->head_node;
        num_patterns_to_use = &sdf_swap_context->num_patterns;
    }
#endif

    /* Retrieve the current policy being parsed */
    policy_id = _dpd.getParserPolicy(sc);
    sfPolicyUserPolicySet(context_to_use, policy_id);
    config = (SDFConfig *) sfPolicyUserDataGetCurrent(context_to_use);

    /* Check that this is a SDF rule, then grab the context data. */
    while (tmp != NULL && tmp->type != RULE_OPTION_TYPE_LEAF_NODE)
    {
        if (tmp->type == RULE_OPTION_TYPE_PREPROCESSOR)
            preproc_info = tmp->context;

        if (preproc_info == NULL ||
            preproc_info->optionEval != (PreprocOptionEval) SDFOptionEval)
        {
            DynamicPreprocessorFatalMessage("%s(%d) Rules with SDF options cannot "
                "have other detection options in the same rule.\n",
                *_dpd.config_file, *_dpd.config_line);
        }

        if (sdf_option_added)
        {
            DynamicPreprocessorFatalMessage("A rule may contain only one "
                "\"%s\" option.\n", SDF_OPTION_NAME);
        }

        if (otn->sigInfo.generator != GENERATOR_SPP_SDF_RULES)
        {
            DynamicPreprocessorFatalMessage("Rules with SDF options must "
                "use GID %d.\n", GENERATOR_SPP_SDF_RULES);
        }

        sdf_data = (SDFOptionData *)preproc_info->data;
        sdf_data->otn = otn;
        sdf_data->sid = otn->sigInfo.id;
        sdf_data->gid = otn->sigInfo.generator;

        /* Add the pattern to the SDF pattern-matching tree */
        AddPii(head_node_to_use, sdf_data);
        sdf_data->counter_index = (*num_patterns_to_use)++;

        AddPortsToConf(sc, config, otn);
        AddProtocolsToConf(sc, config, otn);

        sdf_option_added = 1;
        preproc_info = NULL;
        tmp = tmp->next;
    }

    return 1;
}

/* Take a port object's ports and add them to the preprocessor's port array. */
void AddPortsToConf(struct _SnortConfig *sc, SDFConfig *config, OptTreeNode *otn)
{
    int i, nports;
    char *src_parray, *dst_parray;
    RuleTreeNode *rtn;

    if (config == NULL || otn == NULL)
        return;

    /* RTNs vary based on which policy the rule appears in. */
    rtn = otn->proto_nodes[_dpd.getParserPolicy(sc)];

    /* Take the source port object and add ports to the preproc's array */
    src_parray = _dpd.portObjectCharPortArray(NULL, rtn->src_portobject, &nports);
    if (src_parray == 0)
    {
        /* This is an "any" port object! */
        for (i = 0; i < MAX_PORTS/8; i++)
        {
            config->src_ports[i] = 0xFF;
        }
    }
    else
    {
        /* iterate through an array of ports, add each one. */
        for (i = 0; i < MAX_PORTS; i++)
        {
            if (src_parray[i] == 1)
                config->src_ports[PORT_INDEX(i)] |= CONV_PORT(i);
        }
    }

    /* Repeat for destination ports. */
    dst_parray = _dpd.portObjectCharPortArray(NULL, rtn->dst_portobject, &nports);
    if (dst_parray == 0)
    {
        /* This is an "any" port object! */
        for (i = 0; i < MAX_PORTS/8; i++)
        {
            config->dst_ports[i] = 0xFF;
        }
    }
    else
    {
        /* iterate through an array of ports, add each one. */
        for (i = 0; i < MAX_PORTS; i++)
        {
            if (dst_parray[i] == 1)
                config->dst_ports[PORT_INDEX(i)] |= CONV_PORT(i);
        }
    }

    /* Cleanup */
    if (src_parray)
       free(src_parray);
    if (dst_parray)
        free(dst_parray);
}

void AddProtocolsToConf(struct _SnortConfig *sc, SDFConfig *config, OptTreeNode *otn)
{
#ifdef TARGET_BASED
    unsigned int i;
    int16_t ordinal;
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);

    if (config == NULL || otn == NULL)
        return;

    for (i = 0; i < otn->sigInfo.num_services; i++)
    {
        ordinal = otn->sigInfo.services[i].service_ordinal;
        if (ordinal > 0 && ordinal < MAX_PROTOCOL_ORDINAL)
            config->protocol_ordinals[ordinal] = 1;

        _dpd.streamAPI->set_service_filter_status(
            sc, ordinal, PORT_MONITOR_SESSION, policy_id, 1);
    }
#endif
}

/* Stub function -- We're not evaluating SDF during rule-matching */
int SDFOptionEval(void *p, const uint8_t **cursor, void *data)
{
    return RULE_NOMATCH;
}
