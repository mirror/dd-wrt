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
 * @file ppe_policer.h
 *	NSS PPE RFS definitions.
 */

#ifndef _PPE_POLICER_H_
#define _PPE_POLICER_H_

#include <ppe_drv_public.h>

#define PPE_RULE_POLICER_ACTION_YELLOW_FORWARD	0x1	/**< Forward all yellow coloured traffic */
#define PPE_RULE_POLICER_ACTION_YELLOW_CHG_PRI  0x2	/**< Change priority all yellow coloured traffic */
#define PPE_RULE_POLICER_ACTION_YELLOW_CHG_DP   0x4	/**< Change drop for yellow coloured traffic */
#define PPE_RULE_POLICER_ACTION_YELLOW_CHG_PCP  0x8	/**< Change PCP for yellow coloured traffic */
#define PPE_RULE_POLICER_ACTION_YELLOW_CHG_DEI  0x10	/**< Change DEI yellow coloured traffic */
#define PPE_RULE_POLICER_ACTION_YELLOW_CHG_DSCP  0x20	/**< Change DSCP for yellow coloured traffic */

/*
 * ppe_policer_meter_unit
 *	meter_unit
 */
enum ppe_policer_meter_unit {
	PPE_POLICER_METER_UNIT_BYTE = 0,	/**< Byte based configuration */
	PPE_POLICER_METER_UNIT_PACKET,		/**< Packet based configuration */
};

/*
 * ppe_policer_supported_mode
 *	ppe policer supported rfc
 */
enum ppe_policer_supported_mode {
	PPE_POLICER_MODE_RFC2698 = 0,		/**< RFC2698 enabled */
	PPE_POLICER_MODE_RFC2697,		/**< RFC2698 enabled */
	PPE_POLICER_MODE_RFC4115,		/**< RFC4115 enabled */
	PPE_POLICER_MODE_MEF10_3,		/**< MEF10_3 enabled */
};

/*
 * ppe_policer_type
 *	Policer type
 */
enum ppe_policer_type {
	PPE_POLICER_TYPE_ACL = 0,		/**< Policer type ACL */
	PPE_POLICER_TYPE_PORT,			/**< Policer type Port */
};

/*
 * ppe_policer_status
 *	ppe rule status
 */
typedef enum ppe_policer_ret {
	PPE_POLICER_SUCCESS = 0,			/**< Success */
	PPE_POLICER_PORT_RET_DESTROY_FAIL_INVALID_ID,	/**< Port destroy Invalid ID */
	PPE_POLICER_PORT_RET_CREATE_FAIL_OOM,		/**< Port create fail due to OOM */
	PPE_POLICER_PORT_RET_CREATE_FAIL,		/**< Port creation fail */
	PPE_POLICER_PORT_RET_CREATE_FAIL_RULE_CONFIG,	/**< Port create rule config failure */
	PPE_POLICER_PORT_RET_DESTROY_FAIL,		/**< Port destroy fail */
	PPE_POLICER_ACL_RET_DESTROY_FAIL_INVALID_ID,	/**< Destroy Invalid ID */
	PPE_POLICER_ACL_RET_CREATE_FAIL_OOM,		/**< Create fail due to OOM */
	PPE_POLICER_ACL_RET_CREATE_FAIL,		/**< ACL policer create failure */
	PPE_POLICER_ACL_RET_CREATE_FAIL_RULE_CONFIG,	/**< ACL policer rule failure */
	PPE_POLICER_ACL_RET_CREATE_SUCCESS,		/**< ACL policer create success */
	PPE_POLICER_ACL_RET_DESTROY_FAIL,		/**< ACL Destroy failure */
	PPE_POLICER_CREATE_V4_RULE_FAILURE,		/**< Create v4 rule failure */
	PPE_POLICER_CREATE_V6_RULE_FAILURE,		/**< Create v6 rule failure */
	PPE_POLICER_DESTROY_V4_RULE_FAILURE,		/**< Destroy v4 rule failure */
	PPE_POLICER_DESTROY_V6_RULE_FAILURE,		/**< Destroy v6 rule failure */
} ppe_policer_ret_t;

/*
 * ppe_policer_action_info
 *	PPE policer action info
 */
struct ppe_policer_action_info {
	uint8_t yellow_pri;		/**< Yellow coloured packet priority */
	uint8_t yellow_dp;		/**< Yellow coloured packet drop priority */
	uint8_t yellow_pcp;		/**< Yellow coloured packet pcp */
	uint8_t yellow_dei;		/**< Yellow coloured packet dei */
	uint8_t yellow_dscp;		/**< Yellow coloured packet dscp */
};

/*
 * ppe_policer_config
 *	ppe policer create common information
 */
struct ppe_policer_config {
	uint32_t committed_rate;			/**< Packets per second or bits per second */
	uint32_t committed_burst_size;			/**< Bytes */
	uint32_t peak_rate;				/**< Packets per second or bits per second */
	uint32_t peak_burst_size;			/**< bytes */
	uint32_t action_flags;				/**< Action flags */
	bool colour_aware;		/**< Operation is colour aware */
	bool meter_enable;		/**< Metering is enabled or disabled */
	bool couple_enable;		/**< Couple mode is enabled or disabled */
	enum ppe_policer_supported_mode mode;	/**< RFC supported */
	enum ppe_policer_meter_unit meter_unit;	/**< meter_unit; 0: Byte based 1 : Packet based */
	struct ppe_policer_action_info action_info;	/**< Action info */
};

/*
 * ppe_policer_destroy_info
 *	User policer destroy information
 */
struct ppe_policer_destroy_info {
	/* Only for ACL Policer */
	uint16_t rule_id;				/**< Rule id for ACL + Policer */

	/* Only for Port Policer */
	enum ppe_policer_type policer_type;		/**< Port policer is enabled */
	char name[IFNAMSIZ];				/**< Net device for port policer */
};

/*
 * ppe_policer_create_info
 *	User policer create information
 */
struct ppe_policer_create_info {
	/* common configuration */
	struct ppe_policer_config config;	/**< Create configuration */

	/* Only for ACL Policer */
	uint16_t rule_id;				/**< Rule id for ACL + Policer */

	/* Only for Port Policer */
	enum ppe_policer_type policer_type;		/**< Port policer is enabled */
	char name[IFNAMSIZ];				/**< Net device for port policer */

	/*
	 * Responses
	 */
	enum ppe_policer_ret ret;			/**< Return status */
};

/**
 * ppe_policer_v6_noedit_flow_create
 *	Create PPE flow rule that does not edit the packet and polices the packets that match the flow.
 *
 * @datatypes
 * struct ppe_drv_v6_rule_create
 *
 * @param[in] create Create message.
 *
 * @return
 * status of ppe_policer_v6_noedit_flow_create
 */
ppe_policer_ret_t ppe_policer_v6_noedit_flow_create(struct ppe_drv_v6_rule_create *create);

/**
 * ppe_policer_v6_noedit_flow_destroy
 *	Destroy PPE flow rule that does not edit the packet and polices the packets that match the flow.
 *
 * @datatypes
 * struct ppe_drv_v6_rule_destroy
 *
 * @param[in] destroy destroy message.
 *
 * @return
 * status of ppe_policer_v6_noedit_flow_destroy
 */
ppe_policer_ret_t ppe_policer_v6_noedit_flow_destroy(struct ppe_drv_v6_rule_destroy *destroy);

/**
 * ppe_policer_v4_noedit_flow_create
 *	Create PPE flow rule that does not edit the packet and polices the packets that match the flow.
 *
 * @datatypes
 * struct ppe_drv_v4_rule_create
 *
 * @param[in] create Create message.
 *
 * @return
 * status of ppe_policer_v4_noedit_flow_create
 */
ppe_policer_ret_t ppe_policer_v4_noedit_flow_create(struct ppe_drv_v4_rule_create *create);

/**
 * ppe_policer_v4_noedit_flow_destroy
 *	Destroy PPE flow rule that does not edit the packet and polices the packets that match the flow.
 *
 * @datatypes
 * struct ppe_drv_v4_rule_destroy
 *
 * @param[in] destroy destroy message.
 *
 * @return
 * status of ppe_policer_v4_noedit_flow_destroy
 */
ppe_policer_ret_t ppe_policer_v4_noedit_flow_destroy(struct ppe_drv_v4_rule_destroy *destroy);

/**
 * ppe_policer_destroy
 *	Destroy PPE policer rule.
 *
 * @datatypes
 * struct ppe_policer_destroy_info
 *
 * @param[in] policer_user_id User policer id.
 *
 * @return
 * status of ppe_policer_destroy
 */
ppe_policer_ret_t ppe_policer_destroy(struct ppe_policer_destroy_info *destroy);

/**
 * ppe_policer_create
 *	Create PPE policer rule.
 *
 * @datatypes
 * struct ppe_policer_create_info
 *
 * @param[in] struct ppe_policer_info User configuration for policer.
 *
 * @return
 * status of ppe_policer_create
 */
ppe_policer_ret_t ppe_policer_create(struct ppe_policer_create_info *create);

#endif /* _PPE_POLICER_H_ */
