
/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tonnesen(andreto@olsr.org)
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

#include "print_packet.h"
#include "ipcalc.h"
#include "mantissa.h"
#include "defs.h"
#include "olsr.h"
#include "lq_packet.h"
#include "net_olsr.h"

static void print_messagedump(FILE *, uint8_t *, int16_t);

static void print_midmsg(FILE *, uint8_t *, int16_t);

static void print_hnamsg(FILE *, uint8_t *, int16_t);

static void print_olsr_tcmsg(FILE *, uint8_t *, int16_t);

static void print_olsr_tcmsg_lq(FILE *, uint8_t *, int16_t);

static void print_hellomsg(FILE *, uint8_t *, int16_t);

static void print_hellomsg_lq(FILE *, uint8_t *, int16_t);

/* Entire packet */
int8_t
print_olsr_serialized_packet(FILE * handle, union olsr_packet *pkt, uint16_t size, union olsr_ip_addr *from_addr)
{
  int16_t remainsize = size - OLSR_HEADERSIZE;
  union olsr_message *msg;
  struct ipaddr_str buf;

  /* Print packet header (no IP4/6 difference) */
  fprintf(handle, "  ============== OLSR PACKET ==============\n   source: %s\n   length: %d bytes\n   seqno: %d\n\n",
          from_addr ? olsr_ip_to_string(&buf, from_addr) : "UNKNOWN", ntohs(pkt->v4.olsr_packlen), ntohs(pkt->v4.olsr_seqno));

  /* Check size */
  if (size != ntohs(pkt->v4.olsr_packlen))
    fprintf(handle, "   SIZE MISSMATCH(%d != %d)!\n", size, ntohs(pkt->v4.olsr_packlen));

  msg = (union olsr_message *)pkt->v4.olsr_msg;

  /* Print all messages */
  while ((remainsize > 0) && ntohs(msg->v4.olsr_msgsize)) {
    print_olsr_serialized_message(handle, msg);
    remainsize -= ntohs(msg->v4.olsr_msgsize);
    msg = (union olsr_message *)((char *)msg + ntohs(msg->v4.olsr_msgsize));
  }

  /* Done */
  fprintf(handle, "  =========================================\n\n");
  return 1;
}

/* Single message */
int8_t
print_olsr_serialized_message(FILE * handle, union olsr_message * msg)
{
  struct ipaddr_str buf;

  fprintf(handle, "   ------------ OLSR MESSAGE ------------\n");
  fprintf(handle, "    Sender main addr: %s\n", olsr_ip_to_string(&buf, (union olsr_ip_addr *)&msg->v4.originator));
  fprintf(handle, "    Type: %s, size: %d, vtime: %u ms\n", olsr_msgtype_to_string(msg->v4.olsr_msgtype),
          ntohs(msg->v4.olsr_msgsize), me_to_reltime(msg->v4.olsr_vtime));
  fprintf(handle, "    TTL: %d, Hopcnt: %d, seqno: %d\n", (olsr_cnf->ip_version == AF_INET) ? msg->v4.ttl : msg->v6.ttl,
          (olsr_cnf->ip_version == AF_INET) ? msg->v4.hopcnt : msg->v6.hopcnt,
          ntohs((olsr_cnf->ip_version == AF_INET) ? msg->v4.seqno : msg->v6.seqno));

  switch (msg->v4.olsr_msgtype) {
    /* Print functions for individual messagetypes */
  case (MID_MESSAGE):
    print_midmsg(handle, (olsr_cnf->ip_version == AF_INET) ? (uint8_t *) & msg->v4.message : (uint8_t *) & msg->v6.message,
                 ntohs(msg->v4.olsr_msgsize));
    break;
  case (HNA_MESSAGE):
    print_hnamsg(handle, (olsr_cnf->ip_version == AF_INET) ? (uint8_t *) & msg->v4.message : (uint8_t *) & msg->v6.message,
                 ntohs(msg->v4.olsr_msgsize));
    break;
  case (TC_MESSAGE):
    print_olsr_tcmsg(handle, (olsr_cnf->ip_version == AF_INET) ? (uint8_t *) & msg->v4.message : (uint8_t *) & msg->v6.message,
                     ntohs(msg->v4.olsr_msgsize));
    break;
  case (LQ_TC_MESSAGE):
    print_olsr_tcmsg_lq(handle,
                        (olsr_cnf->ip_version == AF_INET) ? (uint8_t *) & msg->v4.message : (uint8_t *) & msg->v6.message,
                        ntohs(msg->v4.olsr_msgsize));
    break;
  case (HELLO_MESSAGE):
    print_hellomsg(handle, (olsr_cnf->ip_version == AF_INET) ? (uint8_t *) & msg->v4.message : (uint8_t *) & msg->v6.message,
                   ntohs(msg->v4.olsr_msgsize));
    break;
  case (LQ_HELLO_MESSAGE):
    print_hellomsg_lq(handle, (olsr_cnf->ip_version == AF_INET) ? (uint8_t *) & msg->v4.message : (uint8_t *) & msg->v6.message,
                      ntohs(msg->v4.olsr_msgsize));
    break;
  default:
    print_messagedump(handle, (uint8_t *) msg, ntohs(msg->v4.olsr_msgsize));
  }

  fprintf(handle, "   --------------------------------------\n\n");
  return 1;
}

static void
print_messagedump(FILE * handle, uint8_t * msg, int16_t size)
{
  int i, x = 0;

  fprintf(handle, "     Data dump:\n     ");
  for (i = 0; i < size; i++) {
    if (x == 4) {
      x = 0;
      fprintf(handle, "\n     ");
    }
    x++;
    if (olsr_cnf->ip_version == AF_INET)
      fprintf(handle, " %-3i ", (u_char) msg[i]);
    else
      fprintf(handle, " %-2x ", (u_char) msg[i]);
  }
  fprintf(handle, "\n");
}

static void
print_hellomsg(FILE * handle, uint8_t * data, int16_t totsize)
{
  union olsr_ip_addr *haddr;
  int hellosize = totsize - ((olsr_cnf->ip_version == AF_INET) ? OLSR_MSGHDRSZ_IPV4 : OLSR_MSGHDRSZ_IPV6);

  fprintf(handle, "    +Htime: %u ms\n", me_to_reltime(data[2]));

  fprintf(handle, "    +Willingness: %d\n", data[3]);

  if (olsr_cnf->ip_version == AF_INET) {
    /* IPv4 */
    struct hellomsg *h;
    struct hellinfo *hinf;

    h = (struct hellomsg *)data;

    for (hinf = h->hell_info; (char *)hinf < ((char *)data + hellosize);
         hinf = (struct hellinfo *)((char *)hinf + ntohs(hinf->size))) {

      fprintf(handle, "    ++ Link: %s, Status: %s, Size: %d\n", olsr_link_to_string(EXTRACT_LINK(hinf->link_code)),
              olsr_status_to_string(EXTRACT_STATUS(hinf->link_code)), ntohs(hinf->size));

      for (haddr = (union olsr_ip_addr *)&hinf->neigh_addr; (char *)haddr < (char *)hinf + ntohs(hinf->size);
           haddr = (union olsr_ip_addr *)&haddr->v6.s6_addr[4]) {
        struct ipaddr_str buf;
        fprintf(handle, "    ++ %s\n", olsr_ip_to_string(&buf, haddr));
      }
    }

  } else {
    /* IPv6 */
    struct hellomsg6 *h6;
    struct hellinfo6 *hinf6;

    h6 = (struct hellomsg6 *)data;

    for (hinf6 = h6->hell_info; (char *)hinf6 < ((char *)data + (hellosize));
         hinf6 = (struct hellinfo6 *)((char *)hinf6 + ntohs(hinf6->size))) {
      fprintf(handle, "    ++ Link: %s, Status: %s, Size: %d\n", olsr_link_to_string(EXTRACT_LINK(hinf6->link_code)),
              olsr_status_to_string(EXTRACT_STATUS(hinf6->link_code)), ntohs(hinf6->size));

      for (haddr = (union olsr_ip_addr *)hinf6->neigh_addr; (char *)haddr < (char *)hinf6 + ntohs(hinf6->size); haddr++) {
        struct ipaddr_str buf;
        fprintf(handle, "    ++ %s\n", olsr_ip_to_string(&buf, haddr));
      }
    }

  }

}

static void
print_hellomsg_lq(FILE * handle, uint8_t * data, int16_t totsize)
{
  union olsr_ip_addr *haddr;
  int hellosize = totsize - ((olsr_cnf->ip_version == AF_INET) ? OLSR_MSGHDRSZ_IPV4 : OLSR_MSGHDRSZ_IPV6);

  fprintf(handle, "    +Htime: %u ms\n", me_to_reltime(data[2]));

  fprintf(handle, "    +Willingness: %d\n", data[3]);

  if (olsr_cnf->ip_version == AF_INET) {
    /* IPv4 */
    struct hellomsg *h;
    struct hellinfo *hinf;

    h = (struct hellomsg *)data;

    for (hinf = h->hell_info; (char *)hinf < ((char *)data + hellosize);
         hinf = (struct hellinfo *)((char *)hinf + ntohs(hinf->size))) {

      fprintf(handle, "    ++ Link: %s, Status: %s, Size: %d\n", olsr_link_to_string(EXTRACT_LINK(hinf->link_code)),
              olsr_status_to_string(EXTRACT_STATUS(hinf->link_code)), ntohs(hinf->size));

      for (haddr = (union olsr_ip_addr *)&hinf->neigh_addr; (char *)haddr < (char *)hinf + ntohs(hinf->size);
           haddr = (union olsr_ip_addr *)&haddr->v6.s6_addr[8]) {
        struct ipaddr_str buf;
        uint8_t *quality = (uint8_t *) haddr + olsr_cnf->ipsize;
        fprintf(handle, "    ++ %s\n", olsr_ip_to_string(&buf, haddr));
        fprintf(handle, "    ++ LQ = %d, RLQ = %d\n", quality[0], quality[1]);
      }
    }

  } else {
    /* IPv6 */
    struct hellomsg6 *h6;
    struct hellinfo6 *hinf6;

    h6 = (struct hellomsg6 *)data;

    for (hinf6 = h6->hell_info; (char *)hinf6 < ((char *)data + (hellosize));
         hinf6 = (struct hellinfo6 *)((char *)hinf6 + ntohs(hinf6->size))) {
      fprintf(handle, "    ++ Link: %s, Status: %s, Size: %d\n", olsr_link_to_string(EXTRACT_LINK(hinf6->link_code)),
              olsr_status_to_string(EXTRACT_STATUS(hinf6->link_code)), ntohs(hinf6->size));

      for (haddr = (union olsr_ip_addr *)hinf6->neigh_addr; (char *)haddr < (char *)hinf6 + ntohs(hinf6->size) + 4; haddr++) {
        struct ipaddr_str buf;
        uint8_t *quality = (uint8_t *) haddr + olsr_cnf->ipsize;
        fprintf(handle, "    ++ %s\n", olsr_ip_to_string(&buf, haddr));
        fprintf(handle, "    ++ LQ = %d, RLQ = %d\n", quality[0], quality[1]);
      }
    }

  }
}

static void
print_olsr_tcmsg_lq(FILE * handle, uint8_t * data, int16_t totsize)
{
  int remsize = totsize - ((olsr_cnf->ip_version == AF_INET) ? OLSR_MSGHDRSZ_IPV4 : OLSR_MSGHDRSZ_IPV6);

  fprintf(handle, "    +ANSN: %d\n", htons(((struct olsr_tcmsg *)data)->ansn));

  data += 4;
  remsize -= 4;

  while (remsize) {
    struct ipaddr_str buf;
    fprintf(handle, "    +Neighbor: %s\n", olsr_ip_to_string(&buf, (union olsr_ip_addr *)data));
    data += olsr_cnf->ipsize;
    fprintf(handle, "    +LQ: %d, ", *data);
    data += 1;
    fprintf(handle, "RLQ: %d\n", *data);
    data += 3;
    remsize -= (olsr_cnf->ipsize + 4);
  }

}

static void
print_olsr_tcmsg(FILE * handle, uint8_t * data, int16_t totsize)
{
  int remsize = totsize - ((olsr_cnf->ip_version == AF_INET) ? OLSR_MSGHDRSZ_IPV4 : OLSR_MSGHDRSZ_IPV6);

  fprintf(handle, "    +ANSN: %d\n", htons(((struct olsr_tcmsg *)data)->ansn));

  data += 4;
  remsize -= 4;

  while (remsize) {
    struct ipaddr_str buf;
    fprintf(handle, "    +Neighbor: %s\n", olsr_ip_to_string(&buf, (union olsr_ip_addr *)data));
    data += olsr_cnf->ipsize;

    remsize -= olsr_cnf->ipsize;
  }

}

static void
print_hnamsg(FILE * handle, uint8_t * data, int16_t totsize)
{
  int remsize = totsize - ((olsr_cnf->ip_version == AF_INET) ? OLSR_MSGHDRSZ_IPV4 : OLSR_MSGHDRSZ_IPV6);

  while (remsize) {
    struct ipaddr_str buf;
    fprintf(handle, "    +Network: %s\n", olsr_ip_to_string(&buf, (union olsr_ip_addr *)data));
    data += olsr_cnf->ipsize;
    fprintf(handle, "    +Netmask: %s\n", olsr_ip_to_string(&buf, (union olsr_ip_addr *)data));
    data += olsr_cnf->ipsize;

    remsize -= (olsr_cnf->ipsize * 2);
  }

}

static void
print_midmsg(FILE * handle, uint8_t * data, int16_t totsize)
{
  int remsize = totsize - ((olsr_cnf->ip_version == AF_INET) ? OLSR_MSGHDRSZ_IPV4 : OLSR_MSGHDRSZ_IPV6);

  while (remsize) {
    struct ipaddr_str buf;
    fprintf(handle, "    +Alias: %s\n", olsr_ip_to_string(&buf, (union olsr_ip_addr *)data));
    data += olsr_cnf->ipsize;
    remsize -= olsr_cnf->ipsize;
  }
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
