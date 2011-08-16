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

#define MAX_BOUNDARY_LEN  70  /* Max length of boundary string, defined in RFC 2046 */

#define STATE_CONNECT          0
#define STATE_COMMAND          1    /* Command state of SMTP transaction */
#define STATE_DATA             2    /* Data state */
#define STATE_TLS_CLIENT_PEND  3    /* Got STARTTLS */
#define STATE_TLS_SERVER_PEND  4    /* Got STARTTLS */
#define STATE_TLS_DATA         5    /* Successful handshake, TLS encrypted data */
#define STATE_UNKNOWN          6

#define STATE_DATA_INIT    0
#define STATE_DATA_HEADER  1    /* Data header section of data state */
#define STATE_DATA_BODY    2    /* Data body section of data state */
#define STATE_MIME_HEADER  3    /* MIME header section within data section */
#define STATE_DATA_UNKNOWN 4

/* state flags */
#define SMTP_FLAG_GOT_MAIL_CMD               0x00000001
#define SMTP_FLAG_GOT_RCPT_CMD               0x00000002
#define SMTP_FLAG_FOLDING                    0x00000004
#define SMTP_FLAG_IN_CONTENT_TYPE            0x00000008
#define SMTP_FLAG_GOT_BOUNDARY               0x00000010
#define SMTP_FLAG_DATA_HEADER_CONT           0x00000020
#define SMTP_FLAG_IN_CONT_TRANS_ENC          0x00000040
#define SMTP_FLAG_BASE64_DATA                0x00000080
#define SMTP_FLAG_MULTIPLE_BASE64            0x00000100

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
    CMD_LAST

} SMTPCmdEnum;

typedef enum _SMTPRespEnum
{
    RESP_220 = 0,
    RESP_221,
    RESP_250,
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

typedef struct _SMTPMimeBoundary
{
    char   boundary[2 + MAX_BOUNDARY_LEN + 1];  /* '--' + MIME boundary string + '\0' */
    int    boundary_len;
    void  *boundary_search;

} SMTPMimeBoundary;

typedef struct _SMTPPcre
{
    pcre       *re;
    pcre_extra *pe;

} SMTPPcre;

typedef struct s_SMTP_DecodeState
{
    uint8_t decode_present;
    int prev_encoded_bytes;
    unsigned char *prev_encoded_buf;
    int encode_bytes_read;
    int decode_bytes_read;
    int encode_depth;
    int decode_depth;
    uint32_t decoded_bytes;
    MemBucket *mime_bucket;
    unsigned char *encodeBuf;
    unsigned char *decodeBuf;

} SMTP_DecodeState;


typedef struct _SMTP
{
    int state;
    int data_state;
    int state_flags;
    int session_flags;
    int alert_mask;
    int reassembling;
#ifdef DEBUG
    uint64_t session_number;
#endif

    /* may want to keep track where packet didn't end with end of line marker
    int               cur_client_line_len;
    int               cur_server_line_len;
    */

    SMTPMimeBoundary  mime_boundary;
    SMTP_DecodeState *decode_state;

    /* In future if we look at forwarded mail (message/rfc822) we may
     * need to keep track of additional mime boundaries
     * SMTPMimeBoundary  mime_boundary[8];
     * int               current_mime_boundary;
     */

    tSfPolicyId policy_id;
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

/**************************************************************************/

static INLINE void ResetDecodeState(SMTP_DecodeState *ds)
{
    if (ds == NULL)
        return;

/*    memset(ds->mime_bucket->data, 0, ds->encode_depth + ds->decode_depth);*/
    ds->prev_encoded_bytes = 0;
    ds->prev_encoded_buf = NULL;
    ds->encode_bytes_read = 0;
    ds->decode_bytes_read = 0;
    ds->decoded_bytes = 0;
    ds->decode_present = 0;
}

static INLINE void ClearPrevEncode(SMTP_DecodeState *ds)
{
        ds->prev_encoded_bytes = 0; 
        ds->prev_encoded_buf = NULL;
}


#endif  /* __SMTP_H__ */

