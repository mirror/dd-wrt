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
 * spp_gtp.h: Definitions, structs, function prototype(s) for
 *		the GTP preprocessor.
 * Author: Hui Cao
 */

#ifndef SPP_GTP_H
#define SPP_GTP_H
#include <stddef.h>
#include "sfPolicy.h"
#include "sfPolicyUserData.h"
#include "snort_bounds.h"
#include "gtp_roptions.h"


/* Convert port value into an index for the gtp_config->ports array */
#define PORT_INDEX(port) port/8

/* Convert port value into a value for bitwise operations */
#define CONV_PORT(port) 1<<(port%8)

/*
 * Boolean values.
 */
#define GTP_TRUE	(1)
#define GTP_FALSE	(0)

/*
 * Error codes.
 */
#define GTP_SUCCESS	(1)
#define GTP_FAILURE	(0)


/*
 * Per-session data block containing current state
 * of the GTP preprocessor for the session.
 *
 * state_flags:		Bit vector describing the current state of the
 * 				session.
 */
typedef struct _gtpData
{

    uint32_t state_flags;
    GTP_Roptions ropts;
    tSfPolicyId policy_id;
    tSfPolicyUserContextId config;

} GTPData;

typedef struct _GTPMsg
{
    uint8_t version;
    uint8_t msg_type;
    uint16_t msg_length;
    uint16_t header_len;
    uint8_t *gtp_header;
    GTP_IEData *info_elements;
    /* nothing after this point is zeroed ...*/
    uint32_t msg_id; /*internal state, new msg will have a new id*/

} GTPMsg;

#define GTPMSG_ZERO_LEN offsetof(GTPMsg, msg_id)

/*
 * Generator id. Define here the same as the official registry
 * in generators.h
 */
#define GENERATOR_SPP_GTP	143

/* Ultimately calls SnortEventqAdd */
/* Arguments are: gid, sid, rev, classification, priority, message, rule_info */
#define ALERT(x,y) { _dpd.alertAdd(GENERATOR_SPP_GTP, x, 1, 0, 3, y, 0 ); gtp_stats.events++; }

/*
 * GTP preprocessor alert types.
 */
#define GTP_EVENT_BAD_MSG_LEN        (1)
#define GTP_EVENT_BAD_IE_LEN         (2)
#define GTP_EVENT_OUT_OF_ORDER_IE    (3)
#define GTP_TEID_MISSING             (4)
/*
 * GTP preprocessor alert strings.
 */
#define GTP_EVENT_BAD_MSG_LEN_STR	     "(spp_gtp) Message length is invalid"
#define GTP_EVENT_BAD_IE_LEN_STR	     "(spp_gtp) Information element length is invalid"
#define	GTP_EVENT_OUT_OF_ORDER_IE_STR	 "(spp_gtp) Information elements are out of order"
#define GTP_TEID_MISSING_STR                 "(spp_gtp) TEID is Missing"

typedef struct _GTP_Stats
{
    uint64_t sessions;
    uint64_t events;
    uint64_t unknownTypes;
    uint64_t unknownIEs;
    uint64_t messages[MAX_GTP_VERSION_CODE + 1][MAX_GTP_TYPE_CODE + 1];
    GTP_MsgType *msgTypeTable[MAX_GTP_VERSION_CODE + 1][MAX_GTP_TYPE_CODE + 1];

} GTP_Stats;

extern GTP_Stats gtp_stats;
extern GTPConfig *gtp_eval_config;
extern tSfPolicyUserContextId gtp_config;

/* Prototypes for public interface */
void SetupGTP(void);

#endif /* SPP_GTP_H */
