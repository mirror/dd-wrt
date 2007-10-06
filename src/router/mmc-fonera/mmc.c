/*
 * a.lp_mp3 - Fonera MMC Driver
 * Copyright (c) 2003-2005 K. John '2B|!2B' Crispin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA02111-1307USA
 *
 * Feedback, Bugs.... mail john{AT}phrozen.org
 *
 */ 

//mount /dev/mmc0 /mnt/0 -t vfat -o noatime,sync


#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/blkdev.h>
#include <linux/genhd.h>

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <asm/param.h>
#include <linux/timer.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/genhd.h>

#include "ar531xlnx.h"

#ifdef CONFIG_ALP_MP3_MMC_DEBUG
#define MMC_DBG(x) 	printk(x)
#else
#define MMC_DBG(x)
#endif

#define SD_MAJOR 126

#define REQUEST_DELAY 1 

#define MMC_EJECTED	1
#define MMC_INSERTED	2
#define MMC_DETECT_NOCHG	0
#define MMC_DETECT_INSERTED	1
#define MMC_DETECT_REMOVED	2

#define AR5315_DSLBASE          0xB1000000
#define AR5315_GPIO_DI          (AR5315_DSLBASE + 0x0088)
#define AR5315_GPIO_DO          (AR5315_DSLBASE + 0x0090)
#define AR5315_GPIO_CR          (AR5315_DSLBASE + 0x0098)
#define AR5315_GPIO_INT         (AR5315_DSLBASE + 0x00a0)

#define PIN_SW1 1<<3
#define PIN_SW2 1<<4
#define PIN_SW5 1<<1
#define PIN_SW6 1<<7

#define SO		((unsigned int)(PIN_SW1))
#define SCK		((unsigned int)(PIN_SW2))
#define SI		((unsigned int)(PIN_SW5))
#define CS		((unsigned int)(PIN_SW6))

#define MMC_CMD_0_GO_IDLE				0
#define MMC_CMD_1_SEND_OP_COND				1
#define MMC_CMD_9_SEND_CSD				9
#define MMC_CMD_10_SEND_CID				10
#define MMC_CMD_12_STOP					12
#define MMC_CMD_13_SEND_STATUS				13
#define MMC_CMD_16_BLOCKLEN				16
#define MMC_CMD_17_READ_SINGLE				17
#define MMC_CMD_18_READ_MULTIPLE			18
#define MMC_CMD_24_WRITE_SINGLE				24

#define MMC_R1_IN_IDLE 					0x01
#define MMC_R1B_BUSY_BYTE 				0x00

#define MMC_START_TOKEN_SINGLE  			0xfe
#define MMC_START_TOKEN_MULTI   			0xfc

#define MMC_DATA_ACCEPT 				0x2
#define MMC_DATA_CRC	 				0x5
#define MMC_DATA_WRITE_ERROR				0x6

#define SPI_IN(a) { 						\
		a = 0;						\
		sysRegWrite(AR5315_GPIO_DO, shadow_SI);			\
		sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);			\
		if(sysRegRead(AR5315_GPIO_DI)&SO) a|=1<<7;  		\
								\
		sysRegWrite(AR5315_GPIO_DO, shadow_SI);			\
		sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);			\
		if(sysRegRead(AR5315_GPIO_DI)&SO) a |= 1<<6;  		\
								\
		sysRegWrite(AR5315_GPIO_DO, shadow_SI);			\
		sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);			\
		if(sysRegRead(AR5315_GPIO_DI)&SO) a |= 1<<5;  		\
								\
		sysRegWrite(AR5315_GPIO_DO, shadow_SI);			\
		sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);			\
		if(sysRegRead(AR5315_GPIO_DI)&SO) a |= 1<<4;  		\
								\
		sysRegWrite(AR5315_GPIO_DO, shadow_SI);			\
		sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);			\
		if(sysRegRead(AR5315_GPIO_DI)&SO) a |= 1<<3;  		\
								\
		sysRegWrite(AR5315_GPIO_DO, shadow_SI);			\
		sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);			\
		if(sysRegRead(AR5315_GPIO_DI)&SO) a |= 1<<2;  		\
								\
		sysRegWrite(AR5315_GPIO_DO, shadow_SI);			\
		sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);			\
		if(sysRegRead(AR5315_GPIO_DI)&SO) a |= 1<<1;  		\
								\
		sysRegWrite(AR5315_GPIO_DO, shadow_SI);			\
		sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);			\
		if(sysRegRead(AR5315_GPIO_DI)&SO) a |= 1;  			\
	}

#define SPI_OUT(a) { 						\
		if(a & 0x80){					\
			sysRegWrite(AR5315_GPIO_DO, shadow_SI);		\
			sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);		\
		} else {					\
			sysRegWrite(AR5315_GPIO_DO, shadow_none);		\
			sysRegWrite(AR5315_GPIO_DO, shadow_SCK);		\
		};						\
		if(a & 0x40){					\
			sysRegWrite(AR5315_GPIO_DO, shadow_SI);		\
			sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);		\
		} else {					\
			sysRegWrite(AR5315_GPIO_DO, shadow_none);		\
			sysRegWrite(AR5315_GPIO_DO, shadow_SCK);		\
		};						\
		if(a & 0x20){					\
			sysRegWrite(AR5315_GPIO_DO, shadow_SI);		\
			sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);		\
		} else {					\
			sysRegWrite(AR5315_GPIO_DO, shadow_none);		\
			sysRegWrite(AR5315_GPIO_DO, shadow_SCK);		\
		};						\
		if(a & 0x10){					\
			sysRegWrite(AR5315_GPIO_DO, shadow_SI);		\
			sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);		\
		} else {					\
			sysRegWrite(AR5315_GPIO_DO, shadow_none);		\
			sysRegWrite(AR5315_GPIO_DO, shadow_SCK);		\
		};						\
		if(a & 0x08){					\
			sysRegWrite(AR5315_GPIO_DO, shadow_SI);		\
			sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);		\
		} else {					\
			sysRegWrite(AR5315_GPIO_DO, shadow_none);		\
			sysRegWrite(AR5315_GPIO_DO, shadow_SCK);		\
		};						\
		if(a & 0x04){					\
			sysRegWrite(AR5315_GPIO_DO, shadow_SI);		\
			sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);		\
		} else {					\
			sysRegWrite(AR5315_GPIO_DO, shadow_none);		\
			sysRegWrite(AR5315_GPIO_DO, shadow_SCK);		\
		};						\
		if(a & 0x02){					\
			sysRegWrite(AR5315_GPIO_DO, shadow_SI);		\
			sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);		\
		} else {					\
			sysRegWrite(AR5315_GPIO_DO, shadow_none);		\
			sysRegWrite(AR5315_GPIO_DO, shadow_SCK);		\
		};						\
		if(a & 0x01){					\
			sysRegWrite(AR5315_GPIO_DO, shadow_SI);		\
			sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);		\
		} else {					\
			sysRegWrite(AR5315_GPIO_DO, shadow_none);		\
			sysRegWrite(AR5315_GPIO_DO, shadow_SCK);		\
		};						\
	}

#define SPI_OUT_0xFF {						\
		sysRegWrite(AR5315_GPIO_DO, shadow_SI);		\
		sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);	\
								\
		sysRegWrite(AR5315_GPIO_DO, shadow_SI);		\
		sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);	\
								\
		sysRegWrite(AR5315_GPIO_DO, shadow_SI);		\
		sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);	\
								\
		sysRegWrite(AR5315_GPIO_DO, shadow_SI);		\
		sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);	\
								\
		sysRegWrite(AR5315_GPIO_DO, shadow_SI);		\
		sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);	\
								\
		sysRegWrite(AR5315_GPIO_DO, shadow_SI);		\
		sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);	\
								\
		sysRegWrite(AR5315_GPIO_DO, shadow_SI);		\
		sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);	\
								\
		sysRegWrite(AR5315_GPIO_DO, shadow_SI);		\
		sysRegWrite(AR5315_GPIO_DO, shadow_SCK_SI);	\
	}

#define SPI_CS_LO { 						\
		sysRegWrite(AR5315_GPIO_DO, shadow_none);	\
	}

#define SPI_CS_HI { 						\
		sysRegWrite(AR5315_GPIO_DO, shadow_none | CS);	\
	}

#define SPI_SEND_CMD(cmd, data) { SPI_SEND_CMD2(cmd, ((unsigned long int) data)); }
#define SPI_SEND_CMD2(cmd, data) {				\
		SPI_OUT((0x40 + cmd));				\
		SPI_OUT(((data>>24)&0xff));			\
		SPI_OUT(((data>>16)&0xff));			\
		SPI_OUT(((data>>8)&0xff));			\
		SPI_OUT((data&0xff));				\
		SPI_OUT(0x95);					\
	}

#define SPI_SHADOW 								\
		unsigned int data_shadow = sysRegRead(AR5315_GPIO_DO) & ~(SCK | SI | CS);	\
		unsigned int shadow_none = data_shadow; 			\
		unsigned int shadow_SCK = data_shadow | SCK;			\
		unsigned int shadow_SI = data_shadow | SI;			\
		unsigned int shadow_SCK_SI = data_shadow | SI | SCK;	

#define SPI_GET_R1(a) {						\
		do { 						\
			SPI_IN(a);				\
		} while(a & 0x80);				\
	}

#define SPI_GET_DATA(a,b) {					\
		unsigned int len = a;				\
		do {						\
			SPI_IN(retval);				\
		} while(retval != MMC_START_TOKEN_SINGLE);	\
		while(len){					\
			SPI_IN(*b);				\
			len--;					\
			b++;					\
		};						\
		SPI_OUT_0xFF;					\
		SPI_OUT_0xFF;					\
	}

#define SPI_WAIT_TILL_CARD_IS_READY(a) {			\
		SPI_CS_LO;					\
		do {						\
			SPI_OUT_0xFF;				\
			SPI_OUT_0xFF;				\
			SPI_OUT_0xFF;				\
			SPI_OUT_0xFF;				\
			SPI_OUT_0xFF;				\
			SPI_OUT_0xFF;				\
			SPI_OUT_0xFF;				\
			SPI_OUT_0xFF;				\
			SPI_OUT_0xFF;				\
			SPI_OUT_0xFF;				\
			SPI_IN(a);				\
		} while(a == MMC_R1B_BUSY_BYTE);		\
		SPI_CS_HI;					\
	}

typedef struct _VOLUME_INFO{
	unsigned char		size_MB;
	unsigned long int	size;
	unsigned char   	sector_multiply;
	unsigned int		sector_count;
	unsigned int 		sector_size;
	unsigned char		name[6];
} VOLUME_INFO;

void mmcDelay(int ms) {
	int i,a;
	int delayvar=10;

	for (a=0;a<ms;a++) {
		for (i=0;i<150;i++) {
			delayvar*=2;        
			delayvar/=2;
		} 
	}
}   

void MMC_get_CID(unsigned char *ptr_data){
	unsigned char retval;
	SPI_SHADOW;
	SPI_WAIT_TILL_CARD_IS_READY(retval);
	SPI_CS_LO;
	SPI_SEND_CMD(MMC_CMD_10_SEND_CID, 0x0);
	SPI_GET_R1(retval);
	SPI_GET_DATA(10, ptr_data);
	SPI_CS_HI;
	SPI_OUT_0xFF;
}

unsigned char MMC_get_CSD(unsigned char *ptr_data){
	unsigned int max_errors = 1024;
	unsigned char retval;
	SPI_SHADOW;
	SPI_WAIT_TILL_CARD_IS_READY(retval);
	SPI_CS_LO;
	SPI_SEND_CMD(MMC_CMD_9_SEND_CSD, 0x0);
	do { 						
		SPI_IN(retval);			
	} while((retval & 0x80) && (max_errors--));			
	if(retval & 0x80){
		SPI_CS_HI;
		return 0;
	};
	SPI_GET_DATA(10, ptr_data);
	SPI_CS_HI;
	SPI_OUT_0xFF;
	return 1;
}

void MMC_get_volume_info(VOLUME_INFO* vinf){
	unsigned char data[16];
	MMC_get_CSD(data);
	vinf->sector_count = data[6] & 0x03;
	vinf->sector_count <<= 8;
	vinf->sector_count += data[7];
	vinf->sector_count <<= 2;
	vinf->sector_count += (data[8] & 0xc0) >> 6;
		
	vinf->sector_multiply = data[9] & 0x03;
	vinf->sector_multiply <<= 1;
	vinf->sector_multiply += (data[10] & 0x80) >> 7;
	
	
	vinf->size_MB = vinf->sector_count >> (9-vinf->sector_multiply);
	vinf->size    = (vinf->sector_count * 512 )<< (vinf->sector_multiply+2);
	vinf->sector_size = 512;
	
	MMC_get_CID(data);
	vinf->name[0] = data[3];
	vinf->name[1] = data[4];
	vinf->name[2] = data[5];
	vinf->name[3] = data[6];
	vinf->name[4] = data[7];
	vinf->name[5] = '\0';
}

static unsigned char MMC_get_block(unsigned long sector, unsigned char *data){
	unsigned char retval;
	unsigned int max_errors = 1024;
	int length = 512;
	SPI_SHADOW;
	SPI_WAIT_TILL_CARD_IS_READY(retval);
	sector = sector << 9;

	SPI_CS_LO;
	SPI_SEND_CMD(MMC_CMD_17_READ_SINGLE, sector);
	do{
		SPI_IN(retval);
		max_errors--;
	}while((retval & 0x80) && (max_errors>0));
	do{
		SPI_IN(retval);
	}while((retval != MMC_START_TOKEN_SINGLE) );
	
	while(length--){
		SPI_IN(*data);
		data++;
	};
	SPI_OUT_0xFF;
	SPI_OUT_0xFF;
	SPI_CS_HI;
	SPI_OUT_0xFF;
	return 0;
}

static unsigned char MMC_put_block(unsigned long sector, unsigned char *data){
	unsigned char retval;
	unsigned int max_errors = 8092;
	unsigned char tmp;
	int length = 512;
	SPI_SHADOW;
	SPI_WAIT_TILL_CARD_IS_READY(retval);
	sector = sector << 9;
	
	SPI_CS_LO;
	SPI_SEND_CMD(MMC_CMD_24_WRITE_SINGLE, sector);
	do{
		SPI_IN(retval);
		max_errors--;
	}while((retval & 0x80) && (max_errors));
	SPI_OUT(MMC_START_TOKEN_SINGLE);
	
	while(length--){
		SPI_OUT(*data);
		data++;
	};	
	SPI_OUT_0xFF;
	SPI_OUT_0xFF;

	SPI_IN(retval);
	tmp = (retval & 0xf) >> 1;
	if(tmp != MMC_DATA_ACCEPT){
		printk("mmc_drv.ko : ERROR : MMC_write_sector %d\n", tmp);
		return tmp;
	} 
	SPI_CS_HI;
	SPI_OUT_0xFF;
	return tmp;
}




//---------------------- slow init part --------------------------
int bit_get(unsigned int pin){
	return (sysRegRead(AR5315_GPIO_DI)&pin)?(1):(0);
};

void bit_set(unsigned int pin){
	sysRegWrite(AR5315_GPIO_DO, sysRegRead(AR5315_GPIO_DO) | pin);
};

void bit_clear(unsigned int pin){
	sysRegWrite(AR5315_GPIO_DO, sysRegRead(AR5315_GPIO_DO) & ~pin);
};
	
void SPI_clock(void){
	bit_clear(SCK);
	mmcDelay(2);
	bit_set(SCK);
};

unsigned char SPI_io(unsigned char byte){
	int i;
	unsigned char byte_out = 0;
	for(i = 7; i>=0; i--){
		if(byte & (1<<i)){
			sysRegWrite(AR5315_GPIO_DO, sysRegRead(AR5315_GPIO_DO) | SI);
		} else {
			sysRegWrite(AR5315_GPIO_DO, sysRegRead(AR5315_GPIO_DO) & ~SI);
		};	
		sysRegWrite(AR5315_GPIO_DO, sysRegRead(AR5315_GPIO_DO) & ~SCK);
		mmcDelay(2);
		sysRegWrite(AR5315_GPIO_DO, sysRegRead(AR5315_GPIO_DO) | SCK);
		byte_out += (sysRegRead(AR5315_GPIO_DI)&SO)?(1<<i):(0);
	};
	return byte_out;
};

void SPI_o(unsigned char byte){
        int i;
        for(i = 7; i>=0; i--){
	        if(byte & (1<<i)){
			sysRegWrite(AR5315_GPIO_DO, sysRegRead(AR5315_GPIO_DO) | SI);
		} else {
		        sysRegWrite(AR5315_GPIO_DO, sysRegRead(AR5315_GPIO_DO) & ~SI);
                };
                sysRegWrite(AR5315_GPIO_DO, sysRegRead(AR5315_GPIO_DO) & ~SCK);
       		mmcDelay(2);
        	sysRegWrite(AR5315_GPIO_DO, sysRegRead(AR5315_GPIO_DO) | SCK);
	};
};

void spi_io_mem(unsigned char *data, int length){
	while(length){
		SPI_io(*data);
		data++;
		length--;
	};
};

void MMC_send_cmd(unsigned char cmd, unsigned long int data){
	static unsigned char buffer[6] ;
	buffer[0]=0x40 + cmd;
	buffer[1]=(data>>24)&0xff;
	buffer[2]=(data>>16)&0xff;
	buffer[3]=(data>>8)&0xff;
	buffer[4]=data&0xff;
	buffer[5]=0x95;
	spi_io_mem(buffer,6);
};

unsigned char MMC_get_R1(void){
	unsigned char retval;
	unsigned int max_errors = 1024;
	do{
		retval = SPI_io(0xff);
		max_errors--;
	}while(  (retval & 0x80) && (max_errors>0));
	return retval;
};

void MMC_CS_select(void){
	bit_clear(CS);
};

void MMC_CS_deselect(void){
	bit_set(CS);
};

void MMC_cleanup(void){
	MMC_CS_deselect();
	SPI_io(0xff);
};


static unsigned char MMC_init(void){
	unsigned char i, j;
	unsigned char res;
	int max_error = 8000;
	VOLUME_INFO vinf;
	
	for(j = 0; j < 3; j++){
		MMC_CS_deselect();
		for(i = 0; i < 100; i++){
			SPI_clock();
			mmcDelay(1);
		};
		MMC_CS_select();
		MMC_send_cmd(MMC_CMD_0_GO_IDLE,0x0);
		res = MMC_get_R1();
		//printk("mmc_drv.ko : response : %d\n", res);
		if(res == 1){
			j = 100;
		};
	};
	if(res != 0x01){
		if(res == 0xff){
		//	printk("mmc_drv.ko : card not found\n");
			return 1;
		} else {
		//	printk("mmc_drv.ko : invalid response\n");	
			return 2;
		};
	};

	printk("mmc : Card Found\n");
	while((res==0x01) && (max_error)){
		MMC_CS_deselect();
		SPI_io(0xff);
		MMC_CS_select();
		MMC_send_cmd(MMC_CMD_1_SEND_OP_COND,0x0);
		res = MMC_get_R1();
		max_error -= 1;
	};

	if(max_error == 0){
		printk("Too many errors!!!\n");
		MMC_CS_select();
		return 2;
	};
	printk("mmc : card in op mode\n");	
	MMC_cleanup();
	MMC_get_volume_info(&vinf);
	printk("mmc : SIZE : %d, nMUL : %d, COUNT : %d, NAME : %s\n", vinf.size_MB, vinf.sector_multiply, vinf.sector_count, vinf.name);
	return 0;
}

//--------------------- kernel device interface ----------------------

static unsigned long size_of_card;
static unsigned int size_of_block;
static struct gendisk *card;
static struct request_queue *blkdev_requests;
static spinlock_t blkdev_spinlock = SPIN_LOCK_UNLOCKED;
static unsigned char mmc_status;
unsigned char mmc_media_changed;
struct timer_list mmc_timer;

static void mmc_timer_callback(unsigned long ptr);

// a dirty hack to allow mp3 playback without glitches
#ifdef CONFIG_FOX_VS1011B 
void mp3_handle_playback(int external_call);
#endif

static void bd_request (request_queue_t *q){
	
	struct request *pending_request;
	unsigned long start, to_copy, i, max;
	
	while ((pending_request=elv_next_request(q))!=NULL){
		if (!blk_fs_request(pending_request)){
			end_request (pending_request,0);
			continue;
		}
		to_copy = pending_request->current_nr_sectors;
		start = pending_request->sector;
		
		spin_unlock_irq(q->queue_lock);
		
		if((start+to_copy) <= ((size_of_card)>>9)){
			max = pending_request->current_nr_sectors - 1;
			if (rq_data_dir(pending_request) == READ){				
				for(i = 0; i <= max; i++){
					MMC_get_block(pending_request->sector + i,(pending_request->buffer+(512*i)));
				}
			}else{
				for(i = 0; i <= max; i++){
					MMC_put_block(pending_request->sector + i,(pending_request->buffer+(512*i)));
				}
			}
// a dirty hack to allow mp3 playback without glitches
#ifdef CONFIG_FOX_VS1011B
			mp3_handle_playback(1);
#endif
		}else{
			printk("mmc : %ld not in range...\n",start+to_copy);
		}
		spin_lock_irq(q->queue_lock);
		end_request(pending_request,1);
	}	
}

static int mmc_do_open(struct inode *inode, struct file *filp){
        if(mmc_status == MMC_EJECTED){
		MMC_DBG("mmc : error opening mmc device\n");
		return -ENODEV;
	}
 	MMC_DBG("mmc : opening mmc device\n");
        return 0;
}

static int mmc_release(struct inode *inode, struct file *filp){
        return 0;
}

static int mmc_revalidate(struct gendisk *disk){
	return 0;
}

static int mmc_has_media_changed(struct gendisk *dev){
	if (mmc_media_changed == 1){
                mmc_media_changed = 0;
	        MMC_DBG("mmc : media changed\n");
                return 1;
        };
        MMC_DBG("mmc : media not changed\n");
	return 0;
}

static void module_error(void){
	unregister_blkdev (SD_MAJOR, "sd");
}

static struct block_device_operations bdops = {
	.open = mmc_do_open,
	.release = mmc_release,
	.media_changed = mmc_has_media_changed,
	.revalidate_disk = mmc_revalidate,
	.owner = THIS_MODULE,
};

#ifdef REQUEST_DELAY
int mmc_request_busy = 0;
request_queue_t *delay_q;
unsigned char my_is_atomic = 0;
static void mmc_complete_request(unsigned long  data);

DECLARE_TASKLET( tlmmc_descr, mmc_complete_request, 0L );

static void mmc_complete_request(unsigned long  data)
{
	bd_request(delay_q);
	mmc_request_busy = 0;
}

void mmc_delay_request(request_queue_t *q)
{
	if(mmc_request_busy)
		return;
	delay_q = q;
	mmc_request_busy = 1;
	if(my_is_atomic){
		my_is_atomic = 0;
		mmc_complete_request(0);
	} else {
		tasklet_schedule( &tlmmc_descr );
	};
}

#endif

static int mmc_detect_card(void){
	VOLUME_INFO info;
	if(mmc_status == MMC_EJECTED){
		if(MMC_init() != 0){
			MMC_DBG("mmc : ERROR initializing MMC Card\n");
			MMC_DBG("mmc : ERROR Card not inserted ?\n");
			MMC_DBG("mmc : Quitting Driver\n");
			return MMC_DETECT_NOCHG;
		};
		printk("mmc : Card Initialised\n");
	
		if (register_blkdev(SD_MAJOR,"sd")){
			printk("mmc : blockdevice: Majornummer %d not free.", SD_MAJOR);
			return MMC_DETECT_NOCHG;
		}

	
		if ( !(card=alloc_disk(1<<4)) ) {
			printk("mmc : alloc_disk failed ...\n");
			module_error();
			return MMC_DETECT_NOCHG;
		}	
			
		spin_lock_init(&blkdev_spinlock);
#ifdef REQUEST_DELAY
		my_is_atomic = 1;
		if ( (blkdev_requests=blk_init_queue (&mmc_delay_request, &blkdev_spinlock)) == NULL ){
#else
		if ( (blkdev_requests=blk_init_queue (&bd_request,&blkdev_spinlock)) == NULL ){	
#endif
			module_error();
			return MMC_DETECT_NOCHG;
		};
		
		MMC_get_volume_info(&info);
		size_of_block = info.sector_size;
		size_of_card = info.size;
		printk("mmc : The inserted card has a capacity of %lu Bytes\n", size_of_card);
		
		blk_queue_hardsect_size (blkdev_requests,size_of_block);
		
		card->major = SD_MAJOR;
		card->first_minor = 0;
		card->minors = 4;	
		card->queue = blkdev_requests;
		sprintf(card->disk_name,"mmc");
		set_capacity (card,(size_of_card)>>9);
		card->fops = &bdops;
		card->flags = GENHD_FL_REMOVABLE;
		mmc_status = MMC_INSERTED;
		mmc_media_changed = 1;
		printk("mmc : adding disk\n");
		add_disk (card);		
		return MMC_DETECT_INSERTED;
	} else {
		unsigned char csd[16];
		if(MMC_get_CSD(csd) == 1){			
			return MMC_DETECT_NOCHG;
		} else {
			del_gendisk(card);
			put_disk(card);
			blk_cleanup_queue (blkdev_requests);
			unregister_blkdev (SD_MAJOR,"sd");
			mmc_status = MMC_EJECTED;
			mmc_media_changed = 1;
			return MMC_DETECT_REMOVED;
		};
	}
}

static void mmc_timer_setup(void){
	init_timer(&mmc_timer);
	mmc_timer.function = mmc_timer_callback;
	mmc_timer.data = 0;
	mmc_timer.expires = jiffies + (3 * HZ);
	add_timer(&mmc_timer);	
}

static void mmc_timer_callback(unsigned long ptr){
	unsigned char state = mmc_detect_card();
	del_timer(&mmc_timer);
	
	switch(state){
	case MMC_DETECT_REMOVED:
		printk("mmc : Card has been removed\n");
		break;
	case MMC_DETECT_INSERTED:
		printk("mmc : Card has been inserted\n");
		break;
	default:
		break;
	};
	mmc_timer_setup();
}

static int __init mod_init(void){
	printk("mmc : MMC Driver for Fonera Version 2.5 (050507) -- '2B|!2B' (john@phrozen.org)\n");
	
	sysRegWrite(AR5315_GPIO_CR, (sysRegRead(AR5315_GPIO_CR) | SI | SCK | CS) & ~SO);
	
	MMC_DBG("mmc : debug messages enabled\n");
	mmc_status = MMC_EJECTED;
	if(mmc_detect_card() == MMC_DETECT_INSERTED){
		printk("mmc : Card was Found\n");
	};
	mmc_media_changed = 1;
	mmc_timer_setup();
	
	return 0;
}


static void __exit mod_exit(void){
	del_gendisk(card);
	put_disk(card);
	blk_cleanup_queue (blkdev_requests);
	unregister_blkdev (SD_MAJOR,"sd");
}

module_init (mod_init);
module_exit (mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Crispin - john@phrozen.org");
MODULE_DESCRIPTION("AR5315 - GPIO - MMC (SD-SPI) Driver");

