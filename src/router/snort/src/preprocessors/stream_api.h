/* $Id$ */

/*
 * ** Copyright (C) 2005-2011 Sourcefire, Inc.
 * ** AUTHOR: Steven Sturges
 * **
 * ** This program is free software; you can redistribute it and/or modify
 * ** it under the terms of the GNU General Public License Version 2 as
 * ** published by the Free Software Foundation.  You may not use, modify or
 * ** distribute this program under any other version of the GNU General
 * ** Public License.
 * **
 * ** This program is distributed in the hope that it will be useful,
 * ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 * ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * ** GNU General Public License for more details.
 * **
 * ** You should have received a copy of the GNU General Public License
 * ** along with this program; if not, write to the Free Software
 * ** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * */

/* stream_api.h
 *
 * Purpose: Definition of the StreamAPI.  To be used as a common interface
 *          for TCP (and later UDP & ICMP) Stream access for other 
 *          preprocessors and detection plugins.
 *
 * Arguments:
 *
 * Effect:
 *
 * Comments:
 *
 * Any comments?
 *
 */

#ifndef STREAM_API_H_
#define STREAM_API_H_

#include <sys/types.h>

#include "ipv6_port.h"
#include "preprocids.h" /* IDs are used when setting preproc specific data */
#include "bitop.h"
#include "decode.h"
#include "sfPolicy.h"

#define IGNORE_FLAG_ALWAYS 0x01

#define SSN_MISSING_NONE   0x00
#define SSN_MISSING_BEFORE 0x01
#define SSN_MISSING_AFTER  0x02
#define SSN_MISSING_BOTH   (SSN_MISSING_BEFORE | SSN_MISSING_AFTER)

#define SSN_DIR_NONE 0x0
#define SSN_DIR_CLIENT 0x1
#define SSN_DIR_SENDER 0x1
#define SSN_DIR_SERVER 0x2
#define SSN_DIR_RESPONDER 0x2
#define SSN_DIR_BOTH 0x03

#define SSNFLAG_SEEN_CLIENT         0x00000001
#define SSNFLAG_SEEN_SENDER         0x00000001
#define SSNFLAG_SEEN_SERVER         0x00000002
#define SSNFLAG_SEEN_RESPONDER      0x00000002
#define SSNFLAG_ESTABLISHED         0x00000004
#define SSNFLAG_NMAP                0x00000008
#define SSNFLAG_ECN_CLIENT_QUERY    0x00000010
#define SSNFLAG_ECN_SERVER_REPLY    0x00000020
#define SSNFLAG_HTTP_1_1            0x00000040 /* has stream seen HTTP 1.1? */
#define SSNFLAG_SEEN_PMATCH         0x00000080 /* seen pattern match? */
#define SSNFLAG_MIDSTREAM           0x00000100 /* picked up midstream */
#define SSNFLAG_CLIENT_FIN          0x00000200 /* server sent fin */
#define SSNFLAG_SERVER_FIN          0x00000400 /* client sent fin */
#define SSNFLAG_CLIENT_PKT          0x00000800 /* packet is from the client */
#define SSNFLAG_SERVER_PKT          0x00001000 /* packet is from the server */
#define SSNFLAG_COUNTED_INITIALIZE  0x00002000
#define SSNFLAG_COUNTED_ESTABLISH   0x00004000
#define SSNFLAG_COUNTED_CLOSING     0x00008000
#define SSNFLAG_TIMEDOUT            0x00010000
#define SSNFLAG_PRUNED              0x00020000
#define SSNFLAG_RESET               0x00040000
#define SSNFLAG_DROP_CLIENT         0x00080000
#define SSNFLAG_DROP_SERVER         0x00100000
#define SSNFLAG_LOGGED_QUEUE_FULL   0x00200000
#define SSNFLAG_ALL                 0xFFFFFFFF /* all that and a bag of chips */
#define SSNFLAG_NONE                0x00000000 /* nothing, an MT bag of chips */

#define STREAM_FLPOLICY_NONE            0x00
#define STREAM_FLPOLICY_FOOTPRINT       0x01 /* size-based footprint flush */
#define STREAM_FLPOLICY_LOGICAL         0x02 /* queued bytes-based flush */
#define STREAM_FLPOLICY_RESPONSE        0x03 /* flush when we see response */
#define STREAM_FLPOLICY_SLIDING_WINDOW  0x04 /* flush on sliding window */
#if 0
#define STREAM_FLPOLICY_CONSUMED        0x05 /* purge consumed bytes */
#endif
#define STREAM_FLPOLICY_IGNORE          0x06 /* ignore this traffic */

#define STREAM_FLPOLICY_MAX STREAM_FLPOLICY_IGNORE

#define STREAM_FLPOLICY_SET_ABSOLUTE    0x01
#define STREAM_FLPOLICY_SET_APPEND      0x02

#define UNKNOWN_PORT 0

#define STREAM_API_VERSION5 5

typedef void (*StreamAppDataFree)(void *);
typedef int (*PacketIterator)
    (
     DAQ_PktHdr_t *,
     uint8_t *,  /* pkt pointer */
     void *      /* user-defined data pointer */
    );

typedef int (*StreamSegmentIterator)
    (
     DAQ_PktHdr_t *,
     uint8_t *,  /* pkt pointer */
     uint8_t *,  /* payload pointer */
     uint32_t,   /* sequence number */
     void *      /* user-defined data pointer */
    );

typedef struct _StreamFlowData
{
    BITOP boFlowbits;
    unsigned char flowb[1];
} StreamFlowData;

typedef struct _stream_api
{
    int version;

    /*
     * Drop on Inline Alerts for Midstream pickups
     *
     * Parameters
     *,
     * Returns
     *     0 if not alerting
     *     !0 if alerting
     */
    int (*alert_inline_midstream_drops)(void);

    /* Set direction of session
     *
     * Parameters:
     *     Session Ptr
     *     New Direction
     *     IP
     *     Port
     */
    void (*update_direction)(void *, char, snort_ip_p, uint16_t );

    /* Get direction of packet
     *
     * Parameters:
     *     Packet
     */
    uint32_t (*get_packet_direction)(Packet *);

    /* Stop inspection for session, up to count bytes (-1 to ignore
     * for life or until resume).
     *
     * If response flag is set, automatically resume inspection up to
     * count bytes when a data packet in the other direction is seen.
     *
     * Also marks the packet to be ignored
     *
     * Parameters
     *     Session Ptr
     *     Packet
     *     Direction
     *     Bytes
     *     Response Flag
     */
    void (*stop_inspection)(void *, Packet *, char, int32_t, int);

    /* Turn off inspection for potential session.
     * Adds session identifiers to a hash table.
     * TCP only.
     *
     * Parameters
     *     IP addr #1
     *     Port #1
     *     IP addr #2
     *     Port #2
     *     Protocol
     *     Direction
     *     Flags (permanent)
     *
     * Returns
     *     0 on success
     *     -1 on failure
     */
    int (*ignore_session)(snort_ip_p, uint16_t, snort_ip_p, uint16_t,
                          char, char, char);

    /* Resume inspection for session.
     *
     * Parameters
     *     Session Ptr
     *     Direction
     */
    void (*resume_inspection)(void *, char);

    /* Drop traffic arriving on session.
     *
     * Parameters
     *     Session Ptr
     *     Direction
     */
    void (*drop_traffic)(Packet *, void *, char);

    /* Drop retransmitted packet arriving on session.
     *
     * Parameters
     *     Packet
     */
    void (*drop_packet)(Packet *);

    /* Set a reference to application data for a session
     *
     * Parameters
     *     Session Ptr
     *     Application Protocol
     *     Application Data reference (pointer)
     *     Application Data free function
     *
     * Returns
     *     0 on success
     *     -1 on failure
     */
    int (*set_application_data)(void *, uint32_t, void *, StreamAppDataFree);

    /* Set a reference to application data for a session
     *
     * Parameters
     *     Session Ptr
     *     Application Protocol
     *
     * Returns
     *     Application Data reference (pointer)
     */
    void *(*get_application_data)(void *, uint32_t);

    /* Sets the flags for a session
     * This ORs the supplied flags with the previous values
     * 
     * Parameters
     *     Session Ptr
     *     Flags
     *
     * Returns
     *     New Flags
     */
    uint32_t (*set_session_flags)(void *, uint32_t);

    /* Gets the flags for a session
     *
     * Parameters
     *     Session Ptr
     */
    uint32_t (*get_session_flags)(void *);

    /* Flushes the stream on an alert
     * Side that is flushed is the same as the packet.
     *
     * Parameters
     *     Packet
     */
    int (*alert_flush_stream)(Packet *);

    /* Flushes the stream on arrival of another packet
     * Side that is flushed is the opposite of the packet.
     *
     * Parameters
     *     Packet
     */
    int (*response_flush_stream)(Packet *);

    /* Calls user-provided callback function for each packet of
     * a reassembled stream.  If the callback function returns non-zero,
     * iteration ends.
     *
     * Parameters
     *     Packet
     *     Packet Iterator Function (called for each packet in the stream)
     *     user data (may be NULL)
     *
     * Returns
     *     number of packets
     */
    int (*traverse_reassembled)(Packet *, PacketIterator, void *userdata);

    /* Calls user-provided callback function for each segment of
     * a reassembled stream.  If the callback function returns non-zero,
     * iteration ends.
     *
     * Parameters
     *     Packet
     *     StreamSegmentIterator Function (called for each packet in the stream)
     *     user data (may be NULL)
     *
     * Returns
     *     number of packets
     */
    int (*traverse_stream_segments)(Packet *, StreamSegmentIterator, void *userdata);

    /* Add session alert
     *
     * Parameters
     *     Session Ptr
     *     gen ID
     *     sig ID
     *
     * Returns
     *     0 success
     *     -1 failure (max alerts reached)
     *
     */
    int (*add_session_alert)(void *, Packet *p, uint32_t, uint32_t);

    /* Check session alert
     *
     * Parameters
     *     Session Ptr
     *     Packet
     *     gen ID
     *     sig ID
     *
     * Returns
     *     0 if not previously alerted
     *     !0 if previously alerted
     */
    int (*check_session_alerted)(void *, Packet *p, uint32_t, uint32_t);

    /* Get Flowbits data
     *
     * Parameters
     *     Packet
     *
     * Returns
     *     Ptr to Flowbits Data
     */
    StreamFlowData *(*get_flow_data)(Packet *p);

    /* Set reassembly flush policy/direction for given session
     *
     * Parameters
     *     Session Ptr
     *     Flush Policy
     *     Direction(s)
     *     Flags
     *
     * Returns
     *     direction(s) of reassembly for session
     */
    char (*set_reassembly)(void *, uint8_t, char, char);

    /* Get reassembly direction for given session
     *
     * Parameters
     *     Session Ptr
     *
     * Returns
     *     direction(s) of reassembly for session
     */
    char (*get_reassembly_direction)(void *);

    /* Get reassembly flush_policy for given session
     *
     * Parameters
     *     Session Ptr
     *     Direction
     *
     * Returns
     *     flush policy for specified direction
     */
    char (*get_reassembly_flush_policy)(void *, char);

    /* Get true/false as to whether stream data is in
     * sequence or packets are missing
     *
     * Parameters
     *     Session Ptr
     *     Direction
     *
     * Returns
     *     true/false
     */
    char (*is_stream_sequenced)(void *, char);

    /* Get whether there are missing packets before, after or
     * before and after reassembled buffer
     *
     * Parameters
     *      Session Ptr
     *      Direction
     *
     * Returns
     *      SSN_MISSING_BOTH if missing before and after
     *      SSN_MISSING_BEFORE if missing before
     *      SSN_MISSING_AFTER if missing after
     *      SSN_MISSING_NONE if none missing
     */
    int (*missing_in_reassembled)(void *, char);

    /* Get true/false as to whether packets were missed on
     * the stream
     *
     * Parameters
     *     Session Ptr
     *     Direction
     *
     * Returns
     *     true/false
     */
    char (*missed_packets)(void *, char);

#ifdef TARGET_BASED
    /* Get the protocol identifier from a stream
     *
     * Parameters
     *     Session Ptr
     * 
     * Returns
     *     integer protocol identifier
     */
    int16_t (*get_application_protocol_id)(void *);

    /* Set the protocol identifier for a stream
     *
     * Parameters
     *     Session Ptr
     *     ID
     * 
     * Returns
     *     integer protocol identifier
     */
    int16_t (*set_application_protocol_id)(void *, int16_t);

    /** Set service to either ignore, inspect or maintain session state.
     *  If this is called during parsing a preprocessor configuration, make
     *  sure to set the parsing argument to 1.
     */
    void (*set_service_filter_status)(int service, int status, tSfPolicyId policyId, int parsing);
#endif

    /** Set port to either ignore, inspect or maintain session state. 
     *  If this is called during parsing a preprocessor configuration, make
     *  sure to set the parsing argument to 1.
     */
    void (*set_port_filter_status)(int protocol, uint16_t port, int status, tSfPolicyId policyId, int parsing);

#ifdef ACTIVE_RESPONSE
    // initialize response count and expiration time
    void (*init_active_response)(Packet *, void *);
#endif

    // Get the TTL value used at session setup
    // outer=0 to get inner ip ttl for ip in ip; else outer=1
    uint8_t (*get_session_ttl)(void *ssnptr, char direction, int outer);

    /* Get the current flush point
     *
     * Arguments
     *  void * - session pointer
     *  char - direction
     *
     * Returns
     *  Current flush point for session
     */
    uint32_t (*get_flush_point)(void *, char);

    /* Set the next flush point
     *
     * Arguments
     *  void * - session pointer
     *  char - direction
     *  uint32_t - flush point size
     */
    void (*set_flush_point)(void *, char, uint32_t);

#ifdef TARGET_BASED
    /* Turn off inspection for potential session.
     * Adds session identifiers to a hash table.
     * TCP only.
     *
     * Parameters
     *     IP addr #1
     *     Port #1
     *     IP addr #2
     *     Port #2
     *     Protocol
     *     ID
     *
     * Returns
     *     0 on success
     *     -1 on failure
     */
    int (*set_application_protocol_id_expected)(snort_ip_p, uint16_t, snort_ip_p, uint16_t,
                          char, int16_t);
#endif
} StreamAPI;

/* To be set by Stream5 (or Stream4) */
extern StreamAPI *stream_api;

/**Port Inspection States. Port can be either ignored,
 * or inspected or session tracked. The values are bitmasks.
 */
typedef enum { 
    /**Dont monitor the port. */
    PORT_MONITOR_NONE = 0x00, 

    /**Inspect the port. */
    PORT_MONITOR_INSPECT = 0x01,

    /**perform session tracking on the port. */
    PORT_MONITOR_SESSION = 0x02

} PortMonitorStates;

#endif /* STREAM_API_H_ */

