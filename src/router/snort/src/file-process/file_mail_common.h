/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * ** Copyright (C) 2012-2013 Sourcefire, Inc.
 * ** AUTHOR: Hui Cao
 * **
 * ** This program is free software; you can redistribute it and/or modify
 * ** it under the terms of the GNU General Public License Version 2 as
 * ** published by the Free Software Foundation.  You may not use, modify or
 * ** distribute this program under any other version of the GNU General
 * ** Public License.
 * **
 * ** This program is distributed in the hope that it will be useful,
 * ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 * ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * ** GNU General Public License for more details.
 * **
 * ** You should have received a copy of the GNU General Public License
 * ** along with this program; if not, write to the Free Software
 * ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * */

/*
 * Purpose: To be used for mail protocols and mime processing
 *
 *  Author(s):  Hui Cao <huica@cisco.com>
 *
 */

#ifndef FILE_MAIL_COMMON_H_
#define FILE_MAIL_COMMON_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include "sfPolicy.h"

typedef struct s_FILE_LogState
{
    uint8_t *filenames;
    uint16_t file_logged;
    uint16_t file_current;
    uint16_t file_name;
} FILE_LogState;

typedef struct s_MAIL_LogState
{
    void *log_hdrs_bkt;
    unsigned char *emailHdrs;
    uint32_t log_depth;
    uint32_t hdrs_logged;
    uint8_t *recipients;
    uint16_t rcpts_logged;
    uint8_t *senders;
    uint16_t snds_logged;
    FILE_LogState file_log;
}MAIL_LogState;

/* log flags */
#define FLAG_MAIL_FROM_PRESENT               0x00000001
#define FLAG_RCPT_TO_PRESENT                 0x00000002
#define FLAG_FILENAME_PRESENT                0x00000004
#define FLAG_EMAIL_HDRS_PRESENT              0x00000008
#define FLAG_FILENAME_IN_HEADER              0x00000010

typedef struct s_MAIL_LogConfig
{
    uint32_t  memcap;
    char  log_mailfrom;
    char  log_rcptto;
    char  log_filename;
    char  log_email_hdrs;
    uint32_t   email_hdrs_log_depth;
}MAIL_LogConfig;

/* State tracker for data */
typedef enum _MimeDataState
{
    MIME_PAF_FINDING_BOUNDARY_STATE,
    MIME_PAF_FOUND_BOUNDARY_STATE
} MimeDataState;

/* State tracker for Boundary Signature */
typedef enum _MimeBoundaryState
{
    MIME_PAF_BOUNDARY_UNKNOWN = 0,  /* UNKNOWN */
    MIME_PAF_BOUNDARY_LF,           /* '\n' */
    MIME_PAF_BOUNDARY_HYPEN_FIRST,  /* First '-' */
    MIME_PAF_BOUNDARY_HYPEN_SECOND  /* Second '-' */
} MimeBoundaryState;

/* State tracker for end of pop/smtp command */
typedef enum _DataEndState
{
    PAF_DATA_END_UNKNOWN,     /* Start or UNKNOWN */
    PAF_DATA_END_FIRST_CR,    /* First '\r' */
    PAF_DATA_END_FIRST_LF,    /* First '\n' */
    PAF_DATA_END_DOT,         /* '.' */
    PAF_DATA_END_SECOND_CR,   /* Second '\r' */
    PAF_DATA_END_SECOND_LF    /* Second '\n' */
} DataEndState;

#define MAX_BOUNDARY_LEN  70  /* Max length of boundary string, defined in RFC 2046 */

typedef struct _MimeDataPafInfo
{
    MimeDataState data_state;
    char   boundary[ MAX_BOUNDARY_LEN + 1];  /* MIME boundary string + '\0' */
    uint32_t boundary_len;
    char* boundary_search;
    MimeBoundaryState boundary_state;
} MimeDataPafInfo;

typedef int (*Handle_header_line_func)(void *pkt, const uint8_t *ptr, const uint8_t *eol, int max_header_len, void *mime_ssn);
typedef int (*Normalize_data_func)(void *pkt, const uint8_t *ptr, const uint8_t *data_end);
typedef void (*Decode_alert_func)(void *decode_state);
typedef void (*Reset_state_func)(void);
typedef bool (*Is_end_of_data_func)(void* ssn);

typedef struct _MimeMethods
{
    Handle_header_line_func handle_header_line;
    Normalize_data_func  normalize_data;
    Decode_alert_func decode_alert;
    Reset_state_func reset_state;
    Is_end_of_data_func is_end_of_data;
} MimeMethods;

typedef struct _DecodeConfig
{
    char ignore_data;
    int max_mime_mem;
    int max_depth;
    int b64_depth;
    int qp_depth;
    int bitenc_depth;
    int uu_depth;
    int64_t file_depth;
} DecodeConfig;

typedef struct _MimeState
{
    int data_state;
    int state_flags;
    int log_flags;
    void *decode_state;
    MimeDataPafInfo  mime_boundary;
    DecodeConfig *decode_conf;
    MAIL_LogConfig *log_config;
    MAIL_LogState *log_state;
    void *mime_stats;
    void *decode_bkt;
    void *mime_mempool;
    void *log_mempool;
    MimeMethods *methods;
} MimeState;


static inline bool scanning_boundary(MimeDataPafInfo *mime_info, uint32_t boundary_start, uint32_t* fp)
{
    if (boundary_start &&
            mime_info->data_state == MIME_PAF_FOUND_BOUNDARY_STATE &&
            mime_info->boundary_state != MIME_PAF_BOUNDARY_UNKNOWN)
    {
        *fp = boundary_start;
        return true;
    }

    return false;
}


#endif /* FILE_MAIL_COMMON_H_ */
