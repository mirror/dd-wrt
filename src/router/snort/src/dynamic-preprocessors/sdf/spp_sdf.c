/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2009-2013 Sourcefire, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "sf_types.h"
/*
#include "snort.h"
#include "parser.h"
#include "util.h"
#include "plugbase.h"
*/
#include "snort_debug.h"
#include "stream_api.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"
#include "sf_snort_packet.h"

/*
#ifdef TARGET_BASED
#include "sftarget_protocol_reference.h"
#endif
*/

#include "profiler.h"

#include "spp_sdf.h"
#include "sf_preproc_info.h"
#include "sdf_us_ssn.h"
#include "sdf_detection_option.h"
#include "sdf_pattern_match.h"

const int MAJOR_VERSION = 1;
const int MINOR_VERSION = 1;
const int BUILD_VERSION = 1;
const char *PREPROC_NAME = "SF_SDF";

#define SetupSDF DYNAMIC_PREPROC_SETUP

/* PROTOTYPES */
static void SDFInit(struct _SnortConfig *, char *args);
static void ProcessSDF(void *p, void *context);
static SDFConfig * NewSDFConfig(struct _SnortConfig *, tSfPolicyUserContextId);
static void ParseSDFArgs(SDFConfig *config, char *args);
static void SDFCleanExit(int signal, void *unused);
static int SDFFreeConfig(tSfPolicyUserContextId context, tSfPolicyId id, void *pData);
static void SDFFillPacket(sdf_tree_node *node, SDFSessionData *session,
                          SFSnortPacket *p, uint16_t *dlen);
static void SDFPrintPseudoPacket(SDFConfig *config, SDFSessionData *session,
                                 SFSnortPacket *real_packet);

#ifdef SNORT_RELOAD
static void SDFReload(struct _SnortConfig *, char *, void **);
static void * SDFReloadSwap(struct _SnortConfig *, void *);
static void SDFReloadSwapFree(void *);
#endif

/* GLOBALS :( */
SDFContext *sdf_context = NULL;
static uint32_t sdf_config_count = 0;

#ifdef SNORT_RELOAD
sdf_tree_node *swap_head_node = NULL;
uint32_t swap_num_patterns = 0;
#endif

#ifdef PERF_PROFILING
PreprocStats sdfPerfStats;
#endif

#define IPPROTO_SDF 0xFE  // TBD - use same for ps?  (eg IPPROTO_SNORT?)

/*
 * Function: SetupSDF()
 *
 * Purpose: Registers the preprocessor keyword and initialization function
 *          into the preprocessor list.
 *
 * Arguments: None.
 *
 * Returns: void
 *
 */
void SetupSDF(void)
{
#ifndef SNORT_RELOAD
    _dpd.registerPreproc("sensitive_data", SDFInit);
#else
    _dpd.registerPreproc("sensitive_data", SDFInit, SDFReload, NULL, SDFReloadSwap,
                         SDFReloadSwapFree);
#endif
}

/*
 * Function: SDFInit(char *)
 *
 * Purpose: Processes the args sent to the preprocessor, sets up the port list,
 *          links the processing function into the preproc function list
 *
 * Arguments: args => ptr to argument string
 *
 * Returns: void
 *
 */
void SDFInit(struct _SnortConfig *sc, char *args)
{
    SDFConfig *config = NULL;

    /* Check prerequisites */
    if (_dpd.streamAPI == NULL)
    {
        DynamicPreprocessorFatalMessage("SDFInit(): The Stream preprocessor must be enabled.\n");
    }

    /* Create context id, register callbacks. This is only done once. */
    if (sdf_context == NULL)
    {
        sdf_context = (SDFContext *)calloc(1, sizeof(*sdf_context));
        if (!sdf_context)
            DynamicPreprocessorFatalMessage("Failed to allocate memory for SDF "
                                            "configuration.\n");
        sdf_context->context_id = sfPolicyConfigCreate();
        if (!sdf_context->context_id)
            DynamicPreprocessorFatalMessage("Failed to allocate memory for SDF "
                                            "configuration.\n");
        sdf_context->head_node = (sdf_tree_node *)calloc(1, sizeof(*sdf_context->head_node));
        if (!sdf_context->head_node)
            DynamicPreprocessorFatalMessage("Failed to allocate memory for SDF "
                                            "configuration.\n");

        _dpd.addPreprocExit(SDFCleanExit, NULL, PRIORITY_LAST, PP_SDF);

#ifdef PERF_PROFILING
        _dpd.addPreprocProfileFunc("sensitive_data", (void *)&sdfPerfStats, 0, _dpd.totalPerfStats, NULL);
#endif
    }

    /* Handle configuration. This is done once for each policy. */
    config = NewSDFConfig(sc, sdf_context->context_id);
    ParseSDFArgs(config, args);

    /* Register callbacks */
    _dpd.addDetect(sc, ProcessSDF, PRIORITY_FIRST, PP_SDF,
                    PROTO_BIT__TCP | PROTO_BIT__UDP);
    _dpd.preprocOptRegister(sc, SDF_OPTION_NAME, SDFOptionInit, SDFOptionEval,
                            NULL, NULL, NULL, SDFOtnHandler, NULL);
}


/* Check the ports and target-based protocol for a given packet.
 *
 * Returns: 0 if the port check fails
 *          1 if the packet should be inspected
 */
static int SDFCheckPorts(SDFConfig *config, SFSnortPacket *packet)
{
#ifdef TARGET_BASED
    int16_t app_ordinal = SFTARGET_UNKNOWN_PROTOCOL;

    /* Do port checks */
    app_ordinal = _dpd.sessionAPI->get_application_protocol_id(packet->stream_session);
    if (app_ordinal == SFTARGET_UNKNOWN_PROTOCOL)
        return 0;
    if (app_ordinal && (config->protocol_ordinals[app_ordinal] == 0))
        return 0;
    if (app_ordinal == 0)
    {
#endif
        /* No target-based info for this packet. Check ports. */
        if (((config->src_ports[PORT_INDEX(packet->src_port)] & CONV_PORT(packet->src_port)) == 0) ||
            ((config->dst_ports[PORT_INDEX(packet->dst_port)] & CONV_PORT(packet->dst_port)) == 0))
        {
            return 0;
        }
#ifdef TARGET_BASED
    }
#endif

    return 1;
}

/* A free function that gets registered along with our Stream session data */
static void FreeSDFSession(void *data)
{
    SDFSessionData *session = (SDFSessionData *)data;

    if (session == NULL)
        return;

    free(session->counters);
    free(session->rtns_matched);
    free(session);
    return;
}

/* Create a new SDF session data struct.
   Returns:
      Fatal Error if allocation fails
      Valid ptr otherwise
*/
static SDFSessionData * NewSDFSession(SDFConfig *config, SFSnortPacket *packet)
{
    SDFSessionData *session;

    /* Allocate new session data. */
    session = (SDFSessionData *) malloc(sizeof(SDFSessionData));
    if (session == NULL)
    {
        DynamicPreprocessorFatalMessage("Failed to allocate memory for "
                "SDF preprocessor session data.\n");
    }

    if (packet->stream_session)
    {
        _dpd.sessionAPI->set_application_data(packet->stream_session,
                                             PP_SDF, session, FreeSDFSession);
    }

    session->part_match_node= NULL;
    session->part_match_index = 0;
    session->global_counter = 0;
    session->config_num = config->config_num;
    session->last_pkt_seq_num = 0;
    session->last_pkt_data_len = -1;
    /* Allocate counters in the session data */
    session->num_patterns = sdf_context->num_patterns;
    session->counters = calloc(session->num_patterns, sizeof(uint8_t));
    session->rtns_matched = calloc(session->num_patterns, sizeof(int8_t));
    if (session->counters == NULL || session->rtns_matched == NULL)
    {
        DynamicPreprocessorFatalMessage("Failed to allocate memory for "
                "SDF preprocessor session data.\n");
    }

    return session;
}

static inline bool isBufferInPayload(char *begin, char *end, SFSnortPacket *packet)
{
    if ((end <= (char *) packet->payload + packet->payload_size)
        && (begin >= (char *) packet->payload))
        return true;
    else
        return false;
}

static void SDFSearchRecursively(SDFConfig *config, SFSnortPacket *packet,
                      SDFSessionData *session, sdf_tree_node *matched_node, 
                      char **position, uint16_t *buflen, uint16_t match_length, bool *ob_failed)
{
    /* Iterate through the SDFOptionData that matches this pattern. */
    uint16_t i;
    for (i = 0; i < matched_node->num_option_data; i++)
    {
        SDFOptionData *found_pattern = matched_node->option_data_list[i];
        if (found_pattern->match_success)
        {
            int index;
            /* Reset the match_success flag for subsequent matches */
            found_pattern->match_success = 0;

            /* Check the RTN for the PII we found. The IPs & ports might not match.
               We only want to do this once per session */
            index = found_pattern->counter_index;
            if (session->rtns_matched[index] == 0)
            {
                bool check_ports = true;
                OptTreeNode *otn = found_pattern->otn;
                RuleTreeNode *rtn = NULL;
#ifdef TARGET_BASED
                int16_t app_ordinal;
#endif

                if (_dpd.getIpsRuntimePolicy() < otn->proto_node_num)
                    rtn = otn->proto_nodes[_dpd.getIpsRuntimePolicy()];

#ifdef TARGET_BASED
                /* Check the service against the matched OTN. */
                app_ordinal = _dpd.sessionAPI->get_application_protocol_id(packet->stream_session);
                if( app_ordinal != SFTARGET_UNKNOWN_PROTOCOL )
                {
                    unsigned int i;
                    for (i = 0; i < otn->sigInfo.num_services; i++)
                    {
                        if (otn->sigInfo.services[i].service_ordinal == app_ordinal)
                        {
                            check_ports = false;
                            break;
                        }
                    }
                }
#endif
                if (rtn != NULL && _dpd.fpEvalRTN(rtn, packet, check_ports))
                    session->rtns_matched[index] = 1;
                else
                    session->rtns_matched[index] = -1;
            }

            if (session->rtns_matched[index] == 1)
            {
                /* Increment counters */
                session->counters[found_pattern->counter_index]++;

                /* Obfuscate the data.
                   We do this even if it's not time to alert, to obfuscate each match.
                   Only obfuscate built-in patterns */
                if (config->mask_output && found_pattern->validate_func)
                {
                    if (isBufferInPayload(*position, *position + match_length, packet))
                    {
                        uint16_t offset, ob_length = 0;

                        offset = (uint16_t) ((*position) - (char *)packet->payload);

                        if (match_length > SDF_OBFUSCATION_DIGITS_SHOWN)
                            ob_length = match_length - SDF_OBFUSCATION_DIGITS_SHOWN;

                        /* Generally, the CC# and SS# patterns contain non-digits on either
                         * side of the actual number. Sometimes, for the patterns from the
                         * first line of the data might start  with a digit, instead of a
                         * non-digit. Adjust the mask to match.
                         */
                        if (isdigit((int)*position[0]))
                            ob_length = ob_length - 1;

                        else
                        {
                            offset = offset + 1;
                            ob_length = ob_length - 2;
                        }
                        
                        _dpd.obApi->addObfuscationEntry(packet, offset, ob_length,
                                                        SDF_OBFUSCATION_CHAR);
                    }
                    else
                    {
                        *ob_failed = true;
                    }
                }

                if (session->counters[found_pattern->counter_index] == found_pattern->count)
                {

                    /* Raise the alert for this particular pattern */
                    _dpd.alertAdd(GENERATOR_SPP_SDF_RULES,
                                  found_pattern->otn->sigInfo.id,
                                  found_pattern->otn->sigInfo.rev,
                                  found_pattern->otn->sigInfo.class_id,
                                  found_pattern->otn->sigInfo.priority,
                                  found_pattern->otn->sigInfo.message,
                                  0);
                }
            }
        }
    }

    /* Check the global counter and alert */
    session->global_counter++;
    if (session->global_counter == config->threshold)
    {
        /* Do our "combo alert" */
        SDFPrintPseudoPacket(config, session, packet);
        _dpd.genSnortEvent(config->pseudo_packet,
                           GENERATOR_SPP_SDF_PREPROC,
                           SDF_COMBO_ALERT_SID,
                           SDF_COMBO_ALERT_REV,
                           SDF_COMBO_ALERT_CLASS,
                           SDF_COMBO_ALERT_PRIORITY,
                           SDF_COMBO_ALERT_STR);
    }

    /* Update position */
    if (match_length > 1)
    {
        (*position) += match_length - 1;
        (*buflen) -= match_length - 1;
    }
}

/* Search a buffer for PII. Generates alerts when enough PII is found.
   Returns: void
*/
static void SDFSearch(SDFConfig *config, SFSnortPacket *packet,
                      SDFSessionData *session, char *position, char *end,
                      uint16_t buflen, bool *ob_failed)
{
    uint16_t match_length = 0;
    sdf_tree_node *matched_node = NULL;
    uint16_t *partial_index = &(session->part_match_index);
    sdf_tree_node **partial_node = &(session->part_match_node);
    /* Check to see if there was a partial match */ 
    if(*partial_index > 0)
    {
        if( position < end )
        {
            sdf_tree_node *node = *partial_node;
            if(strlen(node->pattern) == *partial_index)
            {
                int i = 0;
                while ((i < node->num_children) && matched_node == NULL)
                {
                    *partial_index = 0;
                    matched_node = FindPiiRecursively(node->children[i], position, &match_length, buflen, config, partial_index, partial_node);
                    i++;
                }
            }
            else
            {
                matched_node = FindPiiRecursively(node, position, &match_length, buflen, config, partial_index, partial_node);
            }

            /* only when matched update the position ptr. FindPiiRecursively only checks one node unlike FindPii */
            if (matched_node)
                SDFSearchRecursively(config, packet, session, matched_node, &position, &buflen, match_length, ob_failed);
            else if (*partial_index)
            {
                position += match_length;
                buflen -= match_length;
            }
        }
        else
        {
            return;
        }
    }

    while (position < end)
    {
        match_length = 0;
        matched_node = NULL;

        /* Traverse the pattern tree and match PII against our data */
        matched_node = FindPii(sdf_context->head_node, position, &match_length,
                               buflen, config, session);

        if (matched_node)
            SDFSearchRecursively(config, packet, session, matched_node, &position, &buflen, match_length, ob_failed);
        else if (*partial_index)
        {
            position += match_length;
            buflen -= match_length;
        }
        else
        {
            position++;
            buflen--;
        }
    }
}

/*
 * Function: ProcessSDF(void *, void *)
 *
 * Purpose: Inspects a packet's payload for Personally Identifiable Information
 *
 * Arguments: p => poitner to the current packet data struct
 *            context => unused void pointer
 *
 * Returns: void
 *
 */
static void ProcessSDF(void *p, void *context)
{
    tSfPolicyId policy_id;
    SDFConfig *config = NULL;
    SFSnortPacket *packet = (SFSnortPacket *)p;
    SDFSessionData *session;
    char *begin, *end;
    uint16_t buflen;
    bool payload_checked = false;
    bool ob_failed = false;

    PROFILE_VARS;

    // preconditions - what we registered for
    assert((IsUDP(packet) || IsTCP(packet)) &&
        packet->payload && packet->payload_size);

    /* Check if we should be working on this packet */
    if ( packet->flags & FLAG_STREAM_INSERT && (!(packet->flags & FLAG_PDU_TAIL)) )
        return;  // Waiting on stream reassembly

    /* Retrieve the corresponding config for this packet */
    policy_id = _dpd.getIpsRuntimePolicy();
    sfPolicyUserPolicySet (sdf_context->context_id, policy_id);
    config = sfPolicyUserDataGetCurrent(sdf_context->context_id);

    /* Retrieve stream session data. Create one if it doesn't exist. */
    session = _dpd.sessionAPI->get_application_data(packet->stream_session, PP_SDF);
    if (session == NULL)
    {
        char pseudo_start[1] = {'0'};
        /* Do port checks */
        if (SDFCheckPorts(config, packet) == 0)
        {
            return;
        }

        /* If there's no stream session, we'll just count PII for one packet */
        if (packet->stream_session == NULL)
        {
            if (config->stateless_session == NULL)
                config->stateless_session = NewSDFSession(config, packet);

            session = config->stateless_session;
            memset(session->counters, 0, session->num_patterns);
            memset(session->rtns_matched, 0, session->num_patterns);
        }
        else
            session = NewSDFSession(config, packet);

        /* Add one byte to support sensitive data starts with first byte */
        begin = pseudo_start;
        buflen = 1;
        end = begin + buflen;
        SDFSearch(config, packet, session, begin, end, buflen, &ob_failed);

    }
    else if( session->config_num != config->config_num )
    {
        /* Config has changed. Don't use rule tree nodes from previous config */
        session->part_match_index = 0;
        session->part_match_node = NULL;
        /* Update the session's config num */
        session->config_num = config->config_num;
    }

    PREPROC_PROFILE_START(sdfPerfStats);

    /* By First checking ports and protocols for a given packet , Snort is able to limit the number of rules that must be evaluated.
       prmFindRuleGroup()  will be called to make sure a match exists for the source and destination ports mentioned in the rule. 
       Sometimes more than one rule group will be matched to the same packet. This is quick way to eliminate the rules without complex pattern matching. 
       Sometimes same packet will be evaluated by Sensitive Data Preprocessor  more than once because packet is matched to more than one rule group.  
       When evaluating same packet for each rule group by SDF,  PII count  will be incremented for the same packet which causes mis-firing alert. 
       Need to avoid evaluating same packet multiple times by SDF.

    */
    if(packet->tcp_header)
    {
         if( (session->last_pkt_seq_num == packet->tcp_header->sequence) )
         {
              if( (_dpd.fileDataBuf->len == 0 ) ||
                  (session->last_pkt_data_len != _dpd.fileDataBuf->len ))
              {
                    // Process the same packet if file data buffer len is diffrent or zero. 
                    session->last_pkt_data_len = _dpd.fileDataBuf->len;
              }
              else
              {
                   // Do not evaluate the same packet if file data buffer len is same.
                   session->last_pkt_data_len = _dpd.fileDataBuf->len;
                   return;
              }
         }
         else
         {
              session->last_pkt_seq_num = packet->tcp_header->sequence;
              session->last_pkt_data_len = _dpd.fileDataBuf->len;
         }
    } 

    /* Inspect HTTP Body or Email attachments. */
    if (_dpd.fileDataBuf->len > 0)
    {
        begin = (char *) _dpd.fileDataBuf->data;
        buflen = _dpd.fileDataBuf->len;
        end = begin + buflen;

        SDFSearch(config, packet, session, begin, end, buflen, &ob_failed);
    }
    else if ( PacketHasPAFPayload(packet) )
    {
        /* SDF already requires stream to be enabled, might as well look
         * at the rebuilt packet */

        begin = (char *)packet->payload;
        buflen = packet->payload_size;
        end = begin + buflen;

        SDFSearch(config, packet, session, begin, end, buflen, &ob_failed);
        payload_checked = true;
    }

    /* If this packet is HTTP, inspect the URI and Client Body while ignoring
     * headers. */
    if (packet->flags & FLAG_HTTP_DECODE)
    {
        unsigned len;
        begin = (char*)_dpd.getHttpBuffer(HTTP_BUFFER_URI, &len);

        if ( begin )
        {
            buflen = (uint16_t)len;
            end = begin + buflen;
            if (!payload_checked || !isBufferInPayload(begin, end, packet))
                SDFSearch(config, packet, session, begin, end, buflen, &ob_failed);
        }
        begin = (char*)_dpd.getHttpBuffer(HTTP_BUFFER_CLIENT_BODY, &len);

        if ( begin )
        {
            buflen = (uint16_t)len;
            end = begin + buflen;
            if (!payload_checked || !isBufferInPayload(begin, end, packet))
                SDFSearch(config, packet, session, begin, end, buflen, &ob_failed);
        }
    }

    /* Found match but not in payload, recheck to mask the rebuilt packet */
    if ( !payload_checked && ob_failed && PacketHasPAFPayload(packet) )
    {
        begin = (char *)packet->payload;
        buflen = packet->payload_size;
        end = begin + buflen;

        SDFSearch(config, packet, session, begin, end, buflen, &ob_failed);
        payload_checked = true;
    }

    /* End. */
    PREPROC_PROFILE_END(sdfPerfStats);
    return;
}

static void DisplaySDFConfig(SDFConfig *config)
{
    if (config == NULL)
        return;

    _dpd.logMsg("Sensitive Data preprocessor config: \n");
    _dpd.logMsg("    Global Alert Threshold: %d\n", config->threshold);
    _dpd.logMsg("    Masked Output: %s\n",
            config->mask_output ? "ENABLED" : "DISABLED" );
}

/*
 * Function: ParseSDFArgs(SDFConfig *, char *)
 *
 * Purpose: Parse the arguments to the SDF preprocessor and instantiate a
 *          SDFConfig struct.
 *
 * Arguments: config => pointer to a newly-allocated SDFConfig struct, which
 *                      will be modified.
 *            args => pointer to string containing SDF preproc arguments.
 *
 * Returns: void
 *
 */
static void ParseSDFArgs(SDFConfig *config, char *args)
{
    char *argcpy = NULL;
    char *cur_tokenp = NULL;

    if (config == NULL || args == NULL) return;

    /* Set default options */
    SSNSetDefaultGroups(config);

    /* Copy args so that we can break them up wtih strtok */
    argcpy = strdup(args);
    if (argcpy == NULL)
        DynamicPreprocessorFatalMessage("Could not allocate memory to parse "
                                        "SDF options.\n");

    cur_tokenp = strtok(argcpy, " ");

    /* Loop through config options */
    while (cur_tokenp)
    {
        /* Parse the global PII threshold */
        if (!strcmp(cur_tokenp, SDF_THRESHOLD_KEYWORD))
        {
            char *endptr;

            cur_tokenp = strtok(NULL, " ");
            if (cur_tokenp == NULL)
            {
                DynamicPreprocessorFatalMessage("SDF preprocessor config option "
                        "\"%s\" requires an argument.\n", SDF_THRESHOLD_KEYWORD);
            }

            if (*cur_tokenp == '-')
            {
                DynamicPreprocessorFatalMessage("SDF preprocessor config option "
                        "\"%s\" cannot take a negative argument.\n",
                        SDF_THRESHOLD_KEYWORD);
            }

            config->threshold = _dpd.SnortStrtoul(cur_tokenp, &endptr, 10);
            if (config->threshold == 0 || config->threshold > USHRT_MAX)
            {
                DynamicPreprocessorFatalMessage("SDF preprocessor config option "
                        "\"%s\" must have an argument between 1 - %u.\n",
                        SDF_THRESHOLD_KEYWORD, USHRT_MAX);
            }
            if (*endptr != '\0')
            {
                DynamicPreprocessorFatalMessage("Invalid argument to SDF config "
                        "option \"%s\": %s", SDF_THRESHOLD_KEYWORD, cur_tokenp);
            }
        }
        /* Parse the output masking option */
        else if (!strcmp(cur_tokenp, SDF_MASK_KEYWORD))
        {
            config->mask_output = 1;
        }
        /* Parse the file containing new SSN group data */
        else if (!strcmp(cur_tokenp, SDF_SSN_FILE_KEYWORD))
        {
            int iRet;

            cur_tokenp = strtok(NULL, " ");
            if (cur_tokenp == NULL)
            {
                DynamicPreprocessorFatalMessage("SDF preprocessor config option "
                        "\"%s\" requires an argument.\n", SDF_SSN_FILE_KEYWORD);
            }

            iRet = ParseSSNGroups(cur_tokenp, config);
            if (iRet < 0)
            {
                DynamicPreprocessorFatalMessage("Error parsing Social Security "
                        "group data from file: %s", cur_tokenp);
            }
        }

        else
        {
                DynamicPreprocessorFatalMessage("%s(%d) => Unknown SDF configuration option %s\n",
                                            *(_dpd.config_file), *(_dpd.config_line), cur_tokenp);
        }

        cur_tokenp = strtok(NULL, " ");
    }

    /* Cleanup */
    DisplaySDFConfig(config);
    free(argcpy);
    argcpy = NULL;
}

/* Allocate & Initialize the pseudo-packet used for logging combo alerts.
 *
 * Returns: 0 on success, -1 on error.
 */
static int SDFPacketInit(SDFConfig *config)
{
    config->pseudo_packet = _dpd.encodeNew();
    return 0;
}

/*
 * Function: NewSDFConfig(void)
 *
 * Purpose: Create a new SDFConfig for the current parser policy.
 *
 * Arguments: context => context ID to use when creating config
 *
 * Returns: Pointer to newly created SDFConfig struct.
 *
 */
static SDFConfig * NewSDFConfig(struct _SnortConfig *sc, tSfPolicyUserContextId context)
{
    SDFConfig *config = NULL;
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);

    /* Check for an existing configuration in this policy */
    sfPolicyUserPolicySet(context, policy_id);

    config = (SDFConfig *) sfPolicyUserDataGetCurrent(context);
    if (config)
        DynamicPreprocessorFatalMessage("SDF preprocessor can only be "
                                        "configured once.\n");

    /* Create and store config */
    config = (SDFConfig *)calloc(1, sizeof(SDFConfig));
    if (!config)
        DynamicPreprocessorFatalMessage("Failed to allocate memory for SDF "
                                        "configuration.\n");
    sfPolicyUserDataSetCurrent(context, config);

    /* Allocate the pseudo-packet used for logging */
    SDFPacketInit(config);
    config->config_num = sdf_config_count++;

    return config;
}

/*
 * Function: SDFCleanExit(int, void *)
 *
 * Purpose: Free memory used by the SDF preprocessor before Snort exits.
 *
 * Arguments: Signal sent to Snort, unused void pointer
 *
 * Returns: void
 *
 */
static void SDFCleanExit(int signal, void *unused)
{
    /* Free the individual configs. */
    if (sdf_context == NULL)
        return;

    sfPolicyUserDataFreeIterate(sdf_context->context_id, SDFFreeConfig);
    sfPolicyConfigDelete(sdf_context->context_id);
    FreePiiTree(sdf_context->head_node);
    free(sdf_context);
    sdf_context = NULL;
}

/*
 * Function: SDFFreeConfig(tSfPolicyUserContextId, tSfPolicyId, void *)
 *
 * Purpose: Callback that frees a SDFConfig struct correctly, and clears data
 *          from the policy.
 *
 * Arguments: context => context ID for the SDF preprocessor
 *            id => policy ID for the policy being destroyed
 *            pData => pointer to SDFConfig struct that gets freed
 *
 * Returns: zero
 *
 */
static int SDFFreeConfig(tSfPolicyUserContextId context, tSfPolicyId id, void *pData)
{
    SDFConfig *config = (SDFConfig *)pData;

    sfPolicyUserDataClear(context, id);

    _dpd.encodeDelete(config->pseudo_packet);
    FreeSDFSession(config->stateless_session);

    free(config);
    return 0;
}

#ifdef SNORT_RELOAD
static void SDFReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    SDFContext *sdf_swap_context = (SDFContext *)*new_config;
    SDFConfig *config = NULL;

    if (sdf_swap_context == NULL)
    {
        if (!_dpd.streamAPI)
            DynamicPreprocessorFatalMessage("SetupSDF(): The Stream preprocessor "
                                            "must be enabled.\n");

        sdf_swap_context = (SDFContext *)calloc(1, sizeof(*sdf_context));
        if (!sdf_swap_context)
            DynamicPreprocessorFatalMessage("Failed to allocate memory for SDF "
                                            "configuration.\n");
        sdf_swap_context->context_id = sfPolicyConfigCreate();
        if (!sdf_swap_context->context_id)
            DynamicPreprocessorFatalMessage("Failed to allocate memory for SDF "
                                            "configuration.\n");
        sdf_swap_context->head_node = (sdf_tree_node *)calloc(1, sizeof(*sdf_swap_context->head_node));
        if (!sdf_swap_context->head_node)
            DynamicPreprocessorFatalMessage("Failed to allocate memory for SDF "
                                            "configuration.\n");
        *new_config = (void *)sdf_swap_context;
    }

    config = NewSDFConfig(sc, sdf_swap_context->context_id);
    ParseSDFArgs(config, args);

    _dpd.addDetect(sc, ProcessSDF, PRIORITY_FIRST, PP_SDF,
            PROTO_BIT__TCP | PROTO_BIT__UDP);
    _dpd.preprocOptRegister(sc, SDF_OPTION_NAME, SDFOptionInit, SDFOptionEval,
                                NULL, NULL, NULL, SDFOtnHandler, NULL);
}

static void * SDFReloadSwap(struct _SnortConfig *sc, void *swap_config)
{
    SDFContext *sdf_swap_context = (SDFContext *)swap_config;
    SDFContext *old_context = sdf_context;

    if (old_context == NULL || sdf_swap_context == NULL)
        return NULL;

    sdf_context = sdf_swap_context;

    return (void *) old_context;
}

static void SDFReloadSwapFree(void *data)
{
    SDFContext *context = (SDFContext *) data;
    if (context == NULL)
        return;

    sfPolicyUserDataFreeIterate(context->context_id, SDFFreeConfig);
    sfPolicyConfigDelete(context->context_id);
    FreePiiTree(context->head_node);
    free(context);
}
#endif

static void SDFPrintPseudoPacket(SDFConfig *config, SDFSessionData *session,
                                 SFSnortPacket *real_packet)
{
    SFSnortPacket* p;

    if (config == NULL || session == NULL || real_packet == NULL)
        return;

    p = config->pseudo_packet;

    _dpd.encodeFormat(ENC_DYN_FWD|ENC_DYN_NET, real_packet, config->pseudo_packet, PSEUDO_PKT_SDF);

    if ( IS_IP4(real_packet) )
    {
        ((IPV4Header *)p->ip4_header)->proto = IPPROTO_SDF;
        p->inner_ip4h.ip_proto = IPPROTO_SDF;
    }
    else if (IS_IP6(p))
    {
        // FIXTHIS assumes there are no ip6 extension headers
        p->inner_ip6h.next = IPPROTO_SDF;
        p->ip6h = &p->inner_ip6h;
    }

    /* Fill in the payload with SDF alert info */
    SDFFillPacket(sdf_context->head_node, session, p, &p->payload_size);

    _dpd.encodeUpdate(config->pseudo_packet);

    if (real_packet->family == AF_INET)
    {
        p->ip4h->ip_len = p->ip4_header->data_length;
    }
    else
    {
        IP6RawHdr* ip6h = (IP6RawHdr*)p->raw_ip6_header;
        if ( ip6h ) p->ip6h->len = ip6h->ip6_payload_len;
    }
}

/* This function traverses the pattern tree and prints out the relevant
 * info into a provided pseudo-packet. */
static void SDFFillPacket(sdf_tree_node *node, SDFSessionData *session,
                          SFSnortPacket *p, uint16_t *dlen)
{
    uint16_t i;

    if (node == NULL || session == NULL || p == NULL || dlen == NULL)
        return;

    /* Recurse to the leaves of the pattern tree */
    for (i = 0; i < node->num_children; i++)
    {
        SDFFillPacket(node->children[i], session, p, dlen);
    }

    for (i = 0; i < node->num_option_data; i++)
    {
        SDFOptionData * option_data = node->option_data_list[i];

        /* Print the info from leaves */
        if (option_data)
        {
            uint32_t index = option_data->counter_index;
            uint8_t counter = session->counters[index];
            if (counter > 0)
            {
                /* Print line */
                const char *sigmessage = option_data->otn->sigInfo.message;
                uint8_t *dest = (uint8_t*)p->payload + *dlen;
                size_t siglen = strlen(sigmessage);
                uint16_t space_left = p->max_payload - *dlen;

                if (space_left < siglen + SDF_ALERT_LENGTH)
                    return;

                *dlen += (siglen + SDF_ALERT_LENGTH);
                snprintf((char *)dest, space_left, "%s: %3d", sigmessage, counter);
            }
        }
    }

    return;
}


