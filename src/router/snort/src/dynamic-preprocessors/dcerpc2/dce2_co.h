/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2008-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************
 * 8/17/2008 - Initial implementation ... Todd Wease <twease@sourcefire.com>
 * 
 ****************************************************************************/

#ifndef _DCE2_CO_H_
#define _DCE2_CO_H_

#include "dce2_session.h"
#include "dce2_list.h"
#include "dce2_utils.h"
#include "dcerpc.h"
#include "sf_types.h"
#include "sf_snort_packet.h"

/********************************************************************
 * Macros
 ********************************************************************/
#define DCE2_MOCK_HDR_LEN__CO_CLI   (sizeof(DceRpcCoHdr) + sizeof(DceRpcCoRequest))
#define DCE2_MOCK_HDR_LEN__CO_SRV   (sizeof(DceRpcCoHdr) + sizeof(DceRpcCoResponse))

/********************************************************************
 * Structures
 ********************************************************************/
typedef struct _DCE2_CoFragTracker
{
    DCE2_Buffer *cli_stub_buf;
    DCE2_Buffer *srv_stub_buf;

    int opnum;    /* Opnum that is ultimatley used for request */
    int ctx_id;   /* Context id that is ultimatley used for request */

    /* These are set on a first fragment received */
    int expected_call_id;  /* Expected call id for fragments */
    int expected_opnum;    /* Expected call id for fragments */
    int expected_ctx_id;   /* Expected call id for fragments */

} DCE2_CoFragTracker;

typedef struct _DCE2_CoSeg
{
    DCE2_Buffer *buf;

    /* If there is enough data in segmentation buffer for header,
     * this will be set to the frag length in the header */
    uint16_t frag_len;

} DCE2_CoSeg;

typedef struct _DCE2_CoTracker
{
    DCE2_List *ctx_ids;  /* splayed list so most recently used goes to front of list */
    int got_bind;        /* got an accepted bind */

    /* Queue of pending client bind or alter context request context items
     * Since the actual context id number doesn't have to occur sequentially
     * in the context list in the client packet, need to keep track to match
     * up server response since server doesn't reply with actual context id
     * numbers, but in the order they were in the client packet */
    DCE2_Queue *pending_ctx_ids;

    /* Keeps track of fragmentation buffer and frag specfic data */
    DCE2_CoFragTracker frag_tracker;

    int max_xmit_frag;    /* The maximum negotiated size of a client request */
    int data_byte_order;  /* Depending on policy is from bind or request */
    int ctx_id;           /* The current context id of the request */
    int opnum;            /* The current opnum of the request */
    int call_id;          /* The current call id of the request */
    const uint8_t *stub_data;   /* Current pointer to stub data in the request */

    /* For transport segmentation */
    DCE2_CoSeg cli_seg;
    DCE2_CoSeg srv_seg;

} DCE2_CoTracker;

/********************************************************************
 * Public function prototypes
 ********************************************************************/
void DCE2_CoInitRdata(uint8_t *, int);
void DCE2_CoProcess(DCE2_SsnData *, DCE2_CoTracker *, const uint8_t *, uint16_t);
void DCE2_CoInitTracker(DCE2_CoTracker *);
void DCE2_CoCleanTracker(DCE2_CoTracker *);

#endif  /* _DCE2_CO_H_ */

