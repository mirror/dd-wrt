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
 * $Id: olsrd_pgraph.c,v 1.2 2005/12/29 19:48:43 tlopatic Exp $
 */

/*
 * Dynamic linked library for the olsr.org olsr daemon
 */

#include "olsrd_pgraph.h"
#include "socket_parser.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#ifdef WIN32
#define close(x) closesocket(x)
#endif

int ipc_socket;
int ipc_open;
int ipc_connection;
int ipc_socket_up;

static void inline
ipc_print_neigh_link(struct neighbor_entry *);

int
plugin_ipc_init(void);

void
olsr_plugin_exit(void);


static void inline
ipc_print_neigh_link(struct neighbor_entry *neighbor)
{
  char buf[256];
  int len;
  char* main_adr;
  char* adr;
//  double etx=0.0;
//  char* style = "solid";
//  struct link_entry* link;

  main_adr = olsr_ip_to_string(&main_addr);
  adr = olsr_ip_to_string(&neighbor->neighbor_main_addr);
  len = sprintf( buf, "add link %s %s\n", main_adr, adr );
  ipc_send(buf, len);
  
//  if (neighbor->status == 0) { // non SYM
//  	style = "dashed";
//  }
//  else {
    /* find best link to neighbor for the ETX */
    //? why cant i just get it one time at fetch_olsrd_data??? (br1)
//    if(olsr_plugin_io(GETD__LINK_SET, &link, sizeof(link)) && link)
//    {
//      link_set = link; // for olsr_neighbor_best_link    
//      link = olsr_neighbor_best_link(&neighbor->neighbor_main_addr);
//      if (link) {
//        etx = calc_etx( link->loss_link_quality, link->neigh_link_quality);
//      }
//    }
//  }
    
  //len = sprintf( buf, "\"%s\"[label=\"%.2f\", style=%s];\n", adr, etx, style );
  //len = sprintf( buf, "%s\n", adr );
  //ipc_send(buf, len);
  
   //if (neighbor->is_mpr) {
   //	len = sprintf( buf, "\"%s\"[shape=box];\n", adr );
   //	ipc_send(buf, len);
   //}
}

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

int
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

#ifdef __FreeBSD__
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

void
ipc_action(int fd)
{
  struct sockaddr_in pin;
  socklen_t addrlen;
  char *addr;  
  char buf[256] ;
  int len ;
   
  addrlen = sizeof(struct sockaddr_in);

  if ((ipc_connection = accept(ipc_socket, (struct sockaddr *)  &pin, &addrlen)) == -1)
    {
      olsr_printf(1, "(DOT DRAW)IPC accept: %s\n", strerror(errno));
      exit(1);
    }
  else
    {
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
	  ipc_open = 1;
	  olsr_printf(1, "(DOT DRAW)IPC: Connection from %s\n",addr);
          len = sprintf(buf, "add node %s\n", olsr_ip_to_string(&main_addr));
  	  ipc_send(buf, len);
	  pcf_event(1, 1, 1);
//	}
    }

}

/*
 * destructor - called at unload
 */
void
olsr_plugin_exit()
{
  if(ipc_open)
    close(ipc_socket);
}



/**
 *Scheduled event
 */
int
pcf_event(int changes_neighborhood,
	  int changes_topology,
	  int changes_hna)
{
  int res;
  olsr_u8_t index;
  struct neighbor_entry *neighbor_table_tmp;
  struct tc_entry *entry;
  struct topo_dst *dst_entry;

  res = 0;

  //if(changes_neighborhood || changes_topology || changes_hna)
  if(changes_neighborhood || changes_topology)
    {
      /* Print tables to IPC socket */

      //ipc_send("start ", strlen("start "));

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

      ipc_send(" end ", strlen(" end "));

      /* HNA entries */
//      for(index=0;index<HASHSIZE;index++)
//	{
//	  tmp_hna = hna_set[index].next;
//	  /* Check all entrys */
//	  while(tmp_hna != &hna_set[index])
//	    {
//	      /* Check all networks */
//	      tmp_net = tmp_hna->networks.next;
//	      
//	      while(tmp_net != &tmp_hna->networks)
//		{
//		  ipc_print_net(&tmp_hna->A_gateway_addr, 
//				&tmp_net->A_network_addr, 
//				&tmp_net->A_netmask);
//		  tmp_net = tmp_net->next;
//		}
//	      
//	      tmp_hna = tmp_hna->next;
//	    }
//	}

//      ipc_send("}\n\n", strlen("}\n\n"));

      res = 1;
    }


  if(!ipc_socket_up)
    plugin_ipc_init();

  return res;
}

#if 0
#define MIN_LINK_QUALITY 0.01
static double 
calc_etx(double loss, double neigh_loss) 
{
  if (loss < MIN_LINK_QUALITY || neigh_loss < MIN_LINK_QUALITY)
    return 0.0;
  else
    return 1.0 / (loss * neigh_loss);
}
#endif

static void inline
ipc_print_tc_link(struct tc_entry *entry, struct topo_dst *dst_entry)
{
  char buf[256];
  int len;
  char* main_adr;
  char* adr;
//  double etx = calc_etx( dst_entry->link_quality, dst_entry->inverse_link_quality );

  main_adr = olsr_ip_to_string(&entry->T_last_addr);
  adr = olsr_ip_to_string(&dst_entry->T_dest_addr);
  len = sprintf( buf, "add link %s %s\n", main_adr, adr );
  ipc_send(buf, len);
}

static void inline
ipc_print_net(union olsr_ip_addr *gw, union olsr_ip_addr *net, union hna_netmask *mask)
{
  char *adr;

  adr = olsr_ip_to_string(gw);
  ipc_send("\"", 1);
  ipc_send(adr, strlen(adr));
  ipc_send("\" -> \"", strlen("\" -> \""));
  adr = olsr_ip_to_string(net);
  ipc_send(adr, strlen(adr));
  ipc_send("/", 1);
  adr = olsr_netmask_to_string(mask);
  ipc_send(adr, strlen(adr));
  ipc_send("\"[label=\"HNA\"];\n", strlen("\"[label=\"HNA\"];\n"));
  ipc_send("\"", 1);
  adr = olsr_ip_to_string(net);
  ipc_send(adr, strlen(adr));
  ipc_send("/", 1);
  adr = olsr_netmask_to_string(mask);
  ipc_send(adr, strlen(adr));
  ipc_send("\"", 1);
  ipc_send("[shape=diamond];\n", strlen("[shape=diamond];\n"));
}



int
ipc_send(char *data, int size)
{
  if(!ipc_open)
    return 0;

#if defined __FreeBSD__ || defined __MacOSX__
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


#define COMP_IP(ip1, ip2) (!memcmp(ip1, ip2, ipsize))
struct link_entry *olsr_neighbor_best_link(union olsr_ip_addr *main)
{
  struct link_entry *walker;
  double best = 0.0;
  double curr;
  struct link_entry *res = NULL;

  // loop through all links

  for (walker = link_set; walker != NULL; walker = walker->next)
  {
    // check whether it's a link to the requested neighbor and
    // whether the link's quality is better than what we have
    if(COMP_IP(main, &walker->neighbor->neighbor_main_addr))
    {
      curr = walker->loss_link_quality * walker->neigh_link_quality;

      if (curr >= best)
      {
        best = curr;
        res = walker;
      }
    }
  }

  return res;
}

