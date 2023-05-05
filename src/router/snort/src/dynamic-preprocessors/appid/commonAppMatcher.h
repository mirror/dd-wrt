/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2005-2013 Sourcefire, Inc.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#ifndef __COMMON_APP_MATCHER_H__
#define __COMMON_APP_MATCHER_H__

#include <stdbool.h>
#include <stdint.h>
#include <strings.h>

#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"
#include "sflsq.h"
#include "service_state.h"
#include "flow.h"
#include "appId.h"
#include "NetworkSet.h"

struct AppIdData;
struct AppidStaticConfig;

typedef struct _appRegistryEntry
{
    tAppId appId;
    uint32_t  additionalInfo;
} tAppRegistryEntry;

extern unsigned appIdPolicyId;
extern uint32_t app_id_netmasks[];

int appMatcherIsAppDetected(void *appSet, tAppId app);
int AppIdCommonInit(struct AppidStaticConfig *config);
int AppIdCommonFini(void);

/**
 * \brief Reload AppId configuration
 *
 * This function reloads AppId configuration. It is used both in the cases of Snort reload
 * and AppId reconfiguration.
 *
 * @param rnaConf - RNA configuration file name with full path
 * @param new_context - return reference that points to new AppId configuration
 * @return 0 on success, -1 on failure
 */
int AppIdCommonReload(struct AppidStaticConfig* appidSC, void **new_context);

/**
 * \brief Swap AppId configuration
 *
 * This function swaps AppId configuration. This function is called after AppIdCommonReload().
 *
 * @param swap_config - Pointer to new configuration. This pointer is returned by AppIdCommonReload().
 * @return Pointer to old configuration
 */
void *AppIdCommonReloadSwap(void *new_context);

/**
 * \brief Clean up AppId configuration
 *
 * This function cleans up all the data structures in an AppId configuration. It does not clean up
 * any global data structures that are used by AppId and are outside the configuration. This
 * function is called after AppIdCommonReloadSwap().
 *
 * @param old_context - Pointer to old configuration. This pointer is returned by AppIdCommonReloadSwap().
 * @return None
 */
void AppIdCommonUnload(void *old_context);

void *AppIDFlowdataGet(struct AppIdData *flow, unsigned id);

#endif  /* __COMMON_APP_MATCHER_H__ */

