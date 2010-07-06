/*
 * dummy.c
 *
 *  Created on: 12.02.2010
 *      Author: henning
 */

#include "../defs.h"
#include "../kernel_routes.h"
#include "../kernel_tunnel.h"
#include "../net_os.h"

int olsr_os_init_iptunnel(void) {
  return -1;
}

void olsr_os_cleanup_iptunnel(void) {
}

struct olsr_iptunnel_entry *olsr_os_add_ipip_tunnel(union olsr_ip_addr *target __attribute__ ((unused)),
    bool transportV4 __attribute__ ((unused))) {
  return NULL;
}

void olsr_os_del_ipip_tunnel(struct olsr_iptunnel_entry *t __attribute__ ((unused))) {
  return;
}

bool olsr_if_isup(const char * dev __attribute__ ((unused))) {
  return false;
}

int olsr_if_set_state(const char *dev __attribute__ ((unused)),
    bool up __attribute__ ((unused))) {
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
    bool set __attribute__ ((unused))) {
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
