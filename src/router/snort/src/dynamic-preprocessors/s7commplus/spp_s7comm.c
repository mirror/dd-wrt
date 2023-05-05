/*
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
 * Copyright (C) 2020-2022 Cisco and/or its affiliates. All rights reserved.
 *
 * Authors: Jeffrey Gu <jgu@cisco.com>, Pradeep Damodharan <prdamodh@cisco.com>
 *
 * Dynamic preprocessor for the S7commplus protocol
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <assert.h>
#include <string.h>

#include "sf_types.h"
#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"
#include "sf_snort_plugin_api.h"
#include "snort_debug.h"

#include "preprocids.h"
#include "spp_s7comm.h"
#include "sf_preproc_info.h"

#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats s7commplusPerfStats;
#endif

#include "sf_types.h"
#include "s7comm_decode.h"
#include "s7comm_roptions.h"
#include "s7comm_paf.h"

const int MAJOR_VERSION = 1;
const int MINOR_VERSION = 0;
const int BUILD_VERSION = 1;
const char *PREPROC_NAME = "SF_S7COMMPLUS";

#define SetupS7commplus DYNAMIC_PREPROC_SETUP

/* Preprocessor config objects */
static tSfPolicyUserContextId s7commplus_context_id = NULL;
static s7commplus_config_t *s7commplus_eval_config = NULL;

/* Target-based app ID */
#ifdef TARGET_BASED
int16_t s7commplus_app_id = SFTARGET_UNKNOWN_PROTOCOL;
#endif

/* Prototypes */
static void S7commplusInit(struct _SnortConfig *, char *);
static inline void S7commplusOneTimeInit(struct _SnortConfig *);
static inline s7commplus_config_t * S7commplusPerPolicyInit(struct _SnortConfig *, tSfPolicyUserContextId);

static void ProcessS7commplus(void *, void *);

#ifdef SNORT_RELOAD
static void S7commplusReload(struct _SnortConfig *, char *, void **);
static int S7commplusReloadVerify(struct _SnortConfig *, void *);
static void * S7commplusReloadSwap(struct _SnortConfig *, void *);
static void S7commplusReloadSwapFree(void *);
#endif

static void registerPortsForDispatch( struct _SnortConfig *sc, s7commplus_config_t *policy );
static void registerPortsForReassembly( s7commplus_config_t *policy, int direction );
static void _addPortsToStreamFilter(struct _SnortConfig *, s7commplus_config_t *, tSfPolicyId);
#ifdef TARGET_BASED
static void _addServicesToStreamFilter(struct _SnortConfig *, tSfPolicyId);
#endif

static void S7commplusFreeConfig(tSfPolicyUserContextId context_id);
static void FreeS7commplusData(void *);
static int S7commplusCheckConfig(struct _SnortConfig *);
static void S7commplusCleanExit(int, void *);

static void ParseS7commplusArgs(s7commplus_config_t *config, char *args);
static void S7commplusPrintConfig(s7commplus_config_t *config);

static int S7commplusPortCheck(s7commplus_config_t *config, SFSnortPacket *packet);
static s7commplus_session_data_t * S7commplusCreateSessionData(SFSnortPacket *);

/* Register init callback */
void SetupS7commplus(void)
{
#ifndef SNORT_RELOAD
    _dpd.registerPreproc("s7commplus", S7commplusInit);
#else
    _dpd.registerPreproc("s7commplus", S7commplusInit, S7commplusReload,
                         S7commplusReloadVerify, S7commplusReloadSwap,
                         S7commplusReloadSwapFree);
#endif
}

/* Allocate memory for preprocessor config, parse the args, set up callbacks */
static void S7commplusInit(struct _SnortConfig *sc, char *argp)
{
    s7commplus_config_t *s7commplus_policy = NULL;

    if (s7commplus_context_id == NULL)
    {
        S7commplusOneTimeInit(sc);
    }

    s7commplus_policy = S7commplusPerPolicyInit(sc, s7commplus_context_id);

    ParseS7commplusArgs(s7commplus_policy, argp);

    /* Can't add ports until they've been parsed... */
    S7commplusAddPortsToPaf(sc, s7commplus_policy, _dpd.getParserPolicy(sc));
#ifdef TARGET_BASED
    S7commplusAddServiceToPaf(sc, s7commplus_app_id, _dpd.getParserPolicy(sc));
#endif
    // register ports with session and stream
    registerPortsForDispatch(sc, s7commplus_policy );
    registerPortsForReassembly( s7commplus_policy, SSN_DIR_FROM_SERVER | SSN_DIR_FROM_CLIENT );
    
    S7commplusPrintConfig(s7commplus_policy);
}

static inline void S7commplusOneTimeInit(struct _SnortConfig *sc)
{
    /* context creation & error checking */
    s7commplus_context_id = sfPolicyConfigCreate();
    if (s7commplus_context_id == NULL)
    {
        _dpd.fatalMsg("%s(%d) Failed to allocate memory for "
                      "S7commplus config.\n", *_dpd.config_file, *_dpd.config_line);
    }

    if (_dpd.streamAPI == NULL)
    {
        _dpd.fatalMsg("%s(%d) SetupS7commplus(): The Stream preprocessor "
                      "must be enabled.\n", *_dpd.config_file, *_dpd.config_line);
    }

    /* callback registration */
    _dpd.addPreprocConfCheck(sc, S7commplusCheckConfig);
    _dpd.addPreprocExit(S7commplusCleanExit, NULL, PRIORITY_LAST, PP_S7COMMPLUS);

#ifdef PERF_PROFILING
    _dpd.addPreprocProfileFunc("s7commplus", (void *)&s7commplusPerfStats, 0, _dpd.totalPerfStats, NULL);
#endif

    /* Set up target-based app id */
#ifdef TARGET_BASED
    s7commplus_app_id = _dpd.findProtocolReference("cotp");
    if (s7commplus_app_id == SFTARGET_UNKNOWN_PROTOCOL)
        s7commplus_app_id = _dpd.addProtocolReference("s7commplus");

// register with session to handle applications
    _dpd.sessionAPI->register_service_handler( PP_S7COMMPLUS, s7commplus_app_id );

#endif
}

/* Responsible for allocating a S7commplus policy. Never returns NULL. */
static inline s7commplus_config_t * S7commplusPerPolicyInit(struct _SnortConfig *sc, tSfPolicyUserContextId context_id)
{
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    s7commplus_config_t *s7commplus_policy = NULL;

    /* Check for existing policy & bail if found */
    sfPolicyUserPolicySet(context_id, policy_id);
    s7commplus_policy = (s7commplus_config_t *)sfPolicyUserDataGetCurrent(context_id);
    if (s7commplus_policy != NULL)
    {
        _dpd.fatalMsg("%s(%d) S7commplus preprocessor can only be "
                      "configured once.\n", *_dpd.config_file, *_dpd.config_line);
    }

    /* Allocate new policy */
    s7commplus_policy = (s7commplus_config_t *)calloc(1, sizeof(s7commplus_config_t));
    if (!s7commplus_policy)
    {
        _dpd.fatalMsg("%s(%d) Could not allocate memory for "
                      "s7commplus preprocessor configuration.\n"
                      , *_dpd.config_file, *_dpd.config_line);
    }

    sfPolicyUserDataSetCurrent(context_id, s7commplus_policy);

    /* Register callbacks that are done for each policy */
    _dpd.addPreproc(sc, ProcessS7commplus, PRIORITY_APPLICATION, PP_S7COMMPLUS, PROTO_BIT__TCP);
   _addPortsToStreamFilter(sc, s7commplus_policy, policy_id);
#ifdef TARGET_BASED
    _addServicesToStreamFilter(sc, policy_id);
#endif

    /* Add preprocessor rule options here */
    _dpd.preprocOptRegister(sc, S7COMMPLUS_OPCODE_NAME, S7commplusOpcodeInit, S7commplusRuleEval, free, NULL, NULL, NULL, NULL);
    _dpd.preprocOptRegister(sc, S7COMMPLUS_FUNC_NAME, S7commplusFuncInit, S7commplusRuleEval, free, NULL, NULL, NULL, NULL);
    _dpd.preprocOptRegister(sc, S7COMMPLUS_CONTENT_NAME, S7commplusContentInit, S7commplusRuleEval, free, NULL, NULL, NULL, NULL);
    
    return s7commplus_policy;
}

static void ParseSinglePort(s7commplus_config_t *config, char *token)
{
    /* single port number */
    char *endptr;
    unsigned long portnum = _dpd.SnortStrtoul(token, &endptr, 10);

    if ((*endptr != '\0') || (portnum >= MAX_PORTS))
    {
        _dpd.fatalMsg("%s(%d) Bad s7commplus port number: %s\n"
                      "Port number must be an integer between 0 and 65535.\n",
                      *_dpd.config_file, *_dpd.config_line, token);
    }

    /* Good port number! */
    config->ports[PORT_INDEX(portnum)] |= CONV_PORT(portnum);
}

static void ParseS7commplusArgs(s7commplus_config_t *config, char *args)
{
    char *saveptr;
    char *token;

    /* Set default port */
    config->ports[PORT_INDEX(S7COMMPLUS_PORT)] |= CONV_PORT(S7COMMPLUS_PORT);

    /* No args? Stick to the default. */
    if (args == NULL)
        return;

    token = strtok_r(args, " ", &saveptr);
    while (token != NULL)
    {
        if (strcmp(token, "ports") == 0)
        {
            unsigned nPorts = 0;

            /* Un-set the default port */
            config->ports[PORT_INDEX(S7COMMPLUS_PORT)] = 0;

            /* Parse ports */
            token = strtok_r(NULL, " ", &saveptr);

            if (token == NULL)
            {
                _dpd.fatalMsg("%s(%d) Missing argument for S7commplus preprocessor "
                              "'ports' option.\n", *_dpd.config_file, *_dpd.config_line);
            }

            if (isdigit(token[0]))
            {
                ParseSinglePort(config, token);
                nPorts++;
            }

            else if (*token == '{')
            {
                /* list of ports */
                token = strtok_r(NULL, " ", &saveptr);
                while (token != NULL && *token != '}')
                {
                    ParseSinglePort(config, token);
                    nPorts++;
                    token = strtok_r(NULL, " ", &saveptr);
                }
            }

            else
            {
                nPorts = 0;
            }
            if ( nPorts == 0 )
            {
                _dpd.fatalMsg("%s(%d) Bad S7commplus 'ports' argument: '%s'\n"
                              "Argument to S7commplus 'ports' must be an integer, or a list "
                              "enclosed in { } braces.\n", *_dpd.config_file, *_dpd.config_line, token);
            }
        }
        else
        {
            _dpd.fatalMsg("%s(%d) Failed to parse s7commplus argument: %s\n",
                          *_dpd.config_file, *_dpd.config_line, token);
        }

        token = strtok_r(NULL, " ", &saveptr);
    }

}

/* Print a S7commplus config */
static void S7commplusPrintConfig(s7commplus_config_t *config)
{
    int index;
    int newline = 1;

    if (config == NULL)
        return;

    _dpd.logMsg("S7commplus config: \n");
    _dpd.logMsg("    Ports:\n");

    /* Loop through port array & print, 5 ports per line */
    for (index = 0; index < MAX_PORTS; index++)
    {
        if (config->ports[PORT_INDEX(index)] & CONV_PORT(index))
        {
            _dpd.logMsg("\t%d", index);
            if ( !((newline++) % 5) )
            {
                _dpd.logMsg("\n");
            }
        }
    }
    _dpd.logMsg("\n");
}

/* Main runtime entry point */
static void ProcessS7commplus(void *ipacketp, void *contextp)
{
    SFSnortPacket *packetp = (SFSnortPacket *)ipacketp;
    s7commplus_session_data_t *sessp;
    PROFILE_VARS;

    // preconditions - what we registered for
    assert(IsTCP(packetp) && packetp->payload && packetp->payload_size);

    PREPROC_PROFILE_START(s7commplusPerfStats);

    /* Fetch me a preprocessor config to use with this VLAN/subnet/etc.! */
    s7commplus_eval_config = sfPolicyUserDataGetCurrent(s7commplus_context_id);

    /* Look for a previously-allocated session data. */
    sessp = _dpd.sessionAPI->get_application_data(packetp->stream_session, PP_S7COMMPLUS);

    if (sessp == NULL)
    {
        /* No existing session. Check those ports. */
        if (S7commplusPortCheck(s7commplus_eval_config, packetp) != true)
        {
            PREPROC_PROFILE_END(s7commplusPerfStats);
            return;
        }
    }

    if ( !PacketHasFullPDU(packetp) && S7commplusIsPafActive(packetp) )
    {
        /* If a packet is rebuilt, but not a full PDU, then it's garbage that
           got flushed at the end of a stream. */
        if ( packetp->flags & (FLAG_REBUILT_STREAM|FLAG_PDU_HEAD) )
        {
            _dpd.alertAdd(GENERATOR_SPP_S7COMMPLUS, S7COMMPLUS_BAD_LENGTH, 1, 0, 3,
                          S7COMMPLUS_BAD_LENGTH_STR, 0);
        }

        PREPROC_PROFILE_END(s7commplusPerfStats);
        return;
    }

    if (sessp == NULL)
    {
        /* Create session data and attach it to the Stream session */
        sessp = S7commplusCreateSessionData(packetp);

        if ( !sessp )
        {
            PREPROC_PROFILE_END(s7commplusPerfStats);
            return;
        }
    }

    /* When pipelined S7commplus PDUs appear in a single TCP segment, the
       detection engine caches the results of the rule options after
       evaluating on the first PDU. Setting this flag stops the caching. */
    packetp->flags |= FLAG_ALLOW_MULTIPLE_DETECT;

    /* Do preprocessor-specific detection stuff here */
    S7commplusDecode(s7commplus_eval_config, packetp);

    /* That's the end! */
    PREPROC_PROFILE_END(s7commplusPerfStats);
}

/* Check ports & services */
static int S7commplusPortCheck(s7commplus_config_t *config, SFSnortPacket *packet)
{
#ifdef TARGET_BASED
    int16_t app_id = _dpd.sessionAPI->get_application_protocol_id(packet->stream_session);

    /* call to get_application_protocol_id gave an error */
    if (app_id == SFTARGET_UNKNOWN_PROTOCOL)
        return false;

    /* this is identified as non-s7commplus */
    if (app_id && (app_id != s7commplus_app_id))
        return false;

    /* this is identified as s7commplus */
    if (app_id == s7commplus_app_id)
        return true;

    /* fall back to port check */
#endif

    if (config->ports[PORT_INDEX(packet->src_port)] & CONV_PORT(packet->src_port))
        return true;

    if (config->ports[PORT_INDEX(packet->dst_port)] & CONV_PORT(packet->dst_port))
        return true;

    return false;
}

static s7commplus_session_data_t* S7commplusCreateSessionData(SFSnortPacket *packet)
{
    s7commplus_session_data_t *data = NULL;

    /* Sanity Check */
    if (!packet || !packet->stream_session)
        return NULL;

    data = (s7commplus_session_data_t *)calloc(1, sizeof(s7commplus_session_data_t));

    if (!data)
        return NULL;

    /* Attach to Stream session */
    _dpd.sessionAPI->set_application_data(packet->stream_session, PP_S7COMMPLUS,
        data, FreeS7commplusData);

    /* This reference counting stuff got from old preprocs */
    data->policy_id = _dpd.getNapRuntimePolicy();
    data->context_id = s7commplus_context_id;
    ((s7commplus_config_t *)sfPolicyUserDataGetCurrent(s7commplus_context_id))->ref_count++;

    return data;
}

/* Reload functions */
#ifdef SNORT_RELOAD
/* Almost like S7commplusInit, but not quite. */
static void S7commplusReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyUserContextId s7commplus_swap_context_id = (tSfPolicyUserContextId)*new_config;
    s7commplus_config_t *s7commplus_policy = NULL;

    if (s7commplus_swap_context_id == NULL)
    {
        s7commplus_swap_context_id = sfPolicyConfigCreate();
        if (s7commplus_swap_context_id == NULL)
        {
            _dpd.fatalMsg("Failed to allocate memory "
                                            "for S7commplus config.\n");
        }

        if (_dpd.streamAPI == NULL)
        {
            _dpd.fatalMsg("SetupS7commplus(): The Stream preprocessor "
                                            "must be enabled.\n");
        }
        *new_config = (void *)s7commplus_swap_context_id;
    }

    s7commplus_policy = S7commplusPerPolicyInit(sc, s7commplus_swap_context_id);

    ParseS7commplusArgs(s7commplus_policy, args);

    /* Can't add ports until they've been parsed... */
    S7commplusAddPortsToPaf(sc, s7commplus_policy, _dpd.getParserPolicy(sc));

    S7commplusPrintConfig(s7commplus_policy);
}

static int S7commplusReloadVerify(struct _SnortConfig *sc, void *swap_config)
{
    if (!_dpd.isPreprocEnabled(sc, PP_STREAM))
    {
        _dpd.errMsg("SetupS7commplus(): The Stream preprocessor must be enabled.\n");
        return -1;
    }

    return 0;
}

static int S7commplusFreeUnusedConfigPolicy(
    tSfPolicyUserContextId context_id,
    tSfPolicyId policy_id,
    void *data
    )
{
    s7commplus_config_t *s7commplus_config = (s7commplus_config_t *)data;

    /* do any housekeeping before freeing s7commplus config */
    if (s7commplus_config->ref_count == 0)
    {
        sfPolicyUserDataClear(context_id, policy_id);
        free(s7commplus_config);
    }

    return 0;
}

static void * S7commplusReloadSwap(struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId s7commplus_swap_context_id = (tSfPolicyUserContextId)swap_config;
    tSfPolicyUserContextId old_context_id = s7commplus_context_id;

    if (s7commplus_swap_context_id == NULL)
        return NULL;

    s7commplus_context_id = s7commplus_swap_context_id;

    sfPolicyUserDataFreeIterate(old_context_id, S7commplusFreeUnusedConfigPolicy);

    if (sfPolicyUserPolicyGetActive(old_context_id) == 0)
    {
        /* No more outstanding configs - free the config array */
        return (void *)old_context_id;
    }

    return NULL;
}

static void S7commplusReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    S7commplusFreeConfig( (tSfPolicyUserContextId)data );
}
#endif   //Reload functions ends here

static void registerPortsForDispatch( struct _SnortConfig *sc, s7commplus_config_t *policy )
{
    uint32_t port;

    for ( port = 0; port < MAX_PORTS; port++ )
    {
        if( isPortEnabled( policy->ports, port ) )
            _dpd.sessionAPI->enable_preproc_for_port( sc, PP_S7COMMPLUS, PROTO_BIT__TCP, port ); 
    }
}

static void registerPortsForReassembly( s7commplus_config_t *policy, int direction )
{
    uint32_t port;

    for ( port = 0; port < MAX_PORTS; port++ )
    {
        if( isPortEnabled( policy->ports, port ) )
            _dpd.streamAPI->register_reassembly_port( NULL, port, direction );
    }
}

/* Stream filter functions */
static void _addPortsToStreamFilter(struct _SnortConfig *sc, s7commplus_config_t *config, tSfPolicyId policy_id)
{
    if (config == NULL)
        return;

    if (_dpd.streamAPI)
    {
        int portNum;

        for (portNum = 0; portNum < MAX_PORTS; portNum++)
        {
            if(config->ports[(portNum/8)] & (1<<(portNum%8)))
            {
                //Add port the port
                _dpd.streamAPI->set_port_filter_status( sc, IPPROTO_TCP, (uint16_t)portNum,
                                                        PORT_MONITOR_SESSION, policy_id, 1 );
            }
        }
    }

}

#ifdef TARGET_BASED
static void _addServicesToStreamFilter(struct _SnortConfig *sc, tSfPolicyId policy_id)
{
    _dpd.streamAPI->set_service_filter_status(sc, s7commplus_app_id, PORT_MONITOR_SESSION, policy_id, 1);
}
#endif

static int S7commplusFreeConfigPolicy(
    tSfPolicyUserContextId context_id,
    tSfPolicyId policy_id,
    void *data
    )
{
    s7commplus_config_t *s7commplus_config = (s7commplus_config_t *)data;

    /* do any housekeeping before freeing s7commplus_config */

    sfPolicyUserDataClear(context_id, policy_id);
    free(s7commplus_config);
    return 0;
}

static void S7commplusFreeConfig(tSfPolicyUserContextId context_id)
{
    if (context_id == NULL)
        return;

    sfPolicyUserDataFreeIterate(context_id, S7commplusFreeConfigPolicy);
    sfPolicyConfigDelete(context_id);
}

static int S7commplusCheckPolicyConfig(
    struct _SnortConfig *sc,
    tSfPolicyUserContextId context_id,
    tSfPolicyId policy_id,
    void *data
    )
{
    _dpd.setParserPolicy(sc, policy_id);

    if (!_dpd.isPreprocEnabled(sc, PP_STREAM))
    {
        _dpd.errMsg("%s(%d) S7commplusCheckPolicyConfig(): The Stream preprocessor "
                      "must be enabled.\n", *_dpd.config_file, *_dpd.config_line);
        return -1;
    }
    return 0;
}

static int S7commplusCheckConfig(struct _SnortConfig *sc)
{
    int rval;

    if ((rval = sfPolicyUserDataIterate(sc, s7commplus_context_id, S7commplusCheckPolicyConfig)))
        return rval;

    return 0;
}

static void S7commplusCleanExit(int signal, void *data)
{
    if (s7commplus_context_id != NULL)
    {
        S7commplusFreeConfig(s7commplus_context_id);
        s7commplus_context_id = NULL;
    }
}

static void FreeS7commplusData(void *data)
{
    s7commplus_session_data_t *session = (s7commplus_session_data_t *)data;
    s7commplus_config_t *config = NULL;

    if (session == NULL)
        return;

    if (session->context_id != NULL)
    {
        config = (s7commplus_config_t *)sfPolicyUserDataGet(session->context_id, session->policy_id);
    }

    if (config != NULL)
    {
        config->ref_count--;
        if ((config->ref_count == 0) &&
            (session->context_id != s7commplus_context_id))
        {
            sfPolicyUserDataClear(session->context_id, session->policy_id);
            free(config);

            if (sfPolicyUserPolicyGetActive(session->context_id) == 0)
            {
                /* No more outstanding configs - free the config array */
                S7commplusFreeConfig(session->context_id);
            }
        }
    }
    free(session);
}
