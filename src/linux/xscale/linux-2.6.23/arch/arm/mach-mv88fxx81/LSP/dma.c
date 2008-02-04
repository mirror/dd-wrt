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
#include <linux/mm.h>
#include <asm/mach/time.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>

#include "mvIdma.h"

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

#ifdef RT_DEBUG
static int dma_wait_loops = 0;
#endif

#define IDMA_MIN_COPY_CHUNK 128
unsigned long mv_dma_min_buffer = IDMA_MIN_COPY;
static unsigned long dma_copy(void *to, const void *from, unsigned long n, unsigned int to_user);


static inline u32 page_remainder(u32 virt)
{
	return PAGE_SIZE - (virt & ~PAGE_MASK);
}


static inline int is_kernel_static(u32 virt)
{
	return((virt >= PAGE_OFFSET) && (virt < (unsigned long)high_memory));
}

/*
 * map a kernel virtual address or kernel logical address to a phys address
 */
static inline u32 physical_address(u32 virt, int write)
{
    struct page *page;
       /* kernel static-mapped address */
    DPRINTK(" get physical address: virt %x , write %d\n", virt, write);
    if (is_kernel_static(virt)) 
    {
        return __pa((u32) virt);
    }
    if (virt >= TASK_SIZE)
    {
	struct vm_area_struct *vma = find_vma(&init_mm, virt);
        page = follow_page(vma, (u32) virt, write);
    }
    else
    {
	struct vm_area_struct *vma = find_vma(current->mm, virt);
        page = follow_page(vma, (u32) virt, write);
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

void
hexdump(char *p, int n)
{
        int i, off;

        for (off = 0; n > 0; off += 16, n -= 16) {
                printk("%s%04x:", off == 0 ? "\n" : "", off);
                i = (n >= 16 ? 16 : n);
                do {
                        printk(" %02x", *p++ & 0xff);
                } while (--i);
                printk("\n");
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
	DPRINTK("IDMA complete in %x cause %x \n", timeout, temp);


#if 0
	/* wait for IDMA to complete */
	temp = MV_REG_READ(IDMA_CAUSE_REG);
	DPRINTK("wait for IDMA chan %d to complete \n", channel);
	while ((temp & (0xf << (8 * channel))) == 0)
	{
		DPRINTK("cause is %x , ctrl low is %x \n", temp, MV_REG_READ(IDMA_CTRL_LOW_REG(channel)));
		//udelay(1);
#ifdef RT_DEBUG
                dma_wait_loops++; 
#endif
		if(timeout++ > CPY_DMA_TIMEOUT)
                {
		    printk("dma_copy: IDMA %d timed out cause %x, ctrl low is %x \n",
                    channel, temp, MV_REG_READ(IDMA_CTRL_LOW_REG(channel)));
                    return 1;
                }
		temp = MV_REG_READ(IDMA_CAUSE_REG);
		
	}
	DPRINTK("IDMA complete in %x cause %x \n", timeout, temp);
	/* check if IDMA finished without errors */
	if((temp & (0x1e << (8 * channel))) != 0)
	{
	    printk("IDMA channel %d finished with error %x \n",channel, temp);
            return 1;
	}
	/* clear the cause reg */
	MV_REG_BYTE_WRITE(IDMA_CAUSE_REG + channel, 0xF0);
#endif

	return 0;

}

#if 0

unsigned int wait_for_idma(MV_U32   channel)
{
	u32 temp;
	u32 timeout = 0;

	/* wait for IDMA to complete */
	temp = MV_REG_READ(IDMA_CAUSE_REG);
	DPRINTK("wait for IDMA chan %d to complete \n", channel);
	while ((temp & (0xf << (8 * channel))) == 0)
	{
		DPRINTK("cause is %x , ctrl low is %x \n", temp, MV_REG_READ(IDMA_CTRL_LOW_REG(channel)));
		//udelay(1);
#ifdef RT_DEBUG
                dma_wait_loops++; 
#endif
		if(timeout++ > CPY_DMA_TIMEOUT)
                {
		    printk("dma_copy: IDMA %d timed out cause %x, ctrl low is %x \n",
                    channel, temp, MV_REG_READ(IDMA_CTRL_LOW_REG(channel)));
                    return 1;
                }
		temp = MV_REG_READ(IDMA_CAUSE_REG);
		
	}
	DPRINTK("IDMA complete in %x cause %x \n", timeout, temp);
	/* check if IDMA finished without errors */
	if((temp & (0x1e << (8 * channel))) != 0)
	{
	    printk("IDMA channel %d finished with error %x \n",channel, temp);
            return 1;
	}
	/* clear the cause reg */
	MV_REG_BYTE_WRITE(IDMA_CAUSE_REG + channel, 0xF0);

	return 0;

}
#endif
static struct proc_dir_entry *dma_proc_entry;
static int dma_to_user = 0;
static int dma_from_user = 0;
#ifdef RT_DEBUG
static int dma_activations = 0;
#endif
static int dma_read_proc(char *, char **, off_t, int, int *, void *);

static int dma_read_proc(char *buf, char **start, off_t offset, int len,
						 int *eof, void *data)
{
	len = 0;
        len += sprintf(buf + len, "DMA min buffer %ld\n", mv_dma_min_buffer);
	len += sprintf(buf + len, "Number of DMA copy to user %d copy from user %d \n", dma_to_user, dma_from_user);
#ifdef RT_DEBUG
	len += sprintf(buf + len, "Number of dma activations %d\n", dma_activations);
	len += sprintf(buf + len, "Number of wait for dma loops %d\n", dma_wait_loops);
#endif
        
	return len;
}
#if 0
static int dma_write_proc(struct file *file, const char __user *buf,
			   unsigned long count, void *data)
{
       char *buffer, *p;
        int err;
        unsigned long min_buffer_len;

        if (!buf || count > PAGE_SIZE)
                return -EINVAL;

        buffer = (char *)__get_free_page(GFP_KERNEL);
        if (!buffer)
                return -ENOMEM;

        err = -EFAULT;
        if (copy_from_user(buffer, buf, count))
                goto out;

        err = -EINVAL;
        if (count < PAGE_SIZE)
                buffer[count] = '\0';
        else if (buffer[PAGE_SIZE-1])
                goto out;

        /*
         * Usage: echo "read min len n" >/proc/dma_copy
         */
        if (!strncmp("read min len", buffer, 12)) {
            p = buffer + 13;
            min_buffer_len = simple_strtoul(p, &p, 0);
            if ( min_buffer_len < 64)
            {
                printk(" min len must be >= 64\n");
                err = -EINVAL;
                goto out;
            }
            printk("DMA COPY: set new value for min read buffer to %ld\n", min_buffer_len);
            mv_dma_min_buffer = min_buffer_len;
        }
#if 0
        /*
         * Usage: echo "read min len n" >/proc/dma_copy
         */
        } else if (!strncmp("scsi remove-single-device", buffer, 25)) {
                p = buffer + 26;

                host = simple_strtoul(p, &p, 0);
                channel = simple_strtoul(p + 1, &p, 0);
                id = simple_strtoul(p + 1, &p, 0);
                lun = simple_strtoul(p + 1, &p, 0);

                err = scsi_remove_single_device(host, channel, id, lun);
        }
#endif
 out:
        free_page((unsigned long)buffer);
        return err;

}
#endif
#if 0

/*=======================================================================*/
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
/*              Needs to be able to handle overlapping regions           */
/*              correctly.                                               */
/*                                                                       */
/*=======================================================================*/
void * dma_memcpy(void *to, const void *from, __kernel_size_t n)
{
	DPRINTK("dma_memcpy(0x%x, 0x%x, %lu): entering\n", (u32) to, (u32) from, (unsigned long) n);

	return asm_memcpy(to, from, n);
}
#endif

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
	dma_from_user++;
	DPRINTK(KERN_CRIT "dma_copy_from_user(0x%x, 0x%x, %lu): entering\n", (u32) to, (u32) from, n);
	return  dma_copy(to, from, n, 0);
}


/*
 * n must be greater equal than 64.
 */
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
			__copy_to_user(to, from, 32 - unaligned_to);
		else
			__copy_from_user(to, from, 32 - unaligned_to);

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
			__copy_to_user((void *)tmp_to, (void *)tmp_from, unaligned_to);
		else
			__copy_from_user((void *)tmp_to, (void *)tmp_from, unaligned_to);

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
        if(is_kernel_static(kaddr))
        {
            kaddr_kernel_static = 1;
            k_chunk = n;
        }
         
        spin_lock_irqsave(&current->mm->page_table_lock, flags);
     
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
        	if(to_user)
	       	    __copy_to_user((void *)to, (void *)from, chunk);
	        else
		    __copy_from_user((void *)to, (void *)from, chunk);
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
	        mvOsCacheFlush(NULL, (void *)from, chunk);
	        mvOsCacheInvalidate(NULL, (void *)to, chunk);
               
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
          spin_unlock_irqrestore(&current->mm->page_table_lock, flags);

        
        DPRINTK("dma_copy(0x%x, 0x%x, %lu): exiting\n", (u32) to,
                (u32) from, n);
       

        if(n != 0)
        {
       	    if(to_user)
                return __copy_to_user((void *)to, (void *)from, n);
	            else
                return __copy_from_user((void *)to, (void *)from, n);
        }
        return 0;
}


int mv_dma_init(void)
{
	printk(KERN_INFO "use IDMA acceleration in copy to/from user buffers. used channels %d and %d \n",
                CPY_CHAN1, CPY_CHAN2);

        MV_REG_WRITE(IDMA_BYTE_COUNT_REG(CPY_CHAN1), 0);
        MV_REG_WRITE(IDMA_CURR_DESC_PTR_REG(CPY_CHAN1), 0);
        MV_REG_WRITE(IDMA_CTRL_HIGH_REG(CPY_CHAN1), ICCHR_ENDIAN_LITTLE | ICCHR_DESC_BYTE_SWAP_EN);
        MV_REG_WRITE(IDMA_CTRL_LOW_REG(CPY_CHAN1), CPY_IDMA_CTRL_LOW_VALUE);

        MV_REG_WRITE(IDMA_BYTE_COUNT_REG(CPY_CHAN2), 0);
        MV_REG_WRITE(IDMA_CURR_DESC_PTR_REG(CPY_CHAN2), 0);
        MV_REG_WRITE(IDMA_CTRL_HIGH_REG(CPY_CHAN2), ICCHR_ENDIAN_LITTLE | ICCHR_DESC_BYTE_SWAP_EN);
        MV_REG_WRITE(IDMA_CTRL_LOW_REG(CPY_CHAN2), CPY_IDMA_CTRL_LOW_VALUE);

        current_dma_channel = CPY_CHAN1;
	dma_proc_entry = create_proc_entry("dma_copy", S_IFREG | S_IRUGO, 0);
	dma_proc_entry->read_proc = dma_read_proc;
//	dma_proc_entry->write_proc = dma_write_proc;
	dma_proc_entry->nlink = 1;

	printk(KERN_INFO "Done. \n");
	return 0;
}

void mv_dma_exit(void)
{
}

module_init(mv_dma_init);
module_exit(mv_dma_exit);
MODULE_LICENSE(GPL);

EXPORT_SYMBOL(dma_copy_to_user);
EXPORT_SYMBOL(dma_copy_from_user);			
