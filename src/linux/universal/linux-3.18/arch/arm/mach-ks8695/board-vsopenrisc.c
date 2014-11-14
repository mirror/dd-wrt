/*
 * arch/arm/mach-ks8695/vsopenrisc.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/physmap.h>
#include <linux/mtd/cfi.h>
#include <linux/i2c.h>
#include <linux/i2c-algo-pca.h>
#include <linux/semaphore.h>
//#include <linux/i2c-pca-platform.h>
#include <linux/ata_platform.h>

#include <linux/i2c.h>
#include <linux/i2c-algo-pca.h>
#include <linux/i2c-pca-platform.h>

#include <asm/mach-types.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>
#include <asm/mach/flash.h>

#include <mach/hardware.h>
#include <mach/devices.h>
#include <mach/regs-mem.h>
#include <mach/regs-irq.h>
#include <mach/gpio-ks8695.h>

#include <mach/vsopenrisc.h>

#include "generic.h"

/*****************************************************************************
 * IDE
 ****************************************************************************/
static struct resource ide_resources[] = {
	[0] = {
		 .start	= VSOPENRISC_EPLD_IDE_DATA_PA,
		 .end	= VSOPENRISC_EPLD_IDE_DATA_PA + 0x400 - 1,
		 .flags	= IORESOURCE_MEM | IORESOURCE_MEM_8BIT,
   	},
	[1] = {
		 .start	= VSOPENRISC_EPLD_IDE_CTRL_PA,
		 .end	= VSOPENRISC_EPLD_IDE_CTRL_PA + 0x100 - 1,
		 .flags	= IORESOURCE_MEM | IORESOURCE_MEM_8BIT,
   	},
	[2] = {
		.start	= KS8695_INTEPLD_IDE,
		.end	= 0,
		.flags	= IORESOURCE_IRQ | IRQF_SHARED | IORESOURCE_IRQ_SHAREABLE, 
	},
};
static struct pata_platform_info __initdata ide_data = {
	.ioport_shift = 2,
	.irq_flags = IRQF_SHARED,
};

static struct platform_device vsopenrisc_ide_device = {
	.name		= "pata_platform",
	.id		= -1,
	.dev		= {
			    .platform_data	= &ide_data,
			},
	.resource	= ide_resources,
	.num_resources	= ARRAY_SIZE(ide_resources),
};




/*****************************************************************************
 * RTC DS1337 on I2C bus
 ****************************************************************************/
static struct i2c_board_info __initdata vsopenrisc_i2c_rtc = {
		I2C_BOARD_INFO("ds1337", 0x68),
};
static struct i2c_pca9564_pf_platform_data  __initdata pca_data ={
	    .gpio = -1,
	    .i2c_clock_speed = I2C_PCA_CON_59kHz,
	    .timeout = HZ,
};

static struct resource pca_resources[] = {
	[0] = {
		 .start	= VSOPENRISC_PA_I2C_BASE,
		 .end	= VSOPENRISC_PA_I2C_BASE + 0x400 - 1,
		 .flags	= IORESOURCE_MEM | IORESOURCE_MEM_32BIT,
   	},
	[1] = {
		.start	= 0,
		.end	= 0,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device vsopenrisc_pca_device = {
	.name		= "i2c-pca-platform",
	.id		= -1,
	.dev		= {
			    .platform_data	= &pca_data,
			},
	.resource	= pca_resources,
	.num_resources	= ARRAY_SIZE(pca_resources),
};

static struct platform_device gpio_vsopenrisc_device = {
	.name		= "gpio_vsopenrisc",
	.id		= -1,
	.num_resources	= 0,
};

static void __init ks8695_add_device_gpio_vsopenrisc(void)
{
	platform_device_register(&gpio_vsopenrisc_device);
}


/*****************************************************************************
 * External GPIO device
 ****************************************************************************/
#define	KS8695_MEM_WRITE(offset, v)	__raw_writel((v), KS8695_MEM_VA + (offset))
#define	KS8695_MEM_READ(offset)		__raw_readl(KS8695_MEM_VA + (offset))

#define EXTIO_SET_ADDR(_start_, _size_) \
	( \
	 	( ((_start_) >> 16) << 12) | \
	 	( (((_start_)+(_size_)-1) >> 16) << 22) \
	)

#define XBOUNDS(a,b,c)	(max(a,min(b,c)))
#define EXTIO_SET_TIMG(tacs,tcos,tact,tcoh) \
	( XBOUNDS(0,(tacs)-0,7)	<< 3	\
	| XBOUNDS(0,(tcos)-1,7) << 0	\
	| XBOUNDS(0,(tact)-1,7)	<< 9	\
	| XBOUNDS(0,(tcoh)-1,7)	<< 6	\
	)


void __init ks8695p_extio_init(void)
{
	u32	uReg;

#ifdef	DEBUG_THIS
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif

	/* EXTIO0 has data width word
	 */
	uReg = KS8695_MEM_READ(KS8695_ERGCON);
	uReg = (uReg & ~(0x3 << 16) ) | (0x3 << 16);
	KS8695_MEM_WRITE(KS8695_ERGCON, uReg);

	/* Enable EXTIO0 and configure timing
	 */
	uReg = EXTIO_SET_ADDR(VSOPENRISC_PA_EXTIO0_BASE, VSOPENRISC_EXTIO0_SIZE);
	/* tacs=32ns tcos=16ns tact=48ns tcoh=16ns
	 */
	//uReg |= (0x4<<3) | (0x1<<0) | (0x5<<9) | (0x1<<6);
	/*  tacs=4*8ns, tcos=2*8ns, tact=7*8ns, tcoh=2*8ns  */
	uReg |= EXTIO_SET_TIMG(4,2,7,2);
	KS8695_MEM_WRITE(KS8695_EXTACON0, uReg);
}


#ifdef CONFIG_PCI
static int micrel_pci_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	return KS8695_IRQ_EXTERN0;
}

static struct ks8695_pci_cfg __initdata micrel_pci = {
	.mode		= KS8695_MODE_PCI,
	.map_irq	= micrel_pci_map_irq,
};
#endif

/*****************************************************************************
 * Flash
 ****************************************************************************/
#ifdef CONFIG_MTD_CFI
static struct resource cfi_flash_resource;

static struct mtd_partition vsopenrisc_partitions[] = {
	{
		.name = "Whole Flash",
		.offset = 0,
		.size = VSOPENRISC_FLASH_SIZE,
	},
	{
		.name = "RedBoot",
		.offset = 0,
		.size = 0x10000,
	},
	{
		.name = "File System",
		.offset = MTDPART_OFS_NXTBLK,
		.size = 0x1F7000,
	},
	{
		.name = "Kernel",
		.offset = MTDPART_OFS_NXTBLK,
		.size = 0x1F6000,
	},
	{
		.name = "Parameter",
		.offset = MTDPART_OFS_NXTBLK,
		.size = 0x1000,
	},
	{
		.name = "RedBoot config",
		.offset = MTDPART_OFS_NXTBLK,
		.size = 0x1000,
	},
	{
		.name = "FIS directory",
		.offset = MTDPART_OFS_NXTBLK,
		.size = 0x1000,
	},
};
static struct physmap_flash_data vsopenrisc_flash_data = {
	.width		= 2,
	.parts		= vsopenrisc_partitions,
	.nr_parts   = ARRAY_SIZE(vsopenrisc_partitions),
};

static struct resource cfi_flash_resource = {
	.start		= VSOPENRISC_FLASH_BASE,
	.end		= VSOPENRISC_FLASH_BASE +  VSOPENRISC_FLASH_SIZE - 1,
	.flags		= IORESOURCE_MEM,
};

static struct platform_device cfi_flash_device = {
	.name 				= "physmap-flash",
	.id					= 0,
	.dev				= {
			.platform_data = &vsopenrisc_flash_data,
	},
	.num_resources		= 1,
	.resource			= &cfi_flash_resource,
};
#endif


int vs_sysid;
struct semaphore ser_can_sync_sem;

static int __init vssysid(char *str)
{
	vs_sysid = simple_strtol(str,NULL,0);
	return 1;
}
__setup("vssysid=", vssysid);


#define VS_WDT_TIME 171
#ifdef CONFIG_MACH_KS8695_VSOPENRISC_WDT_ENABLE
static void enable_wdt(int time)
{
	unsigned long tmcon;
	unsigned long tval = time * CLOCK_TICK_RATE;

	/* disable timer0 */
	tmcon = __raw_readl(KS8695_TMR_VA + KS8695_TMCON);
	__raw_writel(tmcon & ~TMCON_T0EN, KS8695_TMR_VA + KS8695_TMCON);

	/* program timer0 */
	__raw_writel(tval | T0TC_WATCHDOG, KS8695_TMR_VA + KS8695_T0TC);

	/* re-enable timer0 */
	tmcon = __raw_readl(KS8695_TMR_VA + KS8695_TMCON);
	__raw_writel(tmcon | TMCON_T0EN, KS8695_TMR_VA + KS8695_TMCON);

}
#endif

static void __init vsopenrisc_init(void)
{
	switch(vs_sysid)
	{
		case 1:
			printk("VScom OpenRISC Alekto initialized.\n");
			break;
		case 2: 
			printk("VScom OpenRISC Alena initialized.\n");
			break;
		default:
			printk("VScom OpenRISC Alekto initialized.\n");
	}

	sema_init(&ser_can_sync_sem,1);

#ifdef CONFIG_MACH_KS8695_VSOPENRISC_WDT_ENABLE
	printk(KERN_INFO "Starting watchdog timer (%d seconds)\n", VS_WDT_TIME);
	enable_wdt(VS_WDT_TIME);
#endif

	/* setup interrupt sources attached to GPIO controller */
	ks8695_gpio_interrupt(KS8695_GPIO_1, IRQ_TYPE_LEVEL_LOW);
	ks8695_gpio_interrupt(KS8695_GPIO_2, IRQ_TYPE_LEVEL_LOW);
	ks8695_gpio_interrupt(KS8695_GPIO_3, IRQ_TYPE_LEVEL_HIGH);

#ifdef CONFIG_PCI
	/* activating the EXT0 interrupt */
	ks8695_gpio_interrupt(KS8695_GPIO_0, IRQ_TYPE_LEVEL_LOW);
	ks8695_init_pci(&micrel_pci);
#endif

#ifdef CONFIG_MTD_CFI
	platform_device_register(&cfi_flash_device);
#endif
	
	/* Add devices */
	ks8695_add_device_wan();	/* eth0 = WAN */
	ks8695_add_device_lan();	/* eth1 = LAN */

	/* add I2C controller and RTC */
	i2c_register_board_info(0, &vsopenrisc_i2c_rtc, 1);
	platform_device_register(&vsopenrisc_pca_device);

	/* add IDE */
	platform_device_register(&vsopenrisc_ide_device);



	ks8695_add_device_gpio_vsopenrisc();
}

static struct _ser_can_sync_tab
{
	unsigned char tty_dev_nr;
	unsigned char can_dev_nr;
	unsigned long tty_count;
	unsigned long can_count;
} ser_can_sync_tab[] =
{
	{1,-1,0,0},
	{2,-1,0,0},
	{3,-1,0,0},
	{4,0,0,0}
};

int check_and_set_dev_open_status(int dev_nr, unsigned char flag)
{
	int i, ret = 0;

	if (down_interruptible(&ser_can_sync_sem))
		return -ERESTARTSYS;

	for(i = 0;i < sizeof(ser_can_sync_tab)/sizeof(ser_can_sync_tab[0]);i++)
	{
		if(((flag & DRV_IN_USE_TTY) && (dev_nr == ser_can_sync_tab[i].can_dev_nr))
			|| ((flag & DRV_IN_USE_CAN) && (dev_nr == ser_can_sync_tab[i].tty_dev_nr)))
		{
			if(((flag & DRV_IN_USE_CAN) && (ser_can_sync_tab[i].can_count > 0)) ||
				((flag & DRV_IN_USE_TTY) && (ser_can_sync_tab[i].tty_count > 0)))
			{
				ret = -EBUSY;
			}
			else
			{
				if(flag & DRV_IN_USE_CAN)
					ser_can_sync_tab[i].tty_count++;
				else
					ser_can_sync_tab[i].can_count++;
			}
				break;
		}
	}

	up(&ser_can_sync_sem);
	return ret;
}

int check_dev_open_status(int dev_nr, unsigned char flag)
{
	int i, ret = 0;

	if (down_interruptible(&ser_can_sync_sem))
		return -ERESTARTSYS;

	for(i = 0;i < sizeof(ser_can_sync_tab)/sizeof(ser_can_sync_tab[0]);i++)
	{
		if(((flag & DRV_IN_USE_TTY) && (dev_nr == ser_can_sync_tab[i].can_dev_nr))
			|| ((flag & DRV_IN_USE_CAN) && (dev_nr == ser_can_sync_tab[i].tty_dev_nr)))
		{
			if(((flag & DRV_IN_USE_CAN) && (ser_can_sync_tab[i].can_count > 0)) ||
				((flag & DRV_IN_USE_TTY) && (ser_can_sync_tab[i].tty_count > 0)))
			{
				ret = -EBUSY;
			}
			break;
		}
	}

	up(&ser_can_sync_sem);
	return ret;
}

void clear_dev_open_status(int dev_nr, unsigned char flag)
{
	int i;

	down(&ser_can_sync_sem);

	for(i = 0;i < sizeof(ser_can_sync_tab)/sizeof(ser_can_sync_tab[0]);i++)
	{
		if(((flag & DRV_IN_USE_TTY) && (dev_nr == ser_can_sync_tab[i].can_dev_nr))
			|| ((flag & DRV_IN_USE_CAN) && (dev_nr == ser_can_sync_tab[i].tty_dev_nr)))
		{
			if(flag & DRV_IN_USE_CAN)
			{
				if(ser_can_sync_tab[i].tty_count > 0)
					ser_can_sync_tab[i].tty_count--;
			}
			else
			{
				if(ser_can_sync_tab[i].can_count > 0)
					ser_can_sync_tab[i].can_count--;
			}
			break;

		}
	}

	up(&ser_can_sync_sem);
}

MACHINE_START(KS8695, "VScom OpenRISC platform")
	/* Maintainer: VScom */
	.atag_offset	= 0x100,
	.map_io		= ks8695_map_io,
	.init_irq	= ks8695_init_irq,
	.init_machine	= vsopenrisc_init,
	.init_time	= ks8695_timer_init,
	.restart	= ks8695_restart,
MACHINE_END

