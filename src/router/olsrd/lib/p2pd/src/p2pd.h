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


#ifndef _P2PD_H
#define _P2PD_H

#define REMOVE_LOG_DEBUG

// Either #define or #undef the following line to include extra debugging
#undef INCLUDE_DEBUG_OUTPUT

#include "olsrd_plugin.h"             /* union set_plugin_parameter_addon */
#include "duplicate_set.h"
//#include "socket_parser.h"
#include "dllist.h"

#define P2PD_MESSAGE_TYPE         132
#define PARSER_TYPE               P2PD_MESSAGE_TYPE
#define P2PD_VALID_TIME           180		/* seconds */

/* P2PD plugin data */
#define PLUGIN_NAME               "OLSRD p2pd plugin"
#define PLUGIN_NAME_SHORT         "P2PD"
#define PLUGIN_INTERFACE_VERSION  5

#define IPHDR_FRAGMENT_MASK       0xC000

/* Forward declaration of OLSR interface type */
struct interface_olsr;

struct DupFilterEntry {
  int                            ip_version;
  union olsr_ip_addr             address;
  uint16_t                       seqno;
  uint8_t                        msgtype;
  time_t                         creationtime;
};

struct UdpDestPort {
  int                            ip_version;
  union olsr_ip_addr             address;
  uint16_t                       port;
  struct UdpDestPort *           next;
};

extern int P2pdTtl;
extern int P2pdDuplicateTimeout;
extern int HighestSkfd;
extern fd_set InputSet;
extern struct UdpDestPort * UdpDestPortList;
extern struct DuplicateFilterEntry * FilterList;

void DoP2pd(int sd, void *x, unsigned int y);
void P2pdPError(const char *format, ...) __attribute__ ((format(printf, 1, 2)));
union olsr_ip_addr *MainAddressOf(union olsr_ip_addr *ip);
int InitP2pd(struct interface_olsr *skipThisIntf);
void CloseP2pd(void);
int SetP2pdTtl(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)));
int AddUdpDestPort(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)));
bool InUdpDestPortList(int ip_version, union olsr_ip_addr *addr, uint16_t port);
int SetP2pdTtl(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)));
int SetP2pdUseHashFilter(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)));
int SetP2pdUseTtlDecrement(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)));
bool p2pd_message_seen(struct node **head, struct node **tail, union olsr_message *m);
void p2pd_store_message(struct node **head, struct node **tail, union olsr_message *m);
bool p2pd_is_duplicate_message(union olsr_message *msg);

void olsr_p2pd_gen(unsigned char *packet, int len);

/* Parser function to register with the scheduler */
bool olsr_parser(union olsr_message *, struct interface_olsr *, union olsr_ip_addr *);

#endif /* _P2PD_H */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
