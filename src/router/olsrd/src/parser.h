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

#ifndef _OLSR_MSG_PARSER
#define _OLSR_MSG_PARSER

#include "olsr_protocol.h"
#include "packet.h"

#define PROMISCUOUS 0xffffffff

/* Function returns false if the message should not be forwarded */
typedef bool parse_function(union olsr_message *, struct interface_olsr *, union olsr_ip_addr *);

struct parse_function_entry {
  uint32_t type;                       /* If set to PROMISCUOUS all messages will be received */
  parse_function *function;
  struct parse_function_entry *next;
};

typedef char *preprocessor_function(char *packet, struct interface_olsr *, union olsr_ip_addr *, int *length);

struct preprocessor_function_entry {
  preprocessor_function *function;
  struct preprocessor_function_entry *next;
};

typedef void packetparser_function(struct olsr *olsr, struct interface_olsr *in_if, union olsr_ip_addr *from_addr);

struct packetparser_function_entry {
  packetparser_function *function;
  struct packetparser_function_entry *next;
};

void olsr_init_parser(void);

void olsr_destroy_parser(void);

void olsr_input(int fd, void *, unsigned int);

void olsr_input_hostemu(int fd, void *, unsigned int);

void olsr_parser_add_function(parse_function, uint32_t);

int olsr_parser_remove_function(parse_function, uint32_t);

void olsr_preprocessor_add_function(preprocessor_function);

int olsr_preprocessor_remove_function(preprocessor_function);

void olsr_packetparser_add_function(packetparser_function * function);

int olsr_packetparser_remove_function(packetparser_function * function);

void parse_packet(struct olsr *, int, struct interface_olsr *, union olsr_ip_addr *);

#endif /* _OLSR_MSG_PARSER */
