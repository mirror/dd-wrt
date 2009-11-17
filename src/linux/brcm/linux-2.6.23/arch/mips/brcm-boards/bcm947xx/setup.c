/*
 * HND MIPS boards setup routines
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: setup.c,v 1.6 2008/04/03 03:49:45 Exp $
 */

#include <linux/types.h>
#include <linux/config.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/serial.h>
#include <linux/serialP.h>
#include <linux/serial_core.h>
#if defined(CONFIG_BLK_DEV_IDE) || defined(CONFIG_BLK_DEV_IDE_MODULE)
#include <linux/blkdev.h>
#include <linux/ide.h>
#endif
#include <asm/bootinfo.h>
#include <asm/cpu.h>
#include <asm/time.h>
#include <asm/reboot.h>

#ifdef CONFIG_MTD_PARTITIONS
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/minix_fs.h>
#include <linux/ext2_fs.h>
#include <linux/romfs_fs.h>
#include <linux/cramfs_fs.h>
#include <linux/squashfs_fs.h>
#endif

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmnvram.h>
#include <siutils.h>
#include <hndsoc.h>
#include <hndcpu.h>
#include <mips33_core.h>
#include <mips74k_core.h>
#include <sbchipc.h>
#include <hndchipc.h>
#include <trxhdr.h>
#include "bcm947xx.h"

extern void bcm947xx_time_init(void);
extern void bcm947xx_timer_setup(struct irqaction *irq);

#ifdef CONFIG_KGDB
extern void set_debug_traps(void);
extern void rs_kgdb_hook(struct uart_port *);
extern void breakpoint(void);
#endif

#if defined(CONFIG_BLK_DEV_IDE) || defined(CONFIG_BLK_DEV_IDE_MODULE)
extern struct ide_ops std_ide_ops;
#endif

/* Global SB handle */
si_t *bcm947xx_sih = NULL;
spinlock_t bcm947xx_sih_lock = SPIN_LOCK_UNLOCKED;
EXPORT_SYMBOL(bcm947xx_sih);
EXPORT_SYMBOL(bcm947xx_sih_lock);

/* Convenience */
#define sih bcm947xx_sih
#define sih_lock bcm947xx_sih_lock

/* Kernel command line */
extern char arcs_cmdline[CL_SIZE];

void
bcm947xx_machine_restart(char *command)
{
	printk("Please stand by while rebooting the system...\n");

	/* Set the watchdog timer to reset immediately */
	local_irq_disable();
	hnd_cpu_reset(sih);
}

void
bcm947xx_machine_halt(void)
{
	printk("System halted\n");

	/* Disable interrupts and watchdog and spin forever */
	local_irq_disable();
	si_watchdog(sih, 0);
	while (1);
}

#ifdef CONFIG_SERIAL_CORE

static struct uart_port rs = {
	line: 0,
	flags: ASYNC_BOOT_AUTOCONF,
	iotype: SERIAL_IO_MEM,
};

extern int early_serial_setup(struct uart_port *port);

static void __init
serial_add(void *regs, uint irq, uint baud_base, uint reg_shift)
{
	rs.membase = regs;
	rs.irq = irq + 2;
	rs.uartclk = baud_base;
	rs.regshift = reg_shift;

	early_serial_setup(&rs);

	rs.line++;
}

static void __init
serial_setup(si_t *sih)
{
	si_serial_init(sih, serial_add);

#ifdef CONFIG_KGDB
	/* Use the last port for kernel debugging */
	if (rs.membase)
		rs_kgdb_hook(&rs);
#endif
}

#endif /* CONFIG_SERIAL_CORE */
int iswrt350n=0;
int iswrt300n11=0;
int iswnr3500v2=0;
int iswrt320n=0;
int iswrt160nv3=0;
EXPORT_SYMBOL (iswrt350n);
EXPORT_SYMBOL (iswrt300n11);
EXPORT_SYMBOL (iswnr3500v2);
EXPORT_SYMBOL (iswrt320n);
EXPORT_SYMBOL (iswrt160nv3);

void __init
brcm_setup(void)
{
	char *value;

	/* Get global SB handle */
	sih = si_kattach(SI_OSH);

	/* Initialize clocks and interrupts */
	si_mips_init(sih, SBMIPS_VIRTIRQ_BASE);

	if (BCM330X(current_cpu_data.processor_id) &&
		(read_c0_diag() & BRCM_PFC_AVAIL)) {
		/* 
		 * Now that the sih is inited set the  proper PFC value 
		 */	
		printk("Setting the PFC to its default value\n");
		enable_pfc(PFC_AUTO);
	}


#ifdef CONFIG_SERIAL_CORE
	/* Initialize UARTs */
	serial_setup(sih);
#endif /* CONFIG_SERIAL_CORE */

iswrt350n=1;
iswrt300n11=1;
char *boardtype = nvram_get("boardtype");
char *boothwmodel = nvram_get("boot_hw_model");
char *boothwver = nvram_get("boot_hw_ver");
char *cardbus = nvram_get("cardbus");
char *boardrev = nvram_get("boardrev");
if (boardrev!=NULL && !strcmp(boardrev,"0x1700") && boardtype!=NULL && !strcmp(boardtype,"0x04cd"))iswrt160nv3=1;
if (boardrev!=NULL && (!strcmp(boardrev,"0x1213") || !strcmp(boardrev,"02")) && boardtype!=NULL && !strcmp(boardtype,"0x04CF"))iswnr3500v2=1;
if (boardrev!=NULL && (!strcmp(boardrev,"0x1304") || !strcmp(boardrev,"0x1305")) && boardtype!=NULL && !strcmp(boardtype,"0x04EF"))iswrt320n=1;
if (boardtype==NULL || strcmp(boardtype,"0x478"))iswrt350n=0;
if (cardbus!=NULL && !strcmp(cardbus, "0") && boardtype!=NULL && !strcmp(boardtype,"0x478"))
    {
    iswrt350n=0;
    iswrt300n11=0;
    }
if (boothwmodel==NULL || strcmp(boothwmodel,"WRT300N"))iswrt300n11=0;
if (boothwmodel!=NULL && !strcmp(boothwmodel,"WRT610N"))
    {
    iswrt300n11=0;
    iswrt350n=0;
    }
if (boothwver==NULL || strcmp(boothwver,"1.1"))iswrt300n11=0;
if (iswrt300n11)
    iswrt350n=0;

#if defined(CONFIG_BLK_DEV_IDE) || defined(CONFIG_BLK_DEV_IDE_MODULE)
	ide_ops = &std_ide_ops;
#endif

	/* Override default command line arguments */
	value = nvram_get("kernel_args");
	if (value && strlen(value) && strncmp(value, "empty", 5))
		strncpy(arcs_cmdline, value, sizeof(arcs_cmdline));


	/* Generic setup */
	_machine_restart = bcm947xx_machine_restart;
	_machine_halt = bcm947xx_machine_halt;
	pm_power_off = bcm947xx_machine_halt;

	board_time_init = bcm947xx_time_init;
}

const char *
get_system_type(void)
{
	static char s[32];

	if (bcm947xx_sih) {
		sprintf(s, "Broadcom BCM%X chip rev %d", bcm947xx_sih->chip,
			bcm947xx_sih->chiprev);
		return s;
	}
	else
		return "Broadcom BCM947XX";
}

void __init
bus_error_init(void)
{
}

void __init
plat_mem_setup(void)
{
	brcm_setup();
	return;
}

