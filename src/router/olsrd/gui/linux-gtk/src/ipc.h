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

#include "../../../src/ipc_frontend.h"
#include "../../../src/olsr_protocol.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "packet.h"

#define IPC_PORT 1212
#define	IPC_MESSAGE 11          /* IPC to front-end telling of route changes */
#define IPC_NET 12              /* IPC to front end net-info */

//int ipc_socket;
extern int connected;

/*
 *AND:
 *IPC message sent to the front-end
 *at every route update. Both delete
 *and add
 */

struct routemsg {
  olsr_u8_t msgtype;
  olsr_u16_t size;
  olsr_u8_t metric;
  olsr_u8_t add;
  union olsr_ip_addr target_addr;
  union olsr_ip_addr gateway_addr;
  char device[4];
};

struct netmsg {
  olsr_u8_t msgtype;
  olsr_u16_t size;
  olsr_u8_t mids;                      /* No. of extra interfaces */
  olsr_u8_t hnas;                      /* No. of HNA nets */
  olsr_u8_t unused1;
  olsr_u16_t hello_int;
  olsr_u16_t hello_lan_int;
  olsr_u16_t tc_int;
  olsr_u16_t neigh_hold;
  olsr_u16_t topology_hold;
  olsr_u8_t ipv6;
  union olsr_ip_addr main_addr;
};

/*
 *Private functions
 */

int ipc_get_socket(void);

int ipc_evaluate_message(union olsr_message *);

int ipc_eval_route_packet(struct routemsg *);

int ipc_eval_net_info(struct netmsg *);

int process_hello(int, olsr_u8_t, union olsr_ip_addr *, union hello_message *);

int process_tc(int, olsr_u8_t, union olsr_ip_addr *, union tc_message *);

int process_mid(int, olsr_u8_t, union olsr_ip_addr *, union mid_message *);

int process_hna(int, olsr_u8_t, union olsr_ip_addr *, union hna_message *);

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
