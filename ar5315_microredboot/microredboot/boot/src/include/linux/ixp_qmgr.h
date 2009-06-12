/*
 * Copyright (C) 2006 Christian Hohnstaedt <chohnstaedt@innominate.com>
 *
 * This file is released under the GPLv2
 */

#ifndef IX_QMGR_H
#define IX_QMGR_H

#include <linux/skbuff.h>
#include <linux/list.h>
#include <linux/if_ether.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/ixp_npe.h>
#include <asm/atomic.h>

/* All offsets are in 32bit words */
#define QUE_LOW_STAT0    0x100 /* 4x Status of the 32 lower queues 0-31 */
#define QUE_UO_STAT0     0x104 /* 2x Underflow/Overflow status bits*/
#define QUE_UPP_STAT0    0x106 /* 2x Status of thew 32 upper queues 32-63 */
#define INT0_SRC_SELREG0 0x108 /* 4x */
#define QUE_IE_REG0      0x10c /* 2x */
#define QUE_INT_REG0     0x10e /* 2x IRQ reg, write 1 to reset IRQ */

#define IX_QMGR_QCFG_BASE	0x800
#define IX_QMGR_QCFG_SIZE	0x40
#define IX_QMGR_SRAM_SPACE	(IX_QMGR_QCFG_BASE + IX_QMGR_QCFG_SIZE)

#define MAX_QUEUES 32 /* first, we only support the lower 32 queues */
#define MAX_NPES    3

enum {
	Q_IRQ_ID_E = 0,  /* Queue Empty due to last read  */
	Q_IRQ_ID_NE,     /* Queue Nearly Empty due to last read */
	Q_IRQ_ID_NF,     /* Queue Nearly Full due to last write */
	Q_IRQ_ID_F,      /* Queue Full due to last write  */
	Q_IRQ_ID_NOT_E,  /* Queue Not Empty due to last write */
	Q_IRQ_ID_NOT_NE, /* Queue Not Nearly Empty due to last write */
	Q_IRQ_ID_NOT_NF, /* Queue Not Nearly Full due to last read */
	Q_IRQ_ID_NOT_F   /* Queue Not Full due to last read */
};

extern struct qm_queue *request_queue(int qid, int len);
extern void release_queue(struct qm_queue *queue);
extern int queue_set_irq_src(struct qm_queue *queue, int flag);
extern void queue_set_watermarks(struct qm_queue *, unsigned ne, unsigned nf);
extern int queue_len(struct qm_queue *queue);

struct qm_qmgr;
struct qm_queue;

typedef void(*queue_cb)(struct qm_queue *);

struct qm_queue {
	int addr;	/* word offset from IX_QMGR_SRAM_SPACE */
	int len;	/* size in words */
	int id;		/* Q Id */
	u32 __iomem *acc_reg;
	struct device *dev;
	atomic_t use;
	queue_cb irq_cb;
	void *cb_data;
};

#ifndef CONFIG_NPE_ADDRESS_COHERENT
struct eth_ctl {
	u32 next;
	u16 buf_len;
	u16 pkt_len;
	u32 phys_addr;
	u8 dest_id;
	u8 src_id;
	u16 flags;
	u8 qos;
	u8 padlen;
	u16 vlan_tci;
	u8 dest_mac[ETH_ALEN];
	u8 src_mac[ETH_ALEN];
};

#else
struct eth_ctl {
	u32 next;
	u16 pkt_len;
	u16 buf_len;
	u32 phys_addr;
	u16 flags;
	u8 src_id;
	u8 dest_id;
	u16 vlan_tci;
	u8 padlen;
	u8 qos;
	u8 dest_mac[ETH_ALEN];
	u8 src_mac[ETH_ALEN];
};
#endif

struct npe_cont {
	struct eth_ctl eth;
	void *data;
	struct npe_cont *next;
	struct npe_cont *virt;
	dma_addr_t phys;
};

struct qm_qmgr {
	u32 __iomem *addr;
	struct resource *res;
	struct qm_queue *queues[MAX_QUEUES];
	rwlock_t lock;
	struct npe_cont *pool;
	struct dma_pool *dmapool;
	int irq;
};

static inline void queue_write_cfg_reg(struct qm_queue *queue, u32 val)
{
	struct qm_qmgr *qmgr = dev_get_drvdata(queue->dev);
	*(qmgr->addr + IX_QMGR_QCFG_BASE + queue->id) = val;
}
static inline u32 queue_read_cfg_reg(struct qm_queue *queue)
{
	struct qm_qmgr *qmgr = dev_get_drvdata(queue->dev);
	return *(qmgr->addr + IX_QMGR_QCFG_BASE + queue->id);
}

static inline void queue_ack_irq(struct qm_queue *queue)
{
	struct qm_qmgr *qmgr = dev_get_drvdata(queue->dev);
	*(qmgr->addr + QUE_INT_REG0) = 1 << queue->id;
}

static inline void queue_enable_irq(struct qm_queue *queue)
{
	struct qm_qmgr *qmgr = dev_get_drvdata(queue->dev);
	*(qmgr->addr + QUE_IE_REG0) |= 1 << queue->id;
}

static inline void queue_disable_irq(struct qm_queue *queue)
{
	struct qm_qmgr *qmgr = dev_get_drvdata(queue->dev);
	*(qmgr->addr + QUE_IE_REG0) &= ~(1 << queue->id);
}

static inline void queue_put_entry(struct qm_queue *queue, u32 entry)
{
	*(queue->acc_reg) = npe_to_cpu32(entry);
}

static inline u32 queue_get_entry(struct qm_queue *queue)
{
	return cpu_to_npe32(*queue->acc_reg);
}

static inline struct npe_cont *qmgr_get_cont(struct qm_qmgr *qmgr)
{
	unsigned long flags;
	struct npe_cont *cont;

	if (!qmgr->pool)
		return NULL;
	write_lock_irqsave(&qmgr->lock, flags);
	cont = qmgr->pool;
	qmgr->pool = cont->next;
	write_unlock_irqrestore(&qmgr->lock, flags);
	return cont;
}

static inline void qmgr_return_cont(struct qm_qmgr *qmgr,struct npe_cont *cont)
{
	unsigned long flags;

	write_lock_irqsave(&qmgr->lock, flags);
	cont->next = qmgr->pool;
	qmgr->pool = cont;
	write_unlock_irqrestore(&qmgr->lock, flags);
}

static inline int queue_stat(struct qm_queue *queue)
{
	struct qm_qmgr *qmgr = dev_get_drvdata(queue->dev);
	u32 reg = *(qmgr->addr + QUE_UO_STAT0 + (queue->id >> 4));
	return (reg >> (queue->id & 0xf) << 1) & 3;
}

/* Prints the queue state, which is very, very helpfull for debugging */
static inline void queue_state(struct qm_queue *queue)
{
	u32 val=0, lstat=0;
	int offs;
	struct qm_qmgr *qmgr = dev_get_drvdata(queue->dev);

	offs = queue->id/8 + QUE_LOW_STAT0;
	val = *(qmgr->addr + IX_QMGR_QCFG_BASE + queue->id);
	lstat = (*(qmgr->addr + offs) >>  ((queue->id % 8)*4)) & 0x0f;

	printk("Qid[%02d]: Wptr=%4x, Rptr=%4x, diff=%4x, Stat:%x\n", queue->id,
		val&0x7f, (val>>7) &0x7f, (val - (val >> 7)) & 0x7f, lstat);
}

#endif
