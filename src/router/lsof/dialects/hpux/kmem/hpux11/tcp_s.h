/*
 * tcp_s.h for HP-UX 10.30 and above
 *
 * This header file defines the TCP connection structure, tpc_s, for lsof.
 * Lsof gets the parameters of a TCP connection from tcp_s.  Lsof locates a
 * tcp_s structure by scanning the queue structure chain of a TCP stream,
 * looking for a queue structure whose module name begins with TCP; that queue
 * structure's private data pointer, q_ptr, addresses its associated tcp_s
 * structure.
 *
 * V. Abell
 * February, 1998
 */

#if	!defined(LSOF_TCP_S_H)
#define	LSOF_TCP_S_H

#include "kernbits.h"
#include <sys/types.h>

#define TCPS_CLOSED             -6
#define TCPS_IDLE               -5
#define TCPS_BOUND              -4
#define TCPS_LISTEN             -3
#define TCPS_SYN_SENT           -2
#define TCPS_SYN_RCVD           -1
#define TCPS_ESTABLISHED         0
#define TCPS_CLOSE_WAIT          1
#define TCPS_FIN_WAIT_1          2
#define TCPS_CLOSING             3
#define TCPS_LAST_ACK            4
#define TCPS_FIN_WAIT_2          5
#define TCPS_TIME_WAIT           6

typedef struct iph_s {			/* IP header */
	u_char iph_version_and_hdr_length;
	u_char iph_type_of_service;
	u_char iph_length[2];
	u_char iph_ident[2];
	u_char iph_fragment_offset_and_flags[2];
	u_char iph_ttl;
	u_char iph_protocol;
	u_char iph_hdr_checksum[2];
	u_char iph_src[4];		/* source IP address */
	u_char iph_dst[4];		/* destination IP address */
} iph_t;

typedef struct ipha_s {
	u_char ipha_version_and_hdr_length;
	u_char ipha_type_of_service;
	uint16_t ipha_length;
	uint16_t ipha_ident;
	uint16_t ipha_fragment_offset_and_flags;
	u_char ipha_ttl;
	u_char ipha_protocol;
	uint16_t ipha_hdr_checksum;
	uint32_t ipha_src;
	uint32_t ipha_dst;
} ipha_t;

typedef struct tcphdr_s {
	uint16_t th_lport;		/* local port */
	uint16_t th_fport;		/* foreign port */
	u_char th_seq[4];
	u_char th_ack[4];
	u_char th_offset_and_rsrvd[1];
	u_char th_flags[1];
	u_char th_win[2];
	u_char th_sum[2];
	u_char th_urp[2];
} tcph_t;

typedef struct tcpsb {
	uint32_t tcpsb_start;
	uint32_t tcpsb_end;
} tcpsb_t;

typedef struct tcp_s {
	int tcp_state;			/* connection state */
	KA_T tcp_rq;
	KA_T tcp_wq;
	KA_T tcp_xmit_head;
	KA_T tcp_xmit_last;
	uint tcp_unsent;
	KA_T tcp_xmit_tail;
	uint tcp_xmit_tail_unsent;
	uint32_t tcp_snxt;		/* send: next sequence number */
	uint32_t tcp_suna;		/* send: unacknowledged sequence nr */
	uint tcp_swnd;			/* send: window size */
	uint tcp_swnd_shift;
	uint tcp_cwnd;
	u_long tcp_ibsegs;
	u_long tcp_obsegs;
	uint tcp_mss;
	uint tcp_naglim;
	int tcp_hdr_len;		/* TCP header length */
	int tcp_wroff_extra;
	KA_T tcp_tcph;			/* pointer to TCP header structure */
	int tcp_tcp_hdr_len;
	uint tcp_valid_bits;
	int tcp_xmit_hiwater;
	KA_T tcp_flow_mp;
	int tcp_ms_we_have_waited;
	KA_T tcp_timer_mp;
	uint tcp_timer_interval;
	uint32_t
	    tcp_urp_old : 1,
	    tcp_urp_sig_sent : 1,
	    tcp_hard_binding : 1,
	    tcp_hard_bound : 1,
	    tcp_priv_stream : 1,
	    tcp_fin_acked : 1,
	    tcp_fin_rcvd : 1,
	    tcp_fin_sent : 1,
	    tcp_ordrel_done : 1,
	    tcp_flow_stopped : 1,
	    tcp_detached : 1,
	    tcp_bind_pending : 1,
	    tcp_unbind_pending : 1,
	    tcp_use_ts_opts : 1,
	    tcp_reader_active : 1,
	    tcp_lingering : 1,			/* SO_LINGER */
	    tcp_no_bind : 1,
	    tcp_sack_permitted : 1,
	    tcp_rexmitting : 1,
	    tcp_fast_rexmitting : 1,
	    tcp_ooofin_seen : 1,
	    tcp_smooth_rtt : 1,
	    tcp_main_flags_fill : 10;
	uint32_t
	    tcp_so_debug : 1,			/* SO_DEBUG */
	    tcp_dontroute : 1,			/* SO_DONTROUTE */
	    tcp_broadcast : 1,			/* SO_BROADCAST */
	    tcp_useloopback : 1,		/* SO_USELOOPBACK */
	    tcp_reuseaddr : 1,			/* SO_REUSEADDR */
	    tcp_reuseport : 1,			/* SO_REUSEPORT */
	    tcp_oobinline : 1,			/* SO_OOBONLINE */
	    tcp_xmit_hiwater_set : 1,
	    tcp_xmit_lowater_set : 1,
	    tcp_recv_hiwater_set : 1,
	    tcp_no_window_shift : 1,
	    tcp_window_shift_set : 1,
	    tcp_keepalive_kills : 1,
	    tcp_option_flags_fill : 19;
	uint tcp_dupack_cnt;
	uint32_t tcp_smax;
	uint32_t tcp_rnxt;
	uint tcp_rwnd;			/* read: window size */
	uint tcp_rwnd_shift;
	uint tcp_rwnd_max;
	int tcp_credit;
	int tcp_credit_init;
	KA_T tcp_reass_head;
	KA_T tcp_reass_tail;
	KA_T tcp_rcv_head;
	KA_T tcp_rcv_tail;
	uint tcp_rcv_cnt;
	uint tcp_rcv_threshold;
	uint tcp_cwnd_ssthresh;
	uint tcp_cwnd_bytes_acked;
	uint tcp_cwnd_max;
	uint tcp_cwnd_init;
	int tcp_rto;
	int tcp_rtt_sa;
	int tcp_rtt_sd;
	uint tcp_swl1;
	uint tcp_swl2;
	uint32_t tcp_rack;		/* read: acknowledged sequent number */
	uint tcp_rack_cnt;
	uint tcp_rack_cur_max;
	uint tcp_rack_abs_max;
	KA_T tcp_ts_ptr;
	uint32_t tcp_ts_updated;
	uint tcp_max_swnd;
	uint tcp_sack_cnt;
	tcpsb_t tcp_sack_blocks[4];
	KA_T tcp_listener;
	int tcp_xmit_lowater;
	uint32_t tcp_irs;
	uint32_t tcp_iss;
	uint32_t tcp_fss;
	uint32_t tcp_urg;
	uint32_t tcp_ooofin_seq;
	int tcp_ip_hdr_len;
	int tcp_first_timer_threshold;
	int tcp_second_timer_threshold;
	uint32_t tcp_zero_win_suna;
	int tcp_first_ctimer_threshold;
	int tcp_second_ctimer_threshold;
	int tcp_linger;				/* SO_LINGER interval */
	KA_T tcp_urp_mp;
	KA_T tcp_eager_next;
	KA_T tcp_eager_prev;
	KA_T tcp_eager_data;
	KA_T tcp_conn_ind_mp;
	uint tcp_conn_ind_cnt;
	uint tcp_conn_ind_max;
	uint tcp_conn_ind_seqnum;
	KA_T tcp_conn_ind_list;
	KA_T tcp_pre_conn_ind_list;
	int tcp_keepalive_intrvl;		/* SO_KEEPALIVE interval */
	int tcp_keepalive_detached_intrvl;
	KA_T tcp_keepalive_mp;
	int tcp_client_errno;
	union {
	    iph_t tcp_u_iph;			/* IP header */
	    ipha_t tcp_u_ipha;
	    char tcp_u_buf[128];
	    double tcp_u_aligner;
	} tcp_u;
	uint32_t tcp_sum;
	uint32_t tcp_remote;
	uint32_t tcp_bound_source;
	uint tcp_last_sent_len;
	KA_T tcp_cookie;
	KA_T tcp_hnext_port;
	KA_T tcp_ptphn_port;
	KA_T tcp_hnext_listener;
	KA_T tcp_ptphn_listener;
	KA_T tcp_hnext_established;
	KA_T tcp_ptphn_established;
	uint tcp_mirg;
	KA_T tcp_readers_next;
	KA_T tcp_readers_ptpn;
} tcp_s_t;

#endif	/* !defined(LSOF_TCP_S_H) */
