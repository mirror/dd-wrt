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
 * snort_imap.c
 *
 * Author: Bhagyashree Bantwal <bbantwal@cisco.com>
 *
 * Description:
 *
 * This file handles IMAP protocol checking and normalization.
 *
 * Entry point functions:
 *
 *     SnortIMAP()
 *     IMAP_Init()
 *     IMAP_Free()
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
#include "snort_imap.h"
#include "imap_config.h"
#include "imap_util.h"
#include "imap_log.h"

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

#ifdef DUMP_BUFFER
#include "imap_buffer_dump.h"
#endif

#include "imap_paf.h"
/**************************************************************************/


/* Externs ****************************************************************/

#ifdef PERF_PROFILING
extern PreprocStats imapDetectPerfStats;
extern int imapDetectCalled;
#endif

extern tSfPolicyUserContextId imap_config;
extern IMAPConfig *imap_eval_config;
extern MemPool *imap_mempool;
extern MemPool *imap_mime_mempool;

#ifdef DEBUG_MSGS
extern char imap_print_buffer[];
#endif

/**************************************************************************/


/* Globals ****************************************************************/

const IMAPToken imap_known_cmds[] =
{
    {"APPEND",          6, CMD_APPEND},
    {"AUTHENTICATE",    12, CMD_AUTHENTICATE},
    {"CAPABILITY",      10, CMD_CAPABILITY},
    {"CHECK",           5, CMD_CHECK},
    {"CLOSE",           5, CMD_CLOSE},
    {"COMPARATOR",      10, CMD_COMPARATOR},
    {"COMPRESS",        8, CMD_COMPRESS},
    {"CONVERSIONS",     11, CMD_CONVERSIONS},
    {"COPY",            4, CMD_COPY},
    {"CREATE",          6, CMD_CREATE},
    {"DELETE",          6, CMD_DELETE},
    {"DELETEACL",       9, CMD_DELETEACL},
    {"DONE",            4, CMD_DONE},
    {"EXAMINE",         7, CMD_EXAMINE},
    {"EXPUNGE",         7, CMD_EXPUNGE},
    {"FETCH",           5, CMD_FETCH},
    {"GETACL",          6, CMD_GETACL},
    {"GETMETADATA",     11, CMD_GETMETADATA},
    {"GETQUOTA",        8, CMD_GETQUOTA},
    {"GETQUOTAROOT",    12, CMD_GETQUOTAROOT},
    {"IDLE",            4, CMD_IDLE},
    {"LIST",            4, CMD_LIST},
    {"LISTRIGHTS",      10, CMD_LISTRIGHTS},
    {"LOGIN",           5, CMD_LOGIN},
    {"LOGOUT",          6, CMD_LOGOUT},
    {"LSUB",            4, CMD_LSUB},
    {"MYRIGHTS",        8, CMD_MYRIGHTS},
    {"NOOP",            4, CMD_NOOP},
    {"NOTIFY",          6, CMD_NOTIFY},
    {"RENAME",          6, CMD_RENAME},
    {"SEARCH",          6, CMD_SEARCH},
    {"SELECT",          6, CMD_SELECT},
    {"SETACL",          6, CMD_SETACL},
    {"SETMETADATA",     11, CMD_SETMETADATA},
    {"SETQUOTA",        8, CMD_SETQUOTA},
    {"SORT",            4, CMD_SORT},
    {"STARTTLS",        8, CMD_STARTTLS},
    {"STATUS",          6, CMD_STATUS},
    {"STORE",           5, CMD_STORE},
    {"SUBSCRIBE",       9, CMD_SUBSCRIBE},
    {"THREAD",          6, CMD_THREAD},
    {"UID",             3, CMD_UID},
    {"UNSELECT",        8, CMD_UNSELECT},
    {"UNSUBSCRIBE",     11, CMD_UNSUBSCRIBE},
    {"X",               1, CMD_X},
    {NULL,              0, 0}
};

const IMAPToken imap_resps[] =
{
    {"CAPABILITY",      10, RESP_CAPABILITY},
    {"LIST",            4, RESP_LIST},
    {"LSUB",            4, RESP_LSUB},
    {"STATUS",          6, RESP_STATUS},
    {"SEARCH",          6, RESP_SEARCH},
    {"FLAGS",           5, RESP_FLAGS},
    {"EXISTS",          6, RESP_EXISTS},
    {"RECENT",          6, RESP_RECENT},
    {"EXPUNGE",         7, RESP_EXPUNGE},
    {"FETCH",           5, RESP_FETCH},
	{"BAD",             3, RESP_BAD},
	{"BYE",             3, RESP_BYE},
	{"NO",              2, RESP_NO},
	{"OK",              2, RESP_OK},
	{"PREAUTH",         7, RESP_PREAUTH},
	{"ENVELOPE",        8, RESP_ENVELOPE},
	{"UID",             3, RESP_UID},
	{NULL,              0, 0}
};

IMAP *imap_ssn = NULL;
IMAPSearchInfo imap_search_info;

#ifdef DEBUG_MSGS
uint64_t imap_session_counter = 0;
#endif

#ifdef TARGET_BASED
int16_t imap_proto_id;
#endif

void *imap_resp_search_mpse = NULL;
IMAPSearch imap_resp_search[RESP_LAST];

IMAPSearch *imap_current_search = NULL;


/**************************************************************************/


/* Private functions ******************************************************/

static int IMAP_Setup(SFSnortPacket *p, IMAP *ssn);
static void IMAP_ResetState(void);
static void IMAP_SessionFree(void *);
static int IMAP_GetPacketDirection(SFSnortPacket *, int);
static void IMAP_ProcessClientPacket(SFSnortPacket *);
static void IMAP_ProcessServerPacket(SFSnortPacket *);
static void IMAP_DisableDetect(SFSnortPacket *);
static const uint8_t * IMAP_HandleCommand(SFSnortPacket *, const uint8_t *, const uint8_t *);
static int IMAP_SearchStrFound(void *, void *, int, void *, void *);


static int IMAP_Inspect(SFSnortPacket *);

void IMAP_Set_flow_id( void *app_data, uint32_t fid );
MimeMethods mime_methods = {NULL, NULL, IMAP_DecodeAlert, IMAP_ResetState, is_data_end};
/**************************************************************************/

void IMAP_InitCmds(IMAPConfig *config)
{
    const IMAPToken *tmp;

    if (config == NULL)
        return;

    /* add one to CMD_LAST for NULL entry */
    config->cmds = (IMAPToken *)_dpd.snortAlloc(CMD_LAST + 1, sizeof(IMAPToken), PP_IMAP, 
                                     PP_MEM_CATEGORY_CONFIG);
    if (config->cmds == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => failed to allocate memory for imap "
                                        "command structure\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
    }

    for (tmp = &imap_known_cmds[0]; tmp->name != NULL; tmp++)
    {
        config->cmds[tmp->search_id].name_len = tmp->name_len;
        config->cmds[tmp->search_id].search_id = tmp->search_id;
        config->cmds[tmp->search_id].name = strdup(tmp->name);

        if (config->cmds[tmp->search_id].name == NULL)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => failed to allocate memory for imap "
                                            "command structure\n",
                                            *(_dpd.config_file), *(_dpd.config_line));
        }
    }

    /* initialize memory for command searches */
    config->cmd_search = (IMAPSearch *)_dpd.snortAlloc(CMD_LAST, sizeof(IMAPSearch), PP_IMAP, 
                                            PP_MEM_CATEGORY_CONFIG);
    if (config->cmd_search == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => failed to allocate memory for imap "
                                        "command structure\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
    }

    config->num_cmds = CMD_LAST;
}

/*
 * Initialize IMAP searches
 *
 * @param  none
 *
 * @return none
 */
void IMAP_SearchInit(void)
{
    const IMAPToken *tmp;

    /* Response search */
    imap_resp_search_mpse = _dpd.searchAPI->search_instance_new();
    if (imap_resp_search_mpse == NULL)
    {
        DynamicPreprocessorFatalMessage("Could not allocate IMAP "
                                        "response search.\n");
    }

    for (tmp = &imap_resps[0]; tmp->name != NULL; tmp++)
    {
        imap_resp_search[tmp->search_id].name = tmp->name;
        imap_resp_search[tmp->search_id].name_len = tmp->name_len;

        _dpd.searchAPI->search_instance_add(imap_resp_search_mpse, tmp->name,
                                            tmp->name_len, tmp->search_id);
    }

    _dpd.searchAPI->search_instance_prep(imap_resp_search_mpse);

}

/*
 * Reset IMAP session state
 *
 * @param  none
 *
 * @return none
 */
static void IMAP_ResetState(void)
{
    imap_ssn->state = STATE_COMMAND;
    imap_ssn->state_flags = 0;
    imap_ssn->body_read = imap_ssn->body_len = 0;
}

/*
 * Given a server configuration and a port number, we decide if the port is
 *  in the IMAP server port list.
 *
 *  @param  port       the port number to compare with the configuration
 *
 *  @return integer
 *  @retval  0 means that the port is not a server port
 *  @retval !0 means that the port is a server port
 */
int IMAP_IsServer(uint16_t port)
{
    if( isPortEnabled( imap_eval_config->ports, port ) )
        return 1;

    return 0;
}

static IMAP * IMAP_GetNewSession(SFSnortPacket *p, tSfPolicyId policy_id)
{
    IMAP *ssn;
    IMAPConfig *pPolicyConfig = NULL;
    int ret = 0;

    pPolicyConfig = (IMAPConfig *)sfPolicyUserDataGetCurrent(imap_config);

    DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "Creating new session data structure\n"););

    ssn = (IMAP *)_dpd.snortAlloc(1, sizeof(IMAP), PP_IMAP, PP_MEM_CATEGORY_SESSION);
    if (ssn == NULL)
    {
        DynamicPreprocessorFatalMessage("Failed to allocate IMAP session data\n");
    }

    imap_ssn = ssn;
    ssn->session_flags |= IMAP_FLAG_CHECK_SSL;

    imap_ssn->mime_ssn.log_config = &(imap_eval_config->log_config);
    imap_ssn->mime_ssn.decode_conf = &(imap_eval_config->decode_conf);
    imap_ssn->mime_ssn.mime_mempool = imap_mime_mempool;
    imap_ssn->mime_ssn.log_mempool = imap_mempool;
    imap_ssn->mime_ssn.mime_stats = &(imap_stats.mime_stats);
    imap_ssn->mime_ssn.methods = &(mime_methods);

    if (( ret = _dpd.fileAPI->set_log_buffers(&(imap_ssn->mime_ssn.log_state), &(pPolicyConfig->log_config),imap_mempool, p->stream_session, PP_IMAP)) < 0)
    {
        if( ret == -1 )
        {
            if(imap_stats.log_memcap_exceeded % 10000 == 0)
            {
                _dpd.logMsg("WARNING: IMAP memcap exceeded.\n");
            }
            imap_stats.log_memcap_exceeded++;
        }
	_dpd.snortFree(ssn, sizeof(*ssn), PP_IMAP, PP_MEM_CATEGORY_SESSION);
        return NULL;
    }

    _dpd.sessionAPI->set_application_data(p->stream_session, PP_IMAP,
                                         ssn, &IMAP_SessionFree);

    if (p->flags & SSNFLAG_MIDSTREAM)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "Got midstream packet - "
                                "setting state to unknown\n"););
        ssn->state = STATE_UNKNOWN;
    }

#ifdef DEBUG_MSGS
    imap_session_counter++;
    ssn->session_number = imap_session_counter;
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

    ssn->body_read = ssn->body_len = 0;

    ssn->policy_id = policy_id;
    ssn->config = imap_config;
    ssn->flow_id = 0;
    pPolicyConfig->ref_count++;
    imap_stats.sessions++;
    imap_stats.conc_sessions++;
    imap_stats.cur_sessions++;
    if(imap_stats.max_conc_sessions < imap_stats.conc_sessions)
       imap_stats.max_conc_sessions = imap_stats.conc_sessions;

    return ssn;
}


/*
 * Do first-packet setup
 *
 * @param   p   standard Packet structure
 *
 * @return  none
 */
static int IMAP_Setup(SFSnortPacket *p, IMAP *ssn)
{
    int flags = 0;
    int pkt_dir;

    if (p->stream_session != NULL)
    {
        /* set flags to session flags */
        flags = _dpd.sessionAPI->get_session_flags(p->stream_session);
    }

    /* Figure out direction of packet */
    pkt_dir = IMAP_GetPacketDirection(p, flags);

    DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "Session number: "STDu64"\n", ssn->session_number););

    if (!(ssn->session_flags & IMAP_FLAG_CHECK_SSL))
                ssn->session_flags |= IMAP_FLAG_CHECK_SSL;
    /* Check to see if there is a reassembly gap.  If so, we won't know
     * what state we're in when we get the _next_ reassembled packet */
    if ((pkt_dir != IMAP_PKT_FROM_SERVER) &&
        (p->flags & FLAG_REBUILT_STREAM))
    {
        int missing_in_rebuilt =
            _dpd.streamAPI->missing_in_reassembled(p->stream_session, SSN_DIR_TO_CLIENT);

        if (ssn->session_flags & IMAP_FLAG_NEXT_STATE_UNKNOWN)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "Found gap in previous reassembly buffer - "
                                    "set state to unknown\n"););
            ssn->state = STATE_UNKNOWN;
            ssn->session_flags &= ~IMAP_FLAG_NEXT_STATE_UNKNOWN;
        }

        if (missing_in_rebuilt == SSN_MISSING_BEFORE)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "Found missing packets before "
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
static int IMAP_GetPacketDirection(SFSnortPacket *p, int flags)
{
    int pkt_direction = IMAP_PKT_FROM_UNKNOWN;

    if (flags & SSNFLAG_MIDSTREAM)
    {
        if (IMAP_IsServer(p->src_port) &&
            !IMAP_IsServer(p->dst_port))
        {
            pkt_direction = IMAP_PKT_FROM_SERVER;
        }
        else if (!IMAP_IsServer(p->src_port) &&
                 IMAP_IsServer(p->dst_port))
        {
            pkt_direction = IMAP_PKT_FROM_CLIENT;
        }
    }
    else
    {
        if (p->flags & FLAG_FROM_SERVER)
        {
            pkt_direction = IMAP_PKT_FROM_SERVER;
        }
        else if (p->flags & FLAG_FROM_CLIENT)
        {
            pkt_direction = IMAP_PKT_FROM_CLIENT;
        }

        /* if direction is still unknown ... */
        if (pkt_direction == IMAP_PKT_FROM_UNKNOWN)
        {
            if (IMAP_IsServer(p->src_port) &&
                !IMAP_IsServer(p->dst_port))
            {
                pkt_direction = IMAP_PKT_FROM_SERVER;
            }
            else if (!IMAP_IsServer(p->src_port) &&
                     IMAP_IsServer(p->dst_port))
            {
                pkt_direction = IMAP_PKT_FROM_CLIENT;
            }
        }
    }

    return pkt_direction;
}


/*
 * Free IMAP-specific related to this session
 *
 * @param   v   pointer to IMAP session structure
 *
 *
 * @return  none
 */
static void IMAP_SessionFree(void *session_data)
{
    IMAP *imap = (IMAP *)session_data;
#ifdef SNORT_RELOAD
    IMAPConfig *pPolicyConfig = NULL;
#endif
    ssl_callback_interface_t *ssl_cb = (ssl_callback_interface_t *)_dpd.getSSLCallback();

    if (imap == NULL)
        return;

#ifdef SNORT_RELOAD
    pPolicyConfig = (IMAPConfig *)sfPolicyUserDataGet(imap->config, imap->policy_id);

    if (pPolicyConfig != NULL)
    {
        pPolicyConfig->ref_count--;
        if ((pPolicyConfig->ref_count == 0) &&
            (imap->config != imap_config))
        {
            sfPolicyUserDataClear (imap->config, imap->policy_id);
            IMAP_FreeConfig(pPolicyConfig);

            /* No more outstanding policies for this config */
            if (sfPolicyUserPolicyGetActive(imap->config) == 0)
                IMAP_FreeConfigs(imap->config);
        }
    }
#endif

    if(imap->mime_ssn.decode_state != NULL)
    {
        mempool_free(imap_mime_mempool, imap->mime_ssn.decode_bkt);
	_dpd.snortFree(imap->mime_ssn.decode_state, sizeof(Email_DecodeState), PP_IMAP, 
             PP_MEM_CATEGORY_SESSION);
    }

    if(imap->mime_ssn.log_state != NULL)
    {
        mempool_free(imap_mempool, imap->mime_ssn.log_state->log_hdrs_bkt);
	_dpd.snortFree(imap->mime_ssn.log_state, sizeof(MAIL_LogState), PP_IMAP, 
             PP_MEM_CATEGORY_SESSION);
    }
    if ( ssl_cb )
        ssl_cb->session_free(imap->flow_id);

    _dpd.snortFree(imap, sizeof(*imap), PP_IMAP, PP_MEM_CATEGORY_SESSION);
    if(imap_stats.conc_sessions)
       imap_stats.conc_sessions--;

    if(imap_stats.cur_sessions)
	imap_stats.cur_sessions--;
}

static int IMAP_FreeConfigsPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    IMAPConfig *pPolicyConfig = (IMAPConfig *)pData;

    //do any housekeeping before freeing IMAPConfig
    sfPolicyUserDataClear (config, policyId);
    IMAP_FreeConfig(pPolicyConfig);

    return 0;
}

void IMAP_FreeConfigs(tSfPolicyUserContextId config)
{
    if (config == NULL)
        return;

    sfPolicyUserDataFreeIterate (config, IMAP_FreeConfigsPolicy);
    sfPolicyConfigDelete(config);
}

void IMAP_FreeConfig(IMAPConfig *config)
{
    if (config == NULL)
        return;

    if (config->cmds != NULL)
    {
        IMAPToken *tmp = config->cmds;

        for (; tmp->name != NULL; tmp++)
	    _dpd.snortFree(tmp->name, sizeof(*(tmp->name)), PP_IMAP, PP_MEM_CATEGORY_CONFIG);

	_dpd.snortFree(config->cmds, sizeof(*(config->cmds)), PP_IMAP, PP_MEM_CATEGORY_CONFIG);
    }

    if (config->cmd_search_mpse != NULL)
        _dpd.searchAPI->search_instance_free(config->cmd_search_mpse);

    if (config->cmd_search != NULL)
	_dpd.snortFree(config->cmd_search, sizeof(*(config->cmd_search)), PP_IMAP, PP_MEM_CATEGORY_CONFIG);

    _dpd.snortFree(config, sizeof(*config), PP_IMAP, PP_MEM_CATEGORY_CONFIG);
}


/*
 * Free anything that needs it before shutting down preprocessor
 *
 * @param   none
 *
 * @return  none
 */
void IMAP_Free(void)
{
    IMAP_FreeConfigs(imap_config);
    imap_config = NULL;

    if (imap_resp_search_mpse != NULL)
        _dpd.searchAPI->search_instance_free(imap_resp_search_mpse);
}


/*
 * Callback function for string search
 *
 * @param   id      id in array of search strings from imap_config.cmds
 * @param   index   index in array of search strings from imap_config.cmds
 * @param   data    buffer passed in to search function
 *
 * @return response
 * @retval 1        commands caller to stop searching
 */
static int IMAP_SearchStrFound(void *id, void *unused, int index, void *data, void *unused2)
{
    int search_id = (int)(uintptr_t)id;

    imap_search_info.id = search_id;
    imap_search_info.index = index;
    imap_search_info.length = imap_current_search[search_id].name_len;

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
static const uint8_t * IMAP_HandleCommand(SFSnortPacket *p, const uint8_t *ptr, const uint8_t *end)
{
    const uint8_t *eol;   /* end of line */
    const uint8_t *eolm;  /* end of line marker */
    int cmd_found;

    /* get end of line and end of line marker */
    IMAP_GetEOL(ptr, end, &eol, &eolm);

    /* TODO If the end of line marker coincides with the end of payload we can't be
     * sure that we got a command and not a substring which we could tell through
     * inspection of the next packet. Maybe a command pending state where the first
     * char in the next packet is checked for a space and end of line marker */

    /* do not confine since there could be space chars before command */
    imap_current_search = &imap_eval_config->cmd_search[0];
    cmd_found = _dpd.searchAPI->search_instance_find
        (imap_eval_config->cmd_search_mpse, (const char *)ptr,
         eolm - ptr, 0, IMAP_SearchStrFound);

    /* if command not found, alert and move on */
    if (!cmd_found)
    {
        if (imap_ssn->state == STATE_UNKNOWN)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "Command not found, but state is "
                                    "unknown - checking for SSL\n"););

            /* check for encrypted */

            if ((imap_ssn->session_flags & IMAP_FLAG_CHECK_SSL) &&
                        (IsSSL(ptr, end - ptr, p->flags)))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "Packet is SSL encrypted\n"););

                imap_ssn->state = STATE_TLS_DATA;

                /* Ignore data */
                return end;
            }
            else
            {
                DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "Not SSL - try data state\n"););
                /* don't check for ssl again in this packet */
                if (imap_ssn->session_flags & IMAP_FLAG_CHECK_SSL)
                    imap_ssn->session_flags &= ~IMAP_FLAG_CHECK_SSL;

                imap_ssn->state = STATE_DATA;
                //imap_ssn->data_state = STATE_DATA_UNKNOWN;

                return ptr;
            }
        }
        else
        {
            IMAP_GenerateAlert(IMAP_UNKNOWN_CMD, "%s", IMAP_UNKNOWN_CMD_STR);
            DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "No known command found\n"););
            return eol;
        }
    }
    else
    {

#ifdef DUMP_BUFFER
        dumpBuffer(IMAP_CLIENT_CMD_DUMP,ptr,eolm-ptr);
#endif

        if (imap_ssn->state == STATE_UNKNOWN)
            imap_ssn->state = STATE_COMMAND;
    }

    /* At this point we have definitely found a legitimate command */

    DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "%s\n", imap_eval_config->cmds[imap_search_info.id].name););

    if(imap_search_info.id == CMD_STARTTLS)
    {
        if (eol == end)
            imap_ssn->state = STATE_TLS_CLIENT_PEND;
    }

    return eol;
}

/*
 * Process client packet
 *
 * @param   packet  standard Packet structure
 *
 * @return  none
 */
static void IMAP_ProcessClientPacket(SFSnortPacket *p)
{
    const uint8_t *ptr = p->payload;
    const uint8_t *end = p->payload + p->payload_size;

#ifdef DUMP_BUFFER
    dumpBuffer(IMAP_CLIENT_DUMP,p->payload,p->payload_size);
#endif

    ptr = IMAP_HandleCommand(p, ptr, end);


}



/*
 * Process server packet
 *
 * @param   packet  standard Packet structure
 *
 */
static void IMAP_ProcessServerPacket(SFSnortPacket *p)
{
    int resp_found;
    const uint8_t *ptr;
    const uint8_t *end;
    const uint8_t *data_end;
    const uint8_t *eolm;
    const uint8_t *eol;
    int resp_line_len;
    const char *tmp = NULL;
    uint8_t *body_start, *body_end;
    char *eptr;
    uint32_t len = 0;

    body_start = body_end = NULL;

#ifdef DUMP_BUFFER
    dumpBuffer(IMAP_SERVER_DUMP,p->payload,p->payload_size);
#endif


    ptr = p->payload;
    end = p->payload + p->payload_size;

    while (ptr < end)
    {
        if(imap_ssn->state == STATE_DATA)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "DATA STATE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"););
            if( imap_ssn->body_len > imap_ssn->body_read)
            {

                len = imap_ssn->body_len - imap_ssn->body_read ;
                if( (uint32_t)(end - ptr) < len )
                {
                    data_end = end;
                    len = data_end - ptr;
                }
                else
                    data_end = ptr + len;

#ifdef DUMP_BUFFER
                dumpBuffer(IMAP_SERVER_BODY_DATA_DUMP,ptr,len);
#endif

                ptr = _dpd.fileAPI->process_mime_data(p, ptr, end, &(imap_ssn->mime_ssn), 0, true, "IMAP", PP_IMAP);

                if( ptr < data_end)
                    len = len - (data_end - ptr);

                imap_ssn->body_read += len;

                continue;
            }
            else
            {
                imap_ssn->body_len = imap_ssn->body_read = 0;
                IMAP_ResetState();
            }
        }
        IMAP_GetEOL(ptr, end, &eol, &eolm);

        resp_line_len = eol - ptr;

        /* Check for response code */
        imap_current_search = &imap_resp_search[0];
        resp_found = _dpd.searchAPI->search_instance_find
            (imap_resp_search_mpse, (const char *)ptr,
             resp_line_len, 0, IMAP_SearchStrFound);

        if (resp_found > 0)
        {
            const uint8_t *cmd_start = ptr + imap_search_info.index;
            switch (imap_search_info.id)
            {
                case RESP_FETCH:
                    imap_ssn->body_len = imap_ssn->body_read = 0;
                    imap_ssn->state = STATE_DATA;
                    tmp = _dpd.SnortStrcasestr((const char *)cmd_start, (eol - cmd_start), "BODY");
                    if(tmp != NULL)
                        imap_ssn->state = STATE_DATA;
                    else
                    {
                        tmp = _dpd.SnortStrcasestr((const char *)cmd_start, (eol - cmd_start), "RFC822");
                        if(tmp != NULL)
                            imap_ssn->state = STATE_DATA;
                        else
                            imap_ssn->state = STATE_UNKNOWN;
                    }
                    break;
                default:
                    break;
            }

            if(imap_ssn->state == STATE_DATA)
            {
                body_start = (uint8_t *)memchr((char *)ptr, '{', (eol - ptr));
                if( body_start == NULL )
                {
                    imap_ssn->state = STATE_UNKNOWN;
                }
                else
                {
                    if( (body_start + 1) < (uint8_t *)eol )
                    {
                        len = (uint32_t)_dpd.SnortStrtoul((const char *)(body_start + 1), &eptr, 10);
                        if (*eptr != '}')
                        {
                            imap_ssn->state = STATE_UNKNOWN;
                        }
                        else
                            imap_ssn->body_len = len;

                        len = 0;
                    }
                    else
                        imap_ssn->state = STATE_UNKNOWN;

                }
            }

        }
        else
        {
            if ((imap_ssn->session_flags & IMAP_FLAG_CHECK_SSL) &&
                    (IsSSL(ptr, end - ptr, p->flags)))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "Server response is an SSL packet\n"););

                imap_ssn->state = STATE_TLS_DATA;

                return;
            }
            else if (imap_ssn->session_flags & IMAP_FLAG_CHECK_SSL)
            {
                imap_ssn->session_flags &= ~IMAP_FLAG_CHECK_SSL;
            }
            if ( (*ptr != '*') && (*ptr !='+') && (*ptr != '\r') && (*ptr != '\n') )
            {
                IMAP_GenerateAlert(IMAP_UNKNOWN_RESP, "%s", IMAP_UNKNOWN_RESP_STR);
                DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "Server response not found\n"););
            }

        }


        ptr = eol;

    }

    return;
}

/* For Target based
 * If a protocol for the session is already identified and not one IMAP is
 * interested in, IMAP should leave it alone and return without processing.
 * If a protocol for the session is already identified and is one that IMAP is
 * interested in, decode it.
 * If the protocol for the session is not already identified and the preprocessor
 * is configured to detect on one of the packet ports, detect.
 * Returns 0 if we should not inspect
 *         1 if we should continue to inspect
 */
static int IMAP_Inspect(SFSnortPacket *p)
{
#ifdef TARGET_BASED
    /* IMAP could be configured to be stateless.  If stream isn't configured, assume app id
     * will never be set and just base inspection on configuration */
    if (p->stream_session == NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "IMAP: Target-based: No stream session.\n"););

        if ((IMAP_IsServer(p->src_port) && (p->flags & FLAG_FROM_SERVER)) ||
            (IMAP_IsServer(p->dst_port) && (p->flags & FLAG_FROM_CLIENT)))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "IMAP: Target-based: Configured for this "
                                    "traffic, so let's inspect.\n"););
            return 1;
        }
    }
    else
    {
        int16_t app_id = _dpd.sessionAPI->get_application_protocol_id(p->stream_session);

        if (app_id != 0)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "IMAP: Target-based: App id: %u.\n", app_id););

            if (app_id == imap_proto_id)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "IMAP: Target-based: App id is "
                                        "set to \"%s\".\n", IMAP_PROTO_REF_STR););
                return 1;
            }
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "IMAP: Target-based: Unknown protocol for "
                                    "this session.  See if we're configured.\n"););

            if ((IMAP_IsServer(p->src_port) && (p->flags & FLAG_FROM_SERVER)) ||
                (IMAP_IsServer(p->dst_port) && (p->flags & FLAG_FROM_CLIENT)))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "IMAP: Target-based: IMAP port is configured."););
                return 1;
            }
        }
    }

    DEBUG_WRAP(DebugMessage(DEBUG_IMAP,"IMAP: Target-based: Not inspecting ...\n"););

#else
    /* Make sure it's traffic we're interested in */
    if ((IMAP_IsServer(p->src_port) && (p->flags & FLAG_FROM_SERVER)) ||
        (IMAP_IsServer(p->dst_port) && (p->flags & FLAG_FROM_CLIENT)))
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
void SnortIMAP(SFSnortPacket *p)
{
    int detected = 0;
    int pkt_dir;
    tSfPolicyId policy_id = _dpd.getNapRuntimePolicy();
    ssl_callback_interface_t *ssl_cb = (ssl_callback_interface_t *)_dpd.getSSLCallback();

#ifdef DUMP_BUFFER
    dumpBufferInit();
#endif


    PROFILE_VARS;


    imap_ssn = (IMAP *)_dpd.sessionAPI->get_application_data(p->stream_session, PP_IMAP);
    if (imap_ssn != NULL)
        imap_eval_config = (IMAPConfig *)sfPolicyUserDataGet(imap_ssn->config, imap_ssn->policy_id);
    else
        imap_eval_config = (IMAPConfig *)sfPolicyUserDataGetCurrent(imap_config);

    if (imap_eval_config == NULL)
        return;

    if (imap_ssn == NULL)
    {
        if (!IMAP_Inspect(p))
            return;

        imap_ssn = IMAP_GetNewSession(p, policy_id);
        if (imap_ssn == NULL)
            return;
    }

    pkt_dir = IMAP_Setup(p, imap_ssn);

    if (pkt_dir == IMAP_PKT_FROM_CLIENT)
    {
        /* This packet should be a tls client hello */
        if (imap_ssn->state == STATE_TLS_CLIENT_PEND)
        {
            if (IsTlsClientHello(p->payload, p->payload + p->payload_size))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_IMAP,
                                        "TLS DATA STATE ~~~~~~~~~~~~~~~~~~~~~~~~~\n"););

                imap_ssn->state = STATE_TLS_SERVER_PEND;
                if(ssl_cb)
                    ssl_cb->session_initialize(p, imap_ssn, IMAP_Set_flow_id);
                return;
            }
            else
            {
                /* reset state - server may have rejected STARTTLS command */
                imap_ssn->state = STATE_UNKNOWN;
            }
        }
        if ((imap_ssn->state == STATE_TLS_DATA)
                    || (imap_ssn->state == STATE_TLS_SERVER_PEND))
        {
            if( _dpd.streamAPI->is_session_decrypted(p->stream_session))
            {
                imap_ssn->state = STATE_COMMAND;
            }
            else
                return;
        }
        IMAP_ProcessClientPacket(p);
        DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "IMAP client packet\n"););
    }
    else
    {
#ifdef DEBUG_MSGS
        if (pkt_dir == IMAP_PKT_FROM_SERVER)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "IMAP server packet\n"););
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "IMAP packet NOT from client or server! "
                        "Processing as a server packet\n"););
        }
#endif
        if (imap_ssn->state == STATE_TLS_SERVER_PEND)
        {
            if( _dpd.streamAPI->is_session_decrypted(p->stream_session))
            {
                imap_ssn->state = STATE_COMMAND;
            }
            else if (IsTlsServerHello(p->payload, p->payload + p->payload_size))
            {
                imap_ssn->state = STATE_TLS_DATA;
            }
            else if (!(_dpd.sessionAPI->get_session_flags(p->stream_session) & SSNFLAG_MIDSTREAM)
                    && !_dpd.streamAPI->missed_packets(p->stream_session, SSN_DIR_BOTH))
            {
                /* revert back to command state - assume server didn't accept STARTTLS */
                imap_ssn->state = STATE_UNKNOWN;
            }
            else 
                return;
        }

        if (imap_ssn->state == STATE_TLS_DATA)
        {
            if( _dpd.streamAPI->is_session_decrypted(p->stream_session))
            {
                imap_ssn->state = STATE_COMMAND;
            }
            else
                return;
        }

        {
            if (!_dpd.readyForProcess(p))
            {
                /* Packet will be rebuilt, so wait for it */
                DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "Client packet will be reassembled\n"));
                return;
            }
            else if (imap_ssn->reassembling && !(p->flags & FLAG_REBUILT_STREAM))
            {
                /* If this isn't a reassembled packet and didn't get
                 * inserted into reassembly buffer, there could be a
                 * problem.  If we miss syn or syn-ack that had window
                 * scaling this packet might not have gotten inserted
                 * into reassembly buffer because it fell outside of
                 * window, because we aren't scaling it */
                imap_ssn->session_flags |= IMAP_FLAG_GOT_NON_REBUILT;
                imap_ssn->state = STATE_UNKNOWN;
            }
            else if (imap_ssn->reassembling && (imap_ssn->session_flags & IMAP_FLAG_GOT_NON_REBUILT))
            {
                /* This is a rebuilt packet.  If we got previous packets
                 * that were not rebuilt, state is going to be messed up
                 * so set state to unknown. It's likely this was the
                 * beginning of the conversation so reset state */
                DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "Got non-rebuilt packets before "
                    "this rebuilt packet\n"););

                imap_ssn->state = STATE_UNKNOWN;
                imap_ssn->session_flags &= ~IMAP_FLAG_GOT_NON_REBUILT;
            }
            /* Process as a server packet */
            IMAP_ProcessServerPacket(p);
        }
    }


    PREPROC_PROFILE_START(imapDetectPerfStats);

    detected = _dpd.detect(p);

#ifdef PERF_PROFILING
    imapDetectCalled = 1;
#endif

    PREPROC_PROFILE_END(imapDetectPerfStats);

    /* Turn off detection since we've already done it. */
    IMAP_DisableDetect(p);

    if (detected)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "IMAP vulnerability detected\n"););
    }
}

static void IMAP_DisableDetect(SFSnortPacket *p)
{
    _dpd.disableAllDetect(p);

    _dpd.enablePreprocessor(p, PP_SDF);
}

static inline IMAP *IMAP_GetSession(void *data)
{
    if(data)
        return (IMAP *)_dpd.sessionAPI->get_application_data(data, PP_IMAP);

    return NULL;
}

/* Callback to return the MIME attachment filenames accumulated */
int IMAP_GetFilename(void *data, uint8_t **buf, uint32_t *len, uint32_t *type)
{
    IMAP *ssn = IMAP_GetSession(data);

    if(ssn == NULL)
        return 0;

    *buf = ssn->mime_ssn.log_state->file_log.filenames;
    *len = ssn->mime_ssn.log_state->file_log.file_logged;

    return 1;
}

void IMAP_Set_flow_id( void *app_data, uint32_t fid )
{
    IMAP *ssn = (IMAP *)app_data;
    if( ssn )
        ssn->flow_id = fid;
}
