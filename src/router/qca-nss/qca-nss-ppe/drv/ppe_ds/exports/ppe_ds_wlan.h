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
 * @file ppe_ds_wlan.h
 *	PPE-DS WLAN specific definitions.
 */

#ifndef _PPE_DS_WLAN_H_
#define _PPE_DS_WLAN_H_
#include <ppe_vp_public.h>

/*
 * ppe_ds_wlan_node_type_t
 *	PPE-DS node type
 */
typedef enum {
	PPE_DS_NODE_TYPE_2G,
	PPE_DS_NODE_TYPE_5G,
	PPE_DS_NODE_TYPE_6G,
	PPE_DS_NODE_TYPE_MAX,
} ppe_ds_wlan_node_type_t;

/**
 * ppe_ds_wlan_txdesc_elem
 *	PPEDS WLAN Tx descriptor element information
 */
struct ppe_ds_wlan_txdesc_elem {
	uint32_t opaque_lo;		/**< Low 32-bit opaque field content */
	uint32_t opaque_hi;		/**< High 32-bit opaque field content */
	dma_addr_t buff_addr;		/**< Buffer address */
};

/**
 * ppe_ds_wlan_rxdesc_elem
 *	PPEDS WLAN Rx descriptor element information
 */
struct ppe_ds_wlan_rxdesc_elem {
	unsigned long cookie;		/**< cookie information */
};

/**
 * ppe_ds_wlan_reg_info
 *	PPE-DS WLAN rings information
 */
struct ppe_ds_wlan_reg_info {
	dma_addr_t ppe2tcl_ba;		/**< PPE2TCL ring base address */
	dma_addr_t reo2ppe_ba;		/**< REO2PPE ring base address */
	uint32_t ppe2tcl_num_desc;	/**< PPE2TCL ring descriptor count */
	uint32_t reo2ppe_num_desc;	/**< REO2PPE ring descriptor count */
	ppe_ds_wlan_node_type_t node_type;	/**< PPE-DS node type */
	uint32_t ppe2tcl_start_idx;		/**< PPE2TCL ring index */
	uint32_t reo2ppe_start_idx;		/**< REO2PPE ring index */
	bool ppe_ds_int_mode_enabled;  /**< Interrupt mode to process PPE2TCL */
};

/**
 * ppe_ds_wlan_handle
 *	PPE-DS WLAN handle
 */
typedef struct ppe_ds_wlan_handle {
	uint32_t reserved;			/**< Reserved */
	char priv[] __aligned(NETDEV_ALIGN);	/**< contains the address of WLAN SoC base address */
} ppe_ds_wlan_handle_t;

/**
 * ppe_ds_wlan_ctx_info_handle
 *	PPE-DS wlan umac reset handle
 */
struct ppe_ds_wlan_ctx_info_handle {
	uint32_t umac_reset_inprogress;		/**< umac reset in progress information */
};

/**
 * ppe_ds_wlan_ops
 *	PPE-DS WLAN operations
 */
struct ppe_ds_wlan_ops {
	uint32_t (*get_tx_desc_many)(ppe_ds_wlan_handle_t *, struct ppe_ds_wlan_txdesc_elem *,
			 uint32_t num_buff_req, uint32_t buff_size, uint32_t headroom);
				/**< Callback to get WLAN Tx descriptors and buffers */
	void (*release_tx_desc_single)(ppe_ds_wlan_handle_t *, uint32_t cookie);
				/**< Callback to release WLAN Tx descriptor and buffer */
	void (*set_tcl_prod_idx)(ppe_ds_wlan_handle_t *, uint16_t tcl_prod_idx);
				/**< Callback to set PPE2TCL ring's producer index */
	void (*set_reo_cons_idx)(ppe_ds_wlan_handle_t *, uint16_t reo_cons_idx);
				/**< Callback to set REO2PPE ring's consumer index */
	uint16_t (*get_tcl_cons_idx)(ppe_ds_wlan_handle_t *);
				/**< Callback to get PPE2TCL ring's consumer index */
	uint16_t (*get_reo_prod_idx)(ppe_ds_wlan_handle_t *);
				/**< Callback to get REO2PPE ring's producer index */
	void (*release_rx_desc)(ppe_ds_wlan_handle_t *ppeds_handle,
			struct ppe_ds_wlan_rxdesc_elem *arr, uint16_t count);
				/**< Callback to release WLAN Rx descriptors and buffers */
	void (*enable_tx_consume_intr)(ppe_ds_wlan_handle_t *ppeds_handle,
					bool enable);
				/**< Callback to toggle wlan interrupt */
	void (*notify_napi_done)(ppe_ds_wlan_handle_t *ppeds_handle);
				/**< Callback to trigger after ppeds ring process completes */
};

/**
 * ppe_ds_wlan_priv
 *	Wrapper to return PPE-DS WLAN handle's private area pointer.
 *
 * @datatypes
 * ppe_ds_wlan_handle_t
 *
 * @param[in] wlan_handle   PPE-DS WLAN handle
 */
static inline void *ppe_ds_wlan_priv(ppe_ds_wlan_handle_t *wlan_handle)
{
	return ((void *)&wlan_handle->priv);
}

/**
 * ppe_ds_wlan_rx
 *	PPE-DS WLAN REO2PPE processing API
 *
 * @datatypes
 * ppe_ds_wlan_handle_t
 *
 * @param[in] wlan_handle    PPE-DS WLAN handle
 * @param[in] reo_prod_idx   REO2PPE producer index
 */
void ppe_ds_wlan_rx(ppe_ds_wlan_handle_t *wlan_handle, uint16_t reo_prod_idx);

/**
 * ppe_ds_wlan_vp_alloc
 *	PPE-DS WLAN VP alloc API
 *
 * @datatypes
 * ppe_ds_wlan_handle_t
 * net_device
 * ppe_vp_ai
 *
 * @param[in] wlan_handle   PPE-DS WLAN handle
 * @param[in] dev           WLAN VAP interface
 * @param[in] vpai          PPE-VP alloc parameter
 *
 * @return
 * PPE-VP port number if success, -1 if error
 */
ppe_vp_num_t ppe_ds_wlan_vp_alloc(ppe_ds_wlan_handle_t *wlan_handle, struct net_device *dev, struct ppe_vp_ai *vpai);

/**
 * ppe_ds_wlan_get_node_id
 *	PPE-DS WLAN get node id API
 *
 * @datatypes
 * ppe_ds_wlan_handle_t
 *
 * @param[in] wlan_handle   PPE-DS WLAN handle
 *
 * @return
 * valid node id if success, invalid node id if error
 */
uint32_t ppe_ds_wlan_get_node_id(ppe_ds_wlan_handle_t *wlan_handle);

/**
 * ppe_ds_wlan_vp_free
 *	PPE-DS WLAN VP free API
 *
 * @datatypes
 * ppe_ds_wlan_handle_t
 * ppe_vp_num_t
 *
 * @param[in] wlan_handle   PPE-DS WLAN handle
 * @param[in] vp_num        PPE-VP port number
 *
 * @return
 *  0 on success
 */
ppe_vp_status_t ppe_ds_wlan_vp_free(ppe_ds_wlan_handle_t *wlan_handle, ppe_vp_num_t vp_num);

/**
 * ppe_ds_wlan_inst_register
 *	PPE-DS WLAN instance registration API
 *
 * @datatypes
 * ppe_ds_wlan_handle_t
 * ppe_ds_wlan_reg_info
 *
 * @param[in] wlan_handle   PPE-DS WLAN handle
 * @param[in] ring_info     PPE-DS ring information
 *
 * @return
 * Status of the PPE-DS WLAN instance registration
 */
bool ppe_ds_wlan_inst_register(ppe_ds_wlan_handle_t *wlan_handle, struct ppe_ds_wlan_reg_info *ring_info);

/**
 * ppe_ds_wlan_instance_stop
 *	PPE-DS WLAN instance stop API
 *
 * @datatypes
 * ppe_ds_wlan_handle_t
 * ppe_ds_wlan_ctx_info_handle
 *
 * @param[in] wlan_handle   PPE-DS WLAN handle
 * @param[in] wlan_info_hdl    WLAN ctx information handle
 */
void ppe_ds_wlan_instance_stop(ppe_ds_wlan_handle_t *wlan_handle,
			struct ppe_ds_wlan_ctx_info_handle *wlan_info_hdl);

/**
 * ppe_ds_wlan_inst_stop
 *	PPE-DS WLAN instance stop API
 *
 * @datatypes
 * ppe_ds_wlan_handle_t
 *
 * @param[in] wlan_handle   PPE-DS WLAN handle
 */
void ppe_ds_wlan_inst_stop(ppe_ds_wlan_handle_t *wlan_handle);

/**
 * ppe_ds_wlan_instance_start
 *	PPE-DS WLAN instance start API
 *
 * @datatypes
 * ppe_ds_wlan_handle_t
 * ppe_ds_wlan_ctx_info_handle
 *
 * @param[in] wlan_handle   PPE-DS WLAN handle
 * @param[in] wlan_info_hdl    WLAN ctx information handle
 *
 * @return
 * Status of the PPE-DS WLAN instance start
 */
int ppe_ds_wlan_instance_start(ppe_ds_wlan_handle_t *wlan_handle,
			struct ppe_ds_wlan_ctx_info_handle *wlan_info_hdl);

/**
 * ppe_ds_wlan_inst_start
 *	PPE-DS WLAN instance start API
 *
 * @datatypes
 * ppe_ds_wlan_handle_t
 *
 * @param[in] wlan_handle   PPE-DS WLAN handle
 *
 * @return
 * Status of the PPE-DS WLAN instance start
 */
int ppe_ds_wlan_inst_start(ppe_ds_wlan_handle_t *wlan_handle);

/**
 * ppe_ds_wlan_inst_free
 *	PPE-DS WLAN instance free API
 *
 * @datatypes
 * ppe_ds_wlan_handle_t
 *
 * @param[in] wlan_handle   PPE-DS WLAN handle
 */
void ppe_ds_wlan_inst_free(ppe_ds_wlan_handle_t *wlan_handle);

/**
 * ppe_ds_wlan_inst_alloc
 *	PPE-DS WLAN instance allocation API
 *
 * @datatypes
 * ppe_ds_wlan_handle_t
 * ppe_ds_wlan_ops
 *
 * @param[in] ops         PPE-DS WLAN operation callbacks
 * @param[in] priv_size   Size of PPE-DS WLAN handle's private area
 *
 * @return
 * Status of the PPE-DS WLAN instance allcation
 */
ppe_ds_wlan_handle_t *ppe_ds_wlan_inst_alloc(struct ppe_ds_wlan_ops *ops, size_t priv_size);

/**
 * ppe_ds_ppe2tcl_wlan_handle_intr
 *	PPE-DS WLAN irq handling for ppe2tcl ring
 *
 * @param[in] ctxt IRQ context
 *
 */
int ppe_ds_ppe2tcl_wlan_handle_intr(void *ctxt);

/**
 * ppe_ds_reo2ppe_wlan_handle_intr
 *	PPE-DS WLAN irq handling for reo2ppe ring
 *
 * @param[in] ctxt IRQ context
 *
 */
int ppe_ds_reo2ppe_wlan_handle_intr(void *ctxt);

/**
 * ppe_ds_wlan_get_intr_ctxt
 *	PPE-DS get wlan context
 *
 * @datatypes
 * ppeds_wlan_handle_t
 *
 * @param[in] wlan_handle   PPE-DS WLAN handle
 */
void *ppe_ds_wlan_get_intr_ctxt(ppe_ds_wlan_handle_t *wlan_handle);

/**
 * ppe_ds_wlan_service_status_update
 *	PPE-DS ring service update
 *
 * @datatypes
 * ppeds_wlan_handle_t
 *
 * @param[in] wlan_handle   PPE-DS WLAN handle
 * @param[in] enable        Enable/Disable service
 */
void ppe_ds_wlan_service_status_update(ppe_ds_wlan_handle_t *wlan_handle, bool enable);
#endif	/* _PPE_DS_WLAN_H_ */
