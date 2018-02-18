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

#ifndef _OLSR_KERNEL_RT
#define _OLSR_KERNEL_RT

#include "defs.h"
#include "routing_table.h"

int olsr_ioctl_add_route(const struct rt_entry *rt);

int olsr_ioctl_add_route6(const struct rt_entry *rt);

int olsr_ioctl_del_route(const struct rt_entry *rt);

int olsr_ioctl_del_route6(const struct rt_entry *rt);

#ifdef __linux__
  int olsr_new_netlink_route(unsigned char family, uint32_t rttable,
    unsigned int flags, unsigned char scope, int if_index, int metric,
    int protocol, const union olsr_ip_addr *src, const union olsr_ip_addr *gw,
    const struct olsr_ip_prefix *dst, bool set, bool del_similar, bool blackhole);

  int rtnetlink_register_socket(int);
#endif /* __linux__ */

void olsr_os_niit_4to6_route(const struct olsr_ip_prefix *dst_v4, bool set);
void olsr_os_niit_6to4_route(const struct olsr_ip_prefix *dst_v6, bool set);
void olsr_os_inetgw_tunnel_route(uint32_t if_idx, bool ipv4, bool set, uint8_t table);

int olsr_os_policy_rule(int family, int rttable, uint32_t priority, const char *if_name, bool set);
int olsr_os_localhost_if(union olsr_ip_addr *ip, bool create);
int olsr_os_ifip(int ifindex, union olsr_ip_addr *ip, bool create);

#endif /* _OLSR_KERNEL_RT */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
