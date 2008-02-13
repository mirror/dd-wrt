/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
 *                     includes code by Bruno Randolf
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

#include "olsr.h"
#include "ipcalc.h"
#include "olsr_types.h"
#include "neighbor_table.h"
#include "two_hop_neighbor_table.h"
#include "tc_set.h"
#include "hna_set.h"
#include "mid_set.h"
#include "link_set.h"
#include "socket_parser.h"
#include "net_olsr.h"

#include "olsrd_dot_draw.h"
#include "olsrd_plugin.h"


#ifdef WIN32
#define close(x) closesocket(x)
#endif


static int ipc_socket;
static int ipc_connection;



/* IPC initialization function */
static int
plugin_ipc_init(void);

/* Event function to register with the sceduler */
static int
pcf_event(int, int, int);

static void
ipc_action(int);

static void
ipc_print_neigh_link(const struct neighbor_entry *neighbor);

static void
ipc_print_tc_link(const struct tc_entry *entry, const struct tc_edge_entry *dst_entry);

static void
ipc_print_net(const union olsr_ip_addr *, const union olsr_ip_addr *, olsr_u8_t);

static void
ipc_send(const char *, int);

static void
ipc_send_fmt(const char *format, ...) __attribute__((format(printf,1,2)));

#define ipc_send_str(data) ipc_send((data), strlen(data))


/**
 *Do initialization here
 *
 *This function is called by the my_init
 *function in uolsrd_plugin.c
 */
int
olsrd_plugin_init(void)
{
  /* Initial IPC value */
  ipc_socket = -1;
  ipc_connection = -1;

  /* Register the "ProcessChanges" function */
  register_pcf(&pcf_event);

  plugin_ipc_init();

  return 1;
}


/**
 * destructor - called at unload
 */
void
olsr_plugin_exit(void)
{
  if (ipc_connection != -1) {
    CLOSE(ipc_connection);
  }
  if (ipc_socket != -1) {
    CLOSE(ipc_socket);
  }
}


static void
ipc_print_neigh_link(const struct neighbor_entry *neighbor)
{
  struct ipaddr_str mainaddrstrbuf, strbuf;
  double etx = 0.0;
  const char *style;
  const char *adr = olsr_ip_to_string(&mainaddrstrbuf, &olsr_cnf->main_addr);
  struct link_entry* link;

  if (neighbor->status == 0) { // non SYM
    style = "dashed";
  } else {   
    link = get_best_link_to_neighbor(&neighbor->neighbor_main_addr);
    if (link) {
      etx = olsr_calc_link_etx(link);
    }
    style = "solid";
  }
    
  ipc_send_fmt("\"%s\" -> \"%s\"[label=\"%.2f\", style=%s];\n",
               adr,
               olsr_ip_to_string(&strbuf, &neighbor->neighbor_main_addr),
               etx,
               style);
  
  if (neighbor->is_mpr) {
    ipc_send_fmt("\"%s\"[shape=box];\n", adr);
  }
}


static int
plugin_ipc_init(void)
{
  struct sockaddr_in sin;
  olsr_u32_t yes = 1;

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

#if defined __FreeBSD__ && defined SO_NOSIGPIPE
  if (setsockopt(ipc_socket, SOL_SOCKET, SO_NOSIGPIPE, (char *)&yes, sizeof(yes)) < 0) {
    perror("SO_REUSEADDR failed");
    CLOSE(ipc_socket);
    return 0;
  }
#endif

  /* Bind the socket */
      
  /* complete the socket structure */
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(ipc_port);
      
  /* bind the socket to the port number */
  if (bind(ipc_socket, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
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
  //printf("Adding socket with olsrd\n");
  add_olsr_socket(ipc_socket, &ipc_action);

  return 1;
}


static void
ipc_action(int fd __attribute__((unused)))
{
  struct sockaddr_in pin;
  socklen_t addrlen = sizeof(struct sockaddr_in);

  if (ipc_connection != -1) {
    close(ipc_connection);
  }
  
  ipc_connection = accept(ipc_socket, (struct sockaddr *)&pin, &addrlen);
  if (ipc_connection == -1) {
    olsr_printf(1, "(DOT DRAW)IPC accept: %s\n", strerror(errno));
    return;
  }
  if (!ip4equal(&pin.sin_addr, &ipc_accept_ip.v4)) {
    olsr_printf(0, "Front end-connection from foreign host (%s) not allowed!\n", inet_ntoa(pin.sin_addr));
    CLOSE(ipc_connection);
    return;
  }
  olsr_printf(1, "(DOT DRAW)IPC: Connection from %s\n", inet_ntoa(pin.sin_addr));
  pcf_event(1, 1, 1);
}


/**
 *Scheduled event
 */
static int
pcf_event(int changes_neighborhood,
	  int changes_topology,
	  int changes_hna)
{
  int res = 0;
  if(changes_neighborhood || changes_topology || changes_hna) {
    struct neighbor_entry *neighbor_table_tmp;
    struct tc_entry *tc;
    struct tc_edge_entry *tc_edge;
    struct ip_prefix_list *hna;
    int idx;
    
    /* Print tables to IPC socket */
    ipc_send_str("digraph topology\n{\n");

    /* Neighbors */
    for (idx = 0; idx < HASHSIZE; idx++) {	  
      for(neighbor_table_tmp = neighbortable[idx].next;
          neighbor_table_tmp != &neighbortable[idx];
          neighbor_table_tmp = neighbor_table_tmp->next){
        ipc_print_neigh_link( neighbor_table_tmp );
      }
    }

    /* Topology */  
    OLSR_FOR_ALL_TC_ENTRIES(tc) {
      OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) {
        ipc_print_tc_link(tc, tc_edge);
      } OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc, tc_edge);
    } OLSR_FOR_ALL_TC_ENTRIES_END(tc);

    /* HNA entries */
    for (idx = 0; idx < HASHSIZE; idx++) {
      struct hna_entry *tmp_hna;
      /* Check all entrys */
      for (tmp_hna = hna_set[idx].next; tmp_hna != &hna_set[idx]; tmp_hna = tmp_hna->next) {
        /* Check all networks */
        struct hna_net *tmp_net;
        for (tmp_net = tmp_hna->networks.next; tmp_net != &tmp_hna->networks; tmp_net = tmp_net->next) {
          ipc_print_net(&tmp_hna->A_gateway_addr, 
                        &tmp_net->A_network_addr, 
                        tmp_net->prefixlen);
        }
      }
    }

    /* Local HNA entries */
    for (hna = olsr_cnf->hna_entries; hna != NULL; hna = hna->next) {
      ipc_print_net(&olsr_cnf->main_addr,
                    &hna->net.prefix,
                    hna->net.prefix_len);
    }
    ipc_send_str("}\n\n");

    res = 1;
  }

  if (ipc_socket == -1) {
    plugin_ipc_init();
  }
  return res;
}

static void
ipc_print_tc_link(const struct tc_entry *entry, const struct tc_edge_entry *dst_entry)
{
  struct ipaddr_str strbuf1, strbuf2;

  ipc_send_fmt("\"%s\" -> \"%s\"[label=\"%.2f\"];\n",
               olsr_ip_to_string(&strbuf1, &entry->addr),
               olsr_ip_to_string(&strbuf2, &dst_entry->T_dest_addr),
               olsr_calc_tc_etx(dst_entry));
}


static void
ipc_print_net(const union olsr_ip_addr *gw, const union olsr_ip_addr *net, olsr_u8_t prefixlen)
{
  struct ipaddr_str gwbuf, netbuf;

  ipc_send_fmt("\"%s\" -> \"%s/%d\"[label=\"HNA\"];\n",
               olsr_ip_to_string(&gwbuf, gw),
               olsr_ip_to_string(&netbuf, net),
               prefixlen);

  ipc_send_fmt("\"%s/%d\"[shape=diamond];\n",
               olsr_ip_to_string(&netbuf, net),
               prefixlen);
}

static void
ipc_send(const char *data, int size)
{
  if (ipc_connection != -1) {
#if defined __FreeBSD__ || defined __NetBSD__ || defined __OpenBSD__ || defined __MacOSX__
#define FLAGS 0
#else
#define FLAGS MSG_NOSIGNAL
#endif
    if (send(ipc_connection, data, size, FLAGS) == -1) {
      olsr_printf(1, "(DOT DRAW)IPC connection lost!\n");
      CLOSE(ipc_connection);
    }
  }
}

static void
ipc_send_fmt(const char *format, ...)
{
  if (ipc_connection != -1) {
    char buf[4096];
    int len;
    va_list arg;
    va_start(arg, format);
    len = vsnprintf(buf, sizeof(buf), format, arg);
    va_end(arg);
    ipc_send(buf, len);
  }
}
