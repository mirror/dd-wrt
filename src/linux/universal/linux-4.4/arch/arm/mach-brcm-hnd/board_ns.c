#include <linux/version.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <asm/clkdev.h>
#include <asm/cacheflush.h>
#include <asm/hardware/cache-l2x0.h>

#include <mach/clkdev.h>
#include <mach/memory.h>
#include <mach/io_map.h>

#include <plat/bsp.h>
#include <plat/mpcore.h>
#include <plat/plat-bcm5301x.h>

#ifdef CONFIG_MTD
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand.h>
#include <linux/romfs_fs.h>
#include <linux/cramfs_fs.h>
#include <linux/squashfs_fs.h>
#endif

#include <linux/memblock.h>
#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmnvram.h>
#include <bcmendian.h>
#include <hndsoc.h>
#include <siutils.h>
#include <hndcpu.h>
#include <hndpci.h>
#include <pcicfg.h>
#include <bcmdevs.h>
#include <trxhdr.h>
#ifdef HNDCTF
#include <ctf/hndctf.h>
#endif				/* HNDCTF */
#include <hndsflash.h>
#ifdef CONFIG_MTD_NFLASH
#include <hndnand.h>
#endif

extern char __initdata saved_root_name[];

/* Global SB handle */
si_t *bcm947xx_sih = NULL;
DEFINE_SPINLOCK(bcm947xx_sih_lock);
EXPORT_SYMBOL(bcm947xx_sih);
EXPORT_SYMBOL(bcm947xx_sih_lock);

extern int nvram_space;
/* Convenience */
#define sih bcm947xx_sih
#define sih_lock bcm947xx_sih_lock

#ifdef HNDCTF
ctf_t *kcih = NULL;
EXPORT_SYMBOL(kcih);
ctf_attach_t ctf_attach_fn = NULL;
EXPORT_SYMBOL(ctf_attach_fn);
#endif				/* HNDCTF */


/* To store real PHYS_OFFSET value */
unsigned int ddr_phys_offset_va = -1;
EXPORT_SYMBOL(ddr_phys_offset_va);

unsigned int ddr_phys_offset2_va = 0xa8000000;	/* Default value for NS */
EXPORT_SYMBOL(ddr_phys_offset2_va);

unsigned int coherence_win_sz = SZ_256M;
EXPORT_SYMBOL(coherence_win_sz);

/*
 * Coherence flag:
 * 0: arch is non-coherent with NS ACP or BCM53573 ACE (CCI-400) disabled.
 * 1: arch is non-coherent with NS-Ax ACP enabled for ACP WAR.
 * 2: arch is coherent with NS-Bx ACP enabled.
 * 4: arch is coherent with BCM53573 ACE enabled.
 * give non-zero initial value to let this global variable be stored in Data Segment
 */
unsigned int coherence_flag = ~(COHERENCE_MASK);
EXPORT_SYMBOL(coherence_flag);


/* This is the main reference clock 25MHz from external crystal */
static struct clk clk_ref = {
	.name = "Refclk",
	.rate = 25 * 1000000,	/* run-time override */
	.fixed = 1,
	.type = CLK_XTAL,
};

static struct clk_lookup board_clk_lookups[] = {
	{
	 .con_id = "refclk",
	 .clk = &clk_ref,
	 }
};

extern int _memsize;

extern int _chipid;

#if 0
#include <mach/uncompress.h>

void printch(int c)
{
	putc(c);
	flush();
}
#endif
void __init board_map_io(void)
{
	early_printk("board_map_io\n");

	if (BCM53573_CHIP(_chipid)) {
		/* Override the main reference clock to be 40 MHz */
		clk_ref.rate = 40 * 1000000;
	}
	/* Install clock sources into the lookup table */
	clkdev_add_table(board_clk_lookups, ARRAY_SIZE(board_clk_lookups));

	/* Map SoC specific I/O */
	soc_map_io(&clk_ref);
}

void __init board_init_irq(void)
{
	early_printk("board_init_irq\n");
	soc_init_irq();

	/* serial_setup(sih); */
}

void board_init_timer(void)
{
	/* Get global SB handle */
	sih = si_kattach(SI_OSH);
	early_printk("board_init_timer\n");
	soc_init_timer();
}

static int __init rootfs_mtdblock(void)
{
	int bootdev;
	int knldev;
	int block = 0;
#ifdef CONFIG_FAILSAFE_UPGRADE
	char *img_boot = nvram_get(BOOTPARTITION);
#endif

	bootdev = soc_boot_dev((void *)sih);
	knldev = soc_knl_dev((void *)sih);

	/* NANDBOOT */
	if (bootdev == SOC_BOOTDEV_NANDFLASH && knldev == SOC_KNLDEV_NANDFLASH) {
#ifdef CONFIG_FAILSAFE_UPGRADE
		if (img_boot && simple_strtol(img_boot, NULL, 10))
			return 5;
		else
			return 3;
#else
		return 3;
#endif
	}

	/* SFLASH/PFLASH only */
	if (bootdev != SOC_BOOTDEV_NANDFLASH && knldev != SOC_KNLDEV_NANDFLASH) {
#ifdef CONFIG_FAILSAFE_UPGRADE
		if (img_boot && simple_strtol(img_boot, NULL, 10))
			return 4;
		else
			return 2;
#else
		return 2;
#endif
	}
#ifdef BCMCONFMTD
	block++;
#endif
#ifdef CONFIG_FAILSAFE_UPGRADE
	if (img_boot && simple_strtol(img_boot, NULL, 10))
		block += 2;
#endif
	/* Boot from norflash and kernel in nandflash */
	return block + 3;
}

#define WATCHDOG_MIN    3000	/* milliseconds */
extern int panic_timeout;
extern int panic_on_oops;
static int watchdog = 0;

static void __init brcm_setup(void)
{

	if (ACP_WAR_ENAB() && BCM4707_CHIP(CHIPID(sih->chip))) {
		if (sih->chippkg == BCM4708_PKG_ID)
			coherence_win_sz = SZ_128M;
		else if (sih->chippkg == BCM4707_PKG_ID)
			coherence_win_sz = SZ_32M;
		else
			coherence_win_sz = SZ_256M;
	} else if ((BCM4707_CHIP(CHIPID(sih->chip)) &&
		(CHIPREV(sih->chiprev) == 4 || CHIPREV(sih->chiprev) == 6)) ||
		(CHIPID(sih->chip) == BCM47094_CHIP_ID)) {
		/* For NS-Bx and NS47094. Chiprev 4 for NS-B0 and chiprev 6 for NS-B1 */
		coherence_win_sz = SZ_1G;
	} else if (BCM53573_CHIP(sih->chip)) {
		if (PHYS_OFFSET == PADDR_ACE1_BCM53573)
			coherence_win_sz = SZ_512M;
		else
			coherence_win_sz = SZ_256M;
	}
	
	printk(KERN_INFO "coherence_win_size = %X\n",coherence_win_sz);
	printk(KERN_INFO "coherence_flag = %X\n", coherence_flag);
	printk(KERN_INFO "ddr_phys_offset_va =%X\n", ddr_phys_offset_va);
	printk(KERN_INFO "ddr_phys_offset2_va =%X\n", ddr_phys_offset2_va);

//      if (strncmp(boot_command_line, "root=/dev/mtdblock", strlen("root=/dev/mtdblock")) == 0)
//              sprintf(saved_root_name, "/dev/mtdblock%d", rootfs_mtdblock());
	/* Set watchdog interval in ms */
	watchdog = simple_strtoul(nvram_safe_get("watchdog"), NULL, 0);

	/* Ensure at least WATCHDOG_MIN */
	if ((watchdog > 0) && (watchdog < WATCHDOG_MIN))
		watchdog = WATCHDOG_MIN;

}

void soc_watchdog(void)
{
	if (watchdog > 0)
		si_watchdog_ms(sih, watchdog);
}

void __init board_init(void)
{
	early_printk("board_init\n");
	brcm_setup();
	/*
	 * Add common platform devices that do not have board dependent HW
	 * configurations
	 */
	soc_add_devices();

	return;
}

void __init board_fixup(struct tag *t, char **cmdline)
{
	early_printk("board_fixup\n");

	u32 mem_size, lo_size;
	early_printk("board_fixup\n");

	/* Fuxup reference clock rate */
//      if (desc->nr == MACH_TYPE_BRCM_NS_QT )
//              clk_ref.rate = 17594;   /* Emulator ref clock rate */

	if (BCM53573_CHIP(_chipid)) {
		u32 size;
		/* 53573's DDR limitation size is 512MB for shadow region. */
		/* 256MB for first region */
		size = (PHYS_OFFSET == PADDR_ACE1_BCM53573) ? SZ_512M : SZ_256M;
		if (_memsize > size)
			_memsize = size;
	}

	mem_size = _memsize;

	early_printk("board_fixup: mem=%uMiB\n", mem_size >> 20);

	/* for NS-B0-ACP */
#if 1
	if (ACP_WAR_ENAB() || (BCM53573_CHIP(_chipid))) {
		memblock_add(PHYS_OFFSET, mem_size);
		return;
	}
#endif

	lo_size = min(mem_size, DRAM_MEMORY_REGION_SIZE);

	memblock_add(PHYS_OFFSET, lo_size);


	if (lo_size == mem_size)
		return;

	memblock_add(DRAM_LARGE_REGION_BASE + lo_size, mem_size - lo_size);
}

#ifdef CONFIG_ZONE_DMA
/*
 * Adjust the zones if there are restrictions for DMA access.
 */
void __init bcm47xx_adjust_zones(unsigned long *size, unsigned long *hole)
{
	unsigned long dma_size = SZ_128M >> PAGE_SHIFT;

	if (size[0] <= dma_size)
		return;
#if 0
	if (ACP_WAR_ENAB())
		return;
#endif

	size[ZONE_NORMAL] = size[0] - dma_size;
	size[ZONE_DMA] = dma_size;
	hole[ZONE_NORMAL] = hole[0];
	hole[ZONE_DMA] = 0;
}
#endif				/* CONFIG_ZONE_DMA */

//#if (( (IO_BASE_VA >>18) & 0xfffc) != 0x3c40)
//#error IO_BASE_VA 
//#endif

void brcm_reset(char mode, const char *cmd)
{
#ifdef CONFIG_OUTER_CACHE_SYNC
	outer_cache.sync = NULL;
#endif
	hnd_cpu_reset(sih);
}

MACHINE_START(BRCM_NS, "Northstar Prototype")
//   .phys_io =                                         /* UART I/O mapping */
//      IO_BASE_PA,
//   .io_pg_offst =                             /* for early debug */
//      (IO_BASE_VA >>18) & 0xfffc,
    .smp = smp_ops(brcm_smp_ops),
    .fixup = board_fixup,	/* Opt. early setup_arch() */
    .map_io = board_map_io,	/* Opt. from setup_arch() */
    .init_irq = board_init_irq,	/* main.c after setup_arch() */
    .init_time = board_init_timer,	/* main.c after IRQs */
    .init_machine = board_init,	/* Late archinitcall */
    .atag_offset = CONFIG_BOARD_PARAMS_PHYS,.restart = brcm_reset,
#ifdef CONFIG_ZONE_DMA
    .dma_zone_size = SZ_128M,
#endif
    MACHINE_END
#ifdef	CONFIG_MACH_BRCM_NS_QT
MACHINE_START(BRCM_NS_QT, "Northstar Emulation Model")
//   .phys_io =                                         /* UART I/O mapping */
//      IO_BASE_PA,
//   .io_pg_offst =                             /* for early debug */
//      (IO_BASE_VA >>18) & 0xfffc,
    .fixup = board_fixup,		/* Opt. early setup_arch() */
    .map_io = board_map_io,	/* Opt. from setup_arch() */
    .init_irq = board_init_irq,	/* main.c after setup_arch() */
    .init_time = board_init_timer,	/* main.c after IRQs */
    .init_machine = board_init,	/* Late archinitcall */
    .atag_offset = CONFIG_BOARD_PARAMS_PHYS,.restart = brcm_reset,
#ifdef CONFIG_ZONE_DMA
    .dma_zone_size = SZ_128M,
#endif
    MACHINE_END
#endif
#ifdef CONFIG_MTD
static spinlock_t *bcm_mtd_lock = NULL;

spinlock_t *partitions_lock_init(void)
{
	if (!bcm_mtd_lock) {
		bcm_mtd_lock = (spinlock_t *) kzalloc(sizeof(spinlock_t), GFP_KERNEL);
		if (!bcm_mtd_lock)
			return NULL;

		spin_lock_init(bcm_mtd_lock);
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
static uint32 boot_partition_size(uint32 flash_phys)
{
	uint32 bootsz, *bisz;

	/* Default is 256K boot partition */
	bootsz = 256 * 1024;

	/* Do we have a self-describing binary image? */
	bisz = (uint32 *) (flash_phys + BISZ_OFFSET);
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

size_t rootfssize = 0;

#if defined(BCMCONFMTD)
#define MTD_PARTS 1
#else
#define MTD_PARTS 0
#endif
#if defined(PLC)
#define PLC_PARTS 1
#else
#define PLC_PARTS 0
#endif
#if defined(CONFIG_FAILSAFE_UPGRADE)
#define FAILSAFE_PARTS 2
#else
#define FAILSAFE_PARTS 0
#endif
/* boot;nvram;kernel;rootfs;empty */
#define FLASH_PARTS_NUM	(6+MTD_PARTS+PLC_PARTS+FAILSAFE_PARTS)

static struct mtd_partition bcm947xx_flash_parts[FLASH_PARTS_NUM + 1] = { {0} };

static uint lookup_flash_rootfs_offset(struct mtd_info *mtd, int *trx_off, size_t size)
{
	struct romfs_super_block *romfsb;
	struct cramfs_super *cramfsb;
	struct squashfs_super_block *squashfsb;
	struct trx_header *trx;
	unsigned char buf[512];
	int off;
	size_t len;

	romfsb = (struct romfs_super_block *)buf;
	cramfsb = (struct cramfs_super *)buf;
	squashfsb = (void *)buf;
	trx = (struct trx_header *)buf;

	/* Look at every 64 KB boundary */
	for (off = 0; off < size; off += (64 * 1024)) {
		memset(buf, 0xe5, sizeof(buf));

		/*
		 * Read block 0 to test for romfs and cramfs superblock
		 */
		if (mtd_read(mtd, off, sizeof(buf), &len, buf) || len != sizeof(buf))
			continue;

		/* Try looking at TRX header for rootfs offset */
		if (le32_to_cpu(trx->magic) == TRX_MAGIC) {
			*trx_off = off;
			if (trx->offsets[1] == 0)
				continue;
			/*
			 * Read to test for romfs and cramfs superblock
			 */
			off += le32_to_cpu(trx->offsets[1]);
			memset(buf, 0xe5, sizeof(buf));
			if (mtd_read(mtd, off, sizeof(buf), &len, buf) || len != sizeof(buf))
				continue;
		}

		/* romfs is at block zero too */
		if (romfsb->word0 == ROMSB_WORD0 && romfsb->word1 == ROMSB_WORD1) {
			printk(KERN_NOTICE "%s: romfs filesystem found at block %d\n", mtd->name, off / mtd->erasesize);
			break;
		}

		/* so is cramfs */
		if (cramfsb->magic == CRAMFS_MAGIC) {
			printk(KERN_NOTICE "%s: cramfs filesystem found at block %d\n", mtd->name, off / mtd->erasesize);
			break;
		}

		if (squashfsb->s_magic == SQUASHFS_MAGIC) {
			rootfssize = le64_to_cpu(squashfsb->bytes_used);
			printk(KERN_NOTICE "%s: squash filesystem found at block %d\n", mtd->name, off / mtd->erasesize);
			break;
		}
	}

	return off;
}
extern int __init root_dev_setup(char *line);

struct mtd_partition *init_mtd_partitions(hndsflash_t * sfl_info, struct mtd_info *mtd, size_t size)
{
	int bootdev;
	int knldev;
	int nparts = 0;
	uint32 offset = 0;
	uint rfs_off = 0;
	uint vmlz_off, knl_size;
	uint32 top = 0;
	uint32 bootsz;
	uint32 maxsize = 0;
	int is_ex6200 = 0;
	int nobackup = 0;
#ifdef CONFIG_FAILSAFE_UPGRADE
	char *img_boot = nvram_get(BOOTPARTITION);
	char *imag_1st_offset = nvram_get(IMAGE_FIRST_OFFSET);
	char *imag_2nd_offset = nvram_get(IMAGE_SECOND_OFFSET);
	unsigned int image_first_offset = 0;
	unsigned int image_second_offset = 0;
	char dual_image_on = 0;

	/* The image_1st_size and image_2nd_size are necessary if the Flash does not have any
	 * image
	 */

	dual_image_on = (img_boot != NULL && imag_1st_offset != NULL && imag_2nd_offset != NULL);

	if (dual_image_on) {
		image_first_offset = simple_strtol(imag_1st_offset, NULL, 10);
		image_second_offset = simple_strtol(imag_2nd_offset, NULL, 10);
		printk("The first offset=%x, 2nd offset=%x\n", image_first_offset, image_second_offset);

	}
#endif				/* CONFIG_FAILSAFE_UPGRADE */

	if (nvram_match("boardnum", "24") && nvram_match("boardtype", "0x0646")
	    && nvram_match("boardrev", "0x1110")
	    && nvram_match("gpio7", "wps_button") && !nvram_match("gpio6", "wps_led")) {
		maxsize = 0x200000;
		size = maxsize;
		//nobackup = 1;
	}

	if (nvram_match("boardnum", "24") && nvram_match("boardtype", "0x0646")
	    && nvram_match("boardrev", "0x1110")
	    && nvram_match("gpio7", "wps_button") && nvram_match("gpio6", "wps_led")) {
		bootsz = 0x200000;
		//nobackup = 1;
	}

	if (nvram_match("boardnum", "24") && nvram_match("boardtype", "0x072F")
	    && nvram_match("boardrev", "0x1101")
	    && nvram_match("gpio7", "wps_button")) {
		bootsz = 0x200000;
	}

	if (nvram_match("boardnum", "24") && nvram_match("boardtype", "0x0646")
	    && nvram_match("boardrev", "0x1101")) {
		size = 0x200000;
		bootsz = 0x200000;
	}

	if (nvram_match("boardnum", "1234") && nvram_match("boardtype", "0x072F")) {
		nobackup = 1;
	}

	if (nvram_match("model", "RT-AC1200+")) {
		nobackup = 1;
	}
	if (nvram_space == 0x20000)
		nobackup = 1;

	if (nvram_match("boardnum","679") && nvram_match("boardtype", "0x0646") 
	    && (nvram_match("boardrev", "0x1110"))) {
		maxsize = 0x200000;
		if (nvram_match("board_id", "U12H269T00_NETGEAR")){
		    maxsize = 0x800000;
		    is_ex6200 = 1;
		}
		size = maxsize;
	}

	if (nvram_match("boardnum", "32") && nvram_match("boardtype", "0x0646")
	    && nvram_match("boardrev", "0x1601")) {
		maxsize = 0x200000;
		size = maxsize;
	}

	if (nvram_match("boardnum", "32") && nvram_match("boardtype", "0x0665")
	    && nvram_match("boardrev", "0x1301")) {
		maxsize = 0x200000;
		size = maxsize;
	}
	
	if (nvram_match("boardnum", "32") && nvram_match("boardtype", "0x072F")
	    && nvram_match("boardrev", "0x1301")) {
		maxsize = 0x200000;
		size = maxsize;
	}

	bootdev = soc_boot_dev((void *)sih);
	knldev = soc_knl_dev((void *)sih);

	if (bootdev == SOC_BOOTDEV_NANDFLASH) {
		/* Do not init MTD partitions on NOR flash when NAND boot */
		return NULL;
	}

	if (knldev != SOC_KNLDEV_NANDFLASH) {
		rfs_off = lookup_flash_rootfs_offset(mtd, &vmlz_off, size);
		root_dev_setup("1f02");

		/* Size pmon */
		bcm947xx_flash_parts[nparts].name = "boot";
		bcm947xx_flash_parts[nparts].size = vmlz_off;
		bcm947xx_flash_parts[nparts].offset = top;
//              bcm947xx_flash_parts[nparts].mask_flags =  MTD_WRITEABLE; /* forces on read only */
		nparts++;

		/* Setup kernel MTD partition */
		bcm947xx_flash_parts[nparts].name = "linux";
#ifdef CONFIG_FAILSAFE_UPGRADE
		if (dual_image_on) {
			bcm947xx_flash_parts[nparts].size = image_second_offset - image_first_offset;
		} else {
			bcm947xx_flash_parts[nparts].size = mtd->size - vmlz_off;

			/* Reserve for NVRAM */
			bcm947xx_flash_parts[nparts].size -= ROUNDUP(nvram_space, mtd->erasesize);
#ifdef PLC
			/* Reserve for PLC */
			bcm947xx_flash_parts[nparts].size -= ROUNDUP(0x1000, mtd->erasesize);
#endif
#ifdef BCMCONFMTD
			bcm947xx_flash_parts[nparts].size -= (mtd->erasesize * 4);
#endif
		}
#else

		bcm947xx_flash_parts[nparts].size = mtd->size - vmlz_off;
		if(is_ex6200)
			bcm947xx_flash_parts[nparts].size -= 0x10000;

#ifdef PLC
		/* Reserve for PLC */
		bcm947xx_flash_parts[nparts].size -= ROUNDUP(0x1000, mtd->erasesize);
#endif
		/* Reserve for NVRAM */
		bcm947xx_flash_parts[nparts].size -= ROUNDUP(nvram_space, mtd->erasesize);

#ifdef BCMCONFMTD
		bcm947xx_flash_parts[nparts].size -= (mtd->erasesize * 4);
#endif
#endif				/* CONFIG_FAILSAFE_UPGRADE */
		bcm947xx_flash_parts[nparts].offset = vmlz_off;
		knl_size = bcm947xx_flash_parts[nparts].size;
		offset = bcm947xx_flash_parts[nparts].offset + knl_size;
		nparts++;

		/* Setup rootfs MTD partition */
		bcm947xx_flash_parts[nparts].name = "rootfs";
		bcm947xx_flash_parts[nparts].offset = rfs_off;
		bcm947xx_flash_parts[nparts].size = rootfssize;
		size_t offs = bcm947xx_flash_parts[nparts].offset + bcm947xx_flash_parts[nparts].size;
		offs += (mtd->erasesize - 1);
		offs &= ~(mtd->erasesize - 1);
		offs -= bcm947xx_flash_parts[nparts].offset;
		bcm947xx_flash_parts[nparts].size = offs;

		bcm947xx_flash_parts[nparts].mask_flags = MTD_WRITEABLE;	/* forces on read only */
		nparts++;
#ifdef CONFIG_FAILSAFE_UPGRADE
		if (dual_image_on) {
			offset = image_second_offset;
			rfs_off = lookup_flash_rootfs_offset(mtd, &offset, size);
			vmlz_off = offset;
			/* Setup kernel2 MTD partition */
			bcm947xx_flash_parts[nparts].name = "linux2";
			bcm947xx_flash_parts[nparts].size = mtd->size - image_second_offset;
			/* Reserve for NVRAM */
			bcm947xx_flash_parts[nparts].size -= ROUNDUP(nvram_space, mtd->erasesize);

#ifdef BCMCONFMTD
			bcm947xx_flash_parts[nparts].size -= (mtd->erasesize * 4);
#endif
#ifdef PLC
			/* Reserve for PLC */
			bcm947xx_flash_parts[nparts].size -= ROUNDUP(0x1000, mtd->erasesize);
#endif
			bcm947xx_flash_parts[nparts].offset = image_second_offset;
			knl_size = bcm947xx_flash_parts[nparts].size;
			offset = bcm947xx_flash_parts[nparts].offset + knl_size;
			nparts++;

			/* Setup rootfs MTD partition */
			bcm947xx_flash_parts[nparts].name = "rootfs2";
			bcm947xx_flash_parts[nparts].size = knl_size - (rfs_off - image_second_offset);
			bcm947xx_flash_parts[nparts].offset = rfs_off;
			/* forces on read only */
			bcm947xx_flash_parts[nparts].mask_flags = MTD_WRITEABLE;
			nparts++;
		}
#endif				/* CONFIG_FAILSAFE_UPGRADE */

	} else {
		if (nobackup)
			root_dev_setup("1f03");
		else
			root_dev_setup("1f04");
		if (!bootsz)
			bootsz = boot_partition_size(sfl_info->base);
		printk("Boot partition size = %d(0x%x)\n", bootsz, bootsz);
		/* Size pmon */
		if (maxsize)
			bootsz = maxsize;
		bcm947xx_flash_parts[nparts].name = "boot";
		bcm947xx_flash_parts[nparts].size = bootsz;
		bcm947xx_flash_parts[nparts].offset = top;
//              bcm947xx_flash_parts[nparts].mask_flags = MTD_WRITEABLE; /* forces on read only */
		offset = bcm947xx_flash_parts[nparts].size;
		nparts++;
	}

#ifdef BCMCONFMTD
	/* Setup CONF MTD partition */
	bcm947xx_flash_parts[nparts].name = "confmtd";
	bcm947xx_flash_parts[nparts].size = mtd->erasesize * 4;
	bcm947xx_flash_parts[nparts].offset = offset;
	offset = bcm947xx_flash_parts[nparts].offset + bcm947xx_flash_parts[nparts].size;
	nparts++;
#endif				/* BCMCONFMTD */

#ifdef PLC
	/* Setup plc MTD partition */
	bcm947xx_flash_parts[nparts].name = "plc";
	bcm947xx_flash_parts[nparts].size = ROUNDUP(0x1000, mtd->erasesize);
	bcm947xx_flash_parts[nparts].offset = size - (ROUNDUP(nvram_space, mtd->erasesize) + ROUNDUP(0x1000, mtd->erasesize));
	nparts++;
#endif
	if (rootfssize) {
		bcm947xx_flash_parts[nparts].name = "ddwrt";
		bcm947xx_flash_parts[nparts].offset = bcm947xx_flash_parts[2].offset + bcm947xx_flash_parts[2].size;
		bcm947xx_flash_parts[nparts].offset += (mtd->erasesize - 1);
		bcm947xx_flash_parts[nparts].offset &= ~(mtd->erasesize - 1);
		bcm947xx_flash_parts[nparts].size = (size - bcm947xx_flash_parts[nparts].offset) - ROUNDUP(nvram_space, mtd->erasesize);
		nparts++;
	}
	
	if(is_ex6200){
		bcm947xx_flash_parts[nparts].name = "board_data";
		bcm947xx_flash_parts[nparts].size = ROUNDUP(nvram_space, mtd->erasesize);
		bcm947xx_flash_parts[nparts].offset = (size - 0x10000) - bcm947xx_flash_parts[nparts].size;
		nparts++;
	}
	
	/* Setup nvram MTD partition */
	bcm947xx_flash_parts[nparts].name = "nvram_cfe";
	bcm947xx_flash_parts[nparts].size = ROUNDUP(nvram_space, mtd->erasesize);
	if (maxsize)
		bcm947xx_flash_parts[nparts].offset = (size - 0x10000) - bcm947xx_flash_parts[nparts].size;
	else
		bcm947xx_flash_parts[nparts].offset = size - bcm947xx_flash_parts[nparts].size;
	if(!is_ex6200 && !nobackup)//skip on ex6200
		nparts++;
	

	bcm947xx_flash_parts[nparts].name = "nvram";
	bcm947xx_flash_parts[nparts].size = ROUNDUP(nvram_space, mtd->erasesize);
	if (maxsize)
		bcm947xx_flash_parts[nparts].offset = (size - 0x10000) - bcm947xx_flash_parts[nparts].size;
		if(is_ex6200 || nobackup)
			bcm947xx_flash_parts[nparts].offset = size + 0x10000 - bcm947xx_flash_parts[nparts].size;
	else
		bcm947xx_flash_parts[nparts].offset = size - bcm947xx_flash_parts[nparts].size;
	bcm947xx_flash_parts[nparts].offset-=bcm947xx_flash_parts[nparts].size;
	nparts++;

	return bcm947xx_flash_parts;
}

EXPORT_SYMBOL(init_mtd_partitions);

#endif				/* CONFIG_MTD_PARTITIONS */

#ifdef CONFIG_MTD_NFLASH
#define NFLASH_PARTS_NUM 7
static struct mtd_partition bcm947xx_nflash_parts[NFLASH_PARTS_NUM] = { {0} };

static uint lookup_nflash_rootfs_offset(hndnand_t * nfl, struct mtd_info *mtd, int offset, size_t size)
{
	struct romfs_super_block *romfsb;
	struct cramfs_super *cramfsb;
	struct squashfs_super_block *squashfsb;
	struct squashfs_super_block *squashfsb2;
	struct trx_header *trx;
	unsigned char *buf;
	uint blocksize, pagesize, mask, blk_offset, off, shift = 0;
	uint rbsize;
	int ret;

	pagesize = nfl->pagesize;
	buf = (unsigned char *)kmalloc(pagesize, GFP_KERNEL);
	if (!buf) {
		printk("lookup_nflash_rootfs_offset: kmalloc fail\n");
		return 0;
	}

	romfsb = (struct romfs_super_block *)buf;
	cramfsb = (struct cramfs_super *)buf;
	squashfsb = (void *)buf;
	squashfsb2 = (void *)&buf[0x60];
	trx = (struct trx_header *)buf;

	/* Look at every block boundary till 16MB; higher space is reserved for application data. */

	rbsize = blocksize = mtd->erasesize;	//65536;

	if (nvram_match("boardnum", "24") && nvram_match("boardtype", "0x0646")
	    && nvram_match("boardrev", "0x1110")
	    && nvram_match("gpio7", "wps_button")) {
		printk(KERN_INFO "DIR-686L Hack for detecting filesystems\n");
		blocksize = 65536;
	}

	if (nvram_match("boardnum", "24") && nvram_match("boardtype", "0x0646")
	    && nvram_match("boardrev", "0x1100")
	    && nvram_match("gpio8","wps_button")) {
		printk(KERN_INFO "DIR-660L Hack for detecting filesystems\n");
		blocksize = 65536;
	}

	if (nvram_match("boardnum", "24") && nvram_match("boardtype", "0x072F")
	    && nvram_match("boardrev", "0x1101")
	    && nvram_match("gpio7", "wps_button")) {
		printk(KERN_INFO "DIR-890L Hack for detecting filesystems\n");
		blocksize = 65536;
	}

	if (nvram_match("boardnum", "N/A") && nvram_match("boardtype", "0x072F")
	    && nvram_match("boardrev", "0x1101")
	    && nvram_match("gpio7", "wps_button")) {
		printk(KERN_INFO "DIR-885/895L Hack for detecting filesystems\n");
		blocksize = 65536;
	}

	if (nvram_match("boardnum", "24") && nvram_match("boardtype", "0x0646")
	    && nvram_match("boardrev", "0x1101")) {
		printk(KERN_INFO "DIR-868LC Hack for detecting filesystems\n");
		blocksize = 65536;
	}


	printk("lookup_nflash_rootfs_offset: offset = 0x%x size = 0x%x, 0x%x\n", offset, size, blocksize);
	for (off = offset; off < offset + size; off += blocksize) {
		mask = rbsize - 1;
		blk_offset = off & ~mask;
		if (hndnand_checkbadb(nfl, blk_offset) != 0)
			continue;
		memset(buf, 0xe5, pagesize);
		if ((ret = hndnand_read(nfl, off, pagesize, buf)) != pagesize) {
			printk(KERN_NOTICE "%s: nflash_read return %d\n", mtd->name, ret);
			continue;
		}

		/* Try looking at TRX header for rootfs offset */
		if (le32_to_cpu(trx->magic) == TRX_MAGIC) {
			mask = pagesize - 1;
			off = offset + (le32_to_cpu(trx->offsets[1]) & ~mask) - blocksize;
			shift = (le32_to_cpu(trx->offsets[1]) & mask);
			romfsb = (struct romfs_super_block *)((unsigned char *)romfsb + shift);
			cramfsb = (struct cramfs_super *)((unsigned char *)cramfsb + shift);
			squashfsb = (struct squashfs_super_block *)
			    ((unsigned char *)squashfsb + shift);
			squashfsb2 = NULL;
			printk(KERN_INFO "found TRX Header on nflash!\n");
			continue;
		}

		/* romfs is at block zero too */
		if (romfsb->word0 == ROMSB_WORD0 && romfsb->word1 == ROMSB_WORD1) {
			printk(KERN_NOTICE "%s: romfs filesystem found at block %d\n", mtd->name, off / blocksize);
			break;
		}

		/* so is cramfs */
		if (cramfsb->magic == CRAMFS_MAGIC) {
			printk(KERN_NOTICE "%s: cramfs filesystem found at block %d\n", mtd->name, off / blocksize);
			break;
		}

		if (squashfsb->s_magic == SQUASHFS_MAGIC) {
			rootfssize = le64_to_cpu(squashfsb->bytes_used);
			printk(KERN_NOTICE "%s: squash filesystem with lzma found at block %d\n", mtd->name, off / blocksize);
			break;
		}
		if (squashfsb2 && squashfsb2->s_magic == SQUASHFS_MAGIC) {
			rootfssize = le64_to_cpu(squashfsb2->bytes_used);
			off += 0x60;
			printk(KERN_NOTICE "%s: squash filesystem with lzma found at block %d\n", mtd->name, off / blocksize);
			break;
		}

	}

	if (buf)
		kfree(buf);

	return shift + off;
}

struct mtd_partition *init_nflash_mtd_partitions(hndnand_t * nfl, struct mtd_info *mtd, size_t size)
{
	int bootdev;
	int knldev;
	int nparts = 0;
	uint32 offset = 0;
	uint shift = 0;
	uint32 top = 0;
	uint32 bootsz;
	uint32 nvsz = 0;
	uint32 bootossz = NFL_BOOT_OS_SIZE;
	int isbufdual = 0;
	int isbufvxdual = 0;
	int istew = 0;
	uint boardnum = bcm_strtoul(nvram_safe_get("boardnum"), NULL, 0);
	if ((!strncmp(nvram_safe_get("boardnum"),"2013",4) || !strncmp(nvram_safe_get("boardnum"),"2014",4)) && nvram_match("boardtype", "0x0646")
	    && nvram_match("boardrev", "0x1110")) {
		printk(KERN_EMERG "Buffalo WZR-900DHP dualboot\n");
		isbufdual = 1;
		bootossz = 0x4000000;
	}

	if (nvram_match("model","RT-AC87U")) {
		printk(KERN_EMERG "Asus AC87U\n");
		bootossz = 0x4000000;
	}

	if (nvram_match("boardnum", "1234") && nvram_match("boardtype", "0x072F")) {
		bootossz = 0x2000000;
		istew = 1;
	}

	if (boardnum == 00 && nvram_match("boardtype", "0x0665")
	    && nvram_match("boardrev", "0x1103")
	    && nvram_match("melco_id", "RD_BB13049")) {
		printk(KERN_EMERG "Buffalo WXR-1900DHP dualboot\n");
		isbufvxdual = 1;
		bootossz = 0x6000000;
	}

	if (((boardnum == 1) || (nvram_get("boardnum") == NULL)) && nvram_match("boardtype","0xD646") && nvram_match("boardrev","0x1100")) {
		bootossz = 0x4000000;	
		nvsz = 0x100000;
	}

	if (((boardnum == 1) || (nvram_get("boardnum") == NULL)) && nvram_match("boardtype","0xF646") && nvram_match("boardrev","0x1100")) {
		bootossz = 0x4000000;	
		nvsz = 0x100000;
	}

//	if (boardnum == 20140309 && nvram_match("boardtype","0xE646") && nvram_match("boardrev","0x1200")) {
//		bootossz = 0x1f00000;	
//	}

	if (((boardnum == 1) || (nvram_get("boardnum") == NULL)) && nvram_match("boardtype", "0x0646") && nvram_match("boardrev", "0x1100") && !strncmp(nvram_safe_get("modelNumber"),"EA6400",6)) {
		bootossz = 0x3c00000;	
		nvsz = 0x100000;
	}

	if (((boardnum == 1) || (nvram_get("boardnum") == NULL)) && nvram_match("boardtype", "0x0646") && nvram_match("boardrev", "0x1100") && !strncmp(nvram_safe_get("modelNumber"),"EA6300",6)) {
		bootossz = 0x3c00000;	
		nvsz = 0x100000;
	}

#ifdef CONFIG_FAILSAFE_UPGRADE
	char *img_boot = nvram_get(BOOTPARTITION);
	char *imag_1st_offset = nvram_get(IMAGE_FIRST_OFFSET);
	char *imag_2nd_offset = nvram_get(IMAGE_SECOND_OFFSET);
	unsigned int image_first_offset = 0;
	unsigned int image_second_offset = 0;
	char dual_image_on = 0;

	/* The image_1st_size and image_2nd_size are necessary if the Flash does not have any
	 * image
	 */
	if (isbufdual) {
	if (imag_1st_offset == NULL)
		imag_1st_offset = "2097152";
	if (imag_2nd_offset == NULL)
		imag_2nd_offset = "34603008";
	}
	if (isbufvxdual) {
	if (!img_boot)
	    img_boot="0";
	if (imag_1st_offset == NULL)
		imag_1st_offset = "0";
	if (imag_2nd_offset == NULL)
		imag_2nd_offset = "50331648";
	}
	if (istew)
		imag_2nd_offset = "16777216";
	dual_image_on = (img_boot != NULL && imag_1st_offset != NULL && imag_2nd_offset != NULL);

	if (dual_image_on) {
		image_first_offset = simple_strtol(imag_1st_offset, NULL, 10);
		image_second_offset = simple_strtol(imag_2nd_offset, NULL, 10);
		printk("The first offset=%x, 2nd offset=%x\n", image_first_offset, image_second_offset);

	}
#endif				/* CONFIG_FAILSAFE_UPGRADE */

	bootdev = soc_boot_dev((void *)sih);
	knldev = soc_knl_dev((void *)sih);

	if (bootdev == SOC_BOOTDEV_NANDFLASH) {
		bootsz = boot_partition_size(nfl->base);
		if (bootsz > mtd->erasesize) {
			/* Prepare double space in case of bad blocks */
			bootsz = (bootsz << 1);
		} else {
			/* CFE occupies at least one block */
			bootsz = mtd->erasesize;
		}
		printk("Boot partition size = %d(0x%x)\n", bootsz, bootsz);

		/* Size pmon */
		bcm947xx_nflash_parts[nparts].name = "boot";
		bcm947xx_nflash_parts[nparts].size = bootsz;
		bcm947xx_nflash_parts[nparts].offset = top;
//              bcm947xx_nflash_parts[nparts].mask_flags = MTD_WRITEABLE; /* forces on read only */
		offset = bcm947xx_nflash_parts[nparts].size;
		nparts++;
		if (isbufdual)
			offset += 128 * 1024;
		/* Setup NVRAM MTD partition */
		bcm947xx_nflash_parts[nparts].name = "nvram";
		if (nvsz)
		bcm947xx_nflash_parts[nparts].size = nvsz;
		else 
		bcm947xx_nflash_parts[nparts].size = NFL_BOOT_SIZE - offset;
		bcm947xx_nflash_parts[nparts].offset = offset;

		offset = NFL_BOOT_SIZE;
		nparts++;
	}

	if (knldev == SOC_KNLDEV_NANDFLASH) {
		/* Setup kernel MTD partition */
#ifdef CONFIG_FAILSAFE_UPGRADE
		if (dual_image_on) {
			if (!strcmp(img_boot, "1"))
				bcm947xx_nflash_parts[nparts].name = "linux2";
			else
				bcm947xx_nflash_parts[nparts].name = "linux";
			bcm947xx_nflash_parts[nparts].size = image_second_offset - image_first_offset;
		} else
#endif
		{
			bcm947xx_nflash_parts[nparts].name = "linux";
			bcm947xx_nflash_parts[nparts].size = nparts ? (bootossz - NFL_BOOT_SIZE) : bootossz;
		}
		/* fix linux offset for this unit */
		if (nvram_match("boardnum","679") && nvram_match("boardtype", "0x0646") && (nvram_match("boardrev", "0x1110"))) {
			offset += 0x180000;
			bcm947xx_nflash_parts[nparts].size -= 0x180000;
		}
		
		if (nvram_match("boardnum", "32") && nvram_match("boardtype", "0x0665") && nvram_match("boardrev", "0x1301")) {
			bcm947xx_nflash_parts[nparts].size += 0x200000;
		}
		
		if (nvram_match("boardnum", "32") && nvram_match("boardtype", "0x0665") && nvram_match("boardrev", "0x1101")) {
			bcm947xx_nflash_parts[nparts].size += 0x600000;
		}
		
		bcm947xx_nflash_parts[nparts].offset = offset;

		shift = lookup_nflash_rootfs_offset(nfl, mtd, offset, bcm947xx_nflash_parts[nparts].size);
		


#ifdef CONFIG_FAILSAFE_UPGRADE
		if (dual_image_on)
			offset = image_second_offset;
		else
#endif
			offset = bootossz;
		nparts++;

		/* Setup rootfs MTD partition */
#ifdef CONFIG_FAILSAFE_UPGRADE
		if (dual_image_on) {
			if (!strcmp(img_boot, "1"))
				bcm947xx_nflash_parts[nparts].name = "rootfs2";
			else
				bcm947xx_nflash_parts[nparts].name = "rootfs";
			bcm947xx_nflash_parts[nparts].size = image_second_offset - shift;
		} else
#endif
		{
			bcm947xx_nflash_parts[nparts].name = "rootfs";
			bcm947xx_nflash_parts[nparts].size = bootossz - shift;
		}
		bcm947xx_nflash_parts[nparts].offset = shift;
		bcm947xx_nflash_parts[nparts].mask_flags = MTD_WRITEABLE;
		
		if (nvram_match("boardnum", "32") && nvram_match("boardtype", "0x0665") && nvram_match("boardrev", "0x1301")) {			
			bcm947xx_nflash_parts[nparts].size += 0x200000;
		}
		
		if (nvram_match("boardnum", "32") && nvram_match("boardtype", "0x0665") && nvram_match("boardrev", "0x1101")) {
			bcm947xx_nflash_parts[nparts].size += 0x600000;
		}

		nparts++;

		if (nvram_match("boardnum", "32") && nvram_match("boardtype", "0x072F") && nvram_match("boardrev", "0x1101")) {
			
			bcm947xx_nflash_parts[nparts].name = "board_data";
			bcm947xx_nflash_parts[nparts].size = 0x80000;
			bcm947xx_nflash_parts[nparts].offset = 0x7400000;
			nparts++;
		}
		
		if (nvram_match("boardnum", "32") && nvram_match("boardtype", "0x0665") && nvram_match("boardrev", "0x1301")) {
			
			bcm947xx_nflash_parts[nparts].name = "board_data";
			bcm947xx_nflash_parts[nparts].size = 0x40000;
			bcm947xx_nflash_parts[nparts].offset = 0x2200000;
			nparts++;
		}
		
		if (nvram_match("boardnum", "32") && nvram_match("boardtype", "0x0665") && nvram_match("boardrev", "0x1101")) {
			
			bcm947xx_nflash_parts[nparts].name = "board_data";
			bcm947xx_nflash_parts[nparts].size = 0x80000;
			bcm947xx_nflash_parts[nparts].offset = 0x2600000;
			nparts++;
		}
		
		if (nvram_match("boardnum","679") && nvram_match("boardtype", "0x0646") && nvram_match("boardrev", "0x1110") ) {
			bcm947xx_nflash_parts[nparts].name = "board_data";
			bcm947xx_nflash_parts[nparts].size = 0x20000;
			bcm947xx_nflash_parts[nparts].offset = 0x200000;
			nparts++;
		}

#ifdef CONFIG_FAILSAFE_UPGRADE
		/* Setup 2nd kernel MTD partition */
		if (dual_image_on) {
			if (!strcmp(img_boot, "1"))
				bcm947xx_nflash_parts[nparts].name = "linux";
			else
				bcm947xx_nflash_parts[nparts].name = "linux2";

			bcm947xx_nflash_parts[nparts].size = bootossz - image_second_offset;

			bcm947xx_nflash_parts[nparts].offset = image_second_offset;
			shift = lookup_nflash_rootfs_offset(nfl, mtd, image_second_offset, bcm947xx_nflash_parts[nparts].size);
			nparts++;
			/* Setup rootfs MTD partition */
			if (!strcmp(img_boot, "1"))
				bcm947xx_nflash_parts[nparts].name = "rootfs";
			else
				bcm947xx_nflash_parts[nparts].name = "rootfs2";

			bcm947xx_nflash_parts[nparts].size = bootossz - shift;

			bcm947xx_nflash_parts[nparts].offset = shift;
			bcm947xx_nflash_parts[nparts].mask_flags = MTD_WRITEABLE;
			if (!strcmp(img_boot, "1")) {
				// now lets swap the crap
				struct mtd_partition temp;
				// swap rootfs
				memcpy(&temp, &bcm947xx_nflash_parts[nparts], sizeof(temp));
				memcpy(&bcm947xx_nflash_parts[nparts], &bcm947xx_nflash_parts[nparts - 2], sizeof(temp));
				memcpy(&bcm947xx_nflash_parts[nparts - 2], &temp, sizeof(temp));
				// swap kernel enty
				memcpy(&temp, &bcm947xx_nflash_parts[nparts - 1], sizeof(temp));
				memcpy(&bcm947xx_nflash_parts[nparts - 1], &bcm947xx_nflash_parts[nparts - 3], sizeof(temp));
				memcpy(&bcm947xx_nflash_parts[nparts - 3], &temp, sizeof(temp));

			}
			nparts++;

		}
#endif				/* CONFIG_FAILSAFE_UPGRADE */


	}
#if 0
	if (rootfssize) {
		bcm947xx_nflash_parts[nparts].name = "ddwrt";
		bcm947xx_nflash_parts[nparts].offset = bcm947xx_nflash_parts[nparts - 1].offset + bcm947xx_nflash_parts[nparts - 1].size;
		bcm947xx_nflash_parts[nparts].offset += (mtd->erasesize - 1);
		bcm947xx_nflash_parts[nparts].offset &= ~(mtd->erasesize - 1);
		bcm947xx_nflash_parts[nparts].size = (size - bcm947xx_nflash_parts[nparts].offset) - ROUNDUP(NVRAM_SPACE, mtd->erasesize);
		nparts++;
	}
#endif
	return bcm947xx_nflash_parts;
}

EXPORT_SYMBOL(init_nflash_mtd_partitions);
#endif				/* CONFIG_MTD_NFLASH */
