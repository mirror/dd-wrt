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

#ifndef __NSS_DP_PPEDS_H__
#define __NSS_DP_PPEDS_H__

/**
 * nss_dp_ppeds_rx_fill_elem
 *	PPE-DS Rx fill buffer info
 */
struct nss_dp_ppeds_rx_fill_elem {
	uint32_t opaque_lo;		/**< Low 32-bit opaque content */
	uint32_t opaque_hi;		/**< High 32-bit opaque content */
	dma_addr_t buff_addr;		/**< Buffer's physical address */
};

/**
 * nss_dp_ppeds_tx_cmpl_elem
 *	PPE-DS Tx complete buffer info
 */
struct nss_dp_ppeds_tx_cmpl_elem {
	unsigned long cookie;		/**< Cookie information to be filled in the opaque field */
};

/**
 * nss_dp_ppeds_handle
 *	PPE-DS DP handle info
 */
typedef struct nss_dp_ppeds_handle {
	bool polling_for_idx_update;	/**< DS poll mode used */
	dma_addr_t ppe2tcl_ba;		/**< PPE2TCL ring's base address */
	dma_addr_t reo2ppe_ba;		/**< REO2PPE ring's base address */
	uint32_t ppe2tcl_num_desc;	/**< PPE2TCL descriptor count */
	uint32_t reo2ppe_num_desc;	/**< REO2PPE descriptor count */
	uint32_t ppe2tcl_rxfill_num_desc;	/**< PPE2TCL Rxfill descriptor count */
	uint32_t reo2ppe_txcmpl_num_desc;	/**< REO2PPE Txcomplete descriptor count */
	uint32_t eth_rxfill_low_thr;	/**< RxFill ring's low threshold interrupt value */
	uint32_t eth_txcomp_budget;	/**< Tx complete's budget */
	struct nss_dp_ppeds_rx_fill_elem *rx_fill_arr;	/**< RxFill buffer array */
	struct nss_dp_ppeds_tx_cmpl_elem *tx_cmpl_arr;	/**< TxComplete buffer array */
	char priv[] __aligned(NETDEV_ALIGN);	/**< Private area */
} nss_dp_ppeds_handle_t;

/**
 * nss_ppe_ds_ctx_info_handle
 *	PPE-DS umac reset handle
 */
struct nss_ppe_ds_ctx_info_handle {
	uint32_t umac_reset_inprogress;		/**< umac reset in progress information */
};

/*
 * nss_dp_ppeds_cb
 *	PPE-DS callbacks
 */
struct nss_dp_ppeds_cb {
	void (*rx)(nss_dp_ppeds_handle_t *, uint16_t hw_prod_idx);
				/**< PPE-DS EDMA Rx callback */
	uint32_t (*rx_fill)(nss_dp_ppeds_handle_t *, uint32_t num_buff_req, uint32_t buff_size, uint32_t headroom);
				/**< PPE-DS EDMA Rx fill callback */
	void (*rx_release)(nss_dp_ppeds_handle_t *, uint64_t rx_opaque);
				/**< PPE-DS EDMA Tx descriptor free callback */
	void (*tx_cmpl)(nss_dp_ppeds_handle_t *, uint16_t cons_idx);
				/**< PPE-DS EDMA Tx complete callback */
	void (*enable_wlan_intr)(nss_dp_ppeds_handle_t *edma_handle,
				bool enable);
				/**< PPE-DS toggle wlan interrupt */
	void (*notify_napi_done)(nss_dp_ppeds_handle_t *edma_handle);
				/**< PPE-DS notify NAPI completed */
};

/*
 * nss_dp_ppeds_ops
 *	PPE-DS operations
 */
struct nss_dp_ppeds_ops {
	nss_dp_ppeds_handle_t *(*alloc)(const struct nss_dp_ppeds_cb *ops, size_t priv_size);
				/**< PPE-DS instance allocation operation */
	bool (*reg)(nss_dp_ppeds_handle_t *ppeds_handle);
				/**< PPE-DS instance registration operation */
	int (*start)(nss_dp_ppeds_handle_t *ppeds_handle,  uint8_t intr_enable,
			struct nss_ppe_ds_ctx_info_handle *info_hdl);
				/**< PPE-DS instance start operation */
	void (*refill)(nss_dp_ppeds_handle_t *ppeds_handle, int count);
				/**< PPE-DS instance refill operation */
	void (*stop)(nss_dp_ppeds_handle_t *ppeds_handle, uint8_t intr_enable,
			struct nss_ppe_ds_ctx_info_handle *info_hdl);
				/**< PPE-DS instance stop operation */
	void (*free)(nss_dp_ppeds_handle_t *ppeds_handle);
				/**< PPE-DS instance free operation */
	bool (*get_queues)(nss_dp_ppeds_handle_t *ppeds_handle, uint32_t *ppe_queue_start);
				/**< PPE-DS instance get queue operation */
	void (*set_rx_cons_idx)(nss_dp_ppeds_handle_t *ppeds_handle, uint16_t rx_cons_idx);
				/**< PPE-DS Rx consumer index set operation */
	void (*set_tx_prod_idx)(nss_dp_ppeds_handle_t *ppeds_handle, uint16_t txw_prod_idx);
				/**< PPE-DS Tx producer index set operation */
	uint16_t (*get_tx_cons_idx)(nss_dp_ppeds_handle_t *ppeds_handle);
				/**< PPE-DS Tx consumer index get operation */
	uint16_t (*get_rx_prod_idx)(nss_dp_ppeds_handle_t *ppeds_handle);
				/**< PPE-DS Rx producer index get operation */
	uint16_t (*get_rxfill_cons_idx)(nss_dp_ppeds_handle_t *ppeds_handle);
				/**< PPE-DS Get rxfill ring consumer index */
	void (*set_rxfill_prod_idx)(nss_dp_ppeds_handle_t *ppeds_handle, uint16_t prod_idx);
				/**< PPE-DS Set rxfill ring producer index */
	void (*enable_rx_reap_intr)(nss_dp_ppeds_handle_t *ppeds_handle);
				/**< PPE-DS enable edma interrupt */
	void (*service_status_update)(nss_dp_ppeds_handle_t *ppeds_handle, bool enable);
				/**< PPE-DS check and update ring usage service */
};

/**
 * nss_dp_ppeds_get_rx_fill_arr
 *	Wrapper to get the RxFill array from PPE-DS EDMA handle
 *
 * @datatypes
 * nss_dp_ppeds_rx_fill_elem
 * nss_dp_ppeds_handle_t
 *
 * @param[in] ppeds_handle Pointer to the EDMA PPE-DS handle structure
 *
 * @return
 * RxFill array pointer
 */
static inline struct nss_dp_ppeds_rx_fill_elem *nss_dp_ppeds_get_rx_fill_arr(nss_dp_ppeds_handle_t *ppeds_handle)
{
	return ppeds_handle->rx_fill_arr;
}

/**
 * nss_dp_ppeds_get_tx_cmpl_arr
 *	Wrapper to get the TxComplete array from PPE-DS EDMA handle
 *
 * @datatypes
 * nss_dp_ppeds_tx_cmpl_elem
 * nss_dp_ppeds_handle_t
 *
 * @param[in] ppeds_handle Pointer to the EDMA PPE-DS handle structure
 *
 * @return
 * TxComplete array pointer
 */
static inline struct nss_dp_ppeds_tx_cmpl_elem *nss_dp_ppeds_get_tx_cmpl_arr(nss_dp_ppeds_handle_t *ppeds_handle)
{
	return ppeds_handle->tx_cmpl_arr;
}

/**
 * nss_dp_ppeds_priv
 *	Wrapper to get the private area's pointer from the PPE-DS EDMA handle
 *
 * @datatypes
 * nss_dp_ppeds_handle_t
 *
 * @param[in] ppeds_handle Pointer to the EDMA PPE-DS handle structure
 *
 * @return
 * Pointer of the PPE-DS EMDA handle's private area
 */
static inline void *nss_dp_ppeds_priv(nss_dp_ppeds_handle_t *ppeds_handle)
{
	return ((void *)&ppeds_handle->priv);
}

/**
 * nss_dp_ppeds_get_ops
 *	API to get the PPE-DS operations
 *
 * @datatypes
 * nss_dp_ppeds_ops
 *
 * @return
 * PPE-DS operation structure, else NULL
 */
struct nss_dp_ppeds_ops *nss_dp_ppeds_get_ops(void);

#endif	/** __NSS_DP_PPEDS_H__*/
