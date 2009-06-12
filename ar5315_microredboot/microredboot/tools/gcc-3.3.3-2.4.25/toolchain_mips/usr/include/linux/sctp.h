/* SCTP kernel reference Implementation
 * Copyright (c) 1999-2000 Cisco, Inc.
 * Copyright (c) 1999-2001 Motorola, Inc.
 * Copyright (c) 2001-2002 International Business Machines, Corp.
 * Copyright (c) 2001 Intel Corp.
 * Copyright (c) 2001 Nokia, Inc.
 * Copyright (c) 2001 La Monte H.P. Yarroll
 *
 * This file is part of the SCTP kernel reference Implementation
 *
 * Various protocol defined structures.
 *
 * The SCTP reference implementation is free software;
 * you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * The SCTP reference implementation is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 ************************
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU CC; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Please send any bug reports or fixes you make to the
 * email address(es):
 *    lksctp developers <lksctp-developerst@lists.sourceforge.net>
 *
 * Or submit a bug report through the following website:
 *    http://www.sf.net/projects/lksctp
 *
 * Written or modified by:
 *    La Monte H.P. Yarroll <piggy@acm.org>
 *    Karl Knutson <karl@athena.chicago.il.us>
 *    Jon Grimm <jgrimm@us.ibm.com>
 *    Xingang Guo <xingang.guo@intel.com>
 *    randall@sctp.chicago.il.us
 *    kmorneau@cisco.com
 *    qxie1@email.mot.com
 *
 * Any bugs reported given to us we will try to fix... any fixes shared will
 * be incorporated into the next SCTP release.
 */
#ifndef __LINUX_SCTP_H__
#define __LINUX_SCTP_H__

#include <linux/in.h>		/* We need in_addr.  */
#include <linux/in6.h>		/* We need in6_addr.  */


/* Section 3.1.  SCTP Common Header Format */
typedef struct sctphdr {
	__u16 source;
	__u16 dest;
	__u32 vtag;
	__u32 checksum;
} sctp_sctphdr_t __attribute__((packed));

/* Section 3.2.  Chunk Field Descriptions. */
typedef struct sctp_chunkhdr {
	__u8 type;
	__u8 flags;
	__u16 length;
} sctp_chunkhdr_t __attribute__((packed));


/* Section 3.2.  Chunk Type Values.
 * [Chunk Type] identifies the type of information contained in the Chunk
 * Value field. It takes a value from 0 to 254. The value of 255 is
 * reserved for future use as an extension field.
 */
typedef enum {
	SCTP_CID_DATA			= 0,
        SCTP_CID_INIT			= 1,
        SCTP_CID_INIT_ACK		= 2,
        SCTP_CID_SACK			= 3,
        SCTP_CID_HEARTBEAT		= 4,
        SCTP_CID_HEARTBEAT_ACK		= 5,
        SCTP_CID_ABORT			= 6,
        SCTP_CID_SHUTDOWN		= 7,
        SCTP_CID_SHUTDOWN_ACK		= 8,
        SCTP_CID_ERROR			= 9,
        SCTP_CID_COOKIE_ECHO		= 10,
        SCTP_CID_COOKIE_ACK	        = 11,
        SCTP_CID_ECN_ECNE		= 12,
        SCTP_CID_ECN_CWR		= 13,
        SCTP_CID_SHUTDOWN_COMPLETE	= 14,

	/* Use hex, as defined in ADDIP sec. 3.1 */
	SCTP_CID_ASCONF			= 0xC1,
	SCTP_CID_ASCONF_ACK		= 0x80,
} sctp_cid_t; /* enum */


/* Section 3.2
 *  Chunk Types are encoded such that the highest-order two bits specify
 *  the action that must be taken if the processing endpoint does not
 *  recognize the Chunk Type.
 */
typedef enum {
	SCTP_CID_ACTION_DISCARD     = 0x00,
	SCTP_CID_ACTION_DISCARD_ERR = 0x40,
	SCTP_CID_ACTION_SKIP        = 0x80,
	SCTP_CID_ACTION_SKIP_ERR    = 0xc0,
} sctp_cid_action_t;

enum { SCTP_CID_ACTION_MASK = 0xc0, };

/* This flag is used in Chunk Flags for ABORT and SHUTDOWN COMPLETE.
 *
 * 3.3.7 Abort Association (ABORT) (6):
 *    The T bit is set to 0 if the sender had a TCB that it destroyed.
 *    If the sender did not have a TCB it should set this bit to 1.
 */
enum { SCTP_CHUNK_FLAG_T = 0x01 };

/*
 *  Set the T bit
 *
 *      0                   1                   2                   3
 *      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |   Type = 14   |Reserved     |T|      Length = 4               |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Chunk Flags: 8 bits
 *
 *   Reserved:  7 bits
 *     Set to 0 on transmit and ignored on receipt.
 *
 *   T bit:  1 bit
 *     The T bit is set to 0 if the sender had a TCB that it destroyed. If
 *     the sender did NOT have a TCB it should set this bit to 1.
 *
 * Note: Special rules apply to this chunk for verification, please
 * see Section 8.5.1 for details.
 */

#define sctp_test_T_bit(c)    ((c)->chunk_hdr->flags & SCTP_CHUNK_FLAG_T)

/* RFC 2960
 * Section 3.2.1 Optional/Variable-length Parmaeter Format.
 */

typedef struct sctp_paramhdr {
	__u16 type;
	__u16 length;
} sctp_paramhdr_t __attribute((packed));

typedef enum {

	/* RFC 2960 Section 3.3.5 */
	SCTP_PARAM_HEARTBEAT_INFO		= __constant_htons(1),
	/* RFC 2960 Section 3.3.2.1 */
	SCTP_PARAM_IPV4_ADDRESS			= __constant_htons(5),
	SCTP_PARAM_IPV6_ADDRESS			= __constant_htons(6),
	SCTP_PARAM_STATE_COOKIE			= __constant_htons(7),
	SCTP_PARAM_UNRECOGNIZED_PARAMETERS	= __constant_htons(8),
	SCTP_PARAM_COOKIE_PRESERVATIVE		= __constant_htons(9),
	SCTP_PARAM_HOST_NAME_ADDRESS		= __constant_htons(11),
	SCTP_PARAM_SUPPORTED_ADDRESS_TYPES	= __constant_htons(12),
	SCTP_PARAM_ECN_CAPABLE			= __constant_htons(0x8000),

	/* Add-IP Extension. Section 3.2 */
	SCTP_PARAM_ADD_IP		= __constant_htons(0xc001),
	SCTP_PARAM_DEL_IP		= __constant_htons(0xc002),
	SCTP_PARAM_ERR_CAUSE		= __constant_htons(0xc003),
	SCTP_PARAM_SET_PRIMARY		= __constant_htons(0xc004),
	SCTP_PARAM_SUCCESS_REPORT	= __constant_htons(0xc005),
	SCTP_PARAM_ADAPTION_LAYER_IND   = __constant_htons(0xc006),

} sctp_param_t; /* enum */


/* RFC 2960 Section 3.2.1
 *  The Parameter Types are encoded such that the highest-order two bits
 *  specify the action that must be taken if the processing endpoint does
 *  not recognize the Parameter Type.
 *
 */
typedef enum {
	SCTP_PARAM_ACTION_DISCARD     = __constant_htons(0x0000),
	SCTP_PARAM_ACTION_DISCARD_ERR = __constant_htons(0x4000),
	SCTP_PARAM_ACTION_SKIP        = __constant_htons(0x8000),
	SCTP_PARAM_ACTION_SKIP_ERR    = __constant_htons(0xc000),
} sctp_param_action_t;

enum { SCTP_PARAM_ACTION_MASK = __constant_htons(0xc000), };

/* RFC 2960 Section 3.3.1 Payload Data (DATA) (0) */

typedef struct sctp_datahdr {
	__u32 tsn;
	__u16 stream;
	__u16 ssn;
	__u32 ppid;
	__u8  payload[0];
} sctp_datahdr_t __attribute__((packed));

typedef struct sctp_data_chunk {
        sctp_chunkhdr_t chunk_hdr;
        sctp_datahdr_t  data_hdr;
} sctp_data_chunk_t __attribute__((packed));

/* DATA Chuck Specific Flags */
enum {
	SCTP_DATA_MIDDLE_FRAG	= 0x00,
	SCTP_DATA_LAST_FRAG	= 0x01,
	SCTP_DATA_FIRST_FRAG	= 0x02,
	SCTP_DATA_NOT_FRAG	= 0x03,
	SCTP_DATA_UNORDERED	= 0x04,
};
enum { SCTP_DATA_FRAG_MASK = 0x03, };


/* RFC 2960 Section 3.3.2 Initiation (INIT) (1)
 *
 *  This chunk is used to initiate a SCTP association between two
 *  endpoints.
 */
typedef struct sctp_inithdr {
	__u32 init_tag;
	__u32 a_rwnd;
	__u16 num_outbound_streams;
	__u16 num_inbound_streams;
	__u32 initial_tsn;
	__u8  params[0];
} sctp_inithdr_t __attribute__((packed));

typedef struct sctp_init_chunk {
	sctp_chunkhdr_t chunk_hdr;
	sctp_inithdr_t init_hdr;
} sctp_init_chunk_t __attribute__((packed));


/* Section 3.3.2.1. IPv4 Address Parameter (5) */
typedef struct sctp_ipv4addr_param {
	sctp_paramhdr_t param_hdr;
	struct in_addr  addr;
} sctp_ipv4addr_param_t __attribute__((packed));

/* Section 3.3.2.1. IPv6 Address Parameter (6) */
typedef struct sctp_ipv6addr_param {
	sctp_paramhdr_t param_hdr;
	struct in6_addr addr;
} sctp_ipv6addr_param_t __attribute__((packed));

/* Section 3.3.2.1 Cookie Preservative (9) */
typedef struct sctp_cookie_preserve_param {
	sctp_paramhdr_t param_hdr;
	uint32_t        lifespan_increment;
} sctp_cookie_preserve_param_t __attribute__((packed));

/* Section 3.3.2.1 Host Name Address (11) */
typedef struct sctp_hostname_param {
	sctp_paramhdr_t param_hdr;
	uint8_t hostname[0];
} sctp_hostname_param_t __attribute__((packed));

/* Section 3.3.2.1 Supported Address Types (12) */
typedef struct sctp_supported_addrs_param {
	sctp_paramhdr_t param_hdr;
	uint16_t types[0];
} sctp_supported_addrs_param_t __attribute__((packed));

/* Appendix A. ECN Capable (32768) */
typedef struct sctp_ecn_capable_param {
	sctp_paramhdr_t param_hdr;
} sctp_ecn_capable_param_t __attribute__((packed));



/* RFC 2960.  Section 3.3.3 Initiation Acknowledgement (INIT ACK) (2):
 *   The INIT ACK chunk is used to acknowledge the initiation of an SCTP
 *   association.
 */
typedef sctp_init_chunk_t sctp_initack_chunk_t;

/* Section 3.3.3.1 State Cookie (7) */
typedef struct sctp_cookie_param {
	sctp_paramhdr_t p;
	__u8 body[0];
} sctp_cookie_param_t __attribute__((packed));

/* Section 3.3.3.1 Unrecognized Parameters (8) */
typedef struct sctp_unrecognized_param {
	sctp_paramhdr_t param_hdr;
	sctp_paramhdr_t unrecognized;
} sctp_unrecognized_param_t __attribute__((packed));



/*
 * 3.3.4 Selective Acknowledgement (SACK) (3):
 *
 *  This chunk is sent to the peer endpoint to acknowledge received DATA
 *  chunks and to inform the peer endpoint of gaps in the received
 *  subsequences of DATA chunks as represented by their TSNs.
 */

typedef struct sctp_gap_ack_block {
	__u16 start;
	__u16 end;
} sctp_gap_ack_block_t __attribute__((packed));

typedef uint32_t sctp_dup_tsn_t;

typedef union {
	sctp_gap_ack_block_t	gab;
        sctp_dup_tsn_t		dup;
} sctp_sack_variable_t;

typedef struct sctp_sackhdr {
	__u32 cum_tsn_ack;
	__u32 a_rwnd;
	__u16 num_gap_ack_blocks;
	__u16 num_dup_tsns;
	sctp_sack_variable_t variable[0];
} sctp_sackhdr_t __attribute__((packed));

typedef struct sctp_sack_chunk {
	sctp_chunkhdr_t chunk_hdr;
	sctp_sackhdr_t sack_hdr;
} sctp_sack_chunk_t __attribute__((packed));


/* RFC 2960.  Section 3.3.5 Heartbeat Request (HEARTBEAT) (4):
 *
 *  An endpoint should send this chunk to its peer endpoint to probe the
 *  reachability of a particular destination transport address defined in
 *  the present association.
 */

typedef struct sctp_heartbeathdr {
	sctp_paramhdr_t info;
} sctp_heartbeathdr_t __attribute__((packed));

typedef struct sctp_heartbeat_chunk {
	sctp_chunkhdr_t chunk_hdr;
	sctp_heartbeathdr_t hb_hdr;
} sctp_heartbeat_chunk_t __attribute__((packed));


/* For the abort and shutdown ACK we must carry the init tag in the
 * common header. Just the common header is all that is needed with a
 * chunk descriptor.
 */
typedef struct sctp_abort_chunk {
        sctp_chunkhdr_t uh;
} sctp_abort_chunkt_t __attribute__((packed));


/* For the graceful shutdown we must carry the tag (in common header)
 * and the highest consecutive acking value.
 */
typedef struct sctp_shutdownhdr {
	__u32 cum_tsn_ack;
} sctp_shutdownhdr_t __attribute__((packed));

struct sctp_shutdown_chunk_t {
        sctp_chunkhdr_t    chunk_hdr;
        sctp_shutdownhdr_t shutdown_hdr;
} __attribute__((packed));



/* RFC 2960.  Section 3.3.10 Operation Error (ERROR) (9) */

typedef struct sctp_errhdr {
	__u16 cause;
	__u16 length;
	__u8  variable[0];
} sctp_errhdr_t __attribute__((packed));

typedef struct sctp_operr_chunk {
        sctp_chunkhdr_t chunk_hdr;
	sctp_errhdr_t   err_hdr;
} sctp_operr_chunk_t __attribute__((packed));

/* RFC 2960 3.3.10 - Operation Error
 *
 * Cause Code: 16 bits (unsigned integer)
 *
 *     Defines the type of error conditions being reported.
 *    Cause Code
 *     Value           Cause Code
 *     ---------      ----------------
 *      1              Invalid Stream Identifier
 *      2              Missing Mandatory Parameter
 *      3              Stale Cookie Error
 *      4              Out of Resource
 *      5              Unresolvable Address
 *      6              Unrecognized Chunk Type
 *      7              Invalid Mandatory Parameter
 *      8              Unrecognized Parameters
 *      9              No User Data
 *     10              Cookie Received While Shutting Down
 */
typedef enum {

	SCTP_ERROR_NO_ERROR	   = __constant_htons(0x00),
	SCTP_ERROR_INV_STRM	   = __constant_htons(0x01),
	SCTP_ERROR_MISS_PARAM 	   = __constant_htons(0x02),
	SCTP_ERROR_STALE_COOKIE	   = __constant_htons(0x03),
	SCTP_ERROR_NO_RESOURCE 	   = __constant_htons(0x04),
	SCTP_ERROR_DNS_FAILED      = __constant_htons(0x05),
	SCTP_ERROR_UNKNOWN_CHUNK   = __constant_htons(0x06),
	SCTP_ERROR_INV_PARAM       = __constant_htons(0x07),
	SCTP_ERROR_UNKNOWN_PARAM   = __constant_htons(0x08),
	SCTP_ERROR_NO_DATA         = __constant_htons(0x09),
	SCTP_ERROR_COOKIE_IN_SHUTDOWN = __constant_htons(0x0a),


	/* SCTP Implementation Guide:
	 *  11  Restart of an association with new addresses
	 *  12  User Initiated Abort
	 *  13  Protocol Violation
	 */

	SCTP_ERROR_RESTART         = __constant_htons(0x0b),
	SCTP_ERROR_USER_ABORT      = __constant_htons(0x0c),
	SCTP_ERROR_PROTO_VIOLATION = __constant_htons(0x0d),

	/* ADDIP Section 3.3  New Error Causes
	 *
	 * Four new Error Causes are added to the SCTP Operational Errors,
	 * primarily for use in the ASCONF-ACK chunk.
	 *
	 * Value          Cause Code
	 * ---------      ----------------
	 * 0x0100          Request to Delete Last Remaining IP Address.
	 * 0x0101          Operation Refused Due to Resource Shortage.
	 * 0x0102          Request to Delete Source IP Address.
	 * 0x0103          Association Aborted due to illegal ASCONF-ACK
	 */
	SCTP_ERROR_DEL_LAST_IP	= __constant_htons(0x0100),
	SCTP_ERROR_RSRC_LOW	= __constant_htons(0x0101),
	SCTP_ERROR_DEL_SRC_IP	= __constant_htons(0x0102),
	SCTP_ERROR_ASCONF_ACK   = __constant_htons(0x0103),

} sctp_error_t;



/* RFC 2960.  Appendix A.  Explicit Congestion Notification.
 *   Explicit Congestion Notification Echo (ECNE) (12)
 */
typedef struct sctp_ecnehdr {
	__u32 lowest_tsn;
} sctp_ecnehdr_t;

typedef struct sctp_ecne_chunk {
	sctp_chunkhdr_t chunk_hdr;
	sctp_ecnehdr_t ence_hdr;
} sctp_ecne_chunk_t __attribute__((packed));

/* RFC 2960.  Appendix A.  Explicit Congestion Notification.
 *   Congestion Window Reduced (CWR) (13)
 */
typedef struct sctp_cwrhdr {
	__u32 lowest_tsn;
} sctp_cwrhdr_t;

typedef struct sctp_cwr_chunk {
	sctp_chunkhdr_t chunk_hdr;
	sctp_cwrhdr_t cwr_hdr;
} sctp_cwr_chunk_t __attribute__((packed));


/* FIXME:  Cleanup needs to continue below this line. */

/*
 * ADDIP Section 3.1 New Chunk Types
 */


/* ADDIP Section 3.1.1
 *
 * ASCONF-Request Correlation ID: 32 bits (unsigned integer)
 *
 * This is an opaque integer assigned by the sender to identify each
 * request parameter. It is in host byte order and is only meaningful
 * to the sender. The receiver of the ASCONF Chunk will copy this 32
 * bit value into the ASCONF Correlation ID field of the
 * ASCONF-ACK. The sender of the ASCONF can use this same value in the
 * ASCONF-ACK to find which request the response is for.
 *
 * ASCONF Parameter: TLV format
 *
 * Each Address configuration change is represented by a TLV parameter
 * as defined in Section 3.2. One or more requests may be present in
 * an ASCONF Chunk.
 */
typedef struct {
	__u32	correlation;
	sctp_paramhdr_t p;
	__u8		payload[0];
} sctpAsconfReq_t;

/* ADDIP
 * 3.1.1  Address/Stream Configuration Change Chunk (ASCONF)
 *
 * This chunk is used to communicate to the remote endpoint one of the
 * configuration change requests that MUST be acknowledged.  The
 * information carried in the ASCONF Chunk uses the form of a
 * Tag-Length-Value (TLV), as described in "3.2.1
 * Optional/Variable-length Parameter Format" in [RFC2960], for all
 * variable parameters.
 */
typedef struct {
	__u32	serial;
	__u8	reserved[3];
	__u8	addr_type;
	__u32	addr[4];
	sctpAsconfReq_t requests[0];
} sctpAsconf_t;

/* ADDIP
 * 3.1.2 Address/Stream Configuration Acknowledgment Chunk (ASCONF-ACK)
 *
 * ASCONF-Request Correlation ID: 32 bits (unsigned integer)
 *
 * This value is copied from the ASCONF Correlation ID received in the
 * ASCONF Chunk. It is used by the receiver of the ASCONF-ACK to identify
 * which ASCONF parameter this response is associated with.
 *
 * ASCONF Parameter Response : TLV format
 *
 * The ASCONF Parameter Response is used in the ASCONF-ACK to report
 * status of ASCONF processing. By default, if a responding endpoint
 * does not include any Error Cause, a success is indicated. Thus a
 * sender of an ASCONF-ACK MAY indicate complete success of all TLVs in
 * an ASCONF by returning only the Chunk Type, Chunk Flags, Chunk Length
 * (set to 8) and the Serial Number.
 */
typedef union {
	struct {
		__u32		correlation;
		sctp_paramhdr_t header;	/* success report */
	} success;
	struct {
		__u32		correlation;
		sctp_paramhdr_t header;	/* error cause indication */
		sctp_paramhdr_t errcause;
		uint8_t request[0];	/* original request from ASCONF */
	} error;
#define __correlation	success.correlation
#define __header	success.header
#define __cause		error.errcause
#define __request	error.request
}  sctpAsconfAckRsp_t;

/* ADDIP
 * 3.1.2 Address/Stream Configuration Acknowledgment Chunk (ASCONF-ACK)
 *
 * This chunk is used by the receiver of an ASCONF Chunk to
 * acknowledge the reception. It carries zero or more results for any
 * ASCONF Parameters that were processed by the receiver.
 */
typedef struct {
	__u32	serial;
	sctpAsconfAckRsp_t responses[0];
} sctpAsconfAck_t;

/*********************************************************************
 * Internal structures
 *
 * These are data structures which never go out on the wire.
 *********************************************************************/

/* What is this data structure for?  The TLV isn't one--it is just a
 * value.  Perhaps this data structure ought to have a type--otherwise
 * it is not unambigiously parseable.  --piggy
 */
typedef struct {
	struct list_head hook;
	int length;	/* length of the TLV */

	/* the actually TLV to be copied into ASCONF_ACK */
	sctpAsconfAckRsp_t TLV;
} sctpAsconfAckRspNode_t;

#endif /* __LINUX_SCTP_H__ */
