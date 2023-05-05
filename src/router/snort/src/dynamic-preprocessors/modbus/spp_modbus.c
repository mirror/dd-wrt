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
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
 *
 * Author: Ryan Jordan
 *
 * Dynamic preprocessor for the Modbus protocol
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
#include "spp_modbus.h"
#include "sf_preproc_info.h"

#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats modbusPerfStats;
#endif

#include "sf_types.h"

#include "modbus_decode.h"
#include "modbus_roptions.h"
#include "modbus_paf.h"

#ifdef DUMP_BUFFER
#include "modbus_buffer_dump.h"
#endif
const int MAJOR_VERSION = 1;
const int MINOR_VERSION = 1;
const int BUILD_VERSION = 1;
const char *PREPROC_NAME = "SF_MODBUS";

#define SetupModbus DYNAMIC_PREPROC_SETUP

/* Preprocessor config objects */
static tSfPolicyUserContextId modbus_context_id = NULL;
static modbus_config_t *modbus_eval_config = NULL;


/* Target-based app ID */
#ifdef TARGET_BASED
int16_t modbus_app_id = SFTARGET_UNKNOWN_PROTOCOL;
#endif

/* Prototypes */
static void ModbusInit(struct _SnortConfig *, char *);
static inline void ModbusOneTimeInit(struct _SnortConfig *);
static inline modbus_config_t * ModbusPerPolicyInit(struct _SnortConfig *, tSfPolicyUserContextId);

static void ProcessModbus(void *, void *);

#ifdef SNORT_RELOAD
static void ModbusReload(struct _SnortConfig *, char *, void **);
static int ModbusReloadVerify(struct _SnortConfig *, void *);
static void * ModbusReloadSwap(struct _SnortConfig *, void *);
static void ModbusReloadSwapFree(void *);
#endif

static void registerPortsForDispatch( struct _SnortConfig *sc, modbus_config_t *policy );
static void registerPortsForReassembly( modbus_config_t *policy, int direction );
static void _addPortsToStreamFilter(struct _SnortConfig *, modbus_config_t *, tSfPolicyId);
#ifdef TARGET_BASED
static void _addServicesToStreamFilter(struct _SnortConfig *, tSfPolicyId);
#endif

static void ModbusFreeConfig(tSfPolicyUserContextId context_id);
static void FreeModbusData(void *);
static int ModbusCheckConfig(struct _SnortConfig *);
static void ModbusCleanExit(int, void *);

static void ParseModbusArgs(modbus_config_t *config, char *args);
static void ModbusPrintConfig(modbus_config_t *config);

static int ModbusPortCheck(modbus_config_t *config, SFSnortPacket *packet);
static modbus_session_data_t * ModbusCreateSessionData(SFSnortPacket *);

/* Register init callback */
void SetupModbus(void)
{
#ifndef SNORT_RELOAD
    _dpd.registerPreproc("modbus", ModbusInit);
#else
    _dpd.registerPreproc("modbus", ModbusInit, ModbusReload,
                         ModbusReloadVerify, ModbusReloadSwap,
                         ModbusReloadSwapFree);
#endif
#ifdef DUMP_BUFFER
    _dpd.registerBufferTracer(getMODBUSBuffers, MODBUS_BUFFER_DUMP_FUNC);
#endif
}

#ifdef REG_TEST
static inline void PrintMODBUSSize(void)
{
    _dpd.logMsg("\nMODBUS Session Size: %lu\n", (long unsigned int)sizeof(modbus_session_data_t));
}
#endif

/* Allocate memory for preprocessor config, parse the args, set up callbacks */
static void ModbusInit(struct _SnortConfig *sc, char *argp)
{
    modbus_config_t *modbus_policy = NULL;

#ifdef REG_TEST
    PrintMODBUSSize();
#endif

    if (modbus_context_id == NULL)
    {
        ModbusOneTimeInit(sc);
    }

    modbus_policy = ModbusPerPolicyInit(sc, modbus_context_id);

    ParseModbusArgs(modbus_policy, argp);

    /* Can't add ports until they've been parsed... */
    ModbusAddPortsToPaf(sc, modbus_policy, _dpd.getParserPolicy(sc));
#ifdef TARGET_BASED
    ModbusAddServiceToPaf(sc, modbus_app_id, _dpd.getParserPolicy(sc));
#endif
    // register ports with session and stream
    registerPortsForDispatch( sc, modbus_policy );
    registerPortsForReassembly( modbus_policy, SSN_DIR_FROM_SERVER | SSN_DIR_FROM_CLIENT );

    ModbusPrintConfig(modbus_policy);
#ifdef DUMP_BUFFER
        dumpBufferInit();
#endif
}

static inline void ModbusOneTimeInit(struct _SnortConfig *sc)
{
    /* context creation & error checking */
    modbus_context_id = sfPolicyConfigCreate();
    if (modbus_context_id == NULL)
    {
        _dpd.fatalMsg("%s(%d) Failed to allocate memory for "
                      "Modbus config.\n", *_dpd.config_file, *_dpd.config_line);
    }

    if (_dpd.streamAPI == NULL)
    {
        _dpd.fatalMsg("%s(%d) SetupModbus(): The Stream preprocessor "
                      "must be enabled.\n", *_dpd.config_file, *_dpd.config_line);
    }

    /* callback registration */
    _dpd.addPreprocConfCheck(sc, ModbusCheckConfig);
    _dpd.addPreprocExit(ModbusCleanExit, NULL, PRIORITY_LAST, PP_MODBUS);

#ifdef PERF_PROFILING
    _dpd.addPreprocProfileFunc("modbus", (void *)&modbusPerfStats, 0, _dpd.totalPerfStats, NULL);
#endif

    /* Set up target-based app id */
#ifdef TARGET_BASED
    modbus_app_id = _dpd.findProtocolReference("modbus");
    if (modbus_app_id == SFTARGET_UNKNOWN_PROTOCOL)
        modbus_app_id = _dpd.addProtocolReference("modbus");

    // register with session to handle applications
    _dpd.sessionAPI->register_service_handler( PP_MODBUS, modbus_app_id );

#endif
}

/* Responsible for allocating a Modbus policy. Never returns NULL. */
static inline modbus_config_t * ModbusPerPolicyInit(struct _SnortConfig *sc, tSfPolicyUserContextId context_id)
{
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    modbus_config_t *modbus_policy = NULL;

    /* Check for existing policy & bail if found */
    sfPolicyUserPolicySet(context_id, policy_id);
    modbus_policy = (modbus_config_t *)sfPolicyUserDataGetCurrent(context_id);
    if (modbus_policy != NULL)
    {
        _dpd.fatalMsg("%s(%d) Modbus preprocessor can only be "
                      "configured once.\n", *_dpd.config_file, *_dpd.config_line);
    }

    /* Allocate new policy */
    modbus_policy = (modbus_config_t *)calloc(1, sizeof(modbus_config_t));
    if (!modbus_policy)
    {
        _dpd.fatalMsg("%s(%d) Could not allocate memory for "
                      "modbus preprocessor configuration.\n"
                      , *_dpd.config_file, *_dpd.config_line);
    }

    sfPolicyUserDataSetCurrent(context_id, modbus_policy);

    /* Register callbacks that are done for each policy */
    _dpd.addPreproc(sc, ProcessModbus, PRIORITY_APPLICATION, PP_MODBUS, PROTO_BIT__TCP);
   _addPortsToStreamFilter(sc, modbus_policy, policy_id);
#ifdef TARGET_BASED
    _addServicesToStreamFilter(sc, policy_id);
#endif

    /* Add preprocessor rule options here */
    /* _dpd.preprocOptRegister("foo_bar", FOO_init, FOO_rule_eval, free, NULL, NULL, NULL, NULL); */
    _dpd.preprocOptRegister(sc, "modbus_func", ModbusFuncInit, ModbusRuleEval, free, NULL, NULL, NULL, NULL);
    _dpd.preprocOptRegister(sc, "modbus_unit", ModbusUnitInit, ModbusRuleEval, free, NULL, NULL, NULL, NULL);
    _dpd.preprocOptRegister(sc, "modbus_data", ModbusDataInit, ModbusRuleEval, free, NULL, NULL, NULL, NULL);

    return modbus_policy;
}

static void ParseSinglePort(modbus_config_t *config, char *token)
{
    /* single port number */
    char *endptr;
    unsigned long portnum = _dpd.SnortStrtoul(token, &endptr, 10);

    if ((*endptr != '\0') || (portnum >= MAX_PORTS))
    {
        _dpd.fatalMsg("%s(%d) Bad modbus port number: %s\n"
                      "Port number must be an integer between 0 and 65535.\n",
                      *_dpd.config_file, *_dpd.config_line, token);
    }

    /* Good port number! */
    config->ports[PORT_INDEX(portnum)] |= CONV_PORT(portnum);
}

static void ParseModbusArgs(modbus_config_t *config, char *args)
{
    char *saveptr;
    char *token;

    /* Set default port */
    config->ports[PORT_INDEX(MODBUS_PORT)] |= CONV_PORT(MODBUS_PORT);

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
            config->ports[PORT_INDEX(MODBUS_PORT)] = 0;

            /* Parse ports */
            token = strtok_r(NULL, " ", &saveptr);

            if (token == NULL)
            {
                _dpd.fatalMsg("%s(%d) Missing argument for Modbus preprocessor "
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
                _dpd.fatalMsg("%s(%d) Bad Modbus 'ports' argument: '%s'\n"
                              "Argument to Modbus 'ports' must be an integer, or a list "
                              "enclosed in { } braces.\n", *_dpd.config_file, *_dpd.config_line, token);
            }
        }
        else
        {
            _dpd.fatalMsg("%s(%d) Failed to parse modbus argument: %s\n",
                          *_dpd.config_file, *_dpd.config_line, token);
        }

        token = strtok_r(NULL, " ", &saveptr);
    }

}

/* Print a Modbus config */
static void ModbusPrintConfig(modbus_config_t *config)
{
    int index;
    int newline = 1;

    if (config == NULL)
        return;

    _dpd.logMsg("Modbus config: \n");
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
static void ProcessModbus(void *ipacketp, void *contextp)
{
    SFSnortPacket *packetp = (SFSnortPacket *)ipacketp;
    modbus_session_data_t *sessp;
    PROFILE_VARS;
    // preconditions - what we registered for
    assert(IsTCP(packetp) && packetp->payload && packetp->payload_size);

    PREPROC_PROFILE_START(modbusPerfStats);

    /* Fetch me a preprocessor config to use with this VLAN/subnet/etc.! */
    modbus_eval_config = sfPolicyUserDataGetCurrent(modbus_context_id);

    /* Look for a previously-allocated session data. */
    sessp = _dpd.sessionAPI->get_application_data(packetp->stream_session, PP_MODBUS);

    if (sessp == NULL)
    {
        /* No existing session. Check those ports. */
        if (ModbusPortCheck(modbus_eval_config, packetp) != MODBUS_OK)
        {
            PREPROC_PROFILE_END(modbusPerfStats);
            return;
        }
    }

    if ( !PacketHasFullPDU(packetp) && ModbusIsPafActive(packetp) )
    {
        if ( sessp )
        {
            sessp->unit = 0;
            sessp->func = 0;
        }
        /* If a packet is rebuilt, but not a full PDU, then it's garbage that
           got flushed at the end of a stream. */
        if ( packetp->flags & (FLAG_REBUILT_STREAM|FLAG_PDU_HEAD) )
        {
            _dpd.alertAdd(GENERATOR_SPP_MODBUS, MODBUS_BAD_LENGTH, 1, 0, 3,
                          MODBUS_BAD_LENGTH_STR, 0);
        }

        PREPROC_PROFILE_END(modbusPerfStats);
        return;
    }

    if (sessp == NULL)
    {
        /* Create session data and attach it to the Stream session */
        sessp = ModbusCreateSessionData(packetp);

        if ( !sessp )
        {
            PREPROC_PROFILE_END(modbusPerfStats);
            return;
        }
    }

    /* When pipelined Modbus PDUs appear in a single TCP segment, the
       detection engine caches the results of the rule options after
       evaluating on the first PDU. Setting this flag stops the caching. */
    packetp->flags |= FLAG_ALLOW_MULTIPLE_DETECT;

    /* Do preprocessor-specific detection stuff here */
    if (ModbusDecode(modbus_eval_config, packetp) == MODBUS_FAIL)
    {
        sessp->unit = 0;
        sessp->func = 0;
    }

    /* That's the end! */
    PREPROC_PROFILE_END(modbusPerfStats);
}

/* Check ports & services */
static int ModbusPortCheck(modbus_config_t *config, SFSnortPacket *packet)
{
#ifdef TARGET_BASED
    int16_t app_id = _dpd.sessionAPI->get_application_protocol_id(packet->stream_session);

    /* call to get_application_protocol_id gave an error */
    if (app_id == SFTARGET_UNKNOWN_PROTOCOL)
        return MODBUS_FAIL;

    /* this is positively identified as something non-modbus */
    if (app_id && (app_id != modbus_app_id))
        return MODBUS_FAIL;

    /* this is identified as modbus */
    if (app_id == modbus_app_id)
        return MODBUS_OK;

    /* fall back to port check */
#endif

    if (config->ports[PORT_INDEX(packet->src_port)] & CONV_PORT(packet->src_port))
        return MODBUS_OK;

    if (config->ports[PORT_INDEX(packet->dst_port)] & CONV_PORT(packet->dst_port))
        return MODBUS_OK;

    return MODBUS_FAIL;
}

static modbus_session_data_t * ModbusCreateSessionData(SFSnortPacket *packet)
{
    modbus_session_data_t *data = NULL;

    /* Sanity Check */
    if (!packet || !packet->stream_session)
        return NULL;

    data = (modbus_session_data_t *)calloc(1, sizeof(modbus_session_data_t));

    if (!data)
        return NULL;

    /* Attach to Stream session */
    _dpd.sessionAPI->set_application_data(packet->stream_session, PP_MODBUS,
        data, FreeModbusData);

    /* Not sure when this reference counting stuff got added to the old preprocs */
    data->policy_id = _dpd.getNapRuntimePolicy();
    data->context_id = modbus_context_id;
    ((modbus_config_t *)sfPolicyUserDataGetCurrent(modbus_context_id))->ref_count++;

    return data;
}


/* Reload functions */
#ifdef SNORT_RELOAD
/* Almost like ModbusInit, but not quite. */
static void ModbusReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyUserContextId modbus_swap_context_id = (tSfPolicyUserContextId)*new_config;
    modbus_config_t *modbus_policy = NULL;

    if (modbus_swap_context_id == NULL)
    {
        modbus_swap_context_id = sfPolicyConfigCreate();
        if (modbus_swap_context_id == NULL)
        {
            _dpd.fatalMsg("Failed to allocate memory "
                                            "for Modbus config.\n");
        }

        if (_dpd.streamAPI == NULL)
        {
            _dpd.fatalMsg("SetupModbus(): The Stream preprocessor "
                                            "must be enabled.\n");
        }
        *new_config = (void *)modbus_swap_context_id;
    }

    modbus_policy = ModbusPerPolicyInit(sc, modbus_swap_context_id);

    ParseModbusArgs(modbus_policy, args);

    /* Can't add ports until they've been parsed... */
    ModbusAddPortsToPaf(sc, modbus_policy, _dpd.getParserPolicy(sc));

    ModbusPrintConfig(modbus_policy);
}

static int ModbusReloadVerify(struct _SnortConfig *sc, void *swap_config)
{
    if (!_dpd.isPreprocEnabled(sc, PP_STREAM))
    {
        _dpd.errMsg("SetupModbus(): The Stream preprocessor must be enabled.\n");
        return -1;
    }

    return 0;
}

static int ModbusFreeUnusedConfigPolicy(
    tSfPolicyUserContextId context_id,
    tSfPolicyId policy_id,
    void *data
    )
{
    modbus_config_t *modbus_config = (modbus_config_t *)data;

    /* do any housekeeping before freeing modbus config */
    if (modbus_config->ref_count == 0)
    {
        sfPolicyUserDataClear(context_id, policy_id);
        free(modbus_config);
    }

    return 0;
}

static void * ModbusReloadSwap(struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId modbus_swap_context_id = (tSfPolicyUserContextId)swap_config;
    tSfPolicyUserContextId old_context_id = modbus_context_id;

    if (modbus_swap_context_id == NULL)
        return NULL;

    modbus_context_id = modbus_swap_context_id;

    sfPolicyUserDataFreeIterate(old_context_id, ModbusFreeUnusedConfigPolicy);

    if (sfPolicyUserPolicyGetActive(old_context_id) == 0)
    {
        /* No more outstanding configs - free the config array */
        return (void *)old_context_id;
    }

    return NULL;
}

static void ModbusReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    ModbusFreeConfig( (tSfPolicyUserContextId)data );
}
#endif

static void registerPortsForDispatch( struct _SnortConfig *sc, modbus_config_t *policy )
{
    uint32_t port;

    for ( port = 0; port < MAX_PORTS; port++ )
    {
        if( isPortEnabled( policy->ports, port ) )
            _dpd.sessionAPI->enable_preproc_for_port( sc, PP_MODBUS, PROTO_BIT__TCP, port ); 
    }
}

static void registerPortsForReassembly( modbus_config_t *policy, int direction )
{
    uint32_t port;

    for ( port = 0; port < MAX_PORTS; port++ )
    {
        if( isPortEnabled( policy->ports, port ) )
            _dpd.streamAPI->register_reassembly_port( NULL, port, direction );
    }
}

/* Stream filter functions */
static void _addPortsToStreamFilter(struct _SnortConfig *sc, modbus_config_t *config, tSfPolicyId policy_id)
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
    _dpd.streamAPI->set_service_filter_status(sc, modbus_app_id, PORT_MONITOR_SESSION, policy_id, 1);
}
#endif

static int ModbusFreeConfigPolicy(
    tSfPolicyUserContextId context_id,
    tSfPolicyId policy_id,
    void *data
    )
{
    modbus_config_t *modbus_config = (modbus_config_t *)data;

    /* do any housekeeping before freeing modbus_config */

    sfPolicyUserDataClear(context_id, policy_id);
    free(modbus_config);
    return 0;
}

static void ModbusFreeConfig(tSfPolicyUserContextId context_id)
{
    if (context_id == NULL)
        return;

    sfPolicyUserDataFreeIterate(context_id, ModbusFreeConfigPolicy);
    sfPolicyConfigDelete(context_id);
}

static int ModbusCheckPolicyConfig(
    struct _SnortConfig *sc,
    tSfPolicyUserContextId context_id,
    tSfPolicyId policy_id,
    void *data
    )
{
    _dpd.setParserPolicy(sc, policy_id);

    if (!_dpd.isPreprocEnabled(sc, PP_STREAM))
    {
        _dpd.errMsg("%s(%d) ModbusCheckPolicyConfig(): The Stream preprocessor "
                      "must be enabled.\n", *_dpd.config_file, *_dpd.config_line);
        return -1;
    }
    return 0;
}

static int ModbusCheckConfig(struct _SnortConfig *sc)
{
    int rval;

    if ((rval = sfPolicyUserDataIterate(sc, modbus_context_id, ModbusCheckPolicyConfig)))
        return rval;

    return 0;
}

static void ModbusCleanExit(int signal, void *data)
{
    if (modbus_context_id != NULL)
    {
        ModbusFreeConfig(modbus_context_id);
        modbus_context_id = NULL;
    }
}

static void FreeModbusData(void *data)
{
    modbus_session_data_t *session = (modbus_session_data_t *)data;
    modbus_config_t *config = NULL;

    if (session == NULL)
        return;

    if (session->context_id != NULL)
    {
        config = (modbus_config_t *)sfPolicyUserDataGet(session->context_id, session->policy_id);
    }

    if (config != NULL)
    {
        config->ref_count--;
        if ((config->ref_count == 0) &&
            (session->context_id != modbus_context_id))
        {
            sfPolicyUserDataClear(session->context_id, session->policy_id);
            free(config);

            if (sfPolicyUserPolicyGetActive(session->context_id) == 0)
            {
                /* No more outstanding configs - free the config array */
                ModbusFreeConfig(session->context_id);
            }
        }
    }
    free(session);
}
