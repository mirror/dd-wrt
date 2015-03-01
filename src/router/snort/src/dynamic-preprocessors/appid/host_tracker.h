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


#ifndef __HOST_TRACKER_H__
#define __HOST_TRACKER_H__

#define STATE_ID_INVALID_THRESHOLD  2
#define STATE_ID_INVALID_CLIENT_THRESHOLD  9

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

    /**match based on source or destination port in first packet in flow.
     */
    SERVICE_ID_PORT,

    /**match based on pattern in first response from server or client in
     * case of client_services.
     */
    SERVICE_ID_PATTERN,

    /**match based on round-robin through tcpServiceList or UdpServiceList. RNA walks
     * the list from first element to last. In a detector declares a flow incompatible
     * or the flow closes earlier than expected by detector, then the next detector is
     * tried. This can obviously delay detection under some scenarios.
     */
    SERVICE_ID_BRUTE_FORCE,

    /**Same as BRUTE_FORCE, but after the first complete walk of the list.  Continue
     * to round-robin throguh the list infinitely.
     */
    SERVICE_ID_BRUTE_FORCE_FAILED
} SERVICE_ID_STATE;

#define RNA_UNKNOWN_LOG_SESSIONS    5

/**Service state saved in hosttracker, for identifying a service across multiple flow instances.
 */
struct _RNA_SERVICE_ELEMENT;
struct _SERVICE_MATCH;
typedef struct _SERVICE_ID_STATE
{
    struct _RNA_SERVICE_ELEMENT *svc;

    /**Number of consequetive flows that detectors have failed to identify service for.
     */
    unsigned invalid_count;

    /**State of service identification.*/
    SERVICE_ID_STATE state;
    int confidence_confirm;
    int confidence_detract;
    u_int32_t last_detract;

    /**Number of consequetive flows that were declared incompatible by detectors. Incompatibility
     * means client packet did not match.
     */
    unsigned invalid_client_count;

    /**IP address of client in last flow that was declared incompatible. If client IP address is
     * different everytime, then consequetive incompatible status indicate that flow is not using
     * specific service.
     */
    u_int32_t last_invalid_client;

    /** Count for number of unknown sessions saved
     */
    unsigned unknowns_logged;
    time_t reset_time;
    int client_service;

    /**List of ServiceMatch nodes which are sorted in order of pattern match. The list is contructed
     * once on first packet from server and then used for subsequent flows. This saves repeat pattern
     * matching, but has the disadvantage of making one flow match dependent on first instance of the
     * same flow.
     */
    struct _SERVICE_MATCH *serviceList;
} ServiceIDState;

#define DETECTOR_TYPE_PASSIVE   0
#define DETECTOR_TYPE_DECODER   0
#define DETECTOR_TYPE_NETFLOW   1
#define DETECTOR_TYPE_PORT      2
#define DETECTOR_TYPE_DERIVED   3
#define DETECTOR_TYPE_CONFLICT  4
#define DETECTOR_TYPE_PATTERN   5

#endif /* __HOST_TRACKER_H__ */
