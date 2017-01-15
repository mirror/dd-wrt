/*
 *	BIRD -- Linux Netlink Configuration
 *
 *	(c) 1999--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

CF_HDR

CF_DECLS

CF_KEYWORDS(KERNEL, TABLE, METRIC, KRT_PREFSRC, KRT_REALM, KRT_SCOPE, KRT_MTU, KRT_WINDOW,
	    KRT_RTT, KRT_RTTVAR, KRT_SSTRESH, KRT_CWND, KRT_ADVMSS, KRT_REORDERING,
	    KRT_HOPLIMIT, KRT_INITCWND, KRT_RTO_MIN, KRT_INITRWND, KRT_QUICKACK,
	    KRT_LOCK_MTU, KRT_LOCK_WINDOW, KRT_LOCK_RTT, KRT_LOCK_RTTVAR,
	    KRT_LOCK_SSTRESH, KRT_LOCK_CWND, KRT_LOCK_ADVMSS, KRT_LOCK_REORDERING,
	    KRT_LOCK_HOPLIMIT, KRT_LOCK_RTO_MIN, KRT_FEATURE_ECN, KRT_FEATURE_ALLFRAG)

CF_GRAMMAR

CF_ADDTO(kern_proto, kern_proto kern_sys_item ';')

kern_sys_item:
   KERNEL TABLE expr { THIS_KRT->sys.table_id = $3; }
 | METRIC expr { THIS_KRT->sys.metric = $2; }
 ;

CF_ADDTO(dynamic_attr, KRT_PREFSRC	{ $$ = f_new_dynamic_attr(EAF_TYPE_IP_ADDRESS, T_IP, EA_KRT_PREFSRC); })
CF_ADDTO(dynamic_attr, KRT_REALM	{ $$ = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_REALM); })
CF_ADDTO(dynamic_attr, KRT_SCOPE	{ $$ = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_SCOPE); })

CF_ADDTO(dynamic_attr, KRT_MTU		{ $$ = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_MTU); })
CF_ADDTO(dynamic_attr, KRT_WINDOW	{ $$ = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_WINDOW); })
CF_ADDTO(dynamic_attr, KRT_RTT		{ $$ = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_RTT); })
CF_ADDTO(dynamic_attr, KRT_RTTVAR	{ $$ = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_RTTVAR); })
CF_ADDTO(dynamic_attr, KRT_SSTRESH	{ $$ = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_SSTRESH); })
CF_ADDTO(dynamic_attr, KRT_CWND		{ $$ = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_CWND); })
CF_ADDTO(dynamic_attr, KRT_ADVMSS	{ $$ = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_ADVMSS); })
CF_ADDTO(dynamic_attr, KRT_REORDERING	{ $$ = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_REORDERING); })
CF_ADDTO(dynamic_attr, KRT_HOPLIMIT	{ $$ = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_HOPLIMIT); })
CF_ADDTO(dynamic_attr, KRT_INITCWND	{ $$ = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_INITCWND); })
CF_ADDTO(dynamic_attr, KRT_RTO_MIN	{ $$ = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_RTO_MIN); })
CF_ADDTO(dynamic_attr, KRT_INITRWND	{ $$ = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_INITRWND); })
CF_ADDTO(dynamic_attr, KRT_QUICKACK	{ $$ = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_QUICKACK); })

CF_ADDTO(dynamic_attr, KRT_LOCK_MTU	{ $$ = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_LOCK_MTU); })
CF_ADDTO(dynamic_attr, KRT_LOCK_WINDOW	{ $$ = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_LOCK_WINDOW); })
CF_ADDTO(dynamic_attr, KRT_LOCK_RTT	{ $$ = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_LOCK_RTT); })
CF_ADDTO(dynamic_attr, KRT_LOCK_RTTVAR	{ $$ = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_LOCK_RTTVAR); })
CF_ADDTO(dynamic_attr, KRT_LOCK_SSTRESH	{ $$ = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_LOCK_SSTHRESH); })
CF_ADDTO(dynamic_attr, KRT_LOCK_CWND	{ $$ = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_LOCK_CWND); })
CF_ADDTO(dynamic_attr, KRT_LOCK_ADVMSS	{ $$ = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_LOCK_ADVMSS); })
CF_ADDTO(dynamic_attr, KRT_LOCK_REORDERING { $$ = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_LOCK_REORDERING); })
CF_ADDTO(dynamic_attr, KRT_LOCK_HOPLIMIT { $$ = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_LOCK_HOPLIMIT); })
CF_ADDTO(dynamic_attr, KRT_LOCK_RTO_MIN { $$ = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_LOCK_RTO_MIN); })

CF_ADDTO(dynamic_attr, KRT_FEATURE_ECN	{ $$ = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_FEATURE_ECN); })
CF_ADDTO(dynamic_attr, KRT_FEATURE_ALLFRAG { $$ = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_FEATURE_ALLFRAG); })


CF_CODE

CF_END
