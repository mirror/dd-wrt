/*
 * Copyright (c) 2017, 2020, The Linux Foundation. All rights reserved.
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
 * @file ppe_drv_qos.h
 *	PPE QoS specific definitions.
 */

#ifndef _PPE_DRV_QOS_H_
#define _PPE_DRV_QOS_H_

/**
 * @addtogroup ppe_drv_qos_subsystem
 * @{
 */

#define PPE_DRV_QOS_PORT_MAX	8		/**< Maximum ports for QoS. */
#define PPE_DRV_QOS_DRR_WEIGHT_MAX	1024	/**< Maximum DRR weight for QoS schedulers. */
#define PPE_DRV_QOS_PRIORITY_MAX	7	/**< Maximum priority for QoS schedulers. */
#define PPE_DRV_QOS_MCAST_QUEUE_MAX	1 	/**< Maximum multicast queues per port for QoS. */

/**
 * Queues in PPE are assigned blocks of memory (not packets).
 * Each block is 256B in size.
 */
#define PPE_DRV_QOS_MEM_BLOCK_SIZE 256

/**
 * ppe_drv_qos_res_type
 *	QoS resource type.
 */
enum ppe_drv_qos_res_type {
	PPE_DRV_QOS_RES_TYPE_UCAST_QUEUE,	/**< Unicast queue. */
	PPE_DRV_QOS_RES_TYPE_MCAST_QUEUE,	/**< Multicast queue. */
	PPE_DRV_QOS_RES_TYPE_L0_CDRR,		/**< Level 0 C-DRR.*/
	PPE_DRV_QOS_RES_TYPE_L0_EDRR,		/**< Level 0 E-DRR.*/
	PPE_DRV_QOS_RES_TYPE_L0_SP,		/**< Level 0 strict priority resource.*/
	PPE_DRV_QOS_RES_TYPE_L1_CDRR,		/**< Level 1 C-DRR.*/
	PPE_DRV_QOS_RES_TYPE_L1_EDRR,		/**< Level 1 E-DRR.*/
	PPE_DRV_QOS_RES_TYPE_MAX		/**< Maximum resource type. */
};
typedef enum ppe_drv_qos_res_type ppe_drv_qos_res_type_t;

/**
 * ppe_drv_qos_level
 *	Shaper/Scheduler levels.
 */
enum ppe_drv_qos_level {
	PPE_DRV_QOS_SUB_QUEUE_LEVEL,	/**< Resource is for sub queue level. */
	PPE_DRV_QOS_QUEUE_LEVEL,	/**< Resource is for queue level. */
	PPE_DRV_QOS_FLOW_LEVEL,		/**< Resource is for flow level. */
	PPE_DRV_QOS_PORT_LEVEL,		/**< Resource is for port level. */
	PPE_DRV_QOS_MAX_LEVEL		/**< Maximum resource level. */
};
typedef enum ppe_drv_qos_level ppe_drv_qos_level_t;

/**
 * ppe_drv_qos_queue_color
 *	Supported queue colors.
 */
enum ppe_drv_qos_queue_color {
	PPE_DRV_QOS_QUEUE_COLOR_GREEN,		/**< QoS queue color is green. */
	PPE_DRV_QOS_QUEUE_COLOR_YELLOW,		/**< QoS queue color is yellow. */
	PPE_DRV_QOS_QUEUE_COLOR_RED,		/**< Qos queue color is red. */
	PPE_DRV_QOS_QUEUE_COLOR_MAX		/**< Maximum QoS queue color. */
};
typedef enum ppe_drv_qos_queue_color ppe_drv_qos_queue_color_t;

/**
 * ppe_drv_qos_drr_unit
 *	Quantum unit.
 */
enum ppe_drv_qos_drr_unit {
	PPE_DRV_QOS_DRR_UNIT_BYTE,	/**< QoS Scheduler DRR unit is in bytes. */
	PPE_DRV_QOS_DRR_UNIT_PACKET,	/**< QoS scheduler DRR unit is in packets. */
};
typedef enum ppe_drv_qos_drr_unit ppe_drv_qos_drr_unit_t;

/**
 * ppe_drv_qos_q_stat
 *	Information for QoS queue statistics.
 */
struct ppe_drv_qos_q_stat {
	uint32_t tx_pkts;	/**< Number of Tx packets from QoS queue. */
	uint32_t drop_pkts;	/**< Number of packets dropped from QoS queue. */
	uint64_t tx_bytes;	/**< Number of Tx bytes from QoS queue. */
	uint64_t drop_bytes;	/**< Number of bytes dropped from QoS queue. */
};

/**
 * ppe_drv_qos_queue
 *	Information for QoS queue.
 */
struct ppe_drv_qos_queue {
	uint32_t ucast_qid;	/**< Unicast Queue ID. */
	uint32_t mcast_qid;	/**< Multicast Queue ID. */
	uint32_t qlimit;	/**< Queue limit. */
	uint32_t min_th[PPE_DRV_QOS_QUEUE_COLOR_MAX];
				/**< Minimum threshold. */
	uint32_t max_th[PPE_DRV_QOS_QUEUE_COLOR_MAX];
				/**< Maximum threshold. */
	bool color_en;		/**< Enable color mode. */
	bool red_en;		/**< Enable RED algorithm. */
};

/**
 * ppe_drv_qos_shaper
 *	Information for QoS shaper.
 */
struct ppe_drv_qos_shaper {
	uint32_t rate;		/**< Allowed bandwidth. */
	uint32_t burst;		/**< Allowed burst. */
	uint32_t crate;		/**< Ceil bandwidth. */
	uint32_t cburst;	/**< Ceil burst. */
	uint32_t overhead;	/**< Overhead in bytes to be added for each packet. */
};

/**
 * ppe_drv_qos_scheduler
 *	Information for QoS scheduler.
 */
struct ppe_drv_qos_scheduler {
	uint32_t l0c_drrid;	/**< Level 0 C_DRR ID. */
	uint32_t l0e_drrid;	/**< Level 0 E_DRR ID. */
	uint32_t l1c_drrid;	/**< Level 1 C_DRR ID. */
	uint32_t l1e_drrid;	/**< Level 1 E_DRR ID. */
	uint32_t drr_weight;	/**< DRR weight. */
	ppe_drv_qos_drr_unit_t drr_unit;
				/**< DRR quantum unit. */
	uint32_t priority;	/**< Priority value. */
	bool l1_valid;		/**< Level 1 scheduler resources valid? */
	bool l0_valid;		/**< Level 0 scheduler resources valid? */
};

/**
 * ppe_drv_qos_res
 *	Information for QoS resources.
 */
struct ppe_drv_qos_res {
	struct ppe_drv_qos_queue q;		/**< PPE HW queue related parameters. */
	struct ppe_drv_qos_shaper shaper;	/**< PPE HW shaper parameters. */
	struct ppe_drv_qos_scheduler scheduler;	/**< PPE HW scheduler parameters. */
	uint32_t l0spid;			/**< Level 0 SP Id. */
};

/**
 * ppe_drv_qos_port
 *	Information for QoS port resources.
 */
struct ppe_drv_qos_port {
	uint32_t base[PPE_DRV_QOS_RES_TYPE_MAX];	/**< Base Id for QoS resource. */
	uint32_t max[PPE_DRV_QOS_RES_TYPE_MAX];		/**< Maximum QoS resouurce. */
};

/**
 * Callback function for getting INT-PRI value.
 *
 * @datatypes
 * net_device
 *
 * @param[in] dev         Pointer to the associated net device.
 * @param[in] tag         Qos tag.
 */
typedef uint8_t (*ppe_drv_qos_int_pri_callback_t)(struct net_device *dev, uint32_t tag);

/**
 * ppe_drv_qos_int_pri_callback_unregister
 *	API to unregister INT-PRI fetch callback.
 *
 * @return
 * None
 */
void ppe_drv_qos_int_pri_callback_unregister(void);

/**
 * ppe_drv_qos_int_pri_callback_register
 *	API to register INT-PRI fetch callback.
 *
 * @param[in] cb     Pointer to the callback function.
 *
 * @return
 * None
 */
void ppe_drv_qos_int_pri_callback_register(ppe_drv_qos_int_pri_callback_t cb);

/**
 * ppe_drv_qos_int_pri_get
 *	Returns the INT-PRI value for a class ID.
 *
 * @datatypes
 * struct net_device
 *
 * @param[in] dev       Pointer to the network device.
 * @param[in] classid   Class ID of the Qdisc/class.
 *
 * @return
 * INT-PRI value
 */
int ppe_drv_qos_int_pri_get(struct net_device *dev, uint32_t classid);

/**
 * ppe_drv_qos_queue_stats_get
 *	API to fetch queue statistics from PPE HW.
 *
 * @param[in]  qid         Queue ID.
 * @param[in]  is_red      Is queue RED or FIFO?
 * @param[out] stats       Statistics
 *
 * @return
 * None.
 */
void ppe_drv_qos_queue_stats_get(uint32_t qid, bool is_red, struct ppe_drv_qos_q_stat *stats);

/*
 * ppe_drv_qos_queue_stats_reset
 *	API to reset queue statistics in PPE HW.
 *
 * @param[in] qid      Queue ID.
 *
 * @return
 * None.
 */
void ppe_drv_qos_queue_stats_reset(uint32_t qid);

/**
 * ppe_drv_qos_queue_disable
 *	API to disable a queue in PPE HW.
 *
 * @param[in] port_id  Port ID of the port.
 * @param[in] qid      Queue ID.
 *
 * @return
 * None.
 */
void ppe_drv_qos_queue_disable(uint32_t port_id, uint32_t qid);

/**
 * ppe_drv_qos_queue_enable
 *	API to enable a queue in PPE HW.
 *
 * @param[in] qid      Queue ID.
 *
 * @return
 * None.
 */
void ppe_drv_qos_queue_enable(uint32_t qid);

/**
 * ppe_drv_qos_l1_scheduler_set
 *	Sets a level 1 queue scheduler in PPE.
 *
 * @datatypes
 * ppe_drv_qos_res
 *
 * @param[in] res      Pointer to the QoS resource.
 * @param[in] port_id  Port ID of the port.
 *
 * @return
 * Status of the QoS L1 scheduler configuration operation.
 */
ppe_drv_ret_t ppe_drv_qos_l1_scheduler_set(struct ppe_drv_qos_res *res, uint32_t port_id);

/**
 * ppe_drv_qos_l0_scheduler_reset
 *	Resets Level 0 scheduler configuration in PPE.
 *
 * @datatypes
 * ppe_drv_qos_res
 *
 * @param[in] res      Pointer to the QoS resource.
 * @param[in] port_id  Port ID of the port.
 *
 * @return
 * Status of the QoS QoS L0 scheduler reset operation.
 */
ppe_drv_ret_t ppe_drv_qos_l0_scheduler_reset(struct ppe_drv_qos_res *res, uint32_t port_id);

/**
 * ppe_drv_qos_l0_scheduler_set
 *	Sets a level 0 queue scheduler in PPE.
 *
 * @datatypes
 * ppe_drv_qos_res
 *
 * @param[in] res      Pointer to the QoS resource.
 * @param[in] port_id  Port ID of the port.
 *
 * @return
 * Status of the QoS L0 scheduler configuration operation.
 */
ppe_drv_ret_t ppe_drv_qos_l0_scheduler_set(struct ppe_drv_qos_res *res, uint32_t port_id);

/**
 * ppe_drv_qos_port_shaper_reset
 *	Resets a port shaper in PPE.
 *
 * @datatypes
 * ppe_drv_qos_res
 *
 * @param[in] res      Pointer to the QoS resource.
 * @param[in] port_id  Port ID of the port.
 *
 * @return
 * Status of the QoS port shaper reset operation.
 */
ppe_drv_ret_t ppe_drv_qos_port_shaper_reset(struct ppe_drv_qos_res *res, uint32_t port_id);

/**
 * ppe_drv_qos_port_shaper_set
 *	Configures a port shaper in PPE.
 *
 * @datatypes
 * ppe_drv_qos_res
 *
 * @param[in] res      Pointer to the QoS resource.
 * @param[in] port_id  Port ID of the port.
 *
 * @return
 * Status of the QoS port shaper configuration operation.
 */
ppe_drv_ret_t ppe_drv_qos_port_shaper_set(struct ppe_drv_qos_res *res, uint32_t port_id);

/**
 * ppe_drv_qos_flow_shaper_reset
 *	Resets a flow shaper in PPE.
 *
 * @datatypes
 * ppe_drv_qos_res
 *
 * @param[in] res  Pointer to the QoS resource.
 *
 * @return
 * Status of the QoS flow shaper reset operation.
 */
ppe_drv_ret_t ppe_drv_qos_flow_shaper_reset(struct ppe_drv_qos_res *res);

/**
 * ppe_drv_qos_flow_shaper_set
 *	Configures a flow shaper in PPE.
 *
 * @datatypes
 * ppe_drv_qos_res
 *
 * @param[in] res  Pointer to the QoS resource.
 *
 * @return
 * Status of the QoS flow shaper configuration operation.
 */
ppe_drv_ret_t ppe_drv_qos_flow_shaper_set(struct ppe_drv_qos_res *res);

/**
 * ppe_drv_qos_mcast_queue_shaper_reset
 *	Resets a multicast queue shaper in PPE.
 *
 * @datatypes
 * ppe_drv_qos_res
 *
 * @param[in] res  Pointer to the QoS resource.
 *
 * @return
 * Status of the QoS queue shaper reset operation.
 */
ppe_drv_ret_t ppe_drv_qos_mcast_queue_shaper_reset(struct ppe_drv_qos_res *res);

/**
 * ppe_drv_qos_mcast_queue_shaper_set
 *	Configures a multicast queue shaper in PPE.
 *
 * @datatypes
 * ppe_drv_qos_res
 *
 * @param[in] res  Pointer to the QoS resource.
 *
 * @return
 * Status of the QoS queue shaper configuration operation.
 */
ppe_drv_ret_t ppe_drv_qos_mcast_queue_shaper_set(struct ppe_drv_qos_res *res);

/**
 * ppe_drv_qos_queue_shaper_reset
 *	Resets a queue shaper in PPE.
 *
 * @datatypes
 * ppe_drv_qos_res
 *
 * @param[in] res  Pointer to the QoS resource.
 *
 * @return
 * Status of the QoS queue shaper reset operation.
 */
ppe_drv_ret_t ppe_drv_qos_queue_shaper_reset(struct ppe_drv_qos_res *res);

/**
 * ppe_drv_qos_queue_shaper_set
 *	Configures a queue shaper in PPE.
 *
 * @datatypes
 * ppe_drv_qos_res
 *
 * @param[in] res  Pointer to the QoS resource.
 *
 * @return
 * Status of the QoS queue shaper configuration operation.
 */
ppe_drv_ret_t ppe_drv_qos_queue_shaper_set(struct ppe_drv_qos_res *res);

/**
 * ppe_drv_qos_mcast_queue_set
 *	Configures a multicast queue in PPE.
 *
 * @datatypes
 * ppe_drv_qos_res
 *
 * @param[in] res      Pointer to the QoS resource.
 * @param[in] port_id  Port ID of the port.
 *
 * @return
 * Status of the QoS port shaper reset operation.
 */
ppe_drv_ret_t ppe_drv_qos_mcast_queue_set(struct ppe_drv_qos_res *res, uint32_t port_id);

/**
 * ppe_drv_qos_queue_limit_set
 *	Sets queue size in PPE.
 *
 * @datatypes
 * ppe_drv_qos_res
 *
 * @param[in] res  Pointer to the QoS resource.
 *
 * @return
 * Status of the QoS queue limit set operation.
 */
ppe_drv_ret_t ppe_drv_qos_queue_limit_set(struct ppe_drv_qos_res *res);

/**
 * ppe_drv_qos_default_conf_set
 *	Sets default queue scheduler in PPE.
 *
 * @param[in] port_id  Port ID of the port.
 *
 * @return
 * Status of the default configuration set operation.
 */
ppe_drv_ret_t ppe_drv_qos_default_conf_set(uint32_t port_id);

/**
 *
 * ppe_drv_qos_port_bm_control_enable
 * 	Enable or disable port buffer management for flow control
 *
 * @param[in] port_id  Port ID of the port.
 * @param[in] set      Enable/Disable flow control.
 *
 * @return
 * none
 */
void ppe_drv_qos_port_bm_control_enable(uint32_t port_id, bool set);

/**
 * ppe_drv_qos_port_res_get
 *	Gets boot time QoS resource allocation information for a given port.
 *
 * @datatypes
 * ppe_drv_qos_port
 *
 * @param[in] port_id  Port ID of the port.
 * @param[in] port     Pointer to the port.
 *
 * @return
 * Status of the resource get operation.
 */
ppe_drv_ret_t ppe_drv_qos_port_res_get(uint32_t port_id, struct ppe_drv_qos_port *port);

/** @} */ /* end_addtogroup ppe_drv_qos_subsystem */

#endif /* _PPE_DRV_V4_H_ */
