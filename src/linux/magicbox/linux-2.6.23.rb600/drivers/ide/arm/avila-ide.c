/* drivers/ide/arm/avila-ide.c
 *
 * IDE/CF driver for the Gateworks Avila platform
 *
 * Copyright (c) 2005 Gateworks Corporation
 * Dave G <daveg@unixstudios.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/


#include <linux/module.h>
#include <linux/autoconf.h>
#include <linux/slab.h>
#include <linux/blkdev.h>
#include <linux/errno.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <asm/delay.h>


#define AVILA_IDE_BASE IXP4XX_EXP_BUS_CS1_BASE_PHYS
#define AVILA_IDE_IRQ IRQ_IXP4XX_GPIO12
#define AVILA_IDE_CONTROL 0x1e
#define AVILA_IDE_INT 12
#define AVILA_IDE_CS1_BITS 0xbfff0043;

static unsigned char _mode;

static void avila_ide_enable_16(void)
{
	if ( _mode == 0 ) {
		*IXP4XX_EXP_CS1 &= ~(0x00000001);
		udelay(100);
		_mode = 1;
	}
}

static void avila_ide_disable_16(void)
{
	if ( _mode == 1 ) {
		udelay(100);
		*IXP4XX_EXP_CS1 |= (0x00000001);
		_mode = 0;
	}
}

static u8 avila_ide_inb(unsigned long addr)
{

	return readb(addr);

}

static u16 avila_ide_inw(unsigned long addr)
{
	u16 val;

	avila_ide_enable_16();
	val = readw(addr);
	avila_ide_disable_16();
	return val;

}

static void avila_ide_insw(unsigned long addr, void *buf, u32 len)
{
	u16 *buf16p;

	avila_ide_enable_16();
	for (buf16p = (u16 *) buf; (len > 0); len--)
		*buf16p++ = readw(addr);
	avila_ide_disable_16();

}

static void avila_ide_outb(u8 val, unsigned long addr)
{

	writeb(val, addr);

}

static void avila_ide_outbsync(ide_drive_t *drive, u8 val, unsigned long addr)
{

	writeb(val, addr);

}

static void avila_ide_outw(u16 val, unsigned long addr)
{

	avila_ide_enable_16();
	writew(val, addr);
	avila_ide_disable_16();

}


static void avila_ide_outsw(unsigned long addr, void *buf, u32 len)
{
	u16 *buf16p;

	avila_ide_enable_16();
	for (buf16p = (u16 *) buf; (len > 0); len--)
		writew(*buf16p++, addr);
	avila_ide_disable_16();

}


void __init avila_ide_init(void)
{
	hw_regs_t hw;
	ide_hwif_t *hwif;
	unsigned char *avila_ide_iobase;
	int i;

	gpio_line_config(AVILA_IDE_INT, IXP4XX_GPIO_IN | IXP4XX_GPIO_STYLE_ACTIVE_HIGH);
	gpio_line_isr_clear(AVILA_IDE_INT);

	*IXP4XX_EXP_CS1 |= AVILA_IDE_CS1_BITS;
	
	avila_ide_iobase = ioremap(AVILA_IDE_BASE, 0x1000);

	memset(&hw, 0, sizeof(hw));

	hw.irq = AVILA_IDE_IRQ;
	hw.dma = NO_DMA;
	
	for (i = 0; (i <= IDE_STATUS_OFFSET); i++)
		hw.io_ports[i] = (unsigned long)(avila_ide_iobase + i);
	
	hw.io_ports[IDE_CONTROL_OFFSET] = (unsigned long)(avila_ide_iobase + AVILA_IDE_CONTROL);

	printk("ide: Gateworks Avila IDE/CF driver v1.3b\n");

	ide_register_hw(&hw, 1, &hwif);

	hwif->mmio = 2;
	hwif->OUTB = avila_ide_outb;
	hwif->OUTBSYNC = avila_ide_outbsync;
	hwif->OUTW = avila_ide_outw;
	hwif->OUTSW = avila_ide_outsw;
	hwif->INB = avila_ide_inb;
	hwif->INW = avila_ide_inw;
	hwif->INSW = avila_ide_insw;

}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dave G <daveg@unixstudios.net>");
MODULE_DESCRIPTION("Gateworks Avila CF/IDE Driver");
