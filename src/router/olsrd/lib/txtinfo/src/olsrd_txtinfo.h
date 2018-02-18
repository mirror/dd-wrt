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

#ifndef LIB_TXTINFO_SRC_OLSRD_TXTINFO_H_
#define LIB_TXTINFO_SRC_OLSRD_TXTINFO_H_

#include <stdbool.h>

#include "common/autobuf.h"

unsigned long long get_supported_commands_mask(void);
bool isCommand(const char *str, unsigned long long siw);
void output_error(struct autobuf *abuf, unsigned int status, const char * req, bool http_headers);

void ipc_print_neighbors(struct autobuf *abuf);
void ipc_print_links(struct autobuf *abuf);
void ipc_print_routes(struct autobuf *abuf);
void ipc_print_topology(struct autobuf *abuf);
void ipc_print_hna(struct autobuf *abuf);
void ipc_print_mid(struct autobuf *abuf);
void ipc_print_gateways(struct autobuf *abuf);
void ipc_print_sgw(struct autobuf *abuf);
void ipc_print_version(struct autobuf *abuf);
void ipc_print_olsrd_conf(struct autobuf *abuf);
void ipc_print_interfaces(struct autobuf *abuf);
void ipc_print_twohop(struct autobuf *abuf);

#endif /* LIB_TXTINFO_SRC_OLSRD_TXTINFO_H_ */
