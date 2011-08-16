/****************************************************************************
 * Copyright (C) 2008-2011 Sourcefire, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************
 * 
 ****************************************************************************/

#ifndef _DCE2_STATS_H_
#define _DCE2_STATS_H_

#include "dce2_utils.h"
#include "sf_types.h"

/********************************************************************
 * Structures
 ********************************************************************/
typedef struct _DCE2_Stats
{
    uint64_t sessions;
    uint64_t missed_bytes;
    uint64_t overlapped_bytes;
    uint64_t sessions_autodetected;
    uint64_t bad_autodetects;
    uint64_t events;

#ifdef DEBUG
    uint64_t autoports[65535][DCE2_TRANS_TYPE__MAX];
#endif

    uint64_t smb_sessions;
    uint64_t smb_pkts;
    uint64_t smb_ignored_bytes;
    uint64_t smb_non_ipc_packets;
    uint64_t smb_nbss_not_message;
    uint64_t smb_seg_reassembled;

    uint64_t smb_ssx_chained;
    uint64_t smb_ssx_req;
    uint64_t smb_ssx_req_chained_loffx;
    uint64_t smb_ssx_req_chained_tc;
    uint64_t smb_ssx_req_chained_tcx;
    uint64_t smb_ssx_req_chained_tdis;
    uint64_t smb_ssx_req_chained_open;
    uint64_t smb_ssx_req_chained_openx;
    uint64_t smb_ssx_req_chained_ntcx;
    uint64_t smb_ssx_req_chained_close;
    uint64_t smb_ssx_req_chained_trans;
    uint64_t smb_ssx_req_chained_write;
    uint64_t smb_ssx_req_chained_readx;
    uint64_t smb_ssx_req_chained_other;
    uint64_t smb_ssx_resp;
    uint64_t smb_ssx_resp_chained_loffx;
    uint64_t smb_ssx_resp_chained_tc;
    uint64_t smb_ssx_resp_chained_tcx;
    uint64_t smb_ssx_resp_chained_tdis;
    uint64_t smb_ssx_resp_chained_open;
    uint64_t smb_ssx_resp_chained_openx;
    uint64_t smb_ssx_resp_chained_ntcx;
    uint64_t smb_ssx_resp_chained_close;
    uint64_t smb_ssx_resp_chained_trans;
    uint64_t smb_ssx_resp_chained_write;
    uint64_t smb_ssx_resp_chained_readx;
    uint64_t smb_ssx_resp_chained_other;

    uint64_t smb_loffx_chained;
    uint64_t smb_loffx_req;
    uint64_t smb_loffx_req_chained_ssx;
    uint64_t smb_loffx_req_chained_tcx;
    uint64_t smb_loffx_req_chained_tdis;
    uint64_t smb_loffx_req_chained_other;
    uint64_t smb_loffx_resp;
    uint64_t smb_loffx_resp_chained_ssx;
    uint64_t smb_loffx_resp_chained_tcx;
    uint64_t smb_loffx_resp_chained_tdis;
    uint64_t smb_loffx_resp_chained_other;

    uint64_t smb_tc_req;
    uint64_t smb_tc_resp;

    uint64_t smb_tcx_chained;
    uint64_t smb_tcx_req;
    uint64_t smb_tcx_req_chained_ssx;
    uint64_t smb_tcx_req_chained_loffx;
    uint64_t smb_tcx_req_chained_tdis;
    uint64_t smb_tcx_req_chained_open;
    uint64_t smb_tcx_req_chained_openx;
    uint64_t smb_tcx_req_chained_ntcx;
    uint64_t smb_tcx_req_chained_close;
    uint64_t smb_tcx_req_chained_trans;
    uint64_t smb_tcx_req_chained_write;
    uint64_t smb_tcx_req_chained_readx;
    uint64_t smb_tcx_req_chained_other;
    uint64_t smb_tcx_resp;
    uint64_t smb_tcx_resp_chained_ssx;
    uint64_t smb_tcx_resp_chained_loffx;
    uint64_t smb_tcx_resp_chained_tdis;
    uint64_t smb_tcx_resp_chained_open;
    uint64_t smb_tcx_resp_chained_openx;
    uint64_t smb_tcx_resp_chained_ntcx;
    uint64_t smb_tcx_resp_chained_close;
    uint64_t smb_tcx_resp_chained_trans;
    uint64_t smb_tcx_resp_chained_write;
    uint64_t smb_tcx_resp_chained_readx;
    uint64_t smb_tcx_resp_chained_other;

    uint64_t smb_tdis_req;
    uint64_t smb_tdis_resp;

    uint64_t smb_open_req;
    uint64_t smb_open_resp;

    uint64_t smb_openx_chained;
    uint64_t smb_openx_req;
    uint64_t smb_openx_req_chained_ssx;
    uint64_t smb_openx_req_chained_loffx;
    uint64_t smb_openx_req_chained_tc;
    uint64_t smb_openx_req_chained_tcx;
    uint64_t smb_openx_req_chained_tdis;
    uint64_t smb_openx_req_chained_open;
    uint64_t smb_openx_req_chained_openx;
    uint64_t smb_openx_req_chained_ntcx;
    uint64_t smb_openx_req_chained_close;
    uint64_t smb_openx_req_chained_write;
    uint64_t smb_openx_req_chained_readx;
    uint64_t smb_openx_req_chained_other;
    uint64_t smb_openx_resp;
    uint64_t smb_openx_resp_chained_ssx;
    uint64_t smb_openx_resp_chained_loffx;
    uint64_t smb_openx_resp_chained_tc;
    uint64_t smb_openx_resp_chained_tcx;
    uint64_t smb_openx_resp_chained_tdis;
    uint64_t smb_openx_resp_chained_open;
    uint64_t smb_openx_resp_chained_openx;
    uint64_t smb_openx_resp_chained_ntcx;
    uint64_t smb_openx_resp_chained_close;
    uint64_t smb_openx_resp_chained_write;
    uint64_t smb_openx_resp_chained_readx;
    uint64_t smb_openx_resp_chained_other;

    uint64_t smb_ntcx_chained;
    uint64_t smb_ntcx_req;
    uint64_t smb_ntcx_req_chained_ssx;
    uint64_t smb_ntcx_req_chained_loffx;
    uint64_t smb_ntcx_req_chained_tc;
    uint64_t smb_ntcx_req_chained_tcx;
    uint64_t smb_ntcx_req_chained_tdis;
    uint64_t smb_ntcx_req_chained_open;
    uint64_t smb_ntcx_req_chained_openx;
    uint64_t smb_ntcx_req_chained_ntcx;
    uint64_t smb_ntcx_req_chained_close;
    uint64_t smb_ntcx_req_chained_write;
    uint64_t smb_ntcx_req_chained_readx;
    uint64_t smb_ntcx_req_chained_other;
    uint64_t smb_ntcx_resp;
    uint64_t smb_ntcx_resp_chained_ssx;
    uint64_t smb_ntcx_resp_chained_loffx;
    uint64_t smb_ntcx_resp_chained_tc;
    uint64_t smb_ntcx_resp_chained_tcx;
    uint64_t smb_ntcx_resp_chained_tdis;
    uint64_t smb_ntcx_resp_chained_open;
    uint64_t smb_ntcx_resp_chained_openx;
    uint64_t smb_ntcx_resp_chained_ntcx;
    uint64_t smb_ntcx_resp_chained_close;
    uint64_t smb_ntcx_resp_chained_write;
    uint64_t smb_ntcx_resp_chained_readx;
    uint64_t smb_ntcx_resp_chained_other;

    uint64_t smb_close_req;
    uint64_t smb_close_resp;

    uint64_t smb_write_req;
    uint64_t smb_write_resp;

    uint64_t smb_writebr_req;
    uint64_t smb_writebr_resp;

    uint64_t smb_writex_chained;
    uint64_t smb_writex_req;
    uint64_t smb_writex_req_chained_ssx;
    uint64_t smb_writex_req_chained_loffx;
    uint64_t smb_writex_req_chained_tc;
    uint64_t smb_writex_req_chained_tcx;
    uint64_t smb_writex_req_chained_openx;
    uint64_t smb_writex_req_chained_ntcx;
    uint64_t smb_writex_req_chained_close;
    uint64_t smb_writex_req_chained_write;
    uint64_t smb_writex_req_chained_writex;
    uint64_t smb_writex_req_chained_read;
    uint64_t smb_writex_req_chained_readx;
    uint64_t smb_writex_req_chained_other;
    uint64_t smb_writex_resp;
    uint64_t smb_writex_resp_chained_ssx;
    uint64_t smb_writex_resp_chained_loffx;
    uint64_t smb_writex_resp_chained_tc;
    uint64_t smb_writex_resp_chained_tcx;
    uint64_t smb_writex_resp_chained_openx;
    uint64_t smb_writex_resp_chained_ntcx;
    uint64_t smb_writex_resp_chained_close;
    uint64_t smb_writex_resp_chained_write;
    uint64_t smb_writex_resp_chained_writex;
    uint64_t smb_writex_resp_chained_read;
    uint64_t smb_writex_resp_chained_readx;
    uint64_t smb_writex_resp_chained_other;

    uint64_t smb_writeclose_req;
    uint64_t smb_writeclose_resp;

    uint64_t smb_writecomplete_resp;

    uint64_t smb_trans_req;
    uint64_t smb_trans_sec_req;
    uint64_t smb_trans_resp;

    uint64_t smb_read_req;
    uint64_t smb_read_resp;

    uint64_t smb_readbr_req;
    uint64_t smb_readbr_resp;

    uint64_t smb_readx_chained;
    uint64_t smb_readx_req;
    uint64_t smb_readx_req_chained_ssx;
    uint64_t smb_readx_req_chained_loffx;
    uint64_t smb_readx_req_chained_tc;
    uint64_t smb_readx_req_chained_tcx;
    uint64_t smb_readx_req_chained_tdis;
    uint64_t smb_readx_req_chained_openx;
    uint64_t smb_readx_req_chained_ntcx;
    uint64_t smb_readx_req_chained_close;
    uint64_t smb_readx_req_chained_write;
    uint64_t smb_readx_req_chained_readx;
    uint64_t smb_readx_req_chained_other;
    uint64_t smb_readx_resp;
    uint64_t smb_readx_resp_chained_ssx;
    uint64_t smb_readx_resp_chained_loffx;
    uint64_t smb_readx_resp_chained_tc;
    uint64_t smb_readx_resp_chained_tcx;
    uint64_t smb_readx_resp_chained_tdis;
    uint64_t smb_readx_resp_chained_openx;
    uint64_t smb_readx_resp_chained_ntcx;
    uint64_t smb_readx_resp_chained_close;
    uint64_t smb_readx_resp_chained_write;
    uint64_t smb_readx_resp_chained_readx;
    uint64_t smb_readx_resp_chained_other;

    uint64_t smb_rename_req;
    uint64_t smb_rename_resp;

    uint64_t smb_other_req;
    uint64_t smb_other_resp;

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

} DCE2_Stats;

/********************************************************************
 * Public function prototypes
 ********************************************************************/
void DCE2_StatsInit(void);
void DCE2_StatsFree(void);

#endif  /* _DCE2_STATS_H_ */

