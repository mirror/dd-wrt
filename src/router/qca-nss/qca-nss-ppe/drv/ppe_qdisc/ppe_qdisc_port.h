/*
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
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

#ifndef _PPE_QDISC_PORT_H_
#define _PPE_QDISC_PORT_H_

/*
 * Resource Structure.
 */
struct ppe_qdisc_port_res {
	ppe_drv_qos_res_type_t type;		/* resource type */
	uint32_t offset;			/* Resource offset */
	struct ppe_qdisc_port_res *next;	/* Pointer to next resource */
};

/*
 * ppe_qdisc_port structure
 */
struct ppe_qdisc_port {
	struct ppe_drv_qos_port port;	/* QoS port */
	struct ppe_qdisc_port_res *res_used[PPE_DRV_QOS_RES_TYPE_MAX];
					/* Used res list */
	struct ppe_qdisc_port_res *res_free[PPE_DRV_QOS_RES_TYPE_MAX];
					/* Free res list */
	spinlock_t lock;		/* Lock to protect the port structure */
};

/*
 * ppe_qdisc_port_default_conf_set()
 *	Sets default queue scheduler in SSDK.
 */
extern int ppe_qdisc_port_default_conf_set(uint32_t port_id);

/*
 * ppe_qdisc_port_disable_all_queue()
 *	Disables all queues corresponding to a port.
 */
extern void ppe_qdisc_port_disable_all_queue(uint32_t port_id);

/*
 * ppe_qdisc_port_enable_all_queue()
 *	Enables all level L0 queues corresponding to a port.
 */
extern void ppe_qdisc_port_enable_all_queue(uint32_t port_id);

/*
 * ppe_qdisc_port_res_base_get()
 *	Returns base of the particular resource for a given port.
 */
extern uint32_t ppe_qdisc_port_res_base_get(uint32_t port_id, ppe_drv_qos_res_type_t type);

/*
 * ppe_qdisc_port_res_free()
 *	Frees the allocated resource and attach it to free list.
 */
extern int ppe_qdisc_port_res_free(uint32_t port, uint32_t offset, ppe_drv_qos_res_type_t type);

/*
 * ppe_qdisc_port_res_alloc()
 *	Allocates free resource for a given port.
 */
extern struct ppe_qdisc_port_res *ppe_qdisc_port_res_alloc(uint32_t port, ppe_drv_qos_res_type_t type);

/*
 * ppe_qdisc_port_id_get()
 *	Returns the port number associated with the net device.
 */
extern int32_t ppe_qdisc_port_id_get(struct net_device *dev);

/*
 * ppe_qdisc_port_free()
 *	Free PPE ports
 */
extern int ppe_qdisc_port_free(void);

/*
 * ppe_qdisc_port_alloc()
 *	Initializes PPE ports
 */
extern int ppe_qdisc_port_alloc(void);

#endif
