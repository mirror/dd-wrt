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

#ifndef _NET_OLSR
#define _NET_OLSR

#include "olsr_types.h"
#include "interfaces.h"
#include "process_routes.h"

#include <arpa/inet.h>
#include <net/if.h>

typedef int (*packet_transform_function) (uint8_t *, int *);

void init_net(void);

int net_add_buffer(struct interface_olsr *);

int net_remove_buffer(struct interface_olsr *);

int net_outbuffer_bytes_left(const struct interface_olsr *);

uint16_t net_output_pending(const struct interface_olsr *);

int net_reserve_bufspace(struct interface_olsr *, int);

int net_outbuffer_push(struct interface_olsr *, const void *, const uint16_t);

int net_outbuffer_push_reserved(struct interface_olsr *, const void *, const uint16_t);

int net_output(struct interface_olsr *);

int net_sendroute(struct rt_entry *, struct sockaddr *);

int add_ptf(packet_transform_function);

int del_ptf(packet_transform_function);

bool olsr_validate_address(const union olsr_ip_addr *);

void olsr_add_invalid_address(const union olsr_ip_addr *);

#endif /* _NET_OLSR */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
