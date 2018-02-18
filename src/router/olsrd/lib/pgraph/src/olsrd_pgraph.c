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

#include "olsrd_pgraph.h"
#include "ipcalc.h"
#include "olsrd_plugin.h"
#include "plugin_util.h"
#include "net_olsr.h"
#include "olsr.h"
#include "builddata.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#ifdef _WIN32
#define close(x) closesocket(x)
#endif /* _WIN32 */

#define PLUGIN_NAME              "OLSRD pgraph plugin"
#define PLUGIN_INTERFACE_VERSION 5

static union olsr_ip_addr ipc_accept_ip;
static int ipc_port;

static int ipc_socket;
static int ipc_connection;

void my_init(void) __attribute__ ((constructor));

void my_fini(void) __attribute__ ((destructor));

/*
 * Defines the version of the plugin interface that is used
 * THIS IS NOT THE VERSION OF YOUR PLUGIN!
 * Do not alter unless you know what you are doing!
 */
int
olsrd_plugin_interface_version(void)
{
  return PLUGIN_INTERFACE_VERSION;
}

/**
 *Constructor
 */
void
my_init(void)
{
  /* Print plugin info to stdout */
  olsr_printf(0, "%s (%s)\n", PLUGIN_NAME, git_descriptor);

  /* defaults for parameters */
  ipc_port = 2004;
  if (olsr_cnf->ip_version == AF_INET) {
    ipc_accept_ip.v4.s_addr = htonl(INADDR_LOOPBACK);
  } else {
    ipc_accept_ip.v6 = in6addr_loopback;
  }
  ipc_socket = -1;
  ipc_connection = -1;
}

/**
 *Destructor
 */
void
my_fini(void)
{
  if (ipc_socket >= 0) {
    close(ipc_socket);
    ipc_socket = -1;
  }
  if (ipc_connection >= 0) {
    close(ipc_connection);
    ipc_connection = -1;
  }

}

static const struct olsrd_plugin_parameters plugin_parameters[] = {
  {.name = "port",.set_plugin_parameter = &set_plugin_port,.data = &ipc_port},
  {.name = "accept",.set_plugin_parameter = &set_plugin_ipaddress,.data = &ipc_accept_ip},
};

void
olsrd_get_plugin_parameters(const struct olsrd_plugin_parameters **params, int *size)
{
  *params = plugin_parameters;
  *size = sizeof(plugin_parameters) / sizeof(*plugin_parameters);
}

/* Event function to register with the sceduler */
static int pcf_event(int, int, int);

static void ipc_action(int, void *, unsigned int);

static void ipc_print_neigh_link(struct neighbor_entry *neighbor);

static void ipc_print_tc_link(struct tc_entry *entry, struct tc_edge_entry *dst_entry);

static int ipc_send(const char *, int);

static void ipc_print_neigh_link(struct neighbor_entry *);

static int plugin_ipc_init(void);

static void
ipc_print_neigh_link(struct neighbor_entry *neighbor)
{
  char buf[256];
  int len;
  struct ipaddr_str main_adr, adr;
//  double etx=0.0;
//  char* style = "solid";
//  struct link_entry* link;

  len =
    sprintf(buf, "add link %s %s\n", olsr_ip_to_string(&main_adr, &olsr_cnf->main_addr),
            olsr_ip_to_string(&adr, &neighbor->neighbor_main_addr));
  ipc_send(buf, len);

//  if (neighbor->status == 0) { // non SYM
//      style = "dashed";
//  }
//  else {
  /* find best link to neighbor for the ETX */
  //? why cant i just get it one time at fetch_olsrd_data??? (br1)
//    if(olsr_plugin_io(GETD__LINK_SET, &link, sizeof(link)) && link)
//    {
//      link_set = link; // for olsr_neighbor_best_link
//      link = olsr_neighbor_best_link(&neighbor->neighbor_main_addr);
//      if (link) {
//        etx = olsr_calc_etx(link);
//      }
//    }
//  }

  //len = sprintf( buf, "\"%s\"[label=\"%.2f\", style=%s];\n", adr, etx, style );
  //len = sprintf( buf, "%s\n", adr );
  //ipc_send(buf, len);

  //if (neighbor->is_mpr) {
  //   len = sprintf( buf, "\"%s\"[shape=box];\n", adr );
  //   ipc_send(buf, len);
  //}
}

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

  /* Register the "ProcessChanges" function */
  register_pcf(&pcf_event);

  return 1;
}

static int
plugin_ipc_init(void)
{
  struct sockaddr_in sin;
  uint32_t yes = 1;

  /* Init ipc socket */
  if ((ipc_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    olsr_printf(1, "(DOT DRAW)IPC socket %s\n", strerror(errno));
    return 0;
  } else {
    if (setsockopt(ipc_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof(yes)) < 0) {
      perror("SO_REUSEADDR failed");
      return 0;
    }
#if defined __FreeBSD__
    if (setsockopt(ipc_socket, SOL_SOCKET, SO_NOSIGPIPE, (char *)&yes, sizeof(yes)) < 0) {
      perror("SO_NOSIGPIPE failed");
      return 0;
    }
#endif /* defined __FreeBSD__ */

    /* Bind the socket */

    /* complete the socket structure */
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(ipc_port);

    /* bind the socket to the port number */
    if (bind(ipc_socket, (struct sockaddr *)&sin, sizeof(sin)) == -1) {
      olsr_printf(1, "(DOT DRAW)IPC bind %s\n", strerror(errno));
      return 0;
    }

    /* show that we are willing to listen */
    if (listen(ipc_socket, 1) == -1) {
      olsr_printf(1, "(DOT DRAW)IPC listen %s\n", strerror(errno));
      return 0;
    }

    /* Register with olsrd */
    add_olsr_socket(ipc_socket, &ipc_action, NULL, NULL, SP_PR_READ);
  }

  return 1;
}

static void
ipc_action(int fd __attribute__ ((unused)), void *data __attribute__ ((unused)), unsigned int flags __attribute__ ((unused)))
{
  struct sockaddr_in pin;
  socklen_t addrlen;
  char *addr;
  char buf[256];
  int len;

  addrlen = sizeof(struct sockaddr_in);

  if ((ipc_connection = accept(ipc_socket, (struct sockaddr *)&pin, &addrlen)) == -1) {
    char buf2[1024];
    snprintf(buf2, sizeof(buf2), "(DOT DRAW)IPC accept error: %s", strerror(errno));
    olsr_exit(buf2, EXIT_FAILURE);
  } else {
    struct ipaddr_str main_addr;
    addr = inet_ntoa(pin.sin_addr);

/*
      if(ntohl(pin.sin_addr.s_addr) != ntohl(ipc_accept_ip.s_addr))
	{
	  olsr_printf(1, "Front end-connection from foregin host(%s) not allowed!\n", addr);
	  close(ipc_connection);
	  return;
	}
      else
	{
*/
    olsr_printf(1, "(DOT DRAW)IPC: Connection from %s\n", addr);
    len = sprintf(buf, "add node %s\n", olsr_ip_to_string(&main_addr, &olsr_cnf->main_addr));
    ipc_send(buf, len);
    pcf_event(1, 1, 1);
//      }
  }

}

/**
 *Scheduled event
 */
static int
pcf_event(int my_changes_neighborhood, int my_changes_topology, int my_changes_hna __attribute__ ((unused)))
{
  int res;
  struct neighbor_entry *neighbor_table_tmp;
  struct tc_entry *tc;
  struct tc_edge_entry *tc_edge;

  res = 0;

  if (my_changes_neighborhood || my_changes_topology) {
    /* Print tables to IPC socket */

    //ipc_send("start ", strlen("start "));

    /* Neighbors */
    OLSR_FOR_ALL_NBR_ENTRIES(neighbor_table_tmp) {
      ipc_print_neigh_link(neighbor_table_tmp);
    }
    OLSR_FOR_ALL_NBR_ENTRIES_END(neighbor_table_tmp);

    /* Topology */
    OLSR_FOR_ALL_TC_ENTRIES(tc) {
      OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) {
        ipc_print_tc_link(tc, tc_edge);
      }
      OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc, tc_edge);
    }
    OLSR_FOR_ALL_TC_ENTRIES_END(tc);

    ipc_send(" end ", strlen(" end "));

    /* HNA entries */
//      for(index=0;index<HASHSIZE;index++)
//      {
//        tmp_hna = hna_set[index].next;
//        /* Check all entrys */
//        while(tmp_hna != &hna_set[index])
//          {
//            /* Check all networks */
//            tmp_net = tmp_hna->networks.next;
//
//            while(tmp_net != &tmp_hna->networks)
//              {
//                ipc_print_net(&tmp_hna->A_gateway_addr,
//                              &tmp_net->A_network_addr,
//                              &tmp_net->A_netmask);
//                tmp_net = tmp_net->next;
//              }
//
//            tmp_hna = tmp_hna->next;
//          }
//      }

//      ipc_send("}\n\n", strlen("}\n\n"));

    res = 1;
  }

  if (ipc_socket == -1) {
    plugin_ipc_init();
  }

  return res;
}

static void
ipc_print_tc_link(struct tc_entry *entry, struct tc_edge_entry *dst_entry)
{
  char buf[256];
  int len;
  struct ipaddr_str main_adr, adr;
//  double etx = olsr_calc_tc_etx(dst_entry);

  len =
    sprintf(buf, "add link %s %s\n", olsr_ip_to_string(&main_adr, &entry->addr), olsr_ip_to_string(&adr, &dst_entry->T_dest_addr));
  ipc_send(buf, len);
}

static int
ipc_send(const char *data, int size)
{
  if (ipc_connection == -1)
    return 0;

#if defined __FreeBSD__ || defined __FreeBSD_kernel__ || defined __APPLE__ || defined __OpenBSD__
#define FLAG 0
#else /* defined __FreeBSD__ || defined __FreeBSD_kernel__ || defined __APPLE__ || defined __OpenBSD__ */
#define FLAG MSG_NOSIGNAL
#endif /* defined __FreeBSD__ || defined __FreeBSD_kernel__ || defined __APPLE__ || defined __OpenBSD__ */
  if (send(ipc_connection, data, size, FLAG) < 0) {
    olsr_printf(1, "(DOT DRAW)IPC connection lost!\n");
    close(ipc_connection);
    ipc_connection = -1;
    return -1;
  }

  return 1;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
