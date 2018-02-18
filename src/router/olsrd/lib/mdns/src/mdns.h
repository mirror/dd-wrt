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


#ifndef _MDNS_MDNS_H
#define _MDNS_MDNS_H


#include "olsrd_plugin.h"             /* union set_plugin_parameter_addon */
#include "duplicate_set.h"
#include "parser.h"
#include "list_backport.h"

#define MESSAGE_TYPE 132
#define PARSER_TYPE		MESSAGE_TYPE
#define EMISSION_INTERVAL       10      /* seconds */
#define EMISSION_JITTER         25      /* percent */
#define MDNS_VALID_TIME          1800   /* seconds */

/* BMF plugin data */
#define PLUGIN_NAME              "OLSRD mdns plugin"
#define PLUGIN_NAME_SHORT        "MDNS"
#define PLUGIN_INTERFACE_VERSION 5

/* UDP-Port on which multicast packets are encapsulated */
//#define BMF_ENCAP_PORT 50698

/* Forward declaration of OLSR interface type */
struct interface_olsr;

struct FilteredHost{
  union olsr_ip_addr host;

  struct list_entity list;
};

//extern int FanOutLimit;
//extern int BroadcastRetransmitCount;

void DoMDNS(int sd, void *x, unsigned int y);
void DoElection(int skfd, void *x, unsigned int y);
void BmfPError(const char *format, ...) __attribute__ ((format(printf, 1, 2)));
union olsr_ip_addr *MainAddressOf(union olsr_ip_addr *ip);
//int InterfaceChange(struct interface_olsr * interf, int action);
//int SetFanOutLimit(const char* value, void* data, set_plugin_parameter_addon addon);
//int InitBmf(struct interface_olsr * skipThisIntf);
//void CloseBmf(void);
int InitMDNS(struct interface_olsr *skipThisIntf);
void CloseMDNS(void);
int AddFilteredHost(const char *FilteredHost, void *data __attribute__ ((unused)),
			 set_plugin_parameter_addon addon __attribute__ ((unused)));
int isInFilteredList(union olsr_ip_addr *src);
void olsr_mdns_gen(unsigned char *packet, int len);

/* Parser function to register with the scheduler */
bool olsr_parser(union olsr_message *, struct interface_olsr *, union olsr_ip_addr *);


#endif /* _MDNS_MDNS_H */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
