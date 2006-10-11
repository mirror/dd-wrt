/*
 * Generic setup routines for Broadcom MIPS boards
 *
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: setup.c,v 1.6 2004/11/17 08:09:19 amin Exp $
 */

#include <linux/config.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/serialP.h>
#include <linux/ide.h>
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
#include <sbutils.h>
#include <bcmutils.h>
#include <bcmnvram.h>
#include <sbmips.h>
#include <trxhdr.h>

//#include <cy_conf.h>

extern void bcm947xx_time_init(void);
extern void bcm947xx_timer_setup(struct irqaction *irq);

#ifdef CONFIG_REMOTE_DEBUG
extern void set_debug_traps(void);
extern void rs_kgdb_hook(struct serial_state *);
extern void breakpoint(void);
#endif

#if defined(CONFIG_BLK_DEV_IDE) || defined(CONFIG_BLK_DEV_IDE_MODULE)
extern struct ide_ops std_ide_ops;
#endif

#ifdef MULTILANG_SUPPORT
#endif

/* Global SB handle */
void *bcm947xx_sbh = NULL;
spinlock_t bcm947xx_sbh_lock = SPIN_LOCK_UNLOCKED;
EXPORT_SYMBOL(bcm947xx_sbh);
EXPORT_SYMBOL(bcm947xx_sbh_lock);

/* Convenience */
#define sbh bcm947xx_sbh
#define sbh_lock bcm947xx_sbh_lock

/* Kernel command line */
char arcs_cmdline[CL_SIZE] __initdata = CONFIG_CMDLINE;

void
bcm947xx_machine_restart(char *command)
{
	printk("Please stand by while rebooting the system...\n");

	/* Set the watchdog timer to reset immediately */
	__cli();
	sb_watchdog(sbh, 1);
	while (1);
}

void
bcm947xx_machine_halt(void)
{
	printk("System halted\n");

	/* Disable interrupts and watchdog and spin forever */
	__cli();
	sb_watchdog(sbh, 0);
	while (1);
}

#ifdef CONFIG_SERIAL

static struct serial_struct rs = {
	line: 0,
	flags: ASYNC_BOOT_AUTOCONF,
	io_type: SERIAL_IO_MEM,
};

static void __init
serial_add(void *regs, uint irq, uint baud_base, uint reg_shift)
{
	rs.iomem_base = regs;
	rs.irq = irq + 2;
	rs.baud_base = baud_base / 16;
	rs.iomem_reg_shift = reg_shift;

	early_serial_setup(&rs);

	rs.line++;
}

static void __init
serial_setup(void *sbh)
{
	sb_serial_init(sbh, serial_add);

#ifdef CONFIG_REMOTE_DEBUG
	/* Use the last port for kernel debugging */
	if (rs.iomem_base)
		rs_kgdb_hook(&rs);
#endif
}

#endif /* CONFIG_SERIAL */

extern void check_enable_mips_pfc(int val);
void __init
brcm_setup(void)
{
	char *value;
	uint  pfc_val;

	/* Get global SB handle */
	sbh = sb_kattach();

	/* Initialize clocks and interrupts */
	sb_mips_init(sbh);
/*	if (BCM330X(mips_cpu.processor_id) &&
		(read_c0_diag() & BRCM_PFC_AVAIL)) {
		printk("Setting the PFC to its default value\n");
		enable_pfc(PFC_AUTO);
	}
*/

	/* 
	 * Now that the sbh is inited set the  proper PFC value 
	 */	
	pfc_val = sb_mips_get_pfc(sbh);
	printk("Setting the PFC value as 0x%x\n", pfc_val);
	check_enable_mips_pfc(pfc_val);


#ifdef CONFIG_SERIAL
	/* Initialize UARTs */
	serial_setup(sbh);
#endif

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
	_machine_power_off = bcm947xx_machine_halt;

	board_time_init = bcm947xx_time_init;
	board_timer_setup = bcm947xx_timer_setup;
}

const char *
get_system_type(void)
 {
	static char s[32];

	if (bcm947xx_sbh) {
		sprintf(s, "Broadcom BCM%X chip rev %d", sb_chip(bcm947xx_sbh),
			sb_chiprev(bcm947xx_sbh));
		return s;
	}
	else

	return "Broadcom BCM947XX";
}

void __init
bus_error_init(void)
{
}

#ifdef CONFIG_MTD_PARTITIONS

static struct mtd_partition bcm947xx_parts[] = {
	{ name: "pmon",	offset: 0, size: 0, /*mask_flags: MTD_WRITEABLE,*/ },
	{ name: "linux", offset: 0, size: 0, },
	{ name: "rootfs", offset: 0, size: 0, /*mask_flags: MTD_WRITEABLE,*/ },
	{ name: "nvram", offset: 0, size: 0, },
	{ name: "ddwrt", offset: 0, size: 0, },

	{ name: NULL, },
};

static int __init
find_root(struct mtd_info *mtd, size_t size, struct mtd_partition *part)
{
	struct cramfs_super *cramfsb;
	struct squashfs_super_block *squashfsb;
	struct trx_header *trx;

	unsigned char buf[512];
	int off;
	size_t len;

	cramfsb = (struct cramfs_super *) buf;
	squashfsb = (struct squashfs_super_block *) buf;
	trx = (struct trx_header *) buf;

	part->offset = 0;
	part->size = 0;

	for (off = 0; off < size; off += mtd->erasesize) {
		memset(buf, 0xe5, sizeof(buf));

		/*
		 * Read block 0 to test for cramfs superblock
		 */
		if (MTD_READ(mtd, off, sizeof(buf), &len, buf) ||
		    len != sizeof(buf))
			continue;

		/* Try looking at TRX header for rootfs offset */
		if (le32_to_cpu(trx->magic) == TRX_MAGIC) {
//			if (le32_to_cpu(trx->offsets[1]) > off)
			if (le32_to_cpu(trx->offsets[2]) > off) //patch for olegs lzma kernel compression
				off = le32_to_cpu(trx->offsets[2]);
			else if (le32_to_cpu(trx->offsets[1]) > off)
				off = le32_to_cpu(trx->offsets[1]);
			continue;
		}

		/* need to find cramfs */
		if (cramfsb->magic == CRAMFS_MAGIC) {
			printk(KERN_NOTICE
			       "%s: cramfs filesystem found at block %d\n",
			       mtd->name, off / BLOCK_SIZE);

			part->size = cramfsb->size;
			goto done;
		}

		/* or squashfs */
		if (squashfsb->s_magic == SQUASHFS_MAGIC) {
			printk(KERN_NOTICE
				"%s: squashfs filesystem found at block %d\n",
			       mtd->name, off / BLOCK_SIZE);
			part->size = squashfsb->bytes_used+2048;
			goto done;
		}

	}
	printk(KERN_NOTICE
	       "%s: Couldn't find valid cramfs image\n",
	       mtd->name);
	return -1;
	
done:
	part->offset = off;
	return 0;
}


struct mtd_partition * __init
init_mtd_partitions(struct mtd_info *mtd, size_t size)
{

	bcm947xx_parts[0].offset=0;
	bcm947xx_parts[0].size=256*1024;
	if (size==1024*1024*8)
	    {
	bcm947xx_parts[3].offset = size - 0x20000;
	bcm947xx_parts[3].size = size - bcm947xx_parts[3].offset;
	    }else
	    {
	bcm947xx_parts[3].offset = size - ROUNDUP(NVRAM_SPACE, mtd->erasesize);
	bcm947xx_parts[3].size = size - bcm947xx_parts[3].offset;        
	    }
	/* Find and size nvram */

	/* Find and size rootfs */
	//if (off < size) {
	//	bcm947xx_parts[2].offset = off;
	//	bcm947xx_parts[2].size = bcm947xx_parts[3].offset - bcm947xx_parts[2].offset;
	//}

	/* Find and size rootfs */
	find_root(mtd,size,&bcm947xx_parts[2]);
	


	/* Size linux (kernel and rootfs) */
	bcm947xx_parts[1].offset = bcm947xx_parts[0].size;
	bcm947xx_parts[1].size = bcm947xx_parts[3].offset - bcm947xx_parts[1].offset;



	/* calculate leftover flash, and assign it to the jffs partition */
	size_t spot;
	size_t len;
	size_t mask;
	//  get the offset to the end of the root_fs
	spot=bcm947xx_parts[2].offset+bcm947xx_parts[2].size;
	//  round it up to an erase size boundary
    spot+=mtd->erasesize-1;
	//  mask the number to the boundary
	mask=mtd->erasesize;
	mask=mask-1;
	mask=mask^0xffffffff;
	spot&=mask;
	//  length = flashsize - start position - nvram size
	len=size-spot;
	len=len-bcm947xx_parts[3].size;
	

	bcm947xx_parts[4].offset = spot;
	bcm947xx_parts[4].size = len;




	/* Size pmon */
	bcm947xx_parts[0].size = bcm947xx_parts[1].offset - bcm947xx_parts[0].offset;

	//int x;
	//for(x=0; x<5; x++) {
	//	printk(KERN_NOTICE
	//		   "Partition %d mask_flags %08x\n",
	//		   x,bcm947xx_parts[x].mask_flags);
	//}


	return bcm947xx_parts;
}

EXPORT_SYMBOL(init_mtd_partitions);

#endif
