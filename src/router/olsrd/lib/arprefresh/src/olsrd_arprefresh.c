
/*
 * Copyright (c) 2007, Sven-Ola Tuecke <sven-ola-aet-gmx.de>
 * Copyright (c) 2004, Andreas Tonnesen(andreto-at-olsr.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * * Neither the name of the UniK olsr daemon nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * Plugin to refresh the local ARP cache from received OLSR broadcasts
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netpacket/packet.h>
#include <linux/types.h>
#include <linux/filter.h>
#include <unistd.h>

#include "olsrd_arprefresh.h"
#include "kernel_routes.h"
#include "scheduler.h"

#undef ARPREFRESH_DEBUG
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

    if (0 <= size && size >= (ssize_t) sizeof(buf)

                                                 /*** &&
		    ETH_P_IP == ntohs(buf.eth.h_proto) &&
		    IPPROTO_UDP == buf.ip.protocol &&
		    arprefresh_portnum == ntohs(buf.udp.source) &&
		    arprefresh_portnum == ntohs(buf.udp.dest) ***/
      ) {
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
#endif
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
      {0x28, 0, 0, 0x0000000c},
      {0x15, 0, 4, 0x000086dd},
      {0x30, 0, 0, 0x00000014},
      {0x15, 0, 11, 0x00000011},
      {0x28, 0, 0, 0x00000038},
      {0x15, 8, 9, arprefresh_portnum},
      {0x15, 0, 8, 0x00000800},
      {0x30, 0, 0, 0x00000017},
      {0x15, 0, 6, 0x00000011},
      {0x28, 0, 0, 0x00000014},
      {0x45, 4, 0, 0x00001fff},
      {0xb1, 0, 0, 0x0000000e},
      {0x48, 0, 0, 0x00000010},
      {0x15, 0, 1, arprefresh_portnum},
      {0x6, 0, 0, sizeof(arprefresh_buf)}
      ,
      {0x6, 0, 0, 0x00000000}
    };
    filter.len = sizeof(BPF_code) / sizeof(BPF_code[0]);
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
  printf("OLSRD arprefresh plugin by Sven-Ola\n");
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
