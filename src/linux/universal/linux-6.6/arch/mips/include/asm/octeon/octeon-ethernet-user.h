/**********************************************************************
 * Author: Cavium, Inc.
 *
 * Contact: support@cavium.com
 * This file is part of the OCTEON SDK
 *
 * Copyright (c) 2010-2012 Cavium, Inc.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 *
 * This file is distributed in the hope that it will be useful, but
 * AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or
 * NONINFRINGEMENT.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 * or visit http://www.gnu.org/licenses/.
 *
 * This file may also be available under a different license from Cavium.
 * Contact Cavium, Inc. for more information
 **********************************************************************/

/*
 * Device specific IOCTL interface for the Cavium Octeon ethernet driver.
 *
 */
#ifndef OCTEON_ETHERNET_USER_H
#define OCTEON_ETHERNET_USER_H

/*
 * Each of these IOCTLs use the field "ifr_ifru.ifru_ivalue" inside
 * "struct ifreq" to move data. When calling these IOCTLS, supply a
 * "struct ifreq" as the third parameter. The second parameter will be
 * one of the IOCTL numbers defined below.
 */

#define CAVIUM_NET_IOCTL_SETPRIO   (SIOCDEVPRIVATE + 0) /* Priority 0-3. Default is 0 */
#define CAVIUM_NET_IOCTL_GETPRIO   (SIOCDEVPRIVATE + 1) /* Priority 0-3 */
#define CAVIUM_NET_IOCTL_SETIDSIZE (SIOCDEVPRIVATE + 2) /* 0 = 8 bit, 1 = 16 bit IDs. Default is 1 */
#define CAVIUM_NET_IOCTL_GETIDSIZE (SIOCDEVPRIVATE + 3) /* 0 = 8 bit, 1 = 16 bit IDs */
#define CAVIUM_NET_IOCTL_SETSRCID  (SIOCDEVPRIVATE + 4) /* 0 = primary ID, 1 = secondary ID. Default is 0 */
#define CAVIUM_NET_IOCTL_GETSRCID  (SIOCDEVPRIVATE + 5) /* 0 = primary ID, 1 = secondary ID */
#define CAVIUM_NET_IOCTL_SETLETTER (SIOCDEVPRIVATE + 6) /* Letter code 0-3, or -1 for auto. Default is -1 */
#define CAVIUM_NET_IOCTL_GETLETTER (SIOCDEVPRIVATE + 7) /* Letter code 0-3, or -1 for auto */

#ifdef __KERNEL__

/**
 * enum cvm_oct_callback_result -  Return codes for the Ethernet* driver intercept callback.
 *
 * Depending on the return code, the ethernet driver will continue
 * processing in different ways.
 */
enum cvm_oct_callback_result {
	CVM_OCT_PASS,               /**< The ethernet driver will pass the packet
					to the kernel, just as if the intercept
					callback didn't exist */
	CVM_OCT_DROP,               /**< The ethernet driver will drop the packet,
					cleaning of the work queue entry and the
					skbuff */
	CVM_OCT_TAKE_OWNERSHIP_WORK,/**< The intercept callback takes over
					ownership of the work queue entry. It is
					the responsibility of the callback to free
					the work queue entry and all associated
					packet buffers. The ethernet driver will
					dispose of the skbuff without affecting the
					work queue entry */
	CVM_OCT_TAKE_OWNERSHIP_SKB  /**< The intercept callback takes over
					ownership of the skbuff. The work queue
					entry and packet buffer will be disposed of
					in a way keeping the skbuff valid */
};
typedef enum cvm_oct_callback_result cvm_oct_callback_result_t;

/**
 * cvm_oct_callback_result_t -  Ethernet driver intercept callback hook type.
 *
 * The callback receives three parameters and returns a struct
 * cvm_oct_callback_result code.
 *
 * The first parameter is the linux device for the ethernet port the
 * packet came in on.
 *
 * The second parameter is the raw work queue entry from the hardware.
 *
 * The third parameter is the packet converted into a Linux skbuff.
 */
typedef cvm_oct_callback_result_t (*cvm_oct_callback_t)(struct net_device *dev,
							void *work_queue_entry,
							struct sk_buff *skb);

extern struct net_device *cvm_oct_register_callback(const char *, cvm_oct_callback_t);
extern struct net_device *is_oct_dev(const char *device_name);

#ifdef CONFIG_CAVIUM_NET_PACKET_FWD_OFFLOAD
extern int cvm_oct_transmit_qos(struct net_device *dev,
				void *work_queue_entry,
				int do_free,
				int qos);

extern int cvm_oct_transmit_qos_not_free(struct net_device *dev,
					 void *work_queue_entry,
					 struct sk_buff *skb);

extern struct net_device *octeon3_register_callback(
		const char	*device_name, cvm_oct_callback_t callback);

extern struct net_device *octeon3_is_oct_dev(const char *device_name);

extern int octeon3_transmit_qos(struct net_device *dev,
				void *work,
				int do_free,
				int qos);
#else
extern int cvm_oct_transmit_qos(struct net_device *dev, void *work_queue_entry,
			 int do_free, int qos);
#endif
#endif /* __KERNEL__ */

#endif
