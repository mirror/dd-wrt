/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/sysdev.h>
#include <asm/mach/time.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>

#include "xor/mvXor.h"
#include "ctrlEnv/sys/mvSysXor.h"
#include "ctrlEnv/mvCtrlEnvLib.h"

#undef DEBUG


#ifdef DEBUG
	#define DPRINTK(s, args...)  printk("MV_XOR: " s, ## args)
#else
	#define DPRINTK(s, args...)
#endif

#ifdef CONFIG_USE_FOUR_ENGINES
#define XOR_MAX_CHANNELS    4
#elif  CONFIG_USE_TWO_ENGINES
#define XOR_MAX_CHANNELS    2
#else
#define XOR_MAX_CHANNELS    1
#endif


#define NEXT_CHANNEL(chan)	(((chan) + 1) % XOR_MAX_CHANNELS)
#define PREV_CHANNEL(chan)	((chan) ? (chan) - 1 : (XOR_MAX_CHANNELS - 1))

#define XOR_TIMEOUT 0x8000000
struct xor_channel_t
{
    MV_XOR_DESC     *pDescriptor;
    dma_addr_t      descPhyAddr;
    wait_queue_head_t waitq;
    struct semaphore  sema;
#ifdef CONFIG_ENABLE_XOR_INTERRUPTS
    int             irq_num;
    const char      *name;
#endif
    int chan_num;
    int	chan_active;
};

struct xor_channel_t  xor_channel[XOR_MAX_CHANNELS];

struct semaphore  meminit_sema;
int xor_engine_initialized = 0;

MV_XOR_DESC     *pDescriptors;
dma_addr_t      descsPhyAddr;
#define XOR_MIN_COPY_CHUNK 128
static unsigned long mv_dma_min_buffer = XOR_MIN_COPY_CHUNK;

static struct proc_dir_entry *xor_read_proc_entry;
static int xor_dma_hit = 0, xor_dma_miss = 0, xor_dma_unaligned = 0, xor_hit = 0;
static int xor_memzero_hit = 0, xor_memzero_miss = 0, xor_memzero_unaligned = 0;
static int dma_to_user = 0;
static int dma_from_user = 0;
static int copy_tf_failed_on_res = 0;
#define RT_DEBUG
#ifdef RT_DEBUG
static int dma_activations = 0;
#endif
#ifdef CONFIG_MV_USE_XOR_FOR_COPY_USER_BUFFERS
static unsigned long xor_dma_copy(void *to, const void *from, unsigned long n, unsigned int to_user);
#endif
static inline u32 page_remainder(u32 virt)
{
	return PAGE_SIZE - (virt & ~PAGE_MASK);
}

/*
 * map a kernel virtual address or kernel logical address to a phys address
 */
static inline u32 physical_address(u32 virt, int write)
{
    struct page *page;
    struct vm_area_struct *vm;
    struct mm_struct * mm = (virt >= TASK_SIZE)? &init_mm : current->mm;
    unsigned int vm_flags;
    unsigned int flags;

       /* kernel static-mapped address */
    DPRINTK(" get physical address: virt %x , write %d\n", virt, write);
    if (virt_addr_valid(virt)) 
    {
        return __pa((u32) virt);
    }
    if (virt >= (u32)high_memory)
	    return 0;
    
    /* 
    * Require read or write permissions.
    */
    vm_flags  = write ? (VM_WRITE | VM_MAYWRITE) : (VM_READ | VM_MAYREAD);

    vm = find_extend_vma(mm, virt);
    if (!vm || (vm->vm_flags & (VM_IO | VM_PFNMAP))
		|| !(vm_flags & vm->vm_flags)){
			return 0;
		}
    flags = FOLL_PTE_EXIST | FOLL_TOUCH;
    flags |= (write)? FOLL_WRITE : 0;
		 
    page = follow_page(vm, (u32) virt, flags);
    
    if (pfn_valid(page_to_pfn(page)))
    {
        return ((page_to_pfn(page) << PAGE_SHIFT) |
                       ((u32) virt & (PAGE_SIZE - 1)));
    }
    else /* page == 0, otherwise should never happen, since its being checked inside follow_page->vm_normal_page */
    {
        return 0;
    }
}


int allocate_channel(void)
{
    int chan;
    for(chan = 0; chan < XOR_MAX_CHANNELS; chan++)
    {        
        if(down_trylock(&xor_channel[chan].sema))
        {
            DPRINTK("XOR engine %d is busy\n", chan);
            continue;
        }
	if(mvXorStateGet(chan) != MV_IDLE) {
		printk("ERR: %s XOR chan %d is not idle",__FUNCTION__, chan);
	}
        return chan;
    }
    DPRINTK("XOR engines are busy, return\n");
    return -1;
}

void inline free_channel(struct xor_channel_t *channel)
{
	if(mvXorStateGet(channel->chan_num) != MV_IDLE){
		printk("ERR: %s XOR chan %d is not idle",__FUNCTION__, channel->chan_num);
		BUG();
	}
    up(&channel->sema);
}

#define XOR_CAUSE_DONE_MASK(chan) ((BIT0|BIT1) << (chan * 16) )
void xor_waiton_eng(int chan)
{
    int timeout = 0;
    if(!xor_channel[chan].chan_active)
	return;
    
    while(!(MV_REG_READ(XOR_CAUSE_REG(XOR_UNIT(chan))) & XOR_CAUSE_DONE_MASK(XOR_CHAN(chan)))) 
    {
	if(timeout > XOR_TIMEOUT)
	    goto timeout; 
	timeout++;
    }

    timeout = 0;
    while(mvXorStateGet(chan) != MV_IDLE)
    {
	if(timeout > XOR_TIMEOUT)
	    goto timeout; 
	timeout++;
    }
    /* Clear int */
    MV_REG_WRITE(XOR_CAUSE_REG(XOR_UNIT(chan)), ~(XOR_CAUSE_DONE_MASK(XOR_CHAN(chan))));
    xor_channel[chan].chan_active = 0;

timeout:
    if(timeout > XOR_TIMEOUT)
    {
	printk("ERR: XOR eng got timedout!!\n");
	BUG();
    }
    return;

}

void
print_xor_regs(int chan)
{
     printk(" XOR_CHANNEL_ARBITER_REG %08x\n", MV_REG_READ(XOR_CHANNEL_ARBITER_REG(XOR_UNIT(chan))));
     printk(" XOR_CONFIG_REG %08x\n", MV_REG_READ(XOR_CONFIG_REG(XOR_UNIT(chan),
								 XOR_CHAN(chan))));
     printk(" XOR_ACTIVATION_REG %08x\n", MV_REG_READ(XOR_ACTIVATION_REG(
							   XOR_UNIT(chan), XOR_CHAN(chan))));
     printk(" XOR_CAUSE_REG %08x\n", MV_REG_READ(XOR_CAUSE_REG(XOR_UNIT(chan))));
     printk(" XOR_MASK_REG %08x\n", MV_REG_READ(XOR_MASK_REG(XOR_UNIT(chan))));
     printk(" XOR_ERROR_CAUSE_REG %08x\n", MV_REG_READ(XOR_ERROR_CAUSE_REG(XOR_UNIT(chan))));
     printk(" XOR_ERROR_ADDR_REG %08x\n", MV_REG_READ(XOR_ERROR_ADDR_REG(XOR_UNIT(chan))));
     printk(" XOR_NEXT_DESC_PTR_REG %08x\n", MV_REG_READ(XOR_NEXT_DESC_PTR_REG(
							      XOR_UNIT(chan),XOR_CHAN(chan))));
     printk(" XOR_CURR_DESC_PTR_REG %08x\n", MV_REG_READ(XOR_CURR_DESC_PTR_REG(
							      XOR_UNIT(chan),XOR_CHAN(chan))));
     printk(" XOR_BYTE_COUNT_REG %08x\n", MV_REG_READ(XOR_BYTE_COUNT_REG(
							   XOR_UNIT(chan),XOR_CHAN(chan))));
}
#ifdef CONFIG_MV_RAID5_XOR_OFFLOAD
int xor_mv(unsigned int src_no, unsigned int bytes, void **bh_ptr)
{
	void *bptr = NULL;
	int i;
        u32      *srcAddr;
        int         chan;
        struct xor_channel_t *channel;

	if(src_no <= 1)
	{
		printk(KERN_ERR "%s: need more than 1 src for XOR\n",
			__func__);
		BUG();
                return bytes;
	}
        if (xor_engine_initialized == 0)
        {
            printk(KERN_WARNING" %s: xor engines not initialized yet\n", __func__);
            return bytes;
        }

        chan = allocate_channel();
        if ( chan == -1)
         {
                DPRINTK("XOR engines are busy, return\n");
                return bytes;
        }
	DPRINTK("setting up rest of descriptor for channel %d\n", chan);
        channel = &xor_channel[chan];
	// flush the cache to memory before XOR engine touches them
#if defined(MV_CPU_BE)		
	srcAddr = &(channel->pDescriptor->srcAdd1);
#else
        srcAddr = &(channel->pDescriptor->srcAdd0);
#endif
	for(i = src_no-1; i >= 0; i--)
	{
		DPRINTK("flushing source %d\n", i);
		bptr = (bh_ptr[i]);
		/* Buffer 0 is also the destination */
		if(i==0)
			dmac_flush_range(bptr, bptr + bytes);			
		else
			dmac_clean_range(bptr, bptr + bytes);
                srcAddr[i] = virt_to_phys(bh_ptr[i]);
	}
#if defined(MV_CPU_BE)		
	if (src_no & 1) 
		srcAddr[src_no] = virt_to_phys(bh_ptr[src_no-1]);
#endif
	channel->pDescriptor->phyDestAdd = virt_to_phys(bh_ptr[0]);
        channel->pDescriptor->byteCnt = bytes;
        channel->pDescriptor->phyNextDescPtr = 0;
        channel->pDescriptor->descCommand = (1 << src_no) - 1;
        channel->pDescriptor->status = BIT31;
	channel->chan_active = 1;
#if defined(MV_BRIDGE_SYNC_REORDER)
	mvOsBridgeReorderWA();
#endif
        if( mvXorTransfer(chan, MV_XOR, channel->descPhyAddr) != MV_OK )
        {
            printk(KERN_ERR "%s: XOR operation on channel %d failed!\n", __func__, chan);
            print_xor_regs(chan);
            BUG();
            free_channel(channel);
            return bytes;
        }
#ifdef CONFIG_ENABLE_XOR_INTERRUPTS
	/* should unmask xor interrupt*/
        wait_event(channel->waitq, (( channel->pDescriptor->status & BIT31) == 0));/*TODO add timeout*/
	/* shoulds mask back xor interrutps here*/
#else
        xor_waiton_eng(chan);
#endif
	DPRINTK("XOR complete\n");
#if 0
	if (!(channel->pDescriptor->status & BIT30)) {
	    printk(KERN_ERR "%s: XOR operation completed with error!\n", __func__);
            print_xor_regs(chan);            
	    BUG();
            free_channel(channel);
	    return MV_REG_READ(XOR_BYTE_COUNT_REG(chan));
        }
#endif
	DPRINTK("invalidate result in cache\n");
#if 0
	// invalidate the cache region to destination
        bptr = (bh_ptr[0]);
	dmac_inv_range(bptr, bptr + bytes);
#endif
        free_channel(channel);
        xor_hit++;
        return 0;
}
#endif
#ifdef CONFIG_MV_XORMEMCOPY

/*=======================================================================*/
/*  Procedure:  xor_memcpy()                                             */
/*                                                                       */
/*  Description:    DMA-based in-kernel memcpy.                          */
/*                                                                       */
/*  Parameters:  to: destination address                                 */
/*               from: source address                                    */
/*               n: number of bytes to transfer                          */
/*                                                                       */
/*  Returns:     void*: to                                               */
/*                                                                       */
/*  Notes/Assumptions:                                                   */
/*              Assumes that kernel physical memory is contiguous, i.e., */
/*              the physical addresses of contiguous virtual addresses   */
/*              are also contiguous.                                     */
/*              Assumes that kernel memory doesn't get paged.            */
/*              The DMA is polling                                       */
/*                                                                       */
/*=======================================================================*/
void *xor_memcpy(void *to, const void *from, __kernel_size_t n)
{
	u32 xor_dma_unaligned_to, xor_dma_unaligned_from;
	void *orig_to = to;
	u32 to_pa, from_pa;
        int ua = 0;
        int         chan;
        struct xor_channel_t *channel;

	DPRINTK("xor_memcpy(0x%x, 0x%x, %lu): entering\n", (u32) to, (u32) from,
		(unsigned long)n);

        if (xor_engine_initialized == 0)
        {
            DPRINTK(KERN_WARNING" %s: xor engines not initialized yet\n", __func__);
       	    xor_dma_miss++;
	    return asm_memmove(to, from, n);
        }
 	if (!(virt_addr_valid((u32) to) && virt_addr_valid((u32) from))) {
		DPRINTK("xor_memcpy(0x%x, 0x%x, %lu): falling back to memcpy\n",
			(u32) to, (u32) from, (unsigned long)n);
		xor_dma_miss++;
		return asm_memmove(to, from, n);
	}

	/*
	 * We can only handled completely cache-aligned transactions
	 * with the DMA engine.  Source and Dst must be cache-line
	 * aligned AND the length must be a multiple of the cache-line.
	 */

	to_pa = virt_to_phys(to);
	from_pa = virt_to_phys((void*)from);

	if (((to_pa + n > from_pa) && (to_pa < from_pa)) ||
	    ((from_pa < to_pa) && (from_pa + n > to_pa))) {
		DPRINTK("overlapping copy region (0x%x, 0x%x, %lu), falling back\n",
		     to_pa, from_pa, (unsigned long)n);
		xor_dma_miss++;
		return asm_memmove(to, from, n);
	}
	/*
	 * Ok, start addr is not cache line-aligned, so we need to make it so.
	 */
	xor_dma_unaligned_to = (u32) to & 31;
	xor_dma_unaligned_from = (u32) from & 31;;
	if (xor_dma_unaligned_to | xor_dma_unaligned_from) {
		ua++;
		if (xor_dma_unaligned_from > xor_dma_unaligned_to) {
			asm_memmove(to, from, 32 - xor_dma_unaligned_to);
			to = (void *)((u32)to + 32 - xor_dma_unaligned_to);
			from = (void *)((u32)from + 32 - xor_dma_unaligned_to);
			n -= 32 - xor_dma_unaligned_to;
		} else {
			asm_memmove(to, from, 32 - xor_dma_unaligned_from);
			to = (void *)((u32)to + 32 - xor_dma_unaligned_from);
			from = (void *)((u32)from + 32 - xor_dma_unaligned_from);
			n -= 32 - xor_dma_unaligned_from;
		}
	}

	/*
	 * Ok, we're aligned at the top, now let's check the end
	 * of the buffer and align that. After this we should have
	 * a block that is a multiple of cache line size.
	 */
	xor_dma_unaligned_to = ((u32) to + n) & 31;
	xor_dma_unaligned_from = ((u32) from + n) & 31;;
	if (xor_dma_unaligned_to | xor_dma_unaligned_from) {
		ua++;
		if (xor_dma_unaligned_to > xor_dma_unaligned_from) {
			u32 tmp_to = (u32) to + n - xor_dma_unaligned_to;
			u32 tmp_from = (u32) from + n - xor_dma_unaligned_to;

			asm_memmove((void *)tmp_to, (void *)tmp_from,
				   xor_dma_unaligned_to);

			n -= xor_dma_unaligned_to;
		} else {
			u32 tmp_to = (u32) to + n - xor_dma_unaligned_from;
			u32 tmp_from = (u32) from + n - xor_dma_unaligned_from;

			asm_memmove((void *)tmp_to, (void *)tmp_from,
				   xor_dma_unaligned_from);

			n -= xor_dma_unaligned_from;
		}
	}

	/*
	 * OK! We should now be fully aligned on both ends. 
	 */
        chan = allocate_channel();
        if ( chan == -1)
        {
                DPRINTK("XOR engines are busy, return\n");
       		xor_dma_miss++;
		return asm_memmove(to, from, n);
        }
	DPRINTK("setting up rest of descriptor for channel %d\n", chan);
        channel = &xor_channel[chan];
	
        /* Ensure that the cache is clean */
	dmac_clean_range(from, from + n);
	dmac_inv_range(to, to + n);

	DPRINTK("setting up rest of descriptor\n");
	// flush the cache to memory before XOR engine touches them
        channel->pDescriptor->srcAdd0 = virt_to_phys((void*)from);
	channel->pDescriptor->phyDestAdd = virt_to_phys(to);
        channel->pDescriptor->byteCnt = n;
        channel->pDescriptor->phyNextDescPtr = 0;
        channel->pDescriptor->status = BIT31;
	channel->chan_active = 1;
#if defined(MV_BRIDGE_SYNC_REORDER)
	mvOsBridgeReorderWA();
#endif
        if( mvXorTransfer(chan, MV_DMA, channel->descPhyAddr) != MV_OK)
        {
            printk(KERN_ERR "%s: DMA copy operation on channel %d failed!\n", __func__, chan);
            print_xor_regs(chan);
            BUG();
            free_channel(channel);
       	    return asm_memmove(to, from, n);
        }
        xor_waiton_eng(chan);

        DPRINTK("DMA copy complete\n");
	// check to see if failed
#if 0
	if (!(channel->pDescriptor->status & BIT30))
        {
            printk(KERN_ERR "%s: DMA copy operation completed with error!\n", __func__);
            printk(" srcAdd %x DestAddr %x, count %x\n", channel->pDescriptor->srcAdd0,
                                                channel->pDescriptor->phyDestAdd, n); 
            print_xor_regs(chan);            
	    BUG();
            free_channel(channel);
       	    return asm_memmove(to, from, n);
        }
#endif
        free_channel(channel);
 
	xor_dma_hit++;
	if (ua)
		xor_dma_unaligned++;

	return orig_to;
}
EXPORT_SYMBOL(xor_memcpy);
#endif
#ifdef CONFIG_MV_USE_XOR_FOR_COPY_USER_BUFFERS
/*=======================================================================*/
/*  Procedure:  xor_copy_to_user()                                       */
/*                                                                       */
/*  Description:    DMA-based copy_to_user.                              */
/*                                                                       */
/*  Parameters:  to: destination address                                 */
/*               from: source address                                    */
/*               n: number of bytes to transfer                          */
/*                                                                       */
/*  Returns:     unsigned long: number of bytes NOT copied               */
/*                                                                       */
/*  Notes/Assumptions:                                                   */
/*              Assumes that kernel physical memory is contiguous, i.e., */
/*              the physical addresses of contiguous virtual addresses   */
/*              are also contiguous.                                     */
/*              Assumes that kernel memory doesn't get paged.            */
/*              Assumes that to/from memory regions cannot overlap       */
/*                                                                       */
/*=======================================================================*/
unsigned long xor_copy_to_user(void *to, const void *from, unsigned long n)
{
	if(!xor_engine_initialized)
    		return __arch_copy_to_user((void *)to, (void *)from, n);

     	dma_to_user++;
     	DPRINTK(KERN_CRIT "xor_copy_to_user(%#10x, 0x%#10x, %lu): entering\n", (u32) to, (u32) from, n);
    
        return  xor_dma_copy(to, from, n, 1);
}
EXPORT_SYMBOL(xor_copy_to_user);
/*=======================================================================*/
/*  Procedure:  xor_copy_from_user()                                     */
/*                                                                       */
/*  Description:    DMA-based copy_from_user.                            */
/*                                                                       */
/*  Parameters:  to: destination address                                 */
/*               from: source address                                    */
/*               n: number of bytes to transfer                          */
/*                                                                       */
/*  Returns:     unsigned long: number of bytes NOT copied               */
/*                                                                       */
/*  Notes/Assumptions:                                                   */
/*              Assumes that kernel virtual memory is contiguous, i.e.,  */
/*              the physical addresses of contiguous virtual addresses   */
/*              are also contiguous.                                     */
/*              Assumes that kernel memory doesn't get paged.            */
/*              Assumes that to/from memory regions cannot overlap       */
/*              XXX this one doesn't quite work right yet                */
/*                                                                       */
/*=======================================================================*/
unsigned long xor_copy_from_user(void *to, const void *from, unsigned long n)
{
    if(!xor_engine_initialized)
	return __arch_copy_from_user((void *)to, (void *)from, n);

    dma_from_user++;
    DPRINTK(KERN_CRIT "xor_copy_from_user(0x%x, 0x%x, %lu): entering\n", (u32) to, (u32) from, n);
    return  xor_dma_copy(to, from, n, 0);
}

EXPORT_SYMBOL(xor_copy_from_user);
/*
 * n must be greater equal than 64.
 */
static unsigned long xor_dma_copy(void *to, const void *from, unsigned long n, unsigned int to_user)
{
	u32 chunk,i;
	u32 k_chunk = 0;
	u32 u_chunk = 0;
	u32 phys_from, phys_to;
	
        unsigned long flags;
	u32 unaligned_to;
	u32 index = 0;
        u32 temp;
	u32 taken_channel[XOR_MAX_CHANNELS];

        unsigned long uaddr, kaddr;
	int     chan;
        int     current_channel;
        struct xor_channel_t *channel;
       
        DPRINTK("xor_dma_copy: entering\n");
	
	/* allocate all channels*/
	for(chan = 0; chan < XOR_MAX_CHANNELS; chan++)
	{
	     if((taken_channel[chan] = allocate_channel()) == -1)
	     {
		  copy_tf_failed_on_res++;
		  for(;chan; chan--)
		       free_channel(&xor_channel[taken_channel[chan-1]]);
		  goto exit_dma;
	     }
	}

	current_channel = 0;
	/* 
      	 * The unaligned is taken care seperatly since the dst might be part of a cache line that is changed 
	 * by other process -> we must not invalidate this cache lines and we can't also flush it, since other 
	 * process (or the exception handler) might fetch the cache line before we copied it. 
	 */

	/*
	 * Ok, start addr is not cache line-aligned, so we need to make it so.
	 */
	unaligned_to = (u32)to & 31;
	if(unaligned_to)
	{
		DPRINTK("Fixing up starting address %d bytes\n", 32 - unaligned_to);

		if(to_user)
		{
		    if((__arch_copy_to_user(to, from, 32 - unaligned_to))) 
			goto free_channels; 
		}
		else
		{
		    if((__arch_copy_from_user(to, from, 32 - unaligned_to))) 
			goto free_channels;
		}

		temp = (u32)to + (32 - unaligned_to);
		to = (void *)temp;
		temp = (u32)from + (32 - unaligned_to);
		from = (void *)temp;

                /*it's ok, n supposed to be greater than 32 bytes at this point*/
		n -= (32 - unaligned_to);
	}

	/*
	 * Ok, we're aligned at the top, now let's check the end
	 * of the buffer and align that. After this we should have
	 * a block that is a multiple of cache line size.
	 */
	unaligned_to = ((u32)to + n) & 31;
	if(unaligned_to)
	{	
		u32 tmp_to = (u32)to + (n - unaligned_to);
		u32 tmp_from = (u32)from + (n - unaligned_to);
		DPRINTK("Fixing ending alignment %d bytes\n", unaligned_to);

		if(to_user)
		{
		    if((__arch_copy_to_user((void *)tmp_to, (void *)tmp_from, unaligned_to)))
			goto free_channels;
		}
		else
		{
		    if((__arch_copy_from_user((void *)tmp_to, (void *)tmp_from, unaligned_to)))
			goto free_channels;
		}

                /*it's ok, n supposed to be greater than 32 bytes at this point*/
		n -= unaligned_to;
	}

        if(to_user)
        {
            uaddr = (unsigned long)to;  
            kaddr = (unsigned long)from;
        }
        else
        {
             uaddr = (unsigned long)from;
             kaddr = (unsigned long)to;
        }
        if(virt_addr_valid(kaddr))
        {
            k_chunk = n;
        }
	else
	{
		DPRINTK("kernel address is not linear, fall back\n");
		goto free_channels;		
	}
#if defined(CONFIG_SMP)
	#warning "*** FIXME: --> for SMP should fix this spin_lock issue ***"
/*      once we put here this -->  spin_lock_irqsave(&current->mm->page_table_lock, flags); 
 *      but since acquiring this spinlock later on, we got a deadlock(on SMP). */
#else
	local_irq_save(flags);
#endif
     
        i = 0;
	while(n > 0)
	{
	    if(k_chunk == 0)
	    {
                /* virtual address */
	        k_chunk = page_remainder((u32)kaddr);
		DPRINTK("kaddr reminder %d \n",k_chunk);
	    }

	    if(u_chunk == 0)
	    {
                u_chunk = page_remainder((u32)uaddr);
                DPRINTK("uaddr reminder %d \n", u_chunk);
            }
        
            chunk = ((u_chunk < k_chunk) ? u_chunk : k_chunk);
            if(n < chunk)
	    {
		chunk = n;
	    }

	    if(chunk == 0)
	    {
	    	break;
	    }
            phys_from = physical_address((u32)from, 0);
            phys_to = physical_address((u32)to, 1);
	    DPRINTK("choose chunk %d \n",chunk);

	    /* if page doesn't exist go out immediatly, don't try to copy by CPU, since it is not right 
	     * to get page fault while int are locked. see do_page_fault */
	    if ((!phys_from) || (!phys_to))
	    {
		/* The requested page isn't available, fall back to */
		DPRINTK(" no physical address, fall back: from %p , to %p \n", from, to);
		goto unlock_dma;
   
	    }
	    /*
	     *  Prepare the IDMA.
	     */
            if (chunk < XOR_MIN_COPY_CHUNK)
            {
                int last_chan = PREV_CHANNEL(current_channel); 
        	DPRINTK(" chunk %d too small , use memcpy \n",chunk);
        	
                /* the "to" address might cross cache line boundary, so part of the  */
		/* line may be subject to DMA, so we need to wait to last DMA engine */
		/* to finish */
                if(index > 0)
                    xor_waiton_eng(last_chan);

                if(to_user)
		{
	       	    if((__arch_copy_to_user((void *)to, (void *)from, chunk))) {
			printk("ERROR: %s %d shouldn't happen\n",__FUNCTION__, __LINE__);	
			goto unlock_dma;
		    }
            }
            else
	    {
	            if((__arch_copy_from_user((void *)to, (void *)from, chunk))) {
			printk("ERROR: %s %d shouldn't happen\n",__FUNCTION__, __LINE__);	
		    goto unlock_dma;
		    }
		}
		}
		else
		{
		    /* 
		    * Ensure that the cache is clean:
		    *      - from range must be cleaned
		    *      - to range must be invalidated
		    */
//		    mvOsCacheFlush(NULL, (void *)from, chunk);
		    dmac_flush_range(from, from + chunk);
		    //	    mvOsCacheInvalidate(NULL, (void *)to, chunk);
		    dmac_inv_range(to, to + chunk);
		    if(index > 0)
		    {
			xor_waiton_eng(current_channel);
		    }
		    channel = &xor_channel[current_channel];
  
		    /* Start DMA */
		    DPRINTK(" activate DMA: channel %d from %x to %x len %x\n",
                            current_channel, phys_from, phys_to, chunk);
		    channel->pDescriptor->srcAdd[0] = phys_from;
		    channel->pDescriptor->phyDestAdd = phys_to;
		    channel->pDescriptor->byteCnt = chunk;
		    channel->pDescriptor->phyNextDescPtr = 0;
		    channel->pDescriptor->status = BIT31;
		    channel->chan_active = 1;
#if defined(MV_BRIDGE_SYNC_REORDER)
		    mvOsBridgeReorderWA();
#endif
		    if( mvXorTransfer(current_channel, MV_DMA, channel->descPhyAddr) != MV_OK)
		    {
			printk(KERN_ERR "%s: DMA copy operation on channel %d failed!\n", __func__, current_channel);
			print_xor_regs(current_channel);
			BUG();
		    }
                
		    current_channel = NEXT_CHANNEL(current_channel);
#ifdef RT_DEBUG
			dma_activations++;
#endif
			index++;
		    }

		/* go to next chunk */
		from += chunk;
		to += chunk;
                kaddr += chunk;
                uaddr += chunk;
		n -= chunk;
		u_chunk -= chunk;
		k_chunk -= chunk;		
	}
unlock_dma:
	for(chan = 0; chan < XOR_MAX_CHANNELS; chan++)
	     xor_waiton_eng(chan);
#if defined(CONFIG_SMP)
	#warning "*** FIXME: <-- for SMP should fix this spin_lock issue ***"
        /*spin_unlock_irqrestore(&current->mm->page_table_lock, flags);*/
#else
	local_irq_restore(flags);
#endif
free_channels:
	for(chan = 0; chan < XOR_MAX_CHANNELS; chan++)
	     free_channel(&xor_channel[chan]);
exit_dma:        
        DPRINTK("xor_dma_copy(0x%x, 0x%x, %lu): exiting\n", (u32) to,
                (u32) from, n);
       
        if(n != 0)
        {
       	    if(to_user)
                return __arch_copy_to_user((void *)to, (void *)from, n);
	            else
                return __arch_copy_from_user((void *)to, (void *)from, n);
        }
        return 0;
}
#endif

#ifdef CONFIG_MV_XORMEMZERO
/*=======================================================================*/
/*  Procedure:  xor_memzero()                                             */
/*                                                                       */
/*  Description:    DMA-based in-kernel memzero.                          */
/*                                                                       */
/*  Parameters:  to: destination address                                 */
/*               n: number of bytes to transfer                          */
/*                                                                       */
/*                                                                       */
/*  Notes/Assumptions:                                                   */
/*              Assumes that kernel physical memory is contiguous, i.e., */
/*              the physical addresses of contiguous virtual addresses   */
/*              are also contiguous.                                     */
/*              Assumes that kernel memory doesn't get paged.            */
/*              The DMA is polling                                       */
/*                                                                       */
/*=======================================================================*/
void xor_memzero(void *to, __kernel_size_t n)
{
	u32 xor_dma_unaligned_to;
	u32 to_pa;
        int ua = 0;
        int         chan;
        struct xor_channel_t *channel;

	DPRINTK("xor_memzero(0x%x, %lu): entering\n", (u32) to, (unsigned long)n);

        if (xor_engine_initialized == 0)
        {
            DPRINTK(KERN_WARNING" %s: xor engines not initialized yet\n", __func__);
       	    xor_memzero_miss++;
	    return asm_memzero(to, n);
        }
 	if (!(virt_addr_valid((u32) to))) {
		DPRINTK("xor_memcpy(0x%x, %lu): falling back to memzero\n",
			(u32) to, (unsigned long)n);
		xor_memzero_miss++;
		return asm_memzero(to, n);
	}

	/*
	 * We can only handled completely cache-aligned transactions
	 * with the DMA engine.  Dst must be cache-line
	 * aligned AND the length must be a multiple of the cache-line.
	 */

	to_pa = virt_to_phys(to);

	/*
	 * Ok, start addr is not cache line-aligned, so we need to make it so.
	 */
	xor_dma_unaligned_to = (u32) to & 31;
	if (xor_dma_unaligned_to)
        {
            ua++;
	    asm_memzero(to, 32 - xor_dma_unaligned_to);
            to = (void *)((u32)to + 32 - xor_dma_unaligned_to);
	    n -= 32 - xor_dma_unaligned_to;
	}

	/*
	 * Ok, we're aligned at the top, now let's check the end
	 * of the buffer and align that. After this we should have
	 * a block that is a multiple of cache line size.
	 */
	xor_dma_unaligned_to = ((u32) to + n) & 31;
	if (xor_dma_unaligned_to) {
	    u32 tmp_to = (u32) to + n - xor_dma_unaligned_to;
	    asm_memzero((void *)tmp_to, xor_dma_unaligned_to);
            n -= xor_dma_unaligned_to;
	    ua++;
	}

	/*
	 * OK! We should now be fully aligned on both ends. 
	 */
        chan = allocate_channel();
        if ( chan == -1)
         {
                DPRINTK("XOR engines are busy, return\n");
       		xor_memzero_miss++;
		return asm_memzero(to, n);
        }
        if (down_trylock(&meminit_sema))
        {
            DPRINTK("meminit is used by one of the XOR engines\n");
            xor_memzero_miss++;
            free_channel(&xor_channel[chan]);
	    return asm_memzero(to, n);
        }

	DPRINTK("setting up rest of descriptor for channel %d\n", chan);
        channel = &xor_channel[chan];
	
        /* Ensure that the cache is clean */
	dmac_inv_range(to, to + n);

	channel->chan_active = 1;

	DPRINTK("setting up rest of descriptor\n");
        if( mvXorMemInit(chan, virt_to_phys(to), n, 0, 0) != MV_OK)
        {
            printk(KERN_ERR "%s: DMA memzero operation on channel %d failed. to %p len %d!\n", __func__, chan,
                to, n);
            free_channel(channel);
            up(&meminit_sema);
       	    return asm_memzero(to, n);
        }
        xor_waiton_eng(chan);


        DPRINTK("DMA memzero complete\n");
	// check to see if failed
        up(&meminit_sema);
        free_channel(channel);
	xor_memzero_hit++;
	if (ua)
		xor_memzero_unaligned++;

}
EXPORT_SYMBOL(xor_memzero);
#endif

#ifdef CONFIG_ENABLE_XOR_INTERRUPTS
static irqreturn_t
mv_xor_isr(int irq, void *dev_id)
{
    MV_U32  reg;

    reg = MV_REG_READ(XOR_CAUSE_REG(0));
    MV_REG_WRITE(XOR_CAUSE_REG(0), ~reg);
    DPRINTK("%s: unit 0: cause 0x%08x, dev_id %d",__func__, reg, (int)dev_id);
    if(reg & BIT1)
    {
       wake_up(&xor_channel[0].waitq);
    }
#if XOR_MAX_CHANNELS  >  1
    if(reg & BIT17)
    {
       wake_up(&xor_channel[1].waitq);
    }
#endif
#if XOR_MAX_CHANNELS  >  2
    reg = MV_REG_READ(XOR_CAUSE_REG(1));
    MV_REG_WRITE(XOR_CAUSE_REG(1), ~reg);
    DPRINTK("%s: unit 1: cause 0x%08x, dev_id %d",__func__, reg, (int)dev_id);
    if(reg & BIT1)
    {
       wake_up(&xor_channel[2].waitq);
    }
#if XOR_MAX_CHANNELS  >  3
    if(reg & BIT17)
    {
       wake_up(&xor_channel[3].waitq);
    }
#endif
#endif

    /*ignore access protection*/ 
    if( reg & ~(BIT20|BIT17|BIT16|BIT4|BIT1|BIT0))
    {
        printk("%s error: cause register 0x%08x\n", __func__, reg);
    }
    return IRQ_HANDLED;
}
#endif
static int xor_read_proc(char *buf, char **start, off_t offset, int len,
			 int *eof, void *data)
{
	len = 0;

	len += sprintf(buf + len, "Number of XOR hits: %d\n", xor_hit);
	len += sprintf(buf + len, "XOR DMA memcopy hits: %d\n", xor_dma_hit);
	len += sprintf(buf + len, "XOR DMA memcopy misses: %d\n", xor_dma_miss);
	len += sprintf(buf + len, "XOR DMA memcopy unaligned buffers: %d\n", xor_dma_unaligned);
	len += sprintf(buf + len, "XOR DMA memzero hits: %d\n", xor_memzero_hit);
	len += sprintf(buf + len, "XOR DMA memzero misses: %d\n", xor_memzero_miss);
	len += sprintf(buf + len, "XOR DMA memzero unaligned buffers: %d\n", xor_memzero_hit);
        len += sprintf(buf + len, "XOR copy to/from user DMA min buffer %ld\n", mv_dma_min_buffer);
	len += sprintf(buf + len, "Number of XOR DMA copy to user %d copy from user %d \n",
                                    dma_to_user, dma_from_user);
	len += sprintf(buf + len, "copy to/from failed on channel allocation 0x%x \n", 
									copy_tf_failed_on_res);
#ifdef RT_DEBUG
	len += sprintf(buf + len, "copy to/from user dma activations %d\n", dma_activations);
#endif
	return len;
}

int mv_xor_init(void)
{
    int chan;
#ifdef CONFIG_ENABLE_XOR_INTERRUPTS
    int err = 0;
#endif
    char *mode = "acceleration";

#ifdef CONFIG_MV78200
    if (MV_FALSE == mvSocUnitIsMappedToThisCpu(XOR))
    {
	    printk(KERN_INFO"XOR engine is not mapped to this CPU\n");
	    return -ENODEV;
    }	
#endif

#ifdef CONFIG_ENABLE_XOR_INTERRUPTS
    mode = "offloading";
#endif
    
    printk(KERN_INFO "Use the XOR engines (%s) for enhancing the following functions:\n", mode);
#ifdef CONFIG_MV_RAID5_XOR_OFFLOAD
    printk(KERN_INFO "  o RAID 5 Xor calculation\n");
#endif
#ifdef CONFIG_MV_XORMEMCOPY
    printk(KERN_INFO "  o kernel memcpy\n");
#endif
#ifdef CONFIG_MV_XORMEMZERO
    printk(KERN_INFO "  o kenrel memzero\n");
#endif
#ifdef CONFIG_MV_USE_XOR_FOR_COPY_USER_BUFFERS
    printk(KERN_INFO "  o copy user to/from kernel buffers\n");
#endif
    printk(KERN_INFO "Number of XOR engines to use: %d\n", XOR_MAX_CHANNELS);

    if(mvCtrlModelGet() == MV_5082_DEV_ID)
    {
        printk(KERN_WARNING " This device doesn't have XOR engines.\n");    
        return -ENODEV;
    }
    mvXorInit();

    /* pre-alloc XOR descriptors */
    pDescriptors = dma_alloc_coherent(NULL, sizeof(MV_XOR_DESC) * XOR_MAX_CHANNELS,
                                            &descsPhyAddr, GFP_KERNEL);  
    if(pDescriptors == NULL)
    {
        printk(KERN_ERR "%s: failed to allocate XOR descriptors\n", __func__);
        return -ENOMEM;
    }
    sema_init(&meminit_sema, 1);
    memset(pDescriptors, 0, sizeof(MV_XOR_DESC) * XOR_MAX_CHANNELS);
    DPRINTK(" allocating XOR Descriptors: virt add %p, phys addr %x\n", 
        pDescriptors, descsPhyAddr);
    for(chan = 0; chan  < XOR_MAX_CHANNELS; chan++)
    {
	xor_channel[chan].chan_num = chan;
        xor_channel[chan].pDescriptor = pDescriptors + chan;
        xor_channel[chan].descPhyAddr = descsPhyAddr + (sizeof(MV_XOR_DESC) * chan);
	xor_channel[chan].chan_active = 0;

        sema_init(&xor_channel[chan].sema, 1);
        init_waitqueue_head(&xor_channel[chan].waitq);
#if 0
        mvXorCtrlSet(chan, (1 << XEXCR_REG_ACC_PROTECT_OFFS) | 
                    (4 << XEXCR_DST_BURST_LIMIT_OFFS) |
                    (4 << XEXCR_SRC_BURST_LIMIT_OFFS)
#if defined(MV_CPU_BE)
					 | (1 << XEXCR_DES_SWP_OFFS)
#endif
					 );
#endif
#ifdef CONFIG_ENABLE_XOR_INTERRUPTS
        switch(chan)
        {
            case 0:
                xor_channel[chan].irq_num = XOR0_IRQ_NUM;
		xor_channel[chan].name = "xor_chan0";
                break;
            case 1:
                xor_channel[chan].irq_num = XOR1_IRQ_NUM;
                xor_channel[chan].name = "xor_chan1";
            	break;
#if XOR_MAX_CHANNELS > 2
            case 2:
                xor_channel[chan].irq_num = XOR2_IRQ_NUM;
                xor_channel[chan].name = "xor_chan2";
            	break;
            case 3:
                xor_channel[chan].irq_num = XOR3_IRQ_NUM;
                xor_channel[chan].name = "xor_chan3";
            	break;
#endif
            default:
                printk(KERN_ERR "%s: trying to configure bad xor channel\n", __func__);
                return -ENXIO; 
        }
        err = request_irq(xor_channel[chan].irq_num, mv_xor_isr, IRQF_DISABLED,
				  xor_channel[chan].name, (void *)chan);
        if (err < 0)
        {
            printk(KERN_ERR "%s: unable to request IRQ %d for "
                            "XOR %d: %d\n", __func__, xor_channel[chan].irq_num, chan, err);
        	return -EBUSY;
        }
#endif

    }
#ifdef CONFIG_PROC_FS
	xor_read_proc_entry =
        create_proc_entry("mv_xor", S_IFREG | S_IRUGO, 0);
        xor_read_proc_entry->read_proc = xor_read_proc;
        xor_read_proc_entry->write_proc = NULL;
	xor_read_proc_entry->nlink = 1;
#endif
    xor_engine_initialized = 1;
    return 0;
}

void mv_xor_exit(void)
{
    printk(KERN_INFO "XOR acceleration exit\n");
    return;
}
module_init(mv_xor_init);
module_exit(mv_xor_exit);
MODULE_LICENSE(GPL);



