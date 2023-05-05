/* $Id$ */
/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * ** Copyright (C) 2005-2013 Sourcefire, Inc.
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
 * ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

#include "ipv6_port.h"
#include "preprocids.h" /* IDs are used when setting preproc specific data */
#include "bitop.h"
#include "decode.h"
#include "sfPolicy.h"
#include "session_api.h"

typedef enum {
    STREAM_FLPOLICY_NONE,
    STREAM_FLPOLICY_FOOTPRINT,       /* size-based footprint flush */
    STREAM_FLPOLICY_LOGICAL,         /* queued bytes-based flush */
    STREAM_FLPOLICY_RESPONSE,        /* flush when we see response */
    STREAM_FLPOLICY_SLIDING_WINDOW,  /* flush on sliding window */
#if 0
    STREAM_FLPOLICY_CONSUMED,        /* purge consumed bytes */
#endif
    STREAM_FLPOLICY_IGNORE,          /* ignore this traffic */
    STREAM_FLPOLICY_PROTOCOL,        /* protocol aware flushing (PAF) */
#ifdef NORMALIZER
    STREAM_FLPOLICY_FOOTPRINT_IPS,   /* protocol agnostic ips */
    STREAM_FLPOLICY_PROTOCOL_IPS,    /* protocol aware ips */
#endif
    STREAM_FLPOLICY_FOOTPRINT_NOACK,    /* protocol aware ips */
    STREAM_FLPOLICY_PROTOCOL_NOACK,    /* protocol aware ips */
#ifdef NORMALIZER
    STREAM_FLPOLICY_FOOTPRINT_IPS_FTP,   /* protocol agnostic ips for FTP*/
#endif

    STREAM_FLPOLICY_DISABLED,       /* reassembly disabled for this traffic */

    STREAM_FLPOLICY_MAX
} FlushPolicy;

typedef enum _PreprocessorFlags{
    PP_FTPTELNET_FTPS                      = 0x00000001,
    PP_HTTPINSPECT_PAF_FLUSH_POST_HDR      = 0x00000002,
}PreprocessorFlags;

typedef enum {
    PAF_TYPE_SERVICE,
    PAF_TYPE_PORT
}PafType;

#define STREAM_FLPOLICY_SET_ABSOLUTE    0x01
#define STREAM_FLPOLICY_SET_APPEND      0x02

#define STREAM_API_VERSION5 6

typedef void (*LogExtraData)(void *ssnptr, void *config, LogFunction *funcs, uint32_t max_count, uint32_t xtradata_mask, uint32_t id, uint32_t sec);

typedef int (*PacketIterator)( DAQ_PktHdr_t *,
     uint8_t *,  /* pkt pointer */
     void *      /* user-defined data pointer */
    );

typedef int (*StreamSegmentIterator)(  DAQ_PktHdr_t *,
     uint8_t *,  /* pkt pointer */
     uint8_t *,  /* payload pointer */
     uint32_t,   /* sequence number */
     void *      /* user-defined data pointer */
    );


/* for protocol aware flushing (PAF): */
typedef enum {
    PAF_ABORT,   /* non-paf operation */
    PAF_START,   /* internal use only */
    PAF_SEARCH,  /* searching for next flush point */
    PAF_FLUSH,   /* flush at given offset */
    PAF_LIMIT,   /* if paf_max is reached, flush up to given offset*/
    PAF_SKIP,    /* skip ahead to given offset */
    PAF_PERFORMED_LMT_FLUSH, /* previously performed PAF_LIMIT  */
    PAF_DISCARD_START, /*start of the discard point */
    PAF_DISCARD_END, /*end of the discard point */
    PAF_PSEUDO_FLUSH_SEARCH, /* payload can be pseudo flushed before flushing */
    PAF_PSEUDO_FLUSH_SKIP, /* HTTP chunked payload can be pseudo flushed */
    PAF_IGNORE,  /* Used for HTTP2.0*/
} PAF_Status;

typedef PAF_Status (*PAF_Callback)(  /* return your scan state */
    void* session,         /* session pointer */
    void** user,           /* arbitrary user data hook */
    const uint8_t* data,   /* in order segment data as it arrives */
    uint32_t len,          /* length of data */
    uint64_t *flags,       /* packet flags indicating direction of data */
    uint32_t* fp,          /* flush point (offset) relative to data */
    uint32_t * fp_eoh      /* flush point (offset) at end-of-header */
);

typedef void (*PAF_Free_Callback)(
    void* user            /* arbitrary user data hook */
);

#if defined(FEAT_OPEN_APPID)
typedef struct s_HEADER_LOCATION {
    const uint8_t *start;
    unsigned len;
} HEADER_LOCATION;

typedef struct _HttpParsedHeaders
{
    HEADER_LOCATION host, url, method, userAgent, referer, via, responseCode, server, xWorkingWith, contentType;
} HttpParsedHeaders;

typedef void (*Http_Processor_Callback)(
    Packet *p,
    HttpParsedHeaders *headers
);
typedef enum {
    APP_PROTOID_SERVICE,
    APP_PROTOID_CLIENT,
    APP_PROTOID_PAYLOAD,
    APP_PROTOID_MISC,
    APP_PROTOID_MAX
} AppProtoIdIndex;
#endif /* defined(FEAT_OPEN_APPID) */

typedef  unsigned int ServiceEventType;

typedef void (*ServiceEventNotifierFunc)(void *ssnptr, ServiceEventType eventType, void *eventData);

typedef void (*Stream_Callback)(Packet *);

typedef void (*FTP_Processor_Flush_Callback)(Packet *p);

struct _ExpectNode;
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

    /* Flushes the stream on an alert
     * Side that is flushed is the same as the packet.
     *
     * Parameters
     *     Packet
     */
    int (*alert_flush_stream)(Packet *);

    /* Flushes the stream on arrival of packet
     * Side that is flushed is the same side of the packet.
     *
     * Parameters
     *     Packet
     */
    int (*request_flush_stream)(Packet *);

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

    /* Set Extra Data Logging
     *
     * Parameters
     *      Session Ptr
     *      Packet
     *      gen ID
     *      sig ID
     * Returns
     *      0 success
     *      -1 failure ( no alerts )
     *
     */
    int (*update_session_alert)(void *, Packet *p, uint32_t, uint32_t, uint32_t, uint32_t);

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
    /* XXX Do not attempt to set flush policy to PROTOCOL or PROTOCOL_IPS. */
    char (*set_reassembly)(void *, uint8_t, char, char);

    /* Set direction of session
     *
     * Parameters:
     *     Session Ptr
     *     New Direction
     *     IP
     *     Port
     */
    void (*update_direction)(void *, char, sfaddr_t*, uint16_t );

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

    /* Drop retransmitted packet arriving on session.
     *
     * Parameters
     *     Packet
     */
    void (*drop_packet)(Packet *);

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

    // register for stateful scanning of in-order payload to determine flush points
    // autoEnable allows PAF regardless of s5 ports config
    uint8_t (*register_paf_port)( struct _SnortConfig *sc, tSfPolicyId, uint16_t server_port, bool toServer,
        PAF_Callback, bool autoEnable);

    // get any paf user data stored for this session
    void** (*get_paf_user_data)(void* ssnptr, bool toServer, uint8_t id);

    bool (*is_paf_active)(void* ssn, bool toServer);
    bool (*activate_paf)(void* ssn, int dir, int16_t service, uint8_t type);

    /** Set flag to force sessions to be created on SYN packets.
     *  This function can only be used with independent bits
     *  acquired from get_preprocessor_status_bit. If this is called
     *  during parsing a preprocessor configuration, make sure to
     *  set the parsing argument to 1.
     */
    void (*set_tcp_syn_session_status)(struct _SnortConfig *sc, uint16_t status, tSfPolicyId policyId, int parsing);

    /** Unset flag that forces sessions to be created on SYN
     *  packets. This function can only be used with independent
     *  bits acquired from get_preprocessor_status_bit. If this is
     *  called during parsing a preprocessor configuration, make
     *  sure to set the parsing argument to 1.
     */
    void (*unset_tcp_syn_session_status)(struct _SnortConfig *sc, uint16_t status, tSfPolicyId policyId, int parsing);

    //Register callbacks for extra data logging
    uint32_t (*reg_xtra_data_cb)(LogFunction );

    //Register Extra Data Log Function
    void (*reg_xtra_data_log)(LogExtraData, void *);

    //Get the Extra data map
    uint32_t (*get_xtra_data_map)(LogFunction **);

    // register for stateful scanning of in-order payload to determine flush points
    // autoEnable allows PAF regardless of s5 ports config
    uint8_t (*register_paf_service)(
        struct _SnortConfig *sc, tSfPolicyId, uint16_t service, bool toServer,
        PAF_Callback, bool autoEnable);

    void (*set_extra_data)(void*, Packet *, uint32_t);
    void (*clear_extra_data)(void*, Packet *, uint32_t);

// These methods may move to Session:
//
    /* Set port to either ignore, inspect or maintain session state.
     * If this is called during parsing a preprocessor configuration, make
     * sure to set the parsing argument to 1.
     */
    void (*set_port_filter_status)(struct _SnortConfig *sc, IpProto protocol, uint16_t port, uint16_t status,
                                   tSfPolicyId policyId, int parsing);

    /* Unset port to maintain session state. This function can only
     *  be used with independent bits acquired from
     *  get_preprocessor_status_bit. If this is called during
     *  parsing a preprocessor configuration, make sure to set the
     *  parsing argument to 1.
     */
    void (*unset_port_filter_status)(struct _SnortConfig *sc, IpProto protocol, uint16_t port, uint16_t status,
                                      tSfPolicyId policyId, int parsing);


    /* Set service to either ignore, inspect or maintain session state.
     * If this is called during parsing a preprocessor configuration, make
     * sure to set the parsing argument to 1.
     */
    void (*set_service_filter_status)( struct _SnortConfig *sc, int service, int status,
                                       tSfPolicyId policyId, int parsing );

    /* Register specified port for reassembly on specified network.  If network is NULL the
     *  port is register for reassembly on the default stream network policy
     */
    void (*register_reassembly_port)( char *, uint16_t, int );

    /* Unregister specified port for reassembly on specified network.  If network is NULL the
     *  port is unregistered for reassembly on the default stream network policy
     */
    void (*unregister_reassembly_port)( char *, uint16_t, int );

    /* Time out the specified session.
     *
     * Parameters
     *     Session Ptr
     */
    void (*expire_session)(void *);
    void (*force_delete_session)(void *);

    /* register returns a non-zero id for use with set; zero is error */
    unsigned (*register_event_handler)(Stream_Callback);
    bool (*set_event_handler)(void* ssnptr, unsigned id, Stream_Event);
    void (*set_reset_policy)(void* ssn, int dir, uint16_t policy, uint16_t mss);
    void (*set_session_decrypted)(void *ssn, bool enable);
    bool (*is_session_decrypted)(void *ssn);

    /* Turn off inspection for potential session.
     * Adds session identifiers to a hash table.
     * TCP only.
     *
     * Parameters
     *     Control Channel Packet
     *     IP addr #1
     *     Port #1
     *     IP addr #2
     *     Port #2
     *     Protocol
     *     ID,
     *     Preprocessor ID calling this function,
     *     Preprocessor specific data,
     *     Preprocessor data free function. If NULL, then static buffer is assumed.
     *     Preprocessor event handler callback ID (used when calling set_event_handler)
     *     Preprocessor event on which to callback (only used when cbId is not NULL )
     *
     * Returns
     *     0 on success
     *     -1 on failure
     */
    int (*set_application_protocol_id_expected_preassign_callback)(const Packet *, sfaddr_t*, uint16_t,
                sfaddr_t*, uint16_t, uint8_t, int16_t, uint32_t, void*, void (*)(void*), unsigned, Stream_Event,
                struct _ExpectNode**);

    // print and reset normalization statistics
    void (*print_normalization_stats)(void);
    void (*reset_normalization_stats)(void);

#if defined(FEAT_OPEN_APPID)
    /* set detected service, client, payload and misc Applicaiton Id.
     *
     * Parameters
     *      Session Ptr
     *      Snort Protocol Id for service application
     *      Snort Protocol Id for client application
     *      Snort Protocol Id for payload application
     *      Snort Protocol Id for misc application
     */
    void (*set_application_id)(void* ssnptr, int16_t serviceAppid, int16_t clientAppid, int16_t payloadAppId, int16_t miscAppid);

    /* get detected service, client, payload and misc Applicaiton Id.
     *
     * Parameters
     *      Session Ptr
     *      Snort Protocol Id for service application
     *      Snort Protocol Id for client application
     *      Snort Protocol Id for payload application
     *      Snort Protocol Id for misc application
     */
    void (*get_application_id)(void* ssnptr, int16_t *serviceAppid, int16_t *clientAppid, int16_t *payloadAppId, int16_t *miscAppid);


    /* Register callback function for processing HTTP headers extracted by HTTP preprocessor.
     *
     * Parameters
     *      Callback function pointer
     */
    int (*register_http_header_callback)(Http_Processor_Callback);
#endif /* defined(FEAT_OPEN_APPID) */

    /* function to publish events
     *
     * Parameters
     *      preprocId - preprocess identifier
     *      ssnptr - sesssion pointer
     *      eventType - type of event enumerated in ServiceEventType
     *      eventData - void data pointer. Structure must be agreed between publisher and subscriber.
     */
    bool (*service_event_publish)(unsigned int preprocId, void *ssnptr, ServiceEventType eventType, void *eventData);

    /* function for subcribing to events.
     *
     * Parameters
     *      preprocId - preprocess identifier
     *      eventType - type of event enumerated in ServiceEventType
     *      Callback function pointer
     */
    bool (*service_event_subscribe)(unsigned int preprocId, ServiceEventType eventType, ServiceEventNotifierFunc cb);

    /* function to register for customized free function
     *
     * Parameters
     *      id - registered paf identifier
     *      Callback function pointer
     */
    void (*register_paf_free)(uint8_t id, PAF_Free_Callback);

    /* function to return the wire packet 
     *
     * Parameters
     *      None
     */
    Packet *(*get_wire_packet)(void);
    
    /* function which returns the forward dir or reverse dir to h2_paf 
     *
     * Parameter
     *      None
     */
    uint8_t (*get_flush_policy_dir)(void);
    
    /* function returns if its a http/2 session
     *
     * Parameters
     *      Session Pointer
     */
    bool (*is_session_http2)(void *ssn);

     /* function sets http/2 session flag
     *
     * Parameters
     *      Session Pointer
     */
    void (*set_session_http2)(void *ssn);

    bool (*is_show_rebuilt_packets_enabled)();
    /* function returns if its a http/2 session Upgrade
     *
     * Parameters
     *      Session Pointer
     */
    bool (*is_session_http2_upg)(void *ssn);

     /* function sets http/2 session Upgrade flag
     *
     * Parameters
     *      Session Pointer
     */
    void (*set_session_http2_upg)(void *ssn);

    /* Gets the proto_flags for a session
     *
     * Parameters
     *    ssnptr - sesssion pointer
     */
    uint32_t (*get_preproc_flags)(void *ssnptr);

    /* Register the FTP Flush callback
     *
     * Parameters
     *   ssnptr - sesssion pointer
     */
    int (*register_ftp_flush_cb)(FTP_Processor_Flush_Callback);

    /* set the ftp file position
     *
     * parameters
     *   ssnptr -session pointer
     *   flage -flush flage
     */
    void (*set_ftp_file_position)(void *ssnptr, bool flush);

#ifdef HAVE_DAQ_DECRYPTED_SSL
    /* Simulate ACK in peer Streamtracker
     *
     * Parameters
     *   ssnptr - sesssion pointer
     *   dir    - direction to be ACKed in( 0:C to S, 1:S to C )
     *   len    - Number of bytes to be ACKed
     */
    int (*simulate_tcp_akc_in_peer_stream_tracker)(void *ssnptr, uint8_t dir, uint32_t len);
#endif
} StreamAPI;
/* To be set by Stream */
extern StreamAPI *stream_api;

#endif /* STREAM_API_H_ */

