#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/rt2880/surfboardint.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include "pcm_ctrl.h"
#include "g711.h"
#include "../ralink_gpio.h"
#include "../spi_drv.h"
#include "dual_io.h"
#include "dual.h"

#include "../ralink_gdma.h"

pcm_config_type* ppcm_config;
pcm_status_type* ppcm_status;
static short drop_buf[PCM_PAGE_SAMPLES];
static short proc_buf[MAX_PCM_CH][PCM_PAGE_SAMPLES];
static short zero_buf[PCM_PAGE_SAMPLES];

int pcm_reg_setup(pcm_config_type* ptrpcm_config); 
int pcm_enable(unsigned int chid, pcm_config_type* ptrpcm_config);
int pcm_disable(unsigned int chid, pcm_config_type* ptrpcm_config);

void pcm_dma_tx_isr(u32 chid);
void pcm_dma_rx_isr(u32 chid);
void pcm_unmask_isr(u32 dma_ch);

static irqreturn_t pcm_irq_isr(int irq, void *irqaction);

struct file_operations pcmdrv_fops = {
    ioctl:      pcm_ioctl,
};
static int pcmdrv_major =  233;
static dma_addr_t TxPage0, TxPage1;
static dma_addr_t RxPage0, RxPage1;

#ifdef PCM_TASKLET
struct tasklet_struct pcm_rx_tasklet;
struct tasklet_struct pcm_tx_tasklet;
#endif

//struct tasklet_struct phone_tasklet;
//extern int phone_task(unsigned long pData);

int __init pcm_init(void)
{
	int i;
	int result=0;
	result = register_chrdev(pcmdrv_major, "PCM", &pcmdrv_fops);
	if (result < 0) {
		printk(KERN_WARNING "pcm_drv: can't get major %d\n",pcmdrv_major);
	return result;
	}
	MSG("pcm_init()\n");
	pcm_open();
	
	return 0;
}

void pcm_exit(void)
{
	pcm_close();
	unregister_chrdev(pcmdrv_major, "PCM");
	return 0;
}

int pcm_open()
{
	int i, j, result, data, cnt0, cnt1, flags;
	
	/* set pcm_config */
	ppcm_config = (pcm_config_type*)kmalloc(sizeof(pcm_config_type), GFP_KERNEL);
	if(ppcm_config==NULL)
		return PCM_OUTOFMEM;
	memset(ppcm_config, 0, sizeof(pcm_config_type));

#ifdef PCM_STATISTIC
	ppcm_status = (pcm_status_type*)kmalloc(sizeof(pcm_status_type), GFP_KERNEL);
	if(ppcm_status==NULL)
		return PCM_OUTOFMEM;
	memset(ppcm_status, 0, sizeof(pcm_status_type));
#endif
	
	ppcm_config->pcm_ch_num = CONFIG_PCM_CH;
	ppcm_config->nch_active = 0;
	ppcm_config->extclk_en = CONFIG_PCM_EXT_CLK_EN;
	ppcm_config->clkout_en = CONFIG_PCM_CLKOUT_EN;
	ppcm_config->ext_fsync = CONFIG_PCM_EXT_FSYNC;
	ppcm_config->long_fynsc = CONFIG_PCM_LONG_FSYNC;
	ppcm_config->fsync_pol = CONFIG_PCM_FSYNC_POL;
	ppcm_config->drx_tri = CONFIG_PCM_DRX_TRI;
	ppcm_config->slot_mode = CONFIG_PCM_SLOTMODE;
	ppcm_config->tff_thres = CONFIG_PCM_TFF_THRES;
	ppcm_config->rff_thres = CONFIG_PCM_RFF_THRES;
		
	for ( i = 0 ; i < ppcm_config->pcm_ch_num; i ++ )
	{
		ppcm_config->lbk[i] = CONFIG_PCM_LBK;
		ppcm_config->ext_lbk[i] = CONFIG_PCM_EXT_LBK;
		ppcm_config->cmp_mode[i] = CONFIG_PCM_CMP_MODE;
		ppcm_config->ts_start[i] = CONFIG_PCM_TS_START + i*16;	
		ppcm_config->txfifo_rd_idx[i] = 0;
		ppcm_config->txfifo_wt_idx[i] = 0;
		ppcm_config->rxfifo_rd_idx[i] = 0;
		ppcm_config->rxfifo_wt_idx[i] = 0;
		ppcm_config->bsfifo_rd_idx[i] = 0;
		ppcm_config->bsfifo_wt_idx[i] = 0;

	}

	MSG("allocate fifo buffer\n");
	/* allocate fifo buffer */
	for ( i = 0 ; i < ppcm_config->pcm_ch_num; i ++ )
	{
		ppcm_config->TxFIFOBuf16Ptr[i] = kmalloc(PCM_FIFO_SIZE, GFP_KERNEL);
		if(ppcm_config->TxFIFOBuf16Ptr[i]==NULL)
		{
			pcm_close();
			return PCM_OUTOFMEM;
		}

		ppcm_config->RxFIFOBuf16Ptr[i] = kmalloc(PCM_FIFO_SIZE, GFP_KERNEL);
		if(ppcm_config->RxFIFOBuf16Ptr[i]==NULL)
		{
			pcm_close();
			return PCM_OUTOFMEM;
		}
		
#ifdef PCM_SW_G729AB		
		ppcm_config->BSFIFOBuf16Ptr[i] = kmalloc(PCM_BSFIFO_SIZE, GFP_KERNEL);
		if(ppcm_config->BSFIFOBuf16Ptr[i]==NULL)
		{
			pcm_close();
			return PCM_OUTOFMEM;
		}
#endif		
	}
	MSG("allocate page buffer\n");
	/* allocate page buffer */
	for ( i = 0 ; i < ppcm_config->pcm_ch_num; i ++ )
	{
		ppcm_config->TxPage0Buf16Ptr[i] = pci_alloc_consistent(NULL, PCM_PAGE_SIZE , &TxPage0);
		if(ppcm_config->TxPage0Buf16Ptr[i]==NULL)
		{
			pcm_close();
			return PCM_OUTOFMEM;
		}
		ppcm_config->TxPage1Buf16Ptr[i] = pci_alloc_consistent(NULL, PCM_PAGE_SIZE , &TxPage1);
		if(ppcm_config->TxPage1Buf16Ptr[i]==NULL)
		{
			pcm_close();
			return PCM_OUTOFMEM;
		}
		ppcm_config->RxPage0Buf16Ptr[i] = pci_alloc_consistent(NULL, PCM_PAGE_SIZE , &RxPage0);
		if(ppcm_config->RxPage0Buf16Ptr[i]==NULL)
		{
			pcm_close();
			return PCM_OUTOFMEM;
		}
		ppcm_config->RxPage1Buf16Ptr[i] = pci_alloc_consistent(NULL, PCM_PAGE_SIZE , &RxPage1);
		if(ppcm_config->RxPage1Buf16Ptr[i]==NULL)
		{
			pcm_close();
			return PCM_OUTOFMEM;
		}
	}
	

	/* PCM controller reset */

PCM_RESET:	
	
	data = 0x00000800;
	pcm_outw(RALINK_SYSCTL_BASE+0x34,data);
	for(i=0;i<100000;i++);
	
	data = 0x00040000;
	pcm_outw(RALINK_SYSCTL_BASE+0x34, data);
	/* PCM controller CLK_DIV set */
	data = pcm_inw(RALINK_SYSCTL_BASE+0x30);

#ifdef PCM_IN_CLK
	data &= ~REGBIT(1, PCM_CLK_SEL);
	data |= REGBIT(1, PCM_CLK_EN);	
	data &= 0xFFFFFFC0;
#if	defined(CONFIG_RT3052_FPGA)
	data |= REGBIT(48, PCM_CLK_DIV);	/* Assume internal clock = 12.5Mhz */
#else
	data |= REGBIT(60, PCM_CLK_DIV);    /* Assume internal clock = 15.625Mhz */
#endif
	data |= 0x00000080;
#else	
	data |= REGBIT(1, PCM_CLK_SEL);
	data |= REGBIT(1, PCM_CLK_EN);
	data &= 0xFFFFFFC0;
	data |= REGBIT(46, PCM_CLK_DIV);	/* Assume REF_CLK = 12Mhz */
#endif	
	
	pcm_outw(RALINK_SYSCTL_BASE+0x30, data);
	MSG("RALINK_SYSCTL_BASE+0x30=0x%08X\n",data);
	
	data = pcm_inw(RALINK_REG_GPIOMODE); 
	data &= 0xFFFFFFE1;
	data |= 0x00000008;
	pcm_outw(RALINK_REG_GPIOMODE, data);
	MSG("RALINK_REG_GPIOMODE=0x%08X\n",data);

	if(pcm_reg_setup(ppcm_config)!=PCM_OK)
		MSG("PCM:pcm_reg_setup() failed\n");


	spin_lock_irqsave(&ppcm_config->lock, flags);

    data = pcm_inw(PCM_GLBCFG);
    data |= REGBIT(0x1, PCM_EN);
    pcm_outw(PCM_GLBCFG, data);
	

				
	MSG("SLIC reset....\n");
	
	pcm_reset_slic();
	if(slic_init()!=0)
	{
		for ( i = 0 ; i < ppcm_config->pcm_ch_num ; i ++ )
			pcm_disable(i, ppcm_config);
			
		spin_unlock_irqrestore(&ppcm_config->lock, flags);	
		goto PCM_RESET;
	}

	spin_unlock_irqrestore(&ppcm_config->lock, flags);
	
#ifdef PCM_TASKLET
	tasklet_init(&pcm_rx_tasklet, pcm_rx_task, ppcm_config);
	tasklet_init(&pcm_tx_tasklet, pcm_tx_task, ppcm_config);
	MSG("pcm tasklet initialization\n");
#endif	
	//data = pcm_inw(PCM_GLBCFG);
	//data |= REGBIT(0x1, DMA_EN);
	//pcm_outw(PCM_GLBCFG, data);

	MSG("pcm_open done...\n");
	
	//tasklet_init(&phone_tasklet, phone_task, NULL);
	//MSG("phone tasklet initialization\n");
	//tasklet_hi_schedule(&phone_tasklet);



	return PCM_OK;
}

int pcm_reg_setup(pcm_config_type* ptrpcm_config)
{
	unsigned int data = 0;
	
	/* set PCMCFG */
	MSG("pcm_reg_setup\n");
	data |= REGBIT(ptrpcm_config->extclk_en, PCM_EXT_CLK_EN);
	data |= REGBIT(ptrpcm_config->clkout_en,  PCM_CLKOUT);
	
	data |= REGBIT(ptrpcm_config->ext_fsync, PCM_EXT_FSYNC);
	data |= REGBIT(ptrpcm_config->long_fynsc, PCM_LONG_FSYNC);
	data |= REGBIT(ptrpcm_config->fsync_pol, PCM_FSYNC_POL);
	data |= REGBIT(ptrpcm_config->drx_tri, PCM_DRX_TRI);
	data |= REGBIT(ptrpcm_config->slot_mode, PCM_SLOTMODE);
	MSG("pcm_reg_setup:PCM_PCMCFG=%X\n",data);
	pcm_outw(PCM_PCMCFG, data);

	/* set GLBCFG's threshold fields */

	data = 0;
	data |= REGBIT(ptrpcm_config->tff_thres, TFF_THRES);
	data |= REGBIT(ptrpcm_config->rff_thres, RFF_THRES);
	MSG("PCM_GLBCFG=%X\n",data);
	pcm_outw(PCM_GLBCFG, data);

	/* set CH0/1_CFG */
	
	data = 0;
	data |= REGBIT(ptrpcm_config->lbk[0], PCM_LBK);
	data |= REGBIT(ptrpcm_config->ext_lbk[0], PCM_EXT_LBK);
	data |= REGBIT(ptrpcm_config->cmp_mode[0], PCM_CMP_MODE);
	data |= REGBIT(ptrpcm_config->ts_start[0], PCM_TS_START);
	MSG("PCM_CH0_CFG=%X\n",data);
	pcm_outw(PCM_CH0_CFG, data);

	data = 0;
	data |= REGBIT(ptrpcm_config->lbk[1], PCM_LBK);
	data |= REGBIT(ptrpcm_config->ext_lbk[1], PCM_EXT_LBK);
	data |= REGBIT(ptrpcm_config->cmp_mode[1], PCM_CMP_MODE);
	data |= REGBIT(ptrpcm_config->ts_start[1], PCM_TS_START);
	MSG("PCM_CH1_CFG=%X\n",data);
	pcm_outw(PCM_CH1_CFG, data);


	return PCM_OK;
}
int pcm_close()
{
	int i;
		
	MSG("pcm_close\n");	
#ifdef PCM_TASKLET
	tasklet_kill(&pcm_rx_tasklet);
	tasklet_kill(&pcm_tx_tasklet);
#endif	

	for( i = 0 ; i < ppcm_config->pcm_ch_num ; i ++ )
		pcm_disable(i, ppcm_config);
	
#ifdef PCM_STATISTIC
	kfree(ppcm_status);
#endif
	
	/* free buffer */
	for( i = 0 ; i < ppcm_config->pcm_ch_num ; i ++ )
	{
		if(ppcm_config->TxPage0Buf16Ptr[i])
			pci_free_consistent(NULL, PCM_PAGE_SIZE, TxPage0, ppcm_config->TxPage0Buf16Ptr[i]);
		if(ppcm_config->TxPage1Buf16Ptr[i])
			pci_free_consistent(NULL, PCM_PAGE_SIZE, TxPage1, ppcm_config->TxPage1Buf16Ptr[i]);	
		if(ppcm_config->RxPage0Buf16Ptr[i])
			pci_free_consistent(NULL, PCM_PAGE_SIZE, RxPage0, ppcm_config->RxPage0Buf16Ptr[i]);
		if(ppcm_config->RxPage1Buf16Ptr[i])
			pci_free_consistent(NULL, PCM_PAGE_SIZE, RxPage1, ppcm_config->RxPage1Buf16Ptr[i]);					
		if(ppcm_config->TxFIFOBuf16Ptr[i])
			kfree(ppcm_config->TxFIFOBuf16Ptr[i]);	
		if(ppcm_config->RxFIFOBuf16Ptr[i])
			kfree(ppcm_config->RxFIFOBuf16Ptr[i]);
#ifdef PCM_SW_G729AB			
		if(ppcm_config->BSFIFOBuf16Ptr[i])
			kfree(ppcm_config->BSFIFOBuf16Ptr[i]);
#endif						
	}

	kfree(ppcm_config);
	ppcm_config = NULL;
	
	return PCM_OK;
}

int pcm_enable(unsigned int chid, pcm_config_type* ptrpcm_config)
{
	unsigned int GLBCFG_Data=0, int_en, Ret,i;
	char* p8Data;
	
	if(ptrpcm_config->nch_active>=ptrpcm_config->pcm_ch_num)
	{
		MSG("There are %d channels already enabled\n",ptrpcm_config->nch_active);
		return PCM_OK;
	}
	int_en = pcm_inw(PCM_INT_EN);
	GLBCFG_Data = pcm_inw(PCM_GLBCFG);

	pcm_outw(PCM_INT_STATUS, 0x0);
	
	switch(chid)
	{
		case 0:
			MSG("PCM:enable CH0\n");
			GLBCFG_Data |= REGBIT(0x1, CH0_TX_EN);
			GLBCFG_Data |= REGBIT(0x1, CH0_RX_EN);
			
			int_en |= REGBIT(0x1, CH0T_DMA_FAULT);
			int_en |= REGBIT(0x1, CH0R_DMA_FAULT);
			
			//int_en |= REGBIT(0x1, CH0T_OVRUN);
			//int_en |= REGBIT(0x1, CH0T_UNRUN);
			//int_en |= REGBIT(0x1, CH0R_OVRUN);
			//int_en |= REGBIT(0x1, CH0R_UNRUN);
			
			//int_en |= REGBIT(0x1, CH0T_THRES);
			//int_en |= REGBIT(0x1, CH0R_THRES);
			ptrpcm_config->nch_active++;
			break;
		case 1:
			MSG("PCM:enable CH1\n");

			GLBCFG_Data |= REGBIT(0x1, CH1_TX_EN);
			GLBCFG_Data |= REGBIT(0x1, CH1_RX_EN);
			
			int_en |= REGBIT(0x1, CH1T_DMA_FAULT);
			int_en |= REGBIT(0x1, CH1R_DMA_FAULT);
			
			//int_en |= REGBIT(0x1, CH1T_OVRUN);
			//int_en |= REGBIT(0x1, CH1T_UNRUN);
			//int_en |= REGBIT(0x1, CH1R_OVRUN);
			//int_en |= REGBIT(0x1, CH1R_UNRUN);

			//int_en |= REGBIT(0x1, CH1T_THRES);
			//int_en |= REGBIT(0x1, CH1R_THRES);
			ptrpcm_config->nch_active++;
			break;
		default:
			break;
	}

	GLBCFG_Data |= REGBIT(0x1, PCM_EN);
	pcm_outw(PCM_INT_EN, int_en);
	pcm_outw(PCM_GLBCFG, GLBCFG_Data);
	
	return PCM_OK;
}

int pcm_disable(unsigned int chid, pcm_config_type* ptrpcm_config)
{
	unsigned int data, int_en;

	if(ptrpcm_config->nch_active<=0)
	{ 
		MSG("No channels needed to disable\n");
		return PCM_OK;
	}
	ppcm_config->txfifo_rd_idx[chid] = 0;
	ppcm_config->txfifo_wt_idx[chid] = 0;
	ppcm_config->rxfifo_rd_idx[chid] = 0;
	ppcm_config->rxfifo_wt_idx[chid] = 0;
	ppcm_config->bsfifo_rd_idx[chid] = 0;
	ppcm_config->bsfifo_wt_idx[chid] = 0;
	
	int_en = pcm_inw(PCM_INT_EN);
	data = pcm_inw(PCM_PCMCFG);
	
	switch(chid)
	{
		case 0:
			MSG("PCM:disable CH0\n");
			data &= ~REGBIT(0x1, CH0_TX_EN);
			data &= ~REGBIT(0x1, CH0_RX_EN);
			int_en &= ~REGBIT(0x1, CH0T_DMA_FAULT);
			int_en &= ~REGBIT(0x1, CH0R_DMA_FAULT);
			pcm_outw(PCM_INT_EN, int_en);
			ptrpcm_config->nch_active--;

			break;
		case 1:
			MSG("PCM:disable CH1\n");
			data &= ~REGBIT(0x1, CH1_TX_EN);
			data &= ~REGBIT(0x1, CH1_RX_EN);
			int_en &= ~REGBIT(0x1, CH1T_DMA_FAULT);
			int_en &= ~REGBIT(0x1, CH1R_DMA_FAULT);
			pcm_outw(PCM_INT_EN, int_en);
			ptrpcm_config->nch_active--;

			break;
		default:
			break;
	}
	if(ptrpcm_config->nch_active<=0)
	{
		data &= ~REGBIT(0x1, PCM_EN);
		data &= ~REGBIT(0x1, DMA_EN);
	}
	pcm_outw(PCM_PCMCFG, data);
	return PCM_OK;
}

void pcm_dma_tx_isr(u32 dma_ch)
{
	int i,j;
	int chid=0;
	int page=0;
	int value;
	short* p16PageBuf=NULL, *p16FIFOBuf=NULL, *p16Data;
	u32 pcm_status,dma_status=0;
	u32* pPCM_FIFO=NULL;

	if((ppcm_config->tx_isr_cnt%170==11)||(ppcm_config->tx_isr_cnt%170==90))
	{
		dma_status=pcm_inw(RALINK_GDMAISTS);
		i= pcm_inw(PCM_GLBCFG);
		pcm_status=pcm_inw(PCM_INT_STATUS);
		//printk("ti c=%d %X %X tc=%d\n",dma_ch,pcm_status,i, ppcm_config->tx_isr_cnt);	
	}

	ppcm_config->tx_isr_cnt++;
	if((dma_ch==GDMA_PCM0_TX0)||(dma_ch==GDMA_PCM0_TX1))
	{
		chid = 0;
		pPCM_FIFO = PCM_CH0_FIFO;
		
	}
	else if((dma_ch==GDMA_PCM1_TX0)||(dma_ch==GDMA_PCM1_TX1))
	{
		chid = 1;
		pPCM_FIFO = PCM_CH1_FIFO;
	}
	else
	{
		printk("PCM ERR : tx dma channel number is illeagle\n");
	}
	
	if((dma_ch==GDMA_PCM0_TX0)||(dma_ch==GDMA_PCM1_TX0))
	{
		page = 0;
		p16PageBuf = ppcm_config->TxPage0Buf16Ptr[chid];
	}
	if((dma_ch==GDMA_PCM0_TX1)||(dma_ch==GDMA_PCM1_TX1))
	{
		page = 1;
		p16PageBuf = ppcm_config->TxPage1Buf16Ptr[chid];
	}
	
	p16FIFOBuf = ppcm_config->TxFIFOBuf16Ptr[chid];
	ppcm_config->pos = 0;

	if(ppcm_config->tx_isr_cnt>4)
	{
	
		for (i = 0 ; i < MAX_PCM_PROC_UNIT ; i ++ )
		{
			if((ppcm_config->txfifo_rd_idx[chid]%MAX_PCM_FIFO)==ppcm_config->txfifo_wt_idx[chid])
			//if(((ppcm_config->txfifo_rd_idx[chid]+1)%MAX_PCM_FIFO)==ppcm_config->txfifo_wt_idx[chid])
			{
				/* tx fifo empty */
				printk("TFE(%d) \n",dma_ch);
				break;
			}
			
			p16Data = p16FIFOBuf + (ppcm_config->txfifo_rd_idx[chid]*PCM_8KHZ_SAMPLES);
			memcpy((void*)(p16PageBuf+ppcm_config->pos), p16Data, PCM_8KHZ_SAMPLES*sizeof(short));
			//dma_cache_wback_inv(p16PageBuf+ppcm_config->pos, sizeof(short)*PCM_8KHZ_SAMPLES);	
			
			ppcm_config->pos+=PCM_8KHZ_SAMPLES;
			ppcm_config->txfifo_rd_idx[chid] = (ppcm_config->txfifo_rd_idx[chid]+1)%MAX_PCM_FIFO;
		}
	}
	
	GdmaPcmTx((char*)p16PageBuf, pPCM_FIFO, chid, page, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);		

	ppcm_config->txcurchid = chid;
#ifdef PCM_TASKLET	
	//if((dma_ch==GDMA_PCM1_TX0)||(dma_ch==GDMA_PCM1_TX1))
	if(ppcm_config->tx_isr_cnt%4==0)
		tasklet_hi_schedule(&pcm_tx_tasklet);
#else
	pcm_task(ppcm_config);
#endif	
	return;
	
}

void pcm_dma_rx_isr(u32 dma_ch)
{
	int i,j;
	int chid=0; 
	int page=0;

	short* p16PageBuf=NULL, *p16FIFOBuf=NULL, *p16Data;

	u32 pcm_status=0,dma_status=0;
	u32* pPCM_FIFO=NULL;
	
	if((dma_ch==GDMA_PCM0_RX0)||(dma_ch==GDMA_PCM0_RX1))
	{
		chid = 0;
		pPCM_FIFO = PCM_CH0_FIFO;	
	}
	else if((dma_ch==GDMA_PCM1_RX0)||(dma_ch==GDMA_PCM1_RX1))
	{
		chid = 1;
		pPCM_FIFO = PCM_CH1_FIFO;
	}
	else
	{
		MSG("PCM ERR : rx dma channel number (CH=%d) is illeagle\n", dma_ch);
	}
	
	
	if((dma_ch==GDMA_PCM0_RX0)||(dma_ch==GDMA_PCM1_RX0))
	{
		page = 0;
		p16PageBuf = (char*)(ppcm_config->RxPage0Buf16Ptr[chid]);
	}
	else if((dma_ch==GDMA_PCM0_RX1)||(dma_ch==GDMA_PCM1_RX1))
	{
		page = 1;
		p16PageBuf = (char*)(ppcm_config->RxPage1Buf16Ptr[chid]);
	}
	else
	{
		MSG("PCM ERR : rx dma channel number (CH=%d) is illeagle\n", dma_ch);
	}
	
	if((ppcm_config->rx_isr_cnt%170==50)||(ppcm_config->rx_isr_cnt%170==129))
	{
		dma_status=pcm_inw(RALINK_GDMAISTS);
		pcm_status=pcm_inw(PCM_INT_STATUS);
		i=pcm_inw(PCM_GLBCFG);
		//printk("ri c=%d %X %X rc=%d\n",dma_ch,pcm_status,i,ppcm_config->rx_isr_cnt);
	}

	ppcm_config->rx_isr_cnt++;
	p16FIFOBuf = ppcm_config->RxFIFOBuf16Ptr[chid];
	ppcm_config->pos = 0;
	
	for (i = 0 ; i < MAX_PCM_PROC_UNIT ; i ++ )
	{
		if(((ppcm_config->rxfifo_wt_idx[chid]+1)%MAX_PCM_FIFO)==ppcm_config->rxfifo_rd_idx[chid])
		{
			/* rx fifo full */
			printk("RFF(%d) ",chid);
			break;
		}

		p16Data = p16FIFOBuf + (ppcm_config->rxfifo_wt_idx[chid]*PCM_8KHZ_SAMPLES);
		memcpy((void*)p16Data, (void*)(p16PageBuf+ppcm_config->pos), PCM_8KHZ_SAMPLES*sizeof(short));
		ppcm_config->pos+=PCM_8KHZ_SAMPLES;
		ppcm_config->rxfifo_wt_idx[chid] = (ppcm_config->rxfifo_wt_idx[chid]+1)%MAX_PCM_FIFO;
		
	}
	
	GdmaPcmRx(pPCM_FIFO, (char*)p16PageBuf, chid, page, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);

	ppcm_config->curchid = chid;
#ifdef PCM_TASKLET	
	//if((dma_ch==GDMA_PCM0_RX0)||(dma_ch==GDMA_PCM0_RX1))
	//if(dma_ch==GDMA_PCM1_RX1)
	if(ppcm_config->rx_isr_cnt%4==1)
		tasklet_hi_schedule(&pcm_rx_tasklet);
#else
	pcm_rx_task(ppcm_config);
#endif		
	return;
}
#if 0
int pcm_tx_task(unsigned long pData)
{
	pcm_config_type* ptrpcm_config = ppcm_config;
	int txch,rxch,ch;
	short* pBuf; 
	short PCMBuf[PCM_8KHZ_SAMPLES];
	short* uCompressBuf = PCMBuf;
	short serial[88];
	int i,j, value;
	
	short* pTx16Data;
	short* pRx16Data;
	char* p8Data;
	unsigned int flags, data;
	/* handle rx->tx fifo buffer */
printk("pcm_tx_task start\n");
	for(ch=0; ch<2; ch++)
	{
		rxch = 1-ch;
		txch = ch;
	
	for (i = 0 ; i < MAX_PCM_PROC_UNIT ; i ++ )
	{
#ifdef PCM_TASKLET	  
		spin_lock_irqsave(&ptrpcm_config->lock, flags);
#endif			
		
#ifdef PCM_SW_G729AB	
		set_g729decVoIPCh(txch);
#endif		
		if(((ptrpcm_config->txfifo_wt_idx[txch]+1)%MAX_PCM_FIFO)==ptrpcm_config->txfifo_rd_idx[txch])
		{
			/* tx fifo full */
			spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
			printk("TTFF(%d) ",txch);
			pTx16Data = NULL;//drop_buf;
			//spin_lock_irqsave(&ptrpcm_config->lock, flags);
		}
		else
		{
			
			pTx16Data = ptrpcm_config->TxFIFOBuf16Ptr[txch] + (ptrpcm_config->txfifo_wt_idx[txch]*PCM_8KHZ_SAMPLES);
			
		}
		if((ptrpcm_config->bsfifo_rd_idx[rxch]%MAX_PCM_BSFIFO)==ptrpcm_config->bsfifo_wt_idx[rxch])
		{
			/* rx fifo empty */
			spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
			printk("TRFE(%d) ",rxch);
			pRx16Data = NULL;//zero_buf;
			//spin_lock_irqsave(&ptrpcm_config->lock, flags);
		}
		else
		{		
			printk("ch=%d, rd_idx=%d\n",rxch, ptrpcm_config->bsfifo_rd_idx[rxch]);
			pRx16Data = ptrpcm_config->BSFIFOBuf16Ptr[rxch] + (ptrpcm_config->bsfifo_rd_idx[rxch]*PCM_BS_SIZE);
			
		}	

#ifdef PCM_TASKLET	
		spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
#endif
		if((pRx16Data==NULL)||(pTx16Data==NULL))
			continue;
			
		ptrpcm_config->txfifo_wt_idx[txch] = (ptrpcm_config->txfifo_wt_idx[txch]+1)%MAX_PCM_FIFO;
		ptrpcm_config->bsfifo_rd_idx[rxch] = (ptrpcm_config->bsfifo_rd_idx[rxch]+1)%MAX_PCM_BSFIFO;	
#ifdef PCM_SW_G729AB
		
		//spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
		data = read_c0_status();
		write_c0_status(data&0xFFFF8FFF);	
		printk("g729ab_decode_frame\n");

		g729ab_decode_frame(pRx16Data, PCMBuf, 0);
		printk("g729ab_decode_frame done\n");
		write_c0_status(data);
		
		//spin_lock_irqsave(&ptrpcm_config->lock, flags);
		Copy(PCMBuf, pTx16Data, PCM_8KHZ_SAMPLES);
		
#endif
	}

}
printk("pcm_tx_task done\n");
	return 0;
}
#endif
#if 1
int pcm_tx_task(unsigned long pData)
{
	pcm_config_type* ptrpcm_config = ppcm_config;//pData;
	int txch,rxch,ch;
	short* pBuf; 
	short PCMBuf[PCM_8KHZ_SAMPLES];
	short* uCompressBuf = PCMBuf;
	short serial[88];
	int i,j, value;
	
	short* pTx16Data;
	short* pRx16Data;
	char* p8Data;
	unsigned int flags, data;
	/* handle rx->tx fifo buffer */
	//ppcm_config->pos = 0;
	
	

	//MSG("TSK=%d\n",ptrpcm_config->curchid);


	for(ch=0; ch<2; ch++)
	{
		rxch = ch;
		txch = 1-ch;

	//data = read_c0_status();
	//write_c0_status(data&0xFFFF8FFF);
			
	for (i = 0 ; i < MAX_PCM_PROC_UNIT ; i ++ )
	{
#ifdef PCM_TASKLET	  
		spin_lock_irqsave(&ptrpcm_config->lock, flags);
#endif			
		
		//txch = ptrpcm_config->txcurchid;
		//rxch = 1-ptrpcm_config->txcurchid;
		
#ifdef PCM_SW_G729AB	
		set_g729encVoIPCh(rxch);
		set_g729decVoIPCh(txch);
#endif		
		if(((ptrpcm_config->txfifo_wt_idx[txch]+1)%MAX_PCM_FIFO)==ptrpcm_config->txfifo_rd_idx[txch])
		{
			/* tx fifo full */
			spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
			printk("TTFF(%d) ",txch);
			pTx16Data = NULL;//drop_buf;
			//spin_lock_irqsave(&ptrpcm_config->lock, flags);
		}
		else
		{
			
			pTx16Data = ptrpcm_config->TxFIFOBuf16Ptr[txch] + (ptrpcm_config->txfifo_wt_idx[txch]*PCM_8KHZ_SAMPLES);
			ptrpcm_config->txfifo_wt_idx[txch] = (ptrpcm_config->txfifo_wt_idx[txch]+1)%MAX_PCM_FIFO;
		}
		//if(((ptrpcm_config->rxfifo_rd_idx[rxch]+1)%MAX_PCM_FIFO)==ptrpcm_config->rxfifo_wt_idx[rxch])
		if((ptrpcm_config->rxfifo_rd_idx[rxch]%MAX_PCM_FIFO)==ptrpcm_config->rxfifo_wt_idx[rxch])
		{
			/* rx fifo empty */
			spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
			printk("TRFE(%d) ",rxch);
			pRx16Data = NULL;//zero_buf;
			//spin_lock_irqsave(&ptrpcm_config->lock, flags);
		}
		else
		{		
			pRx16Data = ptrpcm_config->RxFIFOBuf16Ptr[rxch] + (ptrpcm_config->rxfifo_rd_idx[rxch]*PCM_8KHZ_SAMPLES);
			ptrpcm_config->rxfifo_rd_idx[rxch] = (ptrpcm_config->rxfifo_rd_idx[rxch]+1)%MAX_PCM_FIFO;
		}	

#ifdef PCM_TASKLET	
		spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
#endif
		if((pRx16Data==NULL)||(pTx16Data==NULL))
			continue;
			
#ifdef PCM_RECORD 
		if(ptrpcm_config->iRecordCH==ptrpcm_config->curchid)
		{
			if(ptrpcm_config->bStartRecord)
			{
				p8Data = ptrpcm_config->mmapbuf + ptrpcm_config->mmappos;
				memcpy((void*)p8Data, (void*)pRx16Data, PCM_8KHZ_SAMPLES*sizeof(short));
				ptrpcm_config->mmappos+=PCM_8KHZ_SAMPLES*sizeof(short);
			}
		}
#endif


			
#ifdef PCM_SW_L2U
		ulaw_compress(PCM_8KHZ_SAMPLES, pRx16Data, uCompressBuf);
		ulaw_expand(PCM_8KHZ_SAMPLES, uCompressBuf, pTx16Data);
#endif
#ifdef PCM_SW_L2A
		alaw_compress(PCM_8KHZ_SAMPLES, pRx16Data, uCompressBuf);
		alaw_expand(PCM_8KHZ_SAMPLES, uCompressBuf, pTx16Data);
#endif

#ifdef PCM_SW_G729AB
		
		Copy(pRx16Data, PCMBuf, PCM_8KHZ_SAMPLES);
		//spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
		memset(serial, 0, 82*2);
		
		data = read_c0_status();
		write_c0_status(data&0xFFFF8FFF);
		g729ab_encode_frame(PCMBuf, serial, 1);
		write_c0_status(data);
		data = read_c0_status();
		write_c0_status(data&0xFFFF8FFF);
		g729ab_decode_frame(serial, PCMBuf, 0);
		write_c0_status(data);
		
		//spin_lock_irqsave(&ptrpcm_config->lock, flags);
		Copy(PCMBuf, pTx16Data, PCM_8KHZ_SAMPLES);
		
#endif

#ifdef PCM_SW_CODEC
#else
		memcpy((void*)pTx16Data, (void*)pRx16Data, PCM_8KHZ_SAMPLES*sizeof(short));
#endif
	
#ifdef PCM_TASKLET	
		//spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
#endif	
	}
	//write_c0_status(data);
	}
	return 0;
}
#endif
#if 0
int pcm_rx_task(unsigned long pData)
{
	pcm_config_type* ptrpcm_config = ppcm_config;
	int txch,rxch,ch;
	short* pBuf; 
	short PCMBuf[PCM_8KHZ_SAMPLES];
	short* uCompressBuf = PCMBuf;
	short serial[88];
	int i,j, value;
	
	short* pTx16Data;
	short* pRx16Data;
	char* p8Data;
	unsigned int flags, data;
	
	/* handle rx->tx fifo buffer */

	for( ch = 0 ; ch < 2 ; ch++ )
	{
		rxch = ch;
		txch = 1-ch;
	
		for (i = 0 ; i < MAX_PCM_PROC_UNIT ; i ++ )
		{
#ifdef PCM_TASKLET	  
			spin_lock_irqsave(&ptrpcm_config->lock, flags);
#endif			
		
#ifdef PCM_SW_G729AB	
			set_g729encVoIPCh(rxch);
#endif		
			if(((ptrpcm_config->bsfifo_wt_idx[txch]+1)%MAX_PCM_BSFIFO)==ptrpcm_config->bsfifo_wt_idx[txch])
			{
				/* tx fifo full */
				spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
				printk("TTFF(%d) ",txch);
				pTx16Data = NULL;
			}
			else
			{
				pTx16Data = ptrpcm_config->BSFIFOBuf16Ptr[txch] + (ptrpcm_config->bsfifo_wt_idx[txch]*PCM_BS_SIZE);		
			}

			if((ptrpcm_config->rxfifo_rd_idx[rxch]%MAX_PCM_FIFO)==ptrpcm_config->rxfifo_wt_idx[rxch])
			{
				/* rx fifo empty */
				spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
				printk("RRFE(%d) ",rxch);
				pRx16Data = NULL;
			}
			else
			{		
				pRx16Data = ptrpcm_config->RxFIFOBuf16Ptr[rxch] + (ptrpcm_config->rxfifo_rd_idx[rxch]*PCM_8KHZ_SAMPLES);
			}	

#ifdef PCM_TASKLET	
			spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
#endif
			if((pRx16Data==NULL)||(pTx16Data==NULL))
				continue;

			ptrpcm_config->bsfifo_wt_idx[txch] = (ptrpcm_config->bsfifo_wt_idx[txch]+1)%MAX_PCM_BSFIFO;
			ptrpcm_config->rxfifo_rd_idx[rxch] = (ptrpcm_config->rxfifo_rd_idx[rxch]+1)%MAX_PCM_FIFO;
		
#ifdef PCM_SW_G729AB	
			Copy(pRx16Data, PCMBuf, PCM_8KHZ_SAMPLES);
			data = read_c0_status();
			write_c0_status(data&0xFFFF8FFF);
			printk("g729ab_encode_frame\n");
			g729ab_encode_frame(PCMBuf, pTx16Data, 1);
			write_c0_status(data);	
#endif

#ifdef PCM_SW_CODEC
#else
			memcpy((void*)pTx16Data, (void*)pRx16Data, PCM_8KHZ_SAMPLES*sizeof(short));
#endif
	}
}

	return 0;
}
#endif

int pcm_rx_task(unsigned long pData)
{
	pcm_config_type* ptrpcm_config = ppcm_config;//pData;
	int txch,rxch,ch;
	short* pBuf; 
	short PCMBuf[PCM_8KHZ_SAMPLES];
	short* uCompressBuf = PCMBuf;
	short serial[88];
	int i,j, value;
	
	short* pTx16Data;
	short* pRx16Data;
	char* p8Data;
	unsigned int flags, data;
	
	/* handle rx->tx fifo buffer */
	for( ch = 0 ; ch < 2 ; ch ++ )
	{
		rxch = ch;
		txch = 1-ch;
	
		for (i = 0 ; i < MAX_PCM_PROC_UNIT ; i ++ )
		{
#ifdef PCM_TASKLET	  
			spin_lock_irqsave(&ptrpcm_config->lock, flags);
#endif			
		
#ifdef PCM_SW_G729AB	
			set_g729encVoIPCh(rxch);
			set_g729decVoIPCh(txch);
#endif		
			if(((ptrpcm_config->txfifo_wt_idx[txch]+1)%MAX_PCM_FIFO)==ptrpcm_config->txfifo_rd_idx[txch])
			{
				/* tx fifo full */
				spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
				printk("RTFF(%d) ",txch);
				pTx16Data = NULL;
			}
			else
			{
				
				pTx16Data = ptrpcm_config->TxFIFOBuf16Ptr[txch] + (ptrpcm_config->txfifo_wt_idx[txch]*PCM_8KHZ_SAMPLES);
				ptrpcm_config->txfifo_wt_idx[txch] = (ptrpcm_config->txfifo_wt_idx[txch]+1)%MAX_PCM_FIFO;
			}

			if((ptrpcm_config->rxfifo_rd_idx[rxch]%MAX_PCM_FIFO)==ptrpcm_config->rxfifo_wt_idx[rxch])
			{
				/* rx fifo empty */
				spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
				printk("RRFE(%d) ",rxch);
				pRx16Data = NULL;
			}
			else
			{		
				pRx16Data = ptrpcm_config->RxFIFOBuf16Ptr[rxch] + (ptrpcm_config->rxfifo_rd_idx[rxch]*PCM_8KHZ_SAMPLES);
				ptrpcm_config->rxfifo_rd_idx[rxch] = (ptrpcm_config->rxfifo_rd_idx[rxch]+1)%MAX_PCM_FIFO;
			}	

#ifdef PCM_TASKLET	
			spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
#endif
			if((pRx16Data==NULL)||(pTx16Data==NULL))
				continue;
			
#ifdef PCM_RECORD 
			if(ptrpcm_config->iRecordCH==ptrpcm_config->curchid)
			{
				if(ptrpcm_config->bStartRecord)
				{
					p8Data = ptrpcm_config->mmapbuf + ptrpcm_config->mmappos;
					memcpy((void*)p8Data, (void*)pRx16Data, PCM_8KHZ_SAMPLES*sizeof(short));
					ptrpcm_config->mmappos+=PCM_8KHZ_SAMPLES*sizeof(short);
				}
			}
#endif


			
#ifdef PCM_SW_L2U
			ulaw_compress(PCM_8KHZ_SAMPLES, pRx16Data, uCompressBuf);
			ulaw_expand(PCM_8KHZ_SAMPLES, uCompressBuf, pTx16Data);
#endif
#ifdef PCM_SW_L2A
			alaw_compress(PCM_8KHZ_SAMPLES, pRx16Data, uCompressBuf);
			alaw_expand(PCM_8KHZ_SAMPLES, uCompressBuf, pTx16Data);
#endif

#ifdef PCM_SW_G729AB
			Copy(pRx16Data, PCMBuf, PCM_8KHZ_SAMPLES);
			memset(serial, 0, 82*2);	
			data = read_c0_status();
			write_c0_status(data&0xFFFF8FFF);
			g729ab_encode_frame(PCMBuf, serial, 1);
			write_c0_status(data);
			data = read_c0_status();
			write_c0_status(data&0xFFFF8FFF);
			g729ab_decode_frame(serial, PCMBuf, 0);
			write_c0_status(data);
			Copy(PCMBuf, pTx16Data, PCM_8KHZ_SAMPLES);
#endif

#ifdef PCM_SW_CODEC
#else
			memcpy((void*)pTx16Data, (void*)pRx16Data, PCM_8KHZ_SAMPLES*sizeof(short));
#endif
		}
	}
	return 0;
}

void pcm_unmask_isr(u32 dma_ch)
{
	int i, Ret;
	unsigned long data;
	char* p8Data;
	MSG("umisr c=%d\n",dma_ch);

	/* disable system interrupt for PCM */
	data = pcm_inw(RALINK_REG_INTENA);
	data &= 0xFFFFFFEF;
	pcm_outw(RALINK_REG_INTENA, data);
    		
	for( i = 0 ; i < ppcm_config->pcm_ch_num ; i ++ )
		pcm_disable(i, ppcm_config);
	p8Data = (char*)(ppcm_config->RxPage0Buf16Ptr[0]);
	GdmaPcmRx(PCM_CH0_FIFO, p8Data, 0, 0, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
	
	p8Data = (char*)(ppcm_config->RxPage1Buf16Ptr[0]);
	GdmaPcmRx(PCM_CH0_FIFO, p8Data	, 0, 1, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);	
	
	GdmaUnMaskChannel(GDMA_PCM0_RX0);
	
	p8Data = (char*)(ppcm_config->TxPage0Buf16Ptr[0]);
	GdmaPcmTx(p8Data, PCM_CH0_FIFO, 0, 0, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);
	
	p8Data = (char*)(ppcm_config->TxPage1Buf16Ptr[0]);
	GdmaPcmTx(p8Data, PCM_CH0_FIFO, 0, 1, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);

	GdmaUnMaskChannel(GDMA_PCM0_TX0);
	
	p8Data = (char*)(ppcm_config->TxPage0Buf16Ptr[1]);
	GdmaPcmTx(p8Data, PCM_CH1_FIFO, 1, 0, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);
	
	p8Data = (char*)(ppcm_config->TxPage1Buf16Ptr[1]);
	GdmaPcmTx(p8Data, PCM_CH1_FIFO, 1, 1, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);

	GdmaUnMaskChannel(GDMA_PCM1_TX0);
	
	p8Data = (char*)(ppcm_config->RxPage0Buf16Ptr[1]);
	GdmaPcmRx(PCM_CH1_FIFO, p8Data, 1, 0, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
		
	p8Data = (char*)(ppcm_config->RxPage1Buf16Ptr[1]);					
	GdmaPcmRx(PCM_CH1_FIFO, p8Data, 1, 1, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
	
	GdmaUnMaskChannel(GDMA_PCM1_RX0);
	
	data = pcm_inw(PCM_GLBCFG);
	data |= REGBIT(0x1, DMA_EN);
	pcm_outw(PCM_GLBCFG, data);
	
	/* enable system interrupt for PCM */
	data = pcm_inw(RALINK_REG_INTENA);
	data |=0x010;
	pcm_outw(RALINK_REG_INTENA, data);

	for( i = 0; i < ppcm_config->nch_active; i++)
	{
		ppcm_config->txfifo_rd_idx[i] = 0;
		ppcm_config->txfifo_wt_idx[i] = 0;
		ppcm_config->rxfifo_rd_idx[i] = 0;
		ppcm_config->rxfifo_wt_idx[i] = 0;
		ppcm_config->bsfifo_rd_idx[i] = 0;
		ppcm_config->bsfifo_wt_idx[i] = 0;
	}
	ppcm_config->rx_isr_cnt = 0;
	ppcm_config->tx_isr_cnt = 0;
	
	for ( i = 0 ; i < ppcm_config->pcm_ch_num ; i ++ )
		pcm_enable(i, ppcm_config);


	return;
}

/**
 * @brief PCM interrupt handler 
 *
 * When PCM interrupt happened , call related handler 
 * to do the remain job.
 *
 */
irqreturn_t pcm_irq_isr(int irq, void *irqaction)
{
	u32 pcm_status;
	
	
	pcm_status=pcm_inw(PCM_INT_STATUS);
	MSG("SR=%08X\n",pcm_status);

	/* check CH0 status */
	if(pcm_status&REGBIT(1, CH0T_DMA_FAULT))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0txdmafault++;
#endif
	}
	if(pcm_status&REGBIT(1, CH0T_OVRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0txovrun++;
#endif
	}
	if(pcm_status&REGBIT(1, CH0T_UNRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0txunrun++;
#endif	
	}
	if(pcm_status&REGBIT(1, CH0T_THRES))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0txunrun++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH0R_DMA_FAULT))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0rxdmafault++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH0R_UNRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0rxovrun++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH0R_OVRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0rxunrun++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH0R_THRES))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0rxthres++;
#endif		
	}

	/* check CH1 status */
	if(pcm_status&REGBIT(1, CH1T_DMA_FAULT))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1txdmafault++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH1T_OVRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1txovrun++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH1T_UNRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1txunrun++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH1T_THRES))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1txthres++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH1R_DMA_FAULT))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1rxdmafault++;
#endif	
	}
	if(pcm_status&REGBIT(1, CH1R_OVRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1rxovrun++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH1R_UNRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1rxunrun++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH1R_THRES))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1rxthres++;
#endif		
	}
	pcm_outw(PCM_INT_STATUS, 0xFFFF);
	
	return IRQ_HANDLED;

}


int pcm_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	int data, i, Ret;
	extern unsigned char currentChannel;
	char* p8Data;
	
	pcm_config_type* ptrpcm_config = ppcm_config;
	pcm_record_type* ptrpcm_record;
	
	switch(cmd)
	{
		case PCM_SET_RECORD:
			MSG("iocmd=PCM_SET_RECORD\n");
			ptrpcm_config->mmapbuf = kmalloc(PCM_PAGE_SIZE, GFP_KERNEL);
			if(ptrpcm_config->mmapbuf==NULL)
				return -1;
			ptrpcm_config->mmappos = 0;
			ptrpcm_config->bStartRecord = 1;
			ptrpcm_config->iRecordCH = arg;
			break;
		case PCM_SET_UNRECORD:
			MSG("iocmd=PCM_SET_UNRECORD\n");
			ptrpcm_config->bStartRecord = 0;
			kfree(ptrpcm_config->mmapbuf);
			break;	
		case PCM_READ_PCM:
			ptrpcm_record = arg;
			//MSG("iocmd=PCM_READ_PCM pos=%d\n",ptrpcm_config->mmappos);
			if(ptrpcm_config->nch_active <= 0)
				return -1;
			copy_to_user(ptrpcm_record->pcmbuf, ptrpcm_config->mmapbuf, ptrpcm_config->mmappos);
			ptrpcm_record->size = ptrpcm_config->mmappos;
			ptrpcm_config->mmappos = 0;
			break;
		case PCM_START:
			MSG("iocmd=PCM_START\n");
#ifdef PCM_SW_G729AB
			set_g729encVoIPCh(0);
			set_g729decVoIPCh(0);
			g729ab_encode_init();
			g729ab_decode_init();
			set_g729encVoIPCh(1);
			set_g729decVoIPCh(1); 
			g729ab_encode_init();
			g729ab_decode_init();
#endif	
			p8Data = (char*)(ptrpcm_config->RxPage0Buf16Ptr[0]);
			GdmaPcmRx(PCM_CH0_FIFO, p8Data, 0, 0, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
			
			p8Data = (char*)(ptrpcm_config->RxPage1Buf16Ptr[0]);
			GdmaPcmRx(PCM_CH0_FIFO, p8Data	, 0, 1, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);	
			
			GdmaUnMaskChannel(GDMA_PCM0_RX0);
			
			p8Data = (char*)(ptrpcm_config->TxPage0Buf16Ptr[0]);
			GdmaPcmTx(p8Data, PCM_CH0_FIFO, 0, 0, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);
			
			p8Data = (char*)(ptrpcm_config->TxPage1Buf16Ptr[0]);
			GdmaPcmTx(p8Data, PCM_CH0_FIFO, 0, 1, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);
		
			GdmaUnMaskChannel(GDMA_PCM0_TX0);
			
			p8Data = (char*)(ptrpcm_config->TxPage0Buf16Ptr[1]);
			GdmaPcmTx(p8Data, PCM_CH1_FIFO, 1, 0, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);
			
			p8Data = (char*)(ptrpcm_config->TxPage1Buf16Ptr[1]);
			GdmaPcmTx(p8Data, PCM_CH1_FIFO, 1, 1, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);

			GdmaUnMaskChannel(GDMA_PCM1_TX0);
			
			p8Data = (char*)(ptrpcm_config->RxPage0Buf16Ptr[1]);
			GdmaPcmRx(PCM_CH1_FIFO, p8Data, 1, 0, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
				
			p8Data = (char*)(ptrpcm_config->RxPage1Buf16Ptr[1]);					
			GdmaPcmRx(PCM_CH1_FIFO, p8Data, 1, 1, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
			
			GdmaUnMaskChannel(GDMA_PCM1_RX0);
			
			data = pcm_inw(PCM_GLBCFG);
			data |= REGBIT(0x1, DMA_EN);
			pcm_outw(PCM_GLBCFG, data);
			
			/* enable system interrupt for PCM */
			data = pcm_inw(RALINK_REG_INTENA);
			data |=0x010;
    		pcm_outw(RALINK_REG_INTENA, data);
    
			Ret = request_irq(SURFBOARDINT_PCM, pcm_irq_isr, SA_INTERRUPT, "Ralink_PCM", NULL);
			if(Ret){
				MSG("PCM: IRQ %d is not free.\n", SURFBOARDINT_PCM);
				return PCM_REQUEST_IRQ_FAILED;
			}
			for( i = 0; i < ptrpcm_config->nch_active; i++)
			{
				ptrpcm_config->txfifo_rd_idx[i] = 0;
				ptrpcm_config->txfifo_wt_idx[i] = 0;
				ptrpcm_config->rxfifo_rd_idx[i] = 0;
				ptrpcm_config->rxfifo_wt_idx[i] = 0;
				ptrpcm_config->bsfifo_rd_idx[i] = 0;
				ptrpcm_config->bsfifo_wt_idx[i] = 0;
			}
			ptrpcm_config->rx_isr_cnt = 0;
			ptrpcm_config->tx_isr_cnt = 0;
			
			for ( i = 0 ; i < ptrpcm_config->pcm_ch_num ; i ++ )
				pcm_enable(i, ptrpcm_config);
			break;
		case PCM_STOP:
			MSG("iocmd=PCM_STOP\n");
			data = pcm_inw(PCM_GLBCFG);
			data &= ~REGBIT(0x1, DMA_EN);
			pcm_outw(PCM_GLBCFG, data);
			
			free_irq(SURFBOARDINT_PCM, NULL);
			
			/* disable system interrupt for PCM */
			data = pcm_inw(RALINK_REG_INTENA);
			data &=~0x010;
    		pcm_outw(RALINK_REG_INTENA, data);
			
			
			data = read_c0_status();
			write_c0_status(data|0x00007000);
			
			break;
		default:
			break;
	}
	
	return 0;
}

void pcm_reset_slic ()
{
	//press reset
 	int data;
 	int i;

	/* Set SPI to GPIO mode */
	data = pcm_inw(RALINK_REG_GPIOMODE); 
	data |= RALINK_GPIO(1);
	pcm_outw(RALINK_REG_GPIOMODE, data);
	
	/* RESET set to low */
	data = pcm_inw(RALINK_REG_PIODIR);
	data |= RALINK_GPIO(0);
	pcm_outw(RALINK_REG_PIODIR, data);
	data = pcm_inw(RALINK_REG_PIODATA);
	data &= 0xFFFFFFFE;
	pcm_outw(RALINK_REG_PIODATA, data);
	
	/* CS set to high */
	data = pcm_inw(RALINK_REG_PIODIR);
	data |= RALINK_GPIO(3);
	pcm_outw(RALINK_REG_PIODIR, data);
	data = pcm_inw(RALINK_REG_PIODATA);
	data |= RALINK_GPIO(3); 
	pcm_outw(RALINK_REG_PIODATA, data);
	
	mdelay(1000);
	
	/* RESET set to high */
	data = pcm_inw(RALINK_REG_PIODIR);
	data |= RALINK_GPIO(0);
	pcm_outw(RALINK_REG_PIODIR, data);
	data = pcm_inw(RALINK_REG_PIODATA);
	data |= RALINK_GPIO(0);
	pcm_outw(RALINK_REG_PIODATA, data);

	mdelay(1000);
	
	/* Set GPIO to SPI mode */
	data = pcm_inw(RALINK_REG_GPIOMODE); 
	data &= 0xFFFFFFFD;
	pcm_outw(RALINK_REG_GPIOMODE, data);
	
	return;
}

module_init(pcm_init);
module_exit(pcm_exit);

MODULE_DESCRIPTION("Ralink SoC PCM Controller Module");
MODULE_AUTHOR("Qwert Chin <qwert.chin@ralinktech.com.tw>");
MODULE_LICENSE("GPL");
MODULE_VERSION(MOD_VERSION);

