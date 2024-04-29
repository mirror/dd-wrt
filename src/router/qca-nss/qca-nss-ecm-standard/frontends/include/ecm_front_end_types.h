/*
 **************************************************************************
 * Copyright (c) 2014-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
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
#include <net/netfilter/nf_conntrack.h>

#include "ecm_ae_classifier_public.h"

#define ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC 0xFEC1

/*
 * Constant used with constructing acceleration rules.
 */
#define ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED 0xFFF
#define ECM_FRONT_END_INVALID_VLAN_PCP 0xFF

/*
 * Bridge device macros
 */
#define ecm_front_end_is_bridge_port(dev) (dev && (dev->priv_flags & IFF_BRIDGE_PORT))
#define ecm_front_end_is_bridge_device(dev) (dev->priv_flags & IFF_EBRIDGE)
#define ecm_front_end_is_ovs_bridge_device(dev) (dev->priv_flags & IFF_OPENVSWITCH)

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
 * Protocol type that ported file supports.
 */
enum ecm_front_end_ported_proto_types {
	ECM_FRONT_END_PORTED_PROTO_TCP,
	ECM_FRONT_END_PORTED_PROTO_UDP,
	ECM_FRONT_END_PORTED_PROTO_MAX
};

/*
 * Front end engine types which the frontend
 * isntance is created for.
 */
enum ecm_front_end_engine {
	ECM_FRONT_END_ENGINE_NSS,
	ECM_FRONT_END_ENGINE_PPE,
	ECM_FRONT_END_ENGINE_SFE,
	ECM_FRONT_END_ENGINE_MAX
};

enum ecm_front_end_engine_flag {
	ECM_FRONT_END_ENGINE_FLAG_CAN_ACCEL = 0x00000001,
						/* If a front end can accelerate a connection */
	ECM_FRONT_END_ENGINE_FLAG_PPE_DS = 0x00000002,
						/* If a front end supports PPE DS datapath */
	ECM_FRONT_END_ENGINE_FLAG_PPE_VP = 0x00000004,
						/* If a front end supports PPE VP datapath */
	ECM_FRONT_END_ENGINE_FLAG_AE_PRECEDENCE = 0x00000008,
						/* If the front end enforces AE precedence selection */
	ECM_FRONT_END_ENGINE_FLAG_MAX
						/* Maximum front end engine flags */
};

/*
 * ECM kernel module parameter "front_end_selection" is used to determine
 * which front end should be selected. Its possible values are 0, 1 and 2.
 * They are mapped to following definition.
 * ECM_FRONT_END_TYPE_AUTO: select NSS front end if hardware support it,
 *			    otherwise select SFE or PPE_SFE front end based on the
 *			    underlying SoC.
 * ECM_FRONT_END_TYPE_NSS: select NSS front end if hardware support it,
 *			   otherwise abort initailization.
 * ECM_FRONT_END_TYPE_SFE: select SFE front end.
 * ECM_FRONT_END_TYPE_PPE: select PPE front end.
 * ECM_FRONT_END_TYPE_NSS_SFE: Both NSS and SFE can be selected.
 * ECM_FRONT_END_TYPE_PPE_SFE: Both PPE and SFE can be selected.
 */
enum ecm_front_end_type {
	ECM_FRONT_END_TYPE_AUTO,
	ECM_FRONT_END_TYPE_NSS,
	ECM_FRONT_END_TYPE_SFE,
	ECM_FRONT_END_TYPE_PPE,
	ECM_FRONT_END_TYPE_NSS_SFE,
	ECM_FRONT_END_TYPE_PPE_SFE,
	ECM_FRONT_END_TYPE_MAX
};

/*
 * Acceleration engine precedence
 */
enum ecm_ae_precedence_order {
	ECM_AE_PRECEDENCE_0,
	ECM_AE_PRECEDENCE_1,
	ECM_AE_PRECEDENCE_MAX,
};

/*
 * Features supported in ECM's frontends.
 */
enum ecm_fe_feature {
	ECM_FE_FEATURE_NSS		= (1 << 0),
	ECM_FE_FEATURE_SFE		= (1 << 1),
	ECM_FE_FEATURE_NON_PORTED	= (1 << 2),
	ECM_FE_FEATURE_BRIDGE		= (1 << 3),
	ECM_FE_FEATURE_MULTICAST	= (1 << 4),
	ECM_FE_FEATURE_BONDING		= (1 << 5),
	ECM_FE_FEATURE_IGS		= (1 << 6),
	ECM_FE_FEATURE_SRC_IF_CHECK	= (1 << 7),
	ECM_FE_FEATURE_CONN_LIMIT	= (1 << 8),
	ECM_FE_FEATURE_DSCP_ACTION	= (1 << 9),
	ECM_FE_FEATURE_XFRM		= (1 << 10),
	ECM_FE_FEATURE_OVS_BRIDGE	= (1 << 11),
	ECM_FE_FEATURE_OVS_VLAN		= (1 << 12),
	ECM_FE_FEATURE_PPE		= (1 << 13),
};

/*
 * Global variable for the selected front-end type.
 * It is set once in the module init function.
 */
extern enum ecm_front_end_type selected_front_end;

/*
 * enum ecm_front_end_acceleration_modes
 *	Acceleration mode of a connection as tracked by the front end.
 *
 * Typically when classifiers permit acceleration the front end will then accelerate the connection.
 * These states indicate the front end record of acceleration mode for the connection.
 * An acceleration mode less than zero indicates a connection that cannot be accelerated, maybe due to error.
 */
enum ecm_front_end_acceleration_modes {
	ECM_FRONT_END_ACCELERATION_MODE_FAIL_DEFUNCT_SHORT = -8,/* Acceleration has failed for a short time due to the connection has become defunct and waiting for the removal */
	ECM_FRONT_END_ACCELERATION_MODE_FAIL_DEFUNCT = -7,	/* Acceleration has permanently failed due to the connection has become defunct */
	ECM_FRONT_END_ACCELERATION_MODE_FAIL_DECEL = -6,	/* Acceleration has permanently failed due to deceleration malfunction */
	ECM_FRONT_END_ACCELERATION_MODE_FAIL_NO_ACTION = -5,	/* Acceleration has permanently failed due to too many offloads that were rejected without any packets being offloaded */
	ECM_FRONT_END_ACCELERATION_MODE_FAIL_ACCEL_ENGINE = -4,	/* Acceleration has permanently failed due to too many accel engine NAK's */
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
typedef void (*ecm_front_end_connection_accelerate_method_t)(struct ecm_front_end_connection_instance *feci,
									struct ecm_classifier_process_response *pr, bool is_l2_encap,
									struct nf_conn *ct, struct sk_buff *skb);

typedef bool (*ecm_front_end_connection_decelerate_method_t)(struct ecm_front_end_connection_instance *feci);
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
typedef void (*ecm_front_end_connection_multicast_update_method_t)(ip_addr_t ip_grp_addr, struct net_device *brdev);

typedef void (*ecm_front_end_connection_set_stats_bitmap_t)(struct ecm_front_end_connection_instance *feci, ecm_db_obj_dir_t dir, uint8_t bit);
typedef uint32_t (*ecm_front_end_connection_get_stats_bitmap_t)(struct ecm_front_end_connection_instance *feci, ecm_db_obj_dir_t dir);
typedef void (*ecm_front_end_connection_update_rule_t)(struct ecm_front_end_connection_instance *feci, enum ecm_rule_update_type type, void *arg);

typedef bool (*ecm_front_end_connection_defunct_method_t)(void *arg, int *accel_mode);       /* Defunct callback */

/*
 * Acceleration limiting modes.
 *	Used to apply limiting to accelerated connections.
 */
#define ECM_FRONT_END_ACCEL_LIMIT_MODE_UNLIMITED 0x00	/* No limits on acceleration rule creation */
#define ECM_FRONT_END_ACCEL_LIMIT_MODE_FIXED 0x01	/* Fixed upper limit for connection acceleration based on information from driver */

typedef struct ecm_front_end_connection_instance *
	(*ecm_front_end_connection_alloc_method_t)(uint32_t flags, int protocol, struct ecm_db_connection_instance **nci);

struct ecm_ae_precedence {
	int ae_type;
	ecm_front_end_connection_alloc_method_t ported_ipv4_alloc;
	ecm_front_end_connection_alloc_method_t ported_ipv6_alloc;
#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
	ecm_front_end_connection_alloc_method_t non_ported_ipv4_alloc;
	ecm_front_end_connection_alloc_method_t non_ported_ipv6_alloc;
#endif
};

extern struct ecm_ae_precedence ae_precedence[ECM_AE_PRECEDENCE_MAX + 1];

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
	uint64_t slow_path_packets;		/* The number of slow packets before the acceleration starts */
	unsigned long cmd_time_begun;		/* Time captured when an accel or decel request begun */
	unsigned long cmd_time_completed;	/* Time captured when request finished */
};

/*
 * Common information
 */
struct ecm_front_end_common_fe_info {
	uint32_t from_stats_bitmap;     /* Bitmap of L2 features enabled for from direction */
	uint32_t to_stats_bitmap;       /* Bitmap of L2 features enabled for to direction */
	uint32_t front_end_flags;	/* Front end related flags */
};

/*
 * Connection front end instance
 *	Each new connection requires it to also have one of these to maintain front end specific information and operations
 */
struct ecm_front_end_connection_instance {
	ecm_front_end_connection_accelerate_method_t accelerate;		/* accelerate a connection */
	ecm_front_end_connection_decelerate_method_t decelerate;		/* Decelerate a connection */
	ecm_front_end_connection_accel_ceased_method_t accel_ceased;		/* Acceleration has stopped */
	ecm_front_end_connection_ae_interface_number_by_dev_get_method_t ae_interface_number_by_dev_get;
										/* Get the acceleration engine interface number from the dev instance */
	ecm_front_end_connection_ae_interface_number_by_dev_type_get_method_t ae_interface_number_by_dev_type_get;
										/* Get the acceleration engine interface number from the dev instance and type */
	ecm_front_end_connection_ae_interface_type_get_method_t ae_interface_type_get;
										/* Get the acceleration engine interface type */
	ecm_front_end_connection_regenerate_method_t regenerate;
										/* regenerate a connection */
	ecm_front_end_connection_defunct_method_t defunct;			/* defunct a connection */

#ifdef ECM_STATE_OUTPUT_ENABLE
	ecm_front_end_connection_state_get_callback_t state_get;		/* Obtain state for this object */
#endif
	ecm_front_end_connection_multicast_update_method_t multicast_update;	/* Update existing multicast connection */

	ecm_front_end_connection_set_stats_bitmap_t set_stats_bitmap;		/* Set bitmap of interface types to be updated during sync */
	ecm_front_end_connection_get_stats_bitmap_t get_stats_bitmap;		/* Get bitmap of interface types to be updated during sync */
	ecm_front_end_connection_update_rule_t update_rule;			/* Updates the frontend specific data */

	enum ecm_front_end_engine accel_engine;					/* Acceleration engine type */
	uint8_t ported_accelerated_count_index;                 		/* Index value of accelerated count array (UDP or TCP) */

	struct ecm_front_end_common_fe_info fe_info;          /* Front end information */

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
	bool destroy_fail_handle_pending;			/* Set while handling the connection destroy failure */
	ecm_front_end_acceleration_mode_t accel_mode;		/* Indicates the type of acceleration being applied to a connection, if any. */
	spinlock_t lock;					/* Lock for structure data */
	int refs;						/* Integer to trap we never go negative */
#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
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

struct ecm_front_end_ovs_params {
	ip_addr_t src_ip;
	ip_addr_t dest_ip;
	int src_port;
	int dest_port;
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
void ecm_front_end_ipv4_fill_ovs_params(struct ecm_front_end_ovs_params ovs_params[], ip_addr_t ip_src_addr, ip_addr_t ip_src_addr_nat, ip_addr_t ip_dest_addr,
					ip_addr_t ip_dest_addr_nat, int src_port, int src_port_nat, int dest_port, int dest_port_nat);

bool ecm_front_end_is_feature_supported(enum ecm_fe_feature feature);

void ecm_front_end_set_ae_alloc_methods(struct ecm_ae_precedence *precedence);
bool ecm_front_end_common_feature_check(enum ecm_front_end_engine ae_type,
					struct sk_buff *skb,
					struct ecm_tracker_ip_header *iph,
					bool is_routed);


/*
 * ecm_front_end_type_get()
 *	Returns the selcted fornt-end type.
 */
static inline enum ecm_front_end_type ecm_front_end_type_get(void)
{
	return selected_front_end;
}

/*
 * ecm_front_end_type_select()
 *	Detects and sets which front end to run
 *
 * User can select front end explicitly by passing the AE type
 * to kernel module parameter "front_end_selection". Or let ECM make
 * the decision by passing 0(auto). "auto" is also the default mode if
 * user didn't specify parameter "front_end_selection".
 *
 * In automatic selection mode, we prefer to select NSS front end if
 * hardware support it, then the others.
 *
 * We check device tree to see if NSS is supported by hardware.
 * Currenly all ipq8064, ipq8062, ipq807x, ipq60xx platforms support NSS.
 * Since SFE is a pure software acceleration engine, so all platforms
 * support it.
 */
static inline enum ecm_front_end_type ecm_front_end_type_select(void)
{

#ifdef ECM_FRONT_END_NSS_ENABLE
	extern int front_end_selection;
	bool nss_supported = false;
#ifdef CONFIG_OF
	nss_supported = of_machine_is_compatible("qcom,ipq8064") ||
				of_machine_is_compatible("qcom,ipq8062") ||
				of_machine_is_compatible("qcom,ipq807x") ||
				of_machine_is_compatible("qcom,ipq8074") ||
				of_machine_is_compatible("qcom,ipq6018") ||
				of_machine_is_compatible("qcom,ipq5018");
#else
	nss_supported = true;
#endif
	if (nss_supported && ((front_end_selection == ECM_FRONT_END_TYPE_AUTO) ||
			      (front_end_selection == ECM_FRONT_END_TYPE_NSS))) {
		return ECM_FRONT_END_TYPE_NSS;
	}
#endif

#if defined(ECM_FRONT_END_PPE_ENABLE) && defined(ECM_FRONT_END_SFE_ENABLE)
	if ((front_end_selection == ECM_FRONT_END_TYPE_PPE_SFE)
		|| ((front_end_selection == ECM_FRONT_END_TYPE_AUTO) && of_machine_is_compatible("qcom,ipq9574"))
		|| ((front_end_selection == ECM_FRONT_END_TYPE_AUTO) && of_machine_is_compatible("qcom,ipq5332"))) {
		return ECM_FRONT_END_TYPE_PPE_SFE;
	}
#endif

#ifdef ECM_FRONT_END_SFE_ENABLE
	if ((front_end_selection == ECM_FRONT_END_TYPE_AUTO) ||
	    (front_end_selection == ECM_FRONT_END_TYPE_SFE)) {
		return ECM_FRONT_END_TYPE_SFE;
	}
#endif

#ifdef ECM_FRONT_END_PPE_ENABLE
	if (front_end_selection == ECM_FRONT_END_TYPE_PPE){
		return ECM_FRONT_END_TYPE_PPE;
	}
#endif

#if defined(ECM_FRONT_END_NSS_ENABLE) && defined(ECM_FRONT_END_SFE_ENABLE)
	if (nss_supported && (front_end_selection == ECM_FRONT_END_TYPE_NSS_SFE)) {
		return ECM_FRONT_END_TYPE_NSS_SFE;
	}
#endif

	return ECM_FRONT_END_TYPE_MAX;
}

/*
 * ecm_front_end_set_ae_precendence_array()
 *	Sets the precedence array based on the selected frontend mode.
 *
 * If any new combination of AEs or a single AE is added to the system, this function
 * must be updated for the new frontend modes.
 */
static inline bool ecm_front_end_set_ae_precendence_array(enum ecm_front_end_type type)
{
	switch (type) {
#ifdef ECM_FRONT_END_NSS_ENABLE
	case ECM_FRONT_END_TYPE_NSS:
		ae_precedence[ECM_AE_PRECEDENCE_0].ae_type = ECM_FRONT_END_ENGINE_NSS;
		ecm_front_end_set_ae_alloc_methods(&ae_precedence[ECM_AE_PRECEDENCE_0]);

		ae_precedence[ECM_AE_PRECEDENCE_1].ae_type = ECM_FRONT_END_ENGINE_MAX;
		break;
#endif
#ifdef ECM_FRONT_END_SFE_ENABLE
	case ECM_FRONT_END_TYPE_SFE:
		ae_precedence[ECM_AE_PRECEDENCE_0].ae_type = ECM_FRONT_END_ENGINE_SFE;
		ecm_front_end_set_ae_alloc_methods(&ae_precedence[ECM_AE_PRECEDENCE_0]);

		ae_precedence[ECM_AE_PRECEDENCE_1].ae_type = ECM_FRONT_END_ENGINE_MAX;
		break;
#endif
#ifdef ECM_FRONT_END_PPE_ENABLE
	case ECM_FRONT_END_TYPE_PPE:
		ae_precedence[ECM_AE_PRECEDENCE_0].ae_type = ECM_FRONT_END_ENGINE_PPE;
		ecm_front_end_set_ae_alloc_methods(&ae_precedence[ECM_AE_PRECEDENCE_0]);

		ae_precedence[ECM_AE_PRECEDENCE_1].ae_type = ECM_FRONT_END_ENGINE_MAX;
		break;
#endif
#if defined(ECM_FRONT_END_NSS_ENABLE) && defined(ECM_FRONT_END_SFE_ENABLE)
	case ECM_FRONT_END_TYPE_NSS_SFE:
		ae_precedence[ECM_AE_PRECEDENCE_0].ae_type = ECM_FRONT_END_ENGINE_NSS;
		ecm_front_end_set_ae_alloc_methods(&ae_precedence[ECM_AE_PRECEDENCE_0]);

		ae_precedence[ECM_AE_PRECEDENCE_1].ae_type = ECM_FRONT_END_ENGINE_SFE;
		ecm_front_end_set_ae_alloc_methods(&ae_precedence[ECM_AE_PRECEDENCE_1]);

		ae_precedence[ECM_AE_PRECEDENCE_MAX].ae_type = ECM_FRONT_END_ENGINE_MAX;
		break;
#endif
#if defined(ECM_FRONT_END_PPE_ENABLE) && defined(ECM_FRONT_END_SFE_ENABLE)
	case ECM_FRONT_END_TYPE_PPE_SFE:
		ae_precedence[ECM_AE_PRECEDENCE_0].ae_type = ECM_FRONT_END_ENGINE_PPE;
		ecm_front_end_set_ae_alloc_methods(&ae_precedence[ECM_AE_PRECEDENCE_0]);

		ae_precedence[ECM_AE_PRECEDENCE_1].ae_type = ECM_FRONT_END_ENGINE_SFE;
		ecm_front_end_set_ae_alloc_methods(&ae_precedence[ECM_AE_PRECEDENCE_1]);

		ae_precedence[ECM_AE_PRECEDENCE_MAX].ae_type = ECM_FRONT_END_ENGINE_MAX;
		break;
#endif
	default:
		DEBUG_TRACE("precedence is not supported for the ECM mode type: %d\n", type);
		return false;
	}

	return true;
}
