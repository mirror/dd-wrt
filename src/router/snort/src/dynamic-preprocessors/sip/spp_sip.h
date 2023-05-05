/* $Id */

/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2011-2013 Sourcefire, Inc.
**
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

/*
 * spp_sip.h: Definitions, structs, function prototype(s) for
 *		the SIP preprocessor.
 * Author: Hui Cao
 */

#ifndef SPP_SIP_H
#define SPP_SIP_H
#include <stddef.h>
#include "sfPolicy.h"
#include "sfPolicyUserData.h"
#include "snort_bounds.h"
#include "sip_roptions.h"
#include "sf_ip.h"

/* Convert port value into an index for the sip_config->ports array */
#define PORT_INDEX(port) port/8

/* Convert port value into a value for bitwise operations */
#define CONV_PORT(port) 1<<(port%8)

/*
 * Boolean values.
 */
#define SIP_TRUE	(1)
#define SIP_FALSE	(0)

#define SIP_STATUS_CODE_LEN	(3)
#define SIP_CONTENT_LEN (5)
/*
 * Error codes.
 */
#define SIP_SUCCESS	(1)
#define SIP_FAILURE	(0)

#define SIP_SESSION_SAVED	(1)
#define SIP_SESSION_INIT	(0)

typedef struct _SIP_DialogID
{
	uint32_t callIdHash;
	uint32_t fromTagHash;
	uint32_t toTagHash;
} SIP_DialogID;


typedef struct _SIP_DialogData
{
	SIP_DialogID dlgID;
	SIP_DialogState state;
	SIPMethodsFlag creator;
	uint16_t status_code;
	SIP_MediaList mediaSessions;
	struct _SIP_DialogData *nextD;
	struct _SIP_DialogData *prevD;
} SIP_DialogData;

typedef struct _SIP_DialogList
{
    SIP_DialogData* head;
    uint32_t num_dialogs;
}SIP_DialogList;

/*
 * Per-session data block containing current state
 * of the SIP preprocessor for the session.
 *
 * state_flags:		Bit vector describing the current state of the
 * 				session.
 */
typedef struct _sipData
{
    uint32_t state_flags;
    tSfPolicyId policy_id;
    SIP_DialogList dialogs;
    SIP_Roptions ropts;
    tSfPolicyUserContextId config;
} SIPData;

typedef struct _SIPMsg
{
    uint16_t headerLen;
    uint16_t methodLen;
    SIPMethodsFlag methodFlag;
    uint16_t status_code;

    uint16_t uriLen;
    uint16_t callIdLen;
    uint16_t cseqNameLen;
    uint16_t fromLen;
    uint16_t fromTagLen;
    uint16_t toLen;
    uint16_t toTagLen;
    uint16_t viaLen;
    uint16_t contactLen;
    uint16_t bodyLen;
    uint16_t contentTypeLen;
    uint32_t content_len;
    SIP_DialogID dlgID;
    SIP_MediaSession *mediaSession;
    char *authorization;
    const uint8_t *header;
    const uint8_t *body_data;  /* Set to NULL if not applicable */
    uint64_t cseqnum;

    uint16_t userNameLen;
    uint16_t userAgentLen;
    uint16_t serverLen;
    bool     mediaUpdated;

    /* nothing after this point is zeroed ...*/
    /*Input parameters*/
    unsigned char isTcp;

    char *method;
    char *uri;
    char *call_id;
    char *cseqName;
    char *from;
    char *from_tag;
    char *to;
    char *to_tag;
    char *via;
    char *contact;

    char *content_type;
    char *content_encode;

    const char *userAgent;
    const char *userName;
    const char *server;


} SIPMsg;

#define SIPMSG_ZERO_LEN offsetof(SIPMsg, isTcp)

/*
 * Generator id. Define here the same as the official registry
 * in generators.h
 */
#define GENERATOR_SPP_SIP	140

/* Ultimately calls SnortEventqAdd */
/* Arguments are: gid, sid, rev, classification, priority, message, rule_info */
#define ALERT(x,y) { _dpd.alertAdd(GENERATOR_SPP_SIP, x, 1, 0, 3, y, 0 ); sip_stats.events++; }

/*
 * SIP preprocessor alert types.
 */
#define SIP_EVENT_MAX_SESSIONS        1
#define SIP_EVENT_EMPTY_REQUEST_URI	  2
#define SIP_EVENT_BAD_URI		      3
#define SIP_EVENT_EMPTY_CALL_ID	      4
#define SIP_EVENT_BAD_CALL_ID		  5
#define SIP_EVENT_BAD_CSEQ_NUM        6
#define SIP_EVENT_BAD_CSEQ_NAME       7
#define SIP_EVENT_EMPTY_FROM          8
#define SIP_EVENT_BAD_FROM			  9
#define SIP_EVENT_EMPTY_TO		      10
#define SIP_EVENT_BAD_TO	          11
#define SIP_EVENT_EMPTY_VIA		      12
#define SIP_EVENT_BAD_VIA             13
#define SIP_EVENT_EMPTY_CONTACT       14
#define SIP_EVENT_BAD_CONTACT         15
#define SIP_EVENT_BAD_CONTENT_LEN	  16
#define SIP_EVENT_MULTI_MSGS		  17
#define SIP_EVENT_MISMATCH_CONTENT_LEN	        18
#define SIP_EVENT_INVALID_CSEQ_NAME             19
#define SIP_EVENT_AUTH_INVITE_REPLAY_ATTACK		20
#define SIP_EVENT_AUTH_INVITE_DIFF_SESSION      21
#define SIP_EVENT_BAD_STATUS_CODE               22
#define SIP_EVENT_EMPTY_CONTENT_TYPE            23
#define SIP_EVENT_INVALID_VERSION               24
#define SIP_EVENT_MISMATCH_METHOD               25
#define SIP_EVENT_UNKOWN_METHOD                 26
#define SIP_EVENT_MAX_DIALOGS_IN_A_SESSION      27

/*
 * SIP preprocessor alert strings.
 */
#define SIP_EVENT_MAX_SESSIONS_STR	     "(spp_sip) Maximum sessions reached"
#define SIP_EVENT_EMPTY_REQUEST_URI_STR	 "(spp_sip) Empty request URI"
#define	SIP_EVENT_BAD_URI_STR		     "(spp_sip) URI is too long"
#define SIP_EVENT_EMPTY_CALL_ID_STR	     "(spp_sip) Empty call-Id"
#define SIP_EVENT_BAD_CALL_ID_STR 		 "(spp_sip) Call-Id is too long"
#define SIP_EVENT_BAD_CSEQ_NUM_STR	     "(spp_sip) CSeq number is too large or negative"
#define SIP_EVENT_BAD_CSEQ_NAME_STR      "(spp_sip) Request name in CSeq is too long"
#define SIP_EVENT_EMPTY_FROM_STR	     "(spp_sip) Empty From header"
#define SIP_EVENT_BAD_FROM_STR 		     "(spp_sip) From header is too long"
#define SIP_EVENT_EMPTY_TO_STR	         "(spp_sip) Empty To header"
#define SIP_EVENT_BAD_TO_STR 		     "(spp_sip) To header is too long"
#define SIP_EVENT_EMPTY_VIA_STR	         "(spp_sip) Empty Via header"
#define SIP_EVENT_BAD_VIA_STR 		     "(spp_sip) Via header is too long"
#define SIP_EVENT_EMPTY_CONTACT_STR	     "(spp_sip) Empty Contact"
#define SIP_EVENT_BAD_CONTACT_STR 		 "(spp_sip) Contact is too long"
#define SIP_EVENT_BAD_CONTENT_LEN_STR 	 "(spp_sip) Content length is too large or negative"
#define SIP_EVENT_MULTI_MSGS_STR         "(spp_sip) Multiple SIP messages in a packet"
#define SIP_EVENT_MISMATCH_CONTENT_LEN_STR        "(spp_sip) Content length mismatch"
#define SIP_EVENT_INVALID_CSEQ_NAME_STR           "(spp_sip) Request name is invalid"
#define SIP_EVENT_AUTH_INVITE_REPLAY_ATTACK_STR	  "(spp_sip) Invite replay attack"
#define SIP_EVENT_AUTH_INVITE_DIFF_SESSION_STR 	  "(spp_sip) Illegal session information modification"
#define SIP_EVENT_BAD_STATUS_CODE_STR 	  "(spp_sip) Response status code is not a 3 digit number"
#define SIP_EVENT_EMPTY_CONTENT_TYPE_STR  "(spp_sip) Empty Content-type header"
#define SIP_EVENT_INVALID_VERSION_STR     "(spp_sip) SIP version is invalid"
#define SIP_EVENT_MISMATCH_METHOD_STR     "(spp_sip) Mismatch in METHOD of request and the CSEQ header"
#define SIP_EVENT_UNKOWN_METHOD_STR       "(spp_sip) Method is unknown"
#define SIP_EVENT_MAX_DIALOGS_IN_A_SESSION_STR "(spp_sip) Maximum dialogs within a session reached"

#define MAX_STAT_CODE      999
#define MIN_STAT_CODE      100
#define TOTAL_RESPONSES 0
#define RESPONSE1XX     1
#define RESPONSE2XX     2
#define RESPONSE3XX     3
#define RESPONSE4XX     4
#define RESPONSE5XX     5
#define RESPONSE6XX     6
#define NUM_OF_RESPONSE_TYPES  10
#define TOTAL_REQUESTS 0
#define NUM_OF_REQUEST_TYPES  SIP_METHOD_USER_DEFINE_MAX

typedef struct _SIP_Stats
{
    uint64_t sessions;
    uint64_t events;

    uint64_t dialogs;
    uint64_t requests[NUM_OF_REQUEST_TYPES];
    uint64_t responses[NUM_OF_RESPONSE_TYPES];
    uint64_t ignoreChannels;
    uint64_t ignoreSessions;

} SIP_Stats;

extern SIP_Stats sip_stats;
extern SIPConfig *sip_eval_config;
extern tSfPolicyUserContextId sip_config;


/* Prototypes for public interface */
void SetupSIP(void);

SIPConfig *getParsingSIPConfig(struct _SnortConfig *);

#endif /* SPP_SIP_H */
