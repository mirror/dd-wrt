/*
 * Copyright (c) 2004-2006 Maxim Sobolev <sobomax@FreeBSD.org>
 * Copyright (c) 2006-2014 Sippy Software, Inc., http://www.sippysoft.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifndef _RTPP_CFG_STABLE_H_
#define _RTPP_CFG_STABLE_H_

/*
 * TTL counters are used to detect the absence of audio packets
 * in either direction.  When the counter reaches 0, the call timeout
 * occurs.
 */
typedef enum {
    TTL_UNIFIED = 0,            /* all TTL counters must reach 0 */
    TTL_INDEPENDENT = 1         /* any TTL counter reaches 0 */
} rtpp_ttl_mode;

struct rtpp_cfg_stable {
    const char *pid_file;

    int nodaemon;
    int dmode;
    int bmode;                  /* Bridge mode */
    int port_min;               /* Lowest UDP port for RTP */
    int port_max;               /* Highest UDP port number for RTP */
    int seq_ports;              /* Allocate ports in sequential manner rather than randomly */
    int port_ctl;               /* Port number for UDP control, 0 for Unix domain */
    int max_ttl;
    int max_setup_ttl;
    /*
     * The first address is for external interface, the second one - for
     * internal one. Second can be NULL, in this case there is no bridge
     * mode enabled.
     */
    struct sockaddr *bindaddr[2];   /* RTP socket(s) addresses */
    char const * advaddr[2];        /* advertised addresses */
    int tos;

    const char *rdir;
    const char *sdir;
    int record_pcap;                /* Record in the PCAP format? */
    int record_all;                 /* Record everything */

    int rrtcp;                      /* Whether or not to relay RTCP? */
    rtpp_log_t glog;

    struct rlimit *nofile_limit;
    char *run_uname;
    char *run_gname;
    mode_t sock_mode;
    int no_check;

    rtpp_ttl_mode ttl_mode;

    uid_t run_uid;
    gid_t run_gid;

    int log_level;
    int log_facility;

    uint16_t port_table[65536];
    int port_table_len;

    struct rtpp_hash_table_obj *sessions_ht;

    double sched_offset;
    int sched_policy;
    int sched_hz;
    double target_pfreq;
    struct rtpp_cmd_async_cf *rtpp_cmd_cf;
    struct rtpp_proc_async_cf *rtpp_proc_cf;
    struct rtpp_anetio_cf *rtpp_netio_cf;
    int slowshutdown;

    struct rtpp_stats_obj *rtpp_stats;

    struct rtpp_list *ctrl_socks;
};

#endif
