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

#ifndef _ROUTER_ELECTION_H_
#define _ROUTER_ELECTION_H_

#include <netinet/in.h>

#define ELECTION_TIMER		15
#define HELLO_TIMER		20
#define INIT_TIMER		1
#define ENTRYTTL		10

struct RtElHelloPkt{
  char head[4]; //"$REP"
  int ipFamily;
  union olsr_ip_addr router_id;
  uint8_t network_id;
} __attribute__((__packed__));

struct RouterListEntry{
  struct in_addr router_id;
  uint8_t network_id;
  int ttl;
  int skfd;

  struct list_entity list;
};

struct RouterListEntry6{
  struct in6_addr router_id;
  uint8_t network_id;
  int ttl;
  int skfd;

  struct list_entity list;
};

int UpdateRouterList (struct RouterListEntry *listEntry);	//update router list
int UpdateRouterList6 (struct RouterListEntry6 *listEntry6);
int ParseElectionPacket (struct RtElHelloPkt *rcvPkt, struct RouterListEntry *listEntry, int skfd);	//used to parse a received 
int ParseElectionPacket6 (struct RtElHelloPkt *rcvPkt, struct RouterListEntry6 *listEntry6, int skfd);	//packet into a list entry
int InitRouterList (void *foo __attribute__ ((unused)));
void helloTimer (void *foo __attribute__ ((unused)));
void electTimer (void *foo __attribute__ ((unused)));
void initTimer (void *foo __attribute__ ((unused)));
int set_Network_ID(const char *Network_ID, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)));

#endif

