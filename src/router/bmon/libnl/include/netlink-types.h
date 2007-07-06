/*
 * netlink-types.h	Netlink Types (Private)
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_LOCAL_TYPES_H_
#define NETLINK_LOCAL_TYPES_H_

#include <netlink/list.h>
#include <netlink/route/link.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/rtnl.h>

struct nl_cache_ops;

#define NL_SOCK_BUFSIZE_SET 1
#define NL_SOCK_PASSCRED 2

#define NL_MSG_CRED_PRESENT 1

struct nl_handle;
struct nl_object;

struct nl_cb
{
	nl_recvmsg_msg_cb_t	cb_set[NL_CB_TYPE_MAX+1];
	void *			cb_args[NL_CB_TYPE_MAX+1];
	
	nl_recvmsg_err_cb_t	cb_err;
	void *			cb_err_arg;

	/** May be used to replace nl_recvmsgs with your own implementation
	 * in all internal calls to nl_recvmsgs. */
	int			(*cb_recvmsgs_ow)(struct nl_handle *,
						  struct nl_cb *);

	/** Overwrite internal calls to nl_recv, must return the number of
	 * octets read and allocate a buffer for the received data. */
	int			(*cb_recv_ow)(struct nl_handle *,
					      struct sockaddr_nl *,
					      unsigned char **,
					      struct ucred **);

	/** Overwrites internal calls to nl_send, must send the netlink
	 * message. */
	int			(*cb_send_ow)(struct nl_handle *,
					      struct nl_msg *);
};

struct nl_handle
{
	struct sockaddr_nl	h_local;
	struct sockaddr_nl	h_peer;
	int			h_fd;
	int			h_proto;
	unsigned int		h_seq_next;
	unsigned int		h_seq_expect;
	int			h_flags;
	struct nl_cb *		h_cb;
};

struct nl_cache
{
	struct nl_list_head	c_items;
	int			c_nitems;
	int                     c_iarg1;
	int                     c_iarg2;
	struct nl_cache_ops *   c_ops;
};

struct nl_parser_param;

struct nl_msgtype
{
	int			mt_id;
	char *			mt_name;
};

struct nl_cache_ops
{
	char  *			co_name;
	size_t			co_size;
	int			co_hdrsize;
	int			co_protocol;
	
	/**
	 * Called whenever an update of the cache is required. Must send
	 * a request message to the kernel requesting a complete dump.
	 */
	int   (*co_request_update)(struct nl_cache *, struct nl_handle *);

	/**
	 * Called whenever a new object was allocated
	 */
	void  (*co_constructor)(struct nl_object *);

	/**
	 * Called whenever a object in the cache gets destroyed, must
	 * free the type specific memory allocations
	 */
	void  (*co_free_data)(struct nl_object *);

	/**
	 * Called whenever a message was received that needs to be parsed.
	 * Must parse the message and call the paser callback function
	 * (nl_parser_param) provided via the argument.
	 */
	int   (*co_msg_parser)(struct sockaddr_nl *, struct nlmsghdr *, void *);

	/**
	 * Called whenever a dump of a cache object is requested. Must
	 * dump the specified object to the specified file descriptor
	 */
	int   (*co_dump[NL_DUMP_MAX+1])(struct nl_object *,
					struct nl_dump_params *);

	/**
	 * Must compare the two specified objects and return a non-zero
	 * value if they match.
	 */
	int   (*co_filter)(struct nl_object *, struct nl_object *);


	struct nl_cache_ops *co_next;
	struct nl_cache *co_major_cache;
	struct nl_msgtype	co_msgtypes[];
};

#define NLHDR_COMMON				\
	int			ce_refcnt;	\
	struct nl_cache_ops *	ce_ops;		\
	struct nl_cache *	ce_cache;	\
	struct nl_object *	ce_dataref;	\
	struct nl_list_head	ce_list;	\
	int			ce_msgtype;

struct nl_object
{
	NLHDR_COMMON
};

struct nl_parser_param
{
	int             (*pp_cb)(struct nl_object *, struct nl_parser_param *);
	void *            pp_arg;
};

struct nl_data
{
	size_t			d_size;
	void *			d_data;
};

struct nl_addr
{
	int			a_family;
	unsigned int		a_maxsize;
	unsigned int		a_len;
	int			a_prefixlen;
	int			a_refcnt;
	unsigned char		a_addr[0];
};

struct nl_msg
{
	int			nm_protocol;
	int			nm_flags;
	struct sockaddr_nl	nm_src;
	struct sockaddr_nl	nm_dst;
	struct ucred		nm_creds;
	struct nlmsghdr *	nm_nlh;
};

struct rtnl_link_map
{
	uint64_t lm_mem_start;
	uint64_t lm_mem_end;
	uint64_t lm_base_addr;
	uint16_t lm_irq;
	uint8_t  lm_dma;
	uint8_t  lm_port;
};

#define IFQDISCSIZ	32

struct rtnl_link
{
	NLHDR_COMMON

	char		l_name[IFNAMSIZ];

	uint32_t	l_family;
	uint32_t	l_arptype;
	uint32_t	l_index;
	uint32_t	l_flags;
	uint32_t	l_change;
	uint32_t 	l_mtu;
	uint32_t	l_link;
	uint32_t	l_txqlen;
	uint32_t	l_weight;
	uint32_t	l_master;
	struct nl_addr *l_addr;	
	struct nl_addr *l_bcast;
	char		l_qdisc[IFQDISCSIZ];
	struct rtnl_link_map l_map;
	uint64_t	l_stats[RTNL_LINK_STATS_MAX+1];
	uint32_t	l_mask;	
	uint32_t	l_flag_mask;
};

struct rtnl_ncacheinfo
{
	uint32_t nci_confirmed;	/**< Time since neighbour validty was last confirmed */
	uint32_t nci_used;	/**< Time since neighbour entry was last ued */
	uint32_t nci_updated;	/**< Time since last update */
	uint32_t nci_refcnt;	/**< Reference counter */
};


struct rtnl_neigh
{
	NLHDR_COMMON
	uint32_t	n_family;
	uint32_t	n_ifindex;
	uint16_t	n_state;
	uint8_t		n_flags;
	uint8_t		n_type;	
	struct nl_addr *n_lladdr;
	struct nl_addr *n_dst;	
	uint32_t	n_probes;
	struct rtnl_ncacheinfo n_cacheinfo;
	uint32_t                n_mask;
	uint32_t                n_state_mask;
	uint32_t                n_flag_mask;
};


struct rtnl_addr_cacheinfo
{
	/* Preferred lifetime in seconds */
	uint32_t aci_prefered;

	/* Valid lifetime in seconds */
	uint32_t aci_valid;

	/* Timestamp of creation in 1/100s seince boottime */
	uint32_t aci_cstamp;

	/* Timestamp of last update in 1/100s since boottime */
	uint32_t aci_tstamp;
};

struct rtnl_addr
{
	NLHDR_COMMON

	uint8_t		a_family;
	uint8_t		a_prefixlen;
	uint8_t		a_flags;
	uint8_t		a_scope;
	uint32_t	a_ifindex;

	struct nl_addr *a_peer;	
	struct nl_addr *a_local;
	struct nl_addr *a_bcast;
	struct nl_addr *a_anycast;
	struct nl_addr *a_multicast;

	struct rtnl_addr_cacheinfo a_cacheinfo;
	
	char a_label[IFNAMSIZ];
	uint32_t a_mask;
	uint32_t a_flag_mask;
};

struct rtnl_nexthop
{
	uint8_t			rtnh_flags;
	uint8_t			rtnh_flag_mask;
	uint8_t			rtnh_weight;
	/* 1 byte spare */
	uint32_t		rtnh_ifindex;
	struct nl_addr *	rtnh_gateway;
	uint32_t		rtnh_mask;

	struct nl_list_head	rtnh_list;
};

struct rtnl_rtcacheinfo
{
	uint32_t	rtci_clntref;
	uint32_t	rtci_last_use;
	uint32_t	rtci_expires;
	int32_t		rtci_error;
	uint32_t	rtci_used;
	uint32_t	rtci_id;
	uint32_t	rtci_ts;
	uint32_t	rtci_tsage;
};

struct rtnl_route
{
	NLHDR_COMMON

	uint8_t			rt_family;
	uint8_t			rt_dst_len;
	uint8_t			rt_src_len;
	uint8_t			rt_tos;
	uint8_t			rt_table;
	uint8_t			rt_protocol;
	uint8_t			rt_scope;
	uint8_t			rt_type;
	uint32_t		rt_flags;
	struct nl_addr *	rt_dst;
	struct nl_addr *	rt_src;
	char			rt_iif[IFNAMSIZ];
	uint32_t		rt_oif;
	struct nl_addr *	rt_gateway;
	uint32_t		rt_prio;
	uint32_t		rt_metrics[RTAX_MAX];
	uint32_t		rt_metrics_mask;
	struct nl_addr *	rt_pref_src;
	struct nl_list_head	rt_nexthops;
	realm_t			rt_realms;
	struct rtnl_rtcacheinfo	rt_cacheinfo;
	uint32_t		rt_mp_algo;
	uint32_t		rt_flag_mask;
	uint32_t		rt_mask;
};

struct rtnl_rule
{
	NLHDR_COMMON

	uint64_t	r_fwmark;
	uint32_t	r_prio;
	uint32_t	r_realms;
	uint32_t	r_table;
	uint8_t		r_dsfield;
	uint8_t		r_type;
	uint8_t		r_family;
	uint8_t		r_src_len;
	uint8_t		r_dst_len;
	char		r_iif[IFNAMSIZ];
	struct nl_addr *r_src;
	struct nl_addr *r_dst;
	struct nl_addr *r_srcmap;

	uint32_t	r_mask;	
};

struct rtnl_neightbl_parms
{
	/**
	 * Interface index of the device this parameter set is assigned
	 * to or 0 for the default set.
	 */
	uint32_t		ntp_ifindex;

	/**
	 * Number of references to this parameter set.
	 */
	uint32_t		ntp_refcnt;

	/**
	 * Queue length for pending arp requests, i.e. the number of
	 * packets which are accepted from other layers while the
	 * neighbour address is still being resolved
	 */
	uint32_t		ntp_queue_len;

	/**
	 * Number of requests to send to the user level ARP daemon.
	 * Specify 0 to disable.
	 */
	uint32_t		ntp_app_probes;

	/**
	 * Maximum number of retries for unicast solicitation.
	 */
	uint32_t		ntp_ucast_probes;

	/**
	 * Maximum number of retries for multicast solicitation.
	 */
	uint32_t		ntp_mcast_probes;

	/**
	 * Base value in milliseconds to ompute reachable time, see RFC2461.
	 */
	uint64_t		ntp_base_reachable_time;

	/**
	 * Actual reachable time (read-only)
	 */
	uint64_t		ntp_reachable_time;	/* secs */

	/**
	 * The time in milliseconds between retransmitted Neighbor
	 * Solicitation messages.
	 */
	uint64_t		ntp_retrans_time;

	/**
	 * Interval in milliseconds to check for stale neighbour
	 * entries.
	 */
	uint64_t		ntp_gc_stale_time;	/* secs */

	/**
	 * Delay in milliseconds for the first time probe if
	 * the neighbour is reachable.
	 */
	uint64_t		ntp_probe_delay;	/* secs */

	/**
	 * Maximum delay in milliseconds of an answer to a neighbour
	 * solicitation message.
	 */
	uint64_t		ntp_anycast_delay;

	/**
	 * Minimum age in milliseconds before a neighbour entry
	 * may be replaced.
	 */
	uint64_t		ntp_locktime;

	/**
	 * Delay in milliseconds before answering to an ARP request
	 * for which a proxy ARP entry exists.
	 */
	uint64_t		ntp_proxy_delay;

	/**
	 * Queue length for the delayed proxy arp requests.
	 */
	uint32_t		ntp_proxy_qlen;
	
	/**
	 * Mask of available parameter attributes
	 */
	uint32_t		ntp_mask;
};

#define NTBLNAMSIZ	32

/**
 * Neighbour table
 * @ingroup neightbl
 */
struct rtnl_neightbl
{
	NLHDR_COMMON

	char			nt_name[NTBLNAMSIZ];
	uint32_t		nt_family;
	uint32_t		nt_gc_thresh1;
	uint32_t		nt_gc_thresh2;
	uint32_t		nt_gc_thresh3;
	uint64_t		nt_gc_interval;
	struct ndt_config	nt_config;
	struct rtnl_neightbl_parms nt_parms;
	struct ndt_stats	nt_stats;
	uint32_t                nt_mask;
};

struct rtnl_ratespec
{
	uint8_t			rs_cell_log;
	uint16_t		rs_feature;
	uint16_t		rs_addend;
	uint16_t		rs_mpu;
	uint32_t		rs_rate;
};

struct rtnl_tstats
{
	struct {
		uint64_t            bytes;
		uint64_t            packets;
	} tcs_basic;

	struct {
		uint32_t            bps;
		uint32_t            pps;
	} tcs_rate_est;

	struct {
		uint32_t            qlen;
		uint32_t            backlog;
		uint32_t            drops;
		uint32_t            requeues;
		uint32_t            overlimits;
	} tcs_queue;
};

#define TCKINDSIZ	32

#define NL_TCA_GENERIC(pre)				\
	NLHDR_COMMON					\
	uint32_t		pre ##_family;		\
	uint32_t		pre ##_ifindex;		\
	uint32_t		pre ##_handle;		\
	uint32_t		pre ##_parent;		\
	uint32_t		pre ##_info;		\
	char			pre ##_kind[TCKINDSIZ];	\
	struct nl_data *	pre ##_opts;		\
	uint64_t		pre ##_stats[RTNL_TC_STATS_MAX+1]; \
	struct nl_data *	pre ##_xstats;		\
	void *			pre ##_subdata;		\
	int			pre ##_mask


struct rtnl_tca
{
	NL_TCA_GENERIC(tc);
};

struct rtnl_qdisc
{
	NL_TCA_GENERIC(q);
	struct rtnl_qdisc_ops	*q_ops;
};

struct rtnl_class
{
	NL_TCA_GENERIC(c);
	struct rtnl_class_ops	*c_ops;
};

struct rtnl_cls
{
	NL_TCA_GENERIC(c);
	uint32_t	c_prio;
	uint32_t	c_protocol;
	struct rtnl_cls_ops	*c_ops;
};

struct rtnl_u32
{
	uint32_t		cu_divisor;
	uint32_t		cu_hash;
	uint32_t		cu_classid;
	uint32_t		cu_link;
	struct nl_data *	cu_pcnt;
	struct nl_data *	cu_selector;
	struct nl_data *	cu_act;
	struct nl_data *	cu_police;
	char			cu_indev[IFNAMSIZ];
	int			cu_mask;
};

struct rtnl_fw
{
	uint32_t		cf_classid;
	struct nl_data *	cf_act;
	struct nl_data *	cf_police;
	char			cf_indev[IFNAMSIZ];
	int			cf_mask;
};

struct rtnl_dsmark_qdisc
{
	uint16_t	qdm_indices;
	uint16_t	qdm_default_index;
	uint32_t	qdm_set_tc_index;
	uint32_t	qdm_mask;
};

struct rtnl_dsmark_class
{
	uint8_t		cdm_bmask;
	uint8_t		cdm_value;
	uint32_t	cdm_mask;
};

struct rtnl_fifo
{
	uint32_t	qf_limit;
	uint32_t	qf_mask;
};

struct rtnl_prio
{
	uint32_t	qp_bands;
	uint8_t		qp_priomap[TC_PRIO_MAX+1];
	uint32_t	qp_mask;
};

struct rtnl_tbf
{
	uint32_t		qt_limit;
	uint32_t		qt_mpu;
	struct rtnl_ratespec	qt_rate;
	uint32_t		qt_rate_bucket;
	uint32_t		qt_rate_txtime;
	struct rtnl_ratespec	qt_peakrate;
	uint32_t		qt_peakrate_bucket;
	uint32_t		qt_peakrate_txtime;
	uint32_t		qt_mask;
};

struct rtnl_sfq
{
	uint32_t	qs_quantum;
	uint32_t	qs_perturb;
	uint32_t	qs_limit;
	uint32_t	qs_divisor;
	uint32_t	qs_flows;
	uint32_t	qs_mask;
};

struct rtnl_netem_corr
{
	uint32_t	nmc_delay;
	uint32_t	nmc_loss;
	uint32_t	nmc_duplicate;
};

struct rtnl_netem_reo
{
	uint32_t	nmro_probability;
	uint32_t	nmro_correlation;
};

struct rtnl_netem
{
	uint32_t		qnm_latency;
	uint32_t		qnm_limit;
	uint32_t		qnm_loss;
	uint32_t		qnm_gap;
	uint32_t		qnm_duplicate;
	uint32_t		qnm_jitter;
	uint32_t		qnm_mask;
	struct rtnl_netem_corr	qnm_corr;
	struct rtnl_netem_reo	qnm_ro;
};

struct rtnl_htb_qdisc
{
	uint32_t		qh_rate2quantum;
	uint32_t		qh_defcls;
	uint32_t		qh_mask;
};

struct rtnl_htb_class
{
	uint32_t		ch_prio;
	uint32_t		ch_mtu;
	struct rtnl_ratespec	ch_rate;
	struct rtnl_ratespec	ch_ceil;
	uint32_t		ch_rbuffer;
	uint32_t		ch_cbuffer;
	uint32_t		ch_mask;
};

struct rtnl_cbq
{
	struct tc_cbq_lssopt    cbq_lss;
	struct tc_ratespec      cbq_rate;
	struct tc_cbq_wrropt    cbq_wrr;
	struct tc_cbq_ovl       cbq_ovl;
	struct tc_cbq_fopt      cbq_fopt;
	struct tc_cbq_police    cbq_police;
};

struct rtnl_red
{
	uint32_t	qr_limit;
	uint32_t	qr_qth_min;
	uint32_t	qr_qth_max;
	uint8_t		qr_flags;
	uint8_t		qr_wlog;
	uint8_t		qr_plog;
	uint8_t		qr_scell_log;
	uint32_t	qr_mask;
};

struct flnl_request
{
	struct nl_addr *	lr_addr;
	uint32_t		lr_fwmark;
	uint8_t			lr_tos;
	uint8_t			lr_scope;
	uint8_t			lr_table;
	uint32_t		lr_mask;
	int			lr_refcnt;
};


struct flnl_result
{
	NLHDR_COMMON

	struct flnl_request *	fr_req;
	uint8_t			fr_table_id;
	uint8_t			fr_prefixlen;
	uint8_t			fr_nh_sel;
	uint8_t			fr_type;
	uint8_t			fr_scope;
	uint32_t		fr_error;
};

#endif
