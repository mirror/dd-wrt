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
 * $Id: ipc_frontend.c,v 1.27 2005/11/10 19:33:57 kattemat Exp $
 */

/*
 *
 *IPC - interprocess communication
 *for the OLSRD - GUI front-end
 *
 */

#include "ipc_frontend.h"
#include "link_set.h"
#include "olsr.h"
#include "log.h"
#include "parser.h"
#include "socket_parser.h"
#include "local_hna_set.h"

#ifdef WIN32
#define close(x) closesocket(x)
#define perror(x) WinSockPError(x)
void 
WinSockPError(char *);
#endif

#ifndef linux
#define MSG_NOSIGNAL 0
#endif

/**
 *Create the socket to use for IPC to the
 *GUI front-end
 *
 *@return the socket FD
 */
int
ipc_init()
{
  //int flags;
  struct   sockaddr_in sin;
  int yes = 1;

  /* Add parser function */
  olsr_parser_add_function(&frontend_msgparser, PROMISCUOUS, 0);

  /* get an internet domain socket */
  if ((ipc_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
    {
      perror("IPC socket");
      olsr_exit("IPC socket", EXIT_FAILURE);
    }

  if(setsockopt(ipc_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof(yes)) < 0) 
    {
      perror("SO_REUSEADDR failed");
      return 0;
    }

  /* complete the socket structure */
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(IPC_PORT);

  /* bind the socket to the port number */
  if(bind(ipc_sock, (struct sockaddr *) &sin, sizeof(sin)) == -1) 
    {
      perror("IPC bind");
      OLSR_PRINTF(1, "Will retry in 10 seconds...\n")
      sleep(10);
      if(bind(ipc_sock, (struct sockaddr *) &sin, sizeof(sin)) == -1) 
	{
	  perror("IPC bind");
	  olsr_exit("IPC bind", EXIT_FAILURE);
	}
      OLSR_PRINTF(1, "OK\n")
    }

  /* show that we are willing to listen */
  if(listen(ipc_sock, olsr_cnf->ipc_connections) == -1) 
    {
      perror("IPC listen");
      olsr_exit("IPC listen", EXIT_FAILURE);
    }

  /* Register the socket with the socket parser */
  add_olsr_socket(ipc_sock, &ipc_accept);

  return ipc_sock;
}


void
ipc_accept(int fd)
{
  socklen_t addrlen;
  struct sockaddr_in pin;
  char *addr;  


  addrlen = sizeof (struct sockaddr_in);
  
  if ((ipc_connection = accept(ipc_sock, (struct sockaddr *)  &pin, &addrlen)) == -1)
    {
      perror("IPC accept");
      olsr_exit("IPC accept", EXIT_FAILURE);
    }
  else
    {
      OLSR_PRINTF(1, "Front end connected\n")
      addr = inet_ntoa(pin.sin_addr);
      if(ipc_check_allowed_ip((union olsr_ip_addr *)&pin.sin_addr.s_addr))
	{
	  ipc_active = OLSR_TRUE;
	  ipc_send_net_info();
	  ipc_send_all_routes();
	  OLSR_PRINTF(1, "Connection from %s\n",addr)
	}
      else
	{
	  OLSR_PRINTF(1, "Front end-connection from foregin host(%s) not allowed!\n", addr)
	  olsr_syslog(OLSR_LOG_ERR, "OLSR: Front end-connection from foregin host(%s) not allowed!\n", addr);
	  close(ipc_connection);
	}
    }

}

olsr_bool
ipc_check_allowed_ip(union olsr_ip_addr *addr)
{
  struct ipc_host *ipch = olsr_cnf->ipc_hosts;
  struct ipc_net *ipcn = olsr_cnf->ipc_nets;

  if(addr->v4 == ntohl(INADDR_LOOPBACK))
    return OLSR_TRUE;

  /* check hosts */
  while(ipch)
    {
      if(addr->v4 == ipch->host.v4)
	return OLSR_TRUE;
      ipch = ipch->next;
    }

  /* check nets */
  while(ipcn)
    {
      if((addr->v4 & ipcn->mask.v4) == (ipcn->net.v4 & ipcn->mask.v4))
	return OLSR_TRUE;
      ipcn = ipcn->next;
    }

  return OLSR_FALSE;
}

/**
 *Read input from the IPC socket. Not in use.
 *
 *@todo for future use
 *@param sock the IPC socket
 *@return 1
 */
int
ipc_input(int sock)
{
  /*
  union 
  {
    char	buf[MAXPACKETSIZE+1];
    struct	olsr olsr;
  } inbuf;


  if (recv(sock, dir, sizeof(dir), 0) == -1) 
    {
      perror("recv");
      exit(1);
    }
*/
  return 1;
}


/**
 *Sends a olsr packet on the IPC socket.
 *
 *@param olsr the olsr struct representing the packet
 *
 *@return negative on error
 */
void
frontend_msgparser(union olsr_message *msg, struct interface *in_if, union olsr_ip_addr *from_addr)
{
  int size;

  if(!ipc_active)
    return;
  
  if(olsr_cnf->ip_version == AF_INET)
    size = ntohs(msg->v4.olsr_msgsize);
  else
    size = ntohs(msg->v6.olsr_msgsize);
  
  if (send(ipc_connection, (void *)msg, size, MSG_NOSIGNAL) < 0) 
    {
      OLSR_PRINTF(1, "(OUTPUT)IPC connection lost!\n")
      close(ipc_connection);
      //olsr_cnf->open_ipc = 0;
      ipc_active = OLSR_FALSE;
      return;
    }
  
  return;
}


/**
 *Send a route table update to the front-end.
 *
 *@param kernel_route a rtentry describing the route update
 *@param add 1 if the route is to be added 0 if it is to be deleted
 *@param int_name the name of the interface the route is set to go by
 *
 *@return negative on error
 */
int
ipc_route_send_rtentry(union olsr_ip_addr *dst, union olsr_ip_addr *gw, int met, int add, char *int_name)
{
  struct ipcmsg packet;
  //int i, x;
  char *tmp;

  if(!ipc_active)
    return 0;

  memset(&packet, 0, sizeof(struct ipcmsg));
  packet.size = htons(IPC_PACK_SIZE);
  packet.msgtype = ROUTE_IPC;

  COPY_IP(&packet.target_addr, dst);

  packet.add = add;
  if(add && gw)
    {
      packet.metric = met;
      COPY_IP(&packet.gateway_addr, gw);
    }

  if(int_name != NULL)
    memcpy(&packet.device[0], int_name, 4);
  else
    memset(&packet.device[0], 0, 4);


  tmp = (char *) &packet;
  /*
  x = 0;
  for(i = 0; i < IPC_PACK_SIZE;i++)
    {
      if(x == 4)
	{
	  x = 0;
	  printf("\n\t");
	}
      x++;
      printf(" %03i", (u_char) tmp[i]);
    }
  
  printf("\n");
  */
  
  if (send(ipc_connection, tmp, IPC_PACK_SIZE, MSG_NOSIGNAL) < 0) // MSG_NOSIGNAL to avoid sigpipe
    {
      OLSR_PRINTF(1, "(RT_ENTRY)IPC connection lost!\n")
      close(ipc_connection);
      //olsr_cnf->open_ipc = 0;
      ipc_active = OLSR_FALSE;
      return -1;
    }

  return 1;
}



int
ipc_send_all_routes()
{
  struct rt_entry  *destination;
  struct interface *ifn;
  olsr_u8_t        index;
  struct ipcmsg packet;
  char *tmp;
  

  if(!ipc_active)
    return 0;
  
  for(index=0;index<HASHSIZE;index++)
    {
      for(destination = routingtable[index].next;
	  destination != &routingtable[index];
	  destination = destination->next)
	{
	  ifn = destination->rt_if;
	  

	  memset(&packet, 0, sizeof(struct ipcmsg));
	  packet.size = htons(IPC_PACK_SIZE);
	  packet.msgtype = ROUTE_IPC;
	  
	  COPY_IP(&packet.target_addr, &destination->rt_dst);
	  
	  packet.add = 1;

	  if(olsr_cnf->ip_version == AF_INET)
	    {
	      packet.metric = (olsr_u8_t)(destination->rt_metric - 1);
	    }
	  else
	    {
	      packet.metric = (olsr_u8_t)destination->rt_metric;
	    }
	  COPY_IP(&packet.gateway_addr, &destination->rt_router);

	  if(ifn)
	    memcpy(&packet.device[0], ifn->int_name, 4);
	  else
	    memset(&packet.device[0], 0, 4);


	  tmp = (char *) &packet;
  
	  if (send(ipc_connection, tmp, IPC_PACK_SIZE, MSG_NOSIGNAL) < 0) // MSG_NOSIGNAL to avoid sigpipe
	    {
	      OLSR_PRINTF(1, "(RT_ENTRY)IPC connection lost!\n")
	      close(ipc_connection);
	      //olsr_cnf->open_ipc = 0;
	      ipc_active = OLSR_FALSE;
	      return -1;
	    }

	}
    }

  for(index=0;index<HASHSIZE;index++)
    {
      for(destination = hna_routes[index].next;
	  destination != &hna_routes[index];
	  destination = destination->next)
	{
	  ifn = destination->rt_if;

	  packet.size = htons(IPC_PACK_SIZE);
	  packet.msgtype = ROUTE_IPC;
	  
	  COPY_IP(&packet.target_addr, &destination->rt_dst);
	  
	  packet.add = 1;

	  if(olsr_cnf->ip_version == AF_INET)
	    {
	      packet.metric = (olsr_u8_t)(destination->rt_metric - 1);
	    }
	  else
	    {
	      packet.metric = (olsr_u8_t)destination->rt_metric;
	    }
	  COPY_IP(&packet.gateway_addr, &destination->rt_router);

	  if(ifn)
	    memcpy(&packet.device[0], ifn->int_name, 4);
	  else
	    memset(&packet.device[0], 0, 4);


	  tmp = (char *) &packet;
  
	  if (send(ipc_connection, tmp, IPC_PACK_SIZE, MSG_NOSIGNAL) < 0) // MSG_NOSIGNAL to avoid sigpipe
	    {
	      OLSR_PRINTF(1, "(RT_ENTRY)IPC connection lost!\n")
	      close(ipc_connection);
	      //olsr_cnf->open_ipc = 0;
	      ipc_active = OLSR_FALSE;
	      return -1;
	    }

	}
    }


  return 1;
}



/**
 *Sends OLSR info to the front-end. This info consists of
 *the different time intervals and holding times, number
 *of interfaces, HNA routes and main address.
 *
 *@return negative on error
 */
int
ipc_send_net_info()
{
  struct ipc_net_msg *net_msg;
  //int x, i;
  char *msg;
  

  net_msg = olsr_malloc(sizeof(struct ipc_net_msg), "send net info");

  msg = (char *)net_msg;

  OLSR_PRINTF(1, "Sending net-info to front end...\n")
  
  memset(net_msg, 0, sizeof(struct ipc_net_msg));
  
  /* Message size */
  net_msg->size = htons(sizeof(struct ipc_net_msg));
  /* Message type */
  net_msg->msgtype = NET_IPC;
  
  /* MIDs */
  /* XXX fix IPC MIDcnt */
  net_msg->mids = (ifnet != NULL && ifnet->int_next != NULL) ? 1 : 0;
  
  /* HNAs */
  if(olsr_cnf->ip_version == AF_INET6)
    {
      if(olsr_cnf->hna6_entries == NULL)
	net_msg->hnas = 0;
      else
	net_msg->hnas = 1;
    }

  if(olsr_cnf->ip_version == AF_INET)
    {
      if(olsr_cnf->hna4_entries == NULL)
	net_msg->hnas = 0;
      else
	net_msg->hnas = 1;
    }

  /* Different values */
  /* Temporary fixes */
  /* XXX fix IPC intervals */
  net_msg->hello_int = 0;//htons((olsr_u16_t)hello_int);
  net_msg->hello_lan_int = 0;//htons((olsr_u16_t)hello_int_nw);
  net_msg->tc_int = 0;//htons((olsr_u16_t)tc_int);
  net_msg->neigh_hold = 0;//htons((olsr_u16_t)neighbor_hold_time);
  net_msg->topology_hold = 0;//htons((olsr_u16_t)topology_hold_time);

  if(olsr_cnf->ip_version == AF_INET)
    net_msg->ipv6 = 0;
  else
    net_msg->ipv6 = 1;
 
  /* Main addr */
  COPY_IP(&net_msg->main_addr, &main_addr);


  /*
  printf("\t");
  x = 0;
  for(i = 0; i < sizeof(struct ipc_net_msg);i++)
    {
      if(x == 4)
	{
	  x = 0;
	  printf("\n\t");
	}
      x++;
      printf(" %03i", (u_char) msg[i]);
    }
  
  printf("\n");
  */


  if (send(ipc_connection, (char *)net_msg, sizeof(struct ipc_net_msg), MSG_NOSIGNAL) < 0) 
    {
      OLSR_PRINTF(1, "(NETINFO)IPC connection lost!\n")
      close(ipc_connection);
      //olsr_cnf->open_ipc = 0;
      return -1;
    }

  free(net_msg);
  return 0;
}



int
shutdown_ipc()
{
  OLSR_PRINTF(1, "Shutting down IPC...\n")
  close(ipc_sock);
  close(ipc_connection);
  
  return 1;
}
