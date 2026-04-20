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

/*
 * ppe_drv_policer_type
 */
enum ppe_drv_policer_type {
	PPE_DRV_POLICER_TYPE_NONE = 0,		/**< Policer type None */
	PPE_DRV_POLICER_TYPE_ACL,		/**< ACL policer type */
	PPE_DRV_POLICER_TYPE_PORT,		/**< Port Policer type */
	PPE_DRV_POLICER_TYPE_MAX,		/**< Maxumum */
};

/*
 * ppe_drv_policer_meter_mode
 *	RFCs supported
 */
enum ppe_drv_policer_meter_mode {
	PPE_DRV_POLICER_METER_MODE_RFC2698 = 0,	/**< RFC2698 */
	PPE_DRV_POLICER_METER_MODE_OTHER,	/**< RFC2697/RFC4115 */
};

/*
 * ppe_drv_policer_meter_unit
 *	meter unit
 */
enum ppe_drv_policer_meter_unit {
	PPE_DRV_POLICER_METER_UNIT_BYTE = 0,	/**< Byte based configuration */
	PPE_DRV_POLICER_METER_UNIT_PACKET,	/**< Packet based configuration */
};

/*
 * ppe_drv_policer_colour_mode
 *	Colour mode
 */
enum ppe_drv_policer_colour_mode {
	PPE_DRV_POLICER_COLOUR_BLIND = 0,	/** Colour blind mode */
	PPE_DRV_POLICER_COLOUR_AWARE,		/** Colour Aware mode */
};

struct ppe_drv_policer_acl;
struct ppe_drv_policer_port;

/*
 * ppe_drv_policer_flow
 *	Structure for flow and policer binding.
 */
struct ppe_drv_policer_flow {
	/*
	 * Input: Policer rule ID.
	 */
	uint16_t id;				/**< User based policer rule id */
	bool pkt_noedit;			/**< Policer done in non-edit mode */

	/*
	 * Output: Associated service code.
	 */
	uint8_t sc;				/**< Service code */

	/*
	 * For Some SoC, service code is optional
	 */
	bool sc_valid;				/**< Service code valid */
};

/*
 * ppe_drv_policer_acl_rule_info
 *	ACL configuration information
 */
struct ppe_drv_policer_remap_info {
	uint8_t new_dscp;		/**< Remapped DSCP value */
	uint8_t new_pcp;		/**< Remapped PCP value */
	uint8_t new_dei;		/**< Remapped DEI value */
};

/*
 * ppe_drv_policer_rule_destroy_acl_info
 *	ACL configuration information
 */
struct ppe_drv_policer_rule_destroy_acl_info {
	uint16_t acl_index;			/**< Rule index */
};

/*
 * ppe_drv_policer_rule_destroy_port_info
 *	ACL configuration information
 */
struct ppe_drv_policer_rule_destroy_port_info {
	uint16_t port_index;			/**< Port index */
};

/*
 * ppe_drv_policer_rule_create_action
 *	Action for rule
 */
struct ppe_drv_policer_rule_create_action {
	/* Configuration for Yellow traffic */
	bool exceed_chg_pri;		/**< 1 - Enabled, 0 - Disable */
	bool exceed_chg_dp;		/**< 1 - Enabled, 0 - Disable */
	bool exceed_chg_pcp;		/**< 1 - Enabled, 0 - Disable */
	bool exceed_chg_dei;		/**< 1 - Enabled, 0 - Disable */
	uint32_t exceed_pri;		/**< Priority value */
	uint32_t exceed_dp;		/**< DP value */
	uint32_t exceed_pcp;		/**< PCP value for remarking */
	uint32_t exceed_dei;		/**< DEI value for remarking */

	/* Configuration for Red traffic */
	bool red_drop;			/**< Drop RED traffic or forward */
	bool violate_chg_pri;		/**< 1 - Enabled, 0 - Disable */
	bool violate_chg_dp;		/**< 1 - Enabled, 0 - Disable */
	bool violate_chg_pcp;		/**< 1 - Enabled, 0 - Disable */
	bool violate_chg_dei;		/**< 1 - Enabled, 0 - Disable */
	uint32_t violate_pri;		/**< Priority value */
	uint32_t violate_dp;		/**< DP value */
	uint32_t violate_pcp;		/**< PCP value for remarking */
	uint32_t violate_dei;		/**< DEI value for remarking */

	bool exceed_chg_dscp;		/**< 1 - Enabled, 0 - Disable */
	uint32_t exceed_dscp;		/**< dscp value */

	bool violate_chg_dscp;		/**< 1 - Enabled, 0 - Disable */
	uint32_t violate_dscp;		/**< dscp value */

	bool violate_remap;		/**< 1 - remap table has higher priority than remark */
	bool exceed_remap;		/**< 1 - remap table has higher priority than remark */
};

/*
 * ppe_drv_policer_rule_create_acl_info
 *	ACL  configuration information
 */
struct ppe_drv_policer_rule_create_acl_info {
	bool meter_en;			/**< 1 - Metering enabled, 0 - disabled */
	bool colour_mode;		/**< 0 - Colour bind, 1- Colour Aware */
	bool coupling_flag;		/**< 1- couple C + E bucket; 0 - discouple */
	bool meter_mode;		/**< 0 - RFC2698, 1 - Rest RFC */
	bool meter_unit;		/**< 0 - byte based, 1 - packet based */
	uint8_t token_unit;		/**< 0 - bits 1 - frame */
	uint16_t cbs;			/**< Committed burst size */
	uint32_t cir;			/**< Commiitted Information rate */
	uint16_t ebs;			/**< Exceeded burst size */
	uint32_t eir;			/**< Exceeded Information rate */

	/* MEF */
	uint32_t cir_max;
	uint32_t eir_max;
	bool grp_end;			/**< Is this policer group end */
	bool grp_cf;			/**< Counter overflow */
	uint16_t next_ptr;		/**< Next policer number */

	struct ppe_drv_policer_rule_create_action action; /* Action for traffic */
};

/*
 * ppe_drv_policer_rule_create_port_info
 *	Port configuration information
 */
struct ppe_drv_policer_rule_create_port_info {
	struct net_device *dev;		/**< Dev on which policing is needed */
	bool meter_en;			/**< 1 - Metering enabled, 0 - disabled */
	bool colour_mode;		/**< 0 - Colour bind, 1- Colour Aware */
	uint8_t meter_flag;		/**< 0 - unicast, 1 - known unicast, 2 - multicast, 3 - unknown multicast, 4 - broadcast */
	bool coupling_flag;		/**< 1- couple C + E bucket; 0 - discouple */
	bool meter_mode;		/**< 0 - RFC2698, 1 - Rest RFC */
	bool meter_unit;		/**< 0 - byte based, 1 - packet based */
	uint8_t token_unit;		/**< 0 - bits 1 - frame */
	uint32_t cbs;			/**< Committed burst size */
	uint32_t cir;			/**< Commiitted Information rate */
	uint32_t ebs;			/**< Exceeded burst size */
	uint32_t eir;			/**< Exceeded Information rate */

	struct ppe_drv_policer_rule_create_action action; /* Action for traffic */
};

/*
 * ppe_drv_policer_rule_destroy
 *	External exposed structure to ppe rule module for policer rule destroy
 */
struct ppe_drv_policer_rule_destroy {
	union {
		struct ppe_drv_policer_rule_destroy_port_info port_info;	/**< Port information */
		struct ppe_drv_policer_rule_destroy_acl_info acl_info;		/**< Policer information */
	} msg;
};

/*
 * ppe_drv_policer_rule_create
 *	External exposed structure to ppe rule module for policer rule create
 */
struct ppe_drv_policer_rule_create {
	union {
		struct ppe_drv_policer_rule_create_port_info port_info;		/**< Port information */
		struct ppe_drv_policer_rule_create_acl_info acl_info;		/**< Policer information */
	} msg;

	/* Response */
	uint16_t hw_id;		/**< Port index for port policer; acl index for acl policer */
};

/**
 * ppe_drv_policer_flow_callback_t
 *	Flow policer callback type
 *
 * @param[IN] app_data	Application data.
 * @param[IN] info	Policer Information.
 *
 * @return
 * true or false.
 */
typedef bool (*ppe_drv_policer_flow_callback_t)(void *app_data, struct ppe_drv_policer_flow *info);

/**
 * ppe_drv_policer_flow_unregister_cb
 *	Un-register policer flow callbacks with ppe-rule.
 *
 * @return
 * none.
 */
void ppe_drv_policer_flow_unregister_cb(void);

/**
 * ppe_drv_policer_flow_register_cb
 *	Register policer flow callbacks with ppe-rule.
 *
 * @param[IN] add_cb	Flow add callback.
 * @param[IN] del_cb	Flow delete callback.
 * @param[IN] app_data	Application data.
 *
 * @return
 * none.
 */
void ppe_drv_policer_flow_register_cb(ppe_drv_policer_flow_callback_t add_cb, ppe_drv_policer_flow_callback_t del_cb, void *app_data);

/**
 * ppe_drv_policer_get_policer_id
 *	Get HW policer index
 *
 * @param[IN] ctx ACL policer context.
 *
 * @return
 * Policer HW index.
 */
uint16_t ppe_drv_policer_get_policer_id(struct ppe_drv_policer_acl *ctx);

/**
 * ppe_drv_policer_get_port_id
 *	Get port index
 *
 * @param[IN] ctx  Port policer context.
 *
 * @return
 * Port index.
 */
uint16_t ppe_drv_policer_get_port_id(struct ppe_drv_policer_port *ctx);

/**
 * ppe_drv_policer_user2hw_id_map
 *	Map user to HW id in driver table
 *
 * @param[IN] ctx	PPE driver policer context.
 * @param[IN] user_id	Policer User ID.
 *
 * @return
 * None.
 */
void ppe_drv_policer_user2hw_id_map(struct ppe_drv_policer_acl *ctx, int user_id);

/**
 * ppe_drv_policer_acl_destroy
 *	Destroy ACL policer in PPE HW
 *
 * @param[IN] ctx ACL policer context.
 *
 * @return
 * none.
 */
void ppe_drv_policer_acl_destroy(struct ppe_drv_policer_acl *ctx);

/**
 * ppe_drv_policer_port_destroy
 *	Destroy Port policer in PPE HW
 *
 * @param[IN] ctx Port policer context.
 *
 * @return
 * none.
 */
void ppe_drv_policer_port_destroy(struct ppe_drv_policer_port *ctx);

/**
 * ppe_drv_policer_acl_create
 *	create ACL policer in PPE HW
 *
 * @param[IN] ctx ACL policer context.
 *
 * @return
 * status.
 */
struct ppe_drv_policer_acl *ppe_drv_policer_acl_create(struct ppe_drv_policer_rule_create *create);

/**
 * ppe_drv_policer_port_create
 *	Create Port policer in PPE HW
 *
 * @param[IN] ctx Port policer context.
 *
 * @return
 * status.
 */
struct ppe_drv_policer_port *ppe_drv_policer_port_create(struct ppe_drv_policer_rule_create *create);
