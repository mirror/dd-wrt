/*
 *  rdma.c - CNS3XXX RAID-DMA h/w acceleration
 *
 *  Revision History: arch/arm/mach-cns3xxx/ChangeLog.cns_raid.txt
 */
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/pagemap.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <mach/irqs.h>
#include <linux/mempool.h>
#include <linux/dma-mapping.h>

#include "rdma.h"
#include <mach/pm.h>

int rdma_verbose;
u8 rdma_test_ptn[32] = {0};
unsigned int dma_timeout_jiffies;
mempool_t	*rdma_sg_pool = NULL;	/* pool */
rdma_chan_t *dma = NULL;			/* dma channel */

static DEFINE_SPINLOCK(process_lock);

/* Debug Printk */
#define dprintk(x...)	((void)(rdma_verbose && printk(KERN_WARNING x)))
#define	dump_regs(x)	\
do {	\
	dprintk("pa:%08x sg:%08x bp:%08x fp:%08x st:%08x qp:%08x sz:%08x\n", \
		*((x)->cregs->para),	\
		*((x)->cregs->sgad),	\
		*((x)->cregs->back),	\
		*((x)->cregs->frnt),	\
		*((x)->cregs->stat),	\
		*((x)->cregs->qpar),	\
		*((x)->cregs->blsz));	\
} while (0)


#define	rdma_dmac_flush_range(start, bytes)	\
	do { \
		dma_cache_maint(start, bytes, DMA_BIDIRECTIONAL);	\
	} while (0);

#define	rdma_dmac_inv_range(start, bytes)	\
	do { \
		dma_cache_maint(start, bytes, DMA_FROM_DEVICE);	\
	} while (0);
	
#define	rdma_dmac_clean_range(start, bytes)	\
	do { \
		dma_cache_maint(start, bytes, DMA_TO_DEVICE);	\
	} while (0);



extern void *acs_mempool_alloc(mempool_t *pool);

/**
 * rdma_timeout_handle
 */
static void rdma_timeout_handle(rdma_chan_t *rdma)
{
	printk("%s: timeout handling\n", __FUNCTION__);
	spin_lock_irq(&process_lock);	
	
	if (!list_empty(&rdma->process_q)) {
		sg_t *sg_fin = list_entry(rdma->process_q.next, sg_t, lru);
		list_del_init(&sg_fin->lru);
		sg_fin->status = SG_STATUS_DONE;
	}
	
	*(dma->cregs->para) = 0;
	*(dma->cregs->back) = rdma->q_first_phys;
	*(dma->cregs->frnt) = rdma->q_first_phys;
	flush_cache_all();
	spin_unlock_irq(&process_lock);	
}

/**
 * rdma_mempool_alloc - return a sg from pool
 * @gfp_mask: gfp flag
 *
 * Return:
 *  sg table
 */
static void *rdma_sg_mempool_alloc(unsigned int gfp_mask)
{
	void *element;
	int exception_timeout = 30;

repeat:
	element = acs_mempool_alloc(rdma_sg_pool);
	if (likely(element))
		return element;
	
	if (!(gfp_mask & __GFP_WAIT)) {
		return NULL;
	} else {
		msleep(1000);
		exception_timeout--;
		WARN_ON(exception_timeout < 0);	/* Thresh check, we should check or increase if any warning */
		goto repeat;
	}
}

#define	rdma_mempool_create(pool, name, size, min_nr, alloc_fn, free_fn, privp)	\
do {	\
	printk("%s: pre-allocating %s: %d*%d=%d\n",	\
				__FUNCTION__, (name), (min_nr), (size), (min_nr) * (size));	\
	pool = mempool_create((min_nr), (mempool_alloc_t *)(alloc_fn), free_fn, (privp));	\
	if (!pool)	\
		goto abort;	\
} while(0);

#define rdma_mempool_destroy(pool)	\
do {	\
	if (pool)	\
		mempool_destroy(pool);	\
} while(0);

#define	rdma_kfree_obj(obj)	\
do {	\
	if (obj)	\
		kfree(obj);	\
} while(0);

/**
 * rdma_sg_prealloc_fn - sg mempool pre-allocation callback
 * @gfp_flags: GFP_ flags
 * @data: private data, reserved
 *
 * Return:
 *   pre-alloc sg table
 */
static void *rdma_sg_prealloc_fn(int gfp_flags, void *data)
{
	sg_t *sg = NULL;
	sg = kzalloc(sizeof(sg_t), gfp_flags);
	INIT_LIST_HEAD(&sg->lru);
	init_waitqueue_head(&sg->wait);
	sg->status = SG_STATUS_FREE;
	
	/* Remove Debug Message */
#if 0
	printk("%s: pre-allocating sg=0x%p, phy=0x%p\n",
				__FUNCTION__, (void *)sg, (void *)virt_to_phys(sg));
#endif

	WARN_ON(!sg);
	return (void *)sg;
}

/**
 * rdma_sg_deconstruct_fn - sg mempool de-allocation callback
 * @sg: sg elements
 * @data: private data, reserved
 */
static void rdma_sg_deconstruct_fn(void *sg, void *data)
{
	if (sg) {
		printk("%s: de-allocating sg=0x%p, phy=0x%p\n",
				__FUNCTION__, (void *)sg, (void *)virt_to_phys(sg));
		kfree(sg);
	}
	return;
}



/*-------------------------------------------------------- */
/**
 * rdma_get_sg - alloc an SG
 * @dma: dma chan
 */
static sg_t *rdma_get_sg(rdma_chan_t *dma)
{
	sg_t *sg = (sg_t *)rdma_sg_mempool_alloc(GFP_KERNEL);

	/*
	 * No need to zero rest of un-used SG entries;
	 * we detect the src+dst by parameter + sg, not by zero-valued sg.
	 */
	// memzero(&(sg->entry[0]), SG_ENTRY_BYTES);

	sg->status = SG_STATUS_ACQUIRED;

	return sg;
}	


/**
 * rdma_queue_sg - queue an SG, wait done and put it.
 * @dma: dma chan
 * @sg: sg
 * @q_para: parameter 
 * @q_blsz: block size
 * @q_sgad: SG Addr
 * @sg_cnt: count of (src_cnt + dst_cnt)
 */
#define		QUEUE_MODE
static void rdma_queue_sg(rdma_chan_t *rdma, sg_t *sg, u32 q_para, u32 q_blsz, u32 q_sgad, int sg_cnt)
{
	cmdq_t *this_virt = NULL;
	
	spin_lock_irq(&process_lock);

	sg->status = SG_STATUS_SCHEDULED;
	list_add_tail(&sg->lru, &rdma->process_q);

	dump_regs(rdma);

#ifdef	QUEUE_MODE
	/* Setup BP */
	this_virt = (cmdq_t *)(phys_to_virt(*(rdma->cregs->back)));
	this_virt->parameter = q_para;
	this_virt->block_size = q_blsz;
	this_virt->sg_addr = q_sgad;
	this_virt->reserved = 0;
	dump_regs(rdma);

	/* FP++ */
	*(rdma->cregs->frnt) = *(rdma->cregs->frnt) + 16;
	dump_regs(rdma);

	/* FIXME */
	{
		void *sgp = (void *)sg;
		void *cqp = (void *)this_virt;

		rdma_dmac_flush_range(sgp, (sg_cnt * sizeof(u64)));
		rdma_dmac_flush_range(cqp, sizeof(cmdq_t));
	}

	/* Queue Enable */
	*(rdma->cregs->stat) = REG_STAT_CMD_QUEUE_ENABLE; 
	dump_regs(rdma);

#else
	*(dma->cregs->blsz) = q_blsz;
	*(rdma->cregs->sgad) = q_sgad;
	*(rdma->cregs->para) = q_para;
	dump_regs(rdma);
#endif	/* QUEUE_MODE */		

	spin_unlock_irq(&process_lock);
	dump_regs(rdma);

	wait_event_timeout(sg->wait, 
						sg->status & (SG_STATUS_DONE | SG_STATUS_ERROR), 
						dma_timeout_jiffies);
	dump_regs(rdma);

	/* timed out */
	if (unlikely(sg->status & SG_STATUS_SCHEDULED)) {
		printk("%s: operation timeout\n", __FUNCTION__);
		rdma_timeout_handle(rdma);
	}

	sg->status = SG_STATUS_FREE;
	mempool_free(sg, rdma_sg_pool);
	return;
}


#define	R6_RECOV_PD		1
#define	R6_RECOV_DD		2
#define	R6_RECOV_DQ		3
/**
 * @src_no: source count
 * @bytes: len in bytes
 * @bh_ptr: srcs PA
 * @w1_dst: pd: P,  dd: DD1, qd: DD
 * @w2_dst: pd: DD, dd: DD2, qd: Q
 * @pd_dd_qd: failed layout to recover
 * @w1_idx: idx of w1_dst
 * @w2_idx: idx of w2_dst
 * @src_idx: source index; utilize data index only.
 *
 * Desc:
 *     Recover P+DD / DD1+DD2 / DD+Q from bh_ptr
 */
void do_cns_rdma_gfgen_pd_dd_dq(unsigned int src_no, unsigned int bytes, 
						void **bh_ptr, void *w1_dst, void *w2_dst,
						int pd_dd_qd, unsigned int w1_idx, unsigned int w2_idx,
						unsigned int *src_idx)
{
	int i;
	sg_t	*sg = NULL;
	u32 q_sgad, q_blsz, q_para;

	/* clean src/dst */
	for (i=0; i<src_no; i++) 
	{
		if (likely(bh_ptr[i])) {
			rdma_dmac_clean_range(bh_ptr[i], bytes);
		}
		else
			goto abort;
	}
	rdma_dmac_clean_range(w1_dst, bytes);
	rdma_dmac_clean_range(w2_dst, bytes);
	
	sg = rdma_get_sg(dma);

	/* Setup SG */
	switch(pd_dd_qd) 
	{

	case R6_RECOV_PD:
		/* dd...dQ -> PD */
		for (i=0; i<(src_no - 1); i++) {
			sg->entry[i] = (SG_ADDR_MASK & ((u64)virt_to_phys(bh_ptr[i])))
		                   | (SG_READ_IDX_MASK & ((u64)src_idx[i]) << SG_IDX_SHIFT)
						   | (RWI_RD_D);
		}
		sg->entry[src_no-1] =  (SG_ADDR_MASK & ((u64)virt_to_phys(bh_ptr[i])))
								| (RWI_RD_Q);

		/* pd */
		sg->entry[src_no]   = (SG_ADDR_MASK & ((u64)virt_to_phys(w1_dst))) | (RWI_W_P1);
		sg->entry[src_no+1] = (SG_ADDR_MASK & ((u64)virt_to_phys(w2_dst))) | (RWI_W_D2);
		
		q_para = REG_PARA_ENABLE 
				| REG_PARA_XFER_END 
				| REG_PARA_CALC_P
				| (REG_PARA_FAULTY_DISKS_CNT * 2)
				| w2_idx * REG_PARA_FDISK_2_Q_IDX;
		break;

	case R6_RECOV_DD:
		/* dd...PQ -> DD */
		for (i=0; i<(src_no - 2); i++) {
			sg->entry[i] = (SG_ADDR_MASK & ((u64)virt_to_phys(bh_ptr[i])))
		                   | (SG_READ_IDX_MASK & ((u64)src_idx[i]) << SG_IDX_SHIFT)
						   | (RWI_RD_D);
		}
		
		sg->entry[src_no-2] =  (SG_ADDR_MASK & ((u64)virt_to_phys(bh_ptr[i])))
								| (RWI_RD_P);
		sg->entry[src_no-1] =  (SG_ADDR_MASK & ((u64)virt_to_phys(bh_ptr[i+1])))
								| (RWI_RD_Q);

		/* dd */
		sg->entry[src_no]   = (SG_ADDR_MASK & ((u64)virt_to_phys(w1_dst))) | (RWI_W_D1);
		sg->entry[src_no+1] = (SG_ADDR_MASK & ((u64)virt_to_phys(w2_dst))) | (RWI_W_D2);
		
		q_para = REG_PARA_ENABLE 
				| REG_PARA_XFER_END 
				| REG_PARA_CALC_DATA
				| (REG_PARA_FAULTY_DISKS_CNT * 2)
				| w1_idx * REG_PARA_FDISK_1_P_IDX
				| w2_idx * REG_PARA_FDISK_2_Q_IDX;

		break;

	case R6_RECOV_DQ:
		/* dd...dP -> DQ */
		for (i=0; i<(src_no - 1); i++) {
			sg->entry[i] = (SG_ADDR_MASK & ((u64)virt_to_phys(bh_ptr[i])))
		                   | (SG_READ_IDX_MASK & ((u64)src_idx[i]) << SG_IDX_SHIFT)
						   | (RWI_RD_D);
		}
		sg->entry[src_no-1] =  (SG_ADDR_MASK & ((u64)virt_to_phys(bh_ptr[i])))
								| (RWI_RD_P);

		/* qd */
		sg->entry[src_no]   = (SG_ADDR_MASK & ((u64)virt_to_phys(w1_dst))) | (RWI_W_D1);
		sg->entry[src_no+1] = (SG_ADDR_MASK & ((u64)virt_to_phys(w2_dst))) | (RWI_W_Q2);
		
		q_para = REG_PARA_ENABLE 
				| REG_PARA_XFER_END 
				| REG_PARA_CALC_Q
				| (REG_PARA_FAULTY_DISKS_CNT * 2)
				| w1_idx * REG_PARA_FDISK_1_P_IDX;
		break;

	default:
		BUG();
		break;

	}
	
	q_sgad = virt_to_phys(&(sg->entry[0]));
	q_blsz = bytes & REG_BLSZ_MASK;

	if (unlikely(rdma_verbose)) {
		for (i=0; i<src_no; i++)
			printk("set-SG::SRC[%d] = 0x%016llx\n", i, sg->entry[i]);
		printk("set-SG::DST1ptr= 0x%016llx\n", sg->entry[src_no]);
		printk("set-SG::DST2ptr= 0x%016llx\n", sg->entry[src_no+1]);
	}
 
	/* Queue SG */
	rdma_queue_sg(dma, sg, q_para, q_blsz, q_sgad, (src_no + 2));
  
	/* Invalidate dst */
	rdma_dmac_inv_range(w1_dst, bytes);
	rdma_dmac_inv_range(w2_dst, bytes);
  
abort:
	return;
}

/**
 * @src_no: source count
 * @bytes: len in bytes
 * @bh_ptr: srcs PA
 * @p_dst: P dest PA
 * @q_dst: Q dest PA
 *
 * Desc:
 *     p/q_dst = XOR/GFMUL(bh_ptr[0 ... src_no-1]), in Page Addr
 */
void do_cns_rdma_gfgen(unsigned int src_no, unsigned int bytes, void **bh_ptr, 
				void *p_dst, void *q_dst) // u8 *gfmr
{
	int i;
	sg_t	*sg = NULL;
	u32 q_sgad, q_blsz, q_para;

	/* clean src/dst */
	for (i=0; i<src_no; i++) 
	{
		if (likely(bh_ptr[i])) {
			rdma_dmac_clean_range(bh_ptr[i], bytes);
		}
		else
			goto abort;
	}
	rdma_dmac_clean_range(p_dst, bytes);
	rdma_dmac_clean_range(q_dst, bytes);
	
	sg = rdma_get_sg(dma);

	/* Setup SG::Read::SRC */
	for (i=0; i<src_no; i++) {
		/* Set addr, idx#, rw */
		sg->entry[i] = (SG_ADDR_MASK & ((u64)virt_to_phys(bh_ptr[i])))
	                   | (SG_READ_IDX_MASK & ((u64)i + 1) << SG_IDX_SHIFT)
					   | (RWI_RD_D);
	}

	/* Setup SG::Write::P1 + Q2 */
	sg->entry[src_no]   = (SG_ADDR_MASK & ((u64)virt_to_phys(p_dst))) | (RWI_W_P1);
	sg->entry[src_no+1] = (SG_ADDR_MASK & ((u64)virt_to_phys(q_dst))) | (RWI_W_Q2);

	/* Setup SGAD, BLSZ, PARAMETER */
	q_sgad = virt_to_phys(&(sg->entry[0]));
	q_blsz = bytes & REG_BLSZ_MASK;
	q_para = REG_PARA_ENABLE 
			| REG_PARA_XFER_END 
			| REG_PARA_CALC_PQ 
			| (REG_PARA_FAULTY_DISKS_CNT * 2);
	
	if (unlikely(rdma_verbose)) {
		for (i=0; i<src_no; i++)
			printk("set-SG::SRC[%d] = 0x%016llx\n", i, sg->entry[i]);
		printk("set-SG::DST1ptr= 0x%016llx\n", sg->entry[src_no]);
		printk("set-SG::DST2ptr= 0x%016llx\n", sg->entry[src_no+1]);
	}

	/* Queue SG */
	rdma_queue_sg(dma, sg, q_para, q_blsz, q_sgad, (src_no + 2));
  
	/* Invalidate dst */
	rdma_dmac_inv_range(p_dst, bytes);
	rdma_dmac_inv_range(q_dst, bytes);
  
abort:
	return;
}

/**
 * @src_no: source count
 * @bytes: len in bytes
 * @bh_ptr: srcs PA
 * @dst_ptr: dest PA
 *
 * Desc:
 *     dst_ptr = XOR(bh_ptr[0 ... src_no-1]), in Page Addr
 */
void do_cns_rdma_xorgen(unsigned int src_no, unsigned int bytes, void **bh_ptr, void *dst_ptr)
{
	int i;
	sg_t	*sg = NULL;
	u32 q_sgad, q_blsz, q_para;

	/* clean src/dst */
	for (i=0; i<src_no; i++) 
	{
		if (likely(bh_ptr[i])) {
			rdma_dmac_clean_range(bh_ptr[i], bytes);
		}
		else
			goto abort;
	}
	rdma_dmac_clean_range(dst_ptr, bytes);
	
	sg = rdma_get_sg(dma);

	/* Setup SG::Read::SRC */
	for (i=0; i<src_no; i++) {
		sg->entry[i] = (SG_ADDR_MASK & ((u64)virt_to_phys(bh_ptr[i])))
	                   | (SG_READ_IDX_MASK & ((u64)i + 1) << SG_IDX_SHIFT)
					   | (RWI_RD_D);
	}

	/* Setup SG::Write::P1 */
	sg->entry[src_no] = (SG_ADDR_MASK & ((u64)virt_to_phys(dst_ptr)))
					   | (RWI_W_P1);
 
	/* Setup SGAD, BLSZ, PARAMETER */
	q_sgad = virt_to_phys(&(sg->entry[0]));
	q_blsz = bytes & REG_BLSZ_MASK;
	q_para = REG_PARA_ENABLE 
			| REG_PARA_XFER_END 
			| REG_PARA_CALC_P 
			| (REG_PARA_FAULTY_DISKS_CNT * 1);
	
	if (unlikely(rdma_verbose)) {
		for (i=0; i<src_no; i++)
			printk("set-SG::SRC[%d] = 0x%016llx\n", i, sg->entry[i]);
		printk("set-SG::DST1ptr= 0x%016llx\n", sg->entry[src_no]);
	}

	/* Queue SG */
	rdma_queue_sg(dma, sg, q_para, q_blsz, q_sgad, (src_no + 1));
  
	/* Invalidate dst */
	rdma_dmac_inv_range(dst_ptr, bytes);
  
abort:
	return;
}


/**
 * rdma_isr - rdma isr
 * @irq: irq#
 * @dev_id: private data
 */
static irqreturn_t rdma_isr(int irq, void *dev_id)
{
	unsigned long flags;
	rdma_chan_t *this_dma = (rdma_chan_t *)dev_id;

	/* Make sure the INT is for us */
	if (unlikely(dma != this_dma))
	{
		printk(KERN_ERR "Unexpected Interrupt, irq=%d, dma=%p, dev_id=%p\n", irq, dma, dev_id);
		return IRQ_NONE;
	}

	dprintk("%s: pstat=0x%08x\n", __FUNCTION__, *(this_dma->cregs->stat));
	
	spin_lock_irqsave(&process_lock, flags);

	/* clear */
	*(this_dma->cregs->stat) = REG_STAT_XFER_COMPLETE | REG_STAT_INTERRUPT_FLAG;

	if (!list_empty(&this_dma->process_q)) {
		sg_t *sg_fin = list_entry(this_dma->process_q.next, sg_t, lru);

		BUG_ON(!(sg_fin->status & SG_STATUS_SCHEDULED));
		
		list_del_init(&sg_fin->lru);
		sg_fin->status = SG_STATUS_DONE; // TODO: slave/decoder error handling

		/* FP rewind */
		if (*(dma->cregs->frnt) == this_dma->q_last_phys) {
			*(dma->cregs->back) = this_dma->q_first_phys;
			*(dma->cregs->frnt) = this_dma->q_first_phys;
		}

		wake_up(&sg_fin->wait);	
	}
	spin_unlock_irqrestore(&process_lock, flags);
	
	return IRQ_HANDLED;
}

/**
 * test_show - show unit test result
 */
static void test_show(void **src, unsigned int bytes, void *p, void *q, unsigned int src_cnt, int stage)
{
	int i;
	char *buf;
	
	for (i=0; i<src_cnt; i++) {
		buf = (char *)src[i];
		printk("SRC[%d]-stage=%d: %02x %02x %02x %02x %02x %02x %02x %02x %02x, phys=%lx\n", 
				i, stage,
				buf[0], buf[1], buf[16], buf[64], 
				buf[bytes/16], buf[bytes/8], buf[bytes/4], buf[bytes/2], buf[bytes-1], 
				virt_to_phys(src[i]));
	}
		
	buf = (char *)p;
	printk("P-stage=%d:     %02x %02x %02x %02x %02x %02x %02x %02x %02x, phys=%lx\n", stage,
				buf[0], buf[1], buf[16], buf[64], 
				buf[bytes/16], buf[bytes/8], buf[bytes/4], buf[bytes/2], buf[bytes-1], 
				virt_to_phys(p));
	
	buf = (char *)q;
	printk("Q-stage=%d:     %02x %02x %02x %02x %02x %02x %02x %02x %02x, phys=%lx\n", stage,
				buf[0], buf[1], buf[16], buf[64], 
				buf[bytes/16], buf[bytes/8], buf[bytes/4], buf[bytes/2], buf[bytes-1], 
				virt_to_phys(q));
}

/**
 * rdma_unit_test - unit tset invoked by sysfs
 * @action: test item
 * @src_cnt: how many srcs
 * @bytes: length
 *
 * Desc:
 *    Unit Test
 */
void rdma_unit_test(int action, unsigned int src_cnt, unsigned int bytes)
{
	int i, cnt;
	void *src_ptrs[MAX_ENTRIES_PER_SG];
	void *p_dst, *q_dst;
	unsigned int w1_idx, w2_idx;
	unsigned int read_idx[32] = {0};

	/*
	 * The lx330 demo board has only 256MB installed,
	 * we'd be careful.
	 */
	if (src_cnt >= (MAX_ENTRIES_PER_SG - 2))
		src_cnt = MAX_ENTRIES_PER_SG - 2;

	if (src_cnt < 2)
		src_cnt = 2;

	if (bytes > 65536)
		bytes = 65536;

	if (bytes < 4096)
		bytes = 4096;

	for (i = 0; i < MAX_ENTRIES_PER_SG; i++) {
		if (i < src_cnt) {
			src_ptrs[i] = kmalloc(bytes, GFP_KERNEL);
		} else {
			src_ptrs[i] = NULL;
		}
	}
	p_dst = kmalloc(bytes, GFP_KERNEL);
	q_dst = kmalloc(bytes, GFP_KERNEL);

	printk("%s: ACTION=%d, src_cnt=%u, bytes=%u  p/w1=0x%p, q/w2=0x%p\n",
			__FUNCTION__, action, src_cnt, bytes, p_dst, q_dst);

	/* Shuffle the src and dst */
	for (i = 0; i < src_cnt; i++) {
		if (rdma_test_ptn[0] == 0) {
			memset(src_ptrs[i], (jiffies % 240)+1, bytes);
			msleep(10 + 10 * i);
		} else {
			memset(src_ptrs[i], rdma_test_ptn[i], bytes);
		}
	}
	memset(p_dst, 0xff, bytes);
	memset(q_dst, 0xff, bytes);

	// flush_cache_all();
	test_show(src_ptrs, bytes, p_dst, q_dst, src_cnt, 1);

	switch (action)
	{
		/* P */
		case 1:
			printk("\n%s: XORgen\n\n", __FUNCTION__);
			do_cns_rdma_xorgen(src_cnt, bytes, src_ptrs, p_dst);
			break;

		/* PQ */
		case 2:
			printk("\n%s: PQgen\n\n", __FUNCTION__);
			do_cns_rdma_gfgen(src_cnt, bytes, src_ptrs, p_dst, q_dst);
			break;
		
		/* PD */
		case 3:
			w1_idx = src_cnt + 1;
			w2_idx = 1;
			cnt = 0;

			printk("read_idx: ");
			for (i=1; i<=(src_cnt+2); i++)
				if (i != w1_idx && i != w2_idx) {
					read_idx[cnt] = i;
					printk("%d ", i);
					cnt++;
				}
			printk("\n%s: PDgen w1/w2_idx=%u/%u\n\n", __FUNCTION__, w1_idx, w2_idx);
			do_cns_rdma_gfgen_pd_dd_dq(src_cnt, bytes, src_ptrs, p_dst, q_dst,
							R6_RECOV_PD, w1_idx, w2_idx, read_idx);
			break;
	
		/* DD */
		case 4:
			w1_idx = 1;
			w2_idx = 2;
			cnt = 0;

			printk("read_idx: ");
			for (i=1; i<=(src_cnt+2); i++)
				if (i != w1_idx && i != w2_idx) {
					read_idx[cnt] = i;
					printk("%d ", i);
					cnt++;
				}
			printk("\n%s: DDgen w1/w2_idx=%u/%u\n\n", __FUNCTION__, w1_idx, w2_idx);
			do_cns_rdma_gfgen_pd_dd_dq(src_cnt, bytes, src_ptrs, p_dst, q_dst,
							R6_RECOV_DD, w1_idx, w2_idx, read_idx);
			break;
		
		/* DQ */
		case 5:
			w1_idx = 1;
			w2_idx = src_cnt + 2;
			cnt = 0;

			printk("read_idx: ");
			for (i=1; i<=(src_cnt+2); i++)
				if (i != w1_idx && i != w2_idx) {
					read_idx[cnt] = i;
					printk("%d ", i);
					cnt++;
				}
			printk("\n%s: DQgen w1/w2_idx=%u/%u\n\n", __FUNCTION__, w1_idx, w2_idx);
			do_cns_rdma_gfgen_pd_dd_dq(src_cnt, bytes, src_ptrs, p_dst, q_dst,
							R6_RECOV_DQ, w1_idx, w2_idx, read_idx);
			break;

		/* Verbose */
		case 9999:
			rdma_verbose = (rdma_verbose == 1 ? 0 : 1);
			printk("\n%s: Setup verbose mode => %d\n\n", __FUNCTION__, rdma_verbose);
			break;

		/* 
		 * SRC Pattern Assign 
		 * e.g.  0x00000000 <-- do not assign
		 * e.g.  0xbbccddee <-- 4 src: bb cc dd ee
		 */
		default:
			rdma_test_ptn[0] = (u8)(action >> 24 & 0x000000FF);
			rdma_test_ptn[1] = (u8)(action >> 16 & 0x000000FF);
			rdma_test_ptn[2] = (u8)(action >>  8 & 0x000000FF);
			rdma_test_ptn[3] = (u8)(action       & 0x000000FF);

			printk("\n%s: Setup src test pattern => 0x%02x %02x %02x %02x\n\n", __FUNCTION__, 
						rdma_test_ptn[0],
						rdma_test_ptn[1],
						rdma_test_ptn[2],
						rdma_test_ptn[3]);
			break;
	}

	// flush_cache_all();
	test_show(src_ptrs, bytes, p_dst, q_dst, src_cnt, 2);

	for (i = 0; i < MAX_ENTRIES_PER_SG; i++) {
		rdma_kfree_obj(src_ptrs[i]);
	}
	rdma_kfree_obj(p_dst);
	rdma_kfree_obj(q_dst);

}

void cns_rdma_hw_init(void){

	cns3xxx_pwr_clk_en(0x1 << PM_CLK_GATE_REG_OFFSET_RAID);
	cns3xxx_pwr_soft_rst(0x1 << PM_SOFT_RST_REG_OFFST_RAID);
}

/**
 * cns_rdma_init - module init
 */
int __init cns_rdma_init(void)
{
	int err = 0;

	printk("%s: start\n", __FUNCTION__);

	cns_rdma_hw_init();
	
	rdma_test_ptn[0] = 0;
	rdma_verbose = 0;
	dma_timeout_jiffies = HZ;

	/* DMA chan */
	dma = (rdma_chan_t *) kzalloc(sizeof(rdma_chan_t), GFP_KERNEL);
	if (dma == NULL)
		goto abort;

	INIT_LIST_HEAD(&(dma->process_q));
	
	//static DEFINE_SPINLOCK(dma->process_lock);
	dma->irq = IRQ_CNS3XXX_RAID;
	dma->irq_str = "CNS3XXX RAID acceleration";
	dma->cregs = NULL;
	dma->q_virt = NULL;

	/* control register */
	dma->cregs = (struct ctrl_regs *) kzalloc(sizeof(struct ctrl_regs) + GENERIC_ALIGN, GFP_KERNEL);
	dma->cregs = (struct ctrl_regs *) (((u32) dma->cregs & GENERIC_ALIGN_MASK) + GENERIC_ALIGN);
	
	if (dma->cregs == NULL)
		goto abort;
	
	printk("%s: reg1: virt=0x%p\n",
				__FUNCTION__, (void *)dma->cregs);
	
	dma->cregs->para = RDMA_REGS_VIRT(REG_PARA_OFFSET);
	dma->cregs->blsz = RDMA_REGS_VIRT(REG_BLSZ_OFFSET);
	dma->cregs->sgad = RDMA_REGS_VIRT(REG_SGAD_OFFSET);
	dma->cregs->stat = RDMA_REGS_VIRT(REG_STAT_OFFSET);
	dma->cregs->frnt = RDMA_REGS_VIRT(REG_FRNT_OFFSET);
	dma->cregs->back = RDMA_REGS_VIRT(REG_BACK_OFFSET);
	dma->cregs->qpar = RDMA_REGS_VIRT(REG_QPAR_OFFSET);

	/* Pre-allocate S/G table */
	rdma_mempool_create(rdma_sg_pool, "rdma_sg", sizeof(sg_t),
			                    MAX_SG, rdma_sg_prealloc_fn, rdma_sg_deconstruct_fn, NULL);

	/* Pre-allocate Queue Cmds */
	dma->q_virt = (cmdq_t *) kzalloc(sizeof(cmdq_t) * CURR_Q_DEPTH + CURR_Q_DEPTH_ALIGN, GFP_KERNEL);
	dma->q_virt = (cmdq_t *) (((u32) dma->q_virt & CURR_Q_DEPTH_ALIGN_MASK) + CURR_Q_DEPTH_ALIGN);

	if (dma->q_virt == NULL)
		goto abort;

	dma->q_first_phys = virt_to_phys((void *)dma->q_virt);
	dma->q_last_phys = dma->q_first_phys + sizeof(cmdq_t) * (CURR_Q_DEPTH - 1);

	printk("%s: q1: virt=0x%p, phy=0x%x -> 0x%x\n",
				__FUNCTION__, (void *)dma->q_virt, dma->q_first_phys, dma->q_last_phys);

	*(dma->cregs->qpar) = REG_QPAR_DEPTH_32;
	*(dma->cregs->back) = dma->q_first_phys;
	*(dma->cregs->frnt) = dma->q_first_phys;

	/* Register IRQ */
	err = request_irq(dma->irq, rdma_isr, 0, dma->irq_str, dma);
	if (err) {
		printk("%s: request irq failed\n", __FUNCTION__);
		goto abort;
	}

	/* Clear 31 & 0 */
	*(dma->cregs->stat) = REG_STAT_INTERRUPT_FLAG;

	err = 0;
	goto done;	

abort:
	rdma_mempool_destroy(rdma_sg_pool);
	rdma_kfree_obj(dma->cregs);
	rdma_kfree_obj(dma);
	

done:	
	printk("%s: done, err=%d\n", __FUNCTION__, err);
	return err;
}

/**
 * cns_rdma_exit - module exit
 */
void cns_rdma_exit(void)
{
	printk("%s: start\n", __FUNCTION__);

	rdma_mempool_destroy(rdma_sg_pool);
	rdma_kfree_obj(dma->cregs);
	rdma_kfree_obj(dma);
	printk("%s: done\n", __FUNCTION__);
}

//module_init(cns_rdma_init);
//module_exit(cns_rdma_exit);
