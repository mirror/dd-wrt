/*
**
**  sfcontrol.h
**
**  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
**  Copyright (C) 2002-2013 Sourcefire, Inc.
**  Author(s):  Ron Dempster <rdempster@sourcefire.com>
**
**  NOTES
**  5.16.11 - Initial Source Code. Dempster
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License Version 2 as
**  published by the Free Software Foundation.  You may not use, modify or
**  distribute this program under any other version of the GNU General
**  Public License.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
*/

#ifndef __SF_CONTROL_H__
#define __SF_CONTROL_H__

#define CONTROL_FILE    "SNORT.sock"

#define CS_TYPE_HUP_DAQ         0x0001
#define CS_TYPE_RELOAD          0x0002
#define CS_TYPE_IS_PROCESSING   0x0003
#define CS_TYPE_DUMP_PACKETS    0x0004
#define CS_TYPE_MEM_USAGE       0x0005
#define CS_TYPE_ACTION_STATS    0x0006
#define CS_TYPE_STREAM_STATS    0x0007
#define CS_TYPE_SMTP_STATS      0x0008
#define CS_TYPE_SSL_STATS       0x0009
#define CS_TYPE_SIP_STATS       0x000A
#define CS_TYPE_FLOWIP_START    0x000D
#define CS_TYPE_FLOWIP_STOP     0x000E
#define CS_TYPE_FLOWIP_SHOW     0x000F
#define CS_TYPE_MEM_STATS_CFG   0x0011
#define CS_TYPE_MEM_STATS_SHOW  0x0012
#define CS_TYPE_MAX             0x1FFF
#define CS_HEADER_VERSION       0x0001
#define CS_HEADER_SUCCESS       0x0000
#define CS_HEADER_ERROR         0x0001
#define CS_HEADER_DATA          0x0009
#ifdef REG_TEST
#define CS_CONFIG_SWAP          0x000A
#endif
/* For SFR Display */
#define MEM_STATS_BUF_SIZE 7680 //CS_STATS_BUF_SIZE * (no. of preprocessors) * 2 = 1280 * 3 * 2
#define CS_STATS_BUF_SIZE 1280
#pragma pack(1)
typedef struct _CS_MESSAGE_DATA_HEADER
{
    /* All values must be in network byte order */
    int32_t code;
    uint16_t length;    /* Data length. Does not include this header */
} CSMessageDataHeader;
#pragma pack()

typedef struct _CS_MESSAGE_HEADER
{
    /* All values must be in network byte order */
    uint16_t version;
    uint16_t type;
    uint32_t length;    /* Does not include the header */
} CSMessageHeader;

struct _THREAD_ELEMENT;
typedef int (*ControlDataSendFunc)(struct _THREAD_ELEMENT *te, const uint8_t *data, uint16_t length);
typedef int (*OOBPreControlFunc)(uint16_t type, const uint8_t *data, uint32_t length, void **new_context, char *statusBuf, int statusBuf_len);
typedef int (*IBControlFunc)(uint16_t type, void *new_context, void **old_context);
typedef void (*OOBPostControlFunc)(uint16_t type, void *old_context, struct _THREAD_ELEMENT *te, ControlDataSendFunc f);

#endif

