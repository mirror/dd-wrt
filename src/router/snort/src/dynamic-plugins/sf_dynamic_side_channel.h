/*
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
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2012-2013 Sourcefire, Inc.
 *
 * Author: Michael Altizer <maltizer@sourcefire.com>
 *
 * Dynamic Side Channel Lib function declarations
 *
 */
#ifndef _SF_DYNAMIC_SIDE_CHANNEL_H_
#define _SF_DYNAMIC_SIDE_CHANNEL_H_

#include <stdint.h>

#include "sf_dynamic_common.h"
#include "sf_dynamic_meta.h"
#include "sidechannel_define.h"

#define SIDE_CHANNEL_DATA_VERSION 1

struct _SnortConfig;

typedef int (*RegisterRXHandler)(uint16_t type, SCMProcessMsgFunc processMsgFunc, void *data);
typedef int (*RegisterTXHandler)(uint16_t type, SCMProcessMsgFunc processMsgFunc, void *data);
typedef void (*UnregisterRXHandler)(uint16_t type, SCMProcessMsgFunc processMsgFunc);
typedef void (*UnregisterTXHandler)(uint16_t type, SCMProcessMsgFunc processMsgFunc);
typedef int (*PreallocMessageRX)(uint32_t length, SCMsgHdr **hdr, uint8_t **msg_ptr, void **msg_handle);
typedef int (*PreallocMessageTX)(uint32_t length, SCMsgHdr **hdr, uint8_t **msg_ptr, void **msg_handle);
typedef int (*DiscardMessageRX)(void *msg_handle);
typedef int (*DiscardMessageTX)(void *msg_handle);
typedef int (*EnqueueMessageRX)(SCMsgHdr *hdr, const uint8_t *msg, uint32_t length, void *msg_handle, SCMQMsgFreeFunc msgFreeFunc);
typedef int (*EnqueueMessageTX)(SCMsgHdr *hdr, const uint8_t *msg, uint32_t length, void *msg_handle, SCMQMsgFreeFunc msgFreeFunc);
typedef int (*EnqueueDataRX)(SCMsgHdr *hdr, uint8_t *msg, uint32_t length, SCMQMsgFreeFunc msgFreeFunc);
typedef int (*EnqueueDataTX)(SCMsgHdr *hdr, uint8_t *msg, uint32_t length, SCMQMsgFreeFunc msgFreeFunc);
typedef void (*RegisterModule)(const char *keyword, SCMFunctionBundle *funcs);
typedef sigset_t (*SnortSignalMask)(void);

typedef struct _DynamicSideChannelData
{
    int version;
    int size;

    RegisterModule registerModule;

    RegisterRXHandler registerRXHandler;
    RegisterTXHandler registerTXHandler;
    UnregisterRXHandler unregisterRXHandler;
    UnregisterTXHandler unregisterTXHandler;

    PreallocMessageRX allocMessageRX;
    PreallocMessageTX allocMessageTX;
    DiscardMessageRX discardMessageRX;
    DiscardMessageRX discardMessageTX;
    EnqueueMessageRX enqueueMessageRX;
    EnqueueMessageTX enqueueMessageTX;
    EnqueueDataRX enqueueDataRX;
    EnqueueDataTX enqueueDataTX;

    LogMsgFunc logMsg;
    LogMsgFunc errMsg;
    LogMsgFunc fatalMsg;
    DebugMsgFunc debugMsg;

    GetSnortInstance getSnortInstance;
    SnortSignalMask snortSignalMask;
} DynamicSideChannelData;

/* Function prototypes for Dynamic Detection Plugins */
int LoadDynamicSideChannelLib(struct _SnortConfig *sc, const char * const library_name, int indent);
void CloseDynamicSideChannelLibs(void);
void LoadAllDynamicSideChannelLibs(struct _SnortConfig *sc, const char * const path);
void RemoveDuplicateSideChannelPlugins(void);
int InitDynamicSideChannelPlugins(void);

typedef int (*InitSideChannelLibFunc)(DynamicSideChannelData *dscd);

void *GetNextSideChannelPluginVersion(void *p);
DynamicPluginMeta *GetSideChannelPluginMetaData(void *p);

extern DynamicSideChannelData _dscd;

#endif /* _SF_DYNAMIC_SIDE_CHANNEL_H_ */
