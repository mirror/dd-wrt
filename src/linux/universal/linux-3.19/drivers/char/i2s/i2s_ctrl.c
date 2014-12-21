#include <linux/init.h>
#include <linux/version.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <asm/system.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <asm/rt2880/surfboardint.h>
#include <linux/pci.h>

#include "i2s_ctrl.h"
#include "audiohw.h"
#include "../ralink_gdma.h"
#include "../ralink_gpio.h"

/* external functions declarations */
extern void audiohw_set_frequency(int fsel);

/* internal functions declarations */
irqreturn_t i2s_irq_isr(int irq, void *irqaction);
void i2s_dma_handler(u32 dma_ch);
void i2s_unmask_handler(u32 dma_ch);

/* global varable definitions */
i2s_config_type* pi2s_config;
i2s_status_type* pi2s_status;

static dma_addr_t i2s_dma_addr;
static dma_addr_t i2s_mmap_addr;

static const struct file_operations i2s_fops = {
	owner		: THIS_MODULE,
	mmap		: i2s_mmap,
	open		: i2s_open,
	release		: i2s_release,
	ioctl		: i2s_ioctl,
	
};

int __init i2s_mod_init(void)
{
	int res;
	
	MSG("i2s_mod_init \n");
	
	/* register device with kernel */
	res = register_chrdev(I2S_MAJOR, "I2S", &i2s_fops);
	if (res) {
		MSG("can't register i2s device with kernel\n");
		return res;
	}
	
	return 0;
}

void i2s_mod_exit(void)
{
	int res;
	
	res = unregister_chrdev(I2S_MAJOR, "i2sM0");
	if (res) {
		MSG("can't unregister i2s device with kernel\n");
		return ;
	}
	
	return ;
}


static int i2s_open(struct inode *inode, struct file *filp)
{
	int i;
	int Ret,data;
	int minor = iminor(inode);

	MSG("\n\r");
	if (minor >= I2S_MAX_DEV)
		return -ENODEV;
	
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
	MOD_INC_USE_COUNT;
#else
	try_module_get(THIS_MODULE);
#endif

	if (filp->f_flags & O_NONBLOCK) {
		MSG("filep->f_flags O_NONBLOCK set\n");
		return -EAGAIN;
	}

	/* set i2s_config */
	pi2s_config = (i2s_config_type*)kmalloc(sizeof(i2s_config_type), GFP_KERNEL);
	if(pi2s_config==NULL)
		return -1;

	filp->private_data = pi2s_config;
	memset(pi2s_config, 0, sizeof(i2s_config_type));
	
#ifdef I2S_STATISTIC
	pi2s_status = (i2s_status_type*)kmalloc(sizeof(i2s_status_type), GFP_KERNEL);
	if(pi2s_status==NULL)
		return -1;
	memset(pi2s_status, 0, sizeof(i2s_status_type));	
#endif

	pi2s_config->flag = 0;
	pi2s_config->dmach = GDMA_I2S_TX0;
	pi2s_config->ff_thres = CONFIG_I2S_TFF_THRES;
	pi2s_config->ch_swap = CONFIG_I2S_CH_SWAP;
	pi2s_config->slave_en = CONFIG_I2S_SLAVE_EN;
	
	pi2s_config->srate = 44100;
	pi2s_config->vol = 0;
	
	/* allocate buffer */
	pi2s_config->buf8ptr = (u8*)pci_alloc_consistent(NULL, I2S_PAGE_SIZE*2 , &i2s_dma_addr);
	if(pi2s_config->buf8ptr==NULL)
	{
#ifdef I2S_STATISTIC
		kfree(pi2s_status);
#endif
		kfree(pi2s_config);
		pi2s_config = NULL;
		return -1;
	}

	memset(pi2s_config->buf8ptr, 0, I2S_PAGE_SIZE*2);
	pi2s_config->page0buf8ptr = pi2s_config->buf8ptr;
	pi2s_config->page1buf8ptr = pi2s_config->page0buf8ptr + I2S_PAGE_SIZE;
	
	pi2s_config->w_idx = 0;
	pi2s_config->r_idx = 0;
	
	Ret = request_irq(SURFBOARDINT_I2S, i2s_irq_isr, SA_INTERRUPT, "Ralink_I2S", NULL);
	if(Ret){
		MSG("IRQ %d is not free.\n", SURFBOARDINT_I2S);
		i2s_release(inode, filp);
		return -1;
	}
	MSG("request_irq=%d\n", SURFBOARDINT_I2S);
	
	pi2s_config->dmach = GDMA_I2S_TX0;
 
    init_waitqueue_head(&(pi2s_config->i2s_qh));
    
	MSG("i2s_open succeeds\n");

	return 0;
}


static int i2s_release(struct inode *inode, struct file *filp)
{
	i2s_config_type* ptri2s_config;

	/* decrement usage count */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
	MOD_DEC_USE_COUNT;
#else
	module_put(THIS_MODULE);
#endif
	free_irq(SURFBOARDINT_I2S, NULL);
	
	ptri2s_config = filp->private_data;
	i2s_dev_close(ptri2s_config);
	if(ptri2s_config->pMMAPBufPtr)
	{ 
		pci_free_consistent(NULL, I2S_PAGE_SIZE*MAX_I2S_PAGE, ptri2s_config->pMMAPBufPtr, i2s_mmap_addr);	
	}	
#ifdef I2S_STATISTIC
	kfree(pi2s_status);
#endif

	/* free buffer */
	if(ptri2s_config->buf8ptr)
		pci_free_consistent(NULL, I2S_PAGE_SIZE*2, ptri2s_config->buf8ptr, i2s_dma_addr);

	MSG("i2s_release succeeds\n");
	return 0;
}

static int i2s_mmap(struct file *filp, struct vm_area_struct *vma)
{
	i2s_config_type* ptri2s_config = filp->private_data;
	char* pBuf;
   	int nRet;

	if(ptri2s_config->pMMAPBufPtr==NULL)
	{
		ptri2s_config->pMMAPBufPtr = (u8*)pci_alloc_consistent(NULL, I2S_PAGE_SIZE*MAX_I2S_PAGE , &i2s_mmap_addr);
		if( ptri2s_config->pMMAPBufPtr == NULL ) 
		{
			return -1;
		} 	
		memset(ptri2s_config->pMMAPBufPtr, 0, I2S_PAGE_SIZE*MAX_I2S_PAGE);
	}
	
	nRet = remap_pfn_range(vma, vma->vm_start, virt_to_phys((void *)ptri2s_config->pMMAPBufPtr) >> PAGE_SHIFT,  I2S_PAGE_SIZE*MAX_I2S_PAGE, vma->vm_page_prot);
	
	if( nRet != 0 )
	{
		MSG("i2s_mmap->remap_pfn_range failed\n");
		return -EIO;
	}	
	
	return 0;
}
u32 bswap_32(u32 x)
{
	/*
    u32 b1 = x & 0xff000000;
    u32 b2 = x & 0x00ff0000;
    u32 b3 = x & 0x0000ff00;
    u32 b4 = x & 0x000000ff;
    return (b1 >> 24) | (b2 >> 8) | (b3 << 8) | (b4 << 24);
    */
    u32 b1 = x & 0xffFF0000;
    u32 b2 = x & 0x0000FFFF;
    return (b1 >> 16) | (b2 << 16);
}
int i2s_ioctl (struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	int x;
	unsigned long flags, data;
	i2s_config_type* ptri2s_config;
	
	ptri2s_config = filp->private_data;
	switch (cmd) {
	case I2S_SRATE:
		if((arg>MAX_SRATE_HZ)||(arg<MIN_SRATE_HZ))
		{
			MSG("audio sampling rate %d should be %d ~ %d Hz\n", arg, MIN_SRATE_HZ, MAX_SRATE_HZ);
			break;
		}	
		ptri2s_config->srate = arg;
		MSG("set audio sampling rate to %d Hz\n", ptri2s_config->srate);
		break;
	case I2S_VOL:
		if(((int)arg>MAX_VOL_DB)||((int)arg<MIN_VOL_DB))
		{
			MSG("audio volumn %d should be %d ~ %d dB\n", arg, MIN_VOL_DB, MAX_VOL_DB);
			
			break;
		}	
		ptri2s_config->vol = arg;
		x = 2*arg+255;
		audiohw_set_master_vol(x, x);
		MSG("set audio volumn to %d dB\n", ptri2s_config->vol);
		break;
	case I2S_ENABLE:
		ptri2s_config->isr_cnt = 0;	
		pi2s_status->buffer_unrun = 0;
		pi2s_status->buffer_ovrun = 0;
		pi2s_status->txdmafault = 0;
		pi2s_status->ovrun = 0;
		pi2s_status->unrun = 0;
		pi2s_status->thres = 0;
		pi2s_status->buffer_len = 0;
		data = i2s_inw(RALINK_REG_INTENA);
		//data |=0x0400;
		data &=0xFFBFF;
	    i2s_outw(RALINK_REG_INTENA, data);
		MSG("i2s audio enable\n");
		
		if(i2s_dev_open(ptri2s_config)!=I2S_OK)
		{
			i2s_release(inode, filp);
			return -1;
		}
		wake_up_interruptible(&(ptri2s_config->i2s_qh));
		
		break;
	case I2S_DISABLE:
		MSG("i2s audio disable\n");
		data = i2s_inw(RALINK_REG_INTENA);
		data &= 0xFFFFFBFF;
	    i2s_outw(RALINK_REG_INTENA, data);
		ptri2s_config->bDMAStart = 0;
		i2s_dev_close(ptri2s_config);
		wake_up_interruptible(&(ptri2s_config->i2s_qh));
		break;
	case I2S_GET_WBUF:
#ifdef I2S_FIFO_MODE	
		{
			int i,j;
			long* pBufPtr = pi2s_config->pMMAPBufPtr+(pi2s_config->w_idx)*I2S_PAGE_SIZE;
			for(i=0; i<I2S_PAGE_SIZE>>2; i++)
			{
				long status;
				i2s_outw(I2S_FIFO_WREG, bswap_32(pBufPtr[i]));	
				while(1)
				{
					status = i2s_inw(I2S_FF_STATUS);
					if(status&0x00F!=0)
						break;
				}	
			}
			ptri2s_config->w_idx = (ptri2s_config->w_idx+1)%MAX_I2S_PAGE;
			(int)(*(int*)arg) = (int)ptri2s_config->w_idx;
			break;
		}
#endif		
		if(ptri2s_config->bDMAStart==0)
		{
			GdmaI2sTx(ptri2s_config->page0buf8ptr, I2S_FIFO_WREG, 0, I2S_PAGE_SIZE, i2s_dma_handler, i2s_unmask_handler);
			GdmaI2sTx(ptri2s_config->page1buf8ptr, I2S_FIFO_WREG, 1, I2S_PAGE_SIZE, i2s_dma_handler, i2s_unmask_handler);
			ptri2s_config->bDMAStart=1;
			GdmaUnMaskChannel(GDMA_I2S_TX0);
		}
		
		do{
			spin_lock_irqsave(&ptri2s_config->lock, flags);
			
			if(((ptri2s_config->w_idx+1)%MAX_I2S_PAGE)!=ptri2s_config->r_idx)
			{
				ptri2s_config->w_idx = (ptri2s_config->w_idx+1)%MAX_I2S_PAGE;
				(int)(*(int*)arg) = (int)ptri2s_config->w_idx;
				pi2s_status->buffer_len++;
				spin_unlock_irqrestore(&ptri2s_config->lock, flags);
				break;
			}
			else
			{
				/* Buffer Full */
				//printk("BF w=%d, r=%d\n", ptri2s_config->w_idx, ptri2s_config->r_idx);
				pi2s_status->buffer_ovrun++;
				ptri2s_config->bSleep = 1;
				spin_unlock_irqrestore(&ptri2s_config->lock, flags);
				interruptible_sleep_on(&(ptri2s_config->i2s_qh));
				
			}
		}while(1);
		break;
						
	default :
		MSG("i2s_ioctl: command format error\n");
	}

	return 0;
}
static int i2s_dev_open(i2s_config_type* ptri2s_config)
{
	int i, result, db, srate;

	unsigned int data = 0;
	
	MSG("i2s_dev_open \n");
	
	/* set i2s clk */
	data = i2s_inw(RALINK_SYSCTL_BASE+0x30); 
	data &= 0xFFFF00FF;
#ifdef I2S_MS_MODE

#ifdef I2S_IN_CLKSRC 
	data |= 0x00008000;
	/* for internal clock = 15.625 Mhz */
	switch(ptri2s_config->srate)
	{
		case 8000:
			srate = 60<<8;
			break;
		case 11250:
			srate = 43<<8;
			break;	
		case 16000:
			srate = 30<<8;
			break;
		case 22050:
			srate = 21<<8;
			break;
        case 24000:
			srate = 19<<8;
			break;	
		case 32000:
			srate = 14<<8;
			break;			
		case 44100:
			srate = 10<<8;
			break;
		case 48000:
			srate = 9<<8;
			break;
		case 88200:
			srate = 7<<8;
			break;	
		case 96000:
			srate = 4<<8;
			break;
		default:
			srate = 10<<8;
	}
#ifdef FPGA_BOARD_RT3052	
	/* for internal clock = 12.5Mhz */
	switch(ptri2s_config->srate)
	{
		case 8000:
			srate = 48<<8;
			break;
		case 11250:
			srate = 34<<8;
			break;	
		case 16000:
			srate = 23<<8;
			break;
		case 22050:
			srate = 17<<8;
			break;
        case 24000:
			srate = 15<<8;
			break;	
		case 32000:
			srate = 11<<8;
			break;			
		case 44100:
			srate = 8<<8;
			break;
		case 48000:
			srate = 7<<8;
			break;
		case 88200:
			srate = 5<<8;
			break;	
		case 96000:
			srate = 3<<8;
			break;
		default:
			srate = 8<<8;
	}
#endif	
#else
	data |= 0x0000C000;
	/* for external clock = 12.288Mhz */
	switch(ptri2s_config->srate)
	{
		case 8000:
			srate = 48<<8;
			break;
		case 11250:
			srate = 34<<8;
			break;	
		case 16000:
			srate = 23<<8;
			break;
		case 22050:
			srate = 17<<8;
			break;
        case 24000:
			srate = 15<<8;
			break;	
		case 32000:
			srate = 11<<8;
			break;			
		case 44100:
			srate = 8<<8;
			break;
		case 48000:
			srate = 7<<8;
			break;
		case 88200:
			srate = 5<<8;
			break;	
		case 96000:
			srate = 3<<8;
			break;
		default:
			srate = 8<<8;
	}
#endif
	data |= srate;
#else
	/* for external clock = 12.288Mhz, I2S slave mode */
	switch(ptri2s_config->srate)
	{
		case 8000:
			srate = 0x04;
			break;
		case 11250:
			srate = 0x10;
			break;	
		case 16000:
			srate = 0x14;
			break;
		case 22050:
			srate = 0x38;
			break;
        case 24000:
			srate = 0x38;
			break;	
		case 32000:
			srate = 0x18;
			break;			
		case 44100:
			srate = 0x20;
			break;
		case 48000:
			srate = 0x00;
			break;
		case 88200:
			srate = 0x00;
			break;	
		case 96000:
			srate = 0x1C;
			break;
		default:
			srate = 0x20;
	}
#endif
	
	i2s_outw(RALINK_SYSCTL_BASE+0x30, data);
	MSG("RALINK_SYSCTL_BASE+0x30=0x%08X\n",data);
	
	/* set share pins to i2c */
	data = i2s_inw(RALINK_REG_GPIOMODE); 
	data &= 0xFFFFFFE0;
	data |= 0x00000008;
	i2s_outw(RALINK_REG_GPIOMODE, data);
	MSG("RALINK_REG_GPIOMODE=0x%08X\n",data);
	
	/* DAC initialization */
	audiohw_preinit();
	audiohw_postinit();
#ifdef	I2S_MS_MODE
#else
	audiohw_set_frequency(srate);
#endif	
	//saudiohw_enable_output();
	//db = tenthdb2master(ptri2s_config->vol);
	//audiohw_set_lineout_vol(db, db);
		
	/* set I2S_I2SCFG */
	data = i2s_inw(I2S_I2SCFG);
	data &= 0xFFFFFF80;
	data |= REGBIT(ptri2s_config->ff_thres, I2S_FF_THRES);
	data |= REGBIT(ptri2s_config->ch_swap, I2S_CH_SWAP);
	
#ifdef I2S_MS_MODE
	data &= ~REGBIT(0x1, I2S_SLAVE_EN);
	data &= ~REGBIT(0x1, I2S_CLK_OUT_DIS);
#else
	data |= REGBIT(0x1, I2S_SLAVE_EN);
	data |= REGBIT(0x1, I2S_CLK_OUT_DIS);
#endif
	i2s_outw(I2S_I2SCFG, data);	
	MSG("I2S_I2SCFG=%X\n",data);
	i2s_dev_enable(ptri2s_config);

	return I2S_OK;
}

int i2s_dev_close(i2s_config_type* ptri2s_config)
{
	i2s_dev_disable(ptri2s_config);
	ptri2s_config->isr_cnt = 0;
	ptri2s_config->w_idx = ptri2s_config->r_idx = 0;
	audiohw_close();
	return I2S_OK;
}

int i2s_dev_enable(i2s_config_type* ptri2s_config)
{
	unsigned int data, int_en, Ret;
	
	int_en = i2s_inw(I2S_INT_EN);
	data = i2s_inw(I2S_I2SCFG);
	data |= REGBIT(0x1, I2S_EN);
#ifdef I2S_FIFO_MODE
	data &= ~REGBIT(0x1, I2S_DMA_EN);
#else
	data |= REGBIT(0x1, I2S_DMA_EN);
#endif	
	
	data &= ~REGBIT(0x1, I2S_CH_SWAP);
	data &= ~REGBIT(0x1, I2S_CH1_OFF);
	data &= ~REGBIT(0x1, I2S_CH0_OFF);

	int_en |= REGBIT(0x1, I2S_INT3_EN);
	int_en |= REGBIT(0x1, I2S_INT2_EN);
	int_en |= REGBIT(0x1, I2S_INT1_EN);
	
	//int_en |= REGBIT(0x1, I2S_INT0_EN);

#ifdef I2S_FIFO_MODE	
	int_en = 0;
#endif
	
	MSG("I2S_INT_EN=0x%08X\n",int_en);
	MSG("I2S_I2SCFG=0x%08X\n",data);
	i2s_outw(I2S_INT_EN, int_en);
	i2s_outw(I2S_I2SCFG, data);

	return I2S_OK;
}

int i2s_dev_disable(i2s_config_type* ptri2s_config)
{
	unsigned int data, int_en;

	data = i2s_inw(I2S_I2SCFG);
	data &= 0x3FFFFFFF;
	i2s_outw(I2S_I2SCFG, data);
	i2s_outw(I2S_INT_EN, 0);	
	
	return I2S_OK;
}

void i2s_dma_handler(u32 dma_ch)
{
	int i,iline;
	int srate =0;
	u32 i2s_status;
	int mute = -1;
	int bWakeUp = 0;
	int data;
	
	i2s_status=i2s_inw(I2S_INT_STATUS);
	pi2s_config->isr_cnt++;
	
#ifdef 	I2S_STATISTIC
	if(pi2s_config->isr_cnt%200==0)
		MSG("isr i=%d,l=%d,o=%d,u=%d,s=%X [r=%d,w=%d]\n",pi2s_config->isr_cnt,pi2s_status->buffer_len,pi2s_status->buffer_ovrun,pi2s_status->buffer_unrun,i2s_status,pi2s_config->r_idx,pi2s_config->w_idx);
#endif	
	//i2s_outw(I2S_INT_STATUS, 0xFFFFFFFF);

	if(pi2s_config->bDMAStart==0)
	{
		goto EXIT;
	}

	if(pi2s_config->r_idx==pi2s_config->w_idx)
	{
		/* Buffer Empty */
		MSG("BE r=%d w=%d[i=%d,c=%d]\n",pi2s_config->r_idx,pi2s_config->w_idx,pi2s_config->isr_cnt,dma_ch);
#ifdef I2S_STATISTIC		
		pi2s_status->buffer_unrun++;
#endif		
		if(dma_ch==GDMA_I2S_TX0)
		{
			GdmaI2sTx(pi2s_config->page0buf8ptr , I2S_FIFO_WREG, 0, I2S_PAGE_SIZE, i2s_dma_handler, i2s_unmask_handler);
		}
		else
		{
			GdmaI2sTx(pi2s_config->page1buf8ptr, I2S_FIFO_WREG, 1, I2S_PAGE_SIZE, i2s_dma_handler, i2s_unmask_handler);
		}
		
		goto EXIT;	
	}
	
	if(pi2s_config->pMMAPBufPtr==NULL)
	{
		MSG("mmap buf NULL\n");
		if(dma_ch==GDMA_I2S_TX0)
			GdmaI2sTx(pi2s_config->page0buf8ptr, I2S_FIFO_WREG, 0, I2S_PAGE_SIZE, i2s_dma_handler, i2s_unmask_handler);
		else
			GdmaI2sTx(pi2s_config->page1buf8ptr, I2S_FIFO_WREG, 1, I2S_PAGE_SIZE, i2s_dma_handler, i2s_unmask_handler);

		goto EXIT;	
	}
#ifdef I2S_STATISTIC	
	pi2s_status->buffer_len--;
#endif
	if(dma_ch==GDMA_I2S_TX0)
	{
		//pi2s_config->page0buf8ptr = (pi2s_config->pMMAPBufPtr)+(pi2s_config->r_idx)*I2S_PAGE_SIZE;
		//dma_cache_wback_inv(pi2s_config->page0buf8ptr, I2S_PAGE_SIZE);
		memcpy(pi2s_config->page0buf8ptr,  pi2s_config->pMMAPBufPtr+(pi2s_config->r_idx)*I2S_PAGE_SIZE, I2S_PAGE_SIZE);	
		GdmaI2sTx((unsigned long)(pi2s_config->page0buf8ptr) , I2S_FIFO_WREG, 0, I2S_PAGE_SIZE, i2s_dma_handler, i2s_unmask_handler);
		pi2s_config->dmach = GDMA_I2S_TX0;
		pi2s_config->r_idx = (pi2s_config->r_idx+1)%MAX_I2S_PAGE;
	}
	else
	{
		//pi2s_config->page1buf8ptr = (pi2s_config->pMMAPBufPtr)+(pi2s_config->r_idx)*I2S_PAGE_SIZE;
		//dma_cache_wback_inv((unsigned long)(pi2s_config->page1buf8ptr), I2S_PAGE_SIZE);
		memcpy(pi2s_config->page1buf8ptr,  pi2s_config->pMMAPBufPtr+(pi2s_config->r_idx)*I2S_PAGE_SIZE, I2S_PAGE_SIZE);			
		GdmaI2sTx((unsigned long)(pi2s_config->page1buf8ptr) , I2S_FIFO_WREG, 1, I2S_PAGE_SIZE, i2s_dma_handler, i2s_unmask_handler);
		pi2s_config->dmach = GDMA_I2S_TX1;
		pi2s_config->r_idx = (pi2s_config->r_idx+1)%MAX_I2S_PAGE;
		
	}
EXIT:	
	if(pi2s_config->bSleep==1)
	{
		wake_up_interruptible(&(pi2s_config->i2s_qh));
		pi2s_config->bSleep = 0;
	}
	
	return;
}

void i2s_unmask_handler(u32 dma_ch)
{
	MSG("i2s_unmask_handler ch=%d\n",dma_ch);
	if(dma_ch==GDMA_I2S_TX1)
	{
		GdmaI2sTx(pi2s_config->page0buf8ptr, I2S_FIFO_WREG, 0, I2S_PAGE_SIZE, i2s_dma_handler, i2s_unmask_handler);
		GdmaUnMaskChannel(GDMA_I2S_TX0);
	}
	else
	{
		GdmaI2sTx(pi2s_config->page1buf8ptr, I2S_FIFO_WREG, 1, I2S_PAGE_SIZE, i2s_dma_handler, i2s_unmask_handler);
		GdmaUnMaskChannel(GDMA_I2S_TX1);
	}
}

irqreturn_t i2s_irq_isr(int irq, void *irqaction)
{
	u32 i2s_status;
	MSG("i2s_st=%X\n",i2s_status);
	i2s_status=i2s_inw(I2S_INT_STATUS);

	if(i2s_status&REGBIT(1, I2S_DMA_FAULT))
	{
#ifdef I2S_STATISTIC
		pi2s_status->txdmafault++;
#endif
	}
	if(i2s_status&REGBIT(1, I2S_OVRUN))
	{
#ifdef I2S_STATISTIC
		pi2s_status->ovrun++;
#endif
	}
	if(i2s_status&REGBIT(1, I2S_UNRUN))
	{
#ifdef I2S_STATISTIC
		pi2s_status->unrun++;
#endif
	}
	if(i2s_status&REGBIT(1, I2S_THRES))
	{
#ifdef I2S_STATISTIC
		pi2s_status->thres++;
#endif
	}
	i2s_outw(I2S_INT_STATUS, 0xFFFFFFFF);
	return IRQ_HANDLED;
}


module_init(i2s_mod_init);
module_exit(i2s_mod_exit);

MODULE_DESCRIPTION("Ralink SoC I2S Controller Module");
MODULE_AUTHOR("Qwert Chin <qwert.chin@ralinktech.com.tw>");
MODULE_SUPPORTED_DEVICE("I2S");
MODULE_VERSION(MOD_VERSION);
MODULE_LICENSE("GPL");