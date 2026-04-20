/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * @file ppe_priority.h
 *	NSS PPE priority definitions.
 */

#ifndef _PPE_priority_H_
#define _PPE_priority_H_

/**
 * ppe_priority_ret
 * 	PPE priority return status
 */
enum ppe_priority_ret {
	PPE_PRIORITY_RET_SUCCESS = 0,	/**< operation succeeded */
	PPE_PRIORITY_RET_FAILURE = 1,	/**< operation failed */
};

/**
 * ppe_priority_ipv4_5tuple
 *	Common 5-tuple structure.
 */
struct ppe_priority_ipv4_5tuple {
	__be32 flow_ip;         /**< Flow IP address. */
	__be32 return_ip;       /**< Return IP address. */
	__be16 flow_ident;      /**< Flow identifier, e.g., TCP/UDP port. */
	__be16 return_ident;    /**< Return identifier, e.g., TCP/UDP port. */
	u8 protocol;            /**< Protocol number. */
};

/**
 * ppe_priority_ipv4_rule_create_msg
 *	IPv4 priority rule create structure.
 */
struct ppe_priority_ipv4_rule_create_msg {
	struct ppe_priority_ipv4_5tuple tuple;		/**< Holds values of 5-tuple. */
	u32 flow_mtu;                   		/**< Flow interface`s MTU. */
	u8 priority;					/**< Priority to be set */
};

/**
 * ppe_priority_ipv4_rule_destroy_msg
 *	IPv4 priority  rule destroystructure.
 */
struct ppe_priority_ipv4_rule_destroy_msg {
	struct ppe_priority_ipv4_5tuple tuple;       /**< Holds values of 5-tuple. */
};

/**
 * ppe_priority_ipv6_5tuple
 *	IPv6 5-tuple structure.
 */
struct ppe_priority_ipv6_5tuple {
	__be32 flow_ip[4];      /**< Flow IP address. */
	__be32 return_ip[4];    /**< Return IP address. */
	__be16 flow_ident;      /**< Flow identifier, e.g.,TCP/UDP port. */
	__be16 return_ident;    /**< Return identifier, e.g., TCP/UDP port. */
	u8 protocol;           /**< Protocol number. */
};

/**
 * ppe_priority_ipv6_rule_create_msg
 *	IPv6 rule create message structure.
 */
struct ppe_priority_ipv6_rule_create_msg {
	struct ppe_priority_ipv6_5tuple tuple;		/**< Holds values of the ppe_ipv6_5tuple tuple. */
	u32 flow_mtu;	/**< Flow interface's MTU. */
	u8 priority;		/**< Priority to be set */
};

/**
 * ppe_priority_ipv6_rule_destroy_msg
 *	IPv6 rule destroy message structure.
 */
struct ppe_priority_ipv6_rule_destroy_msg {
	struct ppe_priority_ipv6_5tuple tuple;       /**< Holds values of the ipv6_5tuple tuple */
};

/**
 * ppe_priority_ipv4_rule_create
 *	Push PPE IPV4 priority rules to assist Non PPE engines.
 *
 * @datatype
 * ppe_priority_ipv4_rule_create_msg
 *
 * @param[in] msg   Pointer to the PPE priority rule create message.
 *
 * @return
 * status of ipv4 rule create
 */
enum ppe_priority_ret ppe_priority_ipv4_rule_create(struct ppe_priority_ipv4_rule_create_msg *msg);

/**
 * ppe_priority_ipv4_rule_destroy
 *	Destroy PPE rules pushed to assist Non PPE engines.
 *
 * @datatype
 * ppe_priority_ipv4_rule_destroy_msg
 *
 * @param[in] msg   Pointer to the PPE priority rule destroy message.
 *
 * @return
 * status of ipv4 rule destroy
 */
 enum ppe_priority_ret ppe_priority_ipv4_rule_destroy(struct ppe_priority_ipv4_rule_destroy_msg *msg);

/**
 * ppe_priority_ipv6_rule_create
 *	Push PPE IPv6 priority rules to assist Non PPE engines.
 *
 * @datatype
 * ppe_priority_ipv6_rule_create_msg
 *
 * @param[in] msg   Pointer to the IPv6 PPE Priority rule create message.
 *
 * @return
 * status of ipv6 rule create
 */
enum ppe_priority_ret ppe_priority_ipv6_rule_create(struct ppe_priority_ipv6_rule_create_msg *msg);

/**
 * ppe_priority_ipv6_rule_destroy
 *	Destroy IPv6 PPE priority rules pushed to assist Non PPE engines.
 *
 * @datatype
 * ppe_priority_ipv6_rule_destroy_msg
 *
 * @param[in] msg   Pointer to the PPE RFS rule destroy message.
 *
 * @return
 * status of ipv6 rule destroy
 */
enum ppe_priority_ret ppe_priority_ipv6_rule_destroy(struct ppe_priority_ipv6_rule_destroy_msg *msg);
#endif /* _PPE_DRV_PRIORITY_H_ */

