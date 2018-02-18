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
#include "scheduler.h"
#include "net_olsr.h"
#include "ipcalc.h"

#ifdef _WIN32
#define close(x) closesocket(x)
#define perror(x) WinSockPError(x)
void WinSockPError(const char *);
#endif /* _WIN32 */

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif /* MSG_NOSIGNAL */

static int ipc_sock = -1;
static int ipc_conn = -1;
static int ipc_active = false;

static int ipc_send_all_routes(int fd);

static int ipc_send_net_info(int fd);

/**
 *Create the socket to use for IPC to the
 *GUI front-end
 *
 *@return -1 if an error happened, 0 otherwise
 */
int
ipc_init(void)
{
  //int flags;
  struct sockaddr_in sin;
  int yes = 1;

  /* Add parser function */
  olsr_parser_add_function(&frontend_msgparser, PROMISCUOUS);

  /* get an internet domain socket */
  if ((ipc_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("IPC socket");
    return -1;
  }

  if (setsockopt(ipc_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof(yes)) < 0) {
    perror("SO_REUSEADDR failed");
    close(ipc_sock);
    return -1;
  }

  /* complete the socket structure */
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(IPC_PORT);

  /* bind the socket to the port number */
  if (bind(ipc_sock, (struct sockaddr *)&sin, sizeof(sin)) == -1) {
    perror("IPC bind");
    OLSR_PRINTF(1, "Will retry in 10 seconds...\n");
    sleep(10);
    if (bind(ipc_sock, (struct sockaddr *)&sin, sizeof(sin)) == -1) {
      perror("IPC bind");
      close(ipc_sock);
      return -1;
    }
    OLSR_PRINTF(1, "OK\n");
  }

  /* show that we are willing to listen */
  if (listen(ipc_sock, olsr_cnf->ipc_connections) == -1) {
    perror("IPC listen");
    close(ipc_sock);
    return -1;
  }

  /* Register the socket with the socket parser */
  add_olsr_socket(ipc_sock, &ipc_accept, NULL, NULL, SP_PR_READ);

  return 0;
}


void
ipc_accept(int fd, void *data __attribute__ ((unused)), unsigned int flags __attribute__ ((unused)))
{
  socklen_t addrlen;
  struct sockaddr_in pin;
  char *addr;

  addrlen = sizeof(struct sockaddr_in);

  if ((ipc_conn = accept(fd, (struct sockaddr *)&pin, &addrlen)) == -1) {
    char buf[1024];
    snprintf(buf, sizeof(buf), "IPC accept error: %s", strerror(errno));
    olsr_exit(buf, EXIT_FAILURE);
  } else {
    OLSR_PRINTF(1, "Front end connected\n");
    addr = inet_ntoa(pin.sin_addr);
    if (ipc_check_allowed_ip((union olsr_ip_addr *)&pin.sin_addr.s_addr)) {
      ipc_active = true;
      ipc_send_net_info(ipc_conn);
      ipc_send_all_routes(ipc_conn);
      OLSR_PRINTF(1, "Connection from %s\n", addr);
    } else {
      OLSR_PRINTF(1, "Front end-connection from foregin host(%s) not allowed!\n", addr);
      olsr_syslog(OLSR_LOG_ERR, "OLSR: Front end-connection from foregin host(%s) not allowed!\n", addr);
      CLOSE(ipc_conn);
    }
  }

}

bool
ipc_check_allowed_ip(const union olsr_ip_addr *addr)
{
  struct ip_prefix_list *ipcn;

  if (addr->v4.s_addr == ntohl(INADDR_LOOPBACK)) {
    return true;
  }

  /* check nets */
  for (ipcn = olsr_cnf->ipc_nets; ipcn != NULL; ipcn = ipcn->next) {
    if (ip_in_net(addr, &ipcn->net)) {
      return true;
    }
  }

  return false;
}

/**
 *Sends a olsr packet on the IPC socket.
 *
 *@param msg the olsr struct representing the packet
 *@param in_if the incoming interface
 *@param from_addr the sender address
 *
 *@return true for not preventing forwarding
 */
bool
frontend_msgparser(union olsr_message * msg, struct interface_olsr * in_if __attribute__ ((unused)), union olsr_ip_addr * from_addr
                   __attribute__ ((unused)))
{
  int size;

  if (!ipc_active)
    return true;

  if (olsr_cnf->ip_version == AF_INET)
    size = ntohs(msg->v4.olsr_msgsize);
  else
    size = ntohs(msg->v6.olsr_msgsize);

  if (send(ipc_conn, (void *)msg, size, MSG_NOSIGNAL) < 0) {
    OLSR_PRINTF(1, "(OUTPUT)IPC connection lost!\n");
    CLOSE(ipc_conn);
    ipc_active = false;
  }
  return true;
}

/**
 *Send a route table update to the front-end.
 *
 *@param dst the destination of the route
 *@param gw the gateway for the route
 *@param met the metric for the route
 *@param add 1 if the route is to be added 0 if it is to be deleted
 *@param int_name the name of the interface the route is set to go by
 *
 *@return negative on error
 */
int
ipc_route_send_rtentry(const union olsr_ip_addr *dst, const union olsr_ip_addr *gw, int met, int add, const char *int_name)
{
  struct ipcmsg packet;
  char *tmp;

  if (olsr_cnf->ipc_connections <= 0) {
    return -1;
  }

  if (!ipc_active) {
    return 0;
  }
  memset(&packet, 0, sizeof(struct ipcmsg));
  packet.size = htons(IPC_PACK_SIZE);
  packet.msgtype = ROUTE_IPC;

  packet.target_addr = *dst;

  packet.add = add;
  if (add && gw) {
    packet.metric = met;
    packet.gateway_addr = *gw;
  }

  if (int_name != NULL)
    memcpy(&packet.device[0], int_name, 4);
  else
    memset(&packet.device[0], 0, 4);

  tmp = (char *)&packet;
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

  if (send(ipc_conn, tmp, IPC_PACK_SIZE, MSG_NOSIGNAL) < 0)     // MSG_NOSIGNAL to avoid sigpipe
  {
    OLSR_PRINTF(1, "(RT_ENTRY)IPC connection lost!\n");
    CLOSE(ipc_conn);

    ipc_active = false;
    return -1;
  }

  return 1;
}

static int
ipc_send_all_routes(int fd)
{
  struct rt_entry *rt;
  struct ipcmsg packet;
  char *tmp;

  if (!ipc_active)
    return 0;

  OLSR_FOR_ALL_RT_ENTRIES(rt) {

    memset(&packet, 0, sizeof(struct ipcmsg));
    packet.size = htons(IPC_PACK_SIZE);
    packet.msgtype = ROUTE_IPC;

    packet.target_addr = rt->rt_dst.prefix;

    packet.add = 1;
    packet.metric = (uint8_t) (rt->rt_best->rtp_metric.hops);

    packet.gateway_addr = rt->rt_nexthop.gateway;

    memcpy(&packet.device[0], if_ifwithindex_name(rt->rt_nexthop.iif_index), 4);

    tmp = (char *)&packet;

    /* MSG_NOSIGNAL to avoid sigpipe */
    if (send(fd, tmp, IPC_PACK_SIZE, MSG_NOSIGNAL) < 0) {
      OLSR_PRINTF(1, "(RT_ENTRY)IPC connection lost!\n");
      CLOSE(ipc_conn);
      ipc_active = false;
      return -1;
    }
  }
  OLSR_FOR_ALL_RT_ENTRIES_END(rt);
  return 1;
}

/**
 *Sends OLSR info to the front-end. This info consists of
 *the different time intervals and holding times, number
 *of interfaces, HNA routes and main address.
 *
 *@return negative on error
 */
static int
ipc_send_net_info(int fd)
{
  struct ipc_net_msg net_msg;

  memset(&net_msg, 0, sizeof(net_msg));

  OLSR_PRINTF(1, "Sending net-info to front end...\n");

  /* Message size */
  net_msg.size = htons(sizeof(struct ipc_net_msg));
  /* Message type */
  net_msg.msgtype = NET_IPC;

  /* MIDs */
  /* XXX fix IPC MIDcnt */
  net_msg.mids = (ifnet != NULL && ifnet->int_next != NULL) ? 1 : 0;

  /* HNAs */
  net_msg.hnas = olsr_cnf->hna_entries == NULL ? 0 : 1;

  /* Different values */
  /* Temporary fixes */
  /* XXX fix IPC intervals */
  net_msg.hello_int = 0;       //htons((uint16_t)hello_int);
  net_msg.hello_lan_int = 0;   //htons((uint16_t)hello_int_nw);
  net_msg.tc_int = 0;          //htons((uint16_t)tc_int);
  net_msg.neigh_hold = 0;      //htons((uint16_t)neighbor_hold_time);
  net_msg.topology_hold = 0;   //htons((uint16_t)topology_hold_time);

  net_msg.ipv6 = olsr_cnf->ip_version == AF_INET ? 0 : 1;

  /* Main addr */
  net_msg.main_addr = olsr_cnf->main_addr;

  /*
  {
     unsigned int x, i;

     printf("\t");
     for(i = 0; i < sizeof(struct ipc_net_msg);i++)
     {
     if(x == 4)
     {
     x = 0;
     printf("\n\t");
     }
     x++;
     printf(" %03i", ((u_char *)net_msg)[i]);
     }

     printf("\n");
  }
  */

  if (send(fd, (char *)&net_msg, sizeof(struct ipc_net_msg), MSG_NOSIGNAL) < 0) {
    OLSR_PRINTF(1, "(NETINFO)IPC connection lost!\n");
    CLOSE(ipc_conn);
    return -1;
  }

  return 0;
}

int
shutdown_ipc(void)
{
  OLSR_PRINTF(1, "Shutting down IPC...\n");
  CLOSE(ipc_sock);
  CLOSE(ipc_conn);

  return 1;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
