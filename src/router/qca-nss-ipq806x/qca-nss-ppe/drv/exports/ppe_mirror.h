/*
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * @file ppe_mirror.h
 *	NSS PPE MIRROR definitions.
 */

#ifndef _PPE_MIRROR_H_
#define _PPE_MIRROR_H_

#include <linux/if.h>
#include <linux/if_ether.h>
#include <ppe_drv_port.h>

/**
 * ppe_mirror_capture_callback_t
 *	Callback function definition for user external callback for captured packets.
 */
typedef void (*ppe_mirror_capture_callback_t)(void *app_data, struct sk_buff *skb, struct net_device *dev);

/**
 * ppe_mirror_ret
 *      PPE mirror return codes.
 */
typedef enum ppe_mirror_ret {
	/**< Success */
        PPE_MIRROR_RET_SUCCESS = 0,

	/**< Add cases */
	PPE_MIRROR_RET_ADD_FAIL_MAPPING_INVALID_ACL_ID,		/**< Invalid rule ID. */
	PPE_MIRROR_RET_ADD_FAIL_MAPPING_INVALID_ACL_RULE,	/**< Invalid ACL rule. */
	PPE_MIRROR_RET_ADD_FAIL_MAPPING_CB_REG,			/**< Failed to register cb. */
	PPE_MIRROR_RET_ADD_FAIL_MAPPING_EXIST,			/**< Mapping already exists. */
	PPE_MIRROR_RET_ADD_FAIL_NO_MEM,				/**< Not enough memory. */
	PPE_MIRROR_RET_ADD_FAIL_GROUP_INFO_INVALID,		/**< Invalid group info. */

	/** Delete cases */
	PPE_MIRROR_RET_DELETE_FAIL_MAPPING_INVALID_ACL_ID,	/**< Invalid rule ID. */
	PPE_MIRROR_RET_DELETE_FAIL_MAPPING_INVALID_ACL_RULE,	/**< Invalid ACL rule. */
	PPE_MIRROR_RET_DELETE_FAIL_MAPPING_NOT_FOUND,		/**< Mapping not found. */
	PPE_MIRROR_RET_DELETE_FAIL_GROUP_NOT_FOUND,		/**< Group not found. */

	/** Capture Core */
	PPE_MIRROR_RET_INVALID_CAPTURE_CORE,			/**< Invalid capture core request. */
	PPE_MIRROR_RET_FAIL_EN_CAPTURE_CORE,			/**< Failed to enable capture core. */

	PPE_MIRROR_RET_GENERIC_FAILURE,				/**< Generic failure. */
} ppe_mirror_ret_t;

/*
 * ppe_mirror_phy_port_list
 *	Physical ports to be mapped for mirroring.
 */
struct ppe_mirror_phy_port_list {
	struct net_device *dev;				/**< Physical port. */
	bool is_valid;					/**< If entry is valid. */
};

/**
 * ppe_mirror_port_mapping_info
 *	Exported structure to get the port mapping information.
 */
struct ppe_mirror_port_mapping_info {
	struct ppe_mirror_phy_port_list mirror_phy_port_list[PPE_DRV_PHYSICAL_MAX];
						/**< List of physical devices. */
	struct net_device *capture_dev;		/**< Capture net device. */
	ppe_mirror_capture_callback_t cb;	/**< Capture callback. */
	void *app_data;				/**< APP data. */
};

/**
 * ppe_mirror_acl_mapping_info
 *	Exported structure to get mapping information.
 */
struct ppe_mirror_acl_mapping_info {
	uint16_t acl_id;			/**< User ID of the ACL mirror rule. */
	struct net_device *capture_dev;		/**< Capture net device. */
	ppe_mirror_capture_callback_t cb;	/**< Capture callback. */
	void *app_data;				/**< APP data. */
};

/**
 * ppe_mirror_acl_mapping_add()
 *	Add ACL rule mapping for packet mirroring.
 *
 * @datatypes
 * ppe_mirror_acl_mapping_info
 *
 * @param[IN] mapping_info		Mirror mapping information.
 *
 * @return
 * Return error code for mapping addition.
 */
ppe_mirror_ret_t ppe_mirror_acl_mapping_add(struct ppe_mirror_acl_mapping_info *mapping_info);

/**
 * ppe_mirror_acl_mapping_delete()
 *	Delete ACL rule mapping for packet mirroring.
 *
 * @param[IN] acl_id		ACL ID for mapping delete.
 *
 * @return
 * Return error code for mapping deletion.
 */
ppe_mirror_ret_t ppe_mirror_acl_mapping_delete(uint16_t acl_id);

/**
 * ppe_mirror_phy_port_mapping_add()
 *	Add physical port mapping for packet mirroring.
 *
 * @datatypes
 * ppe_mirror_port_mapping_info
 *
 * @param[IN] pdev_mapping_info		Mirror mapping information.
 *
 * @return
 * Return error code for mapping addition.
 */
ppe_mirror_ret_t ppe_mirror_phy_port_mapping_add(struct ppe_mirror_port_mapping_info *pdev_mapping_info);

/**
 * ppe_mirror_phy_port_mapping_delete()
 *	Delete pdev mapping for packet mirroring.
 *
 * @datatypes
 * ppe_mirror_port_mapping_info
 *
 * @param[IN] pdev_mapping_info		Mirror mapping information.
 *
 * @return
 * Return error code for mapping deletion.
 */
ppe_mirror_ret_t ppe_mirror_phy_port_mapping_delete(struct ppe_mirror_port_mapping_info *pdev_mapping_info);

/**
 * ppe_mirror_enable_capture_core()
 *	Configure the ARM core ID for directing mirror packets to.
 *
 * @param[IN] core_id		Core id for mirrored packets.
 *
 * @return
 * Return error code for enabling capture core.
 */
ppe_mirror_ret_t ppe_mirror_enable_capture_core(uint8_t core_id);

#endif /* _PPE_MIRROR_H_ */
