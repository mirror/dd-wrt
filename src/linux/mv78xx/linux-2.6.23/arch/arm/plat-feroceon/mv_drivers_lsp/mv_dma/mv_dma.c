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

#include "idma/mvIdma.h"

#undef DEBUG
//#define DEBUG

#ifdef DEBUG
	#define DPRINTK(s, args...)  printk("MV_DMA: " s, ## args)
#else
	#define DPRINTK(s, args...)
#endif

#undef CPY_USE_DESC
#undef RT_DEBUG
#define RT_DEBUG

#define CPY_IDMA_CTRL_LOW_VALUE      ICCLR_DST_BURST_LIM_128BYTE   \
                                    | ICCLR_SRC_BURST_LIM_128BYTE   \
                                    | ICCLR_INT_MODE_MASK           \
                                    | ICCLR_BLOCK_MODE              \
                                    | ICCLR_CHAN_ENABLE             \
                                    | ICCLR_DESC_MODE_16M

#define CPY_CHAN1	2
#define CPY_CHAN2	3

#define CPY_DMA_TIMEOUT	0x100000

#define NEXT_CHANNEL(channel) ((CPY_CHAN1 + CPY_CHAN2) - (channel))
#define PREV_CHANNEL(channel) NEXT_CHANNEL(channel)
static MV_U32 current_dma_channel =  CPY_CHAN1;
static int idma_init = 0;
static int idma_busy = 0;

#ifdef RT_DEBUG
static int dma_wait_loops = 0;
#endif

#ifdef CONFIG_MV_IDMA_MEMZERO
#define	DMA_MEMZERO_CHUNK    0x80 /*128*/    /* this is based on the size of the DST and SRC burst limits */
static MV_U8	dmaMemInitBuff[DMA_MEMZERO_CHUNK] __attribute__(( aligned(128) ));
#endif

#define IDMA_MIN_COPY_CHUNK CONFIG_MV_IDMA_COPYUSER_THRESHOLD

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
       /* kernel static-mapped address */
    DPRINTK(" get physical address: virt %x , write %d\n", virt, write);
    if (virt_addr_valid(virt)) 
    {
        return __pa((u32) virt);
    }
    if (virt >= high_memory)
	    return 0;
    
    if (virt >= TASK_SIZE)
    {
        page = follow_page(find_extend_vma(&init_mm, virt), (u32) virt, write);
    }
    else
    {
        page = follow_page(find_extend_vma(current->mm, virt), (u32) virt, write);
    }
    
    if (pfn_valid(page_to_pfn(page)))
    {
        return ((page_to_pfn(page) << PAGE_SHIFT) |
                       ((u32) virt & (PAGE_SIZE - 1)));
    }
    else
    {
        return 0;
    }
}

unsigned int wait_for_idma(MV_U32   channel)
{
	u32 timeout = 0;

	/* wait for completion */
        while( mvDmaStateGet(channel) != MV_IDLE )
	{
		DPRINTK(" ctrl low is %x \n", MV_REG_READ(IDMA_CTRL_LOW_REG(channel)));
		//udelay(1);
#ifdef RT_DEBUG
                dma_wait_loops++; 
#endif
		if(timeout++ > CPY_DMA_TIMEOUT)
                {
		    printk("dma_copy: IDMA %d timed out , ctrl low is %x \n",
                    channel, MV_REG_READ(IDMA_CTRL_LOW_REG(channel)));
                    return 1;
                }		
	}
	DPRINTK("IDMA complete in %x \n", timeout);
	return 0;
}

static struct proc_dir_entry *dma_proc_entry;
static int dma_to_user = 0;
static int dma_from_user = 0;
static int dma_memcpy_cnt = 0;
static int dma_memzero_cnt = 0;
#ifdef RT_DEBUG
static int dma_activations = 0;
#endif
static int dma_read_proc(char *, char **, off_t, int, int *, void *);

static int dma_read_proc(char *buf, char **start, off_t offset, int len,
						 int *eof, void *data)
{
	len = 0;
#ifdef CONFIG_MV_IDMA_COPYUSER
        len += sprintf(buf + len, "DMA min buffer size for copy to/from user %d\n", COPYUSER_MIN_SIZE);
#endif
#ifdef CONFIG_MV_IDMA_MEMCOPY
	len += sprintf(buf + len, "DMA min buffer size for memcpy and memmove %d\n", CONFIG_MV_IDMA_MEMCOPY_THRESHOLD);
#endif
#ifdef CONFIG_MV_IDMA_MEMZERO
	len += sprintf(buf + len, "DMA min buffer size for memzero %d\n", CONFIG_MV_IDMA_MEMZERO_THRESHOLD);
#endif
	len += sprintf(buf + len, "Number of DMA copy to user %d copy from user %d \n", dma_to_user, dma_from_user);
	len += sprintf(buf + len, "Number of DMA memzero %d \n", dma_memzero_cnt);
	len += sprintf(buf + len, "Number of DMA memcpy %d \n", dma_memcpy_cnt);
#ifdef RT_DEBUG
	len += sprintf(buf + len, "Number of dma activations %d\n", dma_activations);
	len += sprintf(buf + len, "Number of wait for dma loops %d\n", dma_wait_loops);
#endif
        
	return len;
}
#ifdef CONFIG_MV_IDMA_COPYUSER
/*=======================================================================*/
/*  Procedure:  dma_copy()                                               */
/*                                                                       */
/*  Description:    DMA-based copy_to_user.                              */
/*                                                                       */
/*  Parameters:  to: destination address                                 */
/*               from: source address                                    */
/*               n: number of bytes to transfer (n must be greater       */
/*                   equal than 64)                                     */
/*		 to_user: (1) Copy TO user (0) Copy FROM user	 	 */
/*                                                                       */
/*  Returns:     unsigned long: number of bytes NOT copied               */
/*                                                                       */
/*=======================================================================*/
static unsigned long dma_copy(void *to, const void *from, unsigned long n, unsigned int to_user)
{
	u32 chunk,i;
	u32 k_chunk = 0;
	u32 u_chunk = 0;
	u32 phys_from, phys_to;
	
        unsigned long flags;
	u32 unaligned_to;
	u32 index = 0;
        u32 temp;

        unsigned long uaddr, kaddr;
        unsigned char kaddr_kernel_static = 0;
	DPRINTK("dma_copy: entering\n");


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
			__arch_copy_to_user(to, from, 32 - unaligned_to);
		else
			__arch_copy_from_user(to, from, 32 - unaligned_to);

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
			__arch_copy_to_user((void *)tmp_to, (void *)tmp_from, unaligned_to);
		else
			__arch_copy_from_user((void *)tmp_to, (void *)tmp_from, unaligned_to);

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
            kaddr_kernel_static = 1;
            k_chunk = n;
        }
	else
	{
		DPRINTK("kernel address is not linear, fall back\n");
		goto exit_dma;
	}
         
        spin_lock_irqsave(&current->mm->page_table_lock, flags);
	if (idma_busy)
	{
	    BUG();
	}
	idma_busy = 1;
     
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
	    /*
	     *  Prepare the IDMA.
	     */
            if (chunk < IDMA_MIN_COPY_CHUNK)
            {
        	DPRINTK(" chunk %d too small , use memcpy \n",chunk);
                /* the "to" address might cross cache line boundary, so part of the line*/  
                /* may be subject to DMA, so we need to wait to last DMA engine to finish */
                if (index > 0)
                {
                    if(wait_for_idma(PREV_CHANNEL(current_dma_channel)))
                    {
	                BUG();
                    }
                }
                
        	if(to_user)
	       	    __arch_copy_to_user((void *)to, (void *)from, chunk);
	        else
		    __arch_copy_from_user((void *)to, (void *)from, chunk);
            }
            else if ((!phys_from) || (!phys_to))
            {
                /* The requested page isn't available, fall back to */
                DPRINTK(" no physical address, fall back: from %p , to %p \n", from, to);
                goto wait_for_idmas;
   
            }
            else
            {
                /* 
	 	 * Ensure that the cache is clean:
	 	 *      - from range must be cleaned
        	 *      - to range must be invalidated
	         */
		dmac_flush_range(from, from + chunk);
		dmac_inv_range(to, to + chunk);
               
               	    if(index > 1)
		    {
		        if(wait_for_idma(current_dma_channel))
                        {
		            BUG(); 
                            goto unlock_dma;
                        }
                    }
		    /* Start DMA */
                    DPRINTK(" activate DMA: channel %d from %x to %x len %x\n",
                            current_dma_channel, phys_from, phys_to, chunk);
		    mvDmaTransfer(current_dma_channel, phys_from, phys_to, chunk, 0);
                    current_dma_channel = NEXT_CHANNEL(current_dma_channel); 
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
        
wait_for_idmas:
        if (index > 1)
        {
	    if(wait_for_idma(current_dma_channel))
            {
	        BUG(); 
            }
        }

        if (index > 0)
        {
            if(wait_for_idma(PREV_CHANNEL(current_dma_channel)))
            {
	        BUG();
            }
        }

unlock_dma:    
	idma_busy = 0;    
        spin_unlock_irqrestore(&current->mm->page_table_lock, flags);
 exit_dma:
        
        DPRINTK("dma_copy(0x%x, 0x%x, %lu): exiting\n", (u32) to,
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

/*=======================================================================*/
/*  Procedure:  dma_copy_to_user()                                       */
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
unsigned long dma_copy_to_user(void *to, const void *from, unsigned long n)
{
	if(!idma_init)
    		return __arch_copy_to_user((void *)to, (void *)from, n);

     	dma_to_user++;
     	DPRINTK(KERN_CRIT "dma_copy_to_user(%#10x, 0x%#10x, %lu): entering\n", (u32) to, (u32) from, n);
    
        return  dma_copy(to, from, n, 1);
}

/*=======================================================================*/
/*  Procedure:  dma_copy_from_user()                                     */
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
unsigned long dma_copy_from_user(void *to, const void *from, unsigned long n)
{
	if(!idma_init)
		return __arch_copy_from_user((void *)to, (void *)from, n);

	dma_from_user++;
	DPRINTK(KERN_CRIT "dma_copy_from_user(0x%x, 0x%x, %lu): entering\n", (u32) to, (u32) from, n);
	return  dma_copy(to, from, n, 0);
}

#endif /* CONFIG_MV_IDMA_COPYUSER */

#ifdef CONFIG_MV_IDMA_MEMZERO
/*=======================================================================*/
/*  Procedure:  dma_memzero()                                             */
/*                                                                       */
/*  Description:    DMA-based in-kernel memzero.                          */
/*                                                                       */
/*  Parameters:  to: destination address                                 */
/*               n: number of bytes to transfer                          */
/*                                                                       */
/*  Notes/Assumptions:                                                   */
/*              Assumes that kernel physical memory is contiguous, i.e., */
/*              the physical addresses of contiguous virtual addresses   */
/*              are also contiguous.                                     */
/*              Assumes that kernel memory doesn't get paged.            */
/*              The DMA is polling                                       */
/*                                                                       */
/*=======================================================================*/
void dma_memzero(void *to, __kernel_size_t n)
{
	u32 phys_from, phys_to;	
	u32 unaligned_to;
	unsigned long flags;	

	DPRINTK("dma_memcopy: entering\n");

	/* This is used in the very early stages */
	if(!idma_init)
    		return asm_memzero(to ,n);

	/* Fallback for the case that one or both buffers are not physically contiguous  */
	if(!virt_addr_valid(to))
        {
		DPRINTK("Failing back to asm_memzero because of limitations\n");
            return asm_memzero(to ,n);
        }	

	++dma_memzero_cnt;	

	/*
	 * If buffer start addr is not cache line-aligned, so we need to make it so.
	 */
	unaligned_to = (u32)to & 31;
	if(unaligned_to)
	{
		DPRINTK("Fixing up starting address %d bytes\n", 32 - unaligned_to);

		asm_memzero(to, 32 - unaligned_to);

		to = (void*)((u32)to + (32 - unaligned_to));

                /*it's ok, n supposed to be greater than 32 bytes at this point*/
		n -= (32 - unaligned_to);
	}	

	/*
	 * If buffer end addr is not cache line-aligned, so we need to make it so.
	 */
	unaligned_to = ((u32)to + n) & 31;
	if(unaligned_to)
	{	
		u32 tmp_to = (u32)to + (n - unaligned_to);
		DPRINTK("Fixing ending alignment %d bytes\n", unaligned_to);

		asm_memzero((void *)tmp_to, unaligned_to);

                /*it's ok, n supposed to be greater than 32 bytes at this point*/
		n -= unaligned_to;
	}

	phys_from = physical_address((u32)dmaMemInitBuff, 0);
        phys_to = physical_address((u32)to, 1);

	/*
	 *  Prepare the IDMA.
	 */
	if ((!phys_from) || (!phys_to))
        {
	    /* The requested page isn't available, fall back to */
            DPRINTK(" no physical address, fall back: to %p \n", to);
            return asm_memzero(to,n);
        }

        spin_lock_irqsave(&current->mm->page_table_lock, flags);
	if (idma_busy)
	{
	    BUG();
	}
	idma_busy = 1;

	/* Ensure that the destination revion is invalidated */
	mvOsCacheInvalidate(NULL, (void *)to, n);
	
	/* Start DMA */
        DPRINTK(" activate DMA: channel %d from %x with source hold to %x len %x\n",CPY_CHAN1, phys_from, phys_to, n);
     	mvDmaMemInit(CPY_CHAN1, phys_from, phys_to, n);
	
#ifdef RT_DEBUG
	dma_activations++;
#endif
        
	if(wait_for_idma(CPY_CHAN1))
        {
	    BUG(); 
	}	

        DPRINTK("dma_memzero(0x%x, %lu): exiting\n", (u32) to, n);

	idma_busy = 0;
	spin_unlock_irqrestore(&current->mm->page_table_lock, flags);
}
#endif  /* CONFIG_MV_IDMA_MEMZERO */

#ifdef CONFIG_MV_IDMA_MEMCOPY
//*=======================================================================*/
/*  Procedure:  dma_memcpy()                                             */
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
/*		source and destination buffers can overlap(like memmove) */
/*                                                                       */
/*=======================================================================*/
void *dma_memcpy(void *to, const void *from, __kernel_size_t n)
{
	u32 phys_from, phys_to;	
	u32 unaligned_to;
	unsigned long flags;

	DPRINTK("dma_memcopy: entering\n");

	/* This is used in the very early stages */
	if(!idma_init)
    		return asm_memmove(to, from,n);

	/* Fallback for the case that one or both buffers are not physically contiguous  */
	if(!virt_addr_valid(to) || !virt_addr_valid(from))
        {
		DPRINTK("Failing back to asm_memmove because of limitations\n");
            return asm_memmove(to,from,n);
        }	

	/* Check for Overlap */
	if (((to + n > from) && (to < from)) ||((from < to) && (from + n > to))) 
	{
		DPRINTK("overlapping copy region (0x%x, 0x%x, %lu), falling back\n",
		     to, from, (unsigned long)n);
		return asm_memmove(to, from, n);
	}

	++dma_memcpy_cnt;

	/*
	 * Ok, start addr is not cache line-aligned, so we need to make it so.
	 */
	unaligned_to = (u32)to & 31;
	if(unaligned_to)
	{
		DPRINTK("Fixing up starting address %d bytes\n", 32 - unaligned_to);

		asm_memmove(to, from, 32 - unaligned_to);

		to = (void*)((u32)to + (32 - unaligned_to));
		from = (void*)((u32)from + (32 - unaligned_to));

                /*it's ok, n supposed to be greater than 32 bytes at this point*/
		n -= (32 - unaligned_to);
	}	

        spin_lock_irqsave(&current->mm->page_table_lock, flags);
	if (idma_busy)
	{
	    BUG();
	}
	idma_busy = 1;
     
        phys_from = physical_address((u32)from, 0);
        phys_to = physical_address((u32)to, 1);
	
    	/*
	 *  Prepare the IDMA.
	 */
	if ((!phys_from) || (!phys_to))
        {
	    /* The requested page isn't available, fall back to */
            DPRINTK(" no physical address, fall back: from %p , to %p \n", from, to);
	    idma_busy = 0;
	    spin_unlock_irqrestore(&current->mm->page_table_lock, flags);
            return asm_memmove(to, from,n);
        }
        else
        {
	    /* 
	     * Ensure that the cache is clean:
	     *      - from range must be cleaned
	     *      - to range must be invalidated
	     */
		dmac_flush_range(from, from + n);
		dmac_inv_range(to, to + n);

               
	    /* Start DMA */
            DPRINTK(" activate DMA: channel %d from %x to %x len %x\n",CPY_CHAN1, phys_from, phys_to, n);
	    mvDmaTransfer(CPY_CHAN1, phys_from, phys_to, n, 0);
#ifdef RT_DEBUG
                    dma_activations++;
#endif
	}
        
	if(wait_for_idma(CPY_CHAN1))
        {
	    BUG(); 
	}	
        
        DPRINTK("dma_memcopy(0x%x, 0x%x, %lu): exiting\n", (u32) to, (u32) from, n);

	idma_busy = 0;
	spin_unlock_irqrestore(&current->mm->page_table_lock, flags);
       
        return 0;
}

#endif  /* CONFIG_MV_IDMA_MEMCOPY */

int mv_dma_init(void)
{
#ifdef CONFIG_MV78200
	if (MV_FALSE == mvSocUnitIsMappedToThisCpu(IDMA))
	{
		printk(KERN_INFO"IDMA is not mapped to this CPU\n");
		return -ENODEV;
	}
#endif
	printk(KERN_INFO "Use IDMA channels %d and %d for enhancing the following function:\n",
                CPY_CHAN1, CPY_CHAN2);
#ifdef CONFIG_MV_IDMA_COPYUSER
        printk(KERN_INFO "  o Copy From/To user space operations.\n");
#endif
#ifdef CONFIG_MV_IDMA_MEMCOPY
	printk(KERN_INFO "  o memcpy() and memmove() operations.\n");
#endif
#ifdef CONFIG_MV_IDMA_MEMZERO
	printk(KERN_INFO "  o memzero() operations.\n");
#endif

#ifdef CONFIG_MV_IDMA_MEMZERO
	DPRINTK(KERN_ERR "ZERO buffer address 0x%08x\n", (u32)dmaMemInitBuff);
	
	asm_memzero(dmaMemInitBuff, sizeof(dmaMemInitBuff));
	dmac_flush_range(dmaMemInitBuff, dmaMemInitBuff + sizeof(dmaMemInitBuff));
#endif

        MV_REG_WRITE(IDMA_BYTE_COUNT_REG(CPY_CHAN1), 0);
        MV_REG_WRITE(IDMA_CURR_DESC_PTR_REG(CPY_CHAN1), 0);
        MV_REG_WRITE(IDMA_CTRL_HIGH_REG(CPY_CHAN1), ICCHR_ENDIAN_LITTLE 
#ifdef MV_CPU_LE
      		| ICCHR_DESC_BYTE_SWAP_EN
#endif
		 );
        MV_REG_WRITE(IDMA_CTRL_LOW_REG(CPY_CHAN1), CPY_IDMA_CTRL_LOW_VALUE);

        MV_REG_WRITE(IDMA_BYTE_COUNT_REG(CPY_CHAN2), 0);
        MV_REG_WRITE(IDMA_CURR_DESC_PTR_REG(CPY_CHAN2), 0);
        MV_REG_WRITE(IDMA_CTRL_HIGH_REG(CPY_CHAN2), ICCHR_ENDIAN_LITTLE 
#ifdef MV_CPU_LE
      		| ICCHR_DESC_BYTE_SWAP_EN
#endif
		 );
        MV_REG_WRITE(IDMA_CTRL_LOW_REG(CPY_CHAN2), CPY_IDMA_CTRL_LOW_VALUE);

        current_dma_channel = CPY_CHAN1;
	dma_proc_entry = create_proc_entry("dma_copy", S_IFREG | S_IRUGO, 0);
	dma_proc_entry->read_proc = dma_read_proc;
//	dma_proc_entry->write_proc = dma_write_proc;
	dma_proc_entry->nlink = 1;

	idma_init = 1;

	return 0;
}

void mv_dma_exit(void)
{
}

module_init(mv_dma_init);
module_exit(mv_dma_exit);
MODULE_LICENSE(GPL);

#ifdef CONFIG_MV_IDMA_MEMCOPY
EXPORT_SYMBOL(dma_memcpy);
#endif

#ifdef CONFIG_MV_IDMA_MEMZERO
EXPORT_SYMBOL(dma_memzero);
#endif

#ifdef CONFIG_MV_IDMA_COPYUSER
EXPORT_SYMBOL(dma_copy_to_user);
EXPORT_SYMBOL(dma_copy_from_user);			
#endif
