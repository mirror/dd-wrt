/*
**
**  reload.h
**
**  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
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

#ifndef __RELOAD_H__
#define __RELOAD_H__

#include <time.h>
#include "snort.h"
#include "util.h"
#include "reload_api.h"
#include "sf_types.h"

void CheckForReload(void);

void ReloadControlSocketRegister(void);

bool SnortDynamicLibsChanged(void);

#if defined(SNORT_RELOAD) && !defined(WIN32)
void * ReloadConfigThread(void *);
#endif

#if defined(SNORT_RELOAD)
typedef struct _ReloadAdjustEntry
{
    struct _ReloadAdjustEntry* raNext;
    const char* raName;
    tSfPolicyId raPolicyId;
    ReloadAdjustFunc raFunc;
    void* raUserData;
    ReloadAdjustUserFreeFunc raUserFreeFunc;
} ReloadAdjustEntry;

int ReloadAdjustRegister(SnortConfig* sc, const char* raName, tSfPolicyId raPolicyId,
                         ReloadAdjustFunc raFunc, void* raUserData,
                         ReloadAdjustUserFreeFunc raUserFreeFunc);
int ReloadAdjustSessionRegister(SnortConfig* sc, const char* raName, tSfPolicyId raPolicyId,
                                ReloadAdjustFunc raFunc, void* raUserData,
                                ReloadAdjustUserFreeFunc raUserFreeFunc);

void ReloadFreeAdjusters(SnortConfig* sc);

static inline void ReloadAdjust(bool idle, time_t tv_sec)
{
    ReloadAdjustEntry* rae = snort_conf->raEntry;

    if (rae)
    {
        static unsigned num_idle_callbacks = 0;
        static unsigned num_packet_callbacks = 0;
        static struct timeval adjust_started_at;

        if (rae != snort_conf->raCurrentEntry)
        {
            snort_conf->raCurrentEntry = rae;
            snort_conf->raLastLog = tv_sec;
            memset(&adjust_started_at, 0 , sizeof(adjust_started_at));
            gettimeofday(&adjust_started_at, NULL);
            LogMessage("Adjusting %s during reload ( Started at : (%"PRIu64") milliseconds.\n", rae->raName, 
                    (uint64_t)(adjust_started_at.tv_sec * 1000 + adjust_started_at.tv_usec / 1000));
        }
        if (idle)
            num_idle_callbacks++;
        else
            num_packet_callbacks++;

        if (rae->raFunc(idle, rae->raPolicyId, rae->raUserData))
        {
            struct timeval curr_time;
            gettimeofday(&curr_time, NULL);
            LogMessage("Finished adjusting memory for %s : Total calls (packet loop=%u, idle loop=%u), Elapsed time = (%"PRIu64") milliseconds.\n",
                    rae->raName, num_packet_callbacks, num_idle_callbacks,
                    (uint64_t)((curr_time.tv_sec * 1000 + curr_time.tv_usec / 1000) - (adjust_started_at.tv_sec * 1000 + adjust_started_at.tv_usec / 1000)) );
            num_idle_callbacks = 0;
            num_packet_callbacks = 0;
            memset(&adjust_started_at, 0 , sizeof(adjust_started_at));
            snort_conf->raEntry = rae->raNext;
            free(rae);
        }
        else if (!snort_conf->raLastLog)
            snort_conf->raLastLog = tv_sec;
        else if ((unsigned)(tv_sec - snort_conf->raLastLog) >= 10)
        {
            snort_conf->raLastLog = tv_sec;
            WarningMessage("Waiting for %s to adjust for reload (packet loop=%u, idle loop=%u).\n", rae->raName, num_packet_callbacks, num_idle_callbacks);
        }
    }
}
#endif

extern SnortConfig *snort_conf_new;

#endif

