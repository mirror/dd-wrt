/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
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

/***************************************************************************
 *
 * smtp_config.h
 *
 * Author: Andy Mullican
 * Author: Todd Wease
 *
 ***************************************************************************/

#ifndef __SMTP_CONFIG_H__
#define __SMTP_CONFIG_H__

#include "sfPolicyUserData.h"
#include "file_mail_common.h"
#include "sf_email_attach_decode.h"
#include "file_api.h"

#define CONF_SEPARATORS                  " \t\n\r"
#define CONF_PORTS                       "ports"
#define CONF_INSPECTION_TYPE             "inspection_type"
#define CONF_NORMALIZE                   "normalize"
#define CONF_NORMALIZE_CMDS              "normalize_cmds"
#define CONF_IGNORE_DATA                 "ignore_data"
#define CONF_IGNORE_TLS_DATA             "ignore_tls_data"
#define CONF_MAX_COMMAND_LINE_LEN        "max_command_line_len"
#define CONF_MAX_HEADER_LINE_LEN         "max_header_line_len"
#define CONF_MAX_RESPONSE_LINE_LEN       "max_response_line_len"
#define CONF_ALT_MAX_COMMAND_LINE_LEN    "alt_max_command_line_len"
#define CONF_MAX_MIME_MEM                "max_mime_mem"
#define CONF_MAX_MIME_DEPTH              "max_mime_depth"
#define CONF_ENABLE_MIME_DECODING        "enable_mime_decoding"
#define CONF_B64_DECODE                  "b64_decode_depth"
#define CONF_QP_DECODE                   "qp_decode_depth"
#define CONF_BITENC_DECODE               "bitenc_decode_depth"
#define CONF_UU_DECODE                   "uu_decode_depth"
#define CONF_LOG_FILENAME                "log_filename"
#define CONF_LOG_MAIL_FROM               "log_mailfrom"
#define CONF_LOG_RCPT_TO                 "log_rcptto"
#define CONF_LOG_EMAIL_HDRS              "log_email_hdrs"
#define CONF_SMTP_MEMCAP                 "memcap"
#define CONF_EMAIL_HDRS_LOG_DEPTH        "email_hdrs_log_depth"
#define CONF_DISABLED                    "disabled"
#define CONF_NO_ALERTS                   "no_alerts"
#define CONF_VALID_CMDS                  "valid_cmds"
#define CONF_INVALID_CMDS                "invalid_cmds"
#define CONF_PRINT_CMDS                  "print_cmds"
#define CONF_ALERT_UNKNOWN_CMDS          "alert_unknown_cmds"
#define CONF_XLINK2STATE                 "xlink2state"
#define CONF_ENABLE                      "enable"
#define CONF_DISABLE                     "disable"
#define CONF_INLINE_DROP                 "drop"
#define CONF_STATEFUL                    "stateful"
#define CONF_STATELESS                   "stateless"
#define CONF_YES                         "yes"
#define CONF_ALL                         "all"
#define CONF_NONE                        "none"
#define CONF_CMDS                        "cmds"
#define CONF_AUTH_CMDS                   "auth_cmds"
#define CONF_DATA_CMDS                   "data_cmds"
#define CONF_BDATA_CMDS                  "binary_data_cmds"
#define CONF_START_LIST "{"
#define CONF_END_LIST   "}"
#define CONF_MAX_AUTH_COMMAND_LINE_LEN   "max_auth_command_line_len"

#define NORMALIZE_NONE 0
#define NORMALIZE_CMDS 1
#define NORMALIZE_ALL  2

#define ACTION_ALERT      0
#define ACTION_NO_ALERT   1
#define ACTION_NORMALIZE  2

#define DEFAULT_MAX_COMMAND_LINE_LEN    0
#define DEFAULT_MAX_HEADER_LINE_LEN     0
#define DEFAULT_MAX_RESPONSE_LINE_LEN   0
#define DEFAULT_AUTH_MAX_COMMAND_LINE_LEN    1000

/*These are temporary values*/
#define MAX_DEPTH                     65535 
#define MIN_DEPTH                     -1
#define DEFAULT_MAX_MIME_MEM           838860
#define DEFAULT_MAX_MIME_DEPTH         1460
#define DEFAULT_SMTP_MEMCAP            838860
#define DEFAULT_LOG_DEPTH              1464
#define MAX_MIME_MEM                   104857600
#define MIN_MIME_MEM                   3276
#define MAX_MIME_DEPTH                 20480
#define MIN_MIME_DEPTH                 4
#define MAX_SMTP_MEMCAP                104857600
#define MIN_SMTP_MEMCAP                3276
#define MAX_LOG_DEPTH                  20480
#define MIN_LOG_DEPTH                  1
#define SMTP_DEFAULT_SERVER_PORT       25  /* SMTP normally runs on port 25 */
#define SMTP_DEFAULT_SUBMISSION_PORT  587  /* SMTP Submission port - see RFC 2476 */
#define XLINK2STATE_DEFAULT_PORT      691  /* XLINK2STATE sometimes runs on port 691 */

#define ERRSTRLEN   512

typedef enum _SMTPCmdTypeEnum
{
    SMTP_CMD_TYPE_NORMAL = 0,
    SMTP_CMD_TYPE_DATA,
    SMTP_CMD_TYPE_BDATA,
    SMTP_CMD_TYPE_AUTH,
    SMTP_CMD_TYPE_LAST

} SMTPCmdTypeEnum;

typedef struct _SMTPSearch
{
    char *name;
    int   name_len;

} SMTPSearch;

typedef struct _SMTPToken
{
    char *name;
    int   name_len;
    int   search_id;
    SMTPCmdTypeEnum type;

} SMTPToken;

typedef struct _SMTPCmdConfig
{
    char alert;          /*  1 if alert when seen                          */
    char normalize;      /*  1 if we should normalize this command         */
    int  max_line_len;   /*  Max length of this particular command         */

} SMTPCmdConfig;

typedef struct _SMTPConfig
{
    uint8_t ports[8192];
    char  inspection_type;
    char  normalize;
    char  ignore_tls_data;
    int   max_command_line_len;
    int   max_header_line_len;
    int   max_response_line_len;
    char  no_alerts;
    char  alert_unknown_cmds;
    char  alert_xlink2state;
    char  drop_xlink2state;
    char  print_cmds;
    char  enable_mime_decoding;
    MAIL_LogConfig log_config;
    uint32_t   memcap;
    int   max_mime_depth; 
    DecodeConfig decode_conf;

    SMTPToken *cmds;
    SMTPCmdConfig *cmd_config;
    SMTPSearch *cmd_search;
    void *cmd_search_mpse;
    int num_cmds;
    int disabled;

    int ref_count;
    uint32_t xtra_filename_id;
    uint32_t xtra_mfrom_id;
    uint32_t xtra_rcptto_id;
    uint32_t xtra_ehdrs_id;
    int max_auth_command_line_len;

} SMTPConfig;

typedef struct _SMTP_Stats
{
    uint64_t sessions;
    uint64_t conc_sessions;
    uint64_t max_conc_sessions;
    uint64_t log_memcap_exceeded;
    uint64_t cur_sessions;
    MimeStats mime_stats;

} SMTP_Stats;

extern SMTP_Stats smtp_stats;

/* Function prototypes  */
void SMTP_ParseArgs(SMTPConfig *, char *);
void SMTP_PrintConfig(SMTPConfig *config);

void SMTP_CheckConfig(SMTPConfig *, tSfPolicyUserContextId);

#endif

