/*
 * Broadcom UPnP module linux specific OSL include file
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_linux_osl.h,v 1.9 2008/06/20 05:23:56 Exp $
 */

#ifndef __UPNP_LINUX_OSL_H__
#define __UPNP_LINUX_OSL_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>

#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <time.h>
#include <stdarg.h>

#define __KERNEL__
#include <asm/types.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>

typedef struct _if_stats {
	unsigned long rx_packets; /* total packets received       */
	unsigned long tx_packets; /* total packets transmitted    */
	unsigned long rx_bytes; /* total bytes received         */
	unsigned long tx_bytes; /* total bytes transmitted      */
	unsigned long rx_errors; /* bad packets received         */
	unsigned long tx_errors; /* packet transmit problems     */
	unsigned long rx_dropped; /* no space in linux buffers    */
	unsigned long tx_dropped; /* no space available in linux  */
	unsigned long rx_multicast; /* multicast packets received   */
	unsigned long rx_compressed;
	unsigned long tx_compressed;
	unsigned long collisions;

	/* detailed rx_errors: */
	unsigned long rx_length_errors;
	unsigned long rx_over_errors; /* receiver ring buff overflow  */
	unsigned long rx_crc_errors; /* recved pkt with crc error    */
	unsigned long rx_frame_errors; /* recv'd frame alignment error */
	unsigned long rx_fifo_errors; /* recv'r fifo overrun          */
	unsigned long rx_missed_errors; /* receiver missed packet       */

	/* detailed tx_errors */
	unsigned long tx_aborted_errors;
	unsigned long tx_carrier_errors;
	unsigned long tx_fifo_errors;
	unsigned long tx_heartbeat_errors;
	unsigned long tx_window_errors;

} if_stats_t;

#undef __KERNEL__

#define upnp_syslog(a, b...) \
	do {                 \
	} while (0)

#define upnp_pid() (int)getpid()
#define upnp_sleep(n) sleep(n)

int upnp_osl_ifname_list(char *ifname_list);
int upnp_osl_ifaddr(const char *ifname, struct in_addr *inaddr);
int upnp_osl_netmask(const char *ifname, struct in_addr *inaddr);
int upnp_osl_hwaddr(const char *ifname, char *mac);
int upnp_open_udp_socket(struct in_addr addr, unsigned short port);
int upnp_open_tcp_socket(struct in_addr addr, unsigned short port);

#define oslib_ifname_list(a) upnp_osl_ifname_list(a)
#define oslib_ifaddr(ifn, a) upnp_osl_ifaddr(ifn, a)
#define oslib_netmask(ifn, a) upnp_osl_netmask(ifn, a)
#define oslib_hwaddr(ifn, a) upnp_osl_hwaddr(ifn, a)
#define oslib_udp_socket(a, p) upnp_open_udp_socket(a, p);
#define oslib_tcp_socket(a, p) upnp_open_tcp_socket(a, p);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UPNP_LINUX_OSL_H__ */
