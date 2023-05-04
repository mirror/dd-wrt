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
 * pop_log.h
 *
 * Author: Bhagyashree Bantwal <bbantwal@cisco.com>
 *
 **************************************************************************/

#ifndef __POP_LOG_H__
#define __POP_LOG_H__


#define GENERATOR_SPP_POP  142

/* Events for POP */
#define POP_UNKNOWN_CMD            1
#define POP_UNKNOWN_RESP           2
#define POP_MEMCAP_EXCEEDED        3
#define POP_B64_DECODING_FAILED    4
#define POP_QP_DECODING_FAILED     5
/* Do not delete or reuse this SID. Commenting this SID as this alert is no longer valid.*
* #define POP_BITENC_DECODING_FAILED 6
*/
#define POP_UU_DECODING_FAILED     7

#define POP_EVENT_MAX  8

/* Messages for each event */
#define POP_UNKNOWN_CMD_STR                 "(POP) Unknown POP3 command"
#define POP_UNKNOWN_RESP_STR                "(POP) Unknown POP3 response"
#define POP_MEMCAP_EXCEEDED_STR             "(POP) No memory available for decoding. Memcap exceeded"
#define POP_B64_DECODING_FAILED_STR         "(POP) Base64 Decoding failed."
#define POP_QP_DECODING_FAILED_STR          "(POP) Quoted-Printable Decoding failed."
#define POP_UU_DECODING_FAILED_STR          "(POP) Unix-to-Unix Decoding failed."

#define EVENT_STR_LEN  256


/* Function prototypes  */
void POP_GenerateAlert(int, char *, ...);
void POP_DecodeAlert(void *);


#endif

