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
 * $Id: interfaces.h,v 1.30 2005/06/03 08:00:55 kattemat Exp $
 */


#ifndef _OLSR_INTERFACE
#define _OLSR_INTERFACE

#include <sys/types.h>
#include <sys/socket.h>

#include "olsr_types.h"

#define _PATH_PROCNET_IFINET6           "/proc/net/if_inet6"


#define IPV6_ADDR_ANY		0x0000U

#define IPV6_ADDR_UNICAST      	0x0001U
#define IPV6_ADDR_MULTICAST    	0x0002U
#define IPV6_ADDR_ANYCAST	0x0004U

#define IPV6_ADDR_LOOPBACK	0x0010U
#define IPV6_ADDR_LINKLOCAL	0x0020U
#define IPV6_ADDR_SITELOCAL	0x0040U

#define IPV6_ADDR_COMPATv4	0x0080U

#define IPV6_ADDR_SCOPE_MASK	0x00f0U

#define IPV6_ADDR_MAPPED	0x1000U
#define IPV6_ADDR_RESERVED	0x2000U


#define MAX_IF_METRIC           100


enum olsr_if_wieght
  {
    WEIGHT_LOWEST = 0,
    WEIGHT_LOW,
    WEIGHT_ETHERNET_1GBP,     /* Ethernet 1Gb+        */
    WEIGHT_ETHERNET_1GB,      /* Ethernet 1Gb         */
    WEIGHT_ETHERNET_100MB,    /* Ethernet 100Mb       */
    WEIGHT_ETHERNET_10MB,     /* Ethernet 10Mb        */
    WEIGHT_ETHERNET_DEFAULT,  /* Ethernet unknown rate*/
    WEIGHT_WLAN_HIGH,         /* >54Mb WLAN           */
    WEIGHT_WLAN_54MB,         /* 54Mb 802.11g         */
    WEIGHT_WLAN_11MB,         /* 11Mb 802.11b         */
    WEIGHT_WLAN_LOW,          /* <11Mb WLAN           */
    WEIGHT_WLAN_DEFAULT,      /* WLAN unknown rate    */
    WEIGHT_SERIAL,            /* Serial device        */
    WEIGHT_HIGH,              /* Max                  */
    WEIGHT_HIGHEST = WEIGHT_HIGH
  };

struct if_gen_property
{
  olsr_u32_t             owner_id;
  void                   *data;
  struct if_gen_property *next;
};

struct vtimes
{
  olsr_u8_t hello;
  olsr_u8_t tc;
  olsr_u8_t mid;
  olsr_u8_t hna;
};

/**
 *A struct containing all necessary information about each
 *interface participating in the OLSD routing
 */
struct interface 
{
  /* IP version 4 */
  struct	sockaddr int_addr;		/* address */
  struct	sockaddr int_netmask;		/* netmask */
  struct	sockaddr int_broadaddr;         /* broadcast address */
  /* IP version 6 */
  struct        sockaddr_in6 int6_addr;         /* Address */
  struct        sockaddr_in6 int6_multaddr;     /* Multicast */
  /* IP independent */
  union         olsr_ip_addr ip_addr;
  int           is_hcif;                        /* Is this a emulated host-client if? */
  int           olsr_socket;                    /* The broadcast socket for this interface */
  int	        int_metric;			/* metric of interface */
  int           int_mtu;                        /* MTU of interface */
  int	        int_flags;			/* see below */
  char	        *int_name;			/* from kernel if structure */
  int           if_index;                       /* Kernels index of this interface */
  int           if_nr;                          /* This interfaces index internally*/
  int           is_wireless;                    /* wireless interface or not*/
  olsr_u16_t    olsr_seqnum;                    /* Olsr message seqno */

  float         hello_etime;
  struct        vtimes valtimes;

  struct        if_gen_property *gen_properties;/* Generic interface properties */

  struct	interface *int_next;
};


#define OLSR_DEFAULT_MTU             1500

/* Ifchange actions */

#define IFCHG_IF_ADD           1
#define IFCHG_IF_REMOVE        2
#define IFCHG_IF_UPDATE        3

/* The rate to poll for interface changes at */
#define IFCHANGES_POLL_INT     2.5


/* The interface linked-list */
extern struct interface *ifnet;

/* Datastructures to use when creating new sockets */
extern struct sockaddr_in addrsock;
extern struct sockaddr_in6 addrsock6;


int
ifinit(void);

olsr_u32_t
get_if_property_id(void);

olsr_bool
add_if_geninfo(struct interface *, void *, olsr_u32_t);

void *
get_if_geninfo(struct interface *, olsr_u32_t);

void *
del_if_geninfo(struct interface *, olsr_u32_t);

void
run_ifchg_cbs(struct interface *, int);

struct interface *
if_ifwithsock(int);

struct interface *
if_ifwithaddr(union olsr_ip_addr *);

struct interface *
if_ifwithname(const char *);

struct olsr_if *
queue_if(char *, int);

int
add_ifchgf(int (*f)(struct interface *, int));

int
del_ifchgf(int (*f)(struct interface *, int));

#endif
