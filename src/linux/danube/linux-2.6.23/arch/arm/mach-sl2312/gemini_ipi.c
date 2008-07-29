/*
 * FILE NAME sl_cir.c
 *
 * BRIEF MODULE DESCRIPTION
 *  IPI Driver for CPU1.
 *
 *  Author: StorLink, Corp.
 *          Jason Lee
 *
 * Copyright 2002~2006 StorLink, Corp.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMit8712D  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE	LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMit8712D   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, writ8712  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/init.h>
#include <linux/pagemap.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <linux/signal.h>
#include <asm/arch/sl2312.h>
#include <asm/arch/int_ctrl.h>
#include <asm/arch/ipi.h>
#include <linux/dma-mapping.h>


#include <linux/mm.h>

#include <linux/bootmem.h>

#include <asm/hardware.h>
#include <asm/page.h>
#include <asm/setup.h>
#include <asm/pgtable.h>
#include <asm/pgalloc.h>

#include <asm/mach/map.h>


static int sl_ipi_debug = 1 ;
#define DEB(x)  if(sl_ipi_debug>=1) x

#define SRAM_PTR		IO_ADDRESS(SL2312_SRAM_BASE)
volatile JSCALE_REQ_T *req=(JSCALE_REQ_T*)SRAM_PTR;
volatile JSCALE_RSP_T *rsp=(JSCALE_RSP_T*)(SRAM_PTR+0x20);

unsigned int jscale_status=0;

#define JSCALE_WAIT	0
#define XXXXXX_WAIT	1
#define MAX_WAIT_Q	8
wait_queue_head_t gemini_ipi_wait[MAX_WAIT_Q];

#define DRAMCTL_DMA_CTL		0X20
#define DRAMCTL_DMA_SA		0X24
#define DRAMCTL_DMA_DA		0X28
#define DRAMCTL_DMA_CNT		0X2C
#define MEMCPY_UNIT		0x40000
int hw_memcpy(const void *to, const void *from, unsigned int bytes)
{
	unsigned int reg_a,reg_d;
	int count = bytes,i=0;

	consistent_sync((unsigned int *)to, bytes, DMA_BIDIRECTIONAL);
	consistent_sync((unsigned int *)from,bytes, DMA_TO_DEVICE);

	DEB(printk("hwmemcpy:count %d\n",count));
	while(count>0){
		// SA
		reg_a = IO_ADDRESS(SL2312_DRAM_CTRL_BASE)+DRAMCTL_DMA_SA;
		reg_d = (unsigned int )__virt_to_phys(from) + i*MEMCPY_UNIT;
		DEB(printk("hwmemcpy:from 0x%08x\n",reg_d));
		writel(reg_d,reg_a);
		// DA
		reg_a = IO_ADDRESS(SL2312_DRAM_CTRL_BASE)+DRAMCTL_DMA_DA;
		reg_d = (unsigned int )__virt_to_phys(to) + i*MEMCPY_UNIT;
		writel(reg_d,reg_a);
		DEB(printk("hwmemcpy:to 0x%08x\n",reg_d));
		// byte count
		reg_a = IO_ADDRESS(SL2312_DRAM_CTRL_BASE)+DRAMCTL_DMA_CNT;
		reg_d = (count>=MEMCPY_UNIT)?MEMCPY_UNIT:count;
		writel(reg_d,reg_a);
		// start DMA
		reg_a = IO_ADDRESS(SL2312_DRAM_CTRL_BASE)+DRAMCTL_DMA_CTL;
		writel(0x80000001,reg_a);

		do{
			cond_resched();
//			msleep(4);
			reg_d = readl(IO_ADDRESS(SL2312_DRAM_CTRL_BASE)+DRAMCTL_DMA_CTL);
		}while(reg_d&0x1);

		count -= MEMCPY_UNIT;
		i++;
	}

	return bytes;
}

static irqreturn_t ipi_interrupt()
{
	unsigned int id=getcpuid(),tmp;

	//dmac_inv_range(__phys_to_virt(SL2312_SRAM_BASE),__phys_to_virt(SHAREADDR)+0x2000);


	// Clear Interrupt
	if(id==CPU0) {
		tmp = readl(CPU1_STATUS);
		tmp &= ~CPU_IPI_BIT_MASK;
		writel(tmp,CPU1_STATUS);
	}
	else{
		tmp = readl(CPU0_STATUS);
		tmp &= ~CPU_IPI_BIT_MASK;
		writel(tmp,CPU0_STATUS);
	}

	//
	DEB(printk("ipi interrupt:0x%x\n",rsp->status));
	switch(rsp->status){
		case JSCALE_STATUS_OK:

			break;
		case JSCALE_UNKNOWN_MSG_TYPE:

			break;
		case JSCALE_FAILED_FILE_SIZE:

			break;
		case JSCALE_FAILED_MALLOC:

			break;
		case JSCALE_FAILED_FORMAT:

			break;
		case JSCALE_DECODE_ERROR:

			break;

	}
	jscale_status = rsp->status;
//	wake_up(&gemini_ipi_wait[JSCALE_WAIT]);

	return IRQ_HANDLED;
}

static int gemini_ipi_open(struct inode *inode, struct file *file)
{
	DEB(printk("ipi open\n"));
	return 0;
}


static int gemini_ipi_release(struct inode *inode, struct file *file)
{
	DEB(printk("ipi release\n"));
	return 0;
}


static int gemini_ipi_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg)
{
	JSCALE_RSP_T tmp;

	switch(cmd) {
		case GEMINI_IPI_JSCALE_REQ:
			DEB(printk("ipi:ioctl jscale request %dX%d Q:%d\n",req->ScaledImageWidth,req->ScaledImageHeight,req->ScaledImageQuality));
			if (copy_from_user(req, (JSCALE_REQ_T *)arg, sizeof(JSCALE_REQ_T)))
				return -EFAULT;
			req->hdr.type = IPC_JSCALE_REQ_MSG;
			req->hdr.length = sizeof(JSCALE_REQ_T);
			req->input_location  = CPU_1_DATA_OFFSET;
			req->output_location = CPU_1_DATA_OFFSET;
			break;
		case GEMINI_IPI_JSCALE_STAT:
			DEB(printk("ipi:ioctl jscale stat \n"));
			if(jscale_status==JSCALE_BUSY){						// not yet
				tmp.status = JSCALE_BUSY;
				if (copy_to_user((JSCALE_RSP_T *)arg,&tmp, sizeof(JSCALE_RSP_T)))
					return -EFAULT;
			}
			else{												// finish or error
				if (copy_to_user((JSCALE_RSP_T *)arg,rsp, sizeof(JSCALE_RSP_T)))
					return -EFAULT;
			}
			break;
		default:
			printk("IPI: Error IOCTL number\n");
			return -ENOIOCTLCMD;
	}

	return 0;
}

#define SRAM_SIZE	0x2000
static ssize_t gemini_ipi_write(struct file *file_p, const char *buf, size_t count, loff_t * ppos)
{
	int i=0,tmp=0,j;
	const char *ptr=(unsigned int)__phys_to_virt(CPU_1_MEM_BASE+CPU_1_DATA_OFFSET);
	DEB(printk("ipi:write 0x%x to 0x%x length:%d\n",&buf,ptr,count));
	memcpy(ptr,buf,count);
	consistent_sync(ptr,count, DMA_TO_DEVICE);
	//hw_memcpy(ptr,&buf,count);

/*	if(count>SRAM_SIZE){
		for(i=0;i<(count/SRAM_SIZE);i++)
			raid_memcpy(ptr+i*SRAM_SIZE,buf+i*SRAM_SIZE,SRAM_SIZE);
		if(count%SRAM_SIZE)
			raid_memcpy(ptr+i*SRAM_SIZE,buf+i*SRAM_SIZE,count%SRAM_SIZE);
	}
	else
		raid_memcpy(ptr,buf,count);
*/

/*	for(i=0;i<count;i++){
		if(buf[i]!=ptr[i])
			printk("ipi error:offset %d valud %x[should %x]\n",i,ptr[i],buf[i]);
	}

	printk("===========input buf===============\n");
	for(i=0;i<64;i+=16){
		for(j=0;j<16;j++)
			printk("%02x ",buf[i+j]);
		printk("\n");
		cond_resched();
	}
	printk("===========output buf==============\n");
	for(i=0;i<64;i+=16){
		for(j=0;j<16;j++)
			printk("%02x ",ptr[i+j]);
		printk("\n");
		cond_resched();
	}
*/
	// send irq for CPU1
	tmp |= CPU_IPI_BIT_MASK;
	writel(tmp,CPU0_STATUS);
	jscale_status = JSCALE_BUSY;

	return count;
}

static ssize_t gemini_ipi_read(struct file * file_p, char *buf, size_t length, loff_t * ppos)
{
	int i=0;
	const char *ptr=(unsigned int )__phys_to_virt(CPU_1_MEM_BASE+CPU_1_DATA_OFFSET);

	consistent_sync(ptr,length, DMA_FROM_DEVICE);
	memcpy(buf,ptr,length);
	DEB(printk("ipi:read 0x%x to 0x%x length:%d\n",ptr,buf,length));

	//consistent_sync((unsigned int *)ptr,0x2000, DMA_FROM_DEVICE);		// invalid
	//hw_memcpy(buf,ptr,length);

	// need encoded file size ********
/*	if(count>SRAM_SIZE){
		for(i=0;i<(count/SRAM_SIZE);i++)
			raid_memcpy(buf+i*SRAM_SIZE,p_mbox->message+i*SRAM_SIZE,SRAM_SIZE);
		if(count%0xFFFF)
			raid_memcpy(buf+i*SRAM_SIZE,p_mbox->message+i*SRAM_SIZE,length%SRAM_SIZE);
	}
	else
		raid_memcpy(buf,p_mbox->message,length);
*/
	return length;
}

void do_mapping_read(struct address_space *mapping,
			     struct file_ra_state *_ra,
			     struct file *filp,
			     loff_t *ppos,
			     read_descriptor_t *desc,
			     read_actor_t actor)
{
	struct inode *inode = mapping->host;
	unsigned long index;
	unsigned long end_index;
	unsigned long offset;
	unsigned long last_index;
	unsigned long next_index;
	unsigned long prev_index;
	loff_t isize;
	struct page *cached_page;
	int error;
	struct file_ra_state ra = *_ra;

	cached_page = NULL;
	index = *ppos >> PAGE_CACHE_SHIFT;
	next_index = index;
	prev_index = ra.prev_page;
	last_index = (*ppos + desc->count + PAGE_CACHE_SIZE-1) >> PAGE_CACHE_SHIFT;
	offset = *ppos & ~PAGE_CACHE_MASK;

	isize = i_size_read(inode);
	if (!isize)
		goto out;

	end_index = (isize - 1) >> PAGE_CACHE_SHIFT;
	for (;;) {
		struct page *page;
		unsigned long nr, ret;

		/* nr is the maximum number of bytes to copy from this page */
		nr = PAGE_CACHE_SIZE;
		if (index >= end_index) {
			if (index > end_index)
				goto out;
			nr = ((isize - 1) & ~PAGE_CACHE_MASK) + 1;
			if (nr <= offset) {
				goto out;
			}
		}
		nr = nr - offset;

		cond_resched();
		if (index == next_index)
			next_index = page_cache_readahead(mapping, &ra, filp,
					index, last_index - index);

find_page:
		page = find_get_page(mapping, index);
		if (unlikely(page == NULL)) {
			handle_ra_miss(mapping, &ra, index);
			goto no_cached_page;
		}
		if (!PageUptodate(page))
			goto page_not_up_to_date;
page_ok:

		/* If users can be writing to this page using arbitrary
		 * virtual addresses, take care about potential aliasing
		 * before reading the page on the kernel side.
		 */
		if (mapping_writably_mapped(mapping))
			flush_dcache_page(page);

		/*
		 * When (part of) the same page is read multiple times
		 * in succession, only mark it as accessed the first time.
		 */
		if (prev_index != index)
			mark_page_accessed(page);
		prev_index = index;

		/*
		 * Ok, we have the page, and it's up-to-date, so
		 * now we can copy it to user space...
		 *
		 * The actor routine returns how many bytes were actually used..
		 * NOTE! This may not be the same as how much of a user buffer
		 * we filled up (we may be padding etc), so we can only update
		 * "pos" here (the actor routine has to update the user buffer
		 * pointers and the remaining count).
		 */
		ret = actor(desc, page, offset, nr);
		offset += ret;
		index += offset >> PAGE_CACHE_SHIFT;
		offset &= ~PAGE_CACHE_MASK;

		page_cache_release(page);
		if (ret == nr && desc->count)
			continue;
		goto out;

page_not_up_to_date:
		/* Get exclusive access to the page ... */
		lock_page(page);

		/* Did it get unhashed before we got the lock? */
		if (!page->mapping) {
			unlock_page(page);
			page_cache_release(page);
			continue;
		}

		/* Did somebody else fill it already? */
		if (PageUptodate(page)) {
			unlock_page(page);
			goto page_ok;
		}

readpage:
		/* Start the actual read. The read will unlock the page. */
		error = mapping->a_ops->readpage(filp, page);

		if (unlikely(error))
			goto readpage_error;

		if (!PageUptodate(page)) {
			lock_page(page);
			if (!PageUptodate(page)) {
				if (page->mapping == NULL) {
					/*
					 * invalidate_inode_pages got it
					 */
					unlock_page(page);
					page_cache_release(page);
					goto find_page;
				}
				unlock_page(page);
				error = -EIO;
				goto readpage_error;
			}
			unlock_page(page);
		}

		/*
		 * i_size must be checked after we have done ->readpage.
		 *
		 * Checking i_size after the readpage allows us to calculate
		 * the correct value for "nr", which means the zero-filled
		 * part of the page is not copied back to userspace (unless
		 * another truncate extends the file - this is desired though).
		 */
		isize = i_size_read(inode);
		end_index = (isize - 1) >> PAGE_CACHE_SHIFT;
		if (unlikely(!isize || index > end_index)) {
			page_cache_release(page);
			goto out;
		}

		/* nr is the maximum number of bytes to copy from this page */
		nr = PAGE_CACHE_SIZE;
		if (index == end_index) {
			nr = ((isize - 1) & ~PAGE_CACHE_MASK) + 1;
			if (nr <= offset) {
				page_cache_release(page);
				goto out;
			}
		}
		nr = nr - offset;
		goto page_ok;

readpage_error:
		/* UHHUH! A synchronous read error occurred. Report it */
		desc->error = error;
		page_cache_release(page);
		goto out;

no_cached_page:
		/*
		 * Ok, it wasn't cached, so we need to create a new
		 * page..
		 */
		if (!cached_page) {
			cached_page = page_cache_alloc_cold(mapping);
			if (!cached_page) {
				desc->error = -ENOMEM;
				goto out;
			}
		}
		error = add_to_page_cache_lru(cached_page, mapping,
						index, GFP_KERNEL);
		if (error) {
			if (error == -EEXIST)
				goto find_page;
			desc->error = error;
			goto out;
		}
		page = cached_page;
		cached_page = NULL;
		goto readpage;
	}

out:
	*_ra = ra;

	*ppos = ((loff_t) index << PAGE_CACHE_SHIFT) + offset;
	if (cached_page)
		page_cache_release(cached_page);
	if (filp)
		file_accessed(filp);
}

int ipi_send_actor(read_descriptor_t * desc, struct page *page, unsigned long offset, unsigned long size)
{
	ssize_t written;
	unsigned long count = desc->count;
	struct file *file = desc->arg.data;
	unsigned int *ptr_to=(unsigned int)__phys_to_virt(CPU_1_MEM_BASE+CPU_1_DATA_OFFSET) + desc->written;
	void *ptr_from;

	if (size > count)
		size = count;

	ptr_from = page_address(page)+offset;
	written = memcpy(ptr_to,ptr_from,size);

	if (written < 0) {
		desc->error = written;
		written = 0;
	}
	desc->count = count - written;
	desc->written += written;
	return written;
}

ssize_t gemini_ipi_sendfile(struct file *in_file, loff_t *ppos,
			 size_t count, read_actor_t actor, void *TARGET)
{
	read_descriptor_t desc;

	if (!count)
		return 0;

	desc.written = 0;
	desc.count = count;
	desc.arg.data = TARGET;
	desc.error = 0;

	do_mapping_read(in_file->f_mapping,&in_file->f_ra,in_file, ppos, &desc, ipi_send_actor);

	if (desc.written)
		return desc.written;
	return desc.error;
}
static struct file_operations gemini_ipi_fops = {
	.owner	=	THIS_MODULE,
	.ioctl	=	gemini_ipi_ioctl,
	.open	=	gemini_ipi_open,
	.release=	gemini_ipi_release,
	.write	=	gemini_ipi_write,
	.read	=	gemini_ipi_read,
	.sendfile = gemini_ipi_sendfile,
};

#ifndef STORLINK_IPI
#define STORLINK_IPI	242		// Documents/devices.txt suggest to use 240~255 for local driver!!
#endif

static struct miscdevice gemini_ipi_miscdev =
{
	STORLINK_IPI,
	"slave_ipc",
	&gemini_ipi_fops
};

int __init sl_ipi_init(void)
{

	printk("Gemini IPI Driver Initialization...\n");
	printk("REQ Head :0x%x(phy:0x%x)\n",(unsigned int)req,(unsigned int)SL2312_SRAM_BASE);
	printk("RSP Head :0x%x(phy:0x%x)\n",(unsigned int)rsp,(unsigned int)SL2312_SRAM_BASE+0x20);
	printk("Data buff:0x%x(phy:0x%x)\n",__phys_to_virt(CPU_1_MEM_BASE+CPU_1_DATA_OFFSET),CPU_1_MEM_BASE+CPU_1_DATA_OFFSET);

	misc_register(&gemini_ipi_miscdev);

	if (request_irq(IRQ_CPU0_IP_IRQ_OFFSET, ipi_interrupt, SA_INTERRUPT, "ipi", NULL))
		printk("Error: Register IRQ for Storlink IPI failed\n");

	return 0;
}

void __exit sl_ipi_exit(void)
{

}

module_init(sl_ipi_init);
module_exit(sl_ipi_exit);

MODULE_AUTHOR("Jason Lee <jason@storlink.com.tw>");
MODULE_DESCRIPTION("Storlink IPI driver");
MODULE_LICENSE("GPL");
