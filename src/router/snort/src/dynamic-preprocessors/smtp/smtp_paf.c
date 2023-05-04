/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2013 Sourcefire, Inc.
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
#include <sys/types.h>
#include "sf_types.h"
#include "smtp_paf.h"
#include "spp_smtp.h"
#include "sf_dynamic_preprocessor.h"
#include "stream_api.h"
#include "snort_smtp.h"
#include "file_api.h"
#include "smtp_log.h"

static uint8_t smtp_paf_id = 0;

extern tSfPolicyUserContextId smtp_config;

/* State tracker for MIME PAF */
typedef enum _SmtpDataCMD
{
    SMTP_PAF_BDAT_CMD,
    SMTP_PAF_DATA_CMD,
    SMTP_PAF_XEXCH50_CMD,
    SMTP_PAF_STRARTTLS_CMD,
    SMTP_PAF_AUTH_CMD
} SmtpDataCMD;

typedef struct _PAFToken
{
    char *name;
    int   name_len;
    int   search_id;
    bool  has_length;
} SmtpPAFToken;

const SmtpPAFToken smtp_paf_tokens[] =
{
        {"BDAT",         4, SMTP_PAF_BDAT_CMD, true},
        {"DATA",         4, SMTP_PAF_DATA_CMD, true},
        {"XEXCH50",      7, SMTP_PAF_XEXCH50_CMD, true},
        {"STRARTTLS",    9, SMTP_PAF_STRARTTLS_CMD, false},
        {"AUTH",         4, SMTP_PAF_AUTH_CMD, false},
        {NULL,           0, 0, false}
};

/* State tracker for data command */
typedef enum _SmtpPafCmdState
{
    SMTP_PAF_CMD_UNKNOWN,
    SMTP_PAF_CMD_START,
    SMTP_PAF_CMD_DETECT,
    SMTP_PAF_CMD_DATA_LENGTH_STATE,
    SMTP_PAF_CMD_DATA_END_STATE
} SmtpPafCmdState;

/* State tracker for SMTP PAF */
typedef enum _SmtpPafState
{
    SMTP_PAF_CMD_STATE,
    SMTP_PAF_DATA_STATE
} SmtpPafState;

typedef struct _SmtpCmdSearchInfo
{
    SmtpPafCmdState cmd_state;
    int   search_id;
    char *search_state;
} SmtpCmdSearchInfo;

/* State tracker for SMTP PAF */
typedef struct _SmtpPafData
{
    DataEndState data_end_state;
    uint32_t length;
    SmtpPafState smtp_state;
    bool end_of_data;
    bool alert_generated;
    SmtpCmdSearchInfo cmd_info;
    MimeDataPafInfo data_info;
} SmtpPafData;

/* State tracker for SMTP PAF */
typedef enum _SmtpPafDataLenStatus
{
    SMTP_PAF_LENGTH_INVALID,
    SMTP_PAF_LENGTH_CONTINUE,
    SMTP_PAF_LENGTH_DONE
} SmtpPafDataLenStatus;

/* Process responses from server, flushed at EOL*/
static inline PAF_Status smtp_paf_server(SmtpPafData *pfdata,
        const uint8_t* data, uint32_t len, uint32_t* fp)
{
    const char *pch;

    pfdata->smtp_state = SMTP_PAF_CMD_STATE;
    pch = memchr (data, '\n', len);

    if (pch != NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Find end of line!\n"););
        *fp = (uint32_t)(pch - (const char*)data) + 1;
        return PAF_FLUSH;
    }
    return PAF_SEARCH;
}

/* Initialize command search based on first byte of command*/
static inline char* init_cmd_search(SmtpCmdSearchInfo *search_info,  uint8_t ch)
{

    /* Use the first byte to choose data command)*/
    switch (ch)
    {
    case 'b':
    case 'B':
        search_info->search_state = &smtp_paf_tokens[SMTP_PAF_BDAT_CMD].name[1];
        search_info->search_id = SMTP_PAF_BDAT_CMD;
        break;
    case 'd':
    case 'D':
        search_info->search_state = &smtp_paf_tokens[SMTP_PAF_DATA_CMD].name[1];
        search_info->search_id = SMTP_PAF_DATA_CMD;
        break;
    case 'x':
    case 'X':
        search_info->search_state = &smtp_paf_tokens[SMTP_PAF_XEXCH50_CMD].name[1];
        search_info->search_id = SMTP_PAF_XEXCH50_CMD;
        break;
    case 's':
    case 'S':
        search_info->search_state = &smtp_paf_tokens[SMTP_PAF_STRARTTLS_CMD].name[1];
        search_info->search_id = SMTP_PAF_STRARTTLS_CMD;
        break;
    case 'a':
    case 'A':
        search_info->search_state = &smtp_paf_tokens[SMTP_PAF_AUTH_CMD].name[1];
        search_info->search_id = SMTP_PAF_AUTH_CMD;
        break;

    default:
        search_info->search_state = NULL;
        break;
    }
    return search_info->search_state;
}

/* Validate whether the command is a data command*/
static inline void validate_command(SmtpCmdSearchInfo *search_info,  uint8_t val)
{
    if (search_info->search_state )
    {
        uint8_t expected = *(search_info->search_state);

        if (toupper(val) == toupper(expected))
        {
            search_info->search_state++;
            /* Found data command, change to SMTP_PAF_CMD_DATA_LENGTH_STATE */
            if (*(search_info->search_state) == '\0')
            {
                search_info->search_state = NULL;
                search_info->cmd_state = SMTP_PAF_CMD_DATA_LENGTH_STATE;
                return;
            }
        }
        else
        {
            search_info->search_state = NULL;
            search_info->cmd_state = SMTP_PAF_CMD_UNKNOWN;
            return;
        }
    }
}

/* Get the length of data from data command
 */
static SmtpPafDataLenStatus get_length(char c, uint32_t *len )
{
    uint32_t length = *len;

    if (isblank(c))
    {
        if (length)
        {
            *len = length;
            return SMTP_PAF_LENGTH_DONE;
        }
    }
    else if (isdigit(c))
    {
        uint64_t tmp_len = (10 * length)  + (c - '0');
        if (tmp_len < UINT32_MAX)
            length = (uint32_t) tmp_len;
        else
        {
            *len = 0;
            return SMTP_PAF_LENGTH_INVALID;
        }
    }
    else
    {
        *len = 0;
        return SMTP_PAF_LENGTH_INVALID;
    }

    *len = length;
    return SMTP_PAF_LENGTH_CONTINUE;
}

/* Reset data states*/
static inline void reset_data_states(SmtpPafData *pfdata)
{
    _dpd.fileAPI->reset_mime_paf_state(&(pfdata->data_info));
    pfdata->length = 0;
}

/* Currently, we support "BDAT", "DATA", "XEXCH50", "STRARTTLS"
 * Each data command should start from offset 0,
 * since previous data have been flushed
 */
static inline bool process_command(SmtpPafData *pfdata,  uint8_t val)
{
    /*State unknown, start cmd search start from EOL, flush on EOL*/
    if (val == '\n')
    {
        if (pfdata->cmd_info.cmd_state == SMTP_PAF_CMD_DATA_END_STATE)
        {
            pfdata->smtp_state = SMTP_PAF_DATA_STATE;
            reset_data_states(pfdata);
            pfdata->end_of_data = false;
        }

        pfdata->cmd_info.cmd_state = SMTP_PAF_CMD_START;
        return 1;
    }

    switch (pfdata->cmd_info.cmd_state)
    {
    case SMTP_PAF_CMD_UNKNOWN:
        break;
    case SMTP_PAF_CMD_START:
        if (init_cmd_search(&(pfdata->cmd_info), val))
            pfdata->cmd_info.cmd_state = SMTP_PAF_CMD_DETECT;
        else
            pfdata->cmd_info.cmd_state = SMTP_PAF_CMD_UNKNOWN;
        break;
    case SMTP_PAF_CMD_DETECT:
        /* Search for data command */
        validate_command(&(pfdata->cmd_info), val);
        break;
    case SMTP_PAF_CMD_DATA_LENGTH_STATE:
        /* Continue finding the data length ...*/
        if (get_length(val, &pfdata->length) != SMTP_PAF_LENGTH_CONTINUE)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Find data length: %d\n",
                    pfdata->length););
            pfdata->cmd_info.cmd_state = SMTP_PAF_CMD_DATA_END_STATE;
        }
        break;
    case SMTP_PAF_CMD_DATA_END_STATE:
        /* Change to Data state at EOL*/
        break;
    default:
        break;
    }

    return 0;
}

/* Flush based on data length*/
static inline bool flush_based_length(SmtpPafData *pfdata)
{
    if (pfdata->length)
    {
        pfdata->length--;
        if (!pfdata->length)
            return true;
    }
    return false;
}

/* Process data length if specified, or end of data marker, flush at the end
 * or
 * Process data boundary and flush each file based on boundary*/
static inline bool process_data(SmtpPafData *pfdata,  uint8_t data)
{

    if (flush_based_length(pfdata)||
            _dpd.fileAPI->check_data_end(&(pfdata->data_end_state), data))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "End of data\n"););
        /*Clean up states*/
        pfdata->smtp_state = SMTP_PAF_CMD_STATE;
        pfdata->end_of_data = true;
        reset_data_states(pfdata);
        return true;
    }

    return _dpd.fileAPI->process_mime_paf_data(&(pfdata->data_info), data);
}

/* Process commands/data from client
 * For command, flush at EOL
 * For data, flush at boundary
 */
static inline PAF_Status smtp_paf_client(SmtpPafData *pfdata,
        const uint8_t* data, uint32_t len, uint32_t* fp)
{
    uint32_t i;
    uint32_t boundary_start = 0;
    tSfPolicyId policy_id = _dpd.getNapRuntimePolicy();
    SMTPConfig* smtp_current_config = (SMTPConfig *)sfPolicyUserDataGet(smtp_config, policy_id);

    DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "From client: %s \n", data););
    for (i = 0; i < len; i++)
    {
        uint8_t ch = data[i];
        switch (pfdata->smtp_state)
        {
        case SMTP_PAF_CMD_STATE:
            if (process_command(pfdata, ch))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Flush command: %s \n", data););
                *fp = i + 1;
                return PAF_FLUSH;
            }
            break;
        case SMTP_PAF_DATA_STATE:
            if (pfdata->cmd_info.search_id == SMTP_PAF_AUTH_CMD)
            {
                if ( smtp_current_config && (smtp_current_config->max_auth_command_line_len != 0) &&
                    (i + pfdata->data_info.boundary_len) > smtp_current_config->max_auth_command_line_len &&
                    !pfdata->alert_generated)
                {
                    if( !smtp_current_config->no_alerts )
                    {
                        _dpd.alertAdd(GENERATOR_SMTP, SMTP_AUTH_COMMAND_OVERFLOW, 1, 0, 3,
                                SMTP_AUTH_COMMAND_OVERFLOW_STR, 0);
                        pfdata->alert_generated = true;
                    }
                }
                if (ch == '\n')
                {
                    pfdata->smtp_state = SMTP_PAF_CMD_STATE;
                    pfdata->end_of_data = true;
                    reset_data_states(pfdata);
                    *fp = i + 1;
                    return PAF_FLUSH;
                }
                else if (i == len-1)
                    pfdata->data_info.boundary_len += len;
            }
            else if (process_data(pfdata, ch))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Flush data!\n"););
                *fp = i + 1;
                return PAF_FLUSH;
            }

            if (pfdata->data_info.boundary_state == MIME_PAF_BOUNDARY_UNKNOWN)
                boundary_start = i;
            break;
        default:
            break;
        }
    }


    if ( scanning_boundary(&pfdata->data_info, boundary_start, fp) )
        return PAF_LIMIT;

    return PAF_SEARCH;
}

/* Main PAF function*/
static PAF_Status smtp_paf_eval(void* ssn, void** ps, const uint8_t* data,
        uint32_t len, uint64_t *flags, uint32_t* fp, uint32_t* fp_eoh)
{
    SmtpPafData *pfdata = *(SmtpPafData **)ps;

    if (pfdata == NULL)
    {
        if (_dpd.fileAPI->check_paf_abort(ssn))
            return PAF_ABORT;

        pfdata = _dpd.snortAlloc(1, sizeof(*pfdata), PP_SMTP, PP_MEM_CATEGORY_SESSION);

        if (pfdata == NULL)
        {
            return PAF_ABORT;
        }

        *ps = pfdata;
        pfdata->data_end_state = PAF_DATA_END_UNKNOWN;
        pfdata->smtp_state = SMTP_PAF_CMD_STATE;
        pfdata->alert_generated = false;
    }

    /*From server side (responses)*/
    if (*flags & FLAG_FROM_SERVER)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "From server!\n"););
        return smtp_paf_server(pfdata, data, len, fp);
    }
    else /*From client side (requests)*/
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "From client!\n"););
        return smtp_paf_client(pfdata, data, len, fp);
    }

    return PAF_SEARCH;
}

bool is_data_end (void* ssn)
{
    if ( ssn )
    {
        SmtpPafData** s = (SmtpPafData **)_dpd.streamAPI->get_paf_user_data(ssn, 1, smtp_paf_id);

        if ( s && (*s) )
            return ((*s)->end_of_data);
    }

    return false;
}

void smtp_paf_cleanup(void *pafData)
{
    if (pafData) {
	_dpd.snortFree(pafData, sizeof(SmtpPafData), PP_SMTP, PP_MEM_CATEGORY_SESSION);
    }
}

#ifdef TARGET_BASED
void register_smtp_paf_service (struct _SnortConfig *sc, int16_t app, tSfPolicyId policy)
{
    if (_dpd.isPafEnabled())
    {
        smtp_paf_id = _dpd.streamAPI->register_paf_service(sc, policy, app, true, smtp_paf_eval, true);
        smtp_paf_id = _dpd.streamAPI->register_paf_service(sc, policy, app, false,smtp_paf_eval, true);
	_dpd.streamAPI->register_paf_free(smtp_paf_id, smtp_paf_cleanup);
    }
}
#endif

void register_smtp_paf_port(struct _SnortConfig *sc, unsigned int i, tSfPolicyId policy)
{
    if (_dpd.isPafEnabled())
    {
        smtp_paf_id = _dpd.streamAPI->register_paf_port(sc, policy, (uint16_t)i, true, smtp_paf_eval, true);
        smtp_paf_id = _dpd.streamAPI->register_paf_port(sc, policy, (uint16_t)i, false, smtp_paf_eval, true);
	_dpd.streamAPI->register_paf_free(smtp_paf_id, smtp_paf_cleanup);
    }
}

