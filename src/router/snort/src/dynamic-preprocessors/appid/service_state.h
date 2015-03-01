/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
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


#ifndef _SERVICE_STATE_H_
#define _SERVICE_STATE_H_

#include <stdint.h>
#include <ipv6_port.h>
#include "host_tracker.h"

#define APP_ID_MAX_VALID_COUNT  5
#define APP_ID_NEEDED_DUPE_DETRACT_COUNT    3

/**Service state saved in hosttracker, for identifying a service across multiple flow instances.
 */
struct _RNA_SERVICE_ELEMENT;
struct _SERVICE_MATCH;
typedef struct _APP_ID_SERVICE_ID_STATE
{
    const struct _RNA_SERVICE_ELEMENT *svc;

    /**Number of consequetive flows that detectors have failed to identify service for.
     */
    unsigned invalid_count;

    /**State of service identification.*/
    SERVICE_ID_STATE state;
    unsigned valid_count;
    unsigned detract_count;
    snort_ip last_detract;

    /**Number of consequetive flows that were declared incompatible by detectors. Incompatibility
     * means client packet did not match.
     */
    unsigned invalid_client_count;

    /**IP address of client in last flow that was declared incompatible. If client IP address is
     * different everytime, then consequetive incompatible status indicate that flow is not using
     * specific service.
     */
    snort_ip last_invalid_client;

    /** Count for number of unknown sessions saved
     */
    unsigned unknowns_logged;
    time_t reset_time;

    /**List of ServiceMatch nodes which are sorted in order of pattern match. The list is contructed
     * once on first packet from server and then used for subsequent flows. This saves repeat pattern
     * matching, but has the disadvantage of making one flow match dependent on first instance of the
     * same flow.
     */
    struct _SERVICE_MATCH *serviceList;
    struct _SERVICE_MATCH *currentService;
} AppIdServiceIDState;

typedef struct
{
    uint16_t port;
    uint16_t proto;
    uint32_t ip;
} AppIdServiceStateKey4;

typedef struct
{
    uint16_t port;
    uint16_t proto;
    uint8_t ip[16];
} AppIdServiceStateKey6;

typedef union
{
    AppIdServiceStateKey4 key4;
    AppIdServiceStateKey6 key6;
} AppIdServiceStateKey;

int AppIdServiceStateInit(unsigned long memcap);
void AppIdServiceStateCleanup(void);
void AppIdRemoveServiceIDState(snort_ip *ip, uint16_t proto, uint16_t port);
AppIdServiceIDState* AppIdGetServiceIDState(snort_ip *ip, uint16_t proto, uint16_t port);
AppIdServiceIDState* AppIdAddServiceIDState(snort_ip *ip, uint16_t proto, uint16_t port);
void AppIdServiceStateDumpStats(void);

#endif

