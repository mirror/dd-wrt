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
#include <linux/mtd/cfi.h>
#include <linux/i2c.h>
#include <linux/i2c-algo-pca.h>
#include <linux/semaphore.h>
//#include <linux/i2c-pca-platform.h>

#include <asm/mach-types.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>
#include <asm/mach/flash.h>

#include <mach/hardware.h>
#include <mach/devices.h>
#include <mach/regs-mem.h>

#include <mach/vsopenrisc.h>

#include "generic.h"


/*****************************************************************************
 * RTC DS1337 on I2C bus
 ****************************************************************************/
static struct i2c_board_info __initdata vsopenrisc_i2c_rtc = {
		I2C_BOARD_INFO("ds1337", 0x68),
};
/*static struct i2c_pca9564_pf_platform_data  __initdata pca_data ={
	    .gpio = -1,
	    .i2c_clock_speed = I2C_PCA_CON_59kHz,
	    .timeout = 1,
};
*/
static struct resource pca_resources[] = {
	[0] = {
		 .start	= VSOPENRISC_PA_I2C_BASE,
		 .end	= VSOPENRISC_PA_I2C_BASE + 0x400 - 1,
		 .flags	= IORESOURCE_MEM | IORESOURCE_MEM_8BIT,
   	},
	[1] = {
		.start	= 0,
		.end	= 0,
		.flags	= IORESOURCE_IRQ,
	},
};
/*
static struct platform_device vsopenrisc_pca_device = {
	.name		= "i2c-pca-platform",
	.id		= -1,
	.dev		= {
			    .platform_data	= &pca_data,
			},
	.resource	= pca_resources,
	.num_resources	= ARRAY_SIZE(pca_resources),
};
*/

static struct platform_device gpio_vsopenrisc_device = {
	.name		= "gpio_vsopenrisc",
	.id		= -1,
	.num_resources	= 0,
};

static void __init ks8695_add_device_gpio_vsopenrisc(void)
{
	platform_device_register(&gpio_vsopenrisc_device);
}

#ifdef CONFIG_PCI
static int micrel_pci_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	return KS8695_IRQ_EXTERN0;
}

static struct ks8695_pci_cfg __initdata micrel_pci = {
	//.mode		= KS8695_MODE_MINIPCI,
	.mode		= KS8695_MODE_PCI,
	.map_irq	= micrel_pci_map_irq,
};
#endif

#ifdef CONFIG_MTD_CFI
static struct resource cfi_flash_resource;
static struct map_info vsopenrisc_map;
static struct mtd_info *mymtd;

extern int add_mtd_device(struct mtd_info *mtd);

static int vsopenrisc_flash_init(void)
{
	// @ 22-31: RB0NPTR, 12-21: RB0BPTR, 0-6: timg, mode
	__raw_writel(((cfi_flash_resource.end << 6) & 0xffc00000) \
				| (cfi_flash_resource.start >> 4) \
				| 0x7c, KS8695_MEM_VA + KS8695_ROMCON0);

	vsopenrisc_map.name = "Whole Flash";
	vsopenrisc_map.size = VSOPENRISC_FLASH_SIZE;
	vsopenrisc_map.phys = VSOPENRISC_FLASH_BASE;
	vsopenrisc_map.virt = ioremap(VSOPENRISC_FLASH_BASE, VSOPENRISC_FLASH_SIZE);
	vsopenrisc_map.bankwidth = 2;

	simple_map_init(&vsopenrisc_map);

	mymtd = do_map_probe("cfi_probe", &vsopenrisc_map);
	if (!mymtd)
		return -ENODEV;
	
	add_mtd_device(mymtd);
	
	return 0;
}

static struct flash_platform_data vsopenrisc_flash_data = {
	.map_name	= "cfi_probe",
	.width		= 2,
	.init		= vsopenrisc_flash_init,
};

static struct resource cfi_flash_resource = {
	.start		= VSOPENRISC_FLASH_BASE,
	.end		= VSOPENRISC_FLASH_BASE + VSOPENRISC_FLASH_SIZE - 1,
	.flags		= IORESOURCE_MEM,
};

// INFO: to use armflash, you must switch on CONFIG_MTD_ARM_INTEGRATOR
static struct platform_device cfi_flash_device = {
	.name 				= "armflash",
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

#ifdef CONFIG_PCI
	ks8695_init_pci(&micrel_pci);
#endif

#ifdef CONFIG_MTD_CFI
	platform_device_register(&cfi_flash_device);
#endif
	
	/* Add devices */
	ks8695_add_device_wan();	/* eth0 = WAN */
	ks8695_add_device_lan();	/* eth1 = LAN */
//	ks8695_add_device_hpna();	/* eth2 = HPNA */

//	i2c_register_board_info(0, &vsopenrisc_i2c_rtc, 1);
//	platform_device_register(&vsopenrisc_pca_device);
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

