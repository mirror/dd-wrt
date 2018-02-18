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

#include "defs.h"
#include "kernel_routes.h"
#include "kernel_tunnel.h"
#include "net_os.h"

/* prototypes: have them here or disable the warnings about missing prototypes! */
int olsr_if_setip(const char *dev __attribute__ ((unused)), union olsr_ip_addr *ip __attribute__ ((unused)), int ipversion __attribute__ ((unused))); 



int olsr_os_init_iptunnel(const char * name __attribute__((unused))) {
  return -1;
}

void olsr_os_cleanup_iptunnel(const char * name __attribute__((unused))) {
}

struct olsr_iptunnel_entry *olsr_os_add_ipip_tunnel(union olsr_ip_addr *target __attribute__ ((unused)),
    bool transportV4 __attribute__ ((unused)), char *name __attribute__((unused))) {
  return NULL;
}

void olsr_os_del_ipip_tunnel(struct olsr_iptunnel_entry *t __attribute__ ((unused))) {
  return;
}

bool olsr_if_isup(const char * dev __attribute__ ((unused))) {
  return false;
}

int olsr_if_setip(const char *dev __attribute__ ((unused)),
    union olsr_ip_addr *ip __attribute__ ((unused)),
    int ipversion __attribute__ ((unused))) {
  return -1;
}

void olsr_os_niit_4to6_route(const struct olsr_ip_prefix *dst_v4 __attribute__ ((unused)),
    bool set __attribute__ ((unused))) {
}
void olsr_os_niit_6to4_route(const struct olsr_ip_prefix *dst_v6 __attribute__ ((unused)),
    bool set __attribute__ ((unused))) {
}
void olsr_os_inetgw_tunnel_route(uint32_t if_idx __attribute__ ((unused)),
    bool ipv4 __attribute__ ((unused)),
    bool set __attribute__ ((unused)),
    uint8_t table __attribute__ ((unused))) {
}

int olsr_os_policy_rule(int family __attribute__ ((unused)),
    int rttable __attribute__ ((unused)),
    uint32_t priority __attribute__ ((unused)),
    const char *if_name __attribute__ ((unused)),
    bool set __attribute__ ((unused))) {
  return -1;
}

int olsr_os_localhost_if(union olsr_ip_addr *ip __attribute__ ((unused)),
    bool create __attribute__ ((unused))) {
  return -1;
}

int olsr_os_ifip(int ifindex __attribute__ ((unused)),
    union olsr_ip_addr *ip __attribute__ ((unused)), bool create __attribute__ ((unused))) {
  return -1;
}
