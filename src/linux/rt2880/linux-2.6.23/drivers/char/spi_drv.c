#include <linux/init.h>
#include <linux/version.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>   
#include <linux/fs.h>       
#include <linux/errno.h>    
#include <linux/types.h>    
#include <linux/proc_fs.h>
#include <linux/fcntl.h>    
#include <asm/system.h>     
#include <linux/wireless.h>

#include "spi_drv.h"
#include "ralink_gpio.h"

#ifdef CONFIG_MAC_TO_MAC_MODE 
#include <linux/delay.h>
#include "vtss.h"
#endif

#ifdef  CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
#endif

#ifdef  CONFIG_DEVFS_FS
static devfs_handle_t devfs_handle;
#endif
int spidrv_major =  217;

/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	spi_chip_select                                         */
/*    INPUTS: ENABLE or DISABLE                                         */
/*   RETURNS: None                                                      */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): Pull on or Pull down #CS                                  */
/*                                                                      */
/*----------------------------------------------------------------------*/

static inline void spi_chip_select(u8 enable)
{
	int i;

	for (i=0; i<spi_busy_loop; i++) {
		if (!IS_BUSY) {
			/* low active */
			if (enable) {		
				RT2880_REG(RT2880_SPICTL_REG) =	SPICTL_SPIENA_ON;
			} else  {
				RT2880_REG(RT2880_SPICTL_REG) = SPICTL_SPIENA_OFF;
			}		
			break;
		}
	}

#ifdef DBG
	if (i == spi_busy_loop) {
		printk("warning : spi_transfer (spi_chip_select) busy !\n");
	}
#endif
}

/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	spi_master_init                                         */
/*    INPUTS: None                                                      */
/*   RETURNS: None                                                      */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): Initialize SPI block to desired state                     */
/*                                                                      */
/*----------------------------------------------------------------------*/
void spi_master_init(void)
{
	int i;
	/* reset spi block */
	RT2880_REG(RT2880_RSTCTRL_REG) = RSTCTRL_SPI_RESET;
	/* udelay(500); */
	for ( i = 0; i < 1000; i++);
	

	RT2880_REG(RT2880_SPICFG_REG) = SPICFG_MSBFIRST | 
									SPICFG_RXCLKEDGE_FALLING |
									SPICFG_TXCLKEDGE_FALLING |
									SPICFG_SPICLK_DIV128;	
	spi_chip_select(DISABLE);

#ifdef DBG
	printk("SPICFG = %08x\n", RT2880_REG(RT2880_SPICFG_REG));
	printk("is busy %d\n", IS_BUSY);
#endif		 	
}

/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	spi_write                                               */
/*    INPUTS: 8-bit data                                                */
/*   RETURNS: None                                                      */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): transfer 8-bit data to SPI                                */
/*                                                                      */
/*----------------------------------------------------------------------*/
static void spi_write(u8 data)
{
	int i;

	for (i=0; i<spi_busy_loop; i++) {
		if (!IS_BUSY) {
			RT2880_REG(RT2880_SPIDATA_REG) = data;
			/* start write transfer */
			RT2880_REG(RT2880_SPICTL_REG) = SPICTL_HIZSDO |  SPICTL_STARTWR | 
											SPICTL_SPIENA_ON;
			break;
		}
	}

#ifdef DBG
	if (i == spi_busy_loop) {
		printk("warning : spi_transfer (write %02x) busy !\n", data);
	}
#endif
}

#ifdef CONFIG_MAC_TO_MAC_MODE 
//write32 MSB first
static void spi_write32(u32 data)
{
	u8 d0, d1, d2, d3;

	d0 = (u8)((data >> 24) & 0xff);
	d1 = (u8)((data >> 16) & 0xff);
	d2 = (u8)((data >> 8) & 0xff);
	d3 = (u8)(data & 0xff);

	spi_write(d0);
	spi_write(d1);
	spi_write(d2);
	spi_write(d3);
}
#endif

/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	spi_read                                                */
/*    INPUTS: None                                                      */
/*   RETURNS: 8-bit data                                                */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): get 8-bit data from SPI                                   */
/*                                                                      */
/*----------------------------------------------------------------------*/
static u8 spi_read(void) 
{
	int i;

	/* 
	 * poll busy bit until it is 0 
	 * then start read transfer
	 */
	for (i=0; i<spi_busy_loop; i++) {
		if (!IS_BUSY) {
			RT2880_REG(RT2880_SPIDATA_REG) = 0;
			/* start read transfer */
			RT2880_REG(RT2880_SPICTL_REG) = SPICTL_HIZSDO | SPICTL_STARTRD |
											SPICTL_SPIENA_ON;
			break;
		}
	}

	/* 
	 * poll busy bit until it is 0 
	 * then get data 
	 */
	for (i=0; i<spi_busy_loop; i++) {
		if (!IS_BUSY) {
			break;
		}
	}
	
#ifdef DBG		
	if (i == spi_busy_loop) {
		printk("warning : spi_transfer busy !\n");
	} 
#endif

	return ((u8)RT2880_REG(RT2880_SPIDATA_REG));
}

#if defined (CONFIG_RALINK_PCM) || defined (CONFIG_RALINK_PCM_MODULE)
static void spi_si3220_master_init(void)
{
	int i;
	/* reset spi block */
	RT2880_REG(RT2880_RSTCTRL_REG) = RSTCTRL_SPI_RESET;
	/* udelay(500); */
	for ( i = 0; i < 1000; i++);
	

	RT2880_REG(RT2880_SPICFG_REG) = SPICFG_MSBFIRST | 
									SPICFG_RXCLKEDGE_FALLING |
									SPICFG_TXCLKEDGE_FALLING |
									SPICFG_SPICLK_DIV128 | SPICFG_SPICLKPOL;	
	spi_chip_select(DISABLE);
#ifdef DBG
	printk("[slic]SPICFG = %08x\n", RT2880_REG(RT2880_SPICFG_REG));
	printk("[slic]is busy %d\n", IS_BUSY);
#endif		 	
}

u8 spi_si3220_read8(unsigned char cid, unsigned char reg)
{
	u8 value;
	int i;
	spi_si3220_master_init();
	spi_chip_select(ENABLE);
	
	spi_write(cid);
	
	spi_chip_select(DISABLE);
	//for(i=0;i<1000;i++);
	spi_chip_select(ENABLE);
	
	spi_write(reg);
	
	spi_chip_select(DISABLE);
	//for(i=0;i<1000;i++);
	spi_chip_select(ENABLE);
	
	value = spi_read();
	
	spi_chip_select(DISABLE);
	return value;
}
static unsigned char high8bit (unsigned short data)
{
return ((data>>8)&(0x0FF));
}

static unsigned char low8bit (unsigned short data)
{
return ((unsigned char)(data & 0x00FF));
}
static unsigned short SixteenBit (unsigned char hi, unsigned char lo)
{
	unsigned short data = hi;
	data = (data<<8)|lo;
	return data;
}

unsigned short spi_si3220_read16(unsigned char cid, unsigned char reg)
{
	unsigned short value;
	unsigned char hi, lo;
	spi_si3220_master_init();
	spi_chip_select(ENABLE);
	
	spi_write(cid);
	spi_write(reg);
	
	spi_chip_select(DISABLE);
	spi_chip_select(ENABLE);
	
	hi = spi_read();
	lo = spi_read();
	
	spi_chip_select(DISABLE);
	return SixteenBit(hi,lo);
}
void spi_si3220_write8(unsigned char cid, unsigned char reg, unsigned char value)
{
	int i; 
	spi_si3220_master_init();
	spi_chip_select(ENABLE);
	spi_write(cid);
	spi_chip_select(DISABLE);
	spi_chip_select(ENABLE);
	spi_write(reg);
	spi_chip_select(DISABLE);
	spi_chip_select(ENABLE);
	spi_write(value);
	spi_chip_select(DISABLE);
}


void spi_si3220_write16(unsigned char cid, unsigned char reg, unsigned short value)
{
	int i; 
	unsigned char hi, lo;
	hi = high8bit(value);
	lo = low8bit(value);
	spi_si3220_master_init();
	spi_chip_select(ENABLE);
	spi_write(cid);
	//spi_chip_select(DISABLE);
	//spi_chip_select(ENABLE);
	spi_write(reg);
	spi_chip_select(DISABLE);

	spi_chip_select(ENABLE);
	
	spi_write(hi);
	spi_write(lo);

	spi_chip_select(DISABLE);
}
#endif

#ifdef CONFIG_MAC_TO_MAC_MODE 
//read32 MSB first
static u32 spi_read32(void)
{
	u8 d0, d1, d2, d3;
	u32 ret;

	d0 = spi_read();
	d1 = spi_read();
	d2 = spi_read();
	d3 = spi_read();
	ret = (d0 << 24) | (d1 << 16) | (d2 << 8) | d3;

	return ret;
}
#endif

/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	eeprom_get_status_reg                                   */
/*    INPUTS: pointer to status                                         */
/*   RETURNS: None                                                      */
/*   OUTPUTS: status                                                    */
/*   NOTE(S): get the status of eeprom (AT25xxxx)                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
static void eeprom_get_status_reg(u8 *status) 
{
	spi_chip_select(ENABLE);
	spi_write(RDSR_CMD);
	*status = spi_read();		
	spi_chip_select(DISABLE);
}


/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	eeprom_read                                             */
/*    INPUTS: address - start address to be read                        */ 
/*            nbytes  - number of bytes to be read                      */
/*            dest    - pointer to read buffer                          */
/*   RETURNS: 0 - successful                                            */
/*            or eeprom status                                          */
/*   OUTPUTS: read buffer                                               */
/*   NOTE(S): If the eeprom is busy , the function returns with status  */
/*            register of eeprom                                        */
/*----------------------------------------------------------------------*/
unsigned char spi_eeprom_read(u16 address, u16 nbytes, u8 *dest)
{
	u8	status;
	u16	cnt = 0;
	int i = 0;

	do {
		i++;
		eeprom_get_status_reg(&status);		
	}
	while((status & (1<<RDY)) && (i < max_ee_busy_loop));

	if (i == max_ee_busy_loop)
		return (status);

	/* eeprom ready */
	if (!(status & (1<<RDY))) {	
		spi_chip_select(ENABLE);
		/* read op */
		spi_write(READ_CMD);
		spi_write((u8)(address >> 8));		/* MSB byte First */
		spi_write((u8)(address & 0x00FF));	/* LSB byte */

		while (cnt < nbytes) {
			*(dest++) = spi_read();
			cnt++;
		}
		status = 0;
		/* deassert cs */
		spi_chip_select(DISABLE);
	} 
	return (status);	
}


/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	eeprom_write_enable                                     */
/*    INPUTS: None                                                      */
/*   RETURNS: None                                                      */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): always perform write enable  before any write operation   */
/*                                                                      */
/*----------------------------------------------------------------------*/
static void eeprom_write_enable(void)
{
	unsigned char	status;
	int i = 0;

	do {
		i++;
		eeprom_get_status_reg(&status);		
	}
	while((status & (1<<RDY)) && (i < max_ee_busy_loop));

	if (i == max_ee_busy_loop)
		return;

	/* eeprom ready */
	if (!(status & (1<<RDY))) 
	{	
		spi_chip_select(ENABLE);
		/* always write enable  before any write operation */
		spi_write(WREN_CMD);

		spi_chip_select(DISABLE);
		
		/* wait for write enable */
		do {
			eeprom_get_status_reg(&status);
		} while((status & (1<<RDY)) || !(status & (1<<WEN)));

	}

}

/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	eeprom_write                                            */
/*    INPUTS: address - the first byte address to be written            */
/*            nbytes  - the number of bytes to be written               */
/*            src     - the pointer to source buffer                    */     
/*   RETURNS: 0  - successful                                           */
/*            or eeprom buy status                                      */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): The different eeprom has various write page size          */
/* 			  The function don't care write page size so the caller     */
/*            must check the page size of eeprom                        */
/*                                                                      */
/*----------------------------------------------------------------------*/
unsigned char spi_eeprom_write(u16 address, u16 nbytes, u8 *src)
{
	unsigned char	status;
	unsigned int	cnt = 0;
	int i = 0;

	do {
		i++;
		eeprom_get_status_reg(&status);		
	}
	while((status & (1<<RDY)) && (i < max_ee_busy_loop));

	if (i == max_ee_busy_loop)
		goto done;


	/* eeprom ready */
	if (!(status & (1<<RDY))) {			
		/* always write enable  before any write operation */		
		eeprom_write_enable();

		spi_chip_select(ENABLE);
		spi_write(WRITE_CMD);
		spi_write((u8)(address >> 8));		/* MSB byte First */
		spi_write((u8)(address & 0x00FF));	/* LSB byte */

		while (cnt < nbytes) {
			spi_write(src[cnt]);
			cnt++;
		}
		status = 0;
		/* last byte sent then pull #cs high  */
		spi_chip_select(DISABLE);
	} 

	i = 0;
	do {
		i++;
		eeprom_get_status_reg(&status);		
	}
	while((status & (1<<RDY)) && (i < max_ee_busy_loop));

done:
	return (status);
}

#ifdef CONFIG_MAC_TO_MAC_MODE 
void spi_vtss_read(u8 blk, u8 subblk, u8 addr, u32 *value)
{
	u8 cmd;

	spi_master_init();
	cmd = (u8)((blk << 5) | subblk);
	spi_write(cmd);
	spi_write(addr);
	spi_read(); //dummy byte
	spi_read(); //dummy byte
	*value = spi_read32();
	//printf("rd %x:%x:%x = %x\n", blk, subblk, addr, *value);
	udelay(100);
}

void spi_vtss_write(u8 blk, u8 subblk, u8 addr, u32 value)
{
	u8 cmd;

	spi_master_init();
	cmd = (u8)((blk << 5) | subblk | 0x10);
	spi_write(cmd);
	spi_write(addr);
	spi_write32(value);
	//printf("wr %x:%x:%x = %x\n", blk, subblk, addr, value);
	udelay(10);
}

void vtss_reset()
{
	le32_to_cpu(*(volatile u_long *)RALINK_REG_GPIOMODE) |= (1 << 1);

	le32_to_cpu(*(volatile u_long *)RALINK_REG_PIODIR) |= (1 << 10);

	//Set Gpio pin 10 to low
	le32_to_cpu(*(volatile u_long *)RALINK_REG_PIODATA) &= ~(1 << 10);

	mdelay(50);
	//Set Gpio pin 10 to high
	le32_to_cpu(*(volatile u_long *)RALINK_REG_PIODATA) |= (1 << 10);

	mdelay(125);
}

//type 0: no vlan, 1: vlan
void vtss_init(int type)
{
	int i, len;
	u32 tmp;

	len = (type == 0)? sizeof(lutonu_novlan) : sizeof(lutonu_vlan);

	//HT_WR(SYSTEM, 0, ICPU_CTRL, (1<<7) | (1<<3) | (1<<2) | (0<<0));
	//read it out to be sure the reset was done.
	while (1) {
		spi_vtss_write(7, 0, 0x10, (1<<7) | (1<<3) | (1<<2) | (0<<0));
		spi_vtss_read(7, 0, 0x10, &tmp);
		if (tmp & ((1<<7) | (1<<3) | (1<<2) | (0<<0)))
			break;
		udelay(1000);
	}

	//HT_WR(SYSTEM, 0, ICPU_ADDR, 0); //clear SP_SELECT and ADDR
	spi_vtss_write(7, 0, 0x11, 0);

	if (type == 0) {
		for (i = 0; i < len; i++) {
			//HT_WR(SYSTEM, 0, ICPU_DATA, lutonu[i]);
			spi_vtss_write(7, 0, 0x12, lutonu_novlan[i]);
		}
	}
	else {
		for (i = 0; i < len; i++) {
			//HT_WR(SYSTEM, 0, ICPU_DATA, lutonu[i]);
			spi_vtss_write(7, 0, 0x12, lutonu_vlan[i]);
		}
	}

	//debug
	/*
	spi_vtss_write(7, 0, 0x11, 0);
	spi_vtss_read(7, 0, 0x11, &tmp);
	printk("ICPU_ADDR %x\n", tmp);
	for (i = 0; i < len; i++) {
		spi_vtss_read(7, 0, 0x12, &tmp);
		printk("%x ", tmp);
	}
	*/

	//HT_WR(SYSTEM, 0, GLORESET, (1<<0)); //MASTER_RESET
	spi_vtss_write(7, 0, 0x14, (1<<0));
	udelay(125000);

	//HT_WR(SYSTEM, 0, ICPU_CTRL, (1<<8) | (1<<3) | (1<<1) | (1<<0));
	spi_vtss_write(7, 0, 0x10, (1<<8) | (1<<3) | (1<<1) | (1<<0));
}
#endif

int spidrv_open(struct inode *inode, struct file *filp)
{
	return 0;
}

int spidrv_release(struct inode *inode, struct file *filp)
{
	return 0;
}

int spidrv_ioctl(struct inode *inode, struct file *filp,
		unsigned int cmd, unsigned long arg)
{
	unsigned int address, value, size;
	SPI_WRITE *spi_wr;
#ifdef CONFIG_MAC_TO_MAC_MODE 
	SPI_VTSS *vtss;
#endif

	switch (cmd) {
	case RT2880_SPI_READ:
		value = 0; address = 0;

		address = (unsigned int)arg;
		spi_master_init();

		spi_eeprom_read(address, 4, (unsigned char*)&value);
		printk("0x%04x : 0x%08x\n", address, value);
		break;
	case RT2880_SPI_WRITE:
		spi_wr = (SPI_WRITE*)arg;
		address = spi_wr->address;
		value   = spi_wr->value;
		size    = spi_wr->size;

		spi_master_init();
		spi_eeprom_write(address, size, (unsigned char*)&value);
		break;
#ifdef CONFIG_MAC_TO_MAC_MODE 
	case RT2880_SPI_INIT_VTSS_NOVLAN:
		vtss_reset();
		vtss_init(0);
		break;
	case RT2880_SPI_INIT_VTSS_VLAN:
		vtss_reset();
		vtss_init(1);
		break;
	case RT2880_SPI_VTSS_READ:
		vtss = (SPI_VTSS *)arg;
		printk("r %x %x %x -> ", vtss->blk, vtss->sub, vtss->reg);
		spi_vtss_read(vtss->blk, vtss->sub, vtss->reg, &vtss->value);
		printk("%x\n", vtss->value);
		break;
	case RT2880_SPI_VTSS_WRITE:
		vtss = (SPI_VTSS *)arg;
		printk("w %x %x %x -> %x\n", vtss->blk, vtss->sub, vtss->reg, vtss->value);
		spi_vtss_write(vtss->blk, vtss->sub, vtss->reg, vtss->value);
		break;
#endif
	default:
		printk("spi_drv: command format error\n");
	}

	return 0;
}

struct file_operations spidrv_fops = {
    ioctl:      spidrv_ioctl,
    open:       spidrv_open,
    release:    spidrv_release,
};

static int spidrv_init(void)
{

#ifdef  CONFIG_DEVFS_FS
    if(devfs_register_chrdev(spidrv_major, SPI_DEV_NAME , &spidrv_fops)) {
	printk(KERN_WARNING " spidrv: can't create device node\n");
	return -EIO;
    }

    devfs_handle = devfs_register(NULL, SPI_DEV_NAME, DEVFS_FL_DEFAULT, spidrv_major, 0,
				S_IFCHR | S_IRUGO | S_IWUGO, &spidrv_fops, NULL);
#else
    int result=0;
    result = register_chrdev(spidrv_major, SPI_DEV_NAME, &spidrv_fops);
    if (result < 0) {
        printk(KERN_WARNING "spi_drv: can't get major %d\n",spidrv_major);
        return result;
    }

    if (spidrv_major == 0) {
	spidrv_major = result; /* dynamic */
    }
#endif
    le32_to_cpu(*(volatile u_long *)RALINK_REG_GPIOMODE) &= ~(1 << 2); //use normal(SPI) mode instead of GPIO mode

    printk("spidrv_major = %d\n", spidrv_major);
    return 0;
}


static void spidrv_exit(void)
{
    printk("spi_drv exit\n");

#ifdef  CONFIG_DEVFS_FS
    devfs_unregister_chrdev(spidrv_major, SPI_DEV_NAME);
    devfs_unregister(devfs_handle);
#else
    unregister_chrdev(spidrv_major, SPI_DEV_NAME);
#endif
}

#if defined (CONFIG_RALINK_PCM) || defined (CONFIG_RALINK_PCM_MODULE)
EXPORT_SYMBOL(spi_si3220_read8);
EXPORT_SYMBOL(spi_si3220_read16);
EXPORT_SYMBOL(spi_si3220_write8);
EXPORT_SYMBOL(spi_si3220_write16);
#endif

module_init(spidrv_init);
module_exit(spidrv_exit);

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,12)
MODULE_PARM (spidrv_major, "i");
#else
module_param (spidrv_major, int, 0);
#endif

MODULE_LICENSE("GPL");
