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
#include "net_olsr.h"
#include "duplicate_handler.h"

#ifdef _WIN32
#undef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#undef errno
#define errno WSAGetLastError()
char *StrError(unsigned int ErrNo);
#undef strerror
#define strerror(x) StrError(x)
#endif /* _WIN32 */

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
olsr_destroy_parser(void) {
  struct parse_function_entry *pe, *pe_next;
  struct preprocessor_function_entry *ppe, *ppe_next;
  struct packetparser_function_entry *pae, *pae_next;

  for (pe = parse_functions; pe; pe = pe_next) {
    pe_next = pe->next;
    free (pe);
  }
  for (ppe = preprocessor_functions; ppe; ppe = ppe_next) {
    ppe_next = ppe->next;
    free (ppe);
  }
  for (pae = packetparser_functions; pae; pae = pae_next) {
    pae_next = pae->next;
    free(pae);
  }
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
 *@param olsr the olsr struct containing the message
 *@param size the size of the message
 *@param in_if the incoming interface
 *@param from_addr the sockaddr struct describing the sender
 */

void
parse_packet(struct olsr *olsr, int size, struct interface_olsr *in_if, union olsr_ip_addr *from_addr)
{
  union olsr_message *m = (union olsr_message *)olsr->olsr_msg;
  uint32_t count;
  uint32_t msgsize;
  uint16_t seqno;
  struct parse_function_entry *entry;
  struct packetparser_function_entry *packetparser;

  count = size - ((char *)m - (char *)olsr);

  /* minimum packet size is 4 */
  if (count < 4)
    return;

  if (ntohs(olsr->olsr_packlen) !=(uint16_t) size) {
    struct ipaddr_str buf;
    OLSR_PRINTF(1, "Size error detected in received packet.\nReceived %d, in packet %d\n", size, ntohs(olsr->olsr_packlen));

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

  /*
   * Hysteresis update - for every OLSR package
   */
  if (olsr_cnf->use_hysteresis) {
    /* IPv4 & IPv6 */
    update_hysteresis_incoming(from_addr, in_if, olsr->olsr_seqno);
  }

  for (; count > 0; m = (union olsr_message *)((char *)m + (msgsize))) {
    bool forward = true;
    bool validated;

    /* minimum message size is 8 + ipsize */
    if (count < 8 + olsr_cnf->ipsize)
      break;

    if (olsr_cnf->ip_version == AF_INET) {
      msgsize = ntohs(m->v4.olsr_msgsize);
      seqno = ntohs(m->v4.seqno);
    }
    else {
      msgsize = ntohs(m->v6.olsr_msgsize);
      seqno = ntohs(m->v6.seqno);
    }

    /* sanity check for msgsize */
    if (msgsize < 8 + olsr_cnf->ipsize) {
      struct ipaddr_str buf;
      union olsr_ip_addr *msgorig = (union olsr_ip_addr *) &m->v4.originator;
      OLSR_PRINTF(1, "Error, OLSR message from %s (type %d) is to small (%d bytes)"
          ", ignoring all further content of the packet\n",
          olsr_ip_to_string(&buf, msgorig), m->v4.olsr_msgtype, msgsize);
      olsr_syslog(OLSR_LOG_ERR, "Error, OLSR message from %s (type %d) is too small (%d bytes)"
          ", ignoring all further content of the packet\n",
          olsr_ip_to_string(&buf, msgorig), m->v4.olsr_msgtype, msgsize);
      break;
    }

    if ((msgsize % 4) != 0) {
      struct ipaddr_str buf;
      union olsr_ip_addr *msgorig = (union olsr_ip_addr *) &m->v4.originator;
      OLSR_PRINTF(1, "Error, OLSR message from %s (type %d) must be"
          " longword aligned, but has a length of %d bytes\n",
          olsr_ip_to_string(&buf, msgorig), m->v4.olsr_msgtype, msgsize);
      olsr_syslog(OLSR_LOG_ERR, "Error, OLSR message from %s (type %d) must be"
          " longword aligned, but has a length of %d bytes",
          olsr_ip_to_string(&buf, msgorig), m->v4.olsr_msgtype, msgsize);
      break;
    }

    if (msgsize > count) {
      struct ipaddr_str buf;
      union olsr_ip_addr *msgorig = (union olsr_ip_addr *) &m->v4.originator;
      OLSR_PRINTF(1, "Error, OLSR message from %s (type %d) says"
          " length=%d, but only %d bytes left\n",
          olsr_ip_to_string(&buf, msgorig), m->v4.olsr_msgtype, msgsize, count);
      olsr_syslog(OLSR_LOG_ERR, "Error, OLSR message from %s (type %d) says"
          " length=%d, but only %d bytes left",
          olsr_ip_to_string(&buf, msgorig), m->v4.olsr_msgtype, msgsize, count);
      break;
    }

    count -= msgsize;

    /*RFC 3626 section 3.4:
     *  2    If the time to live of the message is less than or equal to
     *  '0' (zero), or if the message was sent by the receiving node
     *  (i.e., the Originator Address of the message is the main
     *  address of the receiving node): the message MUST silently be
     *  dropped.
     */

    /* Should be the same for IPv4 and IPv6 */
    validated = olsr_validate_address((union olsr_ip_addr *)&m->v4.originator);
    if (ipequal((union olsr_ip_addr *)&m->v4.originator, &olsr_cnf->main_addr) || !validated) {
#ifdef DEBUG
      struct ipaddr_str buf;
      OLSR_PRINTF(3, "Not processing message originating from %s!\n",
                  olsr_ip_to_string(&buf, (union olsr_ip_addr *)&m->v4.originator));
#endif /* DEBUG */
#ifndef NO_DUPLICATE_DETECTION_HANDLER
      if (validated) {
        olsr_test_originator_collision(m->v4.olsr_msgtype, seqno);
      }
#endif /* NO_DUPLICATE_DETECTION_HANDLER */
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
 *wich interface received the message, Sends IPC(if used)
 *and passes the packet on to parse_packet().
 *
 *@param fd the filedescriptor that data should be read from.
 *@param data unused
 *@param flags unused
 */
void
olsr_input(int fd, void *data __attribute__ ((unused)), unsigned int flags __attribute__ ((unused)))
{
  struct interface_olsr *olsr_in_if;
  union olsr_ip_addr from_addr;
  struct preprocessor_function_entry *entry;
  char *packet;

  cpu_overload_exit = 0;

  for (;;) {
    struct ipaddr_str buf;
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
#ifndef _WIN32
        olsr_syslog(OLSR_LOG_ERR, "error recvfrom: %m");
#endif /* _WIN32 */
      }
      break;
    }

    {
      void * src;
      void * dst;
      size_t size;
      if (olsr_cnf->ip_version == AF_INET) {
        /* IPv4 sender address */
        struct sockaddr_in * x = (struct sockaddr_in *) &from;
        src = &x->sin_addr;
        dst = &from_addr.v4;
        size = sizeof(from_addr.v4);
      } else {
        /* IPv6 sender address */
        struct sockaddr_in6 * x = (struct sockaddr_in6 *) &from;
        src = &x->sin6_addr;
        dst = &from_addr.v6;
        size = sizeof(from_addr.v6);
      }
      memcpy(dst, src, size);
    }

#ifdef DEBUG
    OLSR_PRINTF(5, "Received a packet from %s\n",
        olsr_ip_to_string(&buf, &from_addr));
#endif /* DEBUG */

    if ((olsr_cnf->ip_version == AF_INET) && (fromlen != sizeof(struct sockaddr_in)))
      break;
    else if ((olsr_cnf->ip_version == AF_INET6) && (fromlen != sizeof(struct sockaddr_in6)))
      break;

    /* are we talking to ourselves? */
    if (if_ifwithaddr(&from_addr) != NULL)
      return;

    if ((olsr_in_if = if_ifwithsock(fd)) == NULL) {
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
 *wich interface received the message, Sends IPC(if used)
 *and passes the packet on to parse_packet().
 *
 *@param fd the filedescriptor that data should be read from.
 *@param data unused
 *@param flags unused
 */
void
olsr_input_hostemu(int fd, void *data __attribute__ ((unused)), unsigned int flags __attribute__ ((unused)))
{
  /* sockaddr_in6 is bigger than sockaddr !!!! */
  struct sockaddr_storage from;
  socklen_t fromlen;
  struct interface_olsr *olsr_in_if;
  union olsr_ip_addr from_addr;
  uint16_t pcklen;
  struct preprocessor_function_entry *entry;
  char *packet;

  /* Host emulator receives IP address first to emulate
     direct link */

  int cc = recv(fd, (void*)from_addr.v6.s6_addr, olsr_cnf->ipsize, 0);
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
      char buf[1024];
      snprintf(buf, sizeof(buf), "%s: Lost olsr_switch connection", __func__);
      olsr_exit(buf, EXIT_FAILURE);
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
