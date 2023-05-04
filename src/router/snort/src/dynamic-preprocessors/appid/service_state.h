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


#ifndef _SERVICE_STATE_H_
#define _SERVICE_STATE_H_

#include <stdint.h>
#include <stdbool.h>
#include <ipv6_port.h>
#include <string.h>

/**Service state stored in hosttracker for maintaining service matching states.
 */
typedef enum
{
    /**first search of service. The matching criteria is coded in ProtocolID funtion.
     */
    SERVICE_ID_NEW = 0,

    /**service is already detected and valid.
     */
    SERVICE_ID_VALID,

    /**match based on round-robin through tcpServiceList or UdpServiceList. RNA walks
     * the list from first element to last. In a detector declares a flow incompatible
     * or the flow closes earlier than expected by detector, then the next detector is
     * tried. This can obviously delay detection under some scenarios.
     */
    SERVICE_ID_BRUTE_FORCE,

    /**if brute-force failed after a complete walk of the list,
     * we stop searching and move into this state.
     */
    SERVICE_ID_BRUTE_FORCE_FAILED,

} SERVICE_ID_STATE;

/* Service state stored per flow, which acts based on global SERVICE_ID_STATE
 * at the beginning of the flow, then independently do service discovery, and
 * synchronize findings at the end of service discovery by the flow.
 */
typedef enum
{
    /* First attempt in search of service. */
    SERVICE_ID_START = 0,

    /* Match based on source or destination port in first packet in flow. */
    SERVICE_ID_PORT,

    /* Match based on pattern in first response from server or
     * client in case of client_services. */
    SERVICE_ID_PATTERN,

    /* Flow is in this state after we retrieve all port/pattern candidates. */
    SERVICE_ID_PENDING,

} FLOW_SERVICE_ID_STATE;

#define DETECTOR_TYPE_PASSIVE   0
#define DETECTOR_TYPE_DECODER   0
#define DETECTOR_TYPE_NETFLOW   1
#define DETECTOR_TYPE_PORT      2
#define DETECTOR_TYPE_DERIVED   3
#define DETECTOR_TYPE_CONFLICT  4
#define DETECTOR_TYPE_PATTERN   5

/**We will NOT hold onto pointers to host tracker entries (because they could be pruned).
 * When a session starts discovery, it retrieves the last-known id_state at the beginning of the session,
 * does independent service discovery, and
 * reconciles any changes back to the entry once a service determination is made.
 */
struct RNAServiceElement;
struct _SERVICE_MATCH;
typedef struct _APP_ID_SERVICE_ID_STATE
{
    const struct RNAServiceElement *svc;

    /**State of service identification.*/
    SERVICE_ID_STATE state;
    unsigned valid_count;
    unsigned detract_count;
    sfaddr_t last_detract;

    /**Number of consequetive flows that were declared incompatible by detectors. Incompatibility
     * means client packet did not match.
     */
    unsigned invalid_client_count;

    /**IP address of client in last flow that was declared incompatible. If client IP address is
     * different everytime, then consequetive incompatible status indicate that flow is not using
     * specific service.
     */
    sfaddr_t last_invalid_client;

#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)   
    uint16_t asId;
#endif

    /** Count for number of unknown sessions saved
     */
    unsigned unknowns_logged;
    time_t reset_time;

} AppIdServiceIDState;

typedef struct
{
    uint16_t  port;
    uint16_t  proto;
    uint32_t  ip;
    uint32_t  level;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)   
    uint16_t  asId;      
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
    uint32_t cid;
#endif
} AppIdServiceStateKey4;

typedef struct
{
    uint16_t  port;
    uint16_t  proto;
    uint8_t   ip[16];
    uint32_t  level;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)   
    uint16_t  asId;    
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
    uint32_t cid;
#endif
} AppIdServiceStateKey6;

typedef union
{
    AppIdServiceStateKey4 key4;
    AppIdServiceStateKey6 key6;
} AppIdServiceStateKey;

bool AppIdServiceStateReloadAdjust(bool idle, unsigned long memcap);
int AppIdServiceStateInit(unsigned long memcap);
void AppIdServiceStateCleanup(void);
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
void AppIdRemoveServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port, 
        uint32_t level, uint16_t asId, uint32_t cid);
AppIdServiceIDState* AppIdGetServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port,
        uint32_t level, uint16_t asId, uint32_t cid);
AppIdServiceIDState* AppIdAddServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port,
        uint32_t level, uint16_t asId, uint32_t cid);
#else
void AppIdRemoveServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port, uint32_t level, uint32_t cid);
AppIdServiceIDState* AppIdGetServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port, uint32_t level, uint32_t cid);
AppIdServiceIDState* AppIdAddServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port, uint32_t level, uint32_t cid);
#endif
#else /* No carrier id */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
void AppIdRemoveServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port,
        uint32_t level, uint16_t asId);
AppIdServiceIDState* AppIdGetServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port,
        uint32_t level, uint16_t asId);
AppIdServiceIDState* AppIdAddServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port,
        uint32_t level, uint16_t asId);
#else
void AppIdRemoveServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port, uint32_t level);
AppIdServiceIDState* AppIdGetServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port, uint32_t level);
AppIdServiceIDState* AppIdAddServiceIDState(sfaddr_t *ip, uint16_t proto, uint16_t port, uint32_t level);
#endif
#endif
void AppIdServiceStateDumpStats(void);

#endif

