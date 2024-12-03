/*
 * Network configuration layer (Linux)
 *
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: netconf_linux.h,v 1.1.1.6 2004/04/12 04:34:05 honor Exp $
 */

#ifndef _netconf_linux_h_
#define _netconf_linux_h_

/* Debug malloc() */
#ifdef DMALLOC
#include <dmalloc.h>
#endif /* DMALLOC */

/* iptables definitions */
#include <libiptc/libiptc.h>
#include <iptables.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ip_nat_rule.h>
#define ETH_ALEN ETHER_ADDR_LEN
#include <linux/netfilter_ipv4/ipt_mac.h>
#include <linux/netfilter_ipv4/ipt_state.h>
#include <linux/netfilter_ipv4/ipt_time.h>
#include <linux/netfilter_ipv4/ipt_TCPMSS.h>
#include <linux/netfilter_ipv4/ipt_LOG.h>
#include <linux/netfilter_ipv4/ip_autofw.h>

/* ipt_entry alignment attribute */
#define IPT_ALIGNED ((aligned(__alignof__(struct ipt_entry))))

/* TCP flags */
#define TH_FIN 0x01
#define TH_SYN 0x02
#define TH_RST 0x04
#define TH_PUSH 0x08
#define TH_ACK 0x10
#define TH_URG 0x20

#endif /* _netconf_linux_h_ */
