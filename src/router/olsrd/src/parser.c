
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

#include "parser.h"
#include "ipcalc.h"
#include "defs.h"
#include "process_package.h"
#include "mantissa.h"
#include "hysteresis.h"
#include "duplicate_set.h"
#include "mid_set.h"
#include "olsr.h"
#include "rebuild_packet.h"
#include "net_os.h"
#include "log.h"
#include "print_packet.h"
#include "net_olsr.h"

#ifdef WIN32
#undef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#undef errno
#define errno WSAGetLastError()
#undef strerror
#define strerror(x) StrError(x)
#endif

/* Sven-Ola: On very slow devices used in huge networks
 * the amount of lq_tc messages is so high, that the
 * recv() loop never ends. This is a small hack to end
 * the loop in this cases
 */

unsigned int cpu_overload_exit = 0;

struct parse_function_entry *parse_functions;
struct preprocessor_function_entry *preprocessor_functions;
struct packetparser_function_entry *packetparser_functions;

static uint32_t inbuf_aligned[MAXMESSAGESIZE/sizeof(uint32_t) + 1];
static char *inbuf = (char *)inbuf_aligned;

static bool disp_pack_in = false;

void
parser_set_disp_pack_in(bool val)
{
  disp_pack_in = val;
}

/**
 *Initialize the parser.
 *
 *@return nada
 */
void
olsr_init_parser(void)
{
  OLSR_PRINTF(3, "Initializing parser...\n");

  /* Initialize the packet functions */
  olsr_init_package_process();

}

void
olsr_parser_add_function(parse_function * function, uint32_t type)
{
  struct parse_function_entry *new_entry;

  OLSR_PRINTF(3, "Parser: registering event for type %d\n", type);

  new_entry = olsr_malloc(sizeof(struct parse_function_entry), "Register parse function");

  new_entry->function = function;
  new_entry->type = type;

  /* Queue */
  new_entry->next = parse_functions;
  parse_functions = new_entry;

  OLSR_PRINTF(3, "Register parse function: Added function for type %d\n", type);

}

int
olsr_parser_remove_function(parse_function * function, uint32_t type)
{
  struct parse_function_entry *entry, *prev;

  entry = parse_functions;
  prev = NULL;

  while (entry) {
    if ((entry->function == function) && (entry->type == type)) {
      if (entry == parse_functions) {
        parse_functions = entry->next;
      } else {
        prev->next = entry->next;
      }
      free(entry);
      return 1;
    }

    prev = entry;
    entry = entry->next;
  }

  return 0;
}

void
olsr_preprocessor_add_function(preprocessor_function * function)
{
  struct preprocessor_function_entry *new_entry;

  OLSR_PRINTF(3, "Parser: registering preprocessor\n");

  new_entry = olsr_malloc(sizeof(struct preprocessor_function_entry), "Register preprocessor function");

  new_entry->function = function;

  /* Queue */
  new_entry->next = preprocessor_functions;
  preprocessor_functions = new_entry;

  OLSR_PRINTF(3, "Registered preprocessor function\n");

}

int
olsr_preprocessor_remove_function(preprocessor_function * function)
{
  struct preprocessor_function_entry *entry, *prev;

  entry = preprocessor_functions;
  prev = NULL;

  while (entry) {
    if (entry->function == function) {
      if (entry == preprocessor_functions) {
        preprocessor_functions = entry->next;
      } else {
        prev->next = entry->next;
      }
      free(entry);
      return 1;
    }

    prev = entry;
    entry = entry->next;
  }

  return 0;
}

void
olsr_packetparser_add_function(packetparser_function * function)
{
  struct packetparser_function_entry *new_entry;

  OLSR_PRINTF(3, "Parser: registering packetparser\n");

  new_entry = olsr_malloc(sizeof(struct packetparser_function_entry), "Register packetparser function");

  new_entry->function = function;

  /* Queue */
  new_entry->next = packetparser_functions;
  packetparser_functions = new_entry;

  OLSR_PRINTF(3, "Registered packetparser  function\n");

}

int
olsr_packetparser_remove_function(packetparser_function * function)
{
  struct packetparser_function_entry *entry, *prev;

  entry = packetparser_functions;
  prev = NULL;

  while (entry) {
    if (entry->function == function) {
      if (entry == packetparser_functions) {
        packetparser_functions = entry->next;
      } else {
        prev->next = entry->next;
      }
      free(entry);
      return 1;
    }

    prev = entry;
    entry = entry->next;
  }

  return 0;
}

/**
 *Process a newly received OLSR packet. Checks the type
 *and to the neccessary convertions and call the
 *corresponding functions to handle the information.
 *@param from the sockaddr struct describing the sender
 *@param olsr the olsr struct containing the message
 *@param size the size of the message
 *@return nada
 */

void
parse_packet(struct olsr *olsr, int size, struct interface *in_if, union olsr_ip_addr *from_addr)
{
  union olsr_message *m = (union olsr_message *)olsr->olsr_msg;
  int count;
  int msgsize;
  struct parse_function_entry *entry;
  struct packetparser_function_entry *packetparser;

  count = size - ((char *)m - (char *)olsr);

  if (count < MIN_PACKET_SIZE(olsr_cnf->ip_version))
    return;

  if (ntohs(olsr->olsr_packlen) != size) {
    struct ipaddr_str buf;
    OLSR_PRINTF(1, "Size error detected in received packet.\nRecieved %d, in packet %d\n", size, ntohs(olsr->olsr_packlen));

    olsr_syslog(OLSR_LOG_ERR, " packet length error in  packet received from %s!", olsr_ip_to_string(&buf, from_addr));
    return;
  }
  // translate sequence number to host order
  olsr->olsr_seqno = ntohs(olsr->olsr_seqno);

  // call packetparser
  packetparser = packetparser_functions;
  while (packetparser) {
    packetparser->function(olsr, in_if, from_addr);
    packetparser = packetparser->next;
  }

  //printf("Message from %s\n\n", olsr_ip_to_string(&buf, from_addr));

  /* Display packet */
  if (disp_pack_in)
    print_olsr_serialized_packet(stdout, (union olsr_packet *)olsr, size, from_addr);

  if (olsr_cnf->ip_version == AF_INET)
    msgsize = ntohs(m->v4.olsr_msgsize);
  else
    msgsize = ntohs(m->v6.olsr_msgsize);

  /*
   * Hysteresis update - for every OLSR package
   */
  if (olsr_cnf->use_hysteresis) {
    if (olsr_cnf->ip_version == AF_INET) {
      /* IPv4 */
      update_hysteresis_incoming(from_addr, in_if, ntohs(olsr->olsr_seqno));
    } else {
      /* IPv6 */
      update_hysteresis_incoming(from_addr, in_if, ntohs(olsr->olsr_seqno));
    }
  }

  for (; count > 0; m = (union olsr_message *)((char *)m + (msgsize))) {
    bool forward = true;

    if (count < MIN_PACKET_SIZE(olsr_cnf->ip_version))
      break;

    if (olsr_cnf->ip_version == AF_INET)
      msgsize = ntohs(m->v4.olsr_msgsize);
    else
      msgsize = ntohs(m->v6.olsr_msgsize);

    count -= msgsize;

    /* Check size of message */
    if (count < 0) {
      struct ipaddr_str buf;
      OLSR_PRINTF(1, "packet length error in  packet received from %s!", olsr_ip_to_string(&buf, from_addr));

      olsr_syslog(OLSR_LOG_ERR, " packet length error in  packet received from %s!", olsr_ip_to_string(&buf, from_addr));
      break;
    }

    /*RFC 3626 section 3.4:
     *  2    If the time to live of the message is less than or equal to
     *  '0' (zero), or if the message was sent by the receiving node
     *  (i.e., the Originator Address of the message is the main
     *  address of the receiving node): the message MUST silently be
     *  dropped.
     */

    /* Should be the same for IPv4 and IPv6 */
    if (ipequal((union olsr_ip_addr *)&m->v4.originator, &olsr_cnf->main_addr)
        || !olsr_validate_address((union olsr_ip_addr *)&m->v4.originator)) {
#ifdef DEBUG
      struct ipaddr_str buf;
#endif
#ifdef DEBUG
      OLSR_PRINTF(3, "Not processing message originating from %s!\n",
                  olsr_ip_to_string(&buf, (union olsr_ip_addr *)&m->v4.originator));
#endif
      continue;
    }

    entry = parse_functions;
    while (entry) {
      /* Should be the same for IPv4 and IPv6 */

      /* Promiscuous or exact match */
      if ((entry->type == PROMISCUOUS) || (entry->type == m->v4.olsr_msgtype)) {
        if (!entry->function(m, in_if, from_addr))
          forward = false;
      }
      entry = entry->next;
    }

    if (forward) {
      olsr_forward_message(m, in_if, from_addr);
    }
  }                             /* for olsr_msg */
}

/**
 *Processing OLSR data from socket. Reading data, setting
 *wich interface recieved the message, Sends IPC(if used)
 *and passes the packet on to parse_packet().
 *
 *@param fd the filedescriptor that data should be read from.
 *@return nada
 */
void
olsr_input(int fd)
{
  struct interface *olsr_in_if;
  union olsr_ip_addr from_addr;
  struct preprocessor_function_entry *entry;
  char *packet;

  cpu_overload_exit = 0;

  for (;;) {
#ifdef DEBUG
    struct ipaddr_str buf;
#endif
    /* sockaddr_in6 is bigger than sockaddr !!!! */
    struct sockaddr_storage from;
    socklen_t fromlen;
    int cc;

    if (32 < ++cpu_overload_exit) {
      OLSR_PRINTF(1, "CPU overload detected, ending olsr_input() loop\n");
      break;
    }

    fromlen = sizeof(struct sockaddr_storage);
    cc = olsr_recvfrom(fd, inbuf, sizeof(inbuf_aligned), 0, (struct sockaddr *)&from, &fromlen);

    if (cc <= 0) {
      if (cc < 0 && errno != EWOULDBLOCK) {
        OLSR_PRINTF(1, "error recvfrom: %s", strerror(errno));
        olsr_syslog(OLSR_LOG_ERR, "error recvfrom: %m");
      }
      break;
    }
    if (olsr_cnf->ip_version == AF_INET) {
      /* IPv4 sender address */
      from_addr.v4 = ((struct sockaddr_in *)&from)->sin_addr;
    } else {
      /* IPv6 sender address */
      from_addr.v6 = ((struct sockaddr_in6 *)&from)->sin6_addr;
    }

#ifdef DEBUG
    OLSR_PRINTF(5, "Recieved a packet from %s\n",
        olsr_ip_to_string(&buf, &from_addr));
#endif

    if ((olsr_cnf->ip_version == AF_INET) && (fromlen != sizeof(struct sockaddr_in)))
      break;
    else if ((olsr_cnf->ip_version == AF_INET6) && (fromlen != sizeof(struct sockaddr_in6)))
      break;

    /* are we talking to ourselves? */
    if (if_ifwithaddr(&from_addr) != NULL)
      return;

    if ((olsr_in_if = if_ifwithsock(fd)) == NULL) {
      struct ipaddr_str buf;
      OLSR_PRINTF(1, "Could not find input interface for message from %s size %d\n", olsr_ip_to_string(&buf, &from_addr), cc);
      olsr_syslog(OLSR_LOG_ERR, "Could not find input interface for message from %s size %d\n", olsr_ip_to_string(&buf, &from_addr),
                  cc);
      return;
    }
    // call preprocessors
    entry = preprocessor_functions;
    packet = &inbuf[0];

    while (entry) {
      packet = entry->function(packet, olsr_in_if, &from_addr, &cc);
      // discard package ?
      if (packet == NULL) {
        return;
      }
      entry = entry->next;
    }

    /*
     * &from - sender
     * &inbuf.olsr
     * cc - bytes read
     */
    parse_packet((struct olsr *)packet, cc, olsr_in_if, &from_addr);

  }
}

/**
 *Processing OLSR data from socket. Reading data, setting
 *wich interface recieved the message, Sends IPC(if used)
 *and passes the packet on to parse_packet().
 *
 *@param fd the filedescriptor that data should be read from.
 *@return nada
 */
void
olsr_input_hostemu(int fd)
{
  /* sockaddr_in6 is bigger than sockaddr !!!! */
  struct sockaddr_storage from;
  socklen_t fromlen;
  struct interface *olsr_in_if;
  union olsr_ip_addr from_addr;
  uint16_t pcklen;
  struct preprocessor_function_entry *entry;
  char *packet;

  /* Host emulator receives IP address first to emulate
     direct link */

  int cc = recv(fd, from_addr.v6.s6_addr, olsr_cnf->ipsize, 0);
  if (cc != (int)olsr_cnf->ipsize) {
    fprintf(stderr, "Error receiving host-client IP hook(%d) %s!\n", cc, strerror(errno));
    memcpy(&from_addr, &((struct olsr *)inbuf)->olsr_msg->originator, olsr_cnf->ipsize);
  }

  /* are we talking to ourselves? */
  if (if_ifwithaddr(&from_addr) != NULL)
    return;

  /* Extract size */
  if ((cc = recv(fd, (void *)&pcklen, 2, MSG_PEEK)) != 2) {     /* Win needs a cast */
    if (cc <= 0) {
      fprintf(stderr, "Lost olsr_switch connection - exit!\n");
      olsr_exit(__func__, EXIT_FAILURE);
    }
    fprintf(stderr, "[hust-emu] error extracting size(%d) %s!\n", cc, strerror(errno));
    return;
  } else {
    pcklen = ntohs(pcklen);
  }

  fromlen = sizeof(struct sockaddr_storage);

  cc = olsr_recvfrom(fd, inbuf, pcklen, 0, (struct sockaddr *)&from, &fromlen);

  if (cc <= 0) {
    if (cc < 0 && errno != EWOULDBLOCK) {
      const char *const err_msg = strerror(errno);
      OLSR_PRINTF(1, "error recvfrom: %s", err_msg);
      olsr_syslog(OLSR_LOG_ERR, "error recvfrom: %s", err_msg);
    }
    return;
  }

  if (cc != pcklen) {
    printf("Could not read whole packet(size %d, read %d)\n", pcklen, cc);
    return;
  }

  if ((olsr_in_if = if_ifwithsock(fd)) == NULL) {
    struct ipaddr_str buf;
    OLSR_PRINTF(1, "Could not find input interface for message from %s size %d\n", olsr_ip_to_string(&buf, &from_addr), cc);
    olsr_syslog(OLSR_LOG_ERR, "Could not find input interface for message from %s size %d\n", olsr_ip_to_string(&buf, &from_addr),
                cc);
    return;
  }
  // call preprocessors
  entry = preprocessor_functions;
  packet = &inbuf[0];

  while (entry) {
    packet = entry->function(packet, olsr_in_if, &from_addr, &cc);
    // discard package ?
    if (packet == NULL) {
      return;
    }
    entry = entry->next;
  }

  /*
   * &from - sender
   * &inbuf.olsr
   * cc - bytes read
   */
  parse_packet((struct olsr *)inbuf, cc, olsr_in_if, &from_addr);

}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
