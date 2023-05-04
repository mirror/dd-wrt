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
 * **************************************************************************/

/**************************************************************************
 *
 * snort_smtp.h
 *
 * Author: Andy Mullican
 * Author: Todd Wease
 *
 * Description:
 *
 * This file defines everything specific to the SMTP preprocessor.
 *
 **************************************************************************/

#ifndef __SMTP_H__
#define __SMTP_H__


/* Includes ***************************************************************/

#include <pcre.h>

#include "sf_snort_packet.h"
#include "ssl.h"
#include "smtp_config.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"
#include "mempool.h"
#include "sf_email_attach_decode.h"
#include "file_mail_common.h"
#include "file_api.h"

#ifdef DEBUG
#include "sf_types.h"
#endif

/**************************************************************************/


/* Defines ****************************************************************/

/* Direction packet is coming from, if we can figure it out */
#define SMTP_PKT_FROM_UNKNOWN  0
#define SMTP_PKT_FROM_CLIENT   1
#define SMTP_PKT_FROM_SERVER   2

/* Inspection type */
#define SMTP_STATELESS  0
#define SMTP_STATEFUL   1

#define SEARCH_CMD       0
#define SEARCH_RESP      1
#define SEARCH_HDR       2
#define SEARCH_DATA_END  3
#define NUM_SEARCHES  4

#define BOUNDARY     0

#define STATE_CONNECT          0
#define STATE_COMMAND          1    /* Command state of SMTP transaction */
#define STATE_DATA             2    /* Data state */
#define STATE_BDATA            3    /* Binary data state */
#define STATE_TLS_CLIENT_PEND  4    /* Got STARTTLS */
#define STATE_TLS_SERVER_PEND  5    /* Got STARTTLS */
#define STATE_TLS_DATA         6    /* Successful handshake, TLS encrypted data */
#define STATE_AUTH             7
#define STATE_XEXCH50          8
#define STATE_UNKNOWN          9

#define STATE_DATA_INIT    0
#define STATE_DATA_HEADER  1    /* Data header section of data state */
#define STATE_DATA_BODY    2    /* Data body section of data state */
#define STATE_MIME_HEADER  3    /* MIME header section within data section */
#define STATE_DATA_UNKNOWN 4

/* state flags */
#define SMTP_FLAG_GOT_MAIL_CMD               0x00000001
#define SMTP_FLAG_GOT_RCPT_CMD               0x00000002
#define SMTP_FLAG_BDAT                       0x00001000
#define SMTP_FLAG_ABORT                      0x00002000

/* session flags */
#define SMTP_FLAG_XLINK2STATE_GOTFIRSTCHUNK  0x00000001
#define SMTP_FLAG_XLINK2STATE_ALERTED        0x00000002
#define SMTP_FLAG_NEXT_STATE_UNKNOWN         0x00000004
#define SMTP_FLAG_GOT_NON_REBUILT            0x00000008
#define SMTP_FLAG_CHECK_SSL                  0x00000010

#define SMTP_SSL_ERROR_FLAGS  (SSL_BOGUS_HS_DIR_FLAG | \
                               SSL_BAD_VER_FLAG | \
                               SSL_BAD_TYPE_FLAG | \
                               SSL_UNKNOWN_FLAG)

/* Maximum length of header chars before colon, based on Exim 4.32 exploit */
#define MAX_HEADER_NAME_LEN 64

#define SMTP_PROTO_REF_STR  "smtp"

#define MAX_AUTH_NAME_LEN  20  /* Max length of SASL mechanisms, defined in RFC 4422 */

/**************************************************************************/


/* Data structures ********************************************************/

typedef enum _SMTPCmdEnum
{
    CMD_ATRN = 0,
    CMD_AUTH,
    CMD_BDAT,
    CMD_DATA,
    CMD_DEBUG,
    CMD_EHLO,
    CMD_EMAL,
    CMD_ESAM,
    CMD_ESND,
    CMD_ESOM,
    CMD_ETRN,
    CMD_EVFY,
    CMD_EXPN,
    CMD_HELO,
    CMD_HELP,
    CMD_IDENT,
    CMD_MAIL,
    CMD_NOOP,
    CMD_ONEX,
    CMD_QUEU,
    CMD_QUIT,
    CMD_RCPT,
    CMD_RSET,
    CMD_SAML,
    CMD_SEND,
    CMD_SIZE,
    CMD_STARTTLS,
    CMD_SOML,
    CMD_TICK,
    CMD_TIME,
    CMD_TURN,
    CMD_TURNME,
    CMD_VERB,
    CMD_VRFY,
    CMD_X_EXPS,
    CMD_XADR,
    CMD_XAUTH,
    CMD_XCIR,
    CMD_XEXCH50,
    CMD_XGEN,
    CMD_XLICENSE,
    CMD_X_LINK2STATE,
    CMD_XQUE,
    CMD_XSTA,
    CMD_XTRN,
    CMD_XUSR,
    CMD_ABORT,
    CMD_LAST

} SMTPCmdEnum;

typedef enum _SMTPRespEnum
{
    RESP_220 = 0,
    RESP_221,
    RESP_235,
    RESP_250,
    RESP_334,
    RESP_354,
    RESP_421,
    RESP_450,
    RESP_451,
    RESP_452,
    RESP_500,
    RESP_501,
    RESP_502,
    RESP_503,
    RESP_504,
    RESP_535,
    RESP_550,
    RESP_551,
    RESP_552,
    RESP_553,
    RESP_554,
    RESP_LAST

} SMTPRespEnum;

typedef enum _SMTPHdrEnum
{
    HDR_CONTENT_TYPE = 0,
    HDR_CONT_TRANS_ENC,
    HDR_CONT_DISP,
    HDR_LAST

} SMTPHdrEnum;

typedef enum _SMTPDataEndEnum
{
    DATA_END_1 = 0,
    DATA_END_2,
    DATA_END_3,
    DATA_END_4,
    DATA_END_LAST

} SMTPDataEndEnum;

typedef struct _SMTPSearchInfo
{
    int id;
    int index;
    int length;

} SMTPSearchInfo;

typedef struct _SMTPAuthName
{
    int length;
    char name[MAX_AUTH_NAME_LEN];
} SMTPAuthName;

typedef struct _SMTP
{
    int state;
    int state_flags;
    int session_flags;
    int alert_mask;
    int reassembling;
    uint32_t dat_chunk;
#ifdef DEBUG_MSGS
    uint64_t session_number;
#endif

    /* may want to keep track where packet didn't end with end of line marker
    int               cur_client_line_len;
    int               cur_server_line_len;
    */

    MimeState mime_ssn;
    SMTPAuthName *auth_name;
    /* In future if we look at forwarded mail (message/rfc822) we may
     * need to keep track of additional mime boundaries
     * SMTPMimeBoundary  mime_boundary[8];
     * int               current_mime_boundary;
     */

    tSfPolicyId policy_id;
    uint32_t flow_id;
    tSfPolicyUserContextId config;
} SMTP;


/**************************************************************************/


/* Function prototypes ****************************************************/

void SMTP_InitCmds(SMTPConfig *config);
void SMTP_SearchInit(void);
void SMTP_Free(void);
void SnortSMTP(SFSnortPacket *);
int  SMTP_IsServer(uint16_t);
void SMTP_FreeConfig(SMTPConfig *);
void SMTP_FreeConfigs(tSfPolicyUserContextId);
int SMTP_GetFilename(void *data, uint8_t **buf, uint32_t *len, uint32_t *type);
int SMTP_GetMailFrom(void *data, uint8_t **buf, uint32_t *len, uint32_t *type);
int SMTP_GetRcptTo(void *data, uint8_t **buf, uint32_t *len, uint32_t *type);
int SMTP_GetEmailHdrs(void *data, uint8_t **buf, uint32_t *len, uint32_t *type);
int SMTP_SessionExist(void *data);
void SMTP_MempoolInit(uint32_t, uint32_t);

/**************************************************************************/

#endif  /* __SMTP_H__ */

