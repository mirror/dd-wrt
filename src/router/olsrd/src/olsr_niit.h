/*
 * olsr_niit.h
 *
 *  Created on: 02.02.2010
 *      Author: henning
 */

#ifndef OLSR_NIIT_H_
#define OLSR_NIIT_H_

#include "defs.h"
#include "routing_table.h"

#define DEF_NIIT4TO6_IFNAME         "niit4to6"
#define DEF_NIIT6TO4_IFNAME         "niit6to4"

#ifdef LINUX_NETLINK_ROUTING
void olsr_init_niit(void);
void olsr_setup_niit_routes(void);
void olsr_cleanup_niit_routes(void);

void olsr_niit_handle_route(const struct rt_entry *rt, bool set);
#endif

#endif /* OLSR_NIIT_H_ */
