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
 * olsr.org olsr daemon security plugin
 */

#ifndef _OLSRD_SECURE_MSG
#define _OLSRD_SECURE_MSG

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <stdio.h>

#include "olsr_types.h"
#include "interfaces.h"

/* The type of message you will use */
#define MESSAGE_TYPE 10

/* The type of messages we will receive - can be set to promiscuous */
#define PARSER_TYPE MESSAGE_TYPE

#define TYPE_CHALLENGE 11
#define TYPE_CRESPONSE 12
#define TYPE_RRESPONSE 13

extern char keyfile[FILENAME_MAX + 1];

#ifdef USE_OPENSSL
#define SIGSIZE   20
#else /* USE_OPENSSL */
#define SIGSIZE   16
#endif /* USE_OPENSSL */

/****************************************************************************
 *                            PACKET SECTION                                *
 ****************************************************************************/

struct sig_msg {
  uint8_t type;
  uint8_t algorithm;
  uint16_t reserved;

  time_t timestamp;
  uint8_t signature[SIGSIZE];
};

/*
 * OLSR message (several can exist in one OLSR packet)
 */

struct s_olsrmsg {
  uint8_t olsr_msgtype;
  uint8_t olsr_vtime;
  uint16_t olsr_msgsize;
  uint32_t originator;
  uint8_t ttl;
  uint8_t hopcnt;
  uint16_t seqno;

  /* YOUR PACKET GOES HERE */
  struct sig_msg sig;

};

/*
 * Challenge response messages
 */

struct challengemsg {
  uint8_t olsr_msgtype;
  uint8_t olsr_vtime;
  uint16_t olsr_msgsize;
  uint32_t originator;
  uint8_t ttl;
  uint8_t hopcnt;
  uint16_t seqno;

  uint32_t destination;
  uint32_t challenge;

  uint8_t signature[SIGSIZE];

};

struct c_respmsg {
  uint8_t olsr_msgtype;
  uint8_t olsr_vtime;
  uint16_t olsr_msgsize;
  uint32_t originator;
  uint8_t ttl;
  uint8_t hopcnt;
  uint16_t seqno;

  uint32_t destination;
  uint32_t challenge;
  time_t timestamp;

  uint8_t res_sig[SIGSIZE];

  uint8_t signature[SIGSIZE];
};

struct r_respmsg {
  uint8_t olsr_msgtype;
  uint8_t olsr_vtime;
  uint16_t olsr_msgsize;
  uint32_t originator;
  uint8_t ttl;
  uint8_t hopcnt;
  uint16_t seqno;

  uint32_t destination;
  time_t timestamp;

  uint8_t res_sig[SIGSIZE];

  uint8_t signature[SIGSIZE];
};

/*
 *IPv6
 */

struct s_olsrmsg6 {
  uint8_t olsr_msgtype;
  uint8_t olsr_vtime;
  uint16_t olsr_msgsize;
  struct in6_addr originator;
  uint8_t ttl;
  uint8_t hopcnt;
  uint16_t seqno;

  /* YOUR PACKET GOES HERE */
  struct sig_msg sig;
};

/*
 * Generic OLSR packet - DO NOT ALTER
 */

struct s_olsr {
  uint16_t olsr_packlen;             /* packet length */
  uint16_t olsr_seqno;
  struct s_olsrmsg olsr_msg[1];        /* variable messages */
};

struct s_olsr6 {
  uint16_t olsr_packlen;             /* packet length */
  uint16_t olsr_seqno;
  struct s_olsrmsg6 olsr_msg[1];       /* variable messages */
};

#endif /* _OLSRD_SECURE_MSG */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
