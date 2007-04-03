#include <linux/delay.h>
#include <linux/timer.h>

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/blkpg.h>
#include <linux/hdreg.h>
#include <linux/major.h>
#include <linux/time.h>
#include <asm/uaccess.h>
#include <asm/io.h>



#define DEVICE_NAME "mmc"
#define DEVICE_NR(device) (MINOR(device))
#define DEVICE_ON(device)
#define DEVICE_OFF(device)
#define MAJOR_NR 121

#include <linux/blk.h>
#include "config.h"


#define SD_DIV1 0x20
#define SD_DISHIFT1 0x2

#define SD_DIV4 0x04
#define SD_DISHIFT4 0x5

#define SD_DIVBUF 0x40
#define SD_DISHIFTBUF 0x1

#define SD_DIVBUF2 0x20
#define SD_DISHIFTBUF2 0x2

#define SD_DOWRT 0x10		// pin 4
#define SD_DOSHIFTWRT 0x3

#define SD_DOBUF 0x20		// pin 5
#define SD_DOSHIFTBUF 0x2

#define SD_DOBUF2 0x40		// pin 5
#define SD_DOSHIFTBUF2 0x1

/* GPIO pin 3 */
#define SD_CLK 0x08
/* GPIO pin 7 */
#define SD_CS 0x80

static int SD_DI = SD_DIV1;
static int SHIFT_DI = SD_DISHIFT1;
static int SD_DO = SD_DOWRT;
static int SHIFT_DO = SD_DOSHIFTWRT;



#if SD_DO == 0x10
	#define SHIFT_DO 3
#else
#if SD_DO == 0x40
	#define SHIFT_DO 1
#endif
#endif
#if SD_DI == 0x04
	#define SHIFT_DI 5
#else
#if SD_DI == 0x20
	#define SHIFT_DI 2
#endif
#endif


#include "log.c"



MODULE_AUTHOR("Madsuk/Rohde (speedup Cyril CATTIAUX v1.3.3)");
MODULE_DESCRIPTION("Driver MMC/SD-Cards");
MODULE_SUPPORTED_DEVICE("WRT54G");
MODULE_LICENSE("GPL");

static int mp_max_init_tries = 30000;
module_param(mp_max_init_tries, int, 0);
MODULE_PARM_DESC(mp_max_init_tries, "Number of max CMD0 sent in card init loop.");
static int mp_max_pwr_on_clocks = 80;
module_param(mp_max_pwr_on_clocks, int, 0);
MODULE_PARM_DESC(mp_max_pwr_on_clocks, "Number of max clocks sent to power on card.");

/* we have only one device */
static unsigned int hd_sizes[64];
static unsigned int hd_blocksizes[64];
static unsigned int hd_hardsectsizes[64];
static unsigned int hd_maxsect[64];
static struct hd_struct hd[64];

//static struct timer_list mmc_timer;
static int mmc_media_detect = 0;
static int mmc_media_changed = 1;

typedef unsigned int uint32;

static unsigned char port_state = 0x00;
static volatile unsigned char *gpioaddr_input = (unsigned char *)0xb8000060;
static volatile unsigned char *gpioaddr_output = (unsigned char *)0xb8000064;
static volatile unsigned char *gpioaddr_enable = (unsigned char *)0xb8000068;
//static volatile unsigned char *gpioaddr_control = (unsigned char *)0xb800006c;

static unsigned char ps_di, ps_di_clk, ps_clk;
static unsigned char NOT_DI_NOT_CLK;
static unsigned char DI_CLK;



static void setadapter(int type)
{
switch(type)
{
case 0: // WRT V1
SD_DI=SD_DIV1;
SHIFT_DI=SD_DISHIFT1;
SD_DO=SD_DOWRT;
SHIFT_DO=SD_DOSHIFTWRT;
break;
case 1: // WRT V4/GL
SD_DI=SD_DIV4;
SHIFT_DI=SD_DISHIFT4;
SD_DO=SD_DOWRT;
SHIFT_DO=SD_DOSHIFTWRT;
break;
case 2: // Buffalo
SD_DI=SD_DIVBUF;
SHIFT_DI=SD_DISHIFTBUF;
SD_DO=SD_DOBUF;
SHIFT_DO=SD_DOSHIFTBUF;
break;
case 3: // Buffalo
SD_DI=SD_DIVBUF2;
SHIFT_DI=SD_DISHIFTBUF2;
SD_DO=SD_DOBUF2;
SHIFT_DO=SD_DOSHIFTBUF2;
break;
}
NOT_DI_NOT_CLK=(~SD_DI) & (~SD_CLK);
DI_CLK=SD_DI | SD_CLK;
}



#define GET_RESULT_DO(shift_do) result |= (  ((shift_do)>=0)?( ((*l_gpioaddr_input) & SD_DO) << (shift_do) ):( ((*l_gpioaddr_input) & SD_DO) >> (-(shift_do)) )  );
#define SET_DI(bit_di,shift_di) di = (  ((shift_di)>=0)?( (data_out & bit_di) >> (shift_di) ):( (data_out & bit_di) << (-(shift_di)) )  );

static inline void mmc_spi_cs_low(void) {
  port_state &= ~(SD_CS);
  ps_di =(port_state|SD_DI);
  ps_di_clk =(port_state|DI_CLK);
  ps_clk =(port_state|SD_CLK);
  *gpioaddr_output = port_state;
}

static inline void mmc_spi_cs_high(void) {
  port_state |= SD_CS;
  ps_di =(port_state|SD_DI);
  ps_di_clk =(port_state|DI_CLK);
  ps_clk =(port_state|SD_CLK);
  *gpioaddr_output = port_state;
}

static inline void mmc_spi_clk(void) {
	// Send one CLK to SPI with DI high
	*gpioaddr_output = ps_di;
	*gpioaddr_output = ps_di_clk;
}

static inline void mmc_spi_io_ff_v(void) {
	const unsigned char l_ps_di = ps_di;
	const unsigned char l_ps_di_clk = ps_di_clk;
	volatile unsigned char * const l_gpioaddr_output = (unsigned char *) gpioaddr_output;
	
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
}

static inline unsigned char mmc_spi_io_ff(void) {
	const unsigned char l_ps_di = ps_di;
	const unsigned char l_ps_di_clk = ps_di_clk;
	volatile unsigned char * const l_gpioaddr_output = (unsigned char *) gpioaddr_output;
	volatile unsigned char * l_gpioaddr_input = (unsigned char *) gpioaddr_input;
	unsigned char result = 0;
	
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	GET_RESULT_DO(SHIFT_DO)
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	GET_RESULT_DO(SHIFT_DO-1)
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	GET_RESULT_DO(SHIFT_DO-2)
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	GET_RESULT_DO(SHIFT_DO-3)
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	GET_RESULT_DO(SHIFT_DO-4)
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	GET_RESULT_DO(SHIFT_DO-5)
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	GET_RESULT_DO(SHIFT_DO-6)
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	GET_RESULT_DO(SHIFT_DO-7)
	
	return result;
}

static inline void mmc_spi_io_v(const unsigned char data_out) {
	unsigned char di;
	const unsigned char l_port_state = port_state;
	const unsigned char l_ps_clk = ps_clk;
	volatile unsigned char * const l_gpioaddr_output = (unsigned char *) gpioaddr_output;

	SET_DI(0x80,SHIFT_DI)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	SET_DI(0x40,SHIFT_DI-1)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	SET_DI(0x20,SHIFT_DI-2)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	SET_DI(0x10,SHIFT_DI-3)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	SET_DI(0x08,SHIFT_DI-4)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	SET_DI(0x04,SHIFT_DI-5)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	SET_DI(0x02,SHIFT_DI-6)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	SET_DI(0x01,SHIFT_DI-7)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
}

static unsigned char mmc_spi_io(unsigned char data_out) {
	unsigned char result = 0;
	unsigned char di;
	const unsigned char l_port_state = port_state;
	const unsigned char l_ps_clk= ps_clk;
	volatile unsigned char * const l_gpioaddr_output = (unsigned char *) gpioaddr_output;
	volatile unsigned char * const l_gpioaddr_input = (unsigned char *) gpioaddr_input;
		
	SET_DI(0x80,SHIFT_DI)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	GET_RESULT_DO(SHIFT_DO)
	SET_DI(0x40,SHIFT_DI-1)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	GET_RESULT_DO(SHIFT_DO-1)
	SET_DI(0x20,SHIFT_DI-2)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	GET_RESULT_DO(SHIFT_DO-2)
	SET_DI(0x10,SHIFT_DI-3)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	GET_RESULT_DO(SHIFT_DO-3)
	SET_DI(0x08,SHIFT_DI-4)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	GET_RESULT_DO(SHIFT_DO-4)
	SET_DI(0x04,SHIFT_DI-5)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	GET_RESULT_DO(SHIFT_DO-5)
	SET_DI(0x02,SHIFT_DI-6)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	GET_RESULT_DO(SHIFT_DO-6)
	SET_DI(0x01,SHIFT_DI-7)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	GET_RESULT_DO(SHIFT_DO-7)
 
	return(result);
}

static inline int mmc_write_block(unsigned int dest_addr, unsigned char *data, int nbsectors) {
	unsigned char r = 0;
	unsigned int i;
	int k;
	unsigned char * pdata = data;
	//printk("nbsectors: %d\n", nbsectors);
	mmc_spi_cs_low();
	/* wait */
	for (i = 0; i < 4; i++) mmc_spi_io_ff_v();

#ifdef USE_CMD25	
	mmc_spi_io(0x59);	/* CMD25 */
#else
	mmc_spi_io(0x58);	/* CMD24 */
#endif
	mmc_spi_io_v(0xff & (dest_addr >> 24)); /* msb */
	mmc_spi_io_v(0xff & (dest_addr >> 16));
	mmc_spi_io_v(0xff & (dest_addr >> 8));
	mmc_spi_io_v(0xff & dest_addr); /* lsb */
	mmc_spi_io_ff_v();	/* dummy CRC */
	
	for (i = 0; i < 9; i++) {
		r = mmc_spi_io_ff();	/*  command response */
		if (r != 0xff) break;
	}
	if (r != 0x00) {
		mmc_spi_cs_high();
		mmc_spi_io_ff_v();
		return(-r);
	}

#ifdef USE_CMD25
	for (k = 0; k<nbsectors; k++) {
		/* data token */
		mmc_spi_io_v(0xfc);
#else
		/* data token */
		mmc_spi_io_v(0xfe);
#endif
		
		/* data block */
		i=0;
		while (i < 512) {
			mmc_spi_io_v(*pdata); pdata++; i++;
			mmc_spi_io_v(*pdata); pdata++; i++;
			mmc_spi_io_v(*pdata); pdata++; i++;
			mmc_spi_io_v(*pdata); pdata++; i++;
		}
		
		/* dummy CRC */
		mmc_spi_io_ff_v();
		mmc_spi_io_ff_v();

		for (i = 0; i < 9; i++) {
			r = mmc_spi_io_ff();	/*  data response */
			if (r != 0xff) break;
		}
		if (!(r & 0x05)) {
			// data rejected
			mmc_spi_cs_high();
			mmc_spi_io_ff_v();
			return(2);
		}

		/* busy...*/
		{
			for (i = 0; i < 1000000; i++) {
				r = mmc_spi_io_ff();
				if (r == 0xff) break;
			}
			if (r != 0xff) {
				mmc_spi_cs_high();
				mmc_spi_io_ff_v();
				return(3);
			}
			#if LOG_LEVEL >= 4
			log_debug("mmc_write_block: busy after a block write last %d iterations.");
			#endif
		}
#ifdef USE_CMD25
	}
	
	mmc_spi_io_v(0xfd);
	mmc_spi_io_ff_v();	/* dummy args */
	mmc_spi_io_ff_v();
	mmc_spi_io_ff_v();
	mmc_spi_io_ff_v();
	mmc_spi_io_ff_v();	/* dummy CRC */

	mmc_spi_io_ff_v();	// skipping one byte
	//busy...
	for (i = 0; i < 1000000; i++) {
		yield();
		r = mmc_spi_io_ff();
		if (r == 0xff) break;
	}
	if (r != 0xff) {
		mmc_spi_cs_high();
		mmc_spi_io_ff_v();
		return(-4);
	}
#endif
	mmc_spi_cs_high();
	mmc_spi_io_ff_v();
	return(0);
}

static inline int mmc_read_block(unsigned char *data, unsigned int src_addr, int nbsectors) {
	unsigned char r = 0;
	unsigned int i;
	int k;
	unsigned char * pdata = data;

	mmc_spi_cs_low();
	/* wait */
	for (i = 0; i < 4; i++) mmc_spi_io_ff_v();
	

#ifdef USE_CMD18
	mmc_spi_io_v(0x52);	/* CMD18 */
#else
	mmc_spi_io_v(0x51);	/* CMD17 */
#endif
	mmc_spi_io_v(0xff & (src_addr >> 24)); /* msb */
	mmc_spi_io_v(0xff & (src_addr >> 16));
	mmc_spi_io_v(0xff & (src_addr >> 8));
	mmc_spi_io_v(0xff & src_addr); /* lsb */
	mmc_spi_io_ff_v();	/* dummy CRC */

	for (i = 0; i < 9; i++) {
		r = mmc_spi_io_ff();	/*  command response */
		if (r != 0xff) break;
	}
	if (r != 0x00) {
		mmc_spi_cs_high();
		mmc_spi_io_ff_v();
		return(-r);
	}

#ifdef USE_CMD18
	for (k = 0; k < nbsectors; k++) {
#endif
		for (i = 0; i < 1000000; i++) {
			r = mmc_spi_io_ff();
			if (r != 0xff) break;
		}
		if (r != 0xfe) {
			mmc_spi_cs_high();
			mmc_spi_io_ff_v();
			return 3;
		}

		#if LOG_LEVEL>=5
			log_trace("Reading a block. Dump: ");
		#endif
		/* reading data packet */
		for (i = 0; i < 512; i++) {
			r = mmc_spi_io_ff();
			#if LOG_LEVEL>=5
				if (i!=0 && i%16==0) printk("\n");
				printk("%02x ", r);
			#endif
			*pdata = r;
			pdata++;
		}
		#if LOG_LEVEL>=5
			printk("\n");
		#endif
		/* skipping crc */
		mmc_spi_io_ff_v();
		mmc_spi_io_ff_v();
#ifdef USE_CMD18
	}
	yield();
	
	mmc_spi_io_v(0x4c);	/* CMD12 */
	mmc_spi_io_ff_v();	/* dummy args */
	mmc_spi_io_ff_v();
	mmc_spi_io_ff_v();
	mmc_spi_io_ff_v();
	mmc_spi_io_ff_v();	/* dummy CRC */
	
	mmc_spi_io_ff_v(); /* skipping stuff byte */
	
	/* skipping 1-8 bytes and get R1 */
	for (i = 0; i < 9; i++) {
		r = mmc_spi_io_ff();
		if (r != 0xff) break;
	}
	if (r != 0x00) {
		mmc_spi_cs_high();
		mmc_spi_io_ff_v();
		return 4;
	}

#endif

	mmc_spi_cs_high();
	mmc_spi_io_ff_v();

	return(0);
}


static void mmc_request(request_queue_t *q) {
	unsigned int mmc_address;
	unsigned char *buffer_address;
	unsigned int nr_sectors;
#ifndef USE_CMD18
	int i;
#endif
	int cmd;
	int rc, code;
	
	(void)q;
	while (1) {
		code = 1; // Default is success
		INIT_REQUEST;
		mmc_address = ((unsigned int)(CURRENT->sector + hd[MINOR(CURRENT->rq_dev)].start_sect)) << 9;
		buffer_address = CURRENT->buffer;
		nr_sectors = CURRENT->current_nr_sectors;
		cmd = CURRENT->cmd;
		#if LOG_LEVEL>=5
		log_trace("Request: CURRENT->cmd: %02x, CURRENT->sector: %02x, CURRENT->current_nr_sectors: %02x, ",cmd, CURRENT->sector, nr_sectors);
		#endif
		
		if (((CURRENT->sector + CURRENT->current_nr_sectors + hd[MINOR(CURRENT->rq_dev)].start_sect) > hd[0].nr_sects) || (mmc_media_detect == 0)) {
			code = 0;
		} else if (cmd == READ) {
			spin_unlock_irq(&io_request_lock);
#ifndef USE_CMD18
			for (i = 0; i < nr_sectors; i++) {
#endif
				rc = mmc_read_block(buffer_address, mmc_address, nr_sectors);
				if (rc != 0) {
					log_error("mmc_request: read error: %02x", rc);
					code = 0;
					break;
				} else {
#ifdef USE_CMD18
					mmc_address += nr_sectors << 9;
					buffer_address += nr_sectors << 9;
#else
					mmc_address += 512;
					buffer_address += 512;
#endif
				}
#ifndef USE_CMD18
			}
#endif
			spin_lock_irq(&io_request_lock);
		} else if (cmd == WRITE) {
			spin_unlock_irq(&io_request_lock);
#ifndef USE_CMD25
			for (i = 0; i < nr_sectors; i++) {
#endif
				rc = mmc_write_block(mmc_address, buffer_address, nr_sectors);
				if (rc != 0) {
					log_error("mmc_request: write error: %02x", rc);
					code = 0;
					break;
				} else {
#ifdef USE_CMD25
					mmc_address += nr_sectors << 9;
					buffer_address += nr_sectors << 9;
#else
					mmc_address += 512;
					buffer_address += 512;
#endif
				}
#ifndef USE_CMD25
			}
#endif
			spin_lock_irq(&io_request_lock);
		} else {
			code = 0;
		}
		end_request(code);
	}
}


static int mmc_open(struct inode *inode, struct file *filp) {
	(void)filp;
	(void)inode;

	if (mmc_media_detect == 0) return -ENODEV;

#if defined(MODULE)
	MOD_INC_USE_COUNT;
#endif
	return 0;
}

static int mmc_release(struct inode *inode, struct file *filp) {
	(void)filp;
	fsync_dev(inode->i_rdev);
        invalidate_buffers(inode->i_rdev);

#if defined(MODULE)
	MOD_DEC_USE_COUNT;
#endif
	return 0;
}

extern struct gendisk hd_gendisk;

static int mmc_revalidate(kdev_t dev) {
	int target, max_p, start, i;
	if (mmc_media_detect == 0) return -ENODEV;
	
	target = DEVICE_NR(dev);

	max_p = hd_gendisk.max_p;
	start = target << 6;
	for (i = max_p - 1; i >= 0; i--) {
		int minor = start + i;
		invalidate_device(MKDEV(MAJOR_NR, minor), 1);
		hd_gendisk.part[minor].start_sect = 0;
		hd_gendisk.part[minor].nr_sects = 0;
	}
	
	grok_partitions(&hd_gendisk, target, 64, hd[0].nr_sects);

	return 0;
}

static int mmc_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg) {
	(void)filp;

	if (!inode || !inode->i_rdev) return -EINVAL;

	switch(cmd) {
	case BLKGETSIZE:
		return put_user(hd[MINOR(inode->i_rdev)].nr_sects, (unsigned long *)arg);
	case BLKGETSIZE64:
		return put_user((u64)hd[MINOR(inode->i_rdev)].nr_sects, (u64 *) arg);
	case BLKRRPART:
		if (!capable(CAP_SYS_ADMIN)) return -EACCES;

		return mmc_revalidate(inode->i_rdev);
	case HDIO_GETGEO:
		{
		struct hd_geometry *loc, g;
		loc = (struct hd_geometry *) arg;
		if (!loc)
			return -EINVAL;
		g.heads = 4;
		g.sectors = 16;
		g.cylinders = hd[0].nr_sects / (4 * 16);
		g.start = hd[MINOR(inode->i_rdev)].start_sect;
		return copy_to_user(loc, &g, sizeof(g)) ? -EFAULT : 0;
		}
	default:
		return blk_ioctl(inode->i_rdev, cmd, arg);
	}
}

// this function was contributed by: rcichielo from openwrt forums
/**
 * Comments added by Marc DENTY on 2007-03-20
 *
 * Sequence to read a card's "CID" bytes (name, serial number etc)
 *
 * Send: 4ah,00h,00h,00h,00h,00h  - CMD10, no args, null CRC
 * Read: xx                       - NCR Time
 * Read: xx                       - Command Response (Should be 00h)
 * Read: until FEh is received    - Wait for Data token
 * Read: yy  * 16                 - Get 16 bytes from CID
 * Read: zz                       - Read CRC lo byte
 * Read: zz                       - Read CRC hi byte
 *
 * Useful locations in the returned data packet:
 *
 * 03h-08h Manufacturers's name in ascii
 * 0ah-0dh Card's 32 bit serial number 
 */
static int mmc_read_cid(unsigned char *cid) {
	unsigned char r = 0;
        int i;

	mmc_spi_cs_low();
	/* wait */
	for (i = 0; i < 4; i++) mmc_spi_io_ff_v();
	
	/* issue CID (card identification data) read request */
	mmc_spi_io_v(0x4a);
	mmc_spi_io_v(0x00); /* param: 0x00000000 */
	mmc_spi_io_v(0x00);
	mmc_spi_io_v(0x00);
	mmc_spi_io_v(0x00);
	mmc_spi_io_ff_v();	/* dummy crc */
	/* skip NR and get response */
	for (i = 0; i < 9; i++) {
		r = mmc_spi_io_ff();
		if (r != 0xff) break;
	}
	if (r != 0x00) {
		mmc_spi_cs_high();
		mmc_spi_io_ff_v();
		return(1);
	}
	for (i = 0; i < 8; i++) {
		r = mmc_spi_io_ff();
		if (r == 0xfe) break;
	}
	if (r != 0xfe) {
		mmc_spi_cs_high();
		mmc_spi_io_ff_v();
		return(2);
	}

	for (i = 0; i < 16; i++) {
		r = mmc_spi_io_ff();
		cid[i] = r;
	}
	
	mmc_spi_io_ff_v();
	mmc_spi_io_ff_v();
	
	mmc_spi_cs_high();
	mmc_spi_io_ff_v();

	return 0;
}

/* 
   CMD16 [31:0] block R1 SET_BLOCKLEN Selects a block length (in
         length                       bytes) for all following block
                                      commands (read & write).
   Contributed by Marc DENTY
 */
static int mmc_set_blocklen(unsigned int len) {
	unsigned char r = 0;
        int i;

        mmc_spi_cs_low();
        /* wait */
        for (i = 0; i < 4; i++) mmc_spi_io_ff_v();

        /* issue CID (card identification data) read request */
        mmc_spi_io_v(0x40 | 0x10);
        mmc_spi_io_v((len >> 24) & 0xFF); 
        mmc_spi_io_v((len >> 16) & 0xFF);
        mmc_spi_io_v((len >>  8) & 0xFF);
        mmc_spi_io_v(len & 0xFF);
        mmc_spi_io_ff_v();      /* dummy crc */
        /* skip NR and get response */
        for (i = 0; i < 9; i++) {
                r = mmc_spi_io_ff();
                if (r != 0xff) break;
        }
        if (r != 0x00) {
                mmc_spi_cs_high();
                mmc_spi_io_ff_v();
                return(1);
        }
	return 0;
}

static int mmc_card_init2(void) {
	unsigned char r = 0;
	short i, j;
	unsigned long flags;

	save_flags(flags);
	cli();

	log_info("mmc_card_init: powering card on. sending %d CLK", mp_max_pwr_on_clocks);
	mmc_spi_cs_high();
	for (i = 0; i < mp_max_pwr_on_clocks; i++) mmc_spi_clk();
	mmc_spi_cs_low();
	/* wait */
	for (i = 0; i < 4; i++) mmc_spi_io_ff_v();

	log_info("mmc_card_init: resetting card (CMD0)");
	mmc_spi_io_v(0x40);	/* CMD0 */
	mmc_spi_io_v(0x00);	/* param: 0x00000000 */
	mmc_spi_io_v(0x00);
	mmc_spi_io_v(0x00);
	mmc_spi_io_v(0x00);
	mmc_spi_io_v(0x95);	/* CRC7 */

	/* skipping NR and get response */
	for (i = 0; i < 9; i++) {
		r = mmc_spi_io_ff();
#if LOG_LEVEL >= 5
		log_trace("mmc_card_init: NR after CMD0 : %02x", r);
#endif
		if (r == 0x01) break;
	}
	
	mmc_spi_cs_high();
	mmc_spi_io_ff_v();
	if (r != 0x01) {
		log_fatal("mmc_card_init: invalid response from card: %02x found, waiting for %02x", r, 0x01);
		restore_flags(flags);
		return(1);
	}

	log_info("mmc_card_init: doing initialization loop");
	{
	struct timeval t1, t2;
	do_gettimeofday(&t1);
	for (j = 0; j < mp_max_init_tries; j++) {
		mmc_spi_cs_low();
#if 0
		/* ACMD41 - sd init */
		/* CMD55 */
		mmc_spi_io_v(0x77);
		mmc_spi_io_v(0x00); /* param: 0x00000000 */
		mmc_spi_io_v(0x00);
		mmc_spi_io_v(0x00);
		mmc_spi_io_v(0x00);
		mmc_spi_io_ff_v();	/* dummy crc */
		/* skipping NR and get reponse */
		for (i = 0; i < 9; i++) {
			r = mmc_spi_io_ff(); 
#if LOG_LEVEL >= 5
			log_trace("mmc_card_init: NR after ACMD41.CMD55 : %02x", r);
#endif
			if (r != 0xff) break;
		}
		if (r != 0x01) {
			mmc_spi_cs_high();
			mmc_spi_io_ff_v();
			log_fatal("mmc_card_init: ACMD41 failed in first part (CMD55) with status %02x", r);
			return (4);
		}
		
		/* CMD41 */
		mmc_spi_io_v(0x69);
		mmc_spi_io_v(0x00); /* param: 0x00000000 */
		mmc_spi_io_v(0x00);
		mmc_spi_io_v(0x00);
		mmc_spi_io_v(0x00);
		mmc_spi_io_ff_v();	/* dummy crc */
#else
		/* CMD1 - init */
		mmc_spi_io_v(0x41);
		mmc_spi_io_v(0x00); /* param: 0x00000000 */
		mmc_spi_io_v(0x00);
		mmc_spi_io_v(0x00);
		mmc_spi_io_v(0x00);
		mmc_spi_io_ff_v();	/* dummy crc */
#endif
		/* skipping NR and get reponse */
		for (i = 0; i < 9; i++) {
			r = mmc_spi_io_ff(); 
#if LOG_LEVEL >= 5
			log_trace("mmc_card_init: NR after ACMD41.CMD41 : %02x", r);
#endif
			if (r !=0xff) break;
		}
		mmc_spi_cs_high();
		mmc_spi_io_ff();
		if ((r & 0x01) == 0) {
			/* card is not idle anymore */
			restore_flags(flags);
			do_gettimeofday(&t2);
			log_info("mmc_card_init: card inited successfully in %d tries (%d seconds %d usec).", j+1, t2.tv_sec-t1.tv_sec, t2.tv_usec-t1.tv_usec);
			return(0);
		}
		yield();
	}
	}
	restore_flags(flags);

	log_error("mmc_card_init: card not successfully inited after %d tries.", j);
	return(2);
}

static int mmc_card_config(void) {
	unsigned char r = 0;
	short i;
	unsigned char csd[32];
	unsigned int c_size;
	unsigned int c_size_mult;
	unsigned int mult;
	unsigned int read_bl_len;
	unsigned int blocknr = 0;
	unsigned int block_len = 0;
	unsigned int size = 0;

	mmc_spi_cs_low();
	/* wait */
	for (i = 0; i < 4; i++) mmc_spi_io_ff_v();

	/* CMD9 - Query CSD (card specific data) */
	mmc_spi_io_v(0x49);
	mmc_spi_io_v(0x00);	/* dummy param */
	mmc_spi_io_v(0x00);
	mmc_spi_io_v(0x00);
	mmc_spi_io_v(0x00);
	mmc_spi_io_ff_v();	/* dummy CRC7 */
	/* skip NR and get response */
	for (i = 0; i < 9; i++) {
		r = mmc_spi_io_ff();
		if (r != 0xff) break;
	}
	/* command response should be 0 */
	if (r != 0x00) {
		mmc_spi_cs_high();
		mmc_spi_io_ff();
		return(1);
	}
	/* Wait for 0xfe value (Data token) */
	for (i = 0; i < 8; i++) {
		r = mmc_spi_io_ff();
		if (r == 0xfe) break;
	}
	if (r != 0xfe) {
		mmc_spi_cs_high();
		mmc_spi_io_ff();
		return(2);
	}
	/* get 16 bytes from CSD register */
	for (i = 0; i < 16; i++) {
		r = mmc_spi_io_ff();
		csd[i] = r;
	}
	/* get 2 bytes (CRC low & CRC hi)*/
	for (i = 0; i < 2; i++) {
		r |= mmc_spi_io_ff();
	}
	mmc_spi_cs_high();
	mmc_spi_io_ff();
	/* if CRC == 0 return 3, CRC cannot be 0 ? */
	if (r == 0x00) return(3);
/*
 * Among the useful data in the 16 byte packet is the capacity of the card. Unfortunately its a little cryptic and must be decoded thus:
 *
 * Byte Locations:
 *
 * 06h,07h,08h : (contents AND 00000011 11111111b 11000000b) >> 6 = "Device size (C_Size)"
 * 09h,0ah : (contents AND 00000011 10000000b) >> 7 = "Device size multiplier (C_Mult)"
 * 05h : (contents AND 00001111b) = Sector size ("Read_BL_Len")
 *
 * When you have the 12 bit "C_Size", 3 Bit "C_Mult" and 4 bit "Read_BL_Len" you need to follow the formula:
 *
 * Capacity in bytes = (C_Size+1) * (2 ^ (C_Mult+2)) * (2 ^ Read_BL_Len)
 *
 * (Note: The computed sector size (2 ^ Read_BL_len) is normally 512 bytes)  */

	c_size = (csd[8] & 0xC0) + (csd[7] << 8) + ((csd[6] & 0x03) << 16);
	c_size >>= 6;
	c_size_mult = (csd[10] & 0x80) + ((csd[9] & 0x03) << 8);
	c_size_mult >>= 7;
	read_bl_len = csd[5] & 0x0f;
	mult = 1;
	mult <<= c_size_mult + 2;
	blocknr = (c_size + 1) * mult;
	block_len = 1;
	block_len <<= read_bl_len;
	size = block_len * blocknr;
	size >>= 10;

	for(i=0; i<64; i++) {
	  hd_blocksizes[i] = 1024;
	  hd_hardsectsizes[i] = 512;
	  hd_maxsect[i] = 256;
	}
	hd_sizes[0] = size;
	hd[0].nr_sects = size<<1;


	log_info("mmc_card_config: size = %d, hardsectsize = %d, sectors = %d", size, block_len, blocknr);

	return 0;
}

static int mmc_hardware_init(void) {
  unsigned char gpio_outen;
  
  // Set inputs/outputs here
  log_info("mmc_hardware_init: initializing GPIOs");
  gpio_outen = *gpioaddr_enable;
  
  gpio_outen = (gpio_outen | SD_DI | SD_CLK | SD_CS) & ~SD_DO;
  *gpioaddr_enable = gpio_outen;
  
  port_state = *gpioaddr_input;
  
  // Clock low
  port_state &= ~(SD_CLK | SD_DI | SD_CS);
  *gpioaddr_output = port_state;

  return 0;
}

#if 0
static int mmc_check_media_change(kdev_t dev) {
	(void)dev;
	if (mmc_media_changed == 1) {
		mmc_media_changed = 0;
		return 1;
	}
	else return 0;
}
#endif

static struct block_device_operations mmc_bdops =  {
	open: mmc_open,
	release: mmc_release,
	ioctl: mmc_ioctl,
#if 0
	check_media_change: mmc_check_media_change,
	revalidate: mmc_revalidate,
#endif
};

static struct gendisk hd_gendisk = {
	major:		MAJOR_NR,
	major_name:	DEVICE_NAME,
	minor_shift:	6,
	max_p:		64,
	part:		hd,
	sizes:		hd_sizes,
	fops:		&mmc_bdops,
};

static int mmc_card_init(void)
{
int rc;
	rc = mmc_card_init2();
	if ( rc != 0) {
		rc = mmc_card_init2(); 
		if ( rc != 0) {
			log_error("mmc_init: got an error calling mmc_card_init: %02x", rc);
			return -1;
		}
	}
return rc;
}

static adapter_found=0;
static int mmc_init(void) {
	int rc;


	if ( rc != 0) {
		log_error("mmc_init: got an error calling mmc_hardware_init: %02x", rc);
		return -1;
	}
	if (adapter_found==0)
	    {
	    printk(KERN_EMERG "mmc_init: trying old WRT54G gpio layout\n");
	    setadapter(0);
	    }
	rc = mmc_hardware_init(); 
        if (rc==0)
	rc = mmc_card_init();
	if (rc!=0)
	    {
	    printk(KERN_EMERG "mmc_init: trying new WRT54G/GL gpio layout\n");
	    setadapter(1);
	    rc = mmc_hardware_init(); 
            if (rc==0)
	    rc = mmc_card_init();
	    if (rc!=0)
		{
		printk(KERN_EMERG "mmc_init: Buffalo WHR gpio layout\n");
		setadapter(2);
		rc = mmc_hardware_init(); 
                if (rc==0)
		rc = mmc_card_init();
		if (rc!=0)
		    {
		    printk(KERN_EMERG "mmc_init: Buffalo WHR gpio layout 2\n");
		    setadapter(3);
		    rc = mmc_hardware_init(); 
		    if (rc==0)
		    rc = mmc_card_init();
		    if (rc!=0)
			return -1;
		    }
		}
	    
	    }
	 

	{
		int i;
		unsigned char cid[16];
		rc = mmc_read_cid(cid);
		if (rc == 0) {
			log_info("mmc_init: MMC/SD Card ID:");
			for (i=0; i<16; i++) {
				printk("%02x ", cid[i]);
			}
			cid[9] = 0;
			printk("\nManufacturer : %s, cardUID = %x",&cid[3], *(int*)(&cid[0x0a]));
			printk("\n");
		} else {
			log_warn("mmc_init: impossible to get card indentification info for reason code: %02x", rc);
		}
	}
	
	memset(hd_sizes, 0, sizeof(hd_sizes));
	memset(hd, 0, sizeof(hd));
	
	rc = mmc_card_config(); 
	if ( rc != 0) {
		log_error("mmc_init: got an error when trying to get card configuration registers: %02x", rc);
		return -1;
	}
	log_warn("mmc_init: hd_sizes=%d, hd[0].nr_sects=%d", hd_sizes[0], hd[0].nr_sects);
	
	/* Send CMD16 to fix block size to 512 as other sizes don't work for all cards */
	if(mmc_set_blocklen(512) == 0) {
		log_info("mmc_card_init: set_blocklen (CMD16) succeeded !");
	} else {
		log_warn("mmc_card_init: set_blocklen (CMD16) failed !");
	}
	
	blk_size[MAJOR_NR] = hd_sizes;
	blksize_size[MAJOR_NR] = hd_blocksizes;
	hardsect_size[MAJOR_NR] = hd_hardsectsizes;
	max_sectors[MAJOR_NR] = hd_maxsect;
	hd_gendisk.nr_real = 1;

	/* System will now read the partition table using mmc_request */
	register_disk(&hd_gendisk, MKDEV(MAJOR_NR,0), 64, &mmc_bdops, hd[0].nr_sects);

	return 0;
}

static void mmc_exit(void) {
	blk_size[MAJOR_NR] = NULL;
	blksize_size[MAJOR_NR] = NULL;
	hardsect_size[MAJOR_NR] = NULL;
	max_sectors[MAJOR_NR] = NULL;
	hd[0].nr_sects = 0;
}

static void mmc_check_media(void) {
	int old_state;
	int rc;
	old_state = mmc_media_detect; 

	// TODO: Add card detection here
	mmc_media_detect = 1;
	if (old_state != mmc_media_detect)  {
		mmc_media_changed = 1;
		if (mmc_media_detect == 1) {
			rc = mmc_init();
			if (rc != 0) log_error("mmc_check_media: change detected but was not able to initialize new card: %02x", rc);
		}
		else  {
			mmc_exit();
		}
	}

	/* del_timer(&mmc_timer);
	mmc_timer.expires = jiffies + 10*HZ;
	add_timer(&mmc_timer); */
}

static int __init mmc_driver_init(void) {
	int rc;
	
	rc = devfs_register_blkdev(MAJOR_NR, DEVICE_NAME, &mmc_bdops);
	if (rc < 0) {
		log_fatal("mmc_driver_init: can't get major number %d", MAJOR_NR);
		return rc;
	}
	
	blk_init_queue(BLK_DEFAULT_QUEUE(MAJOR_NR), mmc_request);
	read_ahead[MAJOR_NR] = 8;
	add_gendisk(&hd_gendisk);

	mmc_check_media();

	/*init_timer(&mmc_timer);
	mmc_timer.expires = jiffies + HZ;
	mmc_timer.function = (void *)mmc_check_media;
	add_timer(&mmc_timer);*/

	return 0;
}

static void __exit mmc_driver_exit(void) {
	int i;
	/*del_timer(&mmc_timer);*/

	for (i = 0; i < (64); i++)
		fsync_dev(MKDEV(MAJOR_NR, i));

	blk_cleanup_queue(BLK_DEFAULT_QUEUE(MAJOR_NR));
	del_gendisk(&hd_gendisk);
	devfs_unregister_blkdev(MAJOR_NR, DEVICE_NAME);

	mmc_exit();
}

module_init(mmc_driver_init);
module_exit(mmc_driver_exit);
