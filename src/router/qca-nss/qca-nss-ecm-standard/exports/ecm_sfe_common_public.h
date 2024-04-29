/*
 **************************************************************************
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/**
 * @file ecm_fe_common_public.h
 *	ECM SFE frontend public APIs and data structures.
 */

#ifndef __ECM_SFE_COMMON_PUBLIC_H__
#define __ECM_SFE_COMMON_PUBLIC_H__

/**
 * @addtogroup ecm_sfe_common_subsystem
 * @{
 */

#define ECM_SFE_COMMON_FLOW_L2_ACCEL_ALLOWED (1 << 0)	/**< L2 acceleration is allowed on the flow interface. */
#define ECM_SFE_COMMON_RETURN_L2_ACCEL_ALLOWED (1 << 1)	/**< L2 acceleration is allowed on the return interface. */

/**
 * SFE common 5-tuple for external use.
 */
struct ecm_sfe_common_tuple {
	uint32_t src_addr[4];	/**< Source IP in host order. */
	uint32_t dest_addr[4];	/**< Destination IP in host order. */

	uint16_t src_port;	/**< Source port port in host order. */
	uint16_t dest_port;	/**< Destination port in host order. */
	uint32_t src_ifindex;	/**< Source L2 interface index */
	uint32_t dest_ifindex;	/**< Destination L2 interface index */
	uint8_t protocol;	/**< Next protocol header number. */
	uint8_t ip_ver;		/**< IP version 4 or 6. */
};

/**
 * Callback to which SFE clients will register and return bitmap of values that indicate L2 acceleration for each direction.
 */
typedef uint32_t (*ecm_sfe_common_l2_accel_check_callback_t)(struct ecm_sfe_common_tuple *tuple);

/**
 * Data structure for SFE common callbacks.
 */
struct ecm_sfe_common_callbacks {
	ecm_sfe_common_l2_accel_check_callback_t l2_accel_check;	/**< Callback to decide if L2 acceleration is wanted for the flow. */
};

/**
 * Defuncts an IPv4 5-tuple connection.
 *
 * @param	src_ip		The source IP address.
 * @param	src_port	The source port.
 * @param	dest_ip		The destination IP address.
 * @param	dest_port	The destination port.
 * @param	protocol	The protocol.
 *
 * @return
 * True if defuncted; false if not.
 */
bool ecm_sfe_common_defunct_ipv4_connection(__be32 src_ip, int src_port,
						__be32 dest_ip, int dest_port, int protocol);

/**
 * Defuncts an IPv6 5-tuple connection.
 *
 * @param	src_ip		The source IP address.
 * @param	src_port	The source port.
 * @param	dest_ip		The destination IP address.
 * @param	dest_port	The destination port.
 * @param	protocol	The protocol.
 *
 * @return
 * True if defuncted; false if not.
 */
bool ecm_sfe_common_defunct_ipv6_connection(struct in6_addr *src_ip, int src_port,
						struct in6_addr *dest_ip, int dest_port, int protocol);

/**
 * Defuncts all the connections with this protocol type.
 *
 * @param protocol Protocol type.
 *
 * @return
 * None.
 */
void ecm_sfe_common_defunct_by_protocol(int protocol);

/**
 * Defuncts all the connections with this port number in the correct direction.
 *
 * @param	port		The port number.
 * @param	direction 	The direction of the port (source (1) or destination (2))
 * @param	wan_name	The WAN port interface name.
 *
 * @return
 * None.
 */
void ecm_sfe_common_defunct_by_port(int port, int direction, char *wan_name);

/**
 * Registers a client for SFE common callbacks.
 *
 * @param sfe_cb SFE common callback pointer.
 *
 * @return
 * 0 if success, error value if fails.
 */
int ecm_sfe_common_callbacks_register(struct ecm_sfe_common_callbacks *sfe_cb);

/**
 * Unregisters a client from SFE common callbacks.
 *
 * @return
 * None.
 */
void ecm_sfe_common_callbacks_unregister(void);

/**
 * @}
 */

#endif /* __ECM_SFE_COMMON_PUBLIC_H__ */
