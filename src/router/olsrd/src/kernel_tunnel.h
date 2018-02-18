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

#ifndef KERNEL_TUNNEL_H_
#define KERNEL_TUNNEL_H_

#include <net/if.h>
#ifdef _WIN32
/* compat for win32 */
#include <iprtrmib.h>
#define IF_NAMESIZE MAX_INTERFACE_NAME_LEN
#endif /* _WIN32 */

#include "defs.h"
#include "olsr_types.h"
#include "common/avl.h"

#define TUNNEL_ENDPOINT_IF "tunl0"
#define TUNNEL_ENDPOINT_IF6 "ip6tnl0"

#ifdef __ANDROID__
  #define OS_TUNNEL_PATH "/dev/tun"
#else
  #define OS_TUNNEL_PATH "/dev/net/tun"
#endif

struct olsr_iptunnel_entry {
  struct avl_node node;
  union olsr_ip_addr target;

  char if_name[IF_NAMESIZE];
  int if_index;

  int usage;
};

int olsr_os_init_iptunnel(const char * name);
void olsr_os_cleanup_iptunnel(const char * name);

struct olsr_iptunnel_entry *olsr_os_add_ipip_tunnel(union olsr_ip_addr *target, bool transportV4, char *name);
void olsr_os_del_ipip_tunnel(struct olsr_iptunnel_entry *);

#ifdef __linux__
int os_ip_tunnel(const char *name, void *target);
#endif /* __linux__ */

#endif /* KERNEL_TUNNEL_H_ */
