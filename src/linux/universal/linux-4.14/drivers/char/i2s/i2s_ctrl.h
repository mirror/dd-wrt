#ifndef __RALINK_I2S_H_
#define __RALINK_I2S_H_

#include <asm/rt2880/rt_mmap.h>

#define I2S_MAX_DEV			1
#define MOD_VERSION			"0.1"
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
#define NonCache2Virtual(x)             (((int)x) & 0xDFFFFFFF)

#define i2s_outw(address, value)	*((volatile uint32_t *)(address)) = cpu_to_le32(value)
#define i2s_inw(address)			le32_to_cpu(*(volatile u32 *)(address))


#define I2S_DEBUG
#ifdef I2S_DEBUG
#define MSG(fmt, args...) printk("I2S: " fmt, ## args)
#else
#define MSG(fmt, args...) { }
#endif



/* Register Map, Ref to RT3052 Data Sheet */

/* Register Map Detail */
#define I2S_I2SCFG				(RALINK_I2S_BASE+0x0000)
#define I2S_INT_STATUS			(RALINK_I2S_BASE+0x0004)
#define I2S_INT_EN				(RALINK_I2S_BASE+0x0008)
#define I2S_FF_STATUS			(RALINK_I2S_BASE+0x000c)
#define I2S_FIFO_WREG			(RALINK_I2S_BASE+0x0010)


/* I2SCFG bit field */
#define I2S_EN				31
#define I2S_DMA_EN			30
#define I2S_CLK_OUT_DIS		8
#define I2S_FF_THRES		4
#define I2S_CH_SWAP		3
#define I2S_CH1_OFF			2
#define I2S_CH0_OFF			1
#define I2S_SLAVE_EN		0

/* INT_EN bit field */
#define I2S_INT3_EN			3
#define I2S_INT2_EN			2
#define I2S_INT1_EN			1
#define I2S_INT0_EN			0

/* INT_STATUS bit field */
#define I2S_DMA_FAULT		3
#define I2S_OVRUN			2
#define I2S_UNRUN			1
#define I2S_THRES			0

/* FF_STATUS bit field */
#define I2S_AVCNT			4
#define I2S_EPCNT			0

/* FIFO_WREG bit field */
#define I2S_FIFO_WDATA		0

/* Constant definition */
#define NFF_THRES			4
#define I2S_PAGE_SIZE		(1152*2*2*2)
#define MAX_I2S_PAGE		12//12

#define MAX_SRATE_HZ			96000
#define MIN_SRATE_HZ			8000

#define MAX_VOL_DB				+0			
#define MIN_VOL_DB				-127

#define I2S_SRATE				0
#define I2S_VOL					1
#define I2S_ENABLE				2
#define I2S_DISABLE				3
#define I2S_GET_WBUF			4

#define CONFIG_I2S_TFF_THRES					NFF_THRES
#define CONFIG_I2S_CH_SWAP					0
#define CONFIG_I2S_SLAVE_EN					1
/* driver status definition */
#define I2S_OK						0
#define I2S_OUTOFMEM				0x01
#define I2S_GDMAFAILED				0x02
#define I2S_REQUEST_IRQ_FAILED		0x04
#define I2S_REG_SETUP_FAILED		0x08

//#define I2S_FIFO_MODE
//#define I2S_IN_CLKSRC
#define I2S_MS_MODE
#define I2S_STATISTIC
#define I2S_MAJOR			234

typedef struct i2s_status_t
{
	u32 txdmafault;
	u32 ovrun;
	u32 unrun;
	u32 thres;
	int buffer_unrun;
	int buffer_ovrun;
	int buffer_len;
}i2s_status_type;


typedef struct i2s_config_t
{

	int srate;
	int vol;
	u32	bufthres;
	u32	pos;
	u32  flag;
	u32	isr_cnt;
	int bSleep;
	int bDMAStart;
#ifdef __KERNEL__		
	spinlock_t lock;
	wait_queue_head_t i2s_qh;
#endif
	u32 dmach;
	u32 ff_thres;
	u32 ch_swap;
	u32 slave_en;
	
	int w_idx;
	int r_idx;
	
	u8* buf8ptr;	
	char* pMMAPBufPtr;

	union {
		u16* page0buf16ptr;	
		u8* page0buf8ptr;	
	};
	union {
		u16* page1buf16ptr;	
		u8* page1buf8ptr;	
	};

}i2s_config_type;

/* forward declarations for _fops */
static int i2s_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
static int i2s_mmap(struct file *file, struct vm_area_struct *vma);
static int i2s_open(struct inode *inode, struct file *file);
static int i2s_release(struct inode *inode, struct file *file);

int i2s_dev_open(i2s_config_type* ptri2s_config);
int i2s_dev_close(i2s_config_type* ptri2s_config);
int i2s_dev_enable(i2s_config_type* ptri2s_config);
int i2s_dev_disable(i2s_config_type* ptri2s_config);
void i2s_dma_handler(u32 dma_ch);

#define RALINK_I2S_VERSION	"v1.00"

#endif /* __RALINK_I2S_H_ */

