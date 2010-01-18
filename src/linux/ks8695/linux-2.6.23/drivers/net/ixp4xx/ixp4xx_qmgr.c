/*
 * qmgr.c - reimplementation of the queue configuration interface.
 *
 * Copyright (C) 2006 Christian Hohnstaedt <chohnstaedt@innominate.com>
 *
 * This file is released under the GPLv2
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/dmapool.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include <linux/ixp_qmgr.h>
#include <linux/ixp_npe.h>

#define IXQMGR_VERSION "IXP4XX Q Manager 0.2.1"

static struct device *qmgr_dev = NULL;

static int poll_freq = 4000;
static int poll_enable = 0;
static u32 timer_countup_ticks;

module_param(poll_freq, int, 0644);
module_param(poll_enable, int, 0644);

int queue_len(struct qm_queue *queue)
{
	struct qm_qmgr *qmgr = dev_get_drvdata(queue->dev);
	int diff, offs;
	u32 val;

	offs = queue->id/8 + QUE_LOW_STAT0;
	val = *(qmgr->addr + IX_QMGR_QCFG_BASE + queue->id);

	diff = (val - (val >> 7)) & 0x7f;
	if (!diff) {
		/* diff == 0 means either empty or full, must look at STAT0 */
		if ((*(qmgr->addr + offs) >>  ((queue->id % 8)*4)) & 0x04)
			diff = queue->len;
	}
	return diff;
}

static int request_pool(struct device *dev, int count)
{
	int i;
	struct npe_cont *cont;
	struct qm_qmgr *qmgr = dev_get_drvdata(dev);
	dma_addr_t handle;

	for (i=0; i<count; i++) {
		cont = dma_pool_alloc(qmgr->dmapool, GFP_KERNEL, &handle);
		if (!cont) {
			return -ENOMEM;
		}
		cont->phys = handle;
		cont->virt = cont;
		write_lock(&qmgr->lock);
		cont->next = qmgr->pool;
		qmgr->pool = cont;
		write_unlock(&qmgr->lock);
	}
	return 0;
}

static int free_pool(struct device *dev, int count)
{
	int i;
	struct npe_cont *cont;
	struct qm_qmgr *qmgr = dev_get_drvdata(dev);

	for (i=0; i<count; i++) {
		write_lock(&qmgr->lock);
		cont = qmgr->pool;
		if (!cont) {
			write_unlock(&qmgr->lock);
			return -1;
		}
		qmgr->pool = cont->next;
		write_unlock(&qmgr->lock);
		dma_pool_free(qmgr->dmapool, cont, cont->phys);
	}
	return 0;
}

static int get_free_qspace(struct qm_qmgr *qmgr, int len)
{
	int words = (qmgr->res->end - qmgr->res->start + 1) / 4 -
			IX_QMGR_SRAM_SPACE;
	int i,q;

	for (i=0; i<words; i+=len) {
		for (q=0; q<MAX_QUEUES; q++) {
			struct qm_queue *qu = qmgr->queues[q];
			if (!qu)
				continue;
			if ((qu->addr + qu->len > i) && (qu->addr < i + len))
				break;
		}
		if (q == MAX_QUEUES) {
			/* we have a free address */
			return i;
		}
	}
	return -1;
}

static inline int _log2(int x)
{
	int r=0;
	while(x>>=1)
		r++;
	return r;
}

/*
 * 32bit Config registers at IX_QMGR_QUECONFIG_BASE_OFFSET[Qid]
 *    0 - 6 WRPTR Word offset to baseaddr (index 0 .. BSIZE-1)
 *    7 -13 RDPTR        ''
 *   14 -21 BADDR baseaddr = (offset to IX_QMGR_QUEBUFFER_SPACE_OFFSET) >> 6
 *   22 -23 ESIZE entrySizeInWords (always 00 because entrySizeInWords==1)
 *   24 -25 BSIZE qSizeInWords 00=16,01=32,10=64,11=128
 *   26 -28 NE nearly empty
 *   29 -31 NF nearly full
 */
static int conf_q_regs(struct qm_queue *queue)
{
	int bsize = _log2(queue->len/16);
	int baddr = queue->addr + IX_QMGR_QCFG_SIZE;

	/* +2, because baddr is in words and not in bytes */
	queue_write_cfg_reg(queue,  (bsize << 24) | (baddr<<(14-6+2)) );

	return 0;
}

static void pmu_timer_restart(void)
{
    unsigned long flags;

    local_irq_save(flags);

     __asm__(" mcr p14,0,%0,c1,c1,0\n"  /* write current counter */
            : : "r" (timer_countup_ticks));

    __asm__(" mrc p14,0,r1,c4,c1,0; "  /* get int enable register */
            " orr r1,r1,#1; "
            " mcr p14,0,r1,c5,c1,0; "  /* clear overflow */
            " mcr p14,0,r1,c4,c1,0\n"  /* enable interrupts */
            : : : "r1");

    local_irq_restore(flags);
}

static void pmu_timer_init(void)
{
	u32 controlRegisterMask =
		BIT(0) | /* enable counters */
		BIT(2);  /* reset clock counter; */

	/*
	 *   Compute the number of xscale cycles needed between each
	 *   PMU IRQ. This is done from the result of an OS calibration loop.
	 *
	 *   For 533MHz CPU, 533000000 tick/s / 4000 times/sec = 138250
	 *   4000 times/sec = 37 mbufs/interrupt at line rate
	 *   The pmu timer is reset to -138250 = 0xfffde3f6, to trigger an IRQ
	 *   when this up counter overflows.
	 *
	 *   The multiplication gives a number of instructions per second.
	 *   which is close to the processor frequency, and then close to the
	 *   PMU clock rate.
	 *
	 *   2 is the number of instructions per loop
	 *
	 */

	timer_countup_ticks = - ((loops_per_jiffy * HZ * 2) / poll_freq);

	/* enable the CCNT (clock count) timer from the PMU */
	__asm__(" mcr p14,0,%0,c0,c1,0\n"
	        : : "r" (controlRegisterMask));
}

static void pmu_timer_disable(void)
{
	unsigned long flags;

	local_irq_save(flags);

	__asm__(" mrc p14,0,r1,c4,c1,0; "  /* get int enable register */
	        " and r1,r1,#0x1e; "
	        " mcr p14,0,r1,c4,c1,0\n"  /* disable interrupts */
	        : : : "r1");
	local_irq_restore(flags);
}

void queue_set_watermarks(struct qm_queue *queue, unsigned ne, unsigned nf)
{
	u32 val;
	/* calculate the register values
	 * 0->0, 1->1, 2->2, 4->3, 8->4 16->5...*/
	ne = _log2(ne<<1) & 0x7;
	nf = _log2(nf<<1) & 0x7;

	/* Mask out old watermarks */
	val = queue_read_cfg_reg(queue) & ~0xfc000000;
	queue_write_cfg_reg(queue, val | (ne << 26) | (nf << 29));
}

int queue_set_irq_src(struct qm_queue *queue, int flag)
{
	struct qm_qmgr *qmgr = dev_get_drvdata(queue->dev);
	u32 reg;
	int offs, bitoffs;

	/* Q 0-7 are in REG0, 8-15 are in REG1, etc. They occupy 4 bits/Q */
	offs = queue->id/8 + INT0_SRC_SELREG0;
	bitoffs = (queue->id % 8)*4;

	reg = *(qmgr->addr + offs) & ~(0xf << bitoffs);
	*(qmgr->addr + offs) = reg | (flag << bitoffs);

	return 0;
}

static irqreturn_t irq_qm1(int irq, void *dev_id)
{
	struct qm_qmgr *qmgr = dev_id;
	int offs, reg;
	struct qm_queue *queue;

	if (poll_enable)
		pmu_timer_restart();

	reg = *(qmgr->addr + QUE_INT_REG0);
	while(reg) {
		/*
		 * count leading zeros. "offs" gets
		 * the amount of leading 0 in "reg"
		 */
		asm ("clz %0, %1;" : "=r"(offs) : "r"(reg));
		offs = 31 - offs;
		reg &= ~(1 << offs);
		queue = qmgr->queues[offs];
		if (likely(queue)) {
			if (likely(queue->irq_cb)) {
				queue->irq_cb(queue);
			} else {
				printk(KERN_ERR "Missing callback for Q %d\n",
						offs);
			}
		} else {
			printk(KERN_ERR "IRQ for unregistered Q %d\n", offs);
		}
	}
	return IRQ_HANDLED;
}

struct qm_queue *request_queue(int qid, int len)
{
	int ram;
	struct qm_qmgr *qmgr;
	struct qm_queue *queue;

	if (!qmgr_dev)
		return ERR_PTR(-ENODEV);

	if ((qid < 0) || (qid > MAX_QUEUES))
		return ERR_PTR(-ERANGE);

	switch (len) {
		case 16:
		case 32:
		case 64:
		case 128: break;
		default : return ERR_PTR(-EINVAL);
	}

	qmgr = dev_get_drvdata(qmgr_dev);

	if (qmgr->queues[qid]) {
		/* not an error, just in use already */
		return NULL;
	}
	if ((ram = get_free_qspace(qmgr, len)) < 0) {
		printk(KERN_ERR "No free SRAM space for this queue\n");
		return ERR_PTR(-ENOMEM);
	}
	if (!(queue = kzalloc(sizeof(struct qm_queue), GFP_KERNEL)))
		return ERR_PTR(-ENOMEM);

	if (!try_module_get(THIS_MODULE)) {
		kfree(queue);
		return ERR_PTR(-ENODEV);
	}

	queue->addr = ram;
	queue->len = len;
	queue->id = qid;
	queue->dev = get_device(qmgr_dev);
	queue->acc_reg = qmgr->addr + (4 * qid);
	qmgr->queues[qid] = queue;
	if (request_pool(qmgr_dev, len)) {
		printk(KERN_ERR "Failed to request DMA pool of Q %d\n", qid);
	}

	conf_q_regs(queue);
	return queue;
}

void release_queue(struct qm_queue *queue)
{
	struct qm_qmgr *qmgr = dev_get_drvdata(queue->dev);

	BUG_ON(qmgr->queues[queue->id] != queue);
	qmgr->queues[queue->id] = NULL;

	if (free_pool(queue->dev, queue->len)) {
		printk(KERN_ERR "Failed to release DMA pool of Q %d\n",
				queue->id);
	}
	queue_disable_irq(queue);
	queue_write_cfg_reg(queue, 0);

	module_put(THIS_MODULE);
	put_device(queue->dev);
	kfree(queue);
}




static int qmgr_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct qm_qmgr *qmgr;
	int size, ret=0, i;

	if (!(res = platform_get_resource(pdev, IORESOURCE_MEM, 0)))
		return -EIO;

	if ((i = platform_get_irq(pdev, 0)) < 0)
		return -EIO;

	if (!(qmgr = kzalloc(sizeof(struct qm_qmgr), GFP_KERNEL)))
		return -ENOMEM;

	qmgr->irq = i;
	size = res->end - res->start +1;
	qmgr->res = request_mem_region(res->start, size, "ixp_qmgr");
	if (!qmgr->res) {
		ret = -EBUSY;
		goto out_free;
	}

	qmgr->addr = ioremap(res->start, size);
	if (!qmgr->addr) {
		ret = -ENOMEM;
		goto out_rel;
	}

	/* Reset Q registers */
	for (i=0; i<4; i++)
		*(qmgr->addr + QUE_LOW_STAT0 +i) = 0x33333333;
	for (i=0; i<10; i++)
		*(qmgr->addr + QUE_UO_STAT0 +i) = 0x0;
	for (i=0; i<4; i++)
		*(qmgr->addr + INT0_SRC_SELREG0 +i) = 0x0;
	for (i=0; i<2; i++) {
		*(qmgr->addr + QUE_IE_REG0 +i) = 0x00;
		*(qmgr->addr + QUE_INT_REG0 +i) = 0xffffffff;
	}
	for (i=0; i<64; i++) {
		*(qmgr->addr + IX_QMGR_QCFG_BASE + i) = 0x0;
	}

	if (poll_enable) {
		pmu_timer_init();
		qmgr->irq = IRQ_IXP4XX_XSCALE_PMU;
	}
	ret = request_irq(qmgr->irq, irq_qm1, SA_SHIRQ | SA_INTERRUPT,
			"qmgr", qmgr);
	if (ret) {
		printk(KERN_ERR "Failed to request IRQ(%d)\n", qmgr->irq);
		ret = -EIO;
		goto out_rel;
	}
	if (poll_enable)
		pmu_timer_restart();

	rwlock_init(&qmgr->lock);
	qmgr->dmapool = dma_pool_create("qmgr", &pdev->dev,
			sizeof(struct npe_cont), 32, 0);
	platform_set_drvdata(pdev, qmgr);

	qmgr_dev = &pdev->dev;

	printk(KERN_INFO IXQMGR_VERSION " initialized.\n");

	return 0;

out_rel:
	release_resource(qmgr->res);
out_free:
	kfree(qmgr);
	return ret;
}

static int qmgr_remove(struct platform_device *pdev)
{
	struct qm_qmgr *qmgr = platform_get_drvdata(pdev);
	int i;

	for (i=0; i<MAX_QUEUES; i++) {
		if (qmgr->queues[i]) {
			printk(KERN_ERR "WARNING Unreleased Q: %d\n", i);
			release_queue(qmgr->queues[i]);
		}
	}

	if (poll_enable)
		pmu_timer_disable();

	synchronize_irq (qmgr->irq);
	free_irq(qmgr->irq, qmgr);

	dma_pool_destroy(qmgr->dmapool);
	iounmap(qmgr->addr);
	release_resource(qmgr->res);
	platform_set_drvdata(pdev, NULL);
	qmgr_dev = NULL;
	kfree(qmgr);
	return 0;
}

static struct platform_driver ixp4xx_qmgr = {
	.driver.name    = "ixp4xx_qmgr",
	.probe          = qmgr_probe,
	.remove         = qmgr_remove,
};


static int __init init_qmgr(void)
{
	return platform_driver_register(&ixp4xx_qmgr);
}

static void __exit finish_qmgr(void)
{
	platform_driver_unregister(&ixp4xx_qmgr);
}

module_init(init_qmgr);
module_exit(finish_qmgr);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Christian Hohnstaedt <chohnstaedt@innominate.com>");

EXPORT_SYMBOL(request_queue);
EXPORT_SYMBOL(release_queue);
EXPORT_SYMBOL(queue_set_irq_src);
EXPORT_SYMBOL(queue_set_watermarks);
EXPORT_SYMBOL(queue_len);
