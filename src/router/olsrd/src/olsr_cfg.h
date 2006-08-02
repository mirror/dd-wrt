/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 *
 * * Redistributions of source code must retain the above copyright 
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimer in 
 *   the documentation and/or other materials provided with the 
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 * $Id: olsr_cfg.h,v 1.27 2005/11/17 04:25:44 tlopatic Exp $
 */


#ifndef _OLSRD_CFGPARSER_H
#define _OLSRD_CFGPARSER_H

#include "olsr_types.h"

/* Default valuse not declared in olsr_protocol.h */
#define DEF_POLLRATE        0.05
#define DEF_WILL_AUTO       OLSR_TRUE
#define DEF_ALLOW_NO_INTS   OLSR_TRUE
#define DEF_TOS             16
#define DEF_DEBUGLVL        1
#define DEF_IPC_CONNECTIONS 0
#define DEF_USE_HYST        OLSR_TRUE
#define DEF_LQ_LEVEL        0
#define DEF_LQ_FISH         0
#define DEF_LQ_DIJK_LIMIT   255
#define DEF_LQ_DIJK_INTER   0.0
#define DEF_LQ_WSIZE        10
#define DEF_CLEAR_SCREEN    OLSR_FALSE

/* Bounds */

#define MIN_INTERVAL        0.01

#define MAX_POLLRATE        10.0
#define MIN_POLLRATE        0.01
#define MAX_DEBUGLVL        9
#define MIN_DEBUGLVL        0
#define MAX_TOS             16
#define MIN_TOS             0
#define MAX_WILLINGNESS     7
#define MIN_WILLINGNESS     0
#define MAX_MPR_COVERAGE    20
#define MIN_MPR_COVERAGE    1
#define MAX_TC_REDUNDANCY   2
#define MIN_TC_REDUNDANCY   0
#define MAX_HYST_PARAM      1.0
#define MIN_HYST_PARAM      0.0
#define MAX_LQ_LEVEL        2
#define MIN_LQ_LEVEL        0
#define MAX_LQ_WSIZE        128
#define MIN_LQ_WSIZE        3

#ifndef IPV6_ADDR_SITELOCAL
#define IPV6_ADDR_SITELOCAL    0x0040U
#endif

#include "interfaces.h"

struct olsr_msg_params
{
  float                    emission_interval;
  float                    validity_time;
};

struct olsr_lq_mult
{
  union olsr_ip_addr addr;
  float val;
  struct olsr_lq_mult *next;
};

struct olsr_if_weight
{
  int        value;
  olsr_bool  fixed;
};

struct if_config_options
{
  union olsr_ip_addr       ipv4_broadcast;
  int                      ipv6_addrtype;
  union olsr_ip_addr       ipv6_multi_site;
  union olsr_ip_addr       ipv6_multi_glbl;
  struct olsr_if_weight    weight;
  struct olsr_msg_params   hello_params;
  struct olsr_msg_params   tc_params;
  struct olsr_msg_params   mid_params;
  struct olsr_msg_params   hna_params;
  struct olsr_lq_mult      *lq_mult;
};



struct olsr_if
{
  char                     *name;
  char                     *config;
  int                      index;
  olsr_bool                configured;
  olsr_bool                host_emul;
  union olsr_ip_addr       hemu_ip;
  struct interface         *interf;
  struct if_config_options *cnf;
  struct olsr_if           *next;
};

struct hna4_entry
{
  union olsr_ip_addr       net;
  union olsr_ip_addr       netmask;
  struct hna4_entry        *next;
};

struct hna6_entry
{
  union olsr_ip_addr       net;
  olsr_u16_t               prefix_len;
  struct hna6_entry        *next;
};

struct hyst_param
{
  float                    scaling;
  float                    thr_high;
  float                    thr_low;
};

struct plugin_param
{
  char                     *key;
  char                     *value;
  struct plugin_param      *next;
};

struct plugin_entry
{
  char                     *name;
  struct plugin_param      *params;
  struct plugin_entry      *next;
};

struct ipc_host
{
  union olsr_ip_addr       host;
  struct ipc_host          *next;
};

struct ipc_net
{
  union olsr_ip_addr       net;
  union olsr_ip_addr       mask;
  struct ipc_net           *next;
};

/*
 * The config struct
 */

struct olsrd_config
{
  int                      debug_level;
  olsr_bool                no_fork;
  olsr_bool                host_emul;
  int                      ip_version;
  olsr_bool                allow_no_interfaces;
  olsr_u16_t               tos;
  olsr_bool                willingness_auto;
  olsr_u8_t                willingness;
  int                      ipc_connections;
  olsr_bool                open_ipc;
  olsr_bool                use_hysteresis;
  struct hyst_param        hysteresis_param;
  float                    pollrate;
  olsr_u8_t                tc_redundancy;
  olsr_u8_t                mpr_coverage;
  olsr_bool                clear_screen;
  olsr_u8_t                lq_level;
  olsr_u32_t               lq_wsize;
  olsr_u8_t                lq_fish;
  olsr_u8_t                lq_dlimit;
  float                    lq_dinter;
  struct plugin_entry      *plugins;
  struct hna4_entry        *hna4_entries;
  struct hna6_entry        *hna6_entries;
  struct ipc_host          *ipc_hosts;
  struct ipc_net           *ipc_nets;
  struct olsr_if           *interfaces;
  olsr_u16_t               ifcnt;
};

#if defined __cplusplus
extern "C" {
#endif

/*
 * Interface to parser
 */

struct olsrd_config *
olsrd_parse_cnf(const char *);

int
olsrd_sanity_check_cnf(struct olsrd_config *);

void
olsrd_free_cnf(struct olsrd_config *);

void
olsrd_print_cnf(struct olsrd_config *);

int
olsrd_write_cnf(struct olsrd_config *, const char *);

int
olsrd_write_cnf_buf(struct olsrd_config *, char *, olsr_u32_t);

struct if_config_options *
get_default_if_config(void);

struct olsrd_config *
olsrd_get_default_cnf(void);

void *
olsrd_cnf_malloc(unsigned int);

void
olsrd_cnf_free(void *);

#if defined __cplusplus
}
#endif

#endif
