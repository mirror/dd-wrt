/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
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

/**************************************************************************
 * snort_pop.c
 *
 * Author: Bhagyashree Bantwal <bbantwal@cisco.com>
 *
 * Description:
 *
 * This file handles POP protocol checking and normalization.
 *
 * Entry point functions:
 *
 *     SnortPOP()
 *     POP_Init()
 *     POP_Free()
 *
 **************************************************************************/


/* Includes ***************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "sf_types.h"
#include "snort_pop.h"
#include "pop_config.h"
#include "pop_util.h"
#include "pop_log.h"

#include "sf_snort_packet.h"
#include "stream_api.h"
#include "snort_debug.h"
#include "profiler.h"
#include "snort_bounds.h"
#include "sf_dynamic_preprocessor.h"
#include "ssl.h"
#include "ssl_include.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"
#include "file_api.h"
#ifdef DEBUG_MSGS
#include "sf_types.h"
#endif
#include "pop_paf.h"

#ifdef DUMP_BUFFER
#include "pop_buffer_dump.h"
#endif

/**************************************************************************/


/* Externs ****************************************************************/

#ifdef PERF_PROFILING
extern PreprocStats popDetectPerfStats;
extern int popDetectCalled;
#endif

extern tSfPolicyUserContextId pop_config;
extern POPConfig *pop_eval_config;
extern MemPool *pop_mime_mempool;
extern MemPool *pop_mempool;

#ifdef DEBUG_MSGS
extern char pop_print_buffer[];
#endif

/**************************************************************************/


/* Globals ****************************************************************/

const POPToken pop_known_cmds[] =
{
    {"APOP",          4, CMD_APOP},
    {"AUTH",          4, CMD_AUTH},
    {"CAPA",          4, CMD_CAPA},
    {"DELE",          4, CMD_DELE},
    {"LIST",          4, CMD_LIST},
    {"NOOP",          4, CMD_NOOP},
    {"PASS",          4, CMD_PASS},
    {"QUIT",          4, CMD_QUIT},
    {"RETR",          4, CMD_RETR},
    {"RSET",          4, CMD_RSET},
    {"STAT",          4, CMD_STAT},
    {"STLS",          4, CMD_STLS},
    {"TOP",           3, CMD_TOP},
    {"UIDL",          4, CMD_UIDL},
    {"USER",          4, CMD_USER},
    {NULL,            0, 0}
};

const POPToken pop_resps[] =
{
	{"+OK",   3,  RESP_OK},   /* SUCCESS */
	{"-ERR",  4,  RESP_ERR},  /* FAILURE */
	{NULL,   0,  0}
};

POP *pop_ssn = NULL;
POP pop_no_session;
POPSearchInfo pop_search_info;

#ifdef DEBUG_MSGS
uint64_t pop_session_counter = 0;
#endif

#ifdef TARGET_BASED
int16_t pop_proto_id;
#endif

void *pop_resp_search_mpse = NULL;
POPSearch pop_resp_search[RESP_LAST];

POPSearch *pop_current_search = NULL;


/**************************************************************************/


/* Private functions ******************************************************/

static int POP_Setup(SFSnortPacket *p, POP *ssn);
static void POP_ResetState(void);
static void POP_SessionFree(void *);
static int POP_GetPacketDirection(SFSnortPacket *, int);
static void POP_ProcessClientPacket(SFSnortPacket *);
static void POP_ProcessServerPacket(SFSnortPacket *);
static void POP_DisableDetect(SFSnortPacket *);
static const uint8_t * POP_HandleCommand(SFSnortPacket *, const uint8_t *, const uint8_t *);
static int POP_SearchStrFound(void *, void *, int, void *, void *);

static int POP_Inspect(SFSnortPacket *);
void POP_Set_flow_id( void *app_data, uint32_t fid );
MimeMethods mime_methods = {NULL, NULL, POP_DecodeAlert, POP_ResetState, is_data_end};

/**************************************************************************/

void POP_InitCmds(POPConfig *config)
{
    const POPToken *tmp;

    if (config == NULL)
        return;

    /* add one to CMD_LAST for NULL entry */
    config->cmds = (POPToken *)_dpd.snortAlloc(CMD_LAST + 1, sizeof(POPToken), PP_POP, 
                                    PP_MEM_CATEGORY_CONFIG);
    if (config->cmds == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => failed to allocate memory for pop "
                                        "command structure\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
    }

    for (tmp = &pop_known_cmds[0]; tmp->name != NULL; tmp++)
    {
        config->cmds[tmp->search_id].name_len = tmp->name_len;
        config->cmds[tmp->search_id].search_id = tmp->search_id;
        config->cmds[tmp->search_id].name = strdup(tmp->name);

        if (config->cmds[tmp->search_id].name == NULL)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => failed to allocate memory for pop "
                                            "command structure\n",
                                            *(_dpd.config_file), *(_dpd.config_line));
        }
    }

    /* initialize memory for command searches */
    config->cmd_search = (POPSearch *)_dpd.snortAlloc(CMD_LAST, sizeof(POPSearch), PP_POP,
                                           PP_MEM_CATEGORY_CONFIG);
    
    if (config->cmd_search == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => failed to allocate memory for pop "
                                        "command structure\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
    }

    config->num_cmds = CMD_LAST;
}


/*
 * Initialize POP searches
 *
 * @param  none
 *
 * @return none
 */
void POP_SearchInit(void)
{
    const POPToken *tmp;

    /* Response search */
    pop_resp_search_mpse = _dpd.searchAPI->search_instance_new();
    if (pop_resp_search_mpse == NULL)
    {
        DynamicPreprocessorFatalMessage("Could not allocate POP "
                                        "response search.\n");
    }

    for (tmp = &pop_resps[0]; tmp->name != NULL; tmp++)
    {
        pop_resp_search[tmp->search_id].name = tmp->name;
        pop_resp_search[tmp->search_id].name_len = tmp->name_len;

        _dpd.searchAPI->search_instance_add(pop_resp_search_mpse, tmp->name,
                                            tmp->name_len, tmp->search_id);
    }

    _dpd.searchAPI->search_instance_prep(pop_resp_search_mpse);

}


/*
 * Reset POP session state
 *
 * @param  none
 *
 * @return none
 */
static void POP_ResetState(void)
{
    pop_ssn->state = STATE_COMMAND;
    pop_ssn->prev_response = 0;
    pop_ssn->state_flags = 0;
}


/*
 * Given a server configuration and a port number, we decide if the port is
 *  in the POP server port list.
 *
 *  @param  port       the port number to compare with the configuration
 *
 *  @return integer
 *  @retval  0 means that the port is not a server port
 *  @retval !0 means that the port is a server port
 */
int POP_IsServer(uint16_t port)
{
    if( isPortEnabled( pop_eval_config->ports, port ) )
        return 1;

    return 0;
}

static POP * POP_GetNewSession(SFSnortPacket *p, tSfPolicyId policy_id)
{
    POP *ssn;
    POPConfig *pPolicyConfig = NULL;
    int ret = 0;

    pPolicyConfig = (POPConfig *)sfPolicyUserDataGetCurrent(pop_config);

    DEBUG_WRAP(DebugMessage(DEBUG_POP, "Creating new session data structure\n"););

    ssn = (POP *)_dpd.snortAlloc(1, sizeof(POP), PP_POP,
                      PP_MEM_CATEGORY_SESSION);
    if (ssn == NULL)
    {
        DynamicPreprocessorFatalMessage("Failed to allocate POP session data\n");
    }

    pop_ssn = ssn;
    ssn->prev_response = 0;
    ssn->session_flags |= POP_FLAG_CHECK_SSL;

    pop_ssn->mime_ssn.log_config = &(pop_eval_config->log_config);
    pop_ssn->mime_ssn.decode_conf = &(pop_eval_config->decode_conf);
    pop_ssn->mime_ssn.mime_mempool = pop_mime_mempool;
    pop_ssn->mime_ssn.log_mempool = pop_mempool;
    pop_ssn->mime_ssn.mime_stats = &(pop_stats.mime_stats);
    pop_ssn->mime_ssn.methods = &(mime_methods);

    if (( ret = _dpd.fileAPI->set_log_buffers(&(pop_ssn->mime_ssn.log_state), &(pPolicyConfig->log_config),pop_mempool, p->stream_session, PP_POP)) < 0)
    {
        if( ret == -1 )
        {
            if(pop_stats.log_memcap_exceeded % 10000 == 0)
            {
                _dpd.logMsg("WARNING: POP  memcap exceeded.\n");
            }
            pop_stats.log_memcap_exceeded++;
        }
        _dpd.snortFree(ssn, sizeof(*ssn), PP_POP,
             PP_MEM_CATEGORY_SESSION);
        return NULL;
    }

    _dpd.sessionAPI->set_application_data(p->stream_session, PP_POP,
                                         ssn, &POP_SessionFree);

    if (p->flags & SSNFLAG_MIDSTREAM)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_POP, "Got midstream packet - "
                                "setting state to unknown\n"););
        ssn->state = STATE_UNKNOWN;
    }

#ifdef DEBUG_MSGS
    pop_session_counter++;
    ssn->session_number = pop_session_counter;
#endif

    if (p->stream_session != NULL)
    {
        /* check to see if we're doing client reassembly in stream */
        if (_dpd.streamAPI->get_reassembly_direction(p->stream_session) & SSN_DIR_TO_CLIENT)
            ssn->reassembling = 1;

        if(!ssn->reassembling)
        {
            _dpd.streamAPI->set_reassembly(p->stream_session,
                    STREAM_FLPOLICY_FOOTPRINT, SSN_DIR_TO_CLIENT, STREAM_FLPOLICY_SET_ABSOLUTE);
            ssn->reassembling = 1;
        }
    }

    ssn->policy_id = policy_id;
    ssn->config = pop_config;
    ssn->flow_id = 0;
    pPolicyConfig->ref_count++;
    pop_stats.sessions++;
    pop_stats.conc_sessions++;
    pop_stats.cur_sessions++;
    if(pop_stats.max_conc_sessions < pop_stats.conc_sessions)
       pop_stats.max_conc_sessions = pop_stats.conc_sessions;

    return ssn;
}


/*
 * Do first-packet setup
 *
 * @param   p   standard Packet structure
 *
 * @return  none
 */
static int POP_Setup(SFSnortPacket *p, POP *ssn)
{
    int flags = 0;
    int pkt_dir;

    if (p->stream_session != NULL)
    {
        /* set flags to session flags */
        flags = _dpd.sessionAPI->get_session_flags(p->stream_session);
    }

    /* Figure out direction of packet */
    pkt_dir = POP_GetPacketDirection(p, flags);

    DEBUG_WRAP(DebugMessage(DEBUG_POP, "Session number: "STDu64"\n", ssn->session_number););

    if (!(ssn->session_flags & POP_FLAG_CHECK_SSL))
            ssn->session_flags |= POP_FLAG_CHECK_SSL;
    /* Check to see if there is a reassembly gap.  If so, we won't know
     * what state we're in when we get the _next_ reassembled packet */
    if ((pkt_dir != POP_PKT_FROM_SERVER) &&
        (p->flags & FLAG_REBUILT_STREAM))
    {
        int missing_in_rebuilt =
            _dpd.streamAPI->missing_in_reassembled(p->stream_session, SSN_DIR_TO_CLIENT);

        if (ssn->session_flags & POP_FLAG_NEXT_STATE_UNKNOWN)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_POP, "Found gap in previous reassembly buffer - "
                                    "set state to unknown\n"););
            ssn->state = STATE_UNKNOWN;
            ssn->session_flags &= ~POP_FLAG_NEXT_STATE_UNKNOWN;
        }

        if (missing_in_rebuilt == SSN_MISSING_BEFORE)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_POP, "Found missing packets before "
                                    "in reassembly buffer - set state to unknown\n"););
            ssn->state = STATE_UNKNOWN;
        }
    }

    return pkt_dir;
}

/*
 * Determine packet direction
 *
 * @param   p   standard Packet structure
 *
 * @return  none
 */
static int POP_GetPacketDirection(SFSnortPacket *p, int flags)
{
    int pkt_direction = POP_PKT_FROM_UNKNOWN;

    if (flags & SSNFLAG_MIDSTREAM)
    {
        if (POP_IsServer(p->src_port) &&
            !POP_IsServer(p->dst_port))
        {
            pkt_direction = POP_PKT_FROM_SERVER;
        }
        else if (!POP_IsServer(p->src_port) &&
                 POP_IsServer(p->dst_port))
        {
            pkt_direction = POP_PKT_FROM_CLIENT;
        }
    }
    else
    {
        if (p->flags & FLAG_FROM_SERVER)
        {
            pkt_direction = POP_PKT_FROM_SERVER;
        }
        else if (p->flags & FLAG_FROM_CLIENT)
        {
            pkt_direction = POP_PKT_FROM_CLIENT;
        }

        /* if direction is still unknown ... */
        if (pkt_direction == POP_PKT_FROM_UNKNOWN)
        {
            if (POP_IsServer(p->src_port) &&
                !POP_IsServer(p->dst_port))
            {
                pkt_direction = POP_PKT_FROM_SERVER;
            }
            else if (!POP_IsServer(p->src_port) &&
                     POP_IsServer(p->dst_port))
            {
                pkt_direction = POP_PKT_FROM_CLIENT;
            }
        }
    }

    return pkt_direction;
}


/*
 * Free POP-specific related to this session
 *
 * @param   v   pointer to POP session structure
 *
 *
 * @return  none
 */
static void POP_SessionFree(void *session_data)
{
    POP *pop = (POP *)session_data;
#ifdef SNORT_RELOAD
    POPConfig *pPolicyConfig = NULL;
#endif
    ssl_callback_interface_t *ssl_cb = (ssl_callback_interface_t *)_dpd.getSSLCallback();

    if (pop == NULL)
        return;

#ifdef SNORT_RELOAD
    pPolicyConfig = (POPConfig *)sfPolicyUserDataGet(pop->config, pop->policy_id);

    if (pPolicyConfig != NULL)
    {
        pPolicyConfig->ref_count--;
        if ((pPolicyConfig->ref_count == 0) &&
            (pop->config != pop_config))
        {
            sfPolicyUserDataClear (pop->config, pop->policy_id);
            POP_FreeConfig(pPolicyConfig);

            /* No more outstanding policies for this config */
            if (sfPolicyUserPolicyGetActive(pop->config) == 0)
                POP_FreeConfigs(pop->config);
        }
    }
#endif

    if(pop->mime_ssn.decode_state != NULL)
    {
        mempool_free(pop_mime_mempool, pop->mime_ssn.decode_bkt);
        _dpd.snortFree(pop->mime_ssn.decode_state, sizeof(Email_DecodeState),
             PP_POP,PP_MEM_CATEGORY_SESSION);
    }

    if(pop->mime_ssn.log_state != NULL)
    {
        mempool_free(pop_mempool, pop->mime_ssn.log_state->log_hdrs_bkt);
        _dpd.snortFree(pop->mime_ssn.log_state, sizeof(MAIL_LogState), PP_POP,
             PP_MEM_CATEGORY_SESSION);
       
    }

    if ( ssl_cb )
        ssl_cb->session_free(pop->flow_id);

    _dpd.snortFree(pop, sizeof(*pop), PP_POP, PP_MEM_CATEGORY_SESSION);
    if(pop_stats.cur_sessions)    
	pop_stats.cur_sessions--;
    if(pop_stats.conc_sessions)
       pop_stats.conc_sessions--;
}

static int POP_FreeConfigsPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    POPConfig *pPolicyConfig = (POPConfig *)pData;

    //do any housekeeping before freeing POPConfig
    sfPolicyUserDataClear (config, policyId);
    POP_FreeConfig(pPolicyConfig);

    return 0;
}

void POP_FreeConfigs(tSfPolicyUserContextId config)
{
    if (config == NULL)
        return;

    sfPolicyUserDataFreeIterate (config, POP_FreeConfigsPolicy);
    sfPolicyConfigDelete(config);
}

void POP_FreeConfig(POPConfig *config)
{
    if (config == NULL)
        return;

    if (config->cmds != NULL)
    {
        POPToken *tmp = config->cmds;

        for (; tmp->name != NULL; tmp++)
            _dpd.snortFree(tmp->name, sizeof(*(tmp->name)), PP_POP, PP_MEM_CATEGORY_CONFIG);

        _dpd.snortFree(config->cmds, sizeof(*(config->cmds)), PP_POP, PP_MEM_CATEGORY_CONFIG);
    }

    if (config->cmd_search_mpse != NULL)
        _dpd.searchAPI->search_instance_free(config->cmd_search_mpse);

    if (config->cmd_search != NULL)
        _dpd.snortFree(config->cmd_search, sizeof(*(config->cmd_search)), PP_POP, PP_MEM_CATEGORY_CONFIG);

    _dpd.snortFree(config, sizeof(*config), PP_POP, PP_MEM_CATEGORY_CONFIG);
}


/*
 * Free anything that needs it before shutting down preprocessor
 *
 * @param   none
 *
 * @return  none
 */
void POP_Free(void)
{
    POP_FreeConfigs(pop_config);
    pop_config = NULL;

    if (pop_resp_search_mpse != NULL)
        _dpd.searchAPI->search_instance_free(pop_resp_search_mpse);
}


/*
 * Callback function for string search
 *
 * @param   id      id in array of search strings from pop_config.cmds
 * @param   index   index in array of search strings from pop_config.cmds
 * @param   data    buffer passed in to search function
 *
 * @return response
 * @retval 1        commands caller to stop searching
 */
static int POP_SearchStrFound(void *id, void *unused, int index, void *data, void *unused2)
{
    int search_id = (int)(uintptr_t)id;

    pop_search_info.id = search_id;
    pop_search_info.index = index;
    pop_search_info.length = pop_current_search[search_id].name_len;

    /* Returning non-zero stops search, which is okay since we only look for one at a time */
    return 1;
}

/*
 * Handle COMMAND state
 *
 * @param   p       standard Packet structure
 * @param   ptr     pointer into p->payload buffer to start looking at data
 * @param   end     points to end of p->payload buffer
 *
 * @return          pointer into p->payload where we stopped looking at data
 *                  will be end of line or end of packet
 */
static const uint8_t * POP_HandleCommand(SFSnortPacket *p, const uint8_t *ptr, const uint8_t *end)
{
    const uint8_t *eol;   /* end of line */
    const uint8_t *eolm;  /* end of line marker */
    int cmd_found;

    /* get end of line and end of line marker */
    POP_GetEOL(ptr, end, &eol, &eolm);


    /* TODO If the end of line marker coincides with the end of payload we can't be
     * sure that we got a command and not a substring which we could tell through
     * inspection of the next packet. Maybe a command pending state where the first
     * char in the next packet is checked for a space and end of line marker */

    /* do not confine since there could be space chars before command */
    pop_current_search = &pop_eval_config->cmd_search[0];
    cmd_found = _dpd.searchAPI->search_instance_find
        (pop_eval_config->cmd_search_mpse, (const char *)ptr,
         eolm - ptr, 0, POP_SearchStrFound);

    /* see if we actually found a command and not a substring */
    if (cmd_found > 0)
    {
        const uint8_t *tmp = ptr;
        const uint8_t *cmd_start = ptr + pop_search_info.index;
        const uint8_t *cmd_end = cmd_start + pop_search_info.length;

#ifdef DUMP_BUFFER
     dumpBuffer(POP_REQUEST_COMMAND_DUMP,ptr,(cmd_end-cmd_start));
#endif
        /* move past spaces up until start of command */
        while ((tmp < cmd_start) && isspace((int)*tmp))
            tmp++;

        /* if not all spaces before command, we found a
         * substring */
        if (tmp != cmd_start)
            cmd_found = 0;

        /* if we're before the end of line marker and the next
         * character is not whitespace, we found a substring */
        if ((cmd_end < eolm) && !isspace((int)*cmd_end))
            cmd_found = 0;

        /* there is a chance that end of command coincides with the end of payload
         * in which case, it could be a substring, but for now, we will treat it as found */
    }

    /* if command not found, alert and move on */
    if (!cmd_found)
    {
#ifdef DUMP_BUFFER
     dumpBuffer(POP_REQUEST_COMMAND_DUMP,ptr,(eolm-ptr));
#endif
        if (pop_ssn->state == STATE_UNKNOWN && 
           ((pop_ssn->session_flags & POP_FLAG_CHECK_SSL) && 
           (IsSSL(ptr, end - ptr, p->flags))))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_POP, "Command not found, but state is "
                                                "unknown - checking for SSL\n"););

            /* check for encrypted */

            DEBUG_WRAP(DebugMessage(DEBUG_POP, "Packet is SSL encrypted\n"););

            pop_ssn->state = STATE_TLS_DATA;

            /* Ignore data */
            return end;
        }
        else
        {
            if (pop_ssn->state == STATE_UNKNOWN)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_POP, "Not SSL - try data state\n"););
                /* don't check for ssl again in this packet */
                if (pop_ssn->session_flags & POP_FLAG_CHECK_SSL)
                    pop_ssn->session_flags &= ~POP_FLAG_CHECK_SSL;

                pop_ssn->state = STATE_DATA;
                POP_GenerateAlert(POP_UNKNOWN_CMD, "%s", POP_UNKNOWN_CMD_STR);
                DEBUG_WRAP(DebugMessage(DEBUG_POP, "Unknown POP command found\n"););
                return ptr;
            }
            POP_GenerateAlert(POP_UNKNOWN_CMD, "%s", POP_UNKNOWN_CMD_STR);
            DEBUG_WRAP(DebugMessage(DEBUG_POP, "Unknown POP command found\n"););
            return eol;
        }
    }

    else if (pop_search_info.id == CMD_TOP)
    {
        pop_ssn->state = STATE_DATA;
    }
    else
    {
        if (pop_ssn->state == STATE_UNKNOWN)
            pop_ssn->state = STATE_COMMAND;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_POP, "%s\n", pop_eval_config->cmds[pop_search_info.id].name););

    if(pop_search_info.id == CMD_STLS)
    {
        if (eol == end)
            pop_ssn->state = STATE_TLS_CLIENT_PEND;
    }

/*    switch (pop_search_info.id)
    {
        case CMD_USER:
        case CMD_PASS:
        case CMD_RSET:
        case CMD_QUIT:
        case CMD_RETR:
            break;

        default:
            break;
    }*/

    return eol;
}

/*
 * Process client packet
 *
 * @param   packet  standard Packet structure
 *
 * @return  none
 */
static void POP_ProcessClientPacket(SFSnortPacket *p)
{
    const uint8_t *ptr = p->payload;
    const uint8_t *end = p->payload + p->payload_size;

#ifdef DUMP_BUFFER
    dumpBuffer(POP_REQUEST_PAYLOAD_DUMP,p->payload,p->payload_size);
#endif


    ptr = POP_HandleCommand(p, ptr, end);


}

/*
 * Process server packet
 *
 * @param   packet  standard Packet structure
 *
 */
static void POP_ProcessServerPacket(SFSnortPacket *p)
{
    int resp_found;
    const uint8_t *ptr;
    const uint8_t *end;
    const uint8_t *eolm;
    const uint8_t *eol;
    int resp_line_len;
    const char *tmp = NULL;

    ptr = p->payload;
    end = p->payload + p->payload_size;

#ifdef DUMP_BUFFER
    dumpBuffer(POP_RESPONSE_PAYLOAD_DUMP,p->payload,p->payload_size);
#endif

    while (ptr < end)
    {
        if(pop_ssn->state == STATE_DATA)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_POP, "DATA STATE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"););
            //ptr = POP_HandleData(p, ptr, end);
            ptr = _dpd.fileAPI->process_mime_data(p, ptr, end, &(pop_ssn->mime_ssn), 0, true, "POP", PP_POP);
            continue;
        }
        POP_GetEOL(ptr, end, &eol, &eolm);

        resp_line_len = eol - ptr;

        /* Check for response code */
        pop_current_search = &pop_resp_search[0];
        resp_found = _dpd.searchAPI->search_instance_find
            (pop_resp_search_mpse, (const char *)ptr,
             resp_line_len, 1, POP_SearchStrFound);


        if (resp_found > 0)
        {
            const uint8_t *cmd_start = ptr + pop_search_info.index;

#ifdef DUMP_BUFFER
    dumpBuffer(POP_RESPONSE_STATUS_DUMP,ptr,(eolm-ptr));
#endif
            switch (pop_search_info.id)
            {
                case RESP_OK:
                    tmp = _dpd.SnortStrcasestr((const char *)cmd_start, (eol - cmd_start), "octets");
                    if(tmp != NULL)
                        pop_ssn->state = STATE_DATA;
                    else
                    {
                        pop_ssn->prev_response = RESP_OK;
                        pop_ssn->state = STATE_UNKNOWN;
                    }
                    break;

                default:
                    break;
            }

        }
        else
        {
#ifdef DUMP_BUFFER
    dumpBuffer(POP_RESPONSE_STATUS_DUMP,ptr, (eolm-ptr));
#endif
            DEBUG_WRAP(DebugMessage(DEBUG_POP, "Server response not found - see if it's SSL data\n"););

            if ((pop_ssn->session_flags & POP_FLAG_CHECK_SSL) &&
                    (IsSSL(ptr, end - ptr, p->flags)))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_POP, "Server response is an SSL packet\n"););

                pop_ssn->state = STATE_TLS_DATA;

                return;
            }
            else if (pop_ssn->session_flags & POP_FLAG_CHECK_SSL)
            {
                pop_ssn->session_flags &= ~POP_FLAG_CHECK_SSL;
            }
            if(pop_ssn->prev_response == RESP_OK)
            {
                {
                    pop_ssn->state = STATE_DATA;
                    pop_ssn->prev_response = 0;
                    continue;
                }
            }
            else if(*ptr == '+')
            {
                POP_GenerateAlert(POP_UNKNOWN_RESP, "%s", POP_UNKNOWN_RESP_STR);
                DEBUG_WRAP(DebugMessage(DEBUG_POP, "Server response not found\n"););
            }
        }

        ptr = eol;

    }

    return;
}

/* For Target based
 * If a protocol for the session is already identified and not one POP is
 * interested in, POP should leave it alone and return without processing.
 * If a protocol for the session is already identified and is one that POP is
 * interested in, decode it.
 * If the protocol for the session is not already identified and the preprocessor
 * is configured to detect on one of the packet ports, detect.
 * Returns 0 if we should not inspect
 *         1 if we should continue to inspect
 */
static int POP_Inspect(SFSnortPacket *p)
{
#ifdef TARGET_BASED
    /* POP could be configured to be stateless.  If stream isn't configured, assume app id
     * will never be set and just base inspection on configuration */
    if (p->stream_session == NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_POP, "POP: Target-based: No stream session.\n"););

        if ((POP_IsServer(p->src_port) && (p->flags & FLAG_FROM_SERVER)) ||
            (POP_IsServer(p->dst_port) && (p->flags & FLAG_FROM_CLIENT)))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_POP, "POP: Target-based: Configured for this "
                                    "traffic, so let's inspect.\n"););
            return 1;
        }
    }
    else
    {
        int16_t app_id = _dpd.sessionAPI->get_application_protocol_id(p->stream_session);

        if (app_id != 0)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_POP, "POP: Target-based: App id: %u.\n", app_id););

            if (app_id == pop_proto_id)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_POP, "POP: Target-based: App id is "
                                        "set to \"%s\".\n", POP_PROTO_REF_STR););
                return 1;
            }
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_POP, "POP: Target-based: Unknown protocol for "
                                    "this session.  See if we're configured.\n"););

            if ((POP_IsServer(p->src_port) && (p->flags & FLAG_FROM_SERVER)) ||
                (POP_IsServer(p->dst_port) && (p->flags & FLAG_FROM_CLIENT)))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_POP, "POP: Target-based: POP port is configured."););
                return 1;
            }
        }
    }

    DEBUG_WRAP(DebugMessage(DEBUG_POP,"POP: Target-based: Not inspecting ...\n"););

#else
    /* Make sure it's traffic we're interested in */
    if ((POP_IsServer(p->src_port) && (p->flags & FLAG_FROM_SERVER)) ||
        (POP_IsServer(p->dst_port) && (p->flags & FLAG_FROM_CLIENT)))
        return 1;

#endif  /* TARGET_BASED */

    return 0;
}

/*
 * Entry point to snort preprocessor for each packet
 *
 * @param   packet  standard Packet structure
 *
 * @return  none
 */
void SnortPOP(SFSnortPacket *p)
{
    int detected = 0;
    int pkt_dir;
    tSfPolicyId policy_id = _dpd.getNapRuntimePolicy();
    ssl_callback_interface_t *ssl_cb = (ssl_callback_interface_t *)_dpd.getSSLCallback();

    PROFILE_VARS;
#ifdef DUMP_BUFFER
    dumpBufferInit();
#endif

    pop_ssn = (POP *)_dpd.sessionAPI->get_application_data(p->stream_session, PP_POP);
    if (pop_ssn != NULL)
        pop_eval_config = (POPConfig *)sfPolicyUserDataGet(pop_ssn->config, pop_ssn->policy_id);
    else
        pop_eval_config = (POPConfig *)sfPolicyUserDataGetCurrent(pop_config);

    if (pop_eval_config == NULL)
        return;

    if (pop_ssn == NULL)
    {
        if (!POP_Inspect(p))
            return;

        pop_ssn = POP_GetNewSession(p, policy_id);
        if (pop_ssn == NULL)
            return;
    }

    pkt_dir = POP_Setup(p, pop_ssn);

    if (pkt_dir == POP_PKT_FROM_CLIENT)
    {
        /* This packet should be a tls client hello */
        if (pop_ssn->state == STATE_TLS_CLIENT_PEND)
        {
#ifdef DUMP_BUFFER
    dumpBuffer(POP_STATE_TLS_CLIENT_PEND_DUMP,p->payload,p->payload_size);
#endif
            if (IsTlsClientHello(p->payload, p->payload + p->payload_size))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_POP,
                                        "TLS DATA STATE ~~~~~~~~~~~~~~~~~~~~~~~~~\n"););

                pop_ssn->state = STATE_TLS_SERVER_PEND;
                if(ssl_cb)
                    ssl_cb->session_initialize(p, pop_ssn, POP_Set_flow_id);
                return;
            }
            else
            {
                /* reset state - server may have rejected STARTTLS command */
                pop_ssn->state = STATE_UNKNOWN;
            }
        }
        if ((pop_ssn->state == STATE_TLS_DATA)
                || (pop_ssn->state == STATE_TLS_SERVER_PEND))
        {
#ifdef DUMP_BUFFER
    dumpBuffer(POP_STATE_TLS_DATA_DUMP,p->payload,p->payload_size);
#endif
            if( _dpd.streamAPI->is_session_decrypted(p->stream_session))
            {
                pop_ssn->state = STATE_COMMAND;
            }
            else
            {
                return;
            }
        }
        POP_ProcessClientPacket(p);
        DEBUG_WRAP(DebugMessage(DEBUG_POP, "POP client packet\n"););
    }
    else
    {
#ifdef DEBUG_MSGS
        if (pkt_dir == POP_PKT_FROM_SERVER)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_POP, "POP server packet\n"););
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_POP, "POP packet NOT from client or server! "
                        "Processing as a server packet\n"););
        }
#endif
        if (pop_ssn->state == STATE_TLS_SERVER_PEND)
        {
#ifdef DUMP_BUFFER
    dumpBuffer(POP_STATE_TLS_SERVER_PEND_DUMP,p->payload,p->payload_size);
#endif
            if( _dpd.streamAPI->is_session_decrypted(p->stream_session))
            {
                pop_ssn->state = STATE_COMMAND;
            }
            else if (IsTlsServerHello(p->payload, p->payload + p->payload_size))
            {
                pop_ssn->state = STATE_TLS_DATA;
            }
            else if (!(_dpd.sessionAPI->get_session_flags(p->stream_session) & SSNFLAG_MIDSTREAM)
                        && !_dpd.streamAPI->missed_packets(p->stream_session, SSN_DIR_BOTH))
            {
                /* revert back to command state - assume server didn't accept STARTTLS */
                pop_ssn->state = STATE_UNKNOWN;
            }
            else
                return;
        }

        if (pop_ssn->state == STATE_TLS_DATA)
        {
#ifdef DUMP_BUFFER
    dumpBuffer(POP_STATE_TLS_DATA_DUMP,p->payload,p->payload_size);
#endif
            if( _dpd.streamAPI->is_session_decrypted(p->stream_session))
            {
                pop_ssn->state = STATE_COMMAND;
            }
            else
                return;
        }

        {
            if (!_dpd.readyForProcess(p))
            {
                /* Packet will be rebuilt, so wait for it */
                DEBUG_WRAP(DebugMessage(DEBUG_POP, "Client packet will be reassembled\n"));
                return;
            }
            else if (pop_ssn->reassembling && !(p->flags & FLAG_REBUILT_STREAM))
            {
                /* If this isn't a reassembled packet and didn't get
                 * inserted into reassembly buffer, there could be a
                 * problem.  If we miss syn or syn-ack that had window
                 * scaling this packet might not have gotten inserted
                 * into reassembly buffer because it fell outside of
                 * window, because we aren't scaling it */
                pop_ssn->session_flags |= POP_FLAG_GOT_NON_REBUILT;
                pop_ssn->state = STATE_UNKNOWN;
            }
            else if (pop_ssn->reassembling && (pop_ssn->session_flags & POP_FLAG_GOT_NON_REBUILT))
            {
                /* This is a rebuilt packet.  If we got previous packets
                 * that were not rebuilt, state is going to be messed up
                 * so set state to unknown. It's likely this was the
                 * beginning of the conversation so reset state */
                DEBUG_WRAP(DebugMessage(DEBUG_POP, "Got non-rebuilt packets before "
                    "this rebuilt packet\n"););

                pop_ssn->state = STATE_UNKNOWN;
                pop_ssn->session_flags &= ~POP_FLAG_GOT_NON_REBUILT;
            }
            /* Process as a server packet */
            POP_ProcessServerPacket(p);
        }
    }


    PREPROC_PROFILE_START(popDetectPerfStats);

    detected = _dpd.detect(p);

#ifdef PERF_PROFILING
    popDetectCalled = 1;
#endif

    PREPROC_PROFILE_END(popDetectPerfStats);

    /* Turn off detection since we've already done it. */
    POP_DisableDetect(p);

    if (detected)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_POP, "POP vulnerability detected\n"););
    }
}

static void POP_DisableDetect(SFSnortPacket *p)
{
    _dpd.disableAllDetect(p);

    _dpd.enablePreprocessor(p, PP_SDF);
}

static inline POP *POP_GetSession(void *data)
{
    if(data)
        return (POP *)_dpd.sessionAPI->get_application_data(data, PP_POP);

    return NULL;
}

/* Callback to return the MIME attachment filenames accumulated */
int POP_GetFilename(void *data, uint8_t **buf, uint32_t *len, uint32_t *type)
{
    POP *ssn = POP_GetSession(data);

    if(ssn == NULL)
        return 0;

    *buf = ssn->mime_ssn.log_state->file_log.filenames;
    *len = ssn->mime_ssn.log_state->file_log.file_logged;

    return 1;
}
void POP_Set_flow_id( void *app_data, uint32_t fid )
{
    POP *ssn = (POP *)app_data;
    if( ssn )
        ssn->flow_id = fid;
}
