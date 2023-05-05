/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2012-2013 Sourcefire, Inc.
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
 ****************************************************************************/

/**************************************************************************
 *
 * appId_ss.h
 *
 * Authors: Hareesh Vutla <hvutla@cisco.com>
 *
 * Description:
 *
 * AppID state sharing exported functionality.
 *
 **************************************************************************/

#ifndef __APPID_SS_H__
#define __APPID_SS_H__

#ifdef SIDE_CHANNEL

#include "appIdConfig.h"

void AppIdPrintSSConfig(AppIdSSConfig *appId_ss_config);
void AppIdSSConfigFree(AppIdSSConfig *appId_ss_config);

void AppIdPrintSSStats(void);
void AppIdResetSSStats(void);

void AppIdSSPostConfigInit(struct _SnortConfig *sc, int unused, void *arg);
void AppIdCleanSS(void);

int CreateAppIdSSUpdate(void **msg_handle, void **hdr_ptr, void **data_ptr, uint32_t type, uint32_t data_len);
int SendAppIdSSUpdate(void *msg_handle, void *hdr_ptr, void *data_ptr, uint32_t type, uint32_t data_len);

#endif /* SIDE_CHANNEL */

#endif /* __APPID_SS_H__ */
