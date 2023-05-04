/* $Id$ */

/*
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2004-2013 Sourcefire, Inc.
 * ** AUTHOR: d mcpherson
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

/* session_api.h
 *
 * Purpose: Definition of the SessionAPI.  To be used as a common interface
 *          for other preprocessors and detection plugins that require a
 *          session context for execution.
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

#ifndef _SESSION_API_H_
#define _SESSION_API_H_

#include <sys/types.h>

#include "ipv6_port.h"
#include "preprocids.h" /* IDs are used when setting preproc specific data */
#include "bitop.h"
#include "decode.h"
#include "sfPolicy.h"

/* default limits */
#define STREAM_DEFAULT_PRUNE_QUANTA  30       /* seconds to timeout a session */
#define STREAM_DEFAULT_MEMCAP        8388608  /* 8MB */
#define STREAM_DEFAULT_PRUNE_LOG_MAX 1048576  /* 1MB */
#define STREAM_RIDICULOUS_HI_MEMCAP  ( 1024 * 1024 * 1024 ) /* 1GB */
#define STREAM_RIDICULOUS_LOW_MEMCAP 32768    /* 32k*/
#define STREAM_RIDICULOUS_MAX_SESSIONS ( 1024 * 1024 ) /* 1 million sessions */
#define STREAM_DEFAULT_MAX_TCP_SESSIONS 262144 /* 256k TCP sessions by default */
#define STREAM_DEFAULT_MAX_UDP_SESSIONS 131072 /* 128k UDP sessions by default */
#define STREAM_DEFAULT_MAX_ICMP_SESSIONS 65536 /* 64k ICMP sessions by default */
#define STREAM_DEFAULT_MAX_IP_SESSIONS   16384 /* 16k IP sessions by default */
#define STREAM_DEFAULT_TCP_CACHE_PRUNING_TIMEOUT    30            /*  30 seconds */
#define STREAM_DEFAULT_TCP_CACHE_NOMINAL_TIMEOUT    ( 60 * 60 )   /*  60 minutes */
#define STREAM_DEFAULT_UDP_CACHE_PRUNING_TIMEOUT    30            /*  30 seconds */
#define STREAM_DEFAULT_UDP_CACHE_NOMINAL_TIMEOUT    ( 3 * 60 )    /*  3 minutes */
#define STREAM_MAX_CACHE_TIMEOUT                    ( 12 * 60 * 60 )  /*  12 hours */
#define STREAM_MIN_PRUNE_LOG_MAX     1024      /* 1k packet data stored */
#define STREAM_MAX_PRUNE_LOG_MAX     STREAM_RIDICULOUS_HI_MEMCAP  /* 1GB packet data stored */
#define STREAM_DELAY_SESSION_DELETION true   /* set if session deletion to be delayed */
#define STREAM_DELAY_TIMEOUT_AFTER_CONNECTION_ENDED   (3 * 60)    /*  3 minutes */
#define STREAM_DELAY_SCB_DELETION                      1          /* 1 second */

#define STREAM_EXPECTED_CHANNEL_TIMEOUT 300

#ifdef ACTIVE_RESPONSE
#define STREAM_DEFAULT_MAX_ACTIVE_RESPONSES  0   /* default to no responses */
#define STREAM_DEFAULT_MIN_RESPONSE_SECONDS  1   /* wait at least 1 second between resps */

#define STREAM_MAX_ACTIVE_RESPONSES_MAX      25  /* banging your head against the wall */
#define STREAM_MIN_RESPONSE_SECONDS_MAX      300 /* we want to stop the flow soonest */
#endif

#define EXPECT_FLAG_ALWAYS 0x01

#define SSN_MISSING_NONE   0x00
#define SSN_MISSING_BEFORE 0x01
#define SSN_MISSING_AFTER  0x02
#define SSN_MISSING_BOTH   (SSN_MISSING_BEFORE | SSN_MISSING_AFTER)

#define SSN_DIR_NONE           0x0
#define SSN_DIR_FROM_CLIENT    0x1
#define SSN_DIR_FROM_SENDER    0x1
#define SSN_DIR_TO_SERVER      0x1
#define SSN_DIR_FROM_SERVER    0x2
#define SSN_DIR_FROM_RESPONDER 0x2
#define SSN_DIR_TO_CLIENT      0x2
#define SSN_DIR_BOTH           0x3

#define SSNFLAG_SEEN_CLIENT         0x00000001
#define SSNFLAG_SEEN_SENDER         0x00000001
#define SSNFLAG_SEEN_SERVER         0x00000002
#define SSNFLAG_SEEN_RESPONDER      0x00000002
#define SSNFLAG_SEEN_BOTH           (SSNFLAG_SEEN_SERVER | SSNFLAG_SEEN_CLIENT)  /* used to check asymetric traffic */
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
#define SSNFLAG_STREAM_ORDER_BAD    0x00400000
#define SSNFLAG_FORCE_BLOCK         0x00800000
#define SSNFLAG_CLIENT_SWAP         0x01000000
#define SSNFLAG_CLIENT_SWAPPED      0x02000000
#define SSNFLAG_DETECTION_DISABLED  0x04000000
#define SSNFLAG_HTTP_2              0x08000000
#define SSNFLAG_HTTP_2_UPG          0x10000000
#define SSNFLAG_FREE_APP_DATA       0x20000000
#define SSNFLAG_ALL                 0xFFFFFFFF /* all that and a bag of chips */
#define SSNFLAG_NONE                0x00000000 /* nothing, an MT bag of chips */


// HA Session flags helper macros
#define HA_IGNORED_SESSION_FLAGS   ( SSNFLAG_COUNTED_INITIALIZE | SSNFLAG_COUNTED_ESTABLISH | \
                                     SSNFLAG_COUNTED_CLOSING | SSNFLAG_LOGGED_QUEUE_FULL)

#define HA_TCP_MAJOR_SESSION_FLAGS ( SSNFLAG_ESTABLISHED )

#define UNKNOWN_PORT 0

#define TCP_HZ          100

#define SESSION_API_VERSION1 1

/* NOTE:  The XFF_BUILTING_NAMES value must match the code in snort_httpinspect.c that
          adds the builtin names to the list. */
#define HTTP_XFF_FIELD_X_FORWARDED_FOR  "X-Forwarded-For"
#define HTTP_XFF_FIELD_TRUE_CLIENT_IP   "True-Client-IP"
#define HTTP_XFF_BUILTIN_NAMES          (2)
#define HTTP_MAX_XFF_FIELDS             8

typedef struct _StreamSessionKey
{
/* XXX If this data structure changes size, HashKeyCmp must be updated! */
    uint32_t  ip_l[4]; /* Low IP */
    uint32_t  ip_h[4]; /* High IP */
    uint16_t  port_l; /* Low Port - 0 if ICMP */
    uint16_t  port_h; /* High Port - 0 if ICMP */
    uint16_t  vlan_tag;
    uint8_t   protocol;
    char      pad;
    uint32_t  mplsLabel; /* MPLS label */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    uint16_t  addressSpaceId_l; /* Low ASID */      
    uint16_t  addressSpaceId_h; /* Higher ASID */     
#else
    uint16_t  addressSpaceId;
    uint16_t  addressSpaceIdPad1;
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
    uint32_t  carrierId;
#endif
/* XXX If this data structure changes size, HashKeyCmp must be updated! */
} StreamSessionKey;

typedef StreamSessionKey SessionKey;

typedef void ( *StreamAppDataFree )( void * );
typedef struct _StreamAppData
{
    uint32_t   protocol;
    void        *dataPointer;
    struct _StreamAppData *next;
    struct _StreamAppData *prev;
    StreamAppDataFree freeFunc;
} StreamAppData;

typedef struct _StreamFlowData
{
    BITOP boFlowbits;
    unsigned char flowb[1];
} StreamFlowData;

typedef struct _StreamSessionLimits
{
    uint32_t tcp_session_limit;
    uint32_t udp_session_limit;
    uint32_t icmp_session_limit;
    uint32_t ip_session_limit;
} StreamSessionLimits;

typedef struct _StreamHAState
{
    uint32_t   session_flags;

#ifdef TARGET_BASED
    int16_t    ipprotocol;
    int16_t    application_protocol;
#endif

    char       direction;
    char       ignore_direction; /* flag to ignore traffic on this session */
} StreamHAState;

typedef enum {
    SE_REXMIT,
    SE_EOF,
    SE_MAX
} Stream_Event;

//typedef void (*LogExtraData)(void *ssnptr, void *config, LogFunction *funcs, uint32_t max_count,
//                             uint32_t xtradata_mask, uint32_t id, uint32_t sec);

#ifdef ENABLE_HA
typedef uint32_t ( *StreamHAProducerFunc )( void *ssnptr, uint8_t *buf );
typedef int ( *StreamHAConsumerFunc )( void *ssnptr, const uint8_t *data, uint8_t length );
#endif

extern uint32_t HA_CRITICAL_SESSION_FLAGS;

// Protocol types for creating session cache
#define SESSION_PROTO_TCP 0x00
#define SESSION_PROTO_UDP 0x01
#define SESSION_PROTO_ICMP 0x02
#define SESSION_PROTO_IP 0x03
#define SESSION_PROTO_MAX 0x04

// Snort Policy Types
#define SNORT_NAP_POLICY  0x00
#define SNORT_IPS_POLICY  0x01

struct _SnortConfig;
struct _ExpectNode;

typedef void( *SessionCleanup )( void *ssn );
typedef void ( *nap_selector )( Packet *p, bool client_packet );
typedef void (*MandatoryEarlySessionCreatorFn)(void *ssn, struct _ExpectNode*);
typedef char** (*GetHttpXffPrecedenceFunc)(void* ssn, uint32_t flags, int* nFields);

struct _SessionCache;
typedef struct _session_api
{
    int version;

     /* Create a protocol specific cache for session control blocks
      *
      * Parameters:
      *   Session procotol type
      *   Protocol Session Control Block Size
      *   Cleanup callback function
      */
     struct _SessionCache* (*init_session_cache)(uint32_t, uint32_t, SessionCleanup);

     /* Lookup and return pointer to Session Control Block
      *
      *  Parameters
      *    Session Cache
      *    Packet
      *    Session Key
      */
     void *(*get_session)(struct _SessionCache*, Packet *, SessionKey *);

    /*   Populate a session key from the Packet
     *
     *  Parameters
     *      Packet
     *      Stream session key pointer
     */
     void (*populate_session_key)(Packet *, StreamSessionKey *);


      /* Lookup session by IP and Port from packet and return pointer to Session Control Block
      *
      * Parameters
      *   Source IP
      *   Source Port
      *   Destination IP
      *   Destination Port
      *   Protocol
      *   VLAN
      *   MPLS ID
      *   Address Space ID
      *   Session Key
      */
     int (*get_session_key_by_ip_port)(sfaddr_t*, uint16_t, sfaddr_t*, uint16_t, char, uint16_t,
                                       uint32_t,
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                                       uint16_t, uint16_t,
#else
                                       uint16_t, 
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
                                       uint32_t,
#endif
                                       SessionKey *);

     /* Lookup by session key and return Session Control Block
      *
      * Parameters
      *   Session Cache (protocol specific)
      *   Session Key
      *
      */
     void *(*get_session_by_key)(struct _SessionCache*, const SessionKey *);

     /* Lookup by session key and return Session Control Block - relys on the SessionKey to determine which cache
      *
      * Parameters
      *   Session Key
      *
      */
     void *(*get_session_handle)(const SessionKey *);

     /* Create a new session
      *
      * Parameters
      *   Session Cache (protocol specific)
      *   Packet
      *   Session Key
      *
      */
     void *(*create_session)(struct _SessionCache*, Packet *, const SessionKey *);

     /*  Is session verified by protocol
      *
      *  Parameters
      *    Session Control Block
     */
     bool (*is_session_verified)( void * );

     /*  remove session from oneway list
      *
      *  Parameters
      *    protocol
      *    Session Control Block
     */
     void (*remove_session_from_oneway_list)( uint32_t, void * );

      /* Delete a session
      *
      * Parameters
      *   Session cache (protocol specific)
      *   Session Control Block
      *   Reason
      *   Delete sycnhronous
      */
     int (*delete_session)(struct _SessionCache*, void *, char *, bool);

      /* Delete a session but without providing the session cache.
      *
      * Parameters
      *   Session Control Block
      *   Reason
      */
     int (*delete_session_by_key)(void *, char *);

     /* Print session cache
      *
      * Parameters
      *   Session cache (protocol specific)
      *
      */
     void (*print_session_cache)(struct _SessionCache*);

     /* Delete session cache
      *
      * Parameters
      *      protocol
      *
      */
     int (*delete_session_cache)( uint32_t protocol );

     /* Purge session cache
      *
      * Parameters
      *   Session cache (protocol specific)
      *
      */
     int (*purge_session_cache)(struct _SessionCache*);

     /* Prune session cache
      *
      * Parameters
      *   Session cache (protocol specific)
      *   Time
      *   Session Control Block
      *   Mem Check
      *
      */
     int (*prune_session_cache)(struct _SessionCache*, uint32_t, void *, int);

     /*  Clean memory pool for protocol sessions by protocol
      *
      *  Parameters
      *      protocol
      *
      */
     void (*clean_protocol_session_pool)( uint32_t );

     /*  Free protocol session memory by protocol
      *
      * Parameters
      *     protocol
      *     Session Pointer
      */
     void (*free_protocol_session_pool)( uint32_t, void * );

     /*  Allocate session from protocol session pool
      *
      * Parameters
      *     protocol
      */
     void *(*alloc_protocol_session)( uint32_t );

     /* Get session count
      *
      * Parameters
      *   Session cache (protocol specific)
      *
      */
     int (*get_session_count)(struct _SessionCache*);

     /*  Get prune count by protocol
      *
      *  Parameters
      *      protocol
      */
     uint32_t (*get_session_prune_count)( uint32_t protocol );

     /*  Reset prune count by protocol
      *
      * Parameters
      *     protocol
      */
     void (*reset_session_prune_count)( uint32_t protocol );

     /* Check session timeout
      *
      * Parameters
      *     Flow count
      *     Current time
     */
    void (*check_session_timeout)( uint32_t, time_t );

     /* Return status of protocol tracking for specified protocol
      *
      * Parameters
      *   proto
      *
      */
     int (*protocol_tracking_enabled)( IpProto proto );

      /* Set packet direction flag
      *
      * Parameters
      *   Packet
      *   Session Control Block
      *
      */
     void (*set_packet_direction_flag)(Packet *, void *);

     /* Free session application data
      *
      * Parameters
      *   Session Control Block
      *
      */
     void (*free_application_data)(void *);

    /* Get direction of packet
     *
     * Parameters:
     *     Packet
     */
    uint32_t (*get_packet_direction)(Packet *);

    /* Disable inspection for a sesion.
     *
     * Parameters
     *     Session Ptr
     *     Packet
     */
    void (*disable_inspection)(void *, Packet *);

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
     *     Preprocessor ID
     *     Direction
     *     Flags (permanent)
     *
     * Returns
     *     0 on success
     *     -1 on failure
     */
    int (*ignore_session)(const Packet *, sfaddr_t*, uint16_t, sfaddr_t*, uint16_t, uint8_t,
                          uint32_t, char, char, struct _ExpectNode**);

    /* Get direction that data is being ignored.
     *
     * Parameters
     *     Session Ptr
     */
    int (*get_ignore_direction)(void *);

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

    /*
     * Set Expiration Timeout
     *
     * Parameters
     *     Packet
     *     Session Ptr
     *     timeout
     */
    void (*set_expire_timer)( Packet *, void *, uint32_t );

    /* Get Expriration Timeou
     *
     * Parameters
     *     Packet
     *     Session Ptr
     *
    */
    int (*get_expire_timer)( Packet *, void *);

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

    /*  Get the runtime policy index for policy type
     *  specified
     *
     *  Parameters
     *     Session Ptr
     *     Policy Type: NAP or IPS
     */
    tSfPolicyId (*get_runtime_policy)(void *, int);

    /*  Set the runtime policy index for policy type
     *  specified
     *
     *  Parameters
     *     Session Ptr
     *     Policy Type: NAP or IPS
     *     Index for this policy
     */
     void (*set_runtime_policy)(void *, int, tSfPolicyId);


     /* Get Flowbits data
     *
     * Parameters
     *     Packet
     *
     * Returns
     *     Ptr to Flowbits Data
     */

    StreamFlowData *(*get_flow_data)(Packet *p);

     /* Set if Session Deletion to be delayed
      *
      * Parameters
      *   Session Ptr
      *   bool to set/unset delay_session_deletion_flag
      *
      */
     void (*set_session_deletion_delayed)(void *, bool);

     /* Returns if SessionDeletion to be delayed or not
      *
      * Parameters
      *    Session Ptr
      *
      * Returns
      *    bool value denoting if sessionDeletion Delayed or not
      *
      */
     bool (*is_session_deletion_delayed)(void *);

#ifdef TARGET_BASED
    /*  Register preproc handler for the specifed application id
     *
     *  Parameters
     *      Preprocessor Id
     *      Application ID
     */
    void (*register_service_handler)(uint32_t, int16_t);


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

    /* Get server IP address. This could be used either during packet processing or when
     * a session is being closed. Caller should make a deep copy if return value is needed
     * for later use.
     *
     * Arguments
     *  void * - session pointer
     *  uint32_t - direction. Valid values are SSN_DIR_SERVER or SSN_DIR_CLIENT
     *
     * Returns
     *  IP address. Contents at the buffer should not be changed. The
     */
     sfaddr_t*  (*get_session_ip_address)(void *, uint32_t);

    /* Get server/client ports.
     *
     * Arguments
     *  void * - session pointer
     *  uint16_t *client_port - client port pointer
     *  uint16_t *server_port - server port pointer
     *
     * Returns
     *  Ports.
     */
     void (*get_session_ports)(void *, uint16_t *client_port, uint16_t *server_port);
#endif

    /** Get an independent bit to allow an entity to enable and
     *  disable port session tracking and syn session creation
     *  without affecting the status of set by other entities.
     *  Returns a bitmask (with the bit range 3-15) or 0, if no bits
     *  are available.
     */
    uint16_t (*get_preprocessor_status_bit)(void);

#ifdef ACTIVE_RESPONSE
    // initialize response count and expiration time
    void (*init_active_response)(Packet *, void *);
#endif

    // Get the TTL value used at session setup
    // outer=0 to get inner ip ttl for ip in ip; else outer=1
    uint8_t (*get_session_ttl)(void *ssnptr, char direction, int outer);

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
     *
     * Returns
     *     0 on success
     *     -1 on failure
     */
    int (*set_application_protocol_id_expected)(const Packet *, sfaddr_t*, uint16_t, sfaddr_t*, uint16_t,
                uint8_t, int16_t, uint32_t, void*, void (*)(void*), struct _ExpectNode**);

#ifdef ENABLE_HA
    /* Register a high availability producer and consumer function pair for a
     * particular preprocessor ID and subcode combination.
     *
     * Parameters
     *      Processor ID
     *      Subcode
     *      Maximum Message Size
     *      Message Producer Function
     *      Message Consumer Function
     *
     *  Returns
     *      >= 0 on success
     *          The returned value is the bit number in the HA pending bitmask and
     *          should be stored for future calls to set_ha_pending_bit().
     *      < 0 on failure
     */
    int (*register_ha_funcs)(uint32_t preproc_id, uint8_t subcode, uint8_t size,
                             StreamHAProducerFunc produce, StreamHAConsumerFunc consume);

    /* Unregister a high availability producer and consumer function pair for a
     * particular preprocessor ID and subcode combination.
     *
     * Parameters
     *      Processor ID
     *      Subcode
     */
    void (*unregister_ha_funcs)(uint32_t preproc_id, uint8_t subcode);

    /* Indicate a pending high availability update for a given session.
     *
     * Parameters
     *      Session Ptr
     *      HA Pending Update Bit
     */
    void (*set_ha_pending_bit)(void *, int bit);

    /* Attempt to process any pending HA events for the given session
     *
     * Parameters
     *      Session Ptr
     *      DAQ Packet Header for the packet being processed (Could be NULL)
     */
    void (*process_ha)(void *, const DAQ_PktHdr_t *);
#endif

    //Retrieve the maximum session limits for the given policy
    void (*get_max_session_limits)(tSfPolicyId, StreamSessionLimits*);

    /* Set direction that data is being ignored.
       *
       * Parameters
       *     Session Ptr
       */
    int (*set_ignore_direction)(void *, int);

    /** Retrieve stream session pointer based on the lookup tuples for
     *  cases where Snort does not have an active packet that is
     *  relevant.
     *
     * Parameters
     *     IP addr #1
     *     Port #1 (0 for non TCP/UDP)
     *     IP addr #2
     *     Port #2 (0 for non TCP/UDP)
     *     Protocol
     *     VLAN ID
     *     MPLS ID
     *     Address Space ID
     *
     * Returns
     *     Stream session pointer
     */
    void *(*get_session_ptr_from_ip_port)(sfaddr_t*, uint16_t, sfaddr_t*, uint16_t, char,
                                          uint16_t, uint32_t,
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                                          uint16_t, uint16_t
#else
                                          uint16_t
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
                                          , uint32_t
#endif 
                                          );

    /** Retrieve the session key given a stream session pointer.
     *
     * Parameters
     *     Session Ptr
     *
     * Returns
     *     Stream session key
     */
    const StreamSessionKey *(*get_key_from_session_ptr)(const void *);

    /* Delete the session if it is in the closed session state.
     *
     * Parameters
     *     Packet
     */
    void (*check_session_closed)(Packet *);

    /*  Create a session key from the Packet
     *
     *  Parameters
     *      Packet
     */
    StreamSessionKey *(*get_session_key)(Packet *);

    /*  Get the application data from the session key
     *
     *  Parameters
     *      SessionKey *
     *      Application Protocol
     */
    void *(*get_application_data_from_key)(const StreamSessionKey *, uint32_t);

    /** Retrieve application session data based on the lookup tuples for
     *  cases where Snort does not have an active packet that is
     *  relevant.
     *
     * Parameters
     *     IP addr #1
     *     Port #1 (0 for non TCP/UDP)
     *     IP addr #2
     *     Port #2 (0 for non TCP/UDP)
     *     Protocol
     *     VLAN ID
     *     MPLS ID
     *     Address Space ID
     *     Preprocessor ID
     *
     * Returns
     *     Application Data reference (pointer)
     */
    void *(*get_application_data_from_ip_port)(sfaddr_t*, uint16_t, sfaddr_t*, uint16_t, 
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                                               uint16_t, uint16_t, 
#else
                                               uint16_t,
#endif      
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
                                               uint32_t,
#endif                                       
                                               char, uint16_t, uint32_t, uint32_t);

    void (*disable_preproc_for_session)( void *, uint32_t );
    void (*enable_preproc_for_port)( struct _SnortConfig *, uint32_t, uint32_t, uint16_t );
    void (*enable_preproc_all_ports)( struct _SnortConfig *, uint32_t, uint32_t );
    void (*enable_preproc_all_ports_all_policies)( struct _SnortConfig *, uint32_t, uint32_t );
    bool (*is_preproc_enabled_for_port)( uint32_t, uint16_t );
    void (*register_nap_selector)( nap_selector );
    void (*register_mandatory_early_session_creator)(struct _SnortConfig *,
                                                     MandatoryEarlySessionCreatorFn callback);
    void* (*get_application_data_from_expected_node)(struct _ExpectNode*, uint32_t);
    int (*add_application_data_to_expected_node)(struct _ExpectNode*, uint32_t, void*, void (*)(void*));
    void (*register_get_http_xff_precedence)(GetHttpXffPrecedenceFunc );
    char** (*get_http_xff_precedence)(void* ssn, uint32_t flags, int* nFields);
    struct _ExpectNode* (*get_next_expected_node)(struct _ExpectNode*);
    void (*set_reputation_update_counter) (void *,uint8_t);
} SessionAPI;

/* To be set by Session */
extern SessionAPI *session_api;

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

#define PORT_MONITOR_SESSION_BITS   0xFFFE

#define PP_SESSION_PRIORITY PRIORITY_CORE + PP_CORE_ORDER_SESSION

// Utility functions
//
/*********************************************************************
 * Function: isPortEnabled
 *
 * Checks to see if a port is enabled in the port array mask
 * passed in.
 *
 * Arguments:
 *  uint8_t *
 *      Pointer to a port array mask.
 *  const uint16_t
 *      The port to check for in the mask.
 *
 * Returns:
 *  bool
 *      true if the port is set.
 *      false if the port is not set.
 *
 *********************************************************************/
static inline bool isPortEnabled( const uint8_t *port_array, const uint16_t port )
{
    return port_array[ ( port / 8 ) ] & ( 1 << ( port % 8 ) );
}

/*********************************************************************
 * Function: enablePort()
 *
 * Enable a port in the port array mask passed in.
 *
 * Arguments:
 *  uint8_t *
 *      Pointer to a port array mask.
 *  const uint16_t
 *      The port to set in the port array mask.
 *
 * Returns: None
 *
 *********************************************************************/
static inline void enablePort( uint8_t *port_array, const uint16_t port )
{
    port_array[ ( port / 8 ) ] |= ( 1 << ( port % 8 ) );
}

/*********************************************************************
 * Function: disablePort()
 *
 * Disable a port in the port array mask passed in.
 *
 * Arguments:
 *  uint8_t *
 *      Pointer to a port array mask.
 *  const uint16_t
 *      The port to set in the port array mask.
 *
 * Returns: None
 *
 *********************************************************************/
static inline void disablePort( uint8_t *port_array, const uint16_t port )
{
    port_array[ ( port / 8 ) ] &= ~( 1 << ( port % 8 ) );
}

#endif /* SESSION_API_H_ */

