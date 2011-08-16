/****************************************************************************
 *
 * Copyright (C) 2005-2011 Sourcefire, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/

/**************************************************************************
 * snort_smtp.c
 *
 * Author: Andy Mullican
 * Author: Todd Wease
 *
 * Description:
 *
 * This file handles SMTP protocol checking and normalization.
 *
 * Entry point functions:
 *
 *     SnortSMTP()
 *     SMTP_Init()
 *     SMTP_Free()
 *
 **************************************************************************/


/* Includes ***************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

#include "sf_types.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pcre.h>

#include "snort_smtp.h"
#include "smtp_config.h"
#include "smtp_normalize.h"
#include "smtp_util.h"
#include "smtp_log.h"
#include "smtp_xlink2state.h"

#include "sf_snort_packet.h"
#include "stream_api.h"
#include "debug.h"
#include "profiler.h"
#include "bounds.h"
#include "sf_dynamic_preprocessor.h"
#include "ssl.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"

#ifdef DEBUG
#include "sf_types.h"
#endif

/**************************************************************************/


/* Externs ****************************************************************/

#ifdef PERF_PROFILING
extern PreprocStats smtpDetectPerfStats;
extern int smtpDetectCalled;
#endif

extern tSfPolicyUserContextId smtp_config;
extern SMTPConfig *smtp_eval_config;
extern MemPool *smtp_mime_mempool;
extern DynamicPreprocessorData _dpd;

#ifdef DEBUG
extern char smtp_print_buffer[];
#endif

/**************************************************************************/


/* Globals ****************************************************************/

const SMTPToken smtp_known_cmds[] =
{
    {"ATRN",          4, CMD_ATRN},
    {"AUTH",          4, CMD_AUTH},
    {"BDAT",          4, CMD_BDAT},
    {"DATA",          4, CMD_DATA},
    {"DEBUG",         5, CMD_DEBUG},
    {"EHLO",          4, CMD_EHLO},
    {"EMAL",          4, CMD_EMAL},
    {"ESAM",          4, CMD_ESAM},
    {"ESND",          4, CMD_ESND},
    {"ESOM",          4, CMD_ESOM},
    {"ETRN",          4, CMD_ETRN},
    {"EVFY",          4, CMD_EVFY},
    {"EXPN",          4, CMD_EXPN},
    {"HELO",          4, CMD_HELO},
    {"HELP",          4, CMD_HELP},
    {"IDENT",         5, CMD_IDENT},
    {"MAIL",          4, CMD_MAIL},
    {"NOOP",          4, CMD_NOOP},
    {"ONEX",          4, CMD_ONEX},
    {"QUEU",          4, CMD_QUEU},
    {"QUIT",          4, CMD_QUIT},
    {"RCPT",          4, CMD_RCPT},
    {"RSET",          4, CMD_RSET},
    {"SAML",          4, CMD_SAML},
    {"SEND",          4, CMD_SEND},
    {"SIZE",          4, CMD_SIZE},
    {"STARTTLS",      8, CMD_STARTTLS},
    {"SOML",          4, CMD_SOML},
    {"TICK",          4, CMD_TICK},
    {"TIME",          4, CMD_TIME},
    {"TURN",          4, CMD_TURN},
    {"TURNME",        6, CMD_TURNME},
    {"VERB",          4, CMD_VERB},
    {"VRFY",          4, CMD_VRFY},
    {"X-EXPS",        6, CMD_X_EXPS},
    {"XADR",          4, CMD_XADR},
    {"XAUTH",         5, CMD_XAUTH},
    {"XCIR",          4, CMD_XCIR},
    {"XEXCH50",       7, CMD_XEXCH50},
    {"XGEN",          4, CMD_XGEN},
    {"XLICENSE",      8, CMD_XLICENSE},
    {"X-LINK2STATE", 12, CMD_X_LINK2STATE},
    {"XQUE",          4, CMD_XQUE},
    {"XSTA",          4, CMD_XSTA},
    {"XTRN",          4, CMD_XTRN},
    {"XUSR",          4, CMD_XUSR},
    {NULL,            0, 0}
};

const SMTPToken smtp_resps[] =
{
	{"220",  3,  RESP_220},  /* Service ready - initial response and STARTTLS response */
	{"221",  3,  RESP_221},  /* Goodbye - response to QUIT */
	{"250",  3,  RESP_250},  /* Requested mail action okay, completed */
	{"354",  3,  RESP_354},  /* Start mail input - data response */
	{"421",  3,  RESP_421},  /* Service not availiable - closes connection */
	{"450",  3,  RESP_450},  /* Mailbox unavailable */
	{"451",  3,  RESP_451},  /* Local error in processing */
	{"452",  3,  RESP_452},  /* Insufficient system storage */
	{"500",  3,  RESP_500},  /* Command unrecognized */
	{"501",  3,  RESP_501},  /* Syntax error in parameters or arguments */
	{"502",  3,  RESP_502},  /* Command not implemented */
	{"503",  3,  RESP_503},  /* Bad sequence of commands */
	{"504",  3,  RESP_504},  /* Command parameter not implemented */
	{"550",  3,  RESP_550},  /* Action not taken - mailbox unavailable */
	{"551",  3,  RESP_551},  /* User not local; please try <forward-path> */
	{"552",  3,  RESP_552},  /* Mail action aborted: exceeded storage allocation */
	{"553",  3,  RESP_553},  /* Action not taken: mailbox name not allowed */
	{"554",  3,  RESP_554},  /* Transaction failed */
	{NULL,   0,  0}
};

const SMTPToken smtp_hdrs[] =
{
    {"Content-type:", 13, HDR_CONTENT_TYPE},
    {"Content-Transfer-Encoding:", 26, HDR_CONT_TRANS_ENC},
    {NULL,             0, 0}
};

const SMTPToken smtp_data_end[] =
{
	{"\r\n.\r\n",  5,  DATA_END_1},
	{"\n.\r\n",    4,  DATA_END_2},
	{"\r\n.\n",    4,  DATA_END_3},
	{"\n.\n",      3,  DATA_END_4},
	{NULL,         0,  0}
};

SMTP *smtp_ssn = NULL;
SMTP smtp_no_session;
SMTPPcre mime_boundary_pcre;
char smtp_normalizing;
SMTPSearchInfo smtp_search_info;

#ifdef DEBUG
uint64_t smtp_session_counter = 0;
#endif

#ifdef TARGET_BASED
int16_t smtp_proto_id;
#endif

void *smtp_resp_search_mpse = NULL;
SMTPSearch smtp_resp_search[RESP_LAST];

void *smtp_hdr_search_mpse = NULL;
SMTPSearch smtp_hdr_search[HDR_LAST];

void *smtp_data_search_mpse = NULL;
SMTPSearch smtp_data_end_search[DATA_END_LAST];

SMTPSearch *smtp_current_search = NULL;


/**************************************************************************/


/* Private functions ******************************************************/

static int SMTP_Setup(SFSnortPacket *p, SMTP *ssn);
static void SMTP_ResetState(void);
static void SMTP_SessionFree(void *);
static void SMTP_NoSessionFree(void);
static int SMTP_GetPacketDirection(SFSnortPacket *, int);
static void SMTP_ProcessClientPacket(SFSnortPacket *);
static int SMTP_ProcessServerPacket(SFSnortPacket *);
static void SMTP_DisableDetect(SFSnortPacket *);
static const uint8_t * SMTP_HandleCommand(SFSnortPacket *, const uint8_t *, const uint8_t *);
static const uint8_t * SMTP_HandleData(SFSnortPacket *, const uint8_t *, const uint8_t *);
static const uint8_t * SMTP_HandleHeader(SFSnortPacket *, const uint8_t *, const uint8_t *);
static const uint8_t * SMTP_HandleDataBody(SFSnortPacket *, const uint8_t *, const uint8_t *);
static int SMTP_SearchStrFound(void *, void *, int, void *, void *);

static int SMTP_BoundaryStrFound(void *, void *, int , void *, void *);
static int SMTP_GetBoundary(const char *, int);
static int SMTP_IsTlsClientHello(const uint8_t *, const uint8_t *);
static int SMTP_IsTlsServerHello(const uint8_t *, const uint8_t *);
static int SMTP_IsSSL(const uint8_t *, int, int);

static int SMTP_Inspect(SFSnortPacket *);

/**************************************************************************/

static void SetSmtpBuffers(SMTP *ssn)
{
    if ((ssn != NULL) && (ssn->decode_state == NULL)
            && smtp_eval_config->enable_mime_decoding)
    {
        MemBucket *bkt = mempool_alloc(smtp_mime_mempool);

        if (bkt != NULL)
        {
            ssn->decode_state = (SMTP_DecodeState *)calloc(1, sizeof(SMTP_DecodeState));
            if( ssn->decode_state != NULL )
            {
                ssn->decode_state->decode_present = 0;
                ssn->decode_state->mime_bucket = bkt;
                ssn->decode_state->encode_depth = smtp_eval_config->max_mime_depth;
                ssn->decode_state->decode_depth = smtp_eval_config->max_mime_decode_bytes;
                ssn->decode_state->encodeBuf = (unsigned char *)bkt->data;
                ssn->decode_state->decodeBuf = (unsigned char *)bkt->data + smtp_eval_config->max_mime_depth;
                ssn->decode_state->decoded_bytes = 0;
                ssn->decode_state->prev_encoded_bytes = 0;
                ssn->decode_state->prev_encoded_buf = NULL;
            }
                                                                                                                                    }
    }
}


void SMTP_InitCmds(SMTPConfig *config)
{
    const SMTPToken *tmp;

    if (config == NULL)
        return;

    /* add one to CMD_LAST for NULL entry */
    config->cmds = (SMTPToken *)calloc(CMD_LAST + 1, sizeof(SMTPToken));
    if (config->cmds == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => failed to allocate memory for smtp "
                                        "command structure\n", 
                                        *(_dpd.config_file), *(_dpd.config_line));
    }

    for (tmp = &smtp_known_cmds[0]; tmp->name != NULL; tmp++)
    {
        config->cmds[tmp->search_id].name_len = tmp->name_len;
        config->cmds[tmp->search_id].search_id = tmp->search_id;
        config->cmds[tmp->search_id].name = strdup(tmp->name);

        if (config->cmds[tmp->search_id].name == NULL)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => failed to allocate memory for smtp "
                                            "command structure\n", 
                                            *(_dpd.config_file), *(_dpd.config_line));
        }
    }

    /* initialize memory for command searches */
    config->cmd_search = (SMTPSearch *)calloc(CMD_LAST, sizeof(SMTPSearch));
    if (config->cmd_search == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => failed to allocate memory for smtp "
                                        "command structure\n", 
                                        *(_dpd.config_file), *(_dpd.config_line));
    }

    config->num_cmds = CMD_LAST;
}


/*
 * Initialize SMTP searches
 *
 * @param  none
 *
 * @return none
 */
void SMTP_SearchInit(void)
{
    const char *error;
    int erroffset;
    const SMTPToken *tmp;

    /* Response search */
    smtp_resp_search_mpse = _dpd.searchAPI->search_instance_new();
    if (smtp_resp_search_mpse == NULL)
    {
        DynamicPreprocessorFatalMessage("Could not allocate SMTP "
                                        "response search.\n");
    }

    for (tmp = &smtp_resps[0]; tmp->name != NULL; tmp++)
    {
        smtp_resp_search[tmp->search_id].name = tmp->name;
        smtp_resp_search[tmp->search_id].name_len = tmp->name_len;

        _dpd.searchAPI->search_instance_add(smtp_resp_search_mpse, tmp->name,
                                            tmp->name_len, tmp->search_id);
    }

    _dpd.searchAPI->search_instance_prep(smtp_resp_search_mpse);

    /* Header search */
    smtp_hdr_search_mpse = _dpd.searchAPI->search_instance_new();
    if (smtp_hdr_search_mpse == NULL)
    {
        DynamicPreprocessorFatalMessage("Could not allocate SMTP "
                                        "header search.\n");
    }

    for (tmp = &smtp_hdrs[0]; tmp->name != NULL; tmp++)
    {
        smtp_hdr_search[tmp->search_id].name = tmp->name;
        smtp_hdr_search[tmp->search_id].name_len = tmp->name_len;

        _dpd.searchAPI->search_instance_add(smtp_hdr_search_mpse, tmp->name,
                                            tmp->name_len, tmp->search_id);
    }

    _dpd.searchAPI->search_instance_prep(smtp_hdr_search_mpse);

    /* Data end search */
    smtp_data_search_mpse = _dpd.searchAPI->search_instance_new();
    if (smtp_data_search_mpse == NULL)
    {
        DynamicPreprocessorFatalMessage("Could not allocate SMTP "
                                        "data search.\n");
    }

    for (tmp = &smtp_data_end[0]; tmp->name != NULL; tmp++)
    {
        smtp_data_end_search[tmp->search_id].name = tmp->name;
        smtp_data_end_search[tmp->search_id].name_len = tmp->name_len;

        _dpd.searchAPI->search_instance_add(smtp_data_search_mpse, tmp->name,
                                            tmp->name_len, tmp->search_id);
    }

    _dpd.searchAPI->search_instance_prep(smtp_data_search_mpse);


    /* create regex for finding boundary string - since it can be cut across multiple
     * lines, a straight search won't do. Shouldn't be too slow since it will most
     * likely only be acting on a small portion of data */
    //"^content-type:\\s*multipart.*boundary\\s*=\\s*\"?([^\\s]+)\"?"
    //"^\\s*multipart.*boundary\\s*=\\s*\"?([^\\s]+)\"?"
    //mime_boundary_pcre.re = pcre_compile("^.*boundary\\s*=\\s*\"?([^\\s\"]+)\"?",
    //mime_boundary_pcre.re = pcre_compile("boundary(?:\n|\r\n)?=(?:\n|\r\n)?\"?([^\\s\"]+)\"?",
    mime_boundary_pcre.re = pcre_compile("boundary\\s*=\\s*\"?([^\\s\"]+)\"?",
                                          PCRE_CASELESS | PCRE_DOTALL,
                                          &error, &erroffset, NULL);
    if (mime_boundary_pcre.re == NULL)
    {
        DynamicPreprocessorFatalMessage("Failed to compile pcre regex for getting boundary "
                                        "in a multipart SMTP message: %s\n", error);
    }

    mime_boundary_pcre.pe = pcre_study(mime_boundary_pcre.re, 0, &error);

    if (error != NULL)
    {
        DynamicPreprocessorFatalMessage("Failed to study pcre regex for getting boundary "
                                        "in a multipart SMTP message: %s\n", error);
    }
}

/* 
 * Initialize run-time boundary search
 */
static int SMTP_BoundarySearchInit(void)
{
    if (smtp_ssn->mime_boundary.boundary_search != NULL)
        _dpd.searchAPI->search_instance_free(smtp_ssn->mime_boundary.boundary_search);

    smtp_ssn->mime_boundary.boundary_search = _dpd.searchAPI->search_instance_new();

    if (smtp_ssn->mime_boundary.boundary_search == NULL)
        return -1;

    _dpd.searchAPI->search_instance_add(smtp_ssn->mime_boundary.boundary_search,
                                        smtp_ssn->mime_boundary.boundary,
                                        smtp_ssn->mime_boundary.boundary_len, BOUNDARY);

    _dpd.searchAPI->search_instance_prep(smtp_ssn->mime_boundary.boundary_search);

    return 0;
}



/*
 * Reset SMTP session state
 *
 * @param  none
 *
 * @return none
 */
static void SMTP_ResetState(void)
{
    if (smtp_ssn->mime_boundary.boundary_search != NULL)
    {
        _dpd.searchAPI->search_instance_free(smtp_ssn->mime_boundary.boundary_search);
        smtp_ssn->mime_boundary.boundary_search = NULL;
    }

    smtp_ssn->state = STATE_COMMAND;
    smtp_ssn->data_state = STATE_DATA_INIT;
    smtp_ssn->state_flags = 0;
    ResetDecodeState(smtp_ssn->decode_state);
    memset(&smtp_ssn->mime_boundary, 0, sizeof(SMTPMimeBoundary));
}


/*
 * Given a server configuration and a port number, we decide if the port is
 *  in the SMTP server port list.
 *
 *  @param  port       the port number to compare with the configuration
 *
 *  @return integer
 *  @retval  0 means that the port is not a server port
 *  @retval !0 means that the port is a server port
 */
int SMTP_IsServer(uint16_t port)
{
    if (smtp_eval_config->ports[port / 8] & (1 << (port % 8)))
        return 1;

    return 0;
}

static SMTP * SMTP_GetNewSession(SFSnortPacket *p, tSfPolicyId policy_id)
{
    SMTP *ssn;
    SMTPConfig *pPolicyConfig = NULL;

    pPolicyConfig = (SMTPConfig *)sfPolicyUserDataGetCurrent(smtp_config);

    if ((p->stream_session_ptr == NULL) || (pPolicyConfig->inspection_type == SMTP_STATELESS))
    {
#ifdef DEBUG
        if (p->stream_session_ptr == NULL)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Stream session pointer is NULL - "
                                    "treating packet as stateless\n"););
        }
#endif

        SMTP_NoSessionFree();
        memset(&smtp_no_session, 0, sizeof(SMTP));
        ssn = &smtp_no_session;
        ssn->session_flags |= SMTP_FLAG_CHECK_SSL;

        return ssn;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Creating new session data structure\n"););

    ssn = (SMTP *)calloc(1, sizeof(SMTP));
    if (ssn == NULL)
    {
        DynamicPreprocessorFatalMessage("Failed to allocate SMTP session data\n");
    }

    SetSmtpBuffers(ssn);

    _dpd.streamAPI->set_application_data(p->stream_session_ptr, PP_SMTP,
                                         ssn, &SMTP_SessionFree);   

    if (p->flags & SSNFLAG_MIDSTREAM)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Got midstream packet - "
                                "setting state to unknown\n"););
        ssn->state = STATE_UNKNOWN;
    }

#ifdef DEBUG
    smtp_session_counter++;
    ssn->session_number = smtp_session_counter;
#endif

    if (p->stream_session_ptr != NULL)
    {
        /* check to see if we're doing client reassembly in stream */
        if (_dpd.streamAPI->get_reassembly_direction(p->stream_session_ptr) & SSN_DIR_CLIENT)
            ssn->reassembling = 1;
    }

    ssn->policy_id = policy_id;
    ssn->config = smtp_config;
    pPolicyConfig->ref_count++;

    return ssn;
}


/*
 * Do first-packet setup
 *
 * @param   p   standard Packet structure
 *
 * @return  none
 */
static int SMTP_Setup(SFSnortPacket *p, SMTP *ssn)
{
    int flags = 0;
    int pkt_dir;

    if (p->stream_session_ptr != NULL)
    {
        /* set flags to session flags */
        flags = _dpd.streamAPI->get_session_flags(p->stream_session_ptr);
    }

    /* Figure out direction of packet */
    pkt_dir = SMTP_GetPacketDirection(p, flags);

    DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Session number: "STDu64"\n", ssn->session_number);); 

    /* reset check ssl flag for new packet */
    if (!(ssn->session_flags & SMTP_FLAG_CHECK_SSL))
        ssn->session_flags |= SMTP_FLAG_CHECK_SSL;

    /* Check to see if there is a reassembly gap.  If so, we won't know
     * what state we're in when we get the _next_ reassembled packet */
    if ((pkt_dir != SMTP_PKT_FROM_SERVER) &&
        (p->flags & FLAG_REBUILT_STREAM))
    {
        int missing_in_rebuilt =
            _dpd.streamAPI->missing_in_reassembled(p->stream_session_ptr, SSN_DIR_CLIENT);

        if (ssn->session_flags & SMTP_FLAG_NEXT_STATE_UNKNOWN)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Found gap in previous reassembly buffer - "
                                    "set state to unknown\n"););
            ssn->state = STATE_UNKNOWN;
            ssn->session_flags &= ~SMTP_FLAG_NEXT_STATE_UNKNOWN;
        }

        if (missing_in_rebuilt == SSN_MISSING_BOTH)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Found missing packets before and after "
                                    "in reassembly buffer - set state to unknown and "
                                    "next state to unknown\n"););
            ssn->state = STATE_UNKNOWN;
            ssn->session_flags |= SMTP_FLAG_NEXT_STATE_UNKNOWN;
        }
        else if (missing_in_rebuilt == SSN_MISSING_BEFORE)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Found missing packets before "
                                    "in reassembly buffer - set state to unknown\n"););
            ssn->state = STATE_UNKNOWN;
        }
        else if (missing_in_rebuilt == SSN_MISSING_AFTER)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Found missing packets after "
                                    "in reassembly buffer - set next state to unknown\n"););
            ssn->session_flags |= SMTP_FLAG_NEXT_STATE_UNKNOWN;
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
static int SMTP_GetPacketDirection(SFSnortPacket *p, int flags)
{    
    int pkt_direction = SMTP_PKT_FROM_UNKNOWN;

    if (flags & SSNFLAG_MIDSTREAM)
    {
        if (SMTP_IsServer(p->src_port) &&
            !SMTP_IsServer(p->dst_port))
        {
            pkt_direction = SMTP_PKT_FROM_SERVER;
        }
        else if (!SMTP_IsServer(p->src_port) &&
                 SMTP_IsServer(p->dst_port))
        {
            pkt_direction = SMTP_PKT_FROM_CLIENT;
        }
    }
    else
    {
        if (p->flags & FLAG_FROM_SERVER)
        {
            pkt_direction = SMTP_PKT_FROM_SERVER;
        }
        else if (p->flags & FLAG_FROM_CLIENT)
        {
            pkt_direction = SMTP_PKT_FROM_CLIENT;
        }

        /* if direction is still unknown ... */
        if (pkt_direction == SMTP_PKT_FROM_UNKNOWN)
        {
            if (SMTP_IsServer(p->src_port) &&
                !SMTP_IsServer(p->dst_port))
            {
                pkt_direction = SMTP_PKT_FROM_SERVER;
            }
            else if (!SMTP_IsServer(p->src_port) &&
                     SMTP_IsServer(p->dst_port))
            {
                pkt_direction = SMTP_PKT_FROM_CLIENT;
            }
        }
    }

    return pkt_direction;
}


/*
 * Free SMTP-specific related to this session
 *
 * @param   v   pointer to SMTP session structure
 *
 *
 * @return  none
 */
static void SMTP_SessionFree(void *session_data)
{
    SMTP *smtp = (SMTP *)session_data;
#ifdef SNORT_RELOAD
    SMTPConfig *pPolicyConfig = NULL;
#endif

    if (smtp == NULL)
        return;

#ifdef SNORT_RELOAD
    pPolicyConfig = (SMTPConfig *)sfPolicyUserDataGet(smtp->config, smtp->policy_id);

    if (pPolicyConfig != NULL)
    {
        pPolicyConfig->ref_count--;
        if ((pPolicyConfig->ref_count == 0) &&
            (smtp->config != smtp_config))
        {
            sfPolicyUserDataClear (smtp->config, smtp->policy_id);
            SMTP_FreeConfig(pPolicyConfig);

            /* No more outstanding policies for this config */
            if (sfPolicyUserPolicyGetActive(smtp->config) == 0)
                SMTP_FreeConfigs(smtp->config);
        }
    }
#endif

    if (smtp->mime_boundary.boundary_search != NULL)
    {
        _dpd.searchAPI->search_instance_free(smtp->mime_boundary.boundary_search);
        smtp->mime_boundary.boundary_search = NULL;
    }

    if(smtp->decode_state != NULL)
    {
        mempool_free(smtp_mime_mempool, smtp->decode_state->mime_bucket);
        free(smtp->decode_state);
    }

    free(smtp);
}


static void SMTP_NoSessionFree(void)
{
    if (smtp_no_session.mime_boundary.boundary_search != NULL)
    {
        _dpd.searchAPI->search_instance_free(smtp_no_session.mime_boundary.boundary_search);
        smtp_no_session.mime_boundary.boundary_search = NULL;
    }
}

static int SMTP_FreeConfigsPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId, 
        void* pData
        )
{
    SMTPConfig *pPolicyConfig = (SMTPConfig *)pData;

    //do any housekeeping before freeing SMTPConfig
    sfPolicyUserDataClear (config, policyId);
    SMTP_FreeConfig(pPolicyConfig);

    return 0;
}

void SMTP_FreeConfigs(tSfPolicyUserContextId config)
{
    if (config == NULL)
        return;

    sfPolicyUserDataIterate (config, SMTP_FreeConfigsPolicy);
    sfPolicyConfigDelete(config);
}

void SMTP_FreeConfig(SMTPConfig *config)
{
    if (config == NULL)
        return;

    if (config->cmds != NULL)
    {
        SMTPToken *tmp = config->cmds;

        for (; tmp->name != NULL; tmp++)
            free(tmp->name);

        free(config->cmds);
    }

    if (config->cmd_config != NULL)
        free(config->cmd_config);

    if (config->cmd_search_mpse != NULL)
        _dpd.searchAPI->search_instance_free(config->cmd_search_mpse);

    if (config->cmd_search != NULL)
        free(config->cmd_search);

    free(config);
}


/*
 * Free anything that needs it before shutting down preprocessor
 *
 * @param   none
 *
 * @return  none
 */
void SMTP_Free(void)
{
    SMTP_NoSessionFree();

    SMTP_FreeConfigs(smtp_config);
    smtp_config = NULL;

    if (smtp_resp_search_mpse != NULL)
        _dpd.searchAPI->search_instance_free(smtp_resp_search_mpse);

    if (smtp_hdr_search_mpse != NULL)
        _dpd.searchAPI->search_instance_free(smtp_hdr_search_mpse);

    if (smtp_data_search_mpse != NULL)
        _dpd.searchAPI->search_instance_free(smtp_data_search_mpse);

    if (mime_boundary_pcre.re )
        pcre_free(mime_boundary_pcre.re);

    if (mime_boundary_pcre.pe )
        pcre_free(mime_boundary_pcre.pe);
}


/*
 * Callback function for string search
 *
 * @param   id      id in array of search strings from smtp_config.cmds
 * @param   index   index in array of search strings from smtp_config.cmds
 * @param   data    buffer passed in to search function
 *
 * @return response
 * @retval 1        commands caller to stop searching
 */
static int SMTP_SearchStrFound(void *id, void *unused, int index, void *data, void *unused2)
{
    int search_id = (int)(uintptr_t)id;

    smtp_search_info.id = search_id;
    smtp_search_info.index = index;
    smtp_search_info.length = smtp_current_search[search_id].name_len;

    /* Returning non-zero stops search, which is okay since we only look for one at a time */
    return 1;
}


/*
 * Callback function for boundary search
 *
 * @param   id      id in array of search strings
 * @param   index   index in array of search strings
 * @param   data    buffer passed in to search function
 *
 * @return response
 * @retval 1        commands caller to stop searching
 */
static int SMTP_BoundaryStrFound(void *id, void *unused, int index, void *data, void *unused2)
{
    int boundary_id = (int)(uintptr_t)id;

    smtp_search_info.id = boundary_id;
    smtp_search_info.index = index;
    smtp_search_info.length = smtp_ssn->mime_boundary.boundary_len;

    return 1;
}

static int SMTP_GetBoundary(const char *data, int data_len)
{
    int result;
    int ovector[9];
    int ovecsize = 9;
    const char *boundary;
    int boundary_len;
    int ret;
    char *mime_boundary;
    int  *mime_boundary_len;


    mime_boundary = &smtp_ssn->mime_boundary.boundary[0];
    mime_boundary_len = &smtp_ssn->mime_boundary.boundary_len;
    
    /* result will be the number of matches (including submatches) */
    result = pcre_exec(mime_boundary_pcre.re, mime_boundary_pcre.pe,
                       data, data_len, 0, 0, ovector, ovecsize);
    if (result < 0)
        return -1;

    result = pcre_get_substring(data, ovector, result, 1, &boundary);
    if (result < 0)
        return -1;

    boundary_len = strlen(boundary);
    if (boundary_len > MAX_BOUNDARY_LEN)
    {
        /* XXX should we alert? breaking the law of RFC */
        boundary_len = MAX_BOUNDARY_LEN;
    }

    mime_boundary[0] = '-';
    mime_boundary[1] = '-';
    ret = SafeMemcpy(mime_boundary + 2, boundary, boundary_len,
                     mime_boundary + 2, mime_boundary + 2 + MAX_BOUNDARY_LEN);

    pcre_free_substring(boundary);

    if (ret != SAFEMEM_SUCCESS)
    {
        return -1;
    }

    *mime_boundary_len = 2 + boundary_len;
    mime_boundary[*mime_boundary_len] = '\0';

    return 0;
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
static const uint8_t * SMTP_HandleCommand(SFSnortPacket *p, const uint8_t *ptr, const uint8_t *end)
{
    const uint8_t *eol;   /* end of line */
    const uint8_t *eolm;  /* end of line marker */
    int cmd_line_len;
    int ret;
    int cmd_found;
    char alert_long_command_line = 0;

    /* get end of line and end of line marker */
    SMTP_GetEOL(ptr, end, &eol, &eolm);

    /* calculate length of command line */
    cmd_line_len = eol - ptr;

    /* check for command line exceeding maximum 
     * do this before checking for a command since this could overflow
     * some server's buffers without the presence of a known command */
    if ((smtp_eval_config->max_command_line_len != 0) &&
        (cmd_line_len > smtp_eval_config->max_command_line_len))
    {
        alert_long_command_line = 1;
    }

    /* TODO If the end of line marker coincides with the end of payload we can't be
     * sure that we got a command and not a substring which we could tell through
     * inspection of the next packet. Maybe a command pending state where the first
     * char in the next packet is checked for a space and end of line marker */

    /* do not confine since there could be space chars before command */
    smtp_current_search = &smtp_eval_config->cmd_search[0];
    cmd_found = _dpd.searchAPI->search_instance_find
        (smtp_eval_config->cmd_search_mpse, (const char *)ptr,
         eolm - ptr, 0, SMTP_SearchStrFound);

    /* see if we actually found a command and not a substring */
    if (cmd_found > 0)
    {
        const uint8_t *tmp = ptr;
        const uint8_t *cmd_start = ptr + smtp_search_info.index;
        const uint8_t *cmd_end = cmd_start + smtp_search_info.length;

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
        /* If we missed one or more packets we might not actually be in the command
         * state.  Check to see if we're encrypted */
        if (smtp_ssn->state == STATE_UNKNOWN)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Command not found, but state is "
                                    "unknown - checking for SSL\n"););

            /* check for encrypted */

            if ((smtp_ssn->session_flags & SMTP_FLAG_CHECK_SSL) &&
                (SMTP_IsSSL(ptr, end - ptr, p->flags)))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Packet is SSL encrypted\n"););

                smtp_ssn->state = STATE_TLS_DATA;

                /* Ignore data */
                if (smtp_eval_config->ignore_tls_data)
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Ignoring encrypted data\n"););
                    SetAltBuffer(p, 0);
                }

                return end;
            }
            else
            {
                DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Not SSL - try data state\n"););
                /* don't check for ssl again in this packet */
                if (smtp_ssn->session_flags & SMTP_FLAG_CHECK_SSL)
                    smtp_ssn->session_flags &= ~SMTP_FLAG_CHECK_SSL;

                smtp_ssn->state = STATE_DATA;
                smtp_ssn->data_state = STATE_DATA_UNKNOWN;

                return ptr;
            }
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "No known command found\n"););

            if (smtp_eval_config->alert_unknown_cmds)
            {
                SMTP_GenerateAlert(SMTP_UNKNOWN_CMD, "%s", SMTP_UNKNOWN_CMD_STR);
            }

            if (alert_long_command_line)
            {
                SMTP_GenerateAlert(SMTP_COMMAND_OVERFLOW, "%s: more than %d chars",
                                   SMTP_COMMAND_OVERFLOW_STR, smtp_eval_config->max_command_line_len);
            }

            /* if normalizing, copy line to alt buffer */
            if (smtp_normalizing)
            {
                ret = SMTP_CopyToAltBuffer(p, ptr, eol - ptr);
                if (ret == -1)
                    return NULL;
            }

            return eol;
        }
    }

    /* At this point we have definitely found a legitimate command */

    DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "%s\n", smtp_eval_config->cmds[smtp_search_info.id].name););

    /* check if max command line length for a specific command is exceeded */
    if (smtp_eval_config->cmd_config[smtp_search_info.id].max_line_len != 0)
    {
        if (cmd_line_len > smtp_eval_config->cmd_config[smtp_search_info.id].max_line_len)
        {
            SMTP_GenerateAlert(SMTP_SPECIFIC_CMD_OVERFLOW, "%s: %s, %d chars",
                               SMTP_SPECIFIC_CMD_OVERFLOW_STR,
                               smtp_eval_config->cmd_search[smtp_search_info.id].name, cmd_line_len);
        }
    }
    else if (alert_long_command_line)
    {
        SMTP_GenerateAlert(SMTP_COMMAND_OVERFLOW, "%s: more than %d chars",
                           SMTP_COMMAND_OVERFLOW_STR, smtp_eval_config->max_command_line_len);
    }

    /* Are we alerting on this command? */
    if (smtp_eval_config->cmd_config[smtp_search_info.id].alert)
    {
        SMTP_GenerateAlert(SMTP_ILLEGAL_CMD, "%s: %s",
                           SMTP_ILLEGAL_CMD_STR, smtp_eval_config->cmds[smtp_search_info.id].name);
    }

    switch (smtp_search_info.id)
    {
        /* unless we do our own parsing of MAIL and RCTP commands we have to assume they
         * are ok unless we got a server error in which case we flush and if this is a
         * reassembled packet, the last command in this packet will be the command that
         * caused the error */
        case CMD_MAIL:
            smtp_ssn->state_flags |= SMTP_FLAG_GOT_MAIL_CMD;

            break;

        case CMD_RCPT:
            if ((smtp_ssn->state_flags & SMTP_FLAG_GOT_MAIL_CMD) ||
                smtp_ssn->state == STATE_UNKNOWN)
            {
                smtp_ssn->state_flags |= SMTP_FLAG_GOT_RCPT_CMD;
            }

            break;

        case CMD_RSET:
        case CMD_HELO:
        case CMD_EHLO:
        case CMD_QUIT:
            smtp_ssn->state_flags &= ~(SMTP_FLAG_GOT_MAIL_CMD | SMTP_FLAG_GOT_RCPT_CMD);

            break;

#if 0
        case CMD_BDAT:
            {
                const uint8_t *begin_chunk;
                const uint8_t *end_chunk;
                const uint8_t *last;
                const uint8_t *tmp;
                int num_digits;
                int ten_power;
                uint32_t bdat_chunk;

                begin_chunk = ptr + smtp_search_info.index + smtp_search_info.length;
                while ((begin_chunk < eolm) && isspace((int)*begin_chunk))
                    begin_chunk++;

                /* bad BDAT command - needs chunk argument */
                if (begin_chunk == eolm)
                    break;
                    
                end_chunk = begin_chunk;
                while ((end_chunk < eolm) && isdigit((int)*end_chunk))
                    end_chunk++;

                /* didn't get all digits */
                if ((end_chunk < eolm) && !isspace((int)*end_chunk))
                    break;

                /* get chunk size */
                num_digits = end_chunk - begin_chunk;

                /* more than 9 digits could potentially overflow a 32 bit integer
                 * this allows for one less than a billion bytes which most servers
                 * won't accept */
                if (num_digits > 9)
                    break;

                tmp = end_chunk;
                for (ten_power = 1, tmp--; tmp >= begin_chunk; ten_power *= 10, tmp--)
                {
                    bdat_chunk += (*tmp - '0') * ten_power;
                }

                /* bad bdat chunk size */
                if (bdat_chunk == 0)
                    break;

                /* got a valid chunk size - check to see if this is the last chunk */
                last = end_chunk;
                while ((last < eolm) && isspace((int)*last))
                    last++;

                /* TODO need an ESMTP argument search */
                if (last < eolm)
                {
                    /* must have at least 4 chars for 'last' */
                    if ((eolm - last) >= 4)
                    {
                        if (*last == 'l' || *last == 'L')
                        {
                            last++;
                            if (*last == 'a' || *last == 'A')
                            {
                                last++;
                                if (*last == 's' || *last == 'S')
                                {
                                    last++;
                                    if (*last == 't' || *last == 'T')
                                    {
                                        last++;
                                        while ((last < eolm) && isspace((int)*last))
                                            last++;

                                        if (last != eolm)
                                        {
                                            break;
                                        }
                                        
                                        smtp_ssn->bdat_last = 1;
                                    }
                                }
                            }
                        }
                    }
                }

                smtp_ssn->state = STATE_BDAT;
                smtp_ssn->bdat_chunk = bdat_chunk;
            }

            break;
#endif

        case CMD_BDAT:
        case CMD_DATA:
            if ((smtp_ssn->state_flags & SMTP_FLAG_GOT_RCPT_CMD) ||
                smtp_ssn->state == STATE_UNKNOWN)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Set to data state.\n"););

                smtp_ssn->state = STATE_DATA;
                smtp_ssn->state_flags &= ~(SMTP_FLAG_GOT_MAIL_CMD | SMTP_FLAG_GOT_RCPT_CMD);
            }
            else
            {
                 DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Didn't get MAIL -> RCPT command sequence - "
                                                     "stay in command state.\n"););
            }

            break;

        case CMD_STARTTLS:
            /* if reassembled we flush after seeing a 220 so this should be the last
             * command in reassembled packet and if not reassembled it should be the
             * last line in the packet as you can't pipeline the tls hello */
            if (eol == end)
                smtp_ssn->state = STATE_TLS_CLIENT_PEND;

            break;
        
        case CMD_X_LINK2STATE: 
            if (smtp_eval_config->alert_xlink2state)
                ParseXLink2State(p, ptr + smtp_search_info.index);

            break;
            
        default:
            break;
    }

    /* Since we found a command, if state is still unknown,
     * set to command state */
    if (smtp_ssn->state == STATE_UNKNOWN)
        smtp_ssn->state = STATE_COMMAND;

    /* normalize command line */
    if (smtp_eval_config->normalize == NORMALIZE_ALL ||
        smtp_eval_config->cmd_config[smtp_search_info.id].normalize)
    {
        ret = SMTP_NormalizeCmd(p, ptr, eolm, eol);
        if (ret == -1)
            return NULL;
    }                        
    else if (smtp_normalizing) /* Already normalizing */
    {
        ret = SMTP_CopyToAltBuffer(p, ptr, eol - ptr);
        if (ret == -1)
            return NULL;
    }

    return eol;
}


static const uint8_t * SMTP_HandleData(SFSnortPacket *p, const uint8_t *ptr, const uint8_t *end)
{
    const uint8_t *data_end_marker = NULL;
    const uint8_t *data_end = NULL;
    int data_end_found;
    int ret;

    /* if we've just entered the data state, check for a dot + end of line
     * if found, no data */
    if ((smtp_ssn->data_state == STATE_DATA_INIT) ||
        (smtp_ssn->data_state == STATE_DATA_UNKNOWN))
    {
        if ((ptr < end) && (*ptr == '.'))
        {
            const uint8_t *eol = NULL;
            const uint8_t *eolm = NULL;

            SMTP_GetEOL(ptr, end, &eol, &eolm);

            /* this means we got a real end of line and not just end of payload 
             * and that the dot is only char on line */
            if ((eolm != end) && (eolm == (ptr + 1)))
            {
                /* if we're normalizing and not ignoring data copy data end marker
                 * and dot to alt buffer */
                if (!smtp_eval_config->ignore_data && smtp_normalizing)
                {
                    ret = SMTP_CopyToAltBuffer(p, ptr, eol - ptr);
                    if (ret == -1)
                        return NULL;
                }

                SMTP_ResetState();

                return eol;
            }
        }

        if (smtp_ssn->data_state == STATE_DATA_INIT)
            smtp_ssn->data_state = STATE_DATA_HEADER;

        /* XXX A line starting with a '.' that isn't followed by a '.' is
         * deleted (RFC 821 - 4.5.2.  TRANSPARENCY).  If data starts with
         * '. text', i.e a dot followed by white space then text, some
         * servers consider it data header and some data body.
         * Postfix and Qmail will consider the start of data:
         * . text\r\n
         * .  text\r\n
         * to be part of the header and the effect will be that of a 
         * folded line with the '.' deleted.  Exchange will put the same
         * in the body which seems more reasonable. */
    }

    /* get end of data body
     * TODO check last bytes of previous packet to see if we had a partial
     * end of data */
    smtp_current_search = &smtp_data_end_search[0];
    data_end_found = _dpd.searchAPI->search_instance_find
        (smtp_data_search_mpse, (const char *)ptr, end - ptr,
         0, SMTP_SearchStrFound);

    if (data_end_found > 0)
    {
        data_end_marker = ptr + smtp_search_info.index;
        data_end = data_end_marker + smtp_search_info.length;
    }
    else
    {
        data_end_marker = data_end = end;
    }

    if ((smtp_ssn->data_state == STATE_DATA_HEADER) ||
        (smtp_ssn->data_state == STATE_DATA_UNKNOWN))
    {
#ifdef DEBUG
        if (smtp_ssn->data_state == STATE_DATA_HEADER)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "DATA HEADER STATE ~~~~~~~~~~~~~~~~~~~~~~\n"););
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "DATA UNKNOWN STATE ~~~~~~~~~~~~~~~~~~~~~\n"););
        }
#endif

        ptr = SMTP_HandleHeader(p, ptr, data_end_marker);
        _dpd.setFileDataPtr(ptr, 0);
        if (ptr == NULL)
            return NULL;
    }

    /* if we're ignoring data and not already normalizing, copy everything
     * up to here into alt buffer so detection engine doesn't have
     * to look at the data; otherwise, if we're normalizing and not
     * ignoring data, copy all of the data into the alt buffer */
    if (smtp_eval_config->ignore_data && !smtp_normalizing)
    {
        ret = SMTP_CopyToAltBuffer(p, p->payload, ptr - p->payload);
        if (ret == -1)
            return NULL;
    }
    else if (!smtp_eval_config->ignore_data && smtp_normalizing)
    {
        ret = SMTP_CopyToAltBuffer(p, ptr, data_end - ptr);
        if (ret == -1)
            return NULL;
    }

    /* now we shouldn't have to worry about copying any data to the alt buffer
     * only mime headers if we find them and only if we're ignoring data */

    while ((ptr != NULL) && (ptr < data_end_marker))
    {
        /* multiple base64 attachments in one single packet.
         * Pipeline the base64 decoded data.*/
        if ( smtp_ssn->state_flags & SMTP_FLAG_MULTIPLE_BASE64)
        {
            _dpd.setFileDataPtr(smtp_ssn->decode_state->decodeBuf, smtp_ssn->decode_state->decoded_bytes);
            _dpd.detect(p);
            smtp_ssn->state_flags &= ~SMTP_FLAG_MULTIPLE_BASE64;
            smtp_ssn->decode_state->decode_present = 0;
            smtp_ssn->decode_state->decoded_bytes = 0;
            ClearPrevEncode(smtp_ssn->decode_state);
            p->flags |=FLAG_ALLOW_MULTIPLE_DETECT;
        }
        switch (smtp_ssn->data_state)
        {
            case STATE_MIME_HEADER:
                DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "MIME HEADER STATE ~~~~~~~~~~~~~~~~~~~~~~\n"););
                ptr = SMTP_HandleHeader(p, ptr, data_end_marker);
                break;
            case STATE_DATA_BODY:
                DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "DATA BODY STATE ~~~~~~~~~~~~~~~~~~~~~~~~\n"););
                ptr = SMTP_HandleDataBody(p, ptr, data_end_marker);
                break;
        }
    }

    /* We have either reached the end of MIME header or end of base64 encoded data*/
    
    if(smtp_ssn->decode_state)
    {
        _dpd.setFileDataPtr(smtp_ssn->decode_state->decodeBuf, smtp_ssn->decode_state->decoded_bytes);
        smtp_ssn->decode_state->decode_present = 0;
        smtp_ssn->decode_state->decoded_bytes = 0;
    }

    /* if we got the data end reset state, otherwise we're probably still in the data
     * to expect more data in next packet */
    if (data_end_marker != end)
    {
        SMTP_ResetState();
    }

    return data_end;
}


/*
 * Handle Headers - Data or Mime
 *
 * @param   packet  standard Packet structure
 *
 * @param   i       index into p->payload buffer to start looking at data
 *
 * @return  i       index into p->payload where we stopped looking at data
 */
static const uint8_t * SMTP_HandleHeader(SFSnortPacket *p, const uint8_t *ptr,
                                          const uint8_t *data_end_marker)
{
    const uint8_t *eol;
    const uint8_t *eolm;
    const uint8_t *colon;
    const uint8_t *content_type_ptr = NULL;
    const uint8_t *cont_trans_enc = NULL;
    int header_line_len;
    int header_found;
    int ret;
    const uint8_t *start_hdr;
    int header_name_len;

    start_hdr = ptr;

    /* if we got a content-type in a previous packet and are
     * folding, the boundary still needs to be checked for */
    if (smtp_ssn->state_flags & SMTP_FLAG_IN_CONTENT_TYPE)
        content_type_ptr = ptr;
    
    if (smtp_ssn->state_flags & SMTP_FLAG_IN_CONT_TRANS_ENC)
        cont_trans_enc = ptr;

    while (ptr < data_end_marker)
    {
        SMTP_GetEOL(ptr, data_end_marker, &eol, &eolm);

        /* got a line with only end of line marker should signify end of header */
        if (eolm == ptr)
        {
            /* reset global header state values */
            smtp_ssn->state_flags &=
                ~(SMTP_FLAG_FOLDING | SMTP_FLAG_IN_CONTENT_TYPE | SMTP_FLAG_DATA_HEADER_CONT
                        | SMTP_FLAG_IN_CONT_TRANS_ENC );

            smtp_ssn->data_state = STATE_DATA_BODY;

            /* if no headers, treat as data */
            if (ptr == start_hdr)
                return eolm;
            else
                return eol;
        }

        /* if we're not folding, see if we should interpret line as a data line 
         * instead of a header line */
        if (!(smtp_ssn->state_flags & (SMTP_FLAG_FOLDING | SMTP_FLAG_DATA_HEADER_CONT)))
        {
            char got_non_printable_in_header_name = 0;

            /* if we're not folding and the first char is a space or
             * colon, it's not a header */
            if (isspace((int)*ptr) || *ptr == ':')
            {
                smtp_ssn->data_state = STATE_DATA_BODY;
                return ptr;
            }

            /* look for header field colon - if we're not folding then we need
             * to find a header which will be all printables (except colon) 
             * followed by a colon */
            colon = ptr;
            while ((colon < eolm) && (*colon != ':'))
            {
                if (((int)*colon < 33) || ((int)*colon > 126))
                    got_non_printable_in_header_name = 1;

                colon++;
            }

            /* Check for Exim 4.32 exploit where number of chars before colon is greater than 64 */
            header_name_len = colon - ptr;
            if ((smtp_ssn->data_state != STATE_DATA_UNKNOWN) &&
                (colon < eolm) && (header_name_len > MAX_HEADER_NAME_LEN))
            {
                SMTP_GenerateAlert(SMTP_HEADER_NAME_OVERFLOW, "%s: %d chars before colon",
                                   SMTP_HEADER_NAME_OVERFLOW_STR, header_name_len);
            }

            /* If the end on line marker and end of line are the same, assume
             * header was truncated, so stay in data header state */
            if ((eolm != eol) &&
                ((colon == eolm) || got_non_printable_in_header_name))
            {
                /* no colon or got spaces in header name (won't be interpreted as a header)
                 * assume we're in the body */
                smtp_ssn->state_flags &=
                    ~(SMTP_FLAG_FOLDING | SMTP_FLAG_IN_CONTENT_TYPE | SMTP_FLAG_DATA_HEADER_CONT 
                            |SMTP_FLAG_IN_CONT_TRANS_ENC);

                smtp_ssn->data_state = STATE_DATA_BODY;

                return ptr;
            }

            smtp_current_search = &smtp_hdr_search[0];
            header_found = _dpd.searchAPI->search_instance_find
                (smtp_hdr_search_mpse, (const char *)ptr,
                 eolm - ptr, 1, SMTP_SearchStrFound);

            /* Headers must start at beginning of line */
            if ((header_found > 0) && (smtp_search_info.index == 0))
            {
                switch (smtp_search_info.id)
                {
                    case HDR_CONTENT_TYPE:
                        /* for now we're just looking for the boundary in the data
                         * header section */
                        if (smtp_ssn->data_state != STATE_MIME_HEADER)
                        {
                            content_type_ptr = ptr + smtp_search_info.length;
                            smtp_ssn->state_flags |= SMTP_FLAG_IN_CONTENT_TYPE;
                        }

                        break;
                    case HDR_CONT_TRANS_ENC:
                        cont_trans_enc = ptr + smtp_search_info.length;
                        smtp_ssn->state_flags |= SMTP_FLAG_IN_CONT_TRANS_ENC;
                        break;

                        
                    default:
                        break;
                }
            }
        }
        else
        {
            smtp_ssn->state_flags &= ~SMTP_FLAG_DATA_HEADER_CONT;
        }
        
        /* get length of header line */
        header_line_len = eol - ptr;

        if ((smtp_eval_config->max_header_line_len != 0) &&
            (header_line_len > smtp_eval_config->max_header_line_len))
        {
            if (smtp_ssn->data_state != STATE_DATA_UNKNOWN)
            {
                SMTP_GenerateAlert(SMTP_DATA_HDR_OVERFLOW, "%s: %d chars",
                                   SMTP_DATA_HDR_OVERFLOW_STR, header_line_len);
            }
            else
            {
                /* assume we guessed wrong and are in the body */
                smtp_ssn->data_state = STATE_DATA_BODY;
                smtp_ssn->state_flags &=
                    ~(SMTP_FLAG_FOLDING | SMTP_FLAG_IN_CONTENT_TYPE | SMTP_FLAG_DATA_HEADER_CONT
                            | SMTP_FLAG_IN_CONT_TRANS_ENC);
                return ptr;
            }
        }

        /* XXX Does VRT want data headers normalized?
         * currently the code does not normalize headers */
        if (smtp_normalizing)
        {
            ret = SMTP_CopyToAltBuffer(p, ptr, eol - ptr);
            if (ret == -1)
                return NULL;
        }

        /* check for folding 
         * if char on next line is a space and not \n or \r\n, we are folding */
        if ((eol < data_end_marker) && isspace((int)eol[0]) && (eol[0] != '\n'))
        {
            if ((eol < (data_end_marker - 1)) && (eol[0] != '\r') && (eol[1] != '\n'))
            {
                smtp_ssn->state_flags |= SMTP_FLAG_FOLDING;
            }
            else
            {
                smtp_ssn->state_flags &= ~SMTP_FLAG_FOLDING;
            }
        }
        else if (eol != eolm)
        {
            smtp_ssn->state_flags &= ~SMTP_FLAG_FOLDING;
        }

        /* check if we're in a content-type header and not folding. if so we have the whole
         * header line/lines for content-type - see if we got a multipart with boundary
         * we don't check each folded line, but wait until we have the complete header
         * because boundary=BOUNDARY can be split across mulitple folded lines before
         * or after the '=' */
        if ((smtp_ssn->state_flags &
             (SMTP_FLAG_IN_CONTENT_TYPE | SMTP_FLAG_FOLDING)) == SMTP_FLAG_IN_CONTENT_TYPE)
        {
            /* we got the full content-type header - look for boundary string */
            ret = SMTP_GetBoundary((const char *)content_type_ptr, eolm - content_type_ptr);
            if (ret != -1)
            {
                ret = SMTP_BoundarySearchInit();
                if (ret != -1)
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Got mime boundary: %s\n",
                                                         smtp_ssn->mime_boundary.boundary););

                    smtp_ssn->state_flags |= SMTP_FLAG_GOT_BOUNDARY;
                }
            }

            smtp_ssn->state_flags &= ~SMTP_FLAG_IN_CONTENT_TYPE;
            content_type_ptr = NULL;
        }
        else if ((smtp_ssn->state_flags &
                (SMTP_FLAG_IN_CONT_TRANS_ENC | SMTP_FLAG_FOLDING)) == SMTP_FLAG_IN_CONT_TRANS_ENC)
        {
            /* Check for Content-Transfer-Encoding : base64 */
            if( smtp_eval_config->enable_mime_decoding )
            {
                ret = SMTP_IsBase64Data((const char *)cont_trans_enc, eolm - cont_trans_enc );
                if (ret != -1)
                {
                    /* Check to see if there is already an base64 decoded data*/
                    smtp_ssn->state_flags |= SMTP_FLAG_BASE64_DATA;
                    if( smtp_ssn->decode_state && smtp_ssn->decode_state->decode_present )
                        smtp_ssn->state_flags |= SMTP_FLAG_MULTIPLE_BASE64;
                }
            }
            smtp_ssn->state_flags &= ~SMTP_FLAG_IN_CONT_TRANS_ENC;

            cont_trans_enc = NULL;
        }

        /* if state was unknown, at this point assume we know */
        if (smtp_ssn->data_state == STATE_DATA_UNKNOWN)
            smtp_ssn->data_state = STATE_DATA_HEADER;

        ptr = eol;

        if (ptr == data_end_marker)
            smtp_ssn->state_flags |= SMTP_FLAG_DATA_HEADER_CONT;
    }

    return ptr;
}


/*
 * Handle DATA_BODY state
 *
 * @param   packet  standard Packet structure
 *
 * @param   i       index into p->payload buffer to start looking at data
 *
 * @return  i       index into p->payload where we stopped looking at data
 */
static const uint8_t * SMTP_HandleDataBody(SFSnortPacket *p, const uint8_t *ptr,
                                            const uint8_t *data_end_marker)
{
    int boundary_found = 0;
    const uint8_t *boundary_ptr = NULL;
    const uint8_t *base64_start = NULL;
    const uint8_t *base64_end = NULL;

    if ( smtp_ssn->state_flags & SMTP_FLAG_BASE64_DATA )
        base64_start = ptr;
    /* look for boundary */
    if (smtp_ssn->state_flags & SMTP_FLAG_GOT_BOUNDARY)
    {
        boundary_found = _dpd.searchAPI->search_instance_find
            (smtp_ssn->mime_boundary.boundary_search, (const char *)ptr,
             data_end_marker - ptr, 0, SMTP_BoundaryStrFound);

        if (boundary_found > 0)
        {
            boundary_ptr = ptr + smtp_search_info.index;

            /* should start at beginning of line */
            if ((boundary_ptr == ptr) || (*(boundary_ptr - 1) == '\n'))
            {
                const uint8_t *eol;
                const uint8_t *eolm;
                const uint8_t *tmp;

                if (smtp_ssn->state_flags & SMTP_FLAG_BASE64_DATA )
                {
                    base64_end = boundary_ptr-1;
                    smtp_ssn->state_flags &= ~SMTP_FLAG_BASE64_DATA;
                    SMTP_Base64Decode( base64_start, base64_end);
                }


                /* Check for end boundary */
                tmp = boundary_ptr + smtp_search_info.length;
                if (((tmp + 1) < data_end_marker) && (tmp[0] == '-') && (tmp[1] == '-'))
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Mime boundary end found: %s--\n",
                                            (char *)smtp_ssn->mime_boundary.boundary););

                    /* no more MIME */
                    smtp_ssn->state_flags &= ~SMTP_FLAG_GOT_BOUNDARY;

                    /* free boundary search */
                    _dpd.searchAPI->search_instance_free(smtp_ssn->mime_boundary.boundary_search);
                    smtp_ssn->mime_boundary.boundary_search = NULL;
                }
                else
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Mime boundary found: %s\n",
                                            (char *)smtp_ssn->mime_boundary.boundary););

                    smtp_ssn->data_state = STATE_MIME_HEADER;
                }

                /* get end of line - there could be spaces after boundary before eol */
                SMTP_GetEOL(boundary_ptr + smtp_search_info.length, data_end_marker, &eol, &eolm);

                return eol;
            }
        }
    }

    if ( smtp_ssn->state_flags & SMTP_FLAG_BASE64_DATA )
    {
        base64_end = data_end_marker;
        SMTP_Base64Decode( base64_start, base64_end);
    }
        
    return data_end_marker;
}


/*
 * Process client packet
 *
 * @param   packet  standard Packet structure
 *
 * @return  none
 */
static void SMTP_ProcessClientPacket(SFSnortPacket *p)
{
    const uint8_t *ptr = p->payload;
    const uint8_t *end = p->payload + p->payload_size;

    if (smtp_ssn->state == STATE_CONNECT)
        smtp_ssn->state = STATE_COMMAND;

    while ((ptr != NULL) && (ptr < end))
    {
        switch (smtp_ssn->state)
        {
            case STATE_COMMAND:
                DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "COMMAND STATE ~~~~~~~~~~~~~~~~~~~~~~~~~~\n"););
                ptr = SMTP_HandleCommand(p, ptr, end);
                break;
            case STATE_DATA:
                DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "DATA STATE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"););
                ptr = SMTP_HandleData(p, ptr, end);
                break;
            case STATE_UNKNOWN:
                DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "UNKNOWN STATE ~~~~~~~~~~~~~~~~~~~~~~~~~~\n"););
                /* If state is unknown try command state to see if we can
                 * regain our bearings */
                ptr = SMTP_HandleCommand(p, ptr, end);
                break;
            default:
                DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Bad SMTP state\n"););
                return;
        }
    }

#ifdef DEBUG
    if (smtp_normalizing)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Normalized payload\n%s\n", SMTP_PrintBuffer(p)););
    }
#endif
}


/* very simplistic - just enough to say this is binary data - the rules will make a final 
 * judgement.  Should maybe add an option to the smtp configuration to enable the 
 * continuing of command inspection like ftptelnet. */
static int SMTP_IsTlsClientHello(const uint8_t *ptr, const uint8_t *end)
{
    /* at least 3 bytes of data - see below */
    if ((end - ptr) < 3)
        return 0;

    if ((ptr[0] == 0x16) && (ptr[1] == 0x03))
    {
        /* TLS v1 or SSLv3 */
        return 1;
    }
    else if ((ptr[2] == 0x01) || (ptr[3] == 0x01))
    {
        /* SSLv2 */
        return 1;
    }

    return 0;
}

/* this may at least tell us whether the server accepted the client hello by the presence
 * of binary data */
static int SMTP_IsTlsServerHello(const uint8_t *ptr, const uint8_t *end)
{
    /* at least 3 bytes of data - see below */
    if ((end - ptr) < 3)
        return 0;

    if ((ptr[0] == 0x16) && (ptr[1] == 0x03))
    {
        /* TLS v1 or SSLv3 */
        return 1;
    }
    else if (ptr[2] == 0x04)
    {
        /* SSLv2 */
        return 1;
    }

    return 0;
}


/*
 * Process server packet
 *
 * @param   packet  standard Packet structure
 *
 * @return  do_flush
 * @retval  1           flush queued packets on client side
 * @retval  0           do not flush queued packets on client side
 */
static int SMTP_ProcessServerPacket(SFSnortPacket *p)
{
    int resp_found;
    const uint8_t *ptr;
    const uint8_t *end;
    const uint8_t *eolm;
    const uint8_t *eol;
    int do_flush = 0; 
    int resp_line_len;
#ifdef DEBUG
    const uint8_t *dash;
#endif

    ptr = p->payload;
    end = p->payload + p->payload_size;

    if (smtp_ssn->state == STATE_TLS_SERVER_PEND)
    {
        if (SMTP_IsTlsServerHello(ptr, end))
        {
            smtp_ssn->state = STATE_TLS_DATA;
        }
        else
        {
            /* revert back to command state - assume server didn't accept STARTTLS */
            smtp_ssn->state = STATE_COMMAND;
        }
    }
        
    if (smtp_ssn->state == STATE_TLS_DATA)
    {
        /* Ignore data */
        if (smtp_eval_config->ignore_tls_data)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Ignoring Server TLS encrypted data\n"););
            SetAltBuffer(p, 0);
        }

        return 0;
    }
    
    while (ptr < end)
    {
        SMTP_GetEOL(ptr, end, &eol, &eolm);

        resp_line_len = eol - ptr;

        /* Check for response code */
        smtp_current_search = &smtp_resp_search[0];
        resp_found = _dpd.searchAPI->search_instance_find
            (smtp_resp_search_mpse, (const char *)ptr,
             resp_line_len, 1, SMTP_SearchStrFound);
        
        if (resp_found > 0)
        {
            switch (smtp_search_info.id)
            {
                case RESP_220:
                    /* This is either an initial server response or a STARTTLS response
                     * flush the client side. if we've already seen STARTTLS, no need
                     * to flush */
                    if (smtp_ssn->state == STATE_CONNECT)
                        smtp_ssn->state = STATE_COMMAND;
                    else if (smtp_ssn->state != STATE_TLS_CLIENT_PEND)
                        do_flush = 1;

                    break;

                case RESP_354:
                    do_flush = 1;

                    break;

                default:
                    break;
            }

#ifdef DEBUG
            dash = ptr + smtp_search_info.index + smtp_search_info.length;

            /* only add response if not a dash after response code */
            if ((dash == eolm) || ((dash < eolm) && (*dash != '-')))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Server sent %s response\n", 
                                                    smtp_resps[smtp_search_info.id].name););
            }
#endif
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Server response not found - see if it's SSL data\n"););

            if ((smtp_ssn->session_flags & SMTP_FLAG_CHECK_SSL) &&
                (SMTP_IsSSL(ptr, end - ptr, p->flags)))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Server response is an SSL packet\n"););

                smtp_ssn->state = STATE_TLS_DATA;

                /* Ignore data */
                if (smtp_eval_config->ignore_tls_data)
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Ignoring Server TLS encrypted data\n"););
                    SetAltBuffer(p, 0);
                }

                return 0;
            }
            else if (smtp_ssn->session_flags & SMTP_FLAG_CHECK_SSL)
            {
                smtp_ssn->session_flags &= ~SMTP_FLAG_CHECK_SSL;
            }
        }

        if ((smtp_eval_config->max_response_line_len != 0) &&
            (resp_line_len > smtp_eval_config->max_response_line_len))
        {
            SMTP_GenerateAlert(SMTP_RESPONSE_OVERFLOW, "%s: %d chars",
                               SMTP_RESPONSE_OVERFLOW_STR, resp_line_len);
        }
       
        ptr = eol;
    }

    return do_flush;
}

static int SMTP_IsSSL(const uint8_t *ptr, int len, int pkt_flags)
{
    uint32_t ssl_flags = SSL_decode(ptr, len, pkt_flags);

    if ((ssl_flags != SSL_ARG_ERROR_FLAG) &&
        !(ssl_flags & SMTP_SSL_ERROR_FLAGS))
    {
        return 1;
    }

    return 0;
}

/* For Target based
 * If a protocol for the session is already identified and not one SMTP is
 * interested in, SMTP should leave it alone and return without processing.
 * If a protocol for the session is already identified and is one that SMTP is
 * interested in, decode it.
 * If the protocol for the session is not already identified and the preprocessor
 * is configured to detect on one of the packet ports, detect.
 * Returns 0 if we should not inspect
 *         1 if we should continue to inspect
 */
static int SMTP_Inspect(SFSnortPacket *p)
{
#ifdef TARGET_BASED
    /* SMTP could be configured to be stateless.  If stream isn't configured, assume app id
     * will never be set and just base inspection on configuration */
    if (p->stream_session_ptr == NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "SMTP: Target-based: No stream session.\n"););

        if ((SMTP_IsServer(p->src_port) && (p->flags & FLAG_FROM_SERVER)) || 
            (SMTP_IsServer(p->dst_port) && (p->flags & FLAG_FROM_CLIENT)))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "SMTP: Target-based: Configured for this "
                                    "traffic, so let's inspect.\n"););
            return 1;
        }
    }
    else
    {
        int16_t app_id = _dpd.streamAPI->get_application_protocol_id(p->stream_session_ptr);

        if (app_id != 0)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "SMTP: Target-based: App id: %u.\n", app_id););

            if (app_id == smtp_proto_id)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "SMTP: Target-based: App id is "
                                        "set to \"%s\".\n", SMTP_PROTO_REF_STR););
                return 1;
            }
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "SMTP: Target-based: Unknown protocol for "
                                    "this session.  See if we're configured.\n"););

            if ((SMTP_IsServer(p->src_port) && (p->flags & FLAG_FROM_SERVER)) || 
                (SMTP_IsServer(p->dst_port) && (p->flags & FLAG_FROM_CLIENT)))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "SMTP: Target-based: SMTP port is configured."););
                return 1;
            }
        }
    }

    DEBUG_WRAP(DebugMessage(DEBUG_SMTP,"SMTP: Target-based: Not inspecting ...\n"););

#else
    /* Make sure it's traffic we're interested in */
    if ((SMTP_IsServer(p->src_port) && (p->flags & FLAG_FROM_SERVER)) || 
        (SMTP_IsServer(p->dst_port) && (p->flags & FLAG_FROM_CLIENT)))
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
void SnortSMTP(SFSnortPacket *p)
{
    int detected = 0;
    int pkt_dir;
    tSfPolicyId policy_id = _dpd.getRuntimePolicy();

    PROFILE_VARS;

    smtp_eval_config = (SMTPConfig *)sfPolicyUserDataGetCurrent(smtp_config);

    smtp_ssn = (SMTP *)_dpd.streamAPI->get_application_data(p->stream_session_ptr, PP_SMTP);
    if (smtp_ssn != NULL)
        smtp_eval_config = (SMTPConfig *)sfPolicyUserDataGet(smtp_ssn->config, smtp_ssn->policy_id);

    if (smtp_eval_config == NULL)
        return;

    if (smtp_ssn == NULL)
    {
        if (!SMTP_Inspect(p))
            return;

        smtp_ssn = SMTP_GetNewSession(p, policy_id);
        if (smtp_ssn == NULL)
            return;
    }

    pkt_dir = SMTP_Setup(p, smtp_ssn);

    /* reset normalization stuff */
    smtp_normalizing = 0;
    ResetAltBuffer(p);
    p->normalized_payload_size = 0;

    if (pkt_dir == SMTP_PKT_FROM_SERVER)
    {
        int do_flush = 0;

        DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "SMTP server packet\n"););

        /* Process as a server packet */
        do_flush = SMTP_ProcessServerPacket(p);

        if (do_flush)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Flushing stream\n"););
            _dpd.streamAPI->response_flush_stream(p);
        }
    }
    else
    {
#ifdef DEBUG
        if (pkt_dir == SMTP_PKT_FROM_CLIENT)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "SMTP client packet\n"););
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "SMTP packet NOT from client or server! "
                                    "Processing as a client packet\n"););
        }
#endif

        /* This packet should be a tls client hello */
        if (smtp_ssn->state == STATE_TLS_CLIENT_PEND)
        {
            if (SMTP_IsTlsClientHello(p->payload, p->payload + p->payload_size))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_SMTP,
                                        "TLS DATA STATE ~~~~~~~~~~~~~~~~~~~~~~~~~\n"););

                smtp_ssn->state = STATE_TLS_SERVER_PEND;
            }
            else
            {
                /* reset state - server may have rejected STARTTLS command */
                smtp_ssn->state = STATE_COMMAND;
            }
        }

        if ((smtp_ssn->state == STATE_TLS_DATA) || (smtp_ssn->state == STATE_TLS_SERVER_PEND))
        {
            /* if we're ignoring tls data, set a zero length alt buffer */
            if (smtp_eval_config->ignore_tls_data)
            {
                SetAltBuffer(p, 0);
            }
        }
        else
        {
            if (p->flags & FLAG_STREAM_INSERT)
            {
                /* Packet will be rebuilt, so wait for it */
                DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Client packet will be reassembled\n"));
                return;
            }
            else if (smtp_ssn->reassembling && !(p->flags & FLAG_REBUILT_STREAM))
            {
                /* If this isn't a reassembled packet and didn't get 
                 * inserted into reassembly buffer, there could be a
                 * problem.  If we miss syn or syn-ack that had window
                 * scaling this packet might not have gotten inserted
                 * into reassembly buffer because it fell outside of 
                 * window, because we aren't scaling it */
                smtp_ssn->session_flags |= SMTP_FLAG_GOT_NON_REBUILT;
                smtp_ssn->state = STATE_UNKNOWN;
            }
            else if (smtp_ssn->reassembling && (smtp_ssn->session_flags & SMTP_FLAG_GOT_NON_REBUILT))
            {
                /* This is a rebuilt packet.  If we got previous packets
                 * that were not rebuilt, state is going to be messed up
                 * so set state to unknown. It's likely this was the
                 * beginning of the conversation so reset state */
                DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Got non-rebuilt packets before "
                                        "this rebuilt packet\n"););

                smtp_ssn->state = STATE_UNKNOWN;
                smtp_ssn->session_flags &= ~SMTP_FLAG_GOT_NON_REBUILT;
            }

#ifdef DEBUG
            /* Interesting to see how often packets are rebuilt */
            DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Payload: %s\n%s\n",
                                    (p->flags & FLAG_REBUILT_STREAM) ?
                                    "reassembled" : "not reassembled",
                                    SMTP_PrintBuffer(p)););
#endif

            SMTP_ProcessClientPacket(p);
        }
    }

    PREPROC_PROFILE_START(smtpDetectPerfStats);

    detected = _dpd.detect(p);

#ifdef PERF_PROFILING
    smtpDetectCalled = 1;
#endif

    PREPROC_PROFILE_END(smtpDetectPerfStats);

    /* Turn off detection since we've already done it. */
    SMTP_DisableDetect(p);
     
    if (detected)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "SMTP vulnerability detected\n"););
    }
}

static void SMTP_DisableDetect(SFSnortPacket *p)
{
    _dpd.disableAllDetect(p);

    _dpd.setPreprocBit(p, PP_SFPORTSCAN);
    _dpd.setPreprocBit(p, PP_PERFMONITOR);
    _dpd.setPreprocBit(p, PP_STREAM5);
    _dpd.setPreprocBit(p, PP_SDF);
}


