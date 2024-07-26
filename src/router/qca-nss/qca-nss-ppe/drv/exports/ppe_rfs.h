/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file ppe_rfs.h
 *	NSS PPE RFS definitions.
 */

#ifndef _PPE_RFS_H_
#define _PPE_RFS_H_

/*
 * Rule creation and rule update flags.
 */
#define PPE_RFS_V4_RULE_FLAG_BRIDGE_FLOW 0x1
#define PPE_RFS_V4_RULE_FLAG_RETURN_VALID 0x2
#define PPE_RFS_V4_RULE_FLAG_FLOW_VALID 0x4
#define PPE_RFS_V4_RULE_FLAG_QOS_VALID 0x8

#define PPE_RFS_V6_RULE_FLAG_BRIDGE_FLOW 0x10
#define PPE_RFS_V6_RULE_FLAG_RETURN_VALID 0x20
#define PPE_RFS_V6_RULE_FLAG_FLOW_VALID 0x40
#define PPE_RFS_V6_RULE_FLAG_QOS_VALID 0x80

enum ppe_rfs_ret {
	PPE_RFS_RET_SUCCESS = 0,	/**< RFS operation succeeded */
	PPE_RFS_RET_FAILURE = 1,	/**< RFS operation failed */
};

/**
 * QoS connection rule structure.
 */
struct ppe_rfs_qos_rule {
	u32 flow_qos_tag;	/**< QoS tag associated with this rule for flow direction. */
	u32 return_qos_tag;	/**< QoS tag associated with this rule for return direction. */
};

/**
 * ppe_rfs_ipv4_5tuple
 *	Common 5-tuple structure.
 */
struct ppe_rfs_ipv4_5tuple {
	__be32 flow_ip;         /**< Flow IP address. */
	__be32 return_ip;       /**< Return IP address. */
	__be16 flow_ident;      /**< Flow identifier, e.g., TCP/UDP port. */
	__be16 return_ident;    /**< Return identifier, e.g., TCP/UDP port. */
	u8 protocol;            /**< Protocol number. */
	u8 reserved[3];         /**< Reserved; padding for alignment. */
};

/**
 * ppe_rfs_ipv4_connection_rule
 *	IPv4 connection rule structure.
 */
struct ppe_rfs_ipv4_connection_rule {
	s32 flow_interface_num;         /**< Flow interface number. */
	s32 return_interface_num;       /**< Return interface number. */
	u32 flow_mtu;                   /**< Flow interface`s MTU. */
	u32 return_mtu;                 /**< Return interface`s MTU. */
	__be32 flow_ip_xlate;		/**< Translated flow IP address. */
	__be32 return_ip_xlate;         /**< Translated return IP address. */
	__be16 flow_ident_xlate;        /**< Translated flow identifier, e.g., port. */
	__be16 return_ident_xlate;      /**< Translated return identifier, e.g., port. */
	s32 flow_top_interface_num;	/**< Flow top interface number. */
	s32 return_top_interface_num;	/**< Return top interface number. */
};

/**
 * ppe_rfs_ipv4_5tuple
 *	IPv4 5-tuple structure.
 */
struct ppe_rfs_ipv4_rule_create_msg {
	u16 valid_flags;				/**< Bit flags associated with paramater validity. */
	u16 rule_flags;					/**< Bit flags associated with the rule. */
	struct ppe_rfs_ipv4_5tuple tuple;		/**< Holds values of 5-tuple. */
	struct ppe_rfs_ipv4_connection_rule conn_rule;  /**< Basic connection-specific data. */
	struct ppe_rfs_qos_rule qos_rule;			/**< Holds qos tag information */
};

/**
 * ppe_rfs_ipv4_connection_rule
 *	IPv4 connection rule structure.
 */
struct ppe_rfs_ipv4_rule_destroy_msg {
	struct ppe_rfs_ipv4_5tuple tuple;       /**< Holds values of 5-tuple. */
	struct ppe_rfs_ipv4_connection_rule conn_rule;	/**< Basic connection-specific data. */
	struct net_device *original_dev;
	struct net_device *reply_dev;
};

/**
 * ppe_rfs_ipv6_5tuple
 *	IPv6 5-tuple structure.
 */
struct ppe_rfs_ipv6_5tuple {
	__be32 flow_ip[4];      /**< Flow IP address. */
	__be32 return_ip[4];    /**< Return IP address. */
	__be16 flow_ident;      /**< Flow identifier, e.g.,TCP/UDP port. */
	__be16 return_ident;    /**< Return identifier, e.g., TCP/UDP port. */
	u8 protocol;           /**< Protocol number. */
};

/**
 * ppe_rfs_ipv6_connection_rule
 *	IPv6 connection rule structure.
 */
struct ppe_rfs_ipv6_connection_rule {
	int8_t flow_interface_num;		/**< Flow interface number. */
	int8_t return_interface_num;		/**< Return interface number. */
	uint16_t flow_mtu;			/**< Flow interface's MTU. */
	uint16_t return_mtu;			/**< Return interface's MTU. */
	int8_t flow_top_interface_num;		/**< Flow top interface number. */
	int8_t return_top_interface_num;	/**< Return top interface number. */
};

/**
 * ppe_rfs_ipv6_rule_create_msg
 *	IPv6 rule create message structure.
 */
struct ppe_rfs_ipv6_rule_create_msg {
	u16 valid_flags;				/**< Bit flags associated with parameter validity. */
	u16 rule_flags;					/**< Bit flags associated with the rule. */
	struct ppe_rfs_ipv6_5tuple tuple;		/**< Holds values of the ppe_ipv6_5tuple tuple. */
	struct ppe_rfs_ipv6_connection_rule conn_rule;	/**< Basic connection-specific data. */
	struct ppe_rfs_qos_rule qos_rule;			/**< Holds qos tag information */
};

/**
 * ppe_rfs_ipv6_rule_destroy_msg
 *	IPv6 rule destroy message structure.
 */
struct ppe_rfs_ipv6_rule_destroy_msg {
	struct ppe_rfs_ipv6_5tuple tuple;       /**< Holds values of the ipv6_5tuple tuple */
	struct ppe_rfs_ipv6_connection_rule conn_rule;	/**< Basic connection-specific data. */
	struct net_device *original_dev;	/**< Original device for a connection */
	struct net_device *reply_dev;		/**< Reply device for a connection */
};

/**
 * ppe_rfs_ipv4_rule_create
 *	Push PPE rules to assist Non PPE engines.
 *
 * @datatypes
 * struct ppe_rfs_ipv4_rule_create_msg
 *
 * @param[in] msg   Pointer to the PPE RFS rule create message.
 *
 * @return
 * status of ipv4 rule create
 */
enum ppe_rfs_ret ppe_rfs_ipv4_rule_create(struct ppe_rfs_ipv4_rule_create_msg *msg);

/**
 * ppe_rfs_ipv4_rule_destroy
 *	Destroy PPE rules pushed to assist Non PPE engines.
 *
 * @datatypes
 * struct ppe_rfs_ipv4_rule_destroy_msg
 *
 * @param[in] msg   Pointer to the PPE RFS rule destroy message.
 *
 * @return
 * status of ipv4 rule destroy
 */
enum ppe_rfs_ret ppe_rfs_ipv4_rule_destroy(struct ppe_rfs_ipv4_rule_destroy_msg *msg);

/**
 * ppe_rfs_ipv6_rule_create
 *	Push PPE IPv6 rules to assist Non PPE engines.
 *
 * @datatypes
 * struct ppe_rfs_ipv6_rule_create_msg
 *
 * @param[in] msg   Pointer to the PPE RFS rule create message.
 *
 * @return
 * status of ipv6 rule create
 */
enum ppe_rfs_ret ppe_rfs_ipv6_rule_create(struct ppe_rfs_ipv6_rule_create_msg *msg);

/**
 * ppe_rfs_ipv6_rule_destroy
 *	Destroy PPE rules pushed to assist Non PPE engines.
 *
 * @datatypes
 * struct ppe_rfs_ipv6_rule_destroy_msg
 *
 * @param[in] msg   Pointer to the PPE RFS rule destroy message.
 *
 * @return
 * status of ipv6 rule destroy
 */
enum ppe_rfs_ret ppe_rfs_ipv6_rule_destroy(struct ppe_rfs_ipv6_rule_destroy_msg *msg);
#endif /* _PPE_DRV_RFS_H_ */
