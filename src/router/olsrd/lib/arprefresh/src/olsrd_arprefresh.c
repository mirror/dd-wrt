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

/*
 * Plugin to refresh the local ARP cache from received OLSR broadcasts.
 *
 * Note: this code does not work with IPv6 and not with VLANs (on IPv4 or IPv6)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_arp.h>
#ifndef __ANDROID__
#include <net/ethernet.h>
#endif /* __ANDROID__ */
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netpacket/packet.h>
#include <linux/types.h>
#include <linux/filter.h>
#include <unistd.h>

#include "olsrd_arprefresh.h"
#include "kernel_routes.h"
#include "scheduler.h"
#include "olsr.h"
#include "builddata.h"

#undef ARPREFRESH_DEBUG

#define PLUGIN_TITLE             "OLSRD arprefresh plugin"
#define PLUGIN_INTERFACE_VERSION 5

/****************************************************************************
 *                Functions that the plugin MUST provide                    *
 ****************************************************************************/

/**
 * Plugin interface version
 * Used by main olsrd to check plugin interface version
 */
int
olsrd_plugin_interface_version(void)
{
  return PLUGIN_INTERFACE_VERSION;
}

static const struct olsrd_plugin_parameters plugin_parameters[] = {
};

void
olsrd_get_plugin_parameters(const struct olsrd_plugin_parameters **params, int *size)
{
  *params = plugin_parameters;
  *size = sizeof(plugin_parameters) / sizeof(*plugin_parameters);
}

typedef struct {
  struct ethhdr eth;
  struct iphdr ip;
  struct udphdr udp;
} __attribute__ ((packed)) arprefresh_buf;

static int arprefresh_sockfd = -1;
static const int arprefresh_portnum = 698;

/**
 * Scheduled event to fetch gathered packets and update the ARP cache
 * called from olsrd main thread
 */
static void
olsr_arp_event(void *foo __attribute__ ((unused)))
{
  if (0 <= arprefresh_sockfd) {
    arprefresh_buf buf;
    struct sockaddr_ll from;
    socklen_t fromlen = sizeof(from);

    /*
     * Grab a single snapshot on the packet socket. This only works
     * if not too much IP traffic is currently flowing through.
     */
    ssize_t size = recvfrom(arprefresh_sockfd, &buf, sizeof(buf),
                            MSG_TRUNC, (struct sockaddr *)&from,
                            &fromlen);

    if (0 <= size && size >= (ssize_t) sizeof(buf)) {
      union {
        struct arpreq arp;
        struct sockaddr_in in_pa;
        struct sockaddr_in6 in_pa6;
      } req;

      memset(&req, 0, sizeof(req));
      req.in_pa.sin_family = AF_INET;
      memcpy(&req.in_pa.sin_addr, &buf.ip.saddr, sizeof(buf.ip.saddr));
      req.arp.arp_ha.sa_family = AF_LOCAL;
      memcpy(&req.arp.arp_ha.sa_data, &buf.eth.h_source, sizeof(buf.eth.h_source));
      req.arp.arp_flags = ATF_COM;
      if_indextoname(from.sll_ifindex, req.arp.arp_dev);
#ifdef ARPREFRESH_DEBUG
      {
        int i;
        OLSR_PRINTF(0, "Refresh on %s, %s=", req.arp.arp_dev, inet_ntoa(*((struct in_addr *)&buf.ip.saddr)));
        for (i = 0; i < (ssize_t) sizeof(buf.eth.h_source); i++) {
          OLSR_PRINTF(0, "%02x%s", ((unsigned char *)&buf.eth.h_source)[i],
                      i < (ssize_t) sizeof(buf.eth.h_source) - 1 ? ":" : "\n");
        }
      }
#endif /* ARPREFRESH_DEBUG */
      if (ioctl(arprefresh_sockfd, SIOCSARP, &req) < 0) {
        OLSR_PRINTF(1, "*** ARPREFRESH: SIOCSARP: %s\n", strerror(errno));
        close(arprefresh_sockfd);
        arprefresh_sockfd = -1;
        return;
      }
    }
  }
}

/**
 * Initialize plugin
 * Called after all parameters are passed
 */
int
olsrd_plugin_init(void)
{
  int ret = 0;
  if (AF_INET == olsr_cnf->ip_version) {
    int flags;
    struct sock_fprog filter;
    struct sock_filter BPF_code[] = {
      /* tcpdump -s [sizeof(arprefresh_buf)] -ni lo udp and dst port [arprefresh_portnum] -dd */

      /* Note: This program assumes NON-VLAN */
      /* See also http://www.unix.com/man-page/FreeBSD/4/bpf/ */
      /*      code                          jump-true jump-false generic-multiuse-field */
             {BPF_LD  | BPF_H    | BPF_ABS , 0,        0,         0x0000000c},             // 0x28: A <- P[k:2], get 2-bytes from offset 0x0c : get Ethernet type

/* test4  */ {BPF_JMP | BPF_JEQ  | BPF_K   , 0,        8,         0x00000800},             // 0x15: pc += (A == k) ? ipv4 : ignore            : IPv4?

/* ipv4   */ {BPF_LD  | BPF_B    | BPF_ABS , 0,        0,         0x00000017},             // 0x30: A <- P[k:1], get 1-byte from offset 0x17  : get IPv4 protocol
             {BPF_JMP | BPF_JEQ  | BPF_K   , 0,        6,         0x00000011},             // 0x15: pc += (A == k) ? udp4 : ignore            : UDP?
/* udp4   */ {BPF_LD  | BPF_H    | BPF_ABS , 0,        0,         0x00000014},             // 0x28: A <- P[k:2], get 2-bytes from offset 0x14 : get fragment offset
             {BPF_JMP | BPF_JSET | BPF_K   , 4,        0,         0x00001fff},             // 0x45: pc += (A & k) ? ignore : nofrag           : is this a fragment?
/* nofrag */ {BPF_LDX | BPF_B    | BPF_MSH , 0,        0,         0x0000000e},             // 0xb1: X <- 4*(P[k:1]&0xf)                       : get the IP header length in bytes
             {BPF_LD  | BPF_H    | BPF_IND , 0,        0,         0x00000010},             // 0x48: A <- P[X+k:2]                             : get UDP destination port
             {BPF_JMP | BPF_JEQ  | BPF_K   , 0,        1,         arprefresh_portnum},     // 0x15: pc += (A == k) ? ok : ignore              : sent to port arprefresh_portnum?

/* ok     */ {BPF_RET | BPF_K              , 0,        0,         sizeof(arprefresh_buf)}, //                                                 : accept sizeof(arprefresh_buf) bytes (all headers)
/* ignore */ {BPF_RET | BPF_K              , 0,        0,         0x00000000}              //                                                 : accept 0 bytes

    };
    filter.len = ARRAYSIZE(BPF_code);
    filter.filter = BPF_code;
    if (0 <= (arprefresh_sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP)))
        && 0 <= (flags = fcntl(arprefresh_sockfd, F_GETFL))
        && 0 <= fcntl(arprefresh_sockfd, F_SETFL, flags | O_NONBLOCK)
        && 0 <= setsockopt(arprefresh_sockfd, SOL_SOCKET, SO_ATTACH_FILTER, &filter, sizeof(filter))) {
      /* Register the ARP refresh event */
      olsr_start_timer(2 * MSEC_PER_SEC, 0, OLSR_TIMER_PERIODIC, &olsr_arp_event, NULL, 0);
      ret = 1;
    } else {
      OLSR_PRINTF(1, "*** ARPREFRESH: Cannot create non-blocking filtering packet socket: %s\n", strerror(errno));
    }
  } else {
    OLSR_PRINTF(1, "*** ARPREFRESH: IPv6 not supported\n");
  }
  return ret;
}

/****************************************************************************
 *       Optional private constructor and destructor functions              *
 ****************************************************************************/

static void __attribute__ ((constructor)) my_init(void);
static void __attribute__ ((destructor)) my_fini(void);

static void
my_init(void)
{
  /* Print plugin info to stdout */
  olsr_printf(0, "%s (%s)\n", PLUGIN_TITLE, git_descriptor);
}

/**
 * Optional Private Destructor
 */
static void
my_fini(void)
{
  if (0 <= arprefresh_sockfd) {
    close(arprefresh_sockfd);
    arprefresh_sockfd = -1;
  }
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
