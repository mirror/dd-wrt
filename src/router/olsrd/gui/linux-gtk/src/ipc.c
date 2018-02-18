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

#include "common.h"
#include "ipc.h"
#include "packet.h"
#include "routes.h"

#ifdef _WIN32
#define close(x) closesocket(x)
#undef errno
#define errno WSAGetLastError()
#undef strerror
#define strerror(x) StrError(x)
#define perror(x) WinSockPError(x)
#endif /* _WIN32 */

int ipc_socket = 0;
int connected;

int
ipc_close(void)
{

  if (close(ipc_socket))
    return 1;

  return 0;
}

int
ipc_connect(struct sockaddr_in *pin)
{
#ifdef _WIN32
  int On = 1;
  unsigned long Len;
#else /* _WIN32 */
  int flags;
#endif /* _WIN32 */

  connected = 0;

  if (!ipc_socket)
    if ((ipc_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      perror("socket");
      exit(EXIT_FAILURE);
    }

  printf("Attempting connect...");

  /* connect to PORT on HOST */
  if (connect(ipc_socket, (struct sockaddr *)pin, sizeof(*pin)) < 0) {
    fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno));
    set_net_info_offline();
    printf("connection refused\n");
  } else {
    set_net_info((gchar *)"Connected!", FALSE);
    printf("Connected!!\n");

    /* Setting socket non-blocking */

#ifdef _WIN32
    if (WSAIoctl(ipc_socket, FIONBIO, &On, sizeof(On), NULL, 0, &Len, NULL, NULL) < 0) {
      fprintf(stderr, "Error while making socket non-blocking!\n");
      exit(EXIT_FAILURE);
    }
#else /* _WIN32 */
    if ((flags = fcntl(ipc_socket, F_GETFL, 0)) < 0) {
      fprintf(stderr, "Error getting socket flags!\n");
      exit(EXIT_FAILURE);
    }

    if (fcntl(ipc_socket, F_SETFL, flags | O_NONBLOCK) < 0) {
      fprintf(stderr, "Error setting socket flags!\n");
      exit(EXIT_FAILURE);
    }
#endif /* _WIN32 */
    connected = 1;

    return 1;
  }

  return 0;

}

int
ipc_read(void)
{
  //int x, i;
  int bytes, tmp_len;
  char *tmp;
  union olsr_message *msg;
  union {
    char buf[BUFFSIZE + 1];
    union olsr_message olsr_msg;
  } inbuf;
  //char buf[BUFFSIZE+1];

  //printf(".");fflush(stdout);
  memset(&inbuf, 0, sizeof(inbuf));
  //buf[0] = '\0';

  if (connected) {
    bytes = recv(ipc_socket, (char *)&inbuf, BUFFSIZE, 0);
    if (bytes == 0) {
      shutdown(ipc_socket, SHUT_RDWR);
      set_net_info((gchar *)"Disconnected from server...", TRUE);
      connected = 0;
      close(ipc_socket);
    }

    if (bytes > 0) {

      tmp = (char *)&inbuf.olsr_msg;

      /*
         x = 0;
         printf("\n\t");
         for(i = 0; i < bytes;i++)
         {
         if(x == 4)
         {
         x = 0;
         printf("\n\t");
         }
         x++;
         printf(" %03i", (u_char) tmp[i]);
         }

         printf("\n\nBytes read: %d - msg_size: %d\n", bytes, ntohs(inbuf.olsr.v4.olsr_packlen));
       */

      msg = &inbuf.olsr_msg;

      /* There can be(there probably are!) several packets in the buffer */

      /* Should be the same for v4 and v6 */
      if (ntohs(inbuf.olsr_msg.v4.olsr_msgsize) < bytes) {
        //printf("chomping...\n");
        while (ntohs(msg->v4.olsr_msgsize) < bytes) {
          ipc_evaluate_message(msg);

          tmp_len = ntohs(msg->v4.olsr_msgsize);
          msg = (union olsr_message *)(void *)&tmp[tmp_len];
          tmp = &tmp[tmp_len];
          if (tmp_len == 0)
            break;
          bytes = bytes - tmp_len;

          tmp_len = ntohs(msg->v4.olsr_msgsize);

          //printf("%d/%d ", tmp_len, bytes);
          /* Copy to start of buffer */
          if (tmp_len > bytes) {
            /* Copy the buffer */
            //printf("READING END OF MESSAGE. %d bytes\n", tmp_len-bytes);
            //printf("\tCopying %d bytes\n", bytes);
            memcpy(&inbuf, tmp, bytes);
            //printf("\tReceiving %d bytes to buffer[%d]\n", tmp_len-bytes, bytes);
            bytes = recv(ipc_socket, (char *)&inbuf.buf[bytes], tmp_len - bytes, 0);
            //printf("\tBytes: %d Size: %d\n", bytes, ntohs(msgs->v4.olsr_packlen));
            tmp = (char *)&inbuf.olsr_msg;
            msg = (union olsr_message *)(void *)tmp;
          }
        }
        //printf("\n");
      }

      /* Only one (or the last) message */
      ipc_evaluate_message(msg);

    }

  }

  return 1;
}

int
ipc_send(void)
{

  return 1;
}

int
ipc_evaluate_message(union olsr_message *olsr_in)
{
  int ipc_pack = 0;
  olsr_u8_t type;
  int msgsize;
  char itoa_buf[10];
  olsr_u8_t vtime;
  union olsr_ip_addr *originator;

  /* Find size, vtime, originator and type - same for IPv4 and 6 */
  type = olsr_in->v4.olsr_msgtype;
  msgsize = ntohs(olsr_in->v4.olsr_msgsize);
  vtime = olsr_in->v4.olsr_vtime;
  originator = (union olsr_ip_addr *)&olsr_in->v4.originator;

  gui_itoa(msgsize, itoa_buf);

  switch (type) {
  case HELLO_MESSAGE:
    //printf("Received HELLO packet\n");
    if (!freeze_packets)
      packet_list_add((char *)"HELLO", ip_to_string(originator), itoa_buf);

    if (ipversion == AF_INET) {
      process_hello(msgsize, vtime, originator, (union hello_message *)&olsr_in->v4.message.hello);
    } else {
      process_hello(msgsize, vtime, originator, (union hello_message *)&olsr_in->v6.message.hello);
    }
    break;

  case TC_MESSAGE:
    if (!freeze_packets)
      packet_list_add((char *)"TC", ip_to_string(originator), itoa_buf);

    if (ipversion == AF_INET) {
      process_tc(msgsize, vtime, originator, (union tc_message *)&olsr_in->v4.message.tc);
      //printf("Received TC packet from %s\n", ip_to_string(&m->olsr_tc->tc_origaddr));
    } else {
      process_tc(msgsize, vtime, originator, (union tc_message *)&olsr_in->v6.message.tc);
      //printf("Received TC packet from %s\n", ip_to_string(&m->olsr_tc->tc_origaddr));
    }
    break;

  case MID_MESSAGE:
    if (!freeze_packets)
      packet_list_add((char *)"MID", ip_to_string(originator), itoa_buf);
    if (ipversion == AF_INET) {
      process_mid(msgsize, vtime, originator, (union mid_message *)&olsr_in->v4.message.mid);
      //printf("Received MID packet from %s\n", ip_to_string(&m->olsr_mid->mid_origaddr));
    } else {
      process_mid(msgsize, vtime, originator, (union mid_message *)&olsr_in->v6.message.mid);
      //printf("Received MID packet from %s\n", ip_to_string(&m->olsr_mid->mid_origaddr));
    }

    break;

  case HNA_MESSAGE:

    if (!freeze_packets)
      packet_list_add((char *)"HNA", ip_to_string(originator), itoa_buf);
    if (ipversion == AF_INET) {
      process_hna(msgsize, vtime, originator, (union hna_message *)&olsr_in->v4.message.hna);
      //printf("Received HNA packet\n");
    } else {
      process_hna(msgsize, vtime, originator, (union hna_message *)&olsr_in->v6.message.hna);
      //printf("Received HNA packet\n");
    }

    break;

  case IPC_MESSAGE:
    //printf("Received IPC packet\n");
    ipc_pack = 1;               /* Don't add to buffer */
    ipc_eval_route_packet((struct routemsg *)olsr_in);
    break;
  case IPC_NET:
    //printf("Received IPC packet\n");
    ipc_pack = 1;               /* Don't add to buffer */
    ipc_eval_net_info((struct netmsg *)olsr_in);
    break;
  default:
    if (!freeze_packets) {
      char unk_label[8];
      sprintf(unk_label, "%d", type);
      packet_list_add(unk_label, ip_to_string(originator), itoa_buf);
    }
    printf("Unknown packet type %d\n", type);
    break;
  }

  if (!freeze_packets && !ipc_pack) {
    add_packet_to_buffer(olsr_in, msgsize);
  }

  return 1;
}

int
ipc_eval_net_info(struct netmsg *msg)
{
  char info[256];
  printf("Evaluating NET info...\n");

  /*
     printf("\tMain address: %s\n", ip_to_string(&msg->main_addr));
     printf("\tMid addresses: %d\n", msg->mids);
     printf("\tHna addresses: %d\n", msg->hnas);
     printf("\tHELLO interval: %f\n", (float)(ntohs(msg->hello_int)));
     printf("\tHELLO LAN interval: %f\n", (float)(ntohs(msg->hello_lan_int)));
     printf("\tTC interval: %d\n", ntohs(msg->tc_int));
     printf("\tNeighbor hold time: %d\n", ntohs(msg->neigh_hold));
     printf("\tTopology hold: %d\n", ntohs(msg->topology_hold));
   */
  if (msg->ipv6 == 0) {
    ipversion = AF_INET;
    ipsize = sizeof(struct in_addr);
    sprintf(&info[0],
            "IP version 4\nMain address: %s\nMid addresses: %d\nHna addresses: %d\nHELLO interval: %d\nHELLO LAN interval: %d\nTC interval: %d\nNeighbor hold time: %d\nTopology hold: %d\n",
            ip_to_string(&msg->main_addr), msg->mids, msg->hnas, ntohs(msg->hello_int), ntohs(msg->hello_lan_int),
            ntohs(msg->tc_int), ntohs(msg->neigh_hold), ntohs(msg->topology_hold));
  } else {
    ipversion = AF_INET6;
    ipsize = sizeof(struct in6_addr);
    sprintf(&info[0],
            "IP version 6\nMain address: %s\nMid addresses: %d\nHna addresses: %d\nHELLO interval: %d\nHELLO LAN interval: %d\nTC interval: %d\nNeighbor hold time: %d\nTopology hold: %d\n",
            ip_to_string(&msg->main_addr), msg->mids, msg->hnas, ntohs(msg->hello_int), ntohs(msg->hello_lan_int),
            ntohs(msg->tc_int), ntohs(msg->neigh_hold), ntohs(msg->topology_hold));
  }

  memcpy(&main_addr, &msg->main_addr, ipsize);

  set_net_info(&info[0], 0);

  return 0;
}

int
ipc_eval_route_packet(struct routemsg *msg)
{
  struct route_entry rt_ent;
  char dev[5];
  char gw[16];
  char itoa_buf[10];
  dev[4] = '\0';
  memset(&gw[0], 0, 16);

  printf("Processing route packet\n");

  memset(rt_ent.if_name, 0, MAX_IF_NAMESIZ);

  /* Fill struct */

  memcpy(&rt_ent.gw, &msg->gateway_addr, ipsize);
  memcpy(&rt_ent.dst, &msg->target_addr, ipsize);
  memcpy(rt_ent.if_name, msg->device, 4);
  rt_ent.hopcnt = msg->metric;

  if (msg->add) {
    memcpy(&dev[0], &msg->device[0], 4);

    /*Add node to node list */
    memcpy(&gw[0], ip_to_string(&msg->gateway_addr), 16);

    gui_itoa(msg->metric, itoa_buf);

    route_list_add(ip_to_string(&msg->target_addr), gw, dev, itoa_buf);

    printf("\tRoute to %s(hc %d) added\n", ip_to_string(&msg->target_addr), rt_ent.hopcnt);

    /*
       printf("\tRoute to %s added\n", ip_to_string(&msg->target_addr));
       printf("\tGateway %s\n", gw);
       printf("\tInterface %s\n", msg->device);
       printf("\tMetric %d\n", msg->metric);
     */
  } else {

    if (route_list_del(ip_to_string(&msg->target_addr)) < 1)
      printf("COULD NOT FIND ROUTE TO DELETE!\n\n");

    printf("\tRoute to %s deleted\n", ip_to_string(&msg->target_addr));
  }
  return 1;
}

int
process_hello(int size, olsr_u8_t vtime, union olsr_ip_addr *originator, union hello_message *m)
{
  struct hellinfo *neigh;
  struct hellinfo6 *neigh6;
  int i;
  int nsize;
  int type;
  //int link;

  printf("Processing HELLO from %s size = %d\n", ip_to_string(originator), size);

  if (!update_timer_node(originator, vtime))
    add_node(originator, vtime);

  /* Add neighbors if any */
  size = size - 4 - 8 - ipsize; /* size of neighbors(size - olsrheder- helloheader) */

  if (!size)
    return 0;

  /* Get the neighbortype-blocks start */
  neigh = m->v4.hell_info;
  neigh6 = m->v6.hell_info;

  //printf("HELLO Size: %d\n", size);

  while (size > 0) {

    //printf("\tNEIGH: 0x%x\n", (int)neigh);
    if (ipversion == AF_INET) {
      nsize = ntohs(neigh->size);
      type = EXTRACT_STATUS(ntohs(neigh->link_code));
      //link = EXTRACT_LINK(ntohs(neigh->link_code));
      //printf("TYPE: %d\n", neigh->link_code);
    } else {
      nsize = ntohs(neigh6->size);
      type = EXTRACT_STATUS(ntohs(neigh6->link_code));
      //link = EXTRACT_LINK(ntohs(neigh6->link_code));
    }

    size -= nsize;

    nsize = nsize - 4;          /* - hellinfo header */
    //printf("Size left: %d Current hellinfo: %d\n", size, nsize);
    i = 0;
    while (nsize > 0) {
      //printf("Adding neighbor %s...\n", ip_to_string((union olsr_ip_addr *)&neigh->neigh_addr[i]));
      /*
         if(MPR)
         update_timer_mpr((union olsr_ip_addr *)&mprsinfo->addr, originator);
       */

      if (ipversion == AF_INET) {       /* Update MPRs */
        if (type == MPR_NEIGH) {
          //printf("MPR from HELLO\n");
          update_timer_mpr((union olsr_ip_addr *)&neigh->neigh_addr[i], originator, vtime);
        }
        add_node((union olsr_ip_addr *)&neigh->neigh_addr[i++], vtime);
      } else {
        if (type == MPR_NEIGH) {        /* Update MPRs */
          //printf("MPR from HELLO\n");
          update_timer_mpr((union olsr_ip_addr *)&neigh6->neigh_addr[i], originator, vtime);
        }
        add_node((union olsr_ip_addr *)&neigh6->neigh_addr[i++], vtime);
      }

      nsize = nsize - ipsize;
      //printf("Nsize: %d\n", nsize);
    }

    neigh = (struct hellinfo *)&neigh->neigh_addr[i];
    neigh6 = (struct hellinfo6 *)&neigh6->neigh_addr[i];

  }
  //printf("DONE\n");

  return 0;
}

int
process_tc(int size, olsr_u8_t vtime, union olsr_ip_addr *originator, union tc_message *m)
{

  struct neigh_info *mprsinfo = NULL;
  struct neigh_info6 *mprsinfo6 = NULL;

  printf("Processing TC from %s size = %d\n", ip_to_string(originator), size);

  /* Updating timer */
  if (!update_timer_node(originator, vtime))
    add_node(originator, vtime);

  /* Calculate size of the mprsinfo */
  size = size - 4 - 8 - ipsize;

  //printf("TC Size: %d\n", size);

  if (ipversion == AF_INET)
    mprsinfo = &m->v4.neigh[0];
  else
    mprsinfo6 = &m->v6.neigh[0];

  while (size > 0) {
    if (ipversion == AF_INET) {
      //printf("\tprocessing TC: %s\n", ip_to_string((union olsr_ip_addr *)&mprsinfo->addr));
      add_node((union olsr_ip_addr *)&mprsinfo->addr, vtime);
      update_timer_mpr((union olsr_ip_addr *)&mprsinfo->addr, originator, vtime);
      mprsinfo++;
    } else {
      //printf("\tprocessing TC: %s\n", ip_to_string((union olsr_ip_addr *)&mprsinfo6->addr));
      //printf("TC: add node %s\n", ip_to_string((union olsr_ip_addr *)&mprsinfo6->addr));
      add_node((union olsr_ip_addr *)&mprsinfo6->addr, vtime);
      update_timer_mpr((union olsr_ip_addr *)&mprsinfo6->addr, originator, vtime);
      mprsinfo6++;
    }
    size = size - ipsize;
    //printf("\tsize: %d\n", size);
  }
  //printf("DONE\n");

  return 0;
}

int
process_mid(int size, olsr_u8_t vtime, union olsr_ip_addr *originator, union mid_message *m)
{
  struct midaddr *midaddr = NULL;
  struct midaddr6 *midaddr6 = NULL;

  printf("Processing MID from %s size = %d\n", ip_to_string(originator), size);

  /* Calculate size of the midinfo */
  size = size - 4 - 4 - ipsize;

  if (ipversion == AF_INET)
    midaddr = &m->v4.mid_addr[0];
  else
    midaddr6 = &m->v6.mid_addr[0];

  //printf("MID size: %d\n", size);

  while (size > 0) {
    if (ipversion == AF_INET) {
      //printf("MID: add node %s\n", ip_to_string((union olsr_ip_addr *)&midaddr->addr));
      add_mid_node(originator, (union olsr_ip_addr *)&midaddr->addr, vtime);
      midaddr++;
    } else {
      add_mid_node(originator, (union olsr_ip_addr *)&midaddr6->addr, vtime);
      //printf("MID: add node %s\n", ip_to_string((union olsr_ip_addr *)&midaddr6->addr));
      midaddr6++;
    }
    size = size - ipsize;
  }

  //printf("DONE\n");
  return 0;
}

int
process_hna(int size, olsr_u8_t vtime, union olsr_ip_addr *originator, union hna_message *m)
{
  struct hnapair *hnapairs = NULL;
  struct hnapair6 *hnapairs6 = NULL;

  printf("Processing HNA size = %d\n", size);

  /* Calculate size of the hnainfo */
  size = size - 4 - 4 - ipsize;

  if (ipversion == AF_INET)
    hnapairs = &m->v4.hna_net[0];
  else
    hnapairs6 = &m->v6.hna_net[0];

  while (size > 0) {
    if (ipversion == AF_INET) {
      //printf("\tHNA:%s\n", ip_to_string((union olsr_ip_addr *)&hnapairs->addr));
      add_hna_node(originator, (union olsr_ip_addr *)&hnapairs->addr, (union olsr_ip_addr *)&hnapairs->netmask, vtime);
      hnapairs++;
    } else {
      add_hna_node(originator, (union olsr_ip_addr *)&hnapairs6->addr, (union olsr_ip_addr *)&hnapairs6->netmask, vtime);
      hnapairs6++;
    }

    size = size - ipsize - ipsize;
  }

  return 0;
}

char *
ip_to_string(union olsr_ip_addr *addr)
{
  char *ret;
  struct in_addr in;

  if (ipversion == AF_INET) {
    in = addr->v4;
    ret = inet_ntoa(in);
  } else {
    /* IPv6 */
    ret = (char *)inet_ntop(AF_INET6, &addr->v6, ipv6_buf, sizeof(ipv6_buf));
  }

  return ret;

}

int
gui_itoa(int i, char *buf)
{
  char tmp[10];

  if (snprintf(buf, sizeof(tmp), "%hd", (short)i)) {
    /* This shitty string needs to be converted to UTF-8 */
    snprintf(tmp, sizeof(tmp), "%s", g_locale_to_utf8(buf, -1, NULL, NULL, NULL));
    strcpy(buf, tmp);
    return 1;
    //return ret;
  }
  return 0;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
