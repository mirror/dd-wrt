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
 * **************************************************************************/

/**************************************************************************
 *
 * snort_imap.h
 *
 * Author: Bhagyashree Bantwal <bbantwal@cisco.com>
 *
 * Description:
 *
 * This file defines everything specific to the IMAP preprocessor.
 *
 **************************************************************************/

#ifndef __IMAP_H__
#define __IMAP_H__


/* Includes ***************************************************************/

#include <pcre.h>

#include "sf_snort_packet.h"
#include "imap_config.h"
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
#define IMAP_PKT_FROM_UNKNOWN  0
#define IMAP_PKT_FROM_CLIENT   1
#define IMAP_PKT_FROM_SERVER   2

#define SEARCH_CMD       0
#define SEARCH_RESP      1
#define SEARCH_HDR       2
#define SEARCH_DATA_END  3
#define NUM_SEARCHES  4

#define BOUNDARY     0

#define STATE_DATA             0    /* Data state */
#define STATE_TLS_CLIENT_PEND  1    /* Got STARTTLS */
#define STATE_TLS_SERVER_PEND  2    /* Got STARTTLS */
#define STATE_TLS_DATA         3    /* Successful handshake, TLS encrypted data */
#define STATE_COMMAND          4
#define STATE_UNKNOWN          5

#define STATE_DATA_INIT    0
#define STATE_DATA_HEADER  1    /* Data header section of data state */
#define STATE_DATA_BODY    2    /* Data body section of data state */
#define STATE_MIME_HEADER  3    /* MIME header section within data section */
#define STATE_DATA_UNKNOWN 4

/* session flags */
#define IMAP_FLAG_NEXT_STATE_UNKNOWN         0x00000004
#define IMAP_FLAG_GOT_NON_REBUILT            0x00000008
#define IMAP_FLAG_CHECK_SSL                  0x00000010

#define IMAP_SSL_ERROR_FLAGS  (SSL_BOGUS_HS_DIR_FLAG | \
                               SSL_BAD_VER_FLAG | \
                               SSL_BAD_TYPE_FLAG | \
                               SSL_UNKNOWN_FLAG)

/* Maximum length of header chars before colon, based on Exim 4.32 exploit */
#define MAX_HEADER_NAME_LEN 64

#define IMAP_PROTO_REF_STR  "imap"

/**************************************************************************/


/* Data structures ********************************************************/

typedef enum _IMAPCmdEnum
{
    CMD_APPEND = 0,
    CMD_AUTHENTICATE,
    CMD_CAPABILITY,
    CMD_CHECK,
    CMD_CLOSE,
    CMD_COMPARATOR,
    CMD_COMPRESS,
    CMD_CONVERSIONS,
    CMD_COPY,
    CMD_CREATE,
    CMD_DELETE,
    CMD_DELETEACL,
    CMD_DONE,
    CMD_EXAMINE,
    CMD_EXPUNGE,
    CMD_FETCH,
    CMD_GETACL,
    CMD_GETMETADATA,
    CMD_GETQUOTA,
    CMD_GETQUOTAROOT,
    CMD_IDLE,
    CMD_LIST,
    CMD_LISTRIGHTS,
    CMD_LOGIN,
    CMD_LOGOUT,
    CMD_LSUB,
    CMD_MYRIGHTS,
    CMD_NOOP,
    CMD_NOTIFY,
    CMD_RENAME,
    CMD_SEARCH,
    CMD_SELECT,
    CMD_SETACL,
    CMD_SETMETADATA,
    CMD_SETQUOTA,
    CMD_SORT,
    CMD_STARTTLS,
    CMD_STATUS,
    CMD_STORE,
    CMD_SUBSCRIBE,
    CMD_THREAD,
    CMD_UID,
    CMD_UNSELECT,
    CMD_UNSUBSCRIBE,
    CMD_X,
    CMD_LAST

} IMAPCmdEnum;

typedef enum _IMAPRespEnum
{
    RESP_CAPABILITY = 0,
    RESP_LIST,
    RESP_LSUB,
    RESP_STATUS,
    RESP_SEARCH,
    RESP_FLAGS,
    RESP_EXISTS,
    RESP_RECENT,
    RESP_EXPUNGE,
    RESP_FETCH,
    RESP_BAD,
    RESP_BYE,
    RESP_NO,
    RESP_OK,
    RESP_PREAUTH,
    RESP_ENVELOPE,
    RESP_UID,
    RESP_LAST

} IMAPRespEnum;

typedef enum _IMAPHdrEnum
{
    HDR_CONTENT_TYPE = 0,
    HDR_CONT_TRANS_ENC,
    HDR_CONT_DISP,
    HDR_LAST

} IMAPHdrEnum;

typedef enum _IMAPDataEndEnum
{
    DATA_END_1 = 0,
    DATA_END_2,
    DATA_END_3,
    DATA_END_4,
    DATA_END_LAST

} IMAPDataEndEnum;

typedef struct _IMAPSearchInfo
{
    int id;
    int index;
    int length;

} IMAPSearchInfo;

typedef struct _IMAPMimeBoundary
{
    int    state;
    char   boundary[2 + MAX_BOUNDARY_LEN + 1];  /* '--' + MIME boundary string + '\0' */
    int    boundary_len;
    void  *boundary_search;

} IMAPMimeBoundary;

typedef struct _IMAPPcre
{
    pcre       *re;
    pcre_extra *pe;

} IMAPPcre;

typedef struct _IMAP
{
    int state;
    int state_flags;
    int session_flags;
    int alert_mask;
    int reassembling;
    uint32_t body_len;
    uint32_t body_read;
#ifdef DEBUG_MSGS
    uint64_t session_number;
#endif

    tSfPolicyId policy_id;
    MimeState mime_ssn;

    tSfPolicyUserContextId config;
    uint32_t flow_id;

} IMAP;


/**************************************************************************/


/* Function prototypes ****************************************************/

void IMAP_InitCmds(IMAPConfig *config);
void IMAP_SearchInit(void);
void IMAP_Free(void);
void SnortIMAP(SFSnortPacket *);
int  IMAP_IsServer(uint16_t);
void IMAP_FreeConfig(IMAPConfig *);
void IMAP_FreeConfigs(tSfPolicyUserContextId);
int  IMAP_GetFilename(void *data, uint8_t **buf, uint32_t *len, uint32_t *type);
/**************************************************************************/

#endif  /* __IMAP_H__ */

