/*
 * HND MIPS boards setup routines
 *
 * Copyright (C) 2012, Broadcom Corporation. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: setup.c,v 1.23 2010-10-20 08:26:12 $
 */

#include <linux/types.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/serial.h>
#include <linux/serialP.h>
#include <linux/serial_core.h>
#include <linux/serial_8250.h>	/* for early_serial_setup */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
#include <linux/config.h>
#endif
#if defined(CONFIG_BLK_DEV_IDE) || defined(CONFIG_BLK_DEV_IDE_MODULE)
#include <linux/blkdev.h>
#include <linux/ide.h>
#endif
#include <asm/bootinfo.h>
#include <asm/cpu.h>
#include <asm/time.h>
#include <asm/reboot.h>
#include <linux/squashfs_fs.h>

#ifdef CONFIG_MTD
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/romfs_fs.h>
#include <linux/cramfs_fs.h>
#endif
#ifdef CONFIG_BLK_DEV_INITRD
#include <linux/initrd.h>
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
#include <hndpci.h>
#include <trxhdr.h>
#ifdef HNDCTF
#include <ctf/hndctf.h>
#endif /* HNDCTF */
#include "bcm947xx.h"
#ifdef CONFIG_MTD_NFLASH
#include "nflash.h"
#endif
#include "bcmdevs.h"


static int __init bcm47xx_cpu_fixes(void)
{
	struct cpuinfo_mips *c = &current_cpu_data;
	if (c->cputype == CPU_74K) {
		if (CHIPID(sih->chip) == BCM4706_CHIP_ID) {
			printk(KERN_INFO "enable BCMA BUS Errata\n");
			cpu_wait = NULL;
		}
	}
	return 0;
}
arch_initcall(bcm47xx_cpu_fixes);

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

struct dummy_super_block {
	u32	s_magic ;
};

/* Global SB handle */
si_t *bcm947xx_sih = NULL;
DEFINE_SPINLOCK(bcm947xx_sih_lock);
EXPORT_SYMBOL(bcm947xx_sih);
EXPORT_SYMBOL(bcm947xx_sih_lock);

/* Convenience */
#define sih bcm947xx_sih
#define sih_lock bcm947xx_sih_lock

#ifdef HNDCTF
ctf_t *kcih = NULL;
EXPORT_SYMBOL(kcih);
ctf_attach_t ctf_attach_fn = NULL;
EXPORT_SYMBOL(ctf_attach_fn);
#endif /* HNDCTF */

/* Kernel command line */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
extern char arcs_cmdline[CL_SIZE];
#endif
static int lanports_enable = 0;


EXPORT_SYMBOL( si_router_coma );	/* for loadable modules */
EXPORT_SYMBOL( hnd_jtagm_init );
EXPORT_SYMBOL( hnd_jtagm_disable );
EXPORT_SYMBOL( jtag_scan );

static void
bcm947xx_reboot_handler(void)
{
#ifndef BCMDBG
	int wombo_reset;
#endif /* BCMDBG */

	/* Reset the PCI(e) interfaces */
	if (CHIPID(sih->chip) == BCM4706_CHIP_ID)
		hndpci_deinit(sih);

	if (lanports_enable) {
		uint lp = 1 << lanports_enable;

		si_gpioout(sih, lp, 0, GPIO_DRV_PRIORITY);
		si_gpioouten(sih, lp, lp, GPIO_DRV_PRIORITY);
		bcm_mdelay(1);
	}

#ifndef BCMDBG
	/* gpio 0 is also valid wombo_reset */
	if ((wombo_reset = getgpiopin(NULL, "wombo_reset", GPIO_PIN_NOTDEFINED)) !=
	    GPIO_PIN_NOTDEFINED) {
		int reset = 1 << wombo_reset;

		si_gpioout(sih, reset, 0, GPIO_DRV_PRIORITY);
		si_gpioouten(sih, reset, reset, GPIO_DRV_PRIORITY);
		bcm_mdelay(10);
	}
#endif /* BCMDBG */
}

void
bcm947xx_machine_restart(char *command)
{
	printk("Please stand by while rebooting the system...\n");

	/* Set the watchdog timer to reset immediately */
	local_irq_disable();
	bcm947xx_reboot_handler();
	hnd_cpu_reset(sih);
}

void
bcm947xx_machine_halt(void)
{
	printk("System halted\n");

	/* Disable interrupts and watchdog and spin forever */
	local_irq_disable();
	si_watchdog(sih, 0);
	bcm947xx_reboot_handler();
	while (1);
}

#ifdef CONFIG_SERIAL_CORE

static struct uart_port rs = {
	line: 0,
	flags: ASYNC_BOOT_AUTOCONF,
	iotype: SERIAL_IO_MEM,
};

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



int boot_flags(void)
{
	int bootflags = 0;
	char *val;

	/* Only support chipcommon revision == 38 or BCM4706 for now */
	if ((CHIPID(sih->chip) == BCM4706_CHIP_ID) || sih->ccrev == 38) {
		if (sih->ccrev == 38 && (sih->chipst & (1 << 4)) != 0) {
			/* This is NANDBOOT */
			bootflags = FLASH_BOOT_NFLASH | FLASH_KERNEL_NFLASH;
		}
		else if ((val = nvram_get("bootflags"))) {
			bootflags = simple_strtol(val, NULL, 0);
			bootflags &= FLASH_KERNEL_NFLASH;
		}
	}

	return bootflags;
}

static int rootfs_mtdblock(void)
{
	int bootflags;
	int block = 0;

	bootflags = boot_flags();

	/* NANDBOOT */
	if ((bootflags & (FLASH_BOOT_NFLASH | FLASH_KERNEL_NFLASH)) ==
		(FLASH_BOOT_NFLASH | FLASH_KERNEL_NFLASH))
		return 3;

	/* SFLASH/PFLASH only */
	if ((bootflags & (FLASH_BOOT_NFLASH | FLASH_KERNEL_NFLASH)) == 0)
		return 2;

#ifdef BCMCONFMTD
	block++;
#endif
	/* Boot from norflash and kernel in nandflash */
	return block+3;
}
int iswrt350n=0;
int iswrt300n11=0;
EXPORT_SYMBOL (iswrt350n);
EXPORT_SYMBOL (iswrt300n11);

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

	sprintf(arcs_cmdline, "root=/dev/mtdblock%d console=ttyS0,115200 init=/sbin/preinit", rootfs_mtdblock());

	/* Override default command line arguments */
	value = nvram_get("kernel_args");
	if (value && strlen(value) && strncmp(value, "empty", 5))
		strncpy(arcs_cmdline, value, sizeof(arcs_cmdline));


	if ((lanports_enable = getgpiopin(NULL, "lanports_enable", GPIO_PIN_NOTDEFINED)) ==
		GPIO_PIN_NOTDEFINED)
		lanports_enable = 0;


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
	/* Check if we want to enable cpu wait */
	if (nvram_match("wait", "1"))
		cpu_wait_enable = 1;
#endif

	/* Generic setup */
	_machine_restart = bcm947xx_machine_restart;
	_machine_halt = bcm947xx_machine_halt;
	pm_power_off = bcm947xx_machine_halt;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
	board_time_init = bcm947xx_time_init;
#endif
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

void __init memory_setup(void);
void __init
plat_mem_setup(void)
{
	memory_setup();
	brcm_setup();
	return;
}

#ifdef CONFIG_MTD

static spinlock_t *bcm_mtd_lock = NULL;

spinlock_t *partitions_lock_init(void)
{
	if (!bcm_mtd_lock) {
		bcm_mtd_lock = (spinlock_t *)kzalloc(sizeof(spinlock_t), GFP_KERNEL);
		if (!bcm_mtd_lock)
			return NULL;

		spin_lock_init( bcm_mtd_lock );
	}
	return bcm_mtd_lock;
}
EXPORT_SYMBOL(partitions_lock_init);

static struct nand_hw_control *nand_hwcontrol = NULL;
struct nand_hw_control *nand_hwcontrol_lock_init(void)
{
	if (!nand_hwcontrol) {
		nand_hwcontrol = (struct nand_hw_control *)kzalloc(sizeof(struct nand_hw_control), GFP_KERNEL);
		if (!nand_hwcontrol)
			return NULL;

		spin_lock_init(&nand_hwcontrol->lock);
		init_waitqueue_head(&nand_hwcontrol->wq);
	}
	return nand_hwcontrol;
}
EXPORT_SYMBOL(nand_hwcontrol_lock_init);

/* Find out prom size */
static uint32 boot_partition_size(uint32 flash_phys) {
	uint32 bootsz, *bisz;

	/* Default is 256K boot partition */
	bootsz = 256 * 1024;

	/* Do we have a self-describing binary image? */
	bisz = (uint32 *)KSEG1ADDR(flash_phys + BISZ_OFFSET);
	if (bisz[BISZ_MAGIC_IDX] == BISZ_MAGIC) {
		int isz = bisz[BISZ_DATAEND_IDX] - bisz[BISZ_TXTST_IDX];

		if (isz > (1024 * 1024))
			bootsz = 2048 * 1024;
		else if (isz > (512 * 1024))
			bootsz = 1024 * 1024;
		else if (isz > (256 * 1024))
			bootsz = 512 * 1024;
		else if (isz <= (128 * 1024))
			bootsz = 128 * 1024;
	}
	return bootsz;
}

#if defined(BCMCONFMTD) && defined(PLC)
#define FLASH_PARTS_NUM	7
#elif defined(BCMCONFMTD) || defined(PLC)
#define FLASH_PARTS_NUM	6
#else
#define FLASH_PARTS_NUM	5 /* boot;nvram;kernel;rootfs;empty */
#endif


#ifdef CONFIG_MTD_NFLASH
#define NFLASH_PARTS_NUM	7
static struct mtd_partition bcm947xx_nflash_parts[NFLASH_PARTS_NUM] = {{0}};
int ddwrtoffset;
int ddwrtsize;

static uint lookup_nflash_rootfs_offset(hndnand_t *nfl, struct mtd_info *mtd, int offset, size_t size)
{
	struct romfs_super_block *romfsb;
	struct cramfs_super *cramfsb;
	struct squashfs_super_block *squashfsb;
	struct trx_header *trx;
	unsigned char buf[NFL_SECTOR_SIZE];
	uint blocksize, mask, blk_offset, off, shift = 0;
	int ret;
	
	romfsb = (struct romfs_super_block *) buf;
	cramfsb = (struct cramfs_super *) buf;
	squashfsb = (struct squashfs_super_block *) buf;
	trx = (struct trx_header *) buf;


	/* Look at every block boundary till 16MB; higher space is reserved for application data. */
	blocksize = mtd->erasesize;
	printk("lookup_nflash_rootfs_offset: offset = 0x%x\n", offset);
	for (off = offset; off < NFL_BOOT_OS_SIZE; off += 4096) {
		mask = blocksize - 1;
		blk_offset = off & ~mask;
		if (hndnand_checkbadb(nfl, blk_offset) != 0)
			continue;
		memset(buf, 0xe5, sizeof(buf));
		if ((ret = hndnand_read(nfl, off, sizeof(buf), buf)) != sizeof(buf)) {
			printk(KERN_NOTICE
			       "%s: nflash_read return %d\n", mtd->name, ret);
			continue;
		}
			
		if (le32_to_cpu(trx->magic) == TRX_MAGIC)
		{
		
		printk(KERN_NOTICE "found trx at %X, len =%d\n",off,trx->len);
		int offset = le32_to_cpu(trx->offsets[2]) ? : le32_to_cpu(trx->offsets[1]);
		offset+=off;
		printk(KERN_NOTICE "scan filesys at %X\n",offset);

		if ((ret = hndnand_read(nfl, offset, sizeof(buf), buf)) != sizeof(buf)) {
			printk(KERN_NOTICE
			       "%s: nflash_read return %d\n", mtd->name, ret);
			continue;
		}
		
		if (*((__u32 *) buf) == SQUASHFS_MAGIC) {
			printk(KERN_NOTICE
			       "%s: squash filesystem with lzma found at offset %X\n",
			       mtd->name, off );
		int size = le64_to_cpu(squashfsb->bytes_used);
		//part->size = part->size + 1024; /* uncomment for belkin v2000 ! */
		int len = offset + size;
		len +=  (mtd->erasesize - 1);
		len &= ~(mtd->erasesize - 1);
		ddwrtoffset = len;
		ddwrtsize = mtd->size - ddwrtoffset;
		return offset;
		}
		}

	} 
	return shift + off;
}

struct mtd_partition *
init_nflash_mtd_partitions(hndnand_t *nfl, struct mtd_info *mtd, size_t size)
{
	int bootflags;
	int nparts = 0;
	uint32 offset = 0;
	uint shift = 0;
	uint32 top = 0;
	uint32 bootsz;
#ifdef CONFIG_FAILSAFE_UPGRADE
	char *img_boot = nvram_get(BOOTPARTITION);
	char *imag_1st_offset = nvram_get(IMAGE_FIRST_OFFSET);
	char *imag_2nd_offset = nvram_get(IMAGE_SECOND_OFFSET);
	unsigned int image_first_offset=0;
	unsigned int image_second_offset=0;
	char dual_image_on = 0;

	/* The image_1st_size and image_2nd_size are necessary if the Flash does not have any
	 * image
	 */
	dual_image_on = (img_boot != NULL && imag_1st_offset != NULL && imag_2nd_offset != NULL);

	if (dual_image_on) {
		image_first_offset = simple_strtol(imag_1st_offset, NULL, 10);
		image_second_offset = simple_strtol(imag_2nd_offset, NULL, 10);
		printk("The first offset=%x, 2nd offset=%x\n", image_first_offset,
			image_second_offset);

	}
#endif
	
	bootflags = boot_flags();
	if ((bootflags & FLASH_BOOT_NFLASH) == FLASH_BOOT_NFLASH) {
		bootsz = boot_partition_size(SI_FLASH1);
		if (bootsz > mtd->erasesize) {
			/* Prepare double space in case of bad blocks */
			bootsz = (bootsz << 1);
		} else {
			/* CFE occupies at least one block */
			bootsz = mtd->erasesize;
		}
		printk("Boot partition size = %d(0x%x)\n", bootsz, bootsz);

		/* Size pmon */
		bcm947xx_nflash_parts[nparts].name = "cfe";
		bcm947xx_nflash_parts[nparts].size = bootsz;
		bcm947xx_nflash_parts[nparts].offset = top;
		bcm947xx_nflash_parts[nparts].mask_flags = 0;
		offset = bcm947xx_nflash_parts[nparts].size;
		nparts++;

		/* Setup NVRAM MTD partition */
		bcm947xx_nflash_parts[nparts].name = "nvram";
		bcm947xx_nflash_parts[nparts].size = NFL_BOOT_SIZE - offset;
		bcm947xx_nflash_parts[nparts].offset = offset;
			
		offset = NFL_BOOT_SIZE;
		nparts++;
	}
	
	if (nvram_match("boardnum", "3500L") && nvram_match("boardtype", "0x052b"))
		offset += 0x300000;


	if ((bootflags & FLASH_KERNEL_NFLASH) == FLASH_KERNEL_NFLASH) {
		/* Setup kernel MTD partition */
		bcm947xx_nflash_parts[nparts].name = "nandimage";
#ifdef CONFIG_FAILSAFE_UPGRADE
		if (dual_image_on)
			bcm947xx_nflash_parts[nparts].size = image_second_offset - image_first_offset;
		else
#endif
		bcm947xx_nflash_parts[nparts].size = nparts ? (NFL_BOOT_OS_SIZE - NFL_BOOT_SIZE) : NFL_BOOT_OS_SIZE;
		
		if (nvram_match("boardnum", "3500L") && nvram_match("boardtype", "0x052b"))
			bcm947xx_nflash_parts[nparts].size -= 0x300000;
		
		bcm947xx_nflash_parts[nparts].offset = offset;
			
		shift = lookup_nflash_rootfs_offset(nfl, mtd, offset, bcm947xx_nflash_parts[nparts].size);

#ifdef CONFIG_FAILSAFE_UPGRADE
		if (dual_image_on)
			offset = image_second_offset;
		else
#endif
		offset = NFL_BOOT_OS_SIZE;
		nparts++;
		
		/* Setup rootfs MTD partition */
		bcm947xx_nflash_parts[nparts].name = "rootfs";
#ifdef CONFIG_FAILSAFE_UPGRADE
		if (dual_image_on)
			bcm947xx_nflash_parts[nparts].size = image_second_offset - shift;
		else
#endif
		bcm947xx_nflash_parts[nparts].size = NFL_BOOT_OS_SIZE - shift;
		bcm947xx_nflash_parts[nparts].offset = shift;
		bcm947xx_nflash_parts[nparts].mask_flags = MTD_WRITEABLE;
		nparts++;

//		bcm947xx_nflash_parts[nparts].name = "ddwrt";

//		bcm947xx_nflash_parts[nparts].size = ddwrtsize-0x100000;
//		bcm947xx_nflash_parts[nparts].offset = ddwrtoffset;
		
//		nparts++;

#ifdef CONFIG_FAILSAFE_UPGRADE
		/* Setup 2nd kernel MTD partition */
		if (dual_image_on) {
			printf ("2nd offset %x \n",image_second_offset);
			bcm947xx_nflash_parts[nparts].name = "linux2";
			bcm947xx_nflash_parts[nparts].size = NFL_BOOT_OS_SIZE - image_second_offset;
			bcm947xx_nflash_parts[nparts].offset = image_second_offset;
			shift = lookup_nflash_rootfs_offset(nfl, mtd, image_second_offset,
			                                    bcm947xx_nflash_parts[nparts].size);
			nparts++;
			/* Setup rootfs MTD partition */
			bcm947xx_nflash_parts[nparts].name = "rootfs2";
			bcm947xx_nflash_parts[nparts].size = NFL_BOOT_OS_SIZE - shift;
			bcm947xx_nflash_parts[nparts].offset = shift;
			bcm947xx_nflash_parts[nparts].mask_flags = MTD_WRITEABLE;
			nparts++;
		}
#endif

	}

	return bcm947xx_nflash_parts;
}

EXPORT_SYMBOL(init_nflash_mtd_partitions);
#endif /* CONFIG_MTD_NFLASH */

#endif
