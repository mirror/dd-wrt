/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
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
 */

#ifndef _OLSR_INTERFACE
#define _OLSR_INTERFACE

#include <sys/types.h>
#ifdef _MSC_VER
#include <WS2tcpip.h>
#undef interface
#else /* _MSC_VER */
#include <sys/socket.h>
#endif /* _MSC_VER */
#include <time.h>

#include "olsr_types.h"
#include "mantissa.h"

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

#define WEIGHT_LOWEST           0       /* No weight            */
#define WEIGHT_LOW              1       /* Low                  */
#define WEIGHT_ETHERNET_1GBP    2       /* Ethernet 1Gb+        */
#define WEIGHT_ETHERNET_1GB     4       /* Ethernet 1Gb         */
#define WEIGHT_ETHERNET_100MB   8       /* Ethernet 100Mb       */
#define WEIGHT_ETHERNET_10MB    16      /* Ethernet 10Mb        */
#define WEIGHT_ETHERNET_DEFAULT 32      /* Ethernet unknown rate */
#define WEIGHT_WLAN_HIGH        64      /* >54Mb WLAN           */
#define WEIGHT_WLAN_54MB        128     /* 54Mb 802.11g         */
#define WEIGHT_WLAN_11MB        256     /* 11Mb 802.11b         */
#define WEIGHT_WLAN_LOW         512     /* <11Mb WLAN           */
#define WEIGHT_WLAN_DEFAULT     1024    /* WLAN unknown rate    */
#define WEIGHT_SERIAL           2048    /* Serial device        */
#define WEIGHT_HIGH             4096    /* High                 */
#define WEIGHT_HIGHEST          8192    /* Really high          */

struct if_gen_property {
  uint32_t owner_id;
  void *data;
  struct if_gen_property *next;
};

struct vtimes {
  uint8_t hello;
  uint8_t tc;
  uint8_t mid;
  uint8_t hna;
  uint32_t hna_reltime;
};

/* Output buffer structure. This should actually be in net_olsr.h but we have circular references then.
 */
struct olsr_netbuf {
  uint8_t *buff;                       /* Pointer to the allocated buffer */
  int bufsize;                         /* Size of the buffer */
  int maxsize;                         /* Max bytes of payload that can be added to the buffer */
  int pending;                         /* How much data is currently pending in the buffer */
  int reserved;                        /* Plugins can reserve space in buffers */
};

/**
 *A struct containing all necessary information about each
 *interface participating in the OLSRD routing
 */
struct interface_olsr {
  /* IP version 4 */
  struct sockaddr_in int_addr;         /* address */
  struct sockaddr_in int_netmask;      /* netmask */
  struct sockaddr_in int_broadaddr;    /* broadcast address */
  int mode;                            /* interface mode */
  /* IP version 6 */
  struct sockaddr_in6 int6_addr;       /* Address */
  struct sockaddr_in6 int6_multaddr;   /* Multicast */
  /* IP independent */
  union olsr_ip_addr ip_addr;
  int is_hcif;                         /* Is this a emulated host-client if? */

  int olsr_socket;                     /* The broadcast socket for this interface */
  int send_socket;                     /* The send socket for this interface */

  int int_metric;                      /* metric of interface */
  int int_mtu;                         /* MTU of interface */
  int int_flags;                       /* see below */
  int if_index;                        /* Kernels index of this interface */
  int is_wireless;                     /* wireless interface or not */
  char *int_name;                      /* from kernel if structure */
  uint16_t olsr_seqnum;                /* Olsr message seqno */

  /* Periodic message generation timers */
  struct timer_entry *hello_gen_timer;
  struct timer_entry *hna_gen_timer;
  struct timer_entry *mid_gen_timer;
  struct timer_entry *tc_gen_timer;

#ifdef __linux__

/* Struct used to store original redirect/ingress setting */
  struct nic_state {
    /* The original state of icmp redirect */
    char redirect;

    /* The original state of the IP spoof filter */
    char spoof;
  } nic_state;
#endif /* __linux__ */

  olsr_reltime hello_etime;
  struct vtimes valtimes;

  /* Timeout for OLSR forwarding on this if */
  uint32_t fwdtimer;

  /* Timeout for OLSR to keep sending zero bandwidth sgw HNAs */
  uint32_t sgw_sgw_zero_bw_timeout;

  /* the buffer to construct the packet data */
  struct olsr_netbuf netbuf;

  /* Generic interface properties */
  struct if_gen_property *gen_properties;

  /* index in TTL array for fish-eye */
  int ttl_index;

  /* Hello's are sent immediately normally, this flag prefers to send TC's */
  bool immediate_send_tc;

  /* backpointer to olsr_if configuration */
  struct olsr_if *olsr_if;
  struct interface_olsr *int_next;
};

#define OLSR_DEFAULT_MTU             1500

/* Ifchange actions */

enum olsr_ifchg_flag {
  IFCHG_IF_ADD = 1,
  IFCHG_IF_REMOVE = 2,
  IFCHG_IF_UPDATE = 3
};

/* The interface linked-list */
extern struct interface_olsr *ifnet;

int olsr_init_interfacedb(void);
void olsr_delete_interfaces(void);

void olsr_trigger_ifchange(int if_index, struct interface_olsr *, enum olsr_ifchg_flag);

struct interface_olsr *if_ifwithsock(int);

struct interface_olsr *if_ifwithaddr(const union olsr_ip_addr *);

struct interface_olsr *if_ifwithname(const char *);
struct olsr_if *olsrif_ifwithname(const char *if_name);

const char *if_ifwithindex_name(const int if_index);

struct interface_olsr *if_ifwithindex(const int if_index);

struct olsr_if *olsr_create_olsrif(const char *name, int hemu);

int olsr_add_ifchange_handler(void (*f) (int if_index, struct interface_olsr *, enum olsr_ifchg_flag));
int olsr_remove_ifchange_handler(void (*f) (int if_index, struct interface_olsr *, enum olsr_ifchg_flag));

void olsr_remove_interface(struct olsr_if *);

extern struct olsr_cookie_info *interface_poll_timer_cookie;
extern struct olsr_cookie_info *hello_gen_timer_cookie;
extern struct olsr_cookie_info *tc_gen_timer_cookie;
extern struct olsr_cookie_info *mid_gen_timer_cookie;
extern struct olsr_cookie_info *hna_gen_timer_cookie;

#endif /* _OLSR_INTERFACE */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
