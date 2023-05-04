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
 *
 * imap_log.h
 *
 * Author: Bhagyashree Bantwal <bbantwal@cisco.com>
 *
 **************************************************************************/

#ifndef __IMAP_LOG_H__
#define __IMAP_LOG_H__


#define GENERATOR_SPP_IMAP  141

/* Events for IMAP */
#define IMAP_UNKNOWN_CMD            1
#define IMAP_UNKNOWN_RESP           2
#define IMAP_MEMCAP_EXCEEDED        3
#define IMAP_B64_DECODING_FAILED    4
#define IMAP_QP_DECODING_FAILED     5
/* Do not delete or reuse this SID. Commenting this SID as this alert is no longer valid.*
*   #define IMAP_BITENC_DECODING_FAILED 6
*/
#define IMAP_UU_DECODING_FAILED     7

#define IMAP_EVENT_MAX  8

/* Messages for each event */
#define IMAP_UNKNOWN_CMD_STR                 "(IMAP) Unknown IMAP4 command"
#define IMAP_UNKNOWN_RESP_STR                "(IMAP) Unknown IMAP4 response"
#define IMAP_MEMCAP_EXCEEDED_STR             "(IMAP) No memory available for decoding. Memcap exceeded"
#define IMAP_B64_DECODING_FAILED_STR         "(IMAP) Base64 Decoding failed."
#define IMAP_QP_DECODING_FAILED_STR          "(IMAP) Quoted-Printable Decoding failed."
#define IMAP_UU_DECODING_FAILED_STR          "(IMAP) Unix-to-Unix Decoding failed."

#define EVENT_STR_LEN  256


/* Function prototypes  */
void IMAP_GenerateAlert(int, char *, ...);
void IMAP_DecodeAlert(void *ds);


#endif

