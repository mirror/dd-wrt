/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

#ifndef __EIP_DMA_H
#define __EIP_DMA_H

extern uint32_t ring_sz;

#define EIP_IDX_ADD(i, val, max) (((i) + (val)) & ((max) - 1))
#define EIP_IDX_SUB(i, val, max) (((i) - (val) + (max)) & ((max) - 1))

#define EIP_DMA_DESC_MAX ring_sz
#define EIP_RING_SZ_MAX 8192
#define EIP_RING_SZ_MIN 64

#define EIP_DMA_AVAIL_COUNT(head, tail) EIP_IDX_SUB(head, tail, EIP_DMA_DESC_MAX)
#define EIP_DMA_IDX_ADD(i, val) EIP_IDX_ADD((i), (val), EIP_DMA_DESC_MAX)
#define EIP_DMA_IDX_INC(i) EIP_DMA_IDX_ADD((i), 1)


struct eip_sw_desc;

/*
 * eip_dma_type
 *	Ring configuration type.
 */
enum eip_dma_type {
	EIP_DMA_TYPE_LA = 1,	/* Ring is configured in Lookaside mode */
	EIP_DMA_TYPE_HYBRID,	/* Ring is configured in hybrid mode */
	EIP_DMA_TYPE_MAX
};

/*
 * eip_hw_desc
 *	HW descriptors.
 */
struct eip_hw_desc {
	uint32_t frag[4];
	uint32_t token[8];
	uint32_t bypass[4];
} __attribute__((aligned(L1_CACHE_BYTES)));

/*
 * eip_hw_ring
 *	EIP dma ring
 */
struct eip_hw_ring {
	struct eip_hw_desc *desc;	/* Ring descriptor FIFO */
	uintptr_t *meta;		/* shadow meta ring for sw pointer */
	atomic_t prod_idx;		/* Written by TX  */
	atomic_t cons_idx;		/* Written by RX & Read by TX */

	void __iomem *prep;		/* HW prepared pointer register */
	void __iomem *proc;		/* HW processed pointer register */
	void __iomem *prep_cnt;		/* HW prepared count register */
	void __iomem *proc_cnt;		/* HW processed count register */
	void __iomem *proc_thresh;	/* HW processed threshold register */
	void __iomem *proc_status;	/* HW status register */

	unsigned int irq;		/* IRQ number for DMA ring */
	struct net_device ndev;		/* Dummy_netdev for NAPI */
	struct napi_struct napi;	/* NAPI handler */

	void *kaddr;			/* Pointer to allocated memory */
};

/*
 * eip_dma_stats
 *	Statistics per DMA object.
 */
struct eip_dma_stats {
	uint64_t tx_pkts;		/* Buffer/packet transmitted on this DMA */
	uint64_t tx_frags;		/* fragments transmitted on this DMA */
	uint64_t tx_bytes;		/* bytes transmitted on this DMA */
	uint64_t tx_error;		/* Transmission error. */
	uint64_t rx_pkts;		/* buffer/packet recieved on this DMA */
	uint64_t rx_frags;		/* fragments recieved on this DMA */
	uint64_t rx_bytes;		/* bytes recieved on this DMA */
	uint64_t rx_error;		/* buffer/packet recieved with error on this DMA */
	uint64_t rx_dropped;		/* buffer/packet dropped in driver due to unknown TR */

	uint32_t rx_err_code[256];	/* Detailed error stats for received packet */
};

/*
 * eip_dma
 *	DMA ring structure.
 */
struct eip_dma {
	struct eip_hw_ring in;		/* Command ring (CDR) */
	struct eip_hw_ring out;		/* Result ring (RDR) */
	enum eip_dma_type type;		/* Ring type */
	bool active;			/* DMA is configured */

	uint32_t ring_id;		/* CDR/RDR ring - ID */

	struct eip_dma_stats stats;	/* Stats */
	struct dentry *dentry;		/* debugfs dentry */
};

/*
 * eip_dma_ring_info
 *	Ring Info for initialization.
 */
struct eip_dma_ring_info {
	void __iomem *base_addr;
	irq_handler_t handler;
	int irq;
	int cpu;
};

/*
 * eip_dma_avail_idx_and_count()
 *	Return available descriptor count & producer index in a ring.
 */
static inline uint32_t eip_dma_avail_idx_and_count(struct eip_hw_ring *ring, uint32_t *prod_idx)
{
	uint32_t cons_idx;
	(*prod_idx) = atomic_read(&ring->prod_idx);
	cons_idx = atomic_read(&ring->cons_idx);
	return EIP_DMA_AVAIL_COUNT(cons_idx - 1, *prod_idx);
}

typedef void (*eip_dma_callback_t)(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw);
typedef void (*eip_dma_err_callback_t)(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw, uint16_t cle_err, uint16_t tr_err);

extern const char *irq_name[];
extern const char *dma_name[];
extern struct file_operations dma_stats_ops;

int eip_dma_tx_sg(struct eip_dma *dma, struct eip_sw_desc *sw, struct scatterlist *src, struct scatterlist *dst);
int eip_dma_hy_tx_linear_skb(struct eip_dma *dma, struct eip_sw_desc *sw, struct sk_buff *skb);
int eip_dma_hy_tx_nonlinear_skb(struct eip_dma *dma, struct eip_sw_desc *sw, struct sk_buff *skb);

int eip_dma_cmd_init(struct eip_dma *dma, struct eip_dma_ring_info *info, bool en_irq);
int eip_dma_res_init(struct eip_dma *dma, struct eip_dma_ring_info *info, bool en_irq);

int eip_dma_la_init(struct eip_dma *dma, struct platform_device *pdev, struct device_node *child, int cpu);
void eip_dma_la_deinit(struct eip_dma *dma);

int eip_dma_hy_init(struct eip_dma *dma, struct platform_device *pdev, struct device_node *child, int cpu);
void eip_dma_hy_refill_all(struct eip_ctx *ctx);
void eip_dma_hy_deinit(struct eip_dma *dma);

#endif /* __EIP_DMA_H */
