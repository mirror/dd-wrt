/*
 **************************************************************************
 * Copyright (c) 2014-2019 The Linux Foundation.  All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/of.h>

/*
 * Bridge device macros
 */
#define ecm_front_end_is_bridge_port(dev) (dev && (dev->priv_flags & IFF_BRIDGE_PORT))
#define ecm_front_end_is_bridge_device(dev) (dev->priv_flags & IFF_EBRIDGE)

#ifdef ECM_INTERFACE_BOND_ENABLE
/*
 * LAN Aggregation device macros
 */
#define ecm_front_end_is_lag_master(dev) ((dev->flags & IFF_MASTER)	\
							 && (dev->priv_flags & IFF_BONDING))
#define ecm_front_end_is_lag_slave(dev)	((dev->flags & IFF_SLAVE)	\
							 && (dev->priv_flags & IFF_BONDING))
#endif

/*
 * ECM kernel module parameter "front_end_selection" is used to determine
 * which front end should be selected. Its possible values are 0, 1 and 2.
 * They are mapped to following definition.
 * ECM_FRONT_END_TYPE_AUTO: select NSS front end if hardware support it,
 *			    otherwise select SFE front end.
 * ECM_FRONT_END_TYPE_NSS: select NSS front end if hardware support it,
 *			   otherwise abort initailization.
 * ECM_FRONT_END_TYPE_SFE: select SFE front end.
 */
enum ecm_front_end_type {
	ECM_FRONT_END_TYPE_AUTO,
	ECM_FRONT_END_TYPE_NSS,
	ECM_FRONT_END_TYPE_SFE,
	ECM_FRONT_END_TYPE_NOT_SUPPORTED
};

/*
 * enum ecm_front_end_acceleration_modes
 *	Acceleration mode of a connection as tracked by the front end.
 *
 * Typically when classifiers permit acceleration the front end will then accelerate the connection.
 * These states indicate the front end record of acceleration mode for the connection.
 * An acceleration mode less than zero indicates a connection that cannot be accelerated, maybe due to error.
 */
enum ecm_front_end_acceleration_modes {
	ECM_FRONT_END_ACCELERATION_MODE_FAIL_DEFUNCT = -7,	/* Acceleration has permanently failed due to the connection has become defunct */
	ECM_FRONT_END_ACCELERATION_MODE_FAIL_DECEL = -6,	/* Acceleration has permanently failed due to deceleration malfunction */
	ECM_FRONT_END_ACCELERATION_MODE_FAIL_NO_ACTION = -5,	/* Acceleration has permanently failed due to too many offloads that were rejected without any packets being offloaded */
	ECM_FRONT_END_ACCELERATION_MODE_FAIL_ACCEL_ENGINE = -4,		/* Acceleration has permanently failed due to too many accel engine NAK's */
	ECM_FRONT_END_ACCELERATION_MODE_FAIL_DRIVER = -3,	/* Acceleration has permanently failed due to too many driver interaction failures */
	ECM_FRONT_END_ACCELERATION_MODE_FAIL_RULE = -2,		/* Acceleration has permanently failed due to bad rule data */
	ECM_FRONT_END_ACCELERATION_MODE_FAIL_DENIED = -1,	/* Acceleration has permanently failed due to can_accel denying accel */
	ECM_FRONT_END_ACCELERATION_MODE_DECEL = 0,		/* Connection is not accelerated */
	ECM_FRONT_END_ACCELERATION_MODE_ACCEL_PENDING,		/* Connection is in the process of being accelerated */
	ECM_FRONT_END_ACCELERATION_MODE_ACCEL,			/* Connection is accelerated */
	ECM_FRONT_END_ACCELERATION_MODE_DECEL_PENDING,		/* Connection is in the process of being decelerated */
};
typedef enum ecm_front_end_acceleration_modes ecm_front_end_acceleration_mode_t;

#define ECM_FRONT_END_ACCELERATION_NOT_POSSIBLE(accel_mode) ((accel_mode < 0) || (accel_mode == ECM_FRONT_END_ACCELERATION_MODE_DECEL_PENDING))
#define ECM_FRONT_END_ACCELERATION_POSSIBLE(accel_mode) (accel_mode >= 0)
#define ECM_FRONT_END_ACCELERATION_FAILED(accel_mode) (accel_mode < 0)

/*
 * Front end methods
 */
struct ecm_front_end_connection_instance;
typedef bool (*ecm_front_end_connection_decelerate_method_t)(struct ecm_front_end_connection_instance *feci);
typedef ecm_front_end_acceleration_mode_t (*ecm_front_end_connection_accel_state_get_method_t)(struct ecm_front_end_connection_instance *feci);
typedef void (*ecm_front_end_connection_ref_method_t)(struct ecm_front_end_connection_instance *feci);
typedef int (*ecm_front_end_connection_deref_callback_t)(struct ecm_front_end_connection_instance *feci);
typedef void (*ecm_front_end_connection_action_seen_method_t)(struct ecm_front_end_connection_instance *feci);
typedef void (*ecm_front_end_connection_accel_ceased_method_t)(struct ecm_front_end_connection_instance *feci);
#ifdef ECM_STATE_OUTPUT_ENABLE
typedef int (*ecm_front_end_connection_state_get_callback_t)(struct ecm_front_end_connection_instance *feci, struct ecm_state_file_instance *sfi);
											/*
											 * Get state output.  Return 0 on success.
											 */
#endif
typedef int32_t (*ecm_front_end_connection_ae_interface_number_by_dev_get_method_t)(struct net_device *dev);
typedef int32_t (*ecm_front_end_connection_ae_interface_number_by_dev_type_get_method_t)(struct net_device *dev, uint32_t type);
typedef int32_t (*ecm_front_end_connection_ae_interface_type_get_method_t)(struct ecm_front_end_connection_instance *feci, struct net_device *dev);
typedef void (*ecm_front_end_connection_regenerate_method_t)(struct ecm_front_end_connection_instance *feci, struct ecm_db_connection_instance *ci);

/*
 * Acceleration limiting modes.
 *	Used to apply limiting to accelerated connections.
 */
#define ECM_FRONT_END_ACCEL_LIMIT_MODE_UNLIMITED 0x00	/* No limits on acceleration rule creation */
#define ECM_FRONT_END_ACCEL_LIMIT_MODE_FIXED 0x01	/* Fixed upper limit for connection acceleration based on information from driver */

/*
 * Accel/decel mode statistics data structure.
 */
struct ecm_front_end_connection_mode_stats {
	bool decelerate_pending;		/* Decel was attempted during pending accel - will be actioned when accel is done */
	bool flush_happened;			/* A flush message was received from accel engine before we received an ACK or NACK. (Accel engine Messaging sequence/ordering workaround) */
	uint32_t flush_happened_total;		/* Total of times we see flush_happened */
	uint32_t no_action_seen_total;		/* Total of times acceleration was ended by the accel engine itself without any offload action */
	uint32_t no_action_seen;		/* Count of times consecutive  acceleration was ended by the accel engine itself without any offload action */
	uint32_t no_action_seen_limit;		/* Limit on consecutive no-action at which point offload permanently fails out */
	uint32_t driver_fail_total;		/* Total times driver failed to send our command */
	uint32_t driver_fail;			/* Count of consecutive times driver failed to send our command, when this reaches driver_fail_limit acceleration will permanently fail */
	uint32_t driver_fail_limit;		/* Limit on drivers consecutive fails at which point offload permanently fails out */
	uint32_t ae_nack_total;		/* Total times accel engine NAK's an accel command */
	uint32_t ae_nack;			/* Count of consecutive times driver failed to ack */
	uint32_t ae_nack_limit;		/* Limit on consecutive nacks at which point offload permanently fails out */
	unsigned long cmd_time_begun;		/* Time captured when an accel or decel request begun */
	unsigned long cmd_time_completed;	/* Time captured when request finished */
};

/*
 * Connection front end instance
 *	Each new connection requires it to also have one of these to maintain front end specific information and operations
 */
struct ecm_front_end_connection_instance {
	ecm_front_end_connection_ref_method_t ref;				/* Ref the instance */
	ecm_front_end_connection_deref_callback_t deref;			/* Deref the instance */
	ecm_front_end_connection_decelerate_method_t decelerate;		/* Decelerate a connection */
	ecm_front_end_connection_accel_state_get_method_t accel_state_get;	/* Get the acceleration state */
	ecm_front_end_connection_action_seen_method_t action_seen;		/* Acceleration action has occurred */
	ecm_front_end_connection_accel_ceased_method_t accel_ceased;		/* Acceleration has stopped */
	ecm_front_end_connection_ae_interface_number_by_dev_get_method_t ae_interface_number_by_dev_get;
										/* Get the acceleration engine interface number from the dev instance */
	ecm_front_end_connection_ae_interface_number_by_dev_type_get_method_t ae_interface_number_by_dev_type_get;
										/* Get the acceleration engine interface number from the dev instance and type */
	ecm_front_end_connection_ae_interface_type_get_method_t ae_interface_type_get;
										/* Get the acceleration engine interface type */
	ecm_front_end_connection_regenerate_method_t regenerate;
										/* regenerate a connection */
#ifdef ECM_STATE_OUTPUT_ENABLE
	ecm_front_end_connection_state_get_callback_t state_get;		/* Obtain state for this object */
#endif

	/*
	 * Accel/decel mode statistics.
	 */
	struct ecm_front_end_connection_mode_stats stats;

	/*
	 * Common control items to all front end instances
	 */
	int ip_version;						/* RO: The version of IP protocol this instance was established for */
	int protocol;						/* RO: The protocol this instance was established for */
	struct ecm_db_connection_instance *ci;			/* RO: The connection instance relating to this instance. */
	bool can_accel;						/* RO: True when the connection can be accelerated */
	bool is_defunct;					/* True if the connection has become defunct */
	ecm_front_end_acceleration_mode_t accel_mode;		/* Indicates the type of acceleration being applied to a connection, if any. */
	spinlock_t lock;					/* Lock for structure data */
	int refs;						/* Integer to trap we never go negative */

};

/*
 * Front end's interface construction structure.
 */
struct ecm_front_end_interface_construct_instance {
	struct net_device *from_dev;
	struct net_device *from_other_dev;
	struct net_device *to_dev;
	struct net_device *to_other_dev;
	struct net_device *from_nat_dev;
	struct net_device *from_nat_other_dev;
	struct net_device *to_nat_dev;
	struct net_device *to_nat_other_dev;

	ip_addr_t from_mac_lookup_ip_addr;
	ip_addr_t to_mac_lookup_ip_addr;
	ip_addr_t from_nat_mac_lookup_ip_addr;
	ip_addr_t to_nat_mac_lookup_ip_addr;
};

extern void ecm_front_end_ipv6_interface_construct_netdev_put(struct ecm_front_end_interface_construct_instance *efeici);
extern void ecm_front_end_ipv6_interface_construct_netdev_hold(struct ecm_front_end_interface_construct_instance *efeici);
extern bool ecm_front_end_ipv6_interface_construct_set_and_hold(struct sk_buff *skb, ecm_tracker_sender_type_t sender, ecm_db_direction_t ecm_dir, bool is_routed,
							struct net_device *in_dev, struct net_device *out_dev,
							ip_addr_t ip_src_addr, ip_addr_t ip_dest_addr,
							struct ecm_front_end_interface_construct_instance *efeici);

extern void ecm_front_end_ipv4_interface_construct_netdev_put(struct ecm_front_end_interface_construct_instance *efeici);
extern void ecm_front_end_ipv4_interface_construct_netdev_hold(struct ecm_front_end_interface_construct_instance *efeici);
extern bool ecm_front_end_ipv4_interface_construct_set_and_hold(struct sk_buff *skb, ecm_tracker_sender_type_t sender, ecm_db_direction_t ecm_dir, bool is_routed,
							struct net_device *in_dev, struct net_device *out_dev,
							ip_addr_t ip_src_addr, ip_addr_t ip_src_addr_nat,
							ip_addr_t ip_dest_addr, ip_addr_t ip_dest_addr_nat,
							struct ecm_front_end_interface_construct_instance *efeici);

/*
 * Detect which front end to run
 *
 * User can select front end explicitly by passing 1(nss) or 2(sfe)
 * to kernel module parameter "front_end_selection". Or let ECM make
 * the decision by passing 0(auto). "auto" is also the default mode if
 * user didn't specify parameter "front_end_selection".
 *
 * In automatic selection mode, we prefer to select NSS front end if
 * hardware support it, then SFE front end.
 *
 * We check device tree to see if NSS is supported by hardware.
 * Currenly all ipq8064, ipq8062 and ipq807x  ipq60xx platforms support NSS.
 * Since SFE is a pure software acceleration engine, so all platforms
 * support it.
 */
static inline enum ecm_front_end_type ecm_front_end_type_get(void)
{
#ifdef CONFIG_OF
	bool nss_supported = of_machine_is_compatible("qcom,ipq8064") ||
				of_machine_is_compatible("qcom,ipq8062") ||
				of_machine_is_compatible("qcom,ipq807x") ||
				of_machine_is_compatible("qcom,ipq6018");
#else
	bool nss_supported = true;
#endif
	extern int front_end_selection;

	if (nss_supported && ((front_end_selection == ECM_FRONT_END_TYPE_AUTO) ||
			      (front_end_selection == ECM_FRONT_END_TYPE_NSS))) {
		return ECM_FRONT_END_TYPE_NSS;
	}

	if ((front_end_selection == ECM_FRONT_END_TYPE_AUTO) ||
	    (front_end_selection == ECM_FRONT_END_TYPE_SFE)) {
		return ECM_FRONT_END_TYPE_SFE;
	}

	return ECM_FRONT_END_TYPE_NOT_SUPPORTED;
}
