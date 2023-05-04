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
 * 
 ****************************************************************************/

#ifndef _DCE2_STATS_H_
#define _DCE2_STATS_H_

#include "dce2_utils.h"
#include "smb.h"
#include "sf_types.h"

/********************************************************************
 * Structures
 ********************************************************************/
typedef struct _DCE2_Stats
{
    uint64_t sessions;
    uint64_t sessions_active;
    uint64_t sessions_autodetected;
    uint64_t sessions_aborted;
    uint64_t bad_autodetects;
    uint64_t events;

#ifdef DEBUG
    uint64_t autoports[65535][DCE2_TRANS_TYPE__MAX];
#endif

    uint64_t smb_sessions;
    uint64_t smb_pkts;
    uint64_t smb_ignored_bytes;
    uint64_t smb_cli_seg_reassembled;
    uint64_t smb_srv_seg_reassembled;
    uint64_t smb_max_outstanding_requests;

    uint64_t smb_com_stats[2][SMB_MAX_NUM_COMS];
    uint64_t smb_chained_stats[2][SMB_ANDX_COM__MAX][SMB_MAX_NUM_COMS];
    // The +1 is for codes beyond the range of the highest valid subcommand code
    // Indicates a bogus subcommand 
    uint64_t smb_trans_subcom_stats[2][TRANS_SUBCOM_MAX+1];
    uint64_t smb_trans2_subcom_stats[2][TRANS2_SUBCOM_MAX+1];
    uint64_t smb_nt_transact_subcom_stats[2][NT_TRANSACT_SUBCOM_MAX+1];

    uint64_t smb_files_processed;

    uint64_t tcp_sessions;
    uint64_t tcp_pkts;

    uint64_t udp_sessions;
    uint64_t udp_pkts;

    uint64_t http_proxy_sessions;
    uint64_t http_proxy_pkts;

    uint64_t http_server_sessions;
    uint64_t http_server_pkts;

    uint64_t co_pdus;
    uint64_t co_bind;
    uint64_t co_bind_ack;
    uint64_t co_alter_ctx;
    uint64_t co_alter_ctx_resp;
    uint64_t co_bind_nack;
    uint64_t co_request;
    uint64_t co_response;
    uint64_t co_cancel;
    uint64_t co_orphaned;
    uint64_t co_fault;
    uint64_t co_auth3;
    uint64_t co_shutdown;
    uint64_t co_reject;
    uint64_t co_ms_pdu;
    uint64_t co_other_req;
    uint64_t co_other_resp;
    uint64_t co_req_fragments;
    uint64_t co_resp_fragments;
    uint64_t co_cli_max_frag_size;
    uint64_t co_cli_min_frag_size;
    uint64_t co_cli_seg_reassembled;
    uint64_t co_cli_frag_reassembled;
    uint64_t co_srv_max_frag_size;
    uint64_t co_srv_min_frag_size;
    uint64_t co_srv_seg_reassembled;
    uint64_t co_srv_frag_reassembled;

    uint64_t cl_pkts;
    uint64_t cl_request;
    uint64_t cl_ack;
    uint64_t cl_cancel;
    uint64_t cl_cli_fack;
    uint64_t cl_ping;
    uint64_t cl_response;
    uint64_t cl_reject;
    uint64_t cl_cancel_ack;
    uint64_t cl_srv_fack;
    uint64_t cl_fault;
    uint64_t cl_nocall;
    uint64_t cl_working;
    uint64_t cl_other_req;
    uint64_t cl_other_resp;
    uint64_t cl_fragments;
    uint64_t cl_max_frag_size;
    uint64_t cl_frag_reassembled;
    uint64_t cl_max_seqnum;

    /* SMB2 stats */
    uint64_t smb2_prunes;
    uint64_t smb2_memory_in_use;
    uint64_t smb2_memory_in_use_max;
    uint64_t smb2_create;
    uint64_t smb2_write;
    uint64_t smb2_read;
    uint64_t smb2_set_info;
    uint64_t smb2_tree_connect;
    uint64_t smb2_tree_disconnect;
    uint64_t smb2_close;

} DCE2_Stats;

/********************************************************************
 * Extern variables
 ********************************************************************/
extern DCE2_Stats dce2_stats;
extern char **dce2_trans_strs;

/********************************************************************
 * Public function prototypes
 ********************************************************************/
void DCE2_StatsInit(void);
void DCE2_StatsFree(void);

#endif  /* _DCE2_STATS_H_ */

