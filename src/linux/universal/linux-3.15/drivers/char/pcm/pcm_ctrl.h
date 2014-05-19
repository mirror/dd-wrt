#ifndef __RALINK_PCM_H_
#define __RALINK_PCM_H_

#ifdef __KERNEL__
#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#endif

#include <asm/rt2880/rt_mmap.h>

#define MOD_VERSION 			"0.1"

#define phys_to_bus(a) (a & 0x1FFFFFFF)

#ifndef u32
#define u32 unsigned long
#endif

#ifndef u16
#define u16 unsigned short
#endif

#ifndef u8
#define u8 unsigned char
#endif

#ifndef REGBIT
#define REGBIT(x, n)		(x << n)
#endif

#define Virtual2Physical(x)             (((int)x) & 0x1fffffff)
#define Physical2Virtual(x)             (((int)x) | 0x80000000)
#define Virtual2NonCache(x)             (((int)x) | 0x20000000)
#define Physical2NonCache(x)            (((int)x) | 0xa0000000)

#define pcm_outw(address, value)	*((volatile uint32_t *)(address)) = cpu_to_le32(value)
#define pcm_inw(address)			le32_to_cpu(*(volatile u32 *)(address))

#define PCM_DEBUG
#ifdef PCM_DEBUG
#define MSG(fmt, args...) printk("PCM_API: " fmt, ## args)
#else
#define MSG(fmt, args...) { }
#endif

/* Register Map, Ref to RT3052 Data Sheet */

/* System Controller bit field */
#define PCM_CLK_EN			7
#define PCM_CLK_SEL			6
#define	PCM_CLK_DIV			0

/* Register Map Detail */
#define PCM_GLBCFG				(RALINK_PCM_BASE+0x0000)
#define PCM_PCMCFG				(RALINK_PCM_BASE+0x0004)
#define PCM_INT_STATUS			(RALINK_PCM_BASE+0x0008)
#define PCM_INT_EN				(RALINK_PCM_BASE+0x000C)
#define PCM_FF_STATUS			(RALINK_PCM_BASE+0x0010)
#define PCM_CH0_CFG				(RALINK_PCM_BASE+0x0020)
#define PCM_CH1_CFG				(RALINK_PCM_BASE+0x0024)
#define PCM_RSV_REG16			(RALINK_PCM_BASE+0x0030)
#define PCM_CH0_FIFO			(RALINK_PCM_BASE+0x0080)
#define PCM_CH1_FIFO			(RALINK_PCM_BASE+0x0084)

/* PCMCFG bit field */
#define PCM_EXT_CLK_EN			31
#define PCM_CLKOUT				30
#define PCM_EXT_FSYNC			27
#define PCM_LONG_FSYNC			26
#define PCM_FSYNC_POL			25
#define PCM_DRX_TRI				24
#define PCM_SLOTMODE			0

/* GLBCFG bit field */
#define PCM_EN				31
#define DMA_EN				30
#define RFF_THRES			20
#define TFF_THRES			16
#define CH1_TX_EN			9
#define CH0_TX_EN			8
#define CH1_RX_EN			1
#define CH0_RX_EN			0

/* CH0/1_CFG bit field */
#define PCM_LBK					31
#define PCM_EXT_LBK				30
#define PCM_CMP_MODE			28
#define PCM_TS_START			0

/* INT_STATUS bit field */
#define CH1T_DMA_FAULT			15
#define CH1T_OVRUN				14
#define CH1T_UNRUN				13
#define CH1T_THRES				12
#define CH1R_DMA_FAULT			11
#define CH1R_OVRUN				10
#define CH1R_UNRUN				9
#define CH1R_THRES				8
#define CH0T_DMA_FAULT			7
#define CH0T_OVRUN				6
#define CH0T_UNRUN				5
#define CH0T_THRES				4
#define CH0R_DMA_FAULT			3
#define CH0R_OVRUN				2
#define CH0R_UNRUN				1
#define CH0R_THRES				0

/* INT_EN bit field */
#define INT15_EN				15
#define INT14_EN				14
#define INT13_EN				13
#define INT12_EN				12
#define INT11_EN				11
#define INT10_EN				10
#define INT9_EN					9
#define INT8_EN					8
#define INT7_EN					7
#define INT6_EN					6
#define INT5_EN					5
#define INT4_EN					4
#define INT3_EN					3
#define INT2_EN					2
#define INT1_EN					1
#define INT0_EN					0

/* FF_STATUS bit field */
#define CH1RFF_AVCNT			12
#define CH1TFF_AVCNT			8
#define CH0RFF_AVCNT			4
#define CH0TFF_AVCNT			0

/* Test scenario */
#define PCM_IN_CLK
//#define PCM_SLIC_LOOP
//#define PCM_INLOOP
//#define PCM_EXLOOP
#define PCM_STATISTIC

#define PCM_LINEAR
//#define PCM_ULAW
//#define PCM_ALAW
//#define PCM_U2L2U
//#define PCM_A2L2A
//#define PCM_SW_L2U
//#define PCM_SW_L2A
#define PCM_TASKLET
//#define PCM_RECORD
//#define PCM_SLIC_CLOCK
//#define PCM_SW_G729AB
//#define PCM_SW_CODEC
/* Constant definition */
#ifdef CONFIG_GDMA_PCM_I2S_OTHERS
#define MAX_PCM_CH				1
#else
#define MAX_PCM_CH				2
#endif
#define NTFF_THRES				4
#define NRFF_THRES				4

#define MAX_PCM_PROC_UNIT		3
#if	defined(PCM_ULAW)||defined(PCM_ALAW)
#define PCM_8KHZ_SAMPLES		40
#else
#define PCM_8KHZ_SAMPLES		80
#endif

#define MAX_PCM_FIFO			12
#define PCM_FIFO_SAMPLES		(PCM_8KHZ_SAMPLES*MAX_PCM_FIFO)
#define PCM_FIFO_SIZE			PCM_FIFO_SAMPLES*2

#define MAX_PCM_BSFIFO			12
#define PCM_BS_SIZE				166
#define PCM_BSFIFO_SIZE			(PCM_BS_SIZE*MAX_PCM_BSFIFO)

#define PCM_PAGE_SAMPLES		(PCM_8KHZ_SAMPLES*MAX_PCM_PROC_UNIT)
#define PCM_PAGE_SIZE			PCM_PAGE_SAMPLES*2

#define CONFIG_PCM_CH					MAX_PCM_CH

#ifdef PCM_INLOOP
#define CONFIG_PCM_LBK					1
#define CONFIG_PCM_EXT_LBK				0
#else
#define CONFIG_PCM_LBK					0
#define CONFIG_PCM_EXT_LBK				0
#endif

#ifdef PCM_IN_CLK
#define CONFIG_PCM_EXT_CLK_EN			0
#define CONFIG_PCM_CLKOUT_EN			1	/* It should be always one */
#define CONFIG_PCM_EXT_FSYNC			0
#else
#define CONFIG_PCM_EXT_CLK_EN			0
#define CONFIG_PCM_CLKOUT_EN			1	/* It should be always one */
#define CONFIG_PCM_EXT_FSYNC			0
#endif

#ifdef PCM_LINEAR
#define CONFIG_PCM_CMP_MODE			0
#endif
#if	defined(PCM_ULAW)||defined(PCM_ALAW)
#define CONFIG_PCM_CMP_MODE			1
#endif
#ifdef PCM_U2L2U
#define CONFIG_PCM_CMP_MODE			2
#endif
#ifdef PCM_A2L2A
#define CONFIG_PCM_CMP_MODE			3
#endif

#define CONFIG_PCM_LONG_FSYNC			0
#define CONFIG_PCM_FSYNC_POL			1
#define CONFIG_PCM_DRX_TRI			1
#define CONFIG_PCM_SLOTMODE			0
#define CONFIG_PCM_TS_START			1

#define CONFIG_PCM_TFF_THRES			NTFF_THRES
#define CONFIG_PCM_RFF_THRES			NRFF_THRES


/* driver status definition */
#define PCM_OK					0
#define PCM_OUTOFMEM				0x01
#define PCM_GDMAFAILED				0x02
#define PCM_REQUEST_IRQ_FAILED			0x04

/* driver i/o control command */
#define PCM_SET_RECORD			0
#define PCM_SET_UNRECORD		1	
#define PCM_READ_PCM			2
#define PCM_START				3
#define PCM_STOP				4

typedef struct pcm_status_t
{
	u32 ch0txdmafault;
	u32 ch0txovrun;
	u32 ch0txunrun;
	u32 ch0txthres;
	u32 ch0rxdmafault;
	u32 ch0rxovrun;
	u32 ch0rxunrun;
	u32 ch0rxthres;

	u32 ch1txdmafault;
	u32 ch1txovrun;
	u32 ch1txunrun;
	u32 ch1txthres;
	u32 ch1rxdmafault;
	u32 ch1rxovrun;
	u32 ch1rxunrun;
	u32 ch1rxthres;

}pcm_status_type;

typedef struct pcm_config_t
{
	u32 pcm_ch_num;
	u32 nch_active;
	int curchid,txcurchid;
	int txfifo_rd_idx[MAX_PCM_CH];
	int txfifo_wt_idx[MAX_PCM_CH];
	int rxfifo_rd_idx[MAX_PCM_CH];
	int rxfifo_wt_idx[MAX_PCM_CH];
	
	int bsfifo_rd_idx[MAX_PCM_CH];
	int bsfifo_wt_idx[MAX_PCM_CH];
	
	int rx_isr_cnt;
	int tx_isr_cnt;
	int pos;
	char* mmapbuf;
	int mmappos;
	int bStartRecord;
	int iRecordCH;
	
	u32 extclk_en;
	u32 clkout_en;
	u32 ext_fsync;
	u32 long_fynsc;
	u32 fsync_pol;
	u32 drx_tri;
	u32 slot_mode;

	u32 tff_thres;
	u32 rff_thres;

	u32	lbk[MAX_PCM_CH];
	u32 ext_lbk[MAX_PCM_CH];
	u32 ts_start[MAX_PCM_CH];
	u32 cmp_mode[MAX_PCM_CH];
	
	int lock;
	
	union {
		short* TxPage0Buf16Ptr[MAX_PCM_CH];	
		char* TxPage0Buf8Ptr[MAX_PCM_CH];
	};
	union {
		short* TxPage1Buf16Ptr[MAX_PCM_CH];	
		char* TxPage1Buf8Ptr[MAX_PCM_CH];
	};
	union {
		short* RxPage0Buf16Ptr[MAX_PCM_CH];	
		char* RxPage0Buf8Ptr[MAX_PCM_CH];	
	};
	union {
		short* RxPage1Buf16Ptr[MAX_PCM_CH];	
		char* RxPage1Buf8Ptr[MAX_PCM_CH];	
	};
	
	union {
		short* TxFIFOBuf16Ptr[MAX_PCM_CH];	
		char* TxFIFOBuf8Ptr[MAX_PCM_CH];	
	};
	
	union {
		short* RxFIFOBuf16Ptr[MAX_PCM_CH];	
		char* RxFIFOBuf8Ptr[MAX_PCM_CH];
	};
	
	union {
		short* BSFIFOBuf16Ptr[MAX_PCM_CH];	
		char* BSFIFOBuf8Ptr[MAX_PCM_CH];
	};

}pcm_config_type;

typedef struct pcm_record_t
{
	char* pcmbuf;
	int size;
}pcm_record_type;

int pcm_rx_task(unsigned long pData);
int pcm_tx_task(unsigned long pData);
int pcm_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

extern pcm_config_type* ppcm_config;
extern pcm_status_type* ppcm_status;

extern struct tasklet_struct phone_tasklet;
#define RALINK_PCM_VERSION	"v1.00"

#endif

