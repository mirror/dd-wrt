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
 * Dynamic linked library for the olsr.org olsr daemon
 */

#ifdef _WRS_KERNEL
#include <vxWorks.h>
#include <sockLib.h>
#include <wrn/coreip/netinet/in.h>
#else /* _WRS_KERNEL */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#endif /* _WRS_KERNEL */

#include "olsr.h"
#include "ipcalc.h"
#include "olsr_types.h"
#include "neighbor_table.h"
#include "two_hop_neighbor_table.h"
#include "tc_set.h"
#include "hna_set.h"
#include "mid_set.h"
#include "link_set.h"
#include "net_olsr.h"
#include "lq_plugin.h"
#include "common/autobuf.h"

#include "olsrd_dot_draw.h"
#include "olsrd_plugin.h"

#ifdef _WIN32
#define close(x) closesocket(x)
#endif /* _WIN32 */

#ifdef _WRS_KERNEL
static int ipc_open;
static int ipc_socket_up;
#define DOT_DRAW_PORT 2004
#endif /* _WRS_KERNEL */

static int ipc_socket = -1;

struct autobuf outbuffer;
static int outbuffer_socket = -1;

static struct timer_entry *writetimer_entry = NULL;

/* IPC initialization function */
static int plugin_ipc_init(void);

/* Event function to register with the sceduler */
static int pcf_event(int, int, int);

static void ipc_action(int, void *, unsigned int);

static void ipc_print_neigh_link(struct autobuf *abuf, const struct neighbor_entry *neighbor);

static void ipc_print_tc_link(struct autobuf *abuf, const struct tc_entry *, const struct tc_edge_entry *);

static void ipc_print_net(struct autobuf *abuf, const union olsr_ip_addr *, const union olsr_ip_addr *, uint8_t);

/**
 *Do initialization here
 *
 *This function is called by the my_init
 *function in uolsrd_plugin.c
 */
#ifdef _WRS_KERNEL
int
olsrd_dotdraw_init(void)
#else /* _WRS_KERNEL */
int
olsrd_plugin_init(void)
#endif /* _WRS_KERNEL */
{
  /* Initial IPC */
  plugin_ipc_init();

  /* Register the "ProcessChanges" function */
  register_pcf(&pcf_event);

  return 1;
}

/**
 * destructor - called at unload
 */
#ifdef _WRS_KERNEL
void
olsrd_dotdraw_exit(void)
#else /* _WRS_KERNEL */
void
olsr_plugin_exit(void)
#endif /* _WRS_KERNEL */
{
  if (writetimer_entry) {
    close(outbuffer_socket);
    abuf_free(&outbuffer);
    olsr_stop_timer(writetimer_entry);
  }
  if (ipc_socket != -1) {
    CLOSE(ipc_socket);
  }
}

static void
ipc_print_neigh_link(struct autobuf *abuf, const struct neighbor_entry *neighbor)
{
  static const char DASHED[] = "dashed";
  static const char SOLID[] = "solid";

  struct ipaddr_str mainaddrstrbuf, strbuf;
  olsr_linkcost etx = 0.0;
  const char *style;
  const char *adr = olsr_ip_to_string(&mainaddrstrbuf, &olsr_cnf->main_addr);
  struct link_entry *the_link;
  struct lqtextbuffer lqbuffer;

  if (neighbor->status == 0) {  /* non SYM */
    style = DASHED;
  } else {
    the_link = get_best_link_to_neighbor(&neighbor->neighbor_main_addr);
    if (the_link) {
      etx = the_link->linkcost;
    }
    style = SOLID;
  }

  abuf_appendf(abuf, "\"%s\" -> \"%s\"[label=\"%s\", style=%s];\n", adr, olsr_ip_to_string(&strbuf, &neighbor->neighbor_main_addr),
               get_linkcost_text(etx, false, &lqbuffer), style);

  if (neighbor->is_mpr) {
    abuf_appendf(abuf, "\"%s\"[shape=box];\n", adr);
  }
}

static int
plugin_ipc_init(void)
{
  struct sockaddr_in sock_in;
  uint32_t yes = 1;

  if (ipc_socket != -1) {
    close(ipc_socket);
  }

  /* Init ipc socket */
  ipc_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (ipc_socket == -1) {
    olsr_printf(1, "(DOT DRAW)IPC socket %s\n", strerror(errno));
    return 0;
  }

  if (setsockopt(ipc_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof(yes)) < 0) {
    perror("SO_REUSEADDR failed");
    CLOSE(ipc_socket);
    return 0;
  }
#if (defined __FreeBSD__ || defined __FreeBSD_kernel__) && defined SO_NOSIGPIPE
  if (setsockopt(ipc_socket, SOL_SOCKET, SO_NOSIGPIPE, (char *)&yes, sizeof(yes)) < 0) {
    perror("SO_REUSEADDR failed");
    CLOSE(ipc_socket);
    return 0;
  }
#endif /* (defined __FreeBSD__ || defined __FreeBSD_kernel__) && defined SO_NOSIGPIPE */

  /* Bind the socket */

  /* complete the socket structure */
  memset(&sock_in, 0, sizeof(sock_in));
  sock_in.sin_family = AF_INET;
  sock_in.sin_addr.s_addr = ipc_listen_ip.v4.s_addr;
  sock_in.sin_port = htons(ipc_port);

  /* bind the socket to the port number */
  if (bind(ipc_socket, (struct sockaddr *)&sock_in, sizeof(sock_in)) == -1) {
    olsr_printf(1, "(DOT DRAW)IPC bind %s\n", strerror(errno));
    CLOSE(ipc_socket);
    return 0;
  }

  /* show that we are willing to listen */
  if (listen(ipc_socket, 1) == -1) {
    olsr_printf(1, "(DOT DRAW)IPC listen %s\n", strerror(errno));
    CLOSE(ipc_socket);
    return 0;
  }

  /* Register with olsrd */
  add_olsr_socket(ipc_socket, &ipc_action, NULL, NULL, SP_PR_READ);

  return 1;
}

static void
ipc_action(int fd __attribute__ ((unused)), void *data __attribute__ ((unused)), unsigned int flags __attribute__ ((unused)))
{
  struct sockaddr_in pin;
  socklen_t addrlen = sizeof(struct sockaddr_in);
  int ipc_connection;

  ipc_connection = accept(ipc_socket, (struct sockaddr *)&pin, &addrlen);
  if (ipc_connection == -1) {
    olsr_printf(1, "(DOT DRAW)IPC accept: %s\n", strerror(errno));
    return;
  }

  if (outbuffer_socket != -1) {
    olsr_printf(1, "(DOT DRAW)Only one connection at once allowed.\n");
    close(ipc_connection);
    return;
  }
#ifndef _WRS_KERNEL
  if (!ip4equal(&pin.sin_addr, &ipc_accept_ip.v4) && ipc_accept_ip.v4.s_addr != INADDR_ANY) {
    olsr_printf(0, "Front end-connection from foreign host (%s) not allowed!\n", inet_ntoa(pin.sin_addr));
    CLOSE(ipc_connection);
    return;
  }
#endif /* _WRS_KERNEL */
  olsr_printf(1, "(DOT DRAW)IPC: Connection from %s\n", inet_ntoa(pin.sin_addr));

  abuf_init(&outbuffer, AUTOBUFCHUNK);
  outbuffer_socket = ipc_connection;

  pcf_event(1, 1, 1);
}

static void
dotdraw_write_data(void *foo __attribute__ ((unused))) {
  fd_set set;
  int result;
  struct timeval tv;

  FD_ZERO(&set);
  /* prevent warning on WIN32 */
  FD_SET((unsigned int)outbuffer_socket, &set);

  tv.tv_sec = 0;
  tv.tv_usec = 0;

  result = select(outbuffer_socket + 1, NULL, &set, NULL, &tv);
  if (result <= 0) {
    return;
  }

  if (FD_ISSET(outbuffer_socket, &set)) {
    result = send(outbuffer_socket, outbuffer.buf, outbuffer.len, 0);
    if (result > 0)
      abuf_pull(&outbuffer, result);

    if (result < 0) {
      /* close this socket and cleanup*/
      close(outbuffer_socket);
      abuf_free(&outbuffer);
      olsr_stop_timer(writetimer_entry);
      writetimer_entry = NULL;
      outbuffer_socket = -1;
    }
  }
}

/**
 *Scheduled event
 */
static int
pcf_event(int my_changes_neighborhood, int my_changes_topology, int my_changes_hna)
{
  struct neighbor_entry *neighbor_table_tmp;
  struct tc_entry *tc;
  struct tc_edge_entry *tc_edge;
  struct hna_entry *tmp_hna;
  struct hna_net *tmp_net;
  struct ip_prefix_list *hna;
  int res = 0;

  /* nothing to do */
  if (outbuffer_socket == -1) {
    return 1;
  }

  if (my_changes_neighborhood || my_changes_topology || my_changes_hna) {
    /* Print tables to IPC socket */
    abuf_puts(&outbuffer, "digraph topology\n{\n");

    /* Neighbors */
    OLSR_FOR_ALL_NBR_ENTRIES(neighbor_table_tmp) {
      ipc_print_neigh_link(&outbuffer, neighbor_table_tmp);
    }
    OLSR_FOR_ALL_NBR_ENTRIES_END(neighbor_table_tmp);

    /* Topology */
    OLSR_FOR_ALL_TC_ENTRIES(tc) {
      OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) {
        if (tc_edge->edge_inv) {
          ipc_print_tc_link(&outbuffer, tc, tc_edge);
        }
      }
      OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc, tc_edge);
    }
    OLSR_FOR_ALL_TC_ENTRIES_END(tc);

    /* HNA entries */
    OLSR_FOR_ALL_HNA_ENTRIES(tmp_hna) {

      /* Check all networks */
      for (tmp_net = tmp_hna->networks.next; tmp_net != &tmp_hna->networks; tmp_net = tmp_net->next) {
        ipc_print_net(&outbuffer, &tmp_hna->A_gateway_addr,
            &tmp_net->hna_prefix.prefix, tmp_net->hna_prefix.prefix_len);
      }
    }
    OLSR_FOR_ALL_HNA_ENTRIES_END(tmp_hna);

    /* Local HNA entries */
    for (hna = olsr_cnf->hna_entries; hna != NULL; hna = hna->next) {
      ipc_print_net(&outbuffer, &olsr_cnf->main_addr, &hna->net.prefix, hna->net.prefix_len);
    }
    abuf_puts(&outbuffer, "}\n\n");

    res = 1;
  }

  if (writetimer_entry == NULL) {
    writetimer_entry = olsr_start_timer(100, 0, OLSR_TIMER_PERIODIC, &dotdraw_write_data, NULL, 0);
  }

  return res;
}

static void
ipc_print_tc_link(struct autobuf *abuf, const struct tc_entry *entry, const struct tc_edge_entry *dst_entry)
{
  struct ipaddr_str strbuf1, strbuf2;
  struct lqtextbuffer lqbuffer;

  abuf_appendf(abuf, "\"%s\" -> \"%s\"[label=\"%s\"];\n", olsr_ip_to_string(&strbuf1, &entry->addr),
               olsr_ip_to_string(&strbuf2, &dst_entry->T_dest_addr), get_linkcost_text(dst_entry->cost, false, &lqbuffer));
}

static void
ipc_print_net(struct autobuf *abuf, const union olsr_ip_addr *gw, const union olsr_ip_addr *net, uint8_t prefixlen)
{
  struct ipaddr_str gwbuf, netbuf;

  abuf_appendf(abuf, "\"%s\" -> \"%s/%d\"[label=\"HNA\"];\n", olsr_ip_to_string(&gwbuf, gw), olsr_ip_to_string(&netbuf, net), prefixlen);

  abuf_appendf(abuf, "\"%s/%d\"[shape=diamond];\n", olsr_ip_to_string(&netbuf, net), prefixlen);
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
