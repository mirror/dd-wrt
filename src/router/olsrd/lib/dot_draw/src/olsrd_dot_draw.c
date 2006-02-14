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
 * $Id: olsrd_dot_draw.c,v 1.20 2005/12/30 02:23:59 tlopatic Exp $
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

#include "olsr.h"
#include "olsr_types.h"
#include "neighbor_table.h"
#include "two_hop_neighbor_table.h"
#include "tc_set.h"
#include "hna_set.h"
#include "mid_set.h"
#include "link_set.h"
#include "socket_parser.h"

#include "olsrd_dot_draw.h"
#include "olsrd_plugin.h"


#ifdef WIN32
#define close(x) closesocket(x)
#endif


static int ipc_socket;
static int ipc_open;
static int ipc_connection;
static int ipc_socket_up;



/* IPC initialization function */
static int
plugin_ipc_init(void);

static char*
olsr_netmask_to_string(union hna_netmask *mask);

/* Event function to register with the sceduler */
static int
pcf_event(int, int, int);

static void
ipc_action(int);

static void inline
ipc_print_neigh_link(struct neighbor_entry *neighbor);

static void inline
ipc_print_tc_link(struct tc_entry *entry, struct topo_dst *dst_entry);

static void inline
ipc_print_net(union olsr_ip_addr *, union olsr_ip_addr *, union hna_netmask *);

static int
ipc_send(char *, int);

static int
ipc_send_str(char *);

static double 
calc_etx(double, double);

static void inline
ipc_print_neigh_link(struct neighbor_entry *);


/**
 *Do initialization here
 *
 *This function is called by the my_init
 *function in uolsrd_plugin.c
 */
int
olsrd_plugin_init()
{
  /* Initial IPC value */
  ipc_open = 0;
  ipc_socket_up = 0;

  /* Register the "ProcessChanges" function */
  register_pcf(&pcf_event);

  return 1;
}


/**
 * destructor - called at unload
 */
void
olsr_plugin_exit()
{
  if(ipc_open)
    close(ipc_socket);
}


static void inline
ipc_print_neigh_link(struct neighbor_entry *neighbor)
{
  char buf[256];
  char* adr;
  double etx=0.0;
  char* style = "solid";
  struct link_entry* link;
  adr = olsr_ip_to_string(&main_addr);
  sprintf( buf, "\"%s\" -> ", adr );
  ipc_send_str(buf);
  
  adr = olsr_ip_to_string(&neighbor->neighbor_main_addr);
  
  if (neighbor->status == 0) { // non SYM
  	style = "dashed";
  }
  else {   
      link = get_best_link_to_neighbor(&neighbor->neighbor_main_addr);
      if (link) {
        etx = calc_etx( link->loss_link_quality, link->neigh_link_quality);
      }
  }
    
  sprintf( buf, "\"%s\"[label=\"%.2f\", style=%s];\n", adr, etx, style );
  ipc_send_str(buf);
  
   if (neighbor->is_mpr) {
	sprintf( buf, "\"%s\"[shape=box];\n", adr );
  	ipc_send_str(buf);
  }
}


static int
plugin_ipc_init()
{
  struct sockaddr_in sin;
  olsr_u32_t yes = 1;

  /* Init ipc socket */
  if ((ipc_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
    {
      olsr_printf(1, "(DOT DRAW)IPC socket %s\n", strerror(errno));
      return 0;
    }
  else
    {
      if (setsockopt(ipc_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof(yes)) < 0) 
      {
	perror("SO_REUSEADDR failed");
	return 0;
      }

#if defined __FreeBSD__ && defined SO_NOSIGPIPE
      if (setsockopt(ipc_socket, SOL_SOCKET, SO_NOSIGPIPE, (char *)&yes, sizeof(yes)) < 0) 
      {
	perror("SO_REUSEADDR failed");
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
      if (bind(ipc_socket, (struct sockaddr *) &sin, sizeof(sin)) == -1) 
	{
	  olsr_printf(1, "(DOT DRAW)IPC bind %s\n", strerror(errno));
	  return 0;
	}
      
      /* show that we are willing to listen */
      if (listen(ipc_socket, 1) == -1) 
	{
	  olsr_printf(1, "(DOT DRAW)IPC listen %s\n", strerror(errno));
	  return 0;
	}


      /* Register with olsrd */
      add_olsr_socket(ipc_socket, &ipc_action);
      ipc_socket_up = 1;
    }

  return 1;
}


static void
ipc_action(int fd)
{
  struct sockaddr_in pin;
  socklen_t addrlen;
  char *addr;  

  addrlen = sizeof(struct sockaddr_in);

  if (ipc_open)
    {
      while(close(ipc_connection) == -1) 
        {
          olsr_printf(1, "(DOT DRAW) Error on closing previously active TCP connection on fd %d: %s\n", ipc_connection, strerror(errno));
          if (errno != EINTR)
            {
	      break;
            }
        }
      ipc_open = 0;
    }
  
  if ((ipc_connection = accept(ipc_socket, (struct sockaddr *)  &pin, &addrlen)) == -1)
    {
      olsr_printf(1, "(DOT DRAW)IPC accept: %s\n", strerror(errno));
      exit(1);
    }
  else
    {
      addr = inet_ntoa(pin.sin_addr);
      if(ntohl(pin.sin_addr.s_addr) != ntohl(ipc_accept_ip.s_addr))
	{
	  olsr_printf(1, "Front end-connection from foregin host(%s) not allowed!\n", addr);
	  close(ipc_connection);
	  return;
	}
      else
	{
	  ipc_open = 1;
	  olsr_printf(1, "(DOT DRAW)IPC: Connection from %s\n",addr);
	  pcf_event(1, 1, 1);
	}
    }
}


/**
 *Scheduled event
 */
static int
pcf_event(int changes_neighborhood,
	  int changes_topology,
	  int changes_hna)
{
  int res;
  olsr_u8_t index;
  struct neighbor_entry *neighbor_table_tmp;
  struct tc_entry *entry;
  struct topo_dst *dst_entry;
  struct hna_entry *tmp_hna;
  struct hna_net *tmp_net;

  res = 0;

  if(changes_neighborhood || changes_topology || changes_hna)
    {
      /* Print tables to IPC socket */

      ipc_send_str("digraph topology\n{\n");

      /* Neighbors */
      for(index=0;index<HASHSIZE;index++)
	{
	  
	  for(neighbor_table_tmp = neighbortable[index].next;
	      neighbor_table_tmp != &neighbortable[index];
	      neighbor_table_tmp = neighbor_table_tmp->next)
	    {
	      ipc_print_neigh_link( neighbor_table_tmp );
	    }
	}

      /* Topology */  
      for(index=0;index<HASHSIZE;index++)
	{
	  /* For all TC entries */
	  entry = tc_table[index].next;
	  while(entry != &tc_table[index])
	    {
	      /* For all destination entries of that TC entry */
	      dst_entry = entry->destinations.next;
	      while(dst_entry != &entry->destinations)
		{
		  ipc_print_tc_link(entry, dst_entry);
		  dst_entry = dst_entry->next;
		}
	      entry = entry->next;
	    }
	}

      /* HNA entries */
      for(index=0;index<HASHSIZE;index++)
	{
	  tmp_hna = hna_set[index].next;
	  /* Check all entrys */
	  while(tmp_hna != &hna_set[index])
	    {
	      /* Check all networks */
	      tmp_net = tmp_hna->networks.next;
	      
	      while(tmp_net != &tmp_hna->networks)
		{
		  ipc_print_net(&tmp_hna->A_gateway_addr, 
				&tmp_net->A_network_addr, 
				&tmp_net->A_netmask);
		  tmp_net = tmp_net->next;
		}
	      
	      tmp_hna = tmp_hna->next;
	    }
	}


      ipc_send_str("}\n\n");

      res = 1;
    }


  if(!ipc_socket_up)
    plugin_ipc_init();

  return res;
}


#define MIN_LINK_QUALITY 0.01
static double 
calc_etx(double loss, double neigh_loss) 
{
  if (loss < MIN_LINK_QUALITY || neigh_loss < MIN_LINK_QUALITY)
    return 0.0;
  else
    return 1.0 / (loss * neigh_loss);
}


static void inline
ipc_print_tc_link(struct tc_entry *entry, struct topo_dst *dst_entry)
{
  char buf[256];
  char* adr;
  double etx = calc_etx( dst_entry->link_quality, dst_entry->inverse_link_quality );

  adr = olsr_ip_to_string(&entry->T_last_addr);
  sprintf( buf, "\"%s\" -> ", adr );
  ipc_send_str(buf);
  
  adr = olsr_ip_to_string(&dst_entry->T_dest_addr);
  sprintf( buf, "\"%s\"[label=\"%.2f\"];\n", adr, etx );
  ipc_send_str(buf);
}


static void inline
ipc_print_net(union olsr_ip_addr *gw, union olsr_ip_addr *net, union hna_netmask *mask)
{
  char *adr;

  adr = olsr_ip_to_string(gw);
  ipc_send_str("\"");
  ipc_send_str(adr);
  ipc_send_str("\" -> \"");
  adr = olsr_ip_to_string(net);
  ipc_send_str(adr);
  ipc_send_str("/");
  adr = olsr_netmask_to_string(mask);
  ipc_send_str(adr);
  ipc_send_str("\"[label=\"HNA\"];\n");
  ipc_send_str("\"");
  adr = olsr_ip_to_string(net);
  ipc_send_str(adr);
  ipc_send_str("/");
  adr = olsr_netmask_to_string(mask);
  ipc_send_str(adr);
  ipc_send_str("\"");
  ipc_send_str("[shape=diamond];\n");
}

static int
ipc_send_str(char *data)
{
  if(!ipc_open)
    return 0;
  return ipc_send(data, strlen(data));
}


static int
ipc_send(char *data, int size)
{
  if(!ipc_open)
    return 0;

#if defined __FreeBSD__ || defined __NetBSD__ || defined __OpenBSD__ || defined __MacOSX__
  if (send(ipc_connection, data, size, 0) < 0) 
#else
  if (send(ipc_connection, data, size, MSG_NOSIGNAL) < 0) 
#endif
    {
      olsr_printf(1, "(DOT DRAW)IPC connection lost!\n");
      close(ipc_connection);
      ipc_open = 0;
      return -1;
    }

  return 1;
}

static char*
olsr_netmask_to_string(union hna_netmask *mask)
{
  char *ret;
  struct in_addr in;

  if(olsr_cnf->ip_version == AF_INET)
    {
      in.s_addr = mask->v4;
      ret = inet_ntoa(in);
    }
  else
    {
      /* IPv6 */
      static char netmask[5];
      sprintf(netmask, "%d", mask->v6);
      ret = netmask;
    }

  return ret;
}
