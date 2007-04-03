#include <linux/kernel.h>
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

MODULE_AUTHOR("Madsuk/Rohde/Cyril CATTIAUX/Marc DENTY/rcichielo");
MODULE_DESCRIPTION("Driver MMC/SD-Cards MOD 1.3.4");
MODULE_SUPPORTED_DEVICE("WRT54G");
MODULE_LICENSE("GPL");

#include "config.h"
#include "log.c"
#include "mmc.h"
#include "spi.c"
#include "gpio.c"

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
	r = mmc_spi_send_cmd(25, dest_addr);
#else
	r = mmc_spi_send_cmd(24, dest_addr);
#endif
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
	r = mmc_spi_send_cmd(18, src_addr);
#else
	r = mmc_spi_send_cmd(17, src_addr);
#endif
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
	
	r = mmc_spi_send_cmd_skip(12, 0xffffffff, 1);
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

/**
 * this function was contributed by: rcichielo from openwrt forums
 *
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
/**
 * Comments added by Cyril CATTIAUX on 2007-03-21
 *
 * CID format specification (from Sandisk SD Product Manual v1.9)
 *
 * cid[00   ] Manufacturer ID (unsigned byte)
 * cid[01-02] OEM/Application ID (ASCII)
 * cid[03-07] Product Name (ASCII)
 * cid[08   ] Product Revistion (BCD coded number)
 * cid[09-12] Serial Number (32-bit unsigned int)
 * cid[13-14] Reserved(bit 12->15) - Manufacture Date(bit 0->11)
 * cid[15   ] CRC7(bit 1->7) - Not used, allways 1 (bit 0)
*/
static int mmc_read_cid(unsigned char *cid) {
	unsigned char r = 0;
        int i;

	mmc_spi_cs_low();
	/* wait */
	for (i = 0; i < 4; i++) mmc_spi_io_ff_v();
	
	/* issue CID (card identification data) read request */
	r = mmc_spi_send_cmd(10, 0x00000000);
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
        r = mmc_spi_send_cmd(16, len);
        if (r != 0x00) {
                mmc_spi_cs_high();
                mmc_spi_io_ff_v();
                return(1);
        }
	return 0;
}

extern unsigned long loops_per_jiffy;
/**
 * search the number of cycles (approximately) that this CPU can
 * do per sec. (max arround 4,3GHz - be careful! if greater it will wrap arround.)
*/
static inline cycles_t get_cpu_cycles_per_sec(void) {	
	return loops_per_jiffy*(HZ<<1);
}
#define KHz 1000

static int mmc_card_init(void) {
	unsigned char r = 0;
	short i, j;
	unsigned long flags;
	cycles_t t1, t2, delta;

	/* FREQUENCY LIMIT:
	 This line gets a reference in CPU cycles to output a frequency of 380Khz
	 (MMC specifications states that init phase must be done with a
	 clock frequency slower than 400KHz to be backward compatible with
	 traditionnal MMCs).
	 380KHz*2 because we need the half period of a frequency of 380KHz :
	*/
	hfp_380khz = get_cpu_cycles_per_sec() / (380*KHz*2);
	log_info("mmc_card_init: the period of a 380KHz frequency lasts %d CPU cycles", hfp_380khz<<1);

	save_flags(flags);
	cli();

	log_info("mmc_card_init: powering card on. sending %d CLK", mp_max_pwr_on_clocks);
	mmc_spi_cs_high();
	t1 = get_cycles();
	for (i = 0; i < mp_max_pwr_on_clocks; i++) mmc_spi_clk_fp(&last_clk, hfp_380khz);
	t2 = get_cycles(); if (t1 > t2) delta = t1-t2; else delta = t2-t1;
	log_info("mmc_card_init: %d CLK sent in %d CPU cycles", mp_max_pwr_on_clocks, delta);
	mmc_spi_cs_low();
	/* wait */
	for (i = 0; i < 4; i++) mmc_spi_io_ff_v_fp(&last_clk, hfp_380khz);

	log_info("mmc_card_init: resetting card (CMD0)");
	
	/* CMD0 - reset */
	r = mmc_spi_send_cmd_crc7_fp(&last_clk, hfp_380khz, 0, 0x00000000, 0x95);	/* crc7 pre-computed to 0x95 */
	mmc_spi_cs_high();
	mmc_spi_io_ff_v_fp(&last_clk, hfp_380khz);
	if (r != 0x01) {
		log_fatal("mmc_card_init: invalid response from card: %02x found, waiting for %02x", r, 0x01);
		restore_flags(flags);
		return(1);
	}

	log_info("mmc_card_init: doing initialization loop");

	t1 = get_cycles();
	for (j = 0; j < mp_max_init_tries; j++) {
		mmc_spi_cs_low();
		
		/* CMD1 - init */
		r = mmc_spi_send_cmd_fp(&last_clk, hfp_380khz, 1, 0x00000000);
		mmc_spi_cs_high();
		mmc_spi_io_ff_fp(&last_clk, hfp_380khz);
		if ((r & 0x01) == 0) {
			/* card is not idle anymore */
			restore_flags(flags);
			t2 = get_cycles(); if (t1 > t2) delta = t1-t2; else delta = t2-t1;
			log_info("mmc_card_init: card inited successfully in %d tries (%d CPU cycles).", j+1, delta);
			return(0);
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
	r = mmc_spi_send_cmd(9, 0x00000000);
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

/**
 * Comments added by Cyril CATTIAUX on 2007-03-21
 *
 * CID format specification (from Sandisk SD Product Manual v1.9)
 *
 * cid[00   ] Manufacturer ID (unsigned byte)
 * cid[01-02] OEM/Application ID (ASCII)
 * cid[03-07] Product Name (ASCII)
 * cid[08   ] Product Revision (BCD coded 2 digit number)
 * cid[09-12] Serial Number (32-bit unsigned int)
 * cid[13-14] Manufacture Date(bit 0->11) (BCD coded 3 digit number YYM offset from 2000) - Reserved(bit 12->15)
 * cid[15   ] Not used, allways 1 (bit 0) - CRC7(bit 1->7)
*/
static void mmc_show_cid_info(void) {
	int i, rc;
	unsigned short tmps;
	unsigned char  cid[16];
	
	char           manufacturer_id;
	char           oem_id[3];
	char           product_name[6];
	unsigned char  product_revision_h, product_revision_l;
	unsigned int   product_sn;
	unsigned short product_date_y;
	unsigned char  product_date_m;

	rc = mmc_read_cid(cid);
	if (rc == 0) {
		log_info("mmc_init: MMC/SD Card ID:");
		for (i=0; i<16; i++) {
			printk("%02x ", cid[i]);
		}
		manufacturer_id=cid[0];
		strncpy(oem_id,       &cid[1], 2);
		oem_id[2]='\0';
		strncpy(product_name, &cid[3], 5);
		product_name[5]='\0';
		product_revision_h=(cid[8] >> 4) & 0xf;
		product_revision_l=cid[8] & 0xf;
		product_sn=(cid[9]<<24) + (cid[10]<<16) + (cid[11]<<8) + cid[12];
		tmps=((cid[13]<<8) + cid[14]) & 0x0fff;
		product_date_y=2000 + (((tmps >> 8) & 0xf) * 10) + ((tmps >> 4) & 0xf);
		product_date_m=tmps & 0xf;
		
		log_info("Manufacturer ID   : %02x",  manufacturer_id);
		log_info("OEM/Application ID: %s",    oem_id);
		log_info("Product name      : %s",    product_name);
		log_info("Product revision  : %d.%d", product_revision_h, product_revision_l);
		log_info("Product SN        : %08x",  product_sn);
		log_info("Product Date      : %d-%d", product_date_y, product_date_m);
		
	} else {
		log_warn("mmc_init: impossible to get card indentification info for reason code: %02x", rc);
	}
}

static int mmc_init(void) {
	int rc;

	log_info("mmc_init: GPIO input addr: %08x", gpioaddr_input=(unsigned char*)get_addr_gpioin());
	log_info("mmc_init: GPIO output addr: %08x", gpioaddr_output=(unsigned char*)get_addr_gpioout());
	log_info("mmc_init: GPIO output enable addr: %08x", gpioaddr_enable=(unsigned char*)get_addr_gpioouten());
	
	if (gpioaddr_input==NULL || gpioaddr_output==NULL || gpioaddr_enable==NULL) {
		log_fatal("Impossible to get GPIO addresses registers.");
		return -1;
	}
	
	rc = mmc_hardware_init(); 

	if ( rc != 0) {
		log_error("mmc_init: got an error calling mmc_hardware_init: %02x", rc);
		return -1;
	}

	rc = mmc_card_init(); 
	if ( rc != 0) {
		// Give it an extra shot
		rc = mmc_card_init(); 
		if ( rc != 0) {
			log_error("mmc_init: got an error calling mmc_card_init: %02x", rc);
			return -1;
		}
	}

	mmc_show_cid_info();
	
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

static int mmc_check_media(void) {
	int old_state;
	int rc=0;
	old_state = mmc_media_detect; 

	// TODO: Add card detection here
	mmc_media_detect = 1;
	if (old_state != mmc_media_detect)  {
		mmc_media_changed = 1;
		if (mmc_media_detect == 1) {
			unsigned long flags;
			local_irq_save(flags);
			rc = mmc_init();
			local_irq_restore(flags);
			if (rc != 0) log_error("mmc_check_media: change detected but was not able to initialize new card: %02x", rc);
		}
		else  {
			mmc_exit();
		}
	}
	return rc;
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

	return mmc_check_media();
}

static void __exit mmc_driver_exit(void) {
	int i;

	for (i = 0; i < (64); i++)
		fsync_dev(MKDEV(MAJOR_NR, i));

	blk_cleanup_queue(BLK_DEFAULT_QUEUE(MAJOR_NR));
	del_gendisk(&hd_gendisk);
	devfs_unregister_blkdev(MAJOR_NR, DEVICE_NAME);

	mmc_exit();
}

module_init(mmc_driver_init);
module_exit(mmc_driver_exit);
