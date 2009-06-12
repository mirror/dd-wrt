//==========================================================================
//
//      include/sys/param.h
//
//==========================================================================
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD, 
// FreeBSD or other sources, and are covered by the appropriate
// copyright disclaimers included herein.
//
// Portions created by Red Hat are
// Copyright (C) 2002 Red Hat, Inc. All Rights Reserved.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================

#ifndef _SYS_PARAM_H_
#define _SYS_PARAM_H_

#ifndef __ECOS
#define __ECOS
#endif

#ifdef _KERNEL
// Namespace munging - this should keep the "kernel" namespace
// polution to a minimum

// TEXT symbols
#define MD5Final cyg_MD5Final
#define MD5Init cyg_MD5Init
#define MD5Pad cyg_MD5Pad
#define MD5Transform cyg_MD5Transform
#define MD5Update cyg_MD5Update
#define _ip_mcast_src cyg__ip_mcast_src
#define _ip_mforward cyg__ip_mforward
#define _ip_mrouter_done cyg__ip_mrouter_done
#define _ip_mrouter_get cyg__ip_mrouter_get
#define _ip_mrouter_set cyg__ip_mrouter_set
#define _mrt_ioctl cyg__mrt_ioctl
#define addrsel_policy_init cyg_addrsel_policy_init
#define arc4random cyg_arc4random
#define random cyg_arc4random
#define arp_ifinit cyg_arp_ifinit
#define arpresolve cyg_arpresolve
#define asr_default__15Cyg_SchedThreadUi cyg_asr_default__15Cyg_SchedThreadUi
#define callout_active cyg_callout_active
#define callout_deactivate cyg_callout_deactivate
#define callout_init cyg_callout_init
#define callout_pending cyg_callout_pending
#define callout_reset cyg_callout_reset
#define callout_stop cyg_callout_stop
#define copyin cyg_copyin
#define copyout cyg_copyout
#define defrouter_addreq cyg_defrouter_addreq
#define defrouter_lookup cyg_defrouter_lookup
#define defrouter_reset cyg_defrouter_reset
#define defrouter_select cyg_defrouter_select
#define defrtrlist_del cyg_defrtrlist_del
#define dest6_input cyg_dest6_input
#define dup_sockaddr cyg_dup_sockaddr
#define encap4_input cyg_encap4_input
#define encap6_ctlinput cyg_encap6_ctlinput
#define encap6_input cyg_encap6_input
#define encap_init cyg_encap_init
#define ether_demux cyg_ether_demux
#define ether_ifattach cyg_ether_ifattach
#define ether_input cyg_ether_input
#define ether_ioctl cyg_ether_ioctl
#define ether_output cyg_ether_output
#define ether_output_frame cyg_ether_output_frame
#define frag6_drain cyg_frag6_drain
#define frag6_init cyg_frag6_init
#define frag6_input cyg_frag6_input
#define frag6_slowtimo cyg_frag6_slowtimo
#define getmicrotime cyg_getmicrotime
#define getmicrouptime cyg_getmicrouptime
#define hashinit cyg_hashinit
#define icmp6_ctloutput cyg_icmp6_ctloutput
#define icmp6_error cyg_icmp6_error
#define icmp6_fasttimo cyg_icmp6_fasttimo
#define icmp6_init cyg_icmp6_init
#define icmp6_input cyg_icmp6_input
#define icmp6_mtudisc_update cyg_icmp6_mtudisc_update
#define icmp6_redirect_input cyg_icmp6_redirect_input
#define icmp6_redirect_output cyg_icmp6_redirect_output
#define icmp6_reflect cyg_icmp6_reflect
#define icmp_error cyg_icmp_error
#define icmp_input cyg_icmp_input
#define if_addmulti cyg_if_addmulti
#define if_allmulti cyg_if_allmulti
#define if_attach cyg_if_attach
#define if_clone_create cyg_if_clone_create
#define if_clone_destroy cyg_if_clone_destroy
#define if_clone_list cyg_if_clone_list
#define if_clone_lookup cyg_if_clone_lookup
#define if_delmulti cyg_if_delmulti
#define if_down cyg_if_down
#define if_route cyg_if_route
#define if_setlladdr cyg_if_setlladdr
#define if_simloop cyg_if_simloop
#define if_unroute cyg_if_unroute
#define if_up cyg_if_up
#define ifa_ifwithaddr cyg_ifa_ifwithaddr
#define ifa_ifwithdstaddr cyg_ifa_ifwithdstaddr
#define ifa_ifwithnet cyg_ifa_ifwithnet
#define ifa_ifwithroute cyg_ifa_ifwithroute
#define ifafree cyg_ifafree
#define ifaof_ifpforaddr cyg_ifaof_ifpforaddr
#define ifioctl cyg_ifioctl
#define ifunit cyg_ifunit
#define igmp_fasttimo cyg_igmp_fasttimo
#define igmp_init cyg_igmp_init
#define igmp_input cyg_igmp_input
#define igmp_joingroup cyg_igmp_joingroup
#define igmp_leavegroup cyg_igmp_leavegroup
#define igmp_slowtimo cyg_igmp_slowtimo
#define in6_addmulti cyg_in6_addmulti
#define in6_addr2zoneid cyg_in6_addr2zoneid
#define in6_addrscope cyg_in6_addrscope
#define in6_are_prefix_equal cyg_in6_are_prefix_equal
#define in6_cksum cyg_in6_cksum
#define in6_clearscope cyg_in6_clearscope
#define in6_control cyg_in6_control
#define in6_delmulti cyg_in6_delmulti
#define in6_embedscope cyg_in6_embedscope
#define in6_get_tmpifid cyg_in6_get_tmpifid
#define in6_if_up cyg_in6_if_up
#define in6_ifattach cyg_in6_ifattach
#define in6_ifawithifp cyg_in6_ifawithifp
#define in6_init_prefix_ltimes cyg_in6_init_prefix_ltimes
#define in6_inithead cyg_in6_inithead
#define in6_joingroup cyg_in6_joingroup
#define in6_leavegroup cyg_in6_leavegroup
#define in6_localaddr cyg_in6_localaddr
#define in6_losing cyg_in6_losing
#define in6_mapped_peeraddr cyg_in6_mapped_peeraddr
#define in6_mapped_sockaddr cyg_in6_mapped_sockaddr
#define in6_mask2len cyg_in6_mask2len
#define in6_matchlen cyg_in6_matchlen
#define in6_nigroup cyg_in6_nigroup
#define in6_pcbbind cyg_in6_pcbbind
#define in6_pcbconnect cyg_in6_pcbconnect
#define in6_pcbdetach cyg_in6_pcbdetach
#define in6_pcbdisconnect cyg_in6_pcbdisconnect
#define in6_pcbladdr cyg_in6_pcbladdr
#define in6_pcblookup_hash cyg_in6_pcblookup_hash
#define in6_pcblookup_local cyg_in6_pcblookup_local
#define in6_pcbnotify cyg_in6_pcbnotify
#define in6_pcbsetport cyg_in6_pcbsetport
#define in6_prefixlen2mask cyg_in6_prefixlen2mask
#define in6_purgeaddr cyg_in6_purgeaddr
#define in6_recoverscope cyg_in6_recoverscope
#define in6_rtchange cyg_in6_rtchange
#define in6_selecthlim cyg_in6_selecthlim
#define in6_selectroute cyg_in6_selectroute
#define in6_selectsrc cyg_in6_selectsrc
#define in6_setmaxmtu cyg_in6_setmaxmtu
#define in6_setpeeraddr cyg_in6_setpeeraddr
#define in6_setsockaddr cyg_in6_setsockaddr
#define in6_sin6_2_sin cyg_in6_sin6_2_sin
#define in6_sin6_2_sin_in_sock cyg_in6_sin6_2_sin_in_sock
#define in6_sin_2_v4mapsin6 cyg_in6_sin_2_v4mapsin6
#define in6_sin_2_v4mapsin6_in_sock cyg_in6_sin_2_v4mapsin6_in_sock
#define in6_src_ioctl cyg_in6_src_ioctl
#define in6_tmpaddrtimer cyg_in6_tmpaddrtimer
#define in6_tmpifadd cyg_in6_tmpifadd
#define in6_update_ifa cyg_in6_update_ifa
#define in6if_do_dad cyg_in6if_do_dad
#define in6ifa_ifpforlinklocal cyg_in6ifa_ifpforlinklocal
#define in6ifa_ifpwithaddr cyg_in6ifa_ifpwithaddr
#define in_addmulti cyg_in_addmulti
#define in_addword cyg_in_addword
#define in_broadcast cyg_in_broadcast
#define in_canforward cyg_in_canforward
#define in_cksum cyg_in_cksum
#define in_cksum_hdr cyg_in_cksum_hdr
#define in_cksum_skip cyg_in_cksum_skip
#define in_cksumdata cyg_in_cksumdata
#define in_control cyg_in_control
#define in_delayed_cksum cyg_in_delayed_cksum
#define in_delmulti cyg_in_delmulti
#define in_ifadown cyg_in_ifadown
#define in_ifscrub cyg_in_ifscrub
#define in_inithead cyg_in_inithead
#define in_localaddr cyg_in_localaddr
#define in_losing cyg_in_losing
#define in_pcballoc cyg_in_pcballoc
#define in_pcbbind cyg_in_pcbbind
#define in_pcbconnect cyg_in_pcbconnect
#define in_pcbdetach cyg_in_pcbdetach
#define in_pcbdisconnect cyg_in_pcbdisconnect
#define in_pcbinshash cyg_in_pcbinshash
#define in_pcbladdr cyg_in_pcbladdr
#define in_pcblookup_hash cyg_in_pcblookup_hash
#define in_pcblookup_local cyg_in_pcblookup_local
#define in_pcbnotifyall cyg_in_pcbnotifyall
#define in_pcbpurgeif0 cyg_in_pcbpurgeif0
#define in_pcbrehash cyg_in_pcbrehash
#define in_pcbremlists cyg_in_pcbremlists
#define in_pseudo cyg_in_pseudo
#define in_rtchange cyg_in_rtchange
#define in_rtqdrain cyg_in_rtqdrain
#define in_setpeeraddr cyg_in_setpeeraddr
#define in_setsockaddr cyg_in_setsockaddr
#define init_ip6pktopts cyg_init_ip6pktopts
#define init_loopback_interface cyg_init_loopback_interface
#define init_net cyg_init_net
#define init_net_IPv6 cyg_init_net_IPv6
#define init_sin6 cyg_init_sin6
#define ip6_addaux cyg_ip6_addaux
#define ip6_clearpktopts cyg_ip6_clearpktopts
#define ip6_copypktopts cyg_ip6_copypktopts
#define ip6_ctloutput cyg_ip6_ctloutput
#define ip6_delaux cyg_ip6_delaux
#define ip6_findaux cyg_ip6_findaux
#define ip6_forward cyg_ip6_forward
#define ip6_freemoptions cyg_ip6_freemoptions
#define ip6_freepcbopts cyg_ip6_freepcbopts
#define ip6_get_prevhdr cyg_ip6_get_prevhdr
#define ip6_getdstifaddr cyg_ip6_getdstifaddr
#define ip6_init cyg_ip6_init
#define ip6_input cyg_ip6_input
#define ip6_lasthdr cyg_ip6_lasthdr
#define ip6_mforward cyg_ip6_mforward
#define ip6_mloopback cyg_ip6_mloopback
#define ip6_mrouter_done cyg_ip6_mrouter_done
#define ip6_mrouter_get cyg_ip6_mrouter_get
#define ip6_mrouter_set cyg_ip6_mrouter_set
#define ip6_nexthdr cyg_ip6_nexthdr
#define ip6_notify_pmtu cyg_ip6_notify_pmtu
#define ip6_optlen cyg_ip6_optlen
#define ip6_output cyg_ip6_output
#define ip6_process_hopopts cyg_ip6_process_hopopts
#define ip6_raw_ctloutput cyg_ip6_raw_ctloutput
#define ip6_reset_rcvopt cyg_ip6_reset_rcvopt
#define ip6_savecontrol cyg_ip6_savecontrol
#define ip6_setpktoptions cyg_ip6_setpktoptions
#define ip6_sprintf cyg_ip6_sprintf
#define ip6_unknown_opt cyg_ip6_unknown_opt
#define ip6_update_recvpcbopt cyg_ip6_update_recvpcbopt
#define ip6intr cyg_ip6intr
#define ip_ctloutput cyg_ip_ctloutput
#define ip_drain cyg_ip_drain
#define ip_freemoptions cyg_ip_freemoptions
#define ip_init cyg_ip_init
#define ip_input cyg_ip_input
#define ip_optcopy cyg_ip_optcopy
#define ip_output cyg_ip_output
#define ip_rsvp_done cyg_ip_rsvp_done
#define ip_rsvp_force_done cyg_ip_rsvp_force_done
#define ip_rsvp_init cyg_ip_rsvp_init
#define ip_rsvp_vif_done cyg_ip_rsvp_vif_done
#define ip_rsvp_vif_init cyg_ip_rsvp_vif_init
#define ip_savecontrol cyg_ip_savecontrol
#define ip_slowtimo cyg_ip_slowtimo
#define ip_srcroute cyg_ip_srcroute
#define ip_stripoptions cyg_ip_stripoptions
#define ipflow_create cyg_ipflow_create
#define ipflow_fastforward cyg_ipflow_fastforward
#define ipflow_slowtimo cyg_ipflow_slowtimo
#define iptime cyg_iptime
#define loioctl cyg_loioctl
#define looutput cyg_looutput
#define m_adj cyg_m_adj
#define m_aux_add cyg_m_aux_add
#define m_aux_add2 cyg_m_aux_add2
#define m_aux_delete cyg_m_aux_delete
#define m_aux_find cyg_m_aux_find
#define m_aux_find2 cyg_m_aux_find2
#define m_cat cyg_m_cat
#define m_clalloc cyg_m_clalloc
#define m_clalloc_wait cyg_m_clalloc_wait
#define m_copyback cyg_m_copyback
#define m_copydata cyg_m_copydata
#define m_copym cyg_m_copym
#define m_free cyg_m_free
#define m_freem cyg_m_freem
#define m_get cyg_m_get
#define m_getclr cyg_m_getclr
#define m_gethdr cyg_m_gethdr
#define m_mballoc cyg_m_mballoc
#define m_mballoc_wait cyg_m_mballoc_wait
#define m_prepend cyg_m_prepend
#define m_pulldown cyg_m_pulldown
#define m_pullup cyg_m_pullup
#define m_retry cyg_m_retry
#define m_retryhdr cyg_m_retryhdr
#define m_split cyg_m_split
#define microtime cyg_microtime
#define mld6_fasttimeo cyg_mld6_fasttimeo
#define mld6_init cyg_mld6_init
#define mld6_input cyg_mld6_input
#define mld6_start_listening cyg_mld6_start_listening
#define mld6_stop_listening cyg_mld6_stop_listening
#define mrt6_ioctl cyg_mrt6_ioctl
#define nd6_cache_lladdr cyg_nd6_cache_lladdr
#define nd6_dad_duplicated cyg_nd6_dad_duplicated
#define nd6_dad_start cyg_nd6_dad_start
#define nd6_dad_stop cyg_nd6_dad_stop
#define nd6_ifattach cyg_nd6_ifattach
#define nd6_ifptomac cyg_nd6_ifptomac
#define nd6_init cyg_nd6_init
#define nd6_ioctl cyg_nd6_ioctl
#define nd6_is_addr_neighbor cyg_nd6_is_addr_neighbor
#define nd6_lookup cyg_nd6_lookup
#define nd6_na_input cyg_nd6_na_input
#define nd6_na_output cyg_nd6_na_output
#define nd6_need_cache cyg_nd6_need_cache
#define nd6_ns_input cyg_nd6_ns_input
#define nd6_ns_output cyg_nd6_ns_output
#define nd6_nud_hint cyg_nd6_nud_hint
#define nd6_option cyg_nd6_option
#define nd6_option_init cyg_nd6_option_init
#define nd6_options cyg_nd6_options
#define nd6_output cyg_nd6_output
#define nd6_prefix_lookup cyg_nd6_prefix_lookup
#define nd6_prefix_offlink cyg_nd6_prefix_offlink
#define nd6_prefix_onlink cyg_nd6_prefix_onlink
#define nd6_prelist_add cyg_nd6_prelist_add
#define nd6_ra_input cyg_nd6_ra_input
#define nd6_rs_input cyg_nd6_rs_input
#define nd6_rtrequest cyg_nd6_rtrequest
#define nd6_setdefaultiface cyg_nd6_setdefaultiface
#define nd6_setmtu cyg_nd6_setmtu
#define nd6_storelladdr cyg_nd6_storelladdr
#define nd6_timer cyg_nd6_timer
#define net_add_domain cyg_net_add_domain
#define ovbcopy cyg_ovbcopy
#define pfctlinput cyg_pfctlinput
#define pfctlinput2 cyg_pfctlinput2
#define pffindproto cyg_pffindproto
#define pffindtype cyg_pffindtype
#define pfxlist_onlink_check cyg_pfxlist_onlink_check
#define pim6_input cyg_pim6_input
#define prelist_remove cyg_prelist_remove
#define prelist_update cyg_prelist_update
#define pru_accept_notsupp cyg_pru_accept_notsupp
#define pru_connect2_notsupp cyg_pru_connect2_notsupp
#define pru_control_notsupp cyg_pru_control_notsupp
#define pru_listen_notsupp cyg_pru_listen_notsupp
#define pru_rcvd_notsupp cyg_pru_rcvd_notsupp
#define pru_rcvoob_notsupp cyg_pru_rcvoob_notsupp
#define pru_sense_null cyg_pru_sense_null
#define raw_attach cyg_raw_attach
#define raw_ctlinput cyg_raw_ctlinput
#define raw_detach cyg_raw_detach
#define raw_disconnect cyg_raw_disconnect
#define raw_init cyg_raw_init
#define raw_input cyg_raw_input
#define register_netisr cyg_register_netisr
#define rip6_ctlinput cyg_rip6_ctlinput
#define rip6_ctloutput cyg_rip6_ctloutput
#define rip6_input cyg_rip6_input
#define rip6_output cyg_rip6_output
#define rip_ctlinput cyg_rip_ctlinput
#define rip_ctloutput cyg_rip_ctloutput
#define rip_init cyg_rip_init
#define rip_input cyg_rip_input
#define rip_output cyg_rip_output
#define rn_addmask cyg_rn_addmask
#define rn_addroute cyg_rn_addroute
#define rn_delete cyg_rn_delete
#define rn_init cyg_rn_init
#define rn_inithead cyg_rn_inithead
#define rn_lookup cyg_rn_lookup
#define rn_match cyg_rn_match
#define rn_refines cyg_rn_refines
#define route6_input cyg_route6_input
#define route_init cyg_route_init
//#define route_reinit cyg_route_reinit
#define rsvp_input cyg_rsvp_input
#define rt6_flush cyg_rt6_flush
#define rt_ifmsg cyg_rt_ifmsg
#define rt_missmsg cyg_rt_missmsg
#define rt_newaddrmsg cyg_rt_newaddrmsg
#define rt_newmaddrmsg cyg_rt_newmaddrmsg
#define rt_setgate cyg_rt_setgate
#define rtalloc cyg_rtalloc
#define rtalloc1 cyg_rtalloc1
#define rtalloc_ign cyg_rtalloc_ign
#define rtfree cyg_rtfree
#define rtinit cyg_rtinit
#define rtioctl cyg_rtioctl
#define rtredirect cyg_rtredirect
#define rtrequest cyg_rtrequest
#define sb_lock cyg_sb_lock
#define sbappend cyg_sbappend
#define sbappendaddr cyg_sbappendaddr
#define sbappendcontrol cyg_sbappendcontrol
#define sbappendrecord cyg_sbappendrecord
#define sbcompress cyg_sbcompress
#define sbcreatecontrol cyg_sbcreatecontrol
#define sbdrop cyg_sbdrop
#define sbdroprecord cyg_sbdroprecord
#define sbflush cyg_sbflush
#define sbrelease cyg_sbrelease
#define sbreserve cyg_sbreserve
#define sbwait cyg_sbwait
#define scope6_addr2default cyg_scope6_addr2default
#define scope6_check_id cyg_scope6_check_id
#define scope6_get cyg_scope6_get
#define scope6_get_default cyg_scope6_get_default
#define scope6_ifattach cyg_scope6_ifattach
#define scope6_set cyg_scope6_set
#define scope6_setdefault cyg_scope6_setdefault
#define scope6_setzoneid cyg_scope6_setzoneid
#define selwakeup cyg_selwakeup
#define soabort cyg_soabort
#define soaccept cyg_soaccept
#define soalloc cyg_soalloc
#define sobind cyg_sobind
#define socantrcvmore cyg_socantrcvmore
#define socantsendmore cyg_socantsendmore
#define soclose cyg_soclose
#define soconnect cyg_soconnect
#define socreate cyg_socreate
#define sodealloc cyg_sodealloc
#define sodisconnect cyg_sodisconnect
#define sodropablereq cyg_sodropablereq
#define sofree cyg_sofree
#define sogetopt cyg_sogetopt
#define sohasoutofband cyg_sohasoutofband
#define soisconnected cyg_soisconnected
#define soisconnecting cyg_soisconnecting
#define soisdisconnected cyg_soisdisconnected
#define soisdisconnecting cyg_soisdisconnecting
#define solisten cyg_solisten
#define sonewconn cyg_sonewconn
#define sonewconn3 cyg_sonewconn3
#define soopt_getm cyg_soopt_getm
#define soopt_mcopyin cyg_soopt_mcopyin
#define soopt_mcopyout cyg_soopt_mcopyout
#define sooptcopyin cyg_sooptcopyin
#define sooptcopyout cyg_sooptcopyout
#define sopoll cyg_sopoll
#define soreceive cyg_soreceive
#define soreserve cyg_soreserve
#define sorflush cyg_sorflush
#define sosend cyg_sosend
#define sosetopt cyg_sosetopt
#define soshutdown cyg_soshutdown
#define sowakeup cyg_sowakeup
#define tcp6_ctlinput cyg_tcp6_ctlinput
#define tcp6_input cyg_tcp6_input
#define tcp_canceltimers cyg_tcp_canceltimers
#define tcp_close cyg_tcp_close
#define tcp_ctlinput cyg_tcp_ctlinput
#define tcp_ctloutput cyg_tcp_ctloutput
#define tcp_drain cyg_tcp_drain
#define tcp_drop cyg_tcp_drop
#define tcp_drop_syn_sent cyg_tcp_drop_syn_sent
#define tcp_fillheaders cyg_tcp_fillheaders
#define tcp_gettaocache cyg_tcp_gettaocache
#define tcp_init cyg_tcp_init
#define tcp_input cyg_tcp_input
#define tcp_maketemplate cyg_tcp_maketemplate
#define tcp_mss cyg_tcp_mss
#define tcp_mssopt cyg_tcp_mssopt
#define tcp_mtudisc cyg_tcp_mtudisc
#define tcp_new_isn cyg_tcp_new_isn
#define tcp_newtcpcb cyg_tcp_newtcpcb
#define tcp_output cyg_tcp_output
#define tcp_quench cyg_tcp_quench
#define tcp_respond cyg_tcp_respond
#define tcp_rtlookup cyg_tcp_rtlookup
#define tcp_rtlookup6 cyg_tcp_rtlookup6
#define tcp_setpersist cyg_tcp_setpersist
#define tcp_slowtimo cyg_tcp_slowtimo
#define tcp_timer_2msl cyg_tcp_timer_2msl
#define tcp_timer_delack cyg_tcp_timer_delack
#define tcp_timer_keep cyg_tcp_timer_keep
#define tcp_timer_persist cyg_tcp_timer_persist
#define tcp_timer_rexmt cyg_tcp_timer_rexmt
#define timeout cyg_timeout
#define tvtohz cyg_tvtohz
#define udp6_ctlinput cyg_udp6_ctlinput
#define udp6_input cyg_udp6_input
#define udp6_output cyg_udp6_output
#define udp_ctlinput cyg_udp_ctlinput
#define udp_init cyg_udp_init
#define udp_input cyg_udp_input
#define udp_notify cyg_udp_notify
#define udp_shutdown cyg_udp_shutdown
#define uiomove cyg_uiomove

// DATA symbols
#define arpintrq cyg_arpintrq
#define bsd_nste cyg_bsd_nste
#define bsd_sock_fileops cyg_bsd_sock_fileops
#define bsd_sockops cyg_bsd_sockops
#define encaptab cyg_encaptab
#define etherbroadcastaddr cyg_etherbroadcastaddr
#define fr_checkp cyg_fr_checkp
#define fw_enable cyg_fw_enable
#define hostname cyg_hostname
#define hz cyg_hz
#define icmp6_ifstat cyg_icmp6_ifstat
#define icmp6_ifstatmax cyg_icmp6_ifstatmax
#define icmp6_nodeinfo cyg_icmp6_nodeinfo
#define icmp6_rediraccept cyg_icmp6_rediraccept
#define icmp6errppslim cyg_icmp6errppslim
#define if_cloners cyg_if_cloners
#define if_index cyg_if_index
#define ifindex2ifnet cyg_ifindex2ifnet
#define ifqmaxlen cyg_ifqmaxlen
#define in6_ifstat cyg_in6_ifstat
#define in6_ifstatmax cyg_in6_ifstatmax
#define in6_maxmtu cyg_in6_maxmtu
#define inet6ctlerrmap cyg_inet6ctlerrmap
#define inet6domain cyg_inet6domain
#define inet6sw cyg_inet6sw
#define inetctlerrmap cyg_inetctlerrmap
#define inetdomain cyg_inetdomain
#define inetsw cyg_inetsw
#define ip6_accept_rtadv cyg_ip6_accept_rtadv
#define ip6_auto_flowlabel cyg_ip6_auto_flowlabel
#define ip6_auto_linklocal cyg_ip6_auto_linklocal
#define ip6_dad_count cyg_ip6_dad_count
#define ip6_defhlim cyg_ip6_defhlim
#define ip6_defmcasthlim cyg_ip6_defmcasthlim
#define ip6_forwarding cyg_ip6_forwarding
#define ip6_fw_enable cyg_ip6_fw_enable
#define ip6_hdrnestlimit cyg_ip6_hdrnestlimit
#define ip6_id cyg_ip6_id
#define ip6_log_interval cyg_ip6_log_interval
#define ip6_log_time cyg_ip6_log_time
#define ip6_mrouter cyg_ip6_mrouter
#define ip6_mrouter_ver cyg_ip6_mrouter_ver
#define ip6_prefer_tempaddr cyg_ip6_prefer_tempaddr
#define ip6_temp_preferred_lifetime cyg_ip6_temp_preferred_lifetime
#define ip6_temp_regen_advance cyg_ip6_temp_regen_advance
#define ip6_temp_valid_lifetime cyg_ip6_temp_valid_lifetime
#define ip6_use_defzone cyg_ip6_use_defzone
#define ip6_use_deprecated cyg_ip6_use_deprecated
#define ip6_use_tempaddr cyg_ip6_use_tempaddr
#define ip6_v6only cyg_ip6_v6only
#define ip_defttl cyg_ip_defttl
#define ip_mcast_src cyg_ip_mcast_src
#define ip_mforward cyg_ip_mforward
#define ip_mrouter cyg_ip_mrouter
#define ip_mrouter_done cyg_ip_mrouter_done
#define ip_mrouter_get cyg_ip_mrouter_get
#define ip_mrouter_set cyg_ip_mrouter_set
#define ipforwarding cyg_ipforwarding
#define ipport_firstauto cyg_ipport_firstauto
#define ipport_hifirstauto cyg_ipport_hifirstauto
#define ipport_hilastauto cyg_ipport_hilastauto
#define ipport_lastauto cyg_ipport_lastauto
#define ipport_lowfirstauto cyg_ipport_lowfirstauto
#define ipport_lowlastauto cyg_ipport_lowlastauto
#define legal_vif_num cyg_legal_vif_num
#define llinfo_nd6 cyg_llinfo_nd6
#define log_in_vain cyg_log_in_vain
#define m_clalloc_wid cyg_m_clalloc_wid
#define m_mballoc_wid cyg_m_mballoc_wid
#define maxsockets cyg_maxsockets
#define mbuf_wait cyg_mbuf_wait
#define mrt_ioctl cyg_mrt_ioctl
#define nd6_debug cyg_nd6_debug
#define nd6_delay cyg_nd6_delay
#define nd6_gctimer cyg_nd6_gctimer
#define nd6_maxndopt cyg_nd6_maxndopt
#define nd6_maxnudhint cyg_nd6_maxnudhint
#define nd6_mmaxtries cyg_nd6_mmaxtries
#define nd6_prune cyg_nd6_prune
#define nd6_recalc_reachtm_interval cyg_nd6_recalc_reachtm_interval
#define nd6_umaxtries cyg_nd6_umaxtries
#define nd6_useloopback cyg_nd6_useloopback
#define nd_ifinfo cyg_nd_ifinfo
#define nd_prefix cyg_nd_prefix
#define nmbclusters cyg_nmbclusters
#define proc0 cyg_proc0
#define raw_usrreqs cyg_raw_usrreqs
#define rip6_usrreqs cyg_rip6_usrreqs
#define rip_recvspace cyg_rip_recvspace
#define rip_sendspace cyg_rip_sendspace
#define rip_usrreqs cyg_rip_usrreqs
#define rsvp_on cyg_rsvp_on
#define rsvpdebug cyg_rsvpdebug
#define rtc_resolution cyg_rtc_resolution
#define sb_max cyg_sb_max
#define scope6_ids cyg_scope6_ids
#define ss_fltsz cyg_ss_fltsz
#define ss_fltsz_local cyg_ss_fltsz_local
#define tcp6_usrreqs cyg_tcp6_usrreqs
#define tcp_backoff cyg_tcp_backoff
#define tcp_delack_enabled cyg_tcp_delack_enabled
#define tcp_do_newreno cyg_tcp_do_newreno
#define tcp_lq_overflow cyg_tcp_lq_overflow
#define tcp_mssdflt cyg_tcp_mssdflt
#define tcp_recvspace cyg_tcp_recvspace
#define tcp_sendspace cyg_tcp_sendspace
#define tcp_syn_backoff cyg_tcp_syn_backoff
#define tcp_usrreqs cyg_tcp_usrreqs
#define tcp_v6mssdflt cyg_tcp_v6mssdflt
#define tick cyg_tick
#define udp6_usrreqs cyg_udp6_usrreqs
#define udp_in6 cyg_udp_in6
#define udp_recvspace cyg_udp_recvspace
#define udp_sendspace cyg_udp_sendspace
#define udp_usrreqs cyg_udp_usrreqs
#define udpcksum cyg_udpcksum

// BSS symbols
#define addrsel_policytab cyg_addrsel_policytab
#define defaultaddrpolicy cyg_defaultaddrpolicy
#define domains cyg_domains
#define frag6_doing_reass cyg_frag6_doing_reass
#define frag6_nfragpackets cyg_frag6_nfragpackets
#define icmp6stat cyg_icmp6stat
#define icmpstat cyg_icmpstat
#define if_cloners_count cyg_if_cloners_count
#define ifnet cyg_ifnet
#define ifnet_addrs cyg_ifnet_addrs
#define in6_ifaddr cyg_in6_ifaddr
#define in6_multihead cyg_in6_multihead
#define in6_tmpaddrtimer_ch cyg_in6_tmpaddrtimer_ch
#define in_ifaddrhead cyg_in_ifaddrhead
#define in_multihead cyg_in_multihead
#define ip6_desync_factor cyg_ip6_desync_factor
#define ip6_flow_seq cyg_ip6_flow_seq
#define ip6_forward_rt cyg_ip6_forward_rt
#define ip6_forward_srcrt cyg_ip6_forward_srcrt
#define ip6_fw_chk_ptr cyg_ip6_fw_chk_ptr
#define ip6_fw_ctl_ptr cyg_ip6_fw_ctl_ptr
#define ip6_maxfragpackets cyg_ip6_maxfragpackets
#define ip6_ours_check_algorithm cyg_ip6_ours_check_algorithm
#define ip6_protox cyg_ip6_protox
#define ip6_sourcecheck cyg_ip6_sourcecheck
#define ip6_sourcecheck_interval cyg_ip6_sourcecheck_interval
#define ip6intrq cyg_ip6intrq
#define ip6q cyg_ip6q
#define ip6stat cyg_ip6stat
#define ip_fw_chk_ptr cyg_ip_fw_chk_ptr
#define ip_fw_ctl_ptr cyg_ip_fw_ctl_ptr
#define ip_fw_fwd_addr cyg_ip_fw_fwd_addr
#define ip_id cyg_ip_id
#define ip_protox cyg_ip_protox
#define ip_rsvpd cyg_ip_rsvpd
#define ipintrq cyg_ipintrq
#define ipstat cyg_ipstat
#define isn_ctx cyg_isn_ctx
#define isn_last_reseed cyg_isn_last_reseed
#define isn_secret cyg_isn_secret
#define ktime cyg_ktime
#define loif cyg_loif
#define max_datalen cyg_max_datalen
#define max_hdr cyg_max_hdr
#define max_linkhdr cyg_max_linkhdr
#define max_protohdr cyg_max_protohdr
#define mbstat cyg_mbstat
#define mbtypes cyg_mbtypes
#define mbutl cyg_mbutl
#define mclfree cyg_mclfree
#define mclrefcnt cyg_mclrefcnt
#define mf6ctable cyg_mf6ctable
#define mmbfree cyg_mmbfree
#define mono_time cyg_mono_time
#define mrt6stat cyg_mrt6stat
#define mtab_lock cyg_mtab_lock
#define multicast_register_if cyg_multicast_register_if
#define n6expire cyg_n6expire
#define nd6_defifindex cyg_nd6_defifindex
#define nd6_slowtimo_ch cyg_nd6_slowtimo_ch
#define nd6_timer_ch cyg_nd6_timer_ch
#define nd_defrouter cyg_nd_defrouter
#define netint_flags cyg_netint_flags
#define ng_ether_attach_p cyg_ng_ether_attach_p
#define ng_ether_detach_p cyg_ng_ether_detach_p
#define ng_ether_input_orphan_p cyg_ng_ether_input_orphan_p
#define ng_ether_input_p cyg_ng_ether_input_p
#define ng_ether_output_p cyg_ng_ether_output_p
#define nstab_lock cyg_nstab_lock
#define rawcb_list cyg_rawcb_list
#define rip6stat cyg_rip6stat
#define ripcb cyg_ripcb
#define ripcbinfo cyg_ripcbinfo
#define route_cb cyg_route_cb
#define rt_tables cyg_rt_tables
#define sig_pending cyg_sig_pending
#define signal_mutex cyg_signal_mutex
#define signal_sigwait cyg_signal_sigwait
#define so_gencnt cyg_so_gencnt
#define socket_zone cyg_socket_zone
#define tcb cyg_tcb
#define tcbinfo cyg_tcbinfo
#define tcp_ccgen cyg_tcp_ccgen
#define tcp_delacktime cyg_tcp_delacktime
#define tcp_keepidle cyg_tcp_keepidle
#define tcp_keepinit cyg_tcp_keepinit
#define tcp_keepintvl cyg_tcp_keepintvl
#define tcp_maxidle cyg_tcp_maxidle
#define tcp_maxpersistidle cyg_tcp_maxpersistidle
#define tcp_msl cyg_tcp_msl
#define tcpstat cyg_tcpstat
#define udb cyg_udb
#define udbinfo cyg_udbinfo
#define udp_ip6 cyg_udp_ip6
#define udpstat cyg_udpstat
#endif

// Symbols from the BSD_CRYPTO package

// TEXT symbols

#define SHA256_Data cyg_SHA256_Data
#define SHA256_End cyg_SHA256_End
#define SHA256_Final cyg_SHA256_Final
#define SHA256_Init cyg_SHA256_Init
#define SHA256_Transform cyg_SHA256_Transform
#define SHA256_Update cyg_SHA256_Update
#define SHA384_Data cyg_SHA384_Data
#define SHA384_End cyg_SHA384_End
#define SHA384_Final cyg_SHA384_Final
#define SHA384_Init cyg_SHA384_Init
#define SHA384_Update cyg_SHA384_Update
#define SHA512_Data cyg_SHA512_Data
#define SHA512_End cyg_SHA512_End
#define SHA512_Final cyg_SHA512_Final
#define SHA512_Init cyg_SHA512_Init
#define SHA512_Last cyg_SHA512_Last
#define SHA512_Transform cyg_SHA512_Transform
#define SHA512_Update cyg_SHA512_Update
#define md5_init cyg_md5_init
#define md5_loop cyg_md5_loop
#define md5_pad cyg_md5_pad
#define md5_result cyg_md5_result
#define sha1_init cyg_sha1_init
#define sha1_loop cyg_sha1_loop
#define sha1_pad cyg_sha1_pad
#define sha1_result cyg_sha1_result
#define cast128_decrypt_round12 cyg_cast128_decrypt_round12
#define cast128_decrypt_round16 cyg_cast128_decrypt_round16
#define cast128_encrypt_round12 cyg_cast128_encrypt_round12
#define cast128_encrypt_round16 cyg_cast128_encrypt_round16
#define set_cast128_subkey cyg_set_cast128_subkey
#define twofish_TableOp cyg_twofish_TableOp
#define twofish_blockDecrypt cyg_twofish_blockDecrypt
#define twofish_blockEncrypt cyg_twofish_blockEncrypt
#define twofish_cipherInit cyg_twofish_cipherInit
#define twofish_makeKey cyg_twofish_makeKey
#define twofish_reKey cyg_twofish_reKey
#define rijndaelDecrypt cyg_rijndaelDecrypt
#define rijndaelEncrypt cyg_rijndaelEncrypt
#define rijndaelKeyEncToDec cyg_rijndaelKeyEncToDec
#define rijndaelKeySched cyg_rijndaelKeySched
#define rijndael_blockDecrypt cyg_rijndael_blockDecrypt
#define rijndael_blockEncrypt cyg_rijndael_blockEncrypt
#define rijndael_cipherInit cyg_rijndael_cipherInit
#define rijndael_makeKey cyg_rijndael_makeKey
#define rijndael_padDecrypt cyg_rijndael_padDecrypt
#define rijndael_padEncrypt cyg_rijndael_padEncrypt

// Data symbols

#define numRounds cyg_numRounds

// BSS symbols


#include <pkgconf/net.h>
#include <cyg/infra/cyg_type.h>  // Standard eCos types

#define __P(protos) protos
#if defined(__cplusplus)
#define __BEGIN_DECLS   extern "C" {
#define __END_DECLS     };
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif

#include <sys/types.h>
#include <sys/endian.h>
#include <errno.h>

#ifdef _KERNEL
// External [common] variables
extern int hz;
extern volatile struct timeval ktime;
#define time_second ktime.tv_sec
extern int tick;
extern int proc0;
#define curproc 0
extern void microtime(struct timeval *tp);
extern void getmicrotime(struct timeval *tp);
extern void getmicrouptime(struct timeval *tp);
extern int  tvtohz(struct timeval *tv);
extern int  arc4random(void);

// Function mappings

extern int  cyg_ticks(void);
#define ticks cyg_ticks()
extern int  cyg_tsleep(void *, int, char *, int);
extern void cyg_wakeup(void *);
#define tsleep(e,p,w,t) cyg_tsleep(e,0,w,t)
#define wakeup(e)       cyg_wakeup(e)
#define wakeup_one(e)   cyg_wakeup(e)

#ifdef CYGIMPL_TRACE_SPLX   
extern cyg_uint32  cyg_splimp(const char *file, const int line);
extern cyg_uint32  cyg_splnet(const char *file, const int line);
extern cyg_uint32  cyg_splclock(const char *file, const int line);
extern cyg_uint32  cyg_splsoftnet(const char *file, const int line);
extern void        cyg_splx(cyg_uint32, const char *file, const int line);
#define splimp()   cyg_splimp(__FUNCTION__, __LINE__)
#define splnet()   cyg_splnet(__FUNCTION__, __LINE__)
#define splclock() cyg_splclock(__FUNCTION__, __LINE__)
#define splsoftnet() cyg_splsoftnet(__FUNCTION__, __LINE__)
#define splx(x)    cyg_splx(x, __FUNCTION__, __LINE__)
#define cyg_scheduler_lock() _cyg_scheduler_lock(__FUNCTION__, __LINE__)
#define cyg_scheduler_safe_lock() _cyg_scheduler_safe_lock(__FUNCTION__, __LINE__)
#define cyg_scheduler_unlock() _cyg_scheduler_unlock(__FUNCTION__, __LINE__)
#else
extern cyg_uint32  cyg_splimp(void);
extern cyg_uint32  cyg_splnet(void);
extern cyg_uint32  cyg_splclock(void);
extern cyg_uint32  cyg_splsoftnet(void);
extern cyg_uint32  cyg_splhigh(void);
extern void        cyg_splx(cyg_uint32);
#define splimp     cyg_splimp
#define splnet     cyg_splnet
#define splclock   cyg_splclock
#define splsoftnet cyg_splsoftnet
#define splhigh    cyg_splhigh
#define splx       cyg_splx
#endif

extern void cyg_panic(const char *msg, ...);
#define panic cyg_panic

/*
 * Constants related to network buffer management.
 * MCLBYTES must be no larger than CLBYTES (the software page size), and,
 * on machines that exchange pages of input or output buffers with mbuf
 * clusters (MAPPED_MBUFS), MCLBYTES must also be an integral multiple
 * of the hardware page size.
 */
#define MSIZE           128             /* size of an mbuf */
#define MCLSHIFT        11              /* convert bytes to m_buf clusters */
#define MCLBYTES        (1 << MCLSHIFT) /* size of a m_buf cluster */
#define MCLOFSET        (MCLBYTES - 1)  /* offset within a m_buf cluster */
#define CLBYTES         4096            /* size of actual cluster */
#define PAGE_SIZE       CLBYTES

/*
 * Round p (pointer or byte index) up to a correctly-aligned value
 * for all data types (int, long, ...).   The result is u_int and
 * must be cast to any desired pointer type.
 */
#define ALIGNBYTES      (sizeof(int) - 1)
#define ALIGN(p)        (((u_int)(p) + ALIGNBYTES) &~ ALIGNBYTES)

// These symbols are used in the IPV6 stuff
// (be more defensive about external setup)
#undef __linux__   
#undef __bsdi__    
#undef __FreeBSD__ 
#undef __OpenBSD__ 
#undef __NetBSD__  

#define __FreeBSD__       4
#define __FreeBSD_version 440000

// TEMP

#include <cyg/infra/diag.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/kernel/kapi.h>
#include <cyg/io/file.h>

struct net_stats {
    int              count;
    cyg_uint32       min_time, max_time, total_time;
};

#ifdef CYGDBG_NET_TIMING_STATS
#define START_STATS()                                   \
    cyg_uint32 start_time, end_time, elapsed_time;      \
    HAL_CLOCK_READ(&start_time);
#define FINISH_STATS(stats)                                                             \
    HAL_CLOCK_READ(&end_time);                                                          \
    if (end_time < start_time) {                                                        \
        elapsed_time = (end_time+CYGNUM_KERNEL_COUNTERS_RTC_PERIOD) - start_time;       \
    } else {                                                                            \
        elapsed_time = end_time - start_time;                                           \
    }                                                                                   \
    if (stats.min_time == 0) {                                                          \
        stats.min_time = 0x7FFFFFFF;                                                    \
    }                                                                                   \
    if (elapsed_time < stats.min_time)                                                  \
        stats.min_time = elapsed_time;                                                  \
    if (elapsed_time > stats.max_time)                                                  \
        stats.max_time = elapsed_time;                                                  \
    stats.total_time += elapsed_time;                                                   \
    stats.count++;
#else
#define START_STATS() 
#define FINISH_STATS(X)
#endif

// TEMP

// timeout support
typedef void (timeout_fun)(void *);
typedef struct callout {
    struct callout *next, *prev;
    cyg_int32     delta;  // Number of "ticks" in the future for this timeout
    timeout_fun  *fun;    // Function to execute when it expires
    void         *arg;    // Argument to pass when it does
    int           flags;  // Info about this item
} timeout_entry;
#define CALLOUT_LOCAL    0x0001
#define CALLOUT_ACTIVE   0x0002
#define CALLOUT_PENDING  0x0004
extern cyg_uint32 timeout(timeout_fun *fun, void *arg, cyg_int32 delta);
extern void untimeout(timeout_fun *fun, void *arg);

extern void callout_init(struct callout *);
extern void callout_reset(struct callout *, int, timeout_fun *, void *);
extern void callout_stop(struct callout *);
extern int  callout_active(struct callout *);
extern void callout_deactivate(struct callout *);
extern int  callout_pending(struct callout *);

extern int uiomove(caddr_t cp, int n, struct uio *uio);
extern int copyout(const void *s, void *d, size_t len);
extern int copyin(const void *s, void *d, size_t len);
extern void ovbcopy(const void *s, void *d, size_t len);
extern void get_mono_time(void);
extern int arc4random(void);
extern void get_random_bytes(void *buf, size_t len);
extern void read_random(void *buf, size_t len);
extern int read_random_unlimited(void *buf, size_t len);
extern void *hashinit(int elements, void *type, u_long *hashmask);

// Initialization support - cross between RedBoot & FreeBSD
typedef void _init_fun(void *);
typedef _init_fun *_init_fun_ptr;
struct init_tab_entry {
    _init_fun_ptr fun;
    void *data;
    char *name;
} CYG_HAL_TABLE_TYPE;
#define _cat(a,b) a##b
#define __Net_init(_p1_,_p2_,_f_,_d_)                                    \
struct init_tab_entry _cat(_net_init_tab,_p1_##_p2_##_f_) \
  CYG_HAL_TABLE_QUALIFIED_ENTRY(_Net_inits,_p1_##_p2_##_f_) = { (_init_fun_ptr) _f_, _d_, #_f_ }; 
#define _Net_init(_p1_,_p2_,_f_,_d_)                                    \
        __Net_init(_p1_,_p2_,_f_,_d_)                                    

#define SI_ORDER_FIRST  000000
#define SI_ORDER_SECOND 000001
#define SI_ORDER_THIRD  000002
#define SI_ORDER_MIDDLE 080000
#define SI_ORDER_ANY    FFFFFF

#define SI_SUB_MBUF         0x50
#define SI_SUB_DEVICES      0x60
#define SI_SUB_PSEUDO       0x70
#define SI_SUB_PROTO_IF     0x84
#define SI_SUB_PROTO_DOMAIN 0x88

#define SYSINIT(label, group, order, fun, ident) \
  _Net_init(_cat(group,order), label, fun, ident)
#define PSEUDO_SET(fun, sys) \
  SYSINIT(sys, SI_SUB_PSEUDO, SI_ORDER_FIRST, fun, NULL)

// VM zone stuff
#define ZONE_INTERRUPT  1

typedef struct _elem {
    struct _elem *next;
} elem;

typedef struct vm_zone {
    char *name;
    int   elem_size, free, total;
    int   alloc_tries, alloc_fails, alloc_frees;
    elem *pool;
    struct vm_zone *next;
} *vm_zone_t;
vm_zone_t zinit(char *name, int size, int nentries, int flags, int zalloc);
void *    zalloci(vm_zone_t z);
void      zfreei(vm_zone_t z, void *item);

// Function mapping
#define printf     diag_printf
#define sprintf    diag_sprintf
#define snprintf   diag_snprintf

// Missing standard functions
static __inline int imax(int a, int b) { return (a > b ? a : b); }
static __inline int imin(int a, int b) { return (a < b ? a : b); }
static __inline long lmax(long a, long b) { return (a > b ? a : b); }
static __inline long lmin(long a, long b) { return (a < b ? a : b); }
static __inline u_int max(u_int a, u_int b) { return (a > b ? a : b); }
static __inline u_int min(u_int a, u_int b) { return (a < b ? a : b); }
static __inline quad_t qmax(quad_t a, quad_t b) { return (a > b ? a : b); }
static __inline quad_t qmin(quad_t a, quad_t b) { return (a < b ? a : b); }
static __inline u_long ulmax(u_long a, u_long b) { return (a > b ? a : b); }
static __inline u_long ulmin(u_long a, u_long b) { return (a < b ? a : b); }

// Align a value to a natural multiple (4)
#define _ALIGN(n) (((n)+3)&~3)  // Is this right?
// Round a number 'n' up to a multiple of 'm'
#define roundup(n,m) ((((n)+((m)-1))/(m))*(m))

#define NEW_STRUCT_ROUTE
extern char *hostname;

// Logging facilities
#ifdef CYGPKG_NET_FREEBSD_LOGGING
extern int cyg_net_log_mask;
#define LOG_ERR      0x0001
#define LOG_WARNING  0x0002
#define LOG_NOTICE   0x0004
#define LOG_INFO     0x0008
#define LOG_DEBUG    0x0010
#define LOG_MDEBUG   0x0020
#define LOG_IOCTL    0x0040
#define LOG_ADDR     0x0100
#define LOG_FAIL     0x0200
#define LOG_INIT     0x0080
#define LOG_EMERG    0x4000
#define LOG_CRIT     0x8000
#define log(lvl, args...) \
do { if (cyg_net_log_mask & lvl) diag_printf(args); } while (0)
#define log_dump(lvl, buf, len) \
do { if (cyg_net_log_mask & lvl) diag_dump_buf(buf, len); } while (0)
#define log_(lvl) \
if (cyg_net_log_mask & lvl)
#else
#define log(lvl, args...) 
#define log_dump(lvl, buf, len)
#define log_(lvl)
#endif

#endif // _KERNEL
int extern cyg_arc4random(void);
#endif //_SYS_PARAM_H_
