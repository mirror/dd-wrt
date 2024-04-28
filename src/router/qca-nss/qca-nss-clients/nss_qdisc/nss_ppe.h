/*
 **************************************************************************
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
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

#include <fal_qm.h>
#include <fal_qos.h>
#include <fal_shaper.h>
#include <nss_api_if.h>

/*
 * Queues in PPE are assigned blocks of memory (not packets)
 * Each block is 256B in size.
 */
#define NSS_PPE_MEM_BLOCK_SIZE 256

/*
 * Token number is assigned the max value so as to
 * avoid the packet loss at the start of shaper process.
 */
#define NSS_PPE_TOKEN_MAX		0x3fffffff

#define NSS_PPE_DRR_WEIGHT_MAX		1024

/*
 * Error codes
 */
#define NSS_PPE_QDISC_PARENT_NOT_PPE -1
#define NSS_PPE_QDISC_PARENT_NOT_EXISTING -2

struct nss_qdisc;

/*
 * Quantum unit.
 */
enum nss_ppe_drr_unit {
	NSS_PPE_DRR_UNIT_BYTE,
	NSS_PPE_DRR_UNIT_PACKET,
};
typedef enum nss_ppe_drr_unit nss_ppe_drr_unit_t;

/*
 * Frame mode.
 */
enum nss_ppe_frame_mode {
	NSS_PPE_IPG_PREAMBLE_FRAME_CRC,	/* IPG + Preamble + Frame + CRC */
	NSS_PPE_FRAME_CRC,		/* Frame + CRC */
	NSS_PPE_L3_EXCLUDE_CRC,		/* after Ethernet type exclude CRC*/
};

/*
 * Shaper/Scheduler levels.
 */
enum nss_ppe_level {
	NSS_PPE_SUB_QUEUE_LEVEL,
	NSS_PPE_QUEUE_LEVEL,
	NSS_PPE_FLOW_LEVEL,
	NSS_PPE_PORT_LEVEL,
	NSS_PPE_MAX_LEVEL,
};
typedef enum nss_ppe_level nss_ppe_level_t;

/*
 * Resource type.
 */
enum nss_ppe_res_type {
	NSS_PPE_UCAST_QUEUE,
	NSS_PPE_MCAST_QUEUE,
	NSS_PPE_L0_CDRR,
	NSS_PPE_L0_EDRR,
	NSS_PPE_L0_SP,
	NSS_PPE_L1_CDRR,
	NSS_PPE_L1_EDRR,
	NSS_PPE_MAX_RES_TYPE,
};
typedef enum nss_ppe_res_type nss_ppe_res_type_t;

/*
 * Supported colors
 */
enum nss_ppe_color_types {
	NSS_PPE_COLOR_GREEN,
	NSS_PPE_COLOR_YELLOW,
	NSS_PPE_COLOR_RED,
	NSS_PPE_COLOR_MAX,
};

/*
 * Resource Structure.
 */
struct nss_ppe_res {
	nss_ppe_res_type_t type;	/* resource type */
	uint32_t offset;		/* Resource offset */
	struct nss_ppe_res *next; /* Pointer to next resource */
};

/*
 * nss_ppe_port structure
 */
struct nss_ppe_port {
	uint32_t base[NSS_PPE_MAX_RES_TYPE];	/* Base Id */
	uint32_t max[NSS_PPE_MAX_RES_TYPE];	/* Max resources */
	struct nss_ppe_res *res_used[NSS_PPE_MAX_RES_TYPE];	/* Used res list */
	struct nss_ppe_res *res_free[NSS_PPE_MAX_RES_TYPE];	/* Free res list */

	spinlock_t lock;				/* Lock to protect the port structure */
};

/*
 * nss_ppe_queue structure
 */
struct nss_ppe_queue {
	uint32_t ucast_qid;			/* Unicast Queue ID */
	uint32_t mcast_qid;			/* Multicast Queue ID */
	uint32_t qlimit;			/* Queue limit */
	uint32_t min_th[NSS_PPE_COLOR_MAX];	/* Min threshold */
	uint32_t max_th[NSS_PPE_COLOR_MAX];	/* Max threshold */
	bool color_en;				/* Enable color mode */
	bool red_en;				/* Enable red algorithm */
	bool ucast_valid;			/* Queue ID valid */
	bool mcast_valid;			/* Multicast configuration valid */
	bool mcast_enable;			/* Multicast configuration enabled? */
};

/*
 * nss_ppe_shaper structure
 */
struct nss_ppe_shaper {
	uint32_t rate;		/* Allowed bandwidth */
	uint32_t burst;		/* Allowed burst */
	uint32_t crate;		/* Ceil bandwidth */
	uint32_t cburst;	/* Ceil burst */
	uint32_t overhead;	/* Overhead in bytes to be added for each packet */
};

/*
 * nss_ppe_scheduler structure
 */
struct nss_ppe_scheduler {
	uint32_t drr_weight;	/* DRR weight */
	nss_ppe_drr_unit_t drr_unit;
				/* DRR unit*/
	uint32_t priority;	/* Priority value */
};

/*
 * nss_ppe structure
 */
struct nss_ppe_qdisc {
	enum nss_shaper_config_ppe_sn_type sub_type;	/* Type of PPE Qdisc */
				/* PPE Qdisc type */
	struct nss_ppe_queue q;	/* PPE queue related parameters */
	struct nss_ppe_shaper shaper;	/* PPE shaper parameters */
	struct nss_ppe_scheduler scheduler;	/* PPE scheduler parameters */
	nss_ppe_level_t level;	/* Level at which qdisc is configured */
	uint32_t l0spid;	/* Level 0 SP Id configured in SSDK */
	uint32_t l0c_drrid;	/* Level 0 c_drr Id configured in SSDK */
	uint32_t l0e_drrid;	/* Level 0 e_drr Id configured in SSDK */
	uint32_t l1c_drrid;	/* Level 1 c_drr Id configured in SSDK */
	uint32_t l1e_drrid;	/* Level 1 e_drr Id configured in SSDK */
	bool l1_valid;		/* Level 1 scheduler resources valid */
	bool l0_valid;		/* Level 0 scheduler resources valid */
	bool shaper_present;	/* Shaper parameters present? */
	bool is_configured;	/* Qdisc already configured or not */
};

/*
 * nss_ppe_base_get()
 *	Returns base of the particular resource for a given port.
 */
extern uint32_t nss_ppe_base_get(uint32_t port, nss_ppe_res_type_t type);

/*
 * nss_ppe_port_res_free()
 *	Free resources allocated to PPE ports
 */
extern int nss_ppe_port_res_free(void);

/*
 * nss_ppe_res_alloc()
 *	Allocates free resource for a given port.
 */
extern struct nss_ppe_res *nss_ppe_res_alloc(uint32_t port, nss_ppe_res_type_t type);

/*
 * nss_ppe_res_free()
 *	Frees the allocated resource and attach it to free list.
 */
extern int nss_ppe_res_free(uint32_t port, uint32_t offset, nss_ppe_res_type_t type);

/*
 * nss_ppe_port_res_alloc()
 *	Allocates per port resources
 */
extern int nss_ppe_port_res_alloc(void);

/*
 * nss_ppe_mcast_queue_reset()
 *	Deconfigures and deallocates a multicast queue in SSDK.
 */
extern int nss_ppe_mcast_queue_reset(struct nss_qdisc *nq);

/*
 * nss_ppe_mcast_queue_set()
 *	Allocates and configures a multicast queue in SSDK.
 */
extern int nss_ppe_mcast_queue_set(struct nss_qdisc *nq);

/*
 * nss_ppe_drr_weight_get()
 *	Returns the DRR weight corresponding to quantum.
 */
extern int nss_ppe_drr_weight_get(uint32_t quantum, nss_ppe_drr_unit_t drr_unit);

/*
 * nss_ppe_port_num_get()
 *	Returns the port number.
 */
extern int nss_ppe_port_num_get(struct nss_qdisc *nq);

/*
 * nss_ppe_set_parent()
 *	Sets the parent of given qdisc.
 */
extern int nss_ppe_set_parent(struct Qdisc *sch, struct nss_qdisc *nq, uint32_t parent);

/*
 * nss_ppe_get_max_prio_bands()
 *	Returns the number of PRIO bands supported based on qdisc level.
 */
extern int nss_ppe_get_max_prio_bands(struct nss_qdisc *nq);

/*
 * nss_ppe_node_detach()
 *	Configuration function that helps detach a child shaper node from a parent.
 */
extern int nss_ppe_node_detach(struct nss_qdisc *nq, struct nss_qdisc *nq_child);

/*
 * nss_ppe_node_attach()
 *	Configuration function that helps attach a child shaper node to a parent.
 */
extern int nss_ppe_node_attach(struct nss_qdisc *nq, struct nss_qdisc *nq_child);

/*
 * nss_ppe_node_attach()
 *	Configures the SSDK schedulers and NSS shapers.
 */
extern int nss_ppe_configure(struct nss_qdisc *nq, struct nss_ppe_qdisc *prev_npq);

/*
 * nss_ppe_fallback_to_nss()
 *	Calls the initialization of NSS Qdisc when PPE initialization fails.
 */
extern int nss_ppe_fallback_to_nss(struct nss_qdisc *nq, struct nlattr *opt);

/*
 * nss_ppe_destroy()
 *	Destroys the qdisc.
 */
extern void nss_ppe_destroy(struct nss_qdisc *nq);

/*
 * nss_ppe_init()
 *	Initializes NSS Qdisc and sets the level of this qdisc.
 */
extern int nss_ppe_init(struct Qdisc *sch, struct nss_qdisc *nq, nss_shaper_node_type_t type);
