#ifndef CYGONCE_PPP_NAMES_H
#define CYGONCE_PPP_NAMES_H
// ====================================================================
//
//      names.h
//
//      PPP name remapping
//
// ====================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 eCosCentric Ltd.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting the
// copyright holder.
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
// ====================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):           nickg
// Contributors:        nickg
// Date:                2003-06-01
// Purpose:             Name Remapping
// Description:         This header contains redefinitions of all the
//                      external names defined in the PPP package. This 
//                      avoids polluting the namespace with name that
//                      might clash with application symbols.
//
//####DESCRIPTIONEND####
//
// ====================================================================

// net/ppp/current/src/auth.c:
#define auth_check_options cyg_ppp_auth_check_options
#define auth_ip_addr cyg_ppp_auth_ip_addr
#define auth_peer_fail cyg_ppp_auth_peer_fail
#define auth_peer_success cyg_ppp_auth_peer_success
#define auth_reset cyg_ppp_auth_reset
#define auth_withpeer_fail cyg_ppp_auth_withpeer_fail
#define auth_withpeer_success cyg_ppp_auth_withpeer_success
#define bad_ip_adrs cyg_ppp_bad_ip_adrs
#define check_access cyg_ppp_check_access
#define check_passwd cyg_ppp_check_passwd
#define get_secret cyg_ppp_get_secret
#define link_down cyg_ppp_link_down
#define link_established cyg_ppp_link_established
#define link_required cyg_ppp_link_required
#define link_terminated cyg_ppp_link_terminated
#define np_down cyg_ppp_np_down
#define np_finished cyg_ppp_np_finished
#define np_up cyg_ppp_np_up
#define peer_authname cyg_ppp_peer_authname

// net/ppp/current/src/bsd_comp.c:
#define ppp_bsd_compress cyg_ppp_ppp_bsd_compress

// net/ppp/current/src/cbcp.c:
#define cbcp cyg_ppp_cbcp
#define cbcp_codenames cyg_ppp_cbcp_codenames
#define cbcp_optionnames cyg_ppp_cbcp_optionnames
#define cbcp_protent cyg_ppp_cbcp_protent

// net/ppp/current/src/ccp.c:
#define ccp_allowoptions cyg_ppp_ccp_allowoptions
#define ccp_fsm cyg_ppp_ccp_fsm
#define ccp_gotoptions cyg_ppp_ccp_gotoptions
#define ccp_hisoptions cyg_ppp_ccp_hisoptions
#define ccp_protent cyg_ppp_ccp_protent
#define ccp_wantoptions cyg_ppp_ccp_wantoptions

// net/ppp/current/src/chap.c:
#define ChapAuthPeer cyg_ppp_ChapAuthPeer
#define ChapAuthWithPeer cyg_ppp_ChapAuthWithPeer
#define chap cyg_ppp_chap
#define chap_protent cyg_ppp_chap_protent

// net/ppp/current/src/fsm.c:
#define fsm_close cyg_ppp_fsm_close
#define fsm_init cyg_ppp_fsm_init
#define fsm_input cyg_ppp_fsm_input
#define fsm_lowerdown cyg_ppp_fsm_lowerdown
#define fsm_lowerup cyg_ppp_fsm_lowerup
#define fsm_open cyg_ppp_fsm_open
#define fsm_protreject cyg_ppp_fsm_protreject
#define fsm_sdata cyg_ppp_fsm_sdata
#define peer_mru cyg_ppp_peer_mru

// net/ppp/current/src/if_ppp.c:
#define ppp_dequeue cyg_ppp_ppp_dequeue
#define ppp_restart cyg_ppp_ppp_restart
#define ppp_softc cyg_ppp_ppp_softc
#define pppalloc cyg_ppp_pppalloc
#define pppattach cyg_ppp_pppattach
#define pppdealloc cyg_ppp_pppdealloc
#define pppioctl cyg_ppp_pppioctl
#define pppoutput cyg_ppp_pppoutput
#define ppppktin cyg_ppp_ppppktin

// net/ppp/current/src/ipcp.c:
#define ip_ntoa cyg_ppp_ip_ntoa
#define ipcp_allowoptions cyg_ppp_ipcp_allowoptions
#define ipcp_fsm cyg_ppp_ipcp_fsm
#define ipcp_gotoptions cyg_ppp_ipcp_gotoptions
#define ipcp_hisoptions cyg_ppp_ipcp_hisoptions
#define ipcp_protent cyg_ppp_ipcp_protent
#define ipcp_wantoptions cyg_ppp_ipcp_wantoptions

// net/ppp/current/src/lcp.c:
#define lcp_allowoptions cyg_ppp_lcp_allowoptions
#define lcp_close cyg_ppp_lcp_close
#define lcp_fsm cyg_ppp_lcp_fsm
#define lcp_gotoptions cyg_ppp_lcp_gotoptions
#define lcp_hisoptions cyg_ppp_lcp_hisoptions
#define lcp_loopbackfail cyg_ppp_lcp_loopbackfail
#define lcp_lowerdown cyg_ppp_lcp_lowerdown
#define lcp_lowerup cyg_ppp_lcp_lowerup
#define lcp_open cyg_ppp_lcp_open
#define lcp_protent cyg_ppp_lcp_protent
#define lcp_sprotrej cyg_ppp_lcp_sprotrej
#define lcp_wantoptions cyg_ppp_lcp_wantoptions
#define xmit_accm cyg_ppp_xmit_accm

// net/ppp/current/src/net_ppp_magic.o:
#define magic cyg_ppp_magic
#define magic_init cyg_ppp_magic_init

// net/ppp/current/src/ppp_deflate.c:
#define ppp_deflate cyg_ppp_ppp_deflate
#define ppp_deflate_draft cyg_ppp_ppp_deflate_draft

// net/ppp/current/src/pppd.c:
#define auth_required cyg_ppp_auth_required
#define baud_rate cyg_ppp_baud_rate
#define connector cyg_ppp_connector
#define flowctl cyg_ppp_flowctl
#define cryptpap cyg_ppp_cryptpap
#define debug cyg_ppp_debug
#define default_device cyg_ppp_default_device
#define devnam cyg_ppp_devnam
#define die cyg_ppp_die
#define disable_defaultip cyg_ppp_disable_defaultip
#define disconnector cyg_ppp_disconnector
#define etime cyg_ppp_etime
#define explicit_remote cyg_ppp_explicit_remote
#define fmtmsg cyg_ppp_fmtmsg
#define format_packet cyg_ppp_format_packet
#define holdoff cyg_ppp_holdoff
#define hungup cyg_ppp_hungup
#define idle_time_limit cyg_ppp_idle_time_limit
#define ifname cyg_ppp_ifname
//#define ifunit cyg_ppp_ifunit
#define inpacket_buf cyg_ppp_inpacket_buf
#define inspeed cyg_ppp_inspeed
#define ipparam cyg_ppp_ipparam
#define kdebugflag cyg_ppp_kdebugflag
#define kill_link cyg_ppp_kill_link
#define lcp_echo_fails cyg_ppp_lcp_echo_fails
#define lcp_echo_interval cyg_ppp_lcp_echo_interval
#define line cyg_ppp_line
#define linep cyg_ppp_linep
#define lockflag cyg_ppp_lockflag
#define log_packet cyg_ppp_log_packet
#define max_con_attempts cyg_ppp_max_con_attempts
#define maxconnect cyg_ppp_maxconnect
#define minutes cyg_ppp_minutes
#define modem cyg_ppp_modem
#define need_holdoff cyg_ppp_need_holdoff
#define netmask cyg_ppp_netmask
#define no_ppp_msg cyg_ppp_no_ppp_msg
#define novm cyg_ppp_novm
#define open_ccp_flag cyg_ppp_open_ccp_flag
#define our_name cyg_ppp_our_name
#define outpacket_buf cyg_ppp_outpacket_buf
#define passwd cyg_ppp_passwd
#define persist cyg_ppp_persist
#define phase cyg_ppp_phase
#define print_string cyg_ppp_print_string
#define privileged cyg_ppp_privileged
#define progname cyg_ppp_progname
#define protocols cyg_ppp_protocols
#define proxyarp cyg_ppp_proxyarp
#define quit cyg_ppp_quit
#define refuse_chap cyg_ppp_refuse_chap
#define refuse_pap cyg_ppp_refuse_pap
#define remote_name cyg_ppp_remote_name
#define s_env_nalloc cyg_ppp_s_env_nalloc
#define script cyg_ppp_script
#define script_env cyg_ppp_script_env
#define stime cyg_ppp_stime
#define tty_handle cyg_ppp_tty_handle
#define tty_mode cyg_ppp_tty_mode
#define usehostname cyg_ppp_usehostname
#define uselogin cyg_ppp_uselogin
#define user cyg_ppp_user
#define vfmtmsg cyg_ppp_vfmtmsg
#define welcomer cyg_ppp_welcomer

// net/ppp/current/src/slcompress.c:
#define sl_compress_init cyg_ppp_sl_compress_init
#define sl_compress_tcp cyg_ppp_sl_compress_tcp
#define sl_uncompress_tcp cyg_ppp_sl_uncompress_tcp
#define sl_uncompress_tcp_core cyg_ppp_sl_uncompress_tcp_core

// net/ppp/current/src/sys-ecos.c:
#define GetMask cyg_ppp_GetMask
#define MD5Final cyg_MD5Final
#define MD5Init cyg_MD5Init
#define MD5Update cyg_MD5Update
#define ccp_fatal_error cyg_ppp_ccp_fatal_error
#define ccp_flags_set cyg_ppp_ccp_flags_set
#define ccp_test cyg_ppp_ccp_test
#define cifaddr cyg_ppp_cifaddr
#define cifdefaultroute cyg_ppp_cifdefaultroute
#define cifproxyarp cyg_ppp_cifproxyarp
#define clean_check cyg_ppp_clean_check
#define crypt cyg_ppp_crypt
#define disestablish_ppp cyg_ppp_disestablish_ppp
#define drand48 cyg_ppp_drand48
#define establish_ppp cyg_ppp_establish_ppp
#define get_host_seed cyg_ppp_get_host_seed
#define get_idle_time cyg_ppp_get_idle_time
#define gettimeofday cyg_ppp_gettimeofday
#define mrand48 cyg_ppp_mrand48
#define netmask cyg_ppp_netmask
#define output cyg_ppp_output
#define ppp_available cyg_ppp_ppp_available
#define ppp_recv_config cyg_ppp_ppp_recv_config
#define ppp_send_config cyg_ppp_ppp_send_config
#define ppp_set_xaccm cyg_ppp_ppp_set_xaccm
#define ppp_tty cyg_ppp_ppp_tty
#define read_packet cyg_ppp_read_packet
#define restore_loop cyg_ppp_restore_loop
#define restore_tty cyg_ppp_restore_tty
#define set_up_tty cyg_ppp_set_up_tty
#define setdtr cyg_ppp_setdtr
#define sifaddr cyg_ppp_sifaddr
#define sifdefaultroute cyg_ppp_sifdefaultroute
#define sifdown cyg_ppp_sifdown
#define sifnpmode cyg_ppp_sifnpmode
#define sifproxyarp cyg_ppp_sifproxyarp
#define sifup cyg_ppp_sifup
#define sifvjcomp cyg_ppp_sifvjcomp
#define srand48 cyg_ppp_srand48
#define sys_check_options cyg_ppp_sys_check_options
#define sys_cleanup cyg_ppp_sys_cleanup
#define sys_close cyg_ppp_sys_close
#define sys_exit cyg_ppp_sys_exit
#define sys_init cyg_ppp_sys_init
#define syslog cyg_ppp_syslog
#define wait_input cyg_ppp_wait_input
#define wait_time cyg_ppp_wait_time

// net/ppp/current/src/upap.c:
#define pap_protent cyg_ppp_pap_protent
#define upap cyg_ppp_upap
#define upap_authpeer cyg_ppp_upap_authpeer
#define upap_authwithpeer cyg_ppp_upap_authwithpeer

// net/ppp/current/src/zlib.c:
#define _tr_align cyg_ppp__tr_align
#define _tr_flush_block cyg_ppp__tr_flush_block
#define _tr_init cyg_ppp__tr_init
#define _tr_stored_block cyg_ppp__tr_stored_block
#define _tr_stored_type_only cyg_ppp__tr_stored_type_only
#define _tr_tally cyg_ppp__tr_tally
#define adler32 cyg_ppp_adler32
#define deflate cyg_ppp_deflate
#define deflateCopy cyg_ppp_deflateCopy
#define deflateEnd cyg_ppp_deflateEnd
#define deflateInit2_ cyg_ppp_deflateInit2_
#define deflateInit_ cyg_ppp_deflateInit_
#define deflateOutputPending cyg_ppp_deflateOutputPending
#define deflateParams cyg_ppp_deflateParams
#define deflateReset cyg_ppp_deflateReset
#define deflateSetDictionary cyg_ppp_deflateSetDictionary
#define deflate_copyright cyg_ppp_deflate_copyright
#define inflateEnd cyg_ppp_inflateEnd
#define inflateIncomp cyg_ppp_inflateIncomp
#define inflateInit2_ cyg_ppp_inflateInit2_
#define inflateInit_ cyg_ppp_inflateInit_
#define inflateReset cyg_ppp_inflateReset
#define inflateSetDictionary cyg_ppp_inflateSetDictionary
#define inflateSync cyg_ppp_inflateSync
#define inflate_addhistory cyg_ppp_inflate_addhistory
#define inflate_blocks cyg_ppp_inflate_blocks
#define inflate_blocks_free cyg_ppp_inflate_blocks_free
#define inflate_blocks_new cyg_ppp_inflate_blocks_new
#define inflate_blocks_reset cyg_ppp_inflate_blocks_reset
#define inflate_codes cyg_ppp_inflate_codes
#define inflate_codes_free cyg_ppp_inflate_codes_free
#define inflate_codes_new cyg_ppp_inflate_codes_new
#define inflate_copyright cyg_ppp_inflate_copyright
#define inflate_fast cyg_ppp_inflate_fast
#define inflate_flush cyg_ppp_inflate_flush
#define inflate_mask cyg_ppp_inflate_mask
#define inflate_packet_flush cyg_ppp_inflate_packet_flush
#define inflate_ppp cyg_ppp_inflate_ppp
#define inflate_set_dictionary cyg_ppp_inflate_set_dictionary
#define inflate_trees_bits cyg_ppp_inflate_trees_bits
#define inflate_trees_dynamic cyg_ppp_inflate_trees_dynamic
#define inflate_trees_fixed cyg_ppp_inflate_trees_fixed
#define inflate_trees_free cyg_ppp_inflate_trees_free
#define zlibVersion cyg_ppp_zlibVersion

// ====================================================================
#endif // CYGONCE_PPP_NAMES_H
