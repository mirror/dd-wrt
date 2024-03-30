/*
 * This file contains glue for Atheros ar9100 pf flash interface
 * Primitives are ar9100_pf_*
 * mtd flash implements are ar9100_flash_*
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include "../mtdcore.h"
#include <asm/delay.h>
#include <asm/io.h>
#include <linux/semaphore.h>

#include "ar7100.h"
#include "ar9100_pflash.h"
#include <linux/magic.h>

/* this is passed in as a boot parameter by bootloader */
extern int __ath_flash_size;
extern void ar9100_write(uint32_t, CFG_FLASH_WORD_SIZE);
extern CFG_FLASH_WORD_SIZE ar9100_read(uint32_t offset);

/*
 * bank geometry
 */
typedef struct ar9100_flash_geom {
    uint16_t vendor_id;
    uint16_t device_id;
    char *name;
    uint32_t sector_size;
    uint32_t size;
} ar9100_flash_geom_t;


/*
 * statics
 */
static int ar9100_pflash_erase(ar9100_flash_geom_t *geom,int s_first, int s_last);
static int ar9100_pflash_write_word(ar9100_flash_geom_t *geom,unsigned long dest, unsigned long data);
static int ar9100_pflash_write_buff(ar9100_flash_geom_t *geom,u_char * src, unsigned long addr, unsigned long cnt);
static int ar9100_flash_probe(void);

static const char *part_probes[] __initdata =
    { "cmdlinepart", "RedBoot", NULL };

#define down mutex_lock
#define up mutex_unlock
#define init_MUTEX mutex_init
#define DECLARE_MUTEX(a) struct mutex a

static DECLARE_MUTEX(ar9100_flash_sem);
static DECLARE_MUTEX(ar7100_flash_sem);

/* GLOBAL FUNCTIONS */
void ar9100_pflash_down(void)
{
    down(&ar9100_flash_sem);
}

void ar9100_pflash_up(void)
{
    up(&ar9100_flash_sem);
}

/* For spi flash controller */
void ar7100_flash_spi_down(void)
{
    down(&ar7100_flash_sem);
}

void ar7100_flash_spi_up(void)
{
    up(&ar7100_flash_sem);
}

EXPORT_SYMBOL(ar7100_flash_spi_down);
EXPORT_SYMBOL(ar7100_flash_spi_up);

ar9100_flash_geom_t flash_geom_tbl[] = {
    {0x00bf, 0x2780, "SST-39VF400", 0x01000, 0x080000},	/* 512KB */
    {0x00bf, 0x2782, "SST-39VF160", 0x01000, 0x200000},	/* 2MB */
    {0x00bf, 0x236b, "SST-39VF6401", 0x01000, 0x800000},
    {0x00bf, 0x236a, "SST-39VF6402", 0x01000, 0x800000},
    {0x00bf, 0x236d, "SST-39VF6402", 0x01000, 0x800000},
    {0x0001, 0x227e, "AMD-SPANSION", 0x02000, 0x2000000},  /* 32 MB  */
    {0x00c2, 0x227e, "AMD-SPANSION", 0x02000, 0x2000000},  /* 32 MB  */
    {0xffff, 0xffff, NULL, 0, 0}	/* end list */
};

static int ar9100_flash_probe(void)
{
    uint16_t venid, devid, i;

    /* issue JEDEC query */

    ar9100_write(CFG_FLASH_ADDR0, FLASH_Setup_Code1);
    ar9100_write(CFG_FLASH_ADDR1, FLASH_Setup_Code2);
    ar9100_write(CFG_FLASH_ADDR0, FLASH_Jedec_Query);

    udelay(10000);

    venid = ar9100_read(0);
    devid = ar9100_read(1);

    /* issue software exit */
    ar9100_write(CFG_FLASH_ADDR0, FLASH_Setup_Code1);
    ar9100_write(CFG_FLASH_ADDR1, FLASH_Setup_Code2);
    ar9100_write(CFG_FLASH_ADDR0, FLASH_Soft_Exit);

    udelay(10000);

    for (i = 0; flash_geom_tbl[i].name != NULL; i++) {
        if ((venid == flash_geom_tbl[i].vendor_id) &&
            (devid == flash_geom_tbl[i].device_id)) {
            break;
        }
    }
if (flash_geom_tbl[i].name==NULL)
    {
    printk(KERN_INFO "use default mapping for vendor %X, dev %X\n",venid,devid);
    i=5; 
    }
    printk("FLASH ID: %s ", flash_geom_tbl[i].name);

    if (flash_geom_tbl[i].size >= 0x100000)
        printk("SIZE: (%d MB)\n", flash_geom_tbl[i].size >> 20);
    else
        printk("SIZE: (%d KB)\n", flash_geom_tbl[i].size >> 10);

    return i;
}

static int ar9100_flash_erase(struct mtd_info *mtd, struct erase_info *instr)
{
    int nsect, s_curr, s_last;
	uint64_t  res;
 
    if (instr->addr + instr->len > mtd->size) return (-EINVAL);

    ar9100_pflash_down();

	res = instr->len;
	do_div(res, mtd->erasesize);
	nsect = res;
	if (((uint32_t)instr->len) % mtd->erasesize)
		nsect++;

	res = instr->addr;
	do_div(res,mtd->erasesize);
	s_curr = res;
    s_last  = s_curr + nsect;

    ar9100_pflash_erase((ar9100_flash_geom_t *)mtd->priv,s_curr, s_last);

    ar9100_pflash_up();

    if (instr->callback) {
        instr->state = MTD_ERASE_DONE;
        instr->callback(instr);
    }

    return 0;
}

static int
ar9100_flash_read(struct mtd_info *mtd, loff_t from, size_t len,
	  size_t * retlen, u_char * buf)
{
    uint32_t addr = from | AR9100_PFLASH_CTRLR;

    if (!len)
        return (0);
    
    if (from + len > mtd->size)
        return (-EINVAL);

    ar9100_pflash_down();
    
    memcpy(buf, (uint8_t *) (addr), len);
    *retlen = len;

    ar9100_pflash_up();

    return 0;
}

static int
ar9100_flash_write(struct mtd_info *mtd, loff_t to, size_t len,
	   size_t * retlen, const u_char * buf)
{

//printk(KERN_INFO "%s", __func__);

    ar9100_pflash_down();

    if (mtd->size < to + len)
        return ENOSPC;

    to += AR9100_PFLASH_CTRLR;
    ar9100_pflash_write_buff((ar9100_flash_geom_t *)mtd->priv, (u_char *) buf, to, len);

    ar9100_pflash_up();

    *retlen = len;

    return 0;
}

static struct mtd_partition dir_parts[] = {
        { name: "RedBoot", offset: 0, size: 0x60000, },//, mask_flags: MTD_WRITEABLE, },
        { name: "linux", offset: 0x60000, size: 0x390000, },
        { name: "rootfs", offset: 0x0, size: 0x2b0000,}, //must be detected
        { name: "ddwrt", offset: 0x0, size: 0x2b0000,}, //must be detected
        { name: "nvram", offset: 0x3d0000, size: 0x10000, },
        { name: "board_config", offset: 0x3f0000, size: 0x10000, },
        { name: "fullflash", offset: 0x3f0000, size: 0x10000, },
        { name: NULL, },
};

/*
 * sets up flash_info and returns size of FLASH (bytes)
 */
static int
__init ar9100_flash_init(void)
{
    int np;
    ar9100_flash_geom_t *geom;
    struct mtd_info *mtd;
    struct mtd_partition *mtd_parts;
    uint8_t index;
	char *buf;
	unsigned char *p;
	int offset=0;
	struct squashfs_super_block *sb;
	size_t rootsize;
	size_t len;

    init_MUTEX(&ar9100_flash_sem);

    index = ar9100_flash_probe();
    geom = &flash_geom_tbl[index];

    /* set flash size to value from bootloader if it passed valid value */
    /* otherwise use the default 4MB.                                   */
    if (__ath_flash_size >= 4 && __ath_flash_size <= 32)
        geom->size = __ath_flash_size * 1024 * 1024;

    mtd = kmalloc(sizeof(struct mtd_info), GFP_KERNEL);
    if (!mtd) {
        printk("Cant allocate mtd stuff\n");
        return -1;
    }
    memset(mtd, 0, sizeof(struct mtd_info));

    mtd->name = AR9100_FLASH_NAME;
    mtd->type = MTD_NORFLASH;
    mtd->flags = (MTD_CAP_NORFLASH | MTD_WRITEABLE);
    mtd->size = geom->size;
    mtd->erasesize = (geom->sector_size * 16);	/* Erase block size */
    mtd->numeraseregions = 0;
    mtd->eraseregions = NULL;
    mtd->owner = THIS_MODULE;
    mtd->_erase = ar9100_flash_erase;
    mtd->_read = ar9100_flash_read;
    mtd->_write = ar9100_flash_write;
    mtd->priv = (void *)(&flash_geom_tbl[index]);

	printk(KERN_INFO "scanning for root partition\n");
	
	offset = 0;
	buf = 0xbe000000;
	while((offset+mtd->erasesize)<mtd->size)
	    {
	    //printk(KERN_INFO "[0x%08X] = [0x%08X]!=[0x%08X]\n",offset,*((unsigned int *) buf),SQUASHFS_MAGIC);
	    if (*((__u32 *) buf) == SQUASHFS_MAGIC_SWAP) 
		{
		printk(KERN_INFO "\nfound squashfs at %X\n",offset);
		sb = (struct squashfs_super_block *) buf;
		dir_parts[2].offset = offset;
		dir_parts[2].size = le64_to_cpu(sb->bytes_used);
		len = dir_parts[2].offset + dir_parts[2].size;
		len +=  (mtd->erasesize - 1);
		len &= ~(mtd->erasesize - 1);
		dir_parts[2].size = (len&0x1ffffff) - dir_parts[2].offset;
		dir_parts[3].offset = dir_parts[2].offset + dir_parts[2].size; 
		dir_parts[5].offset = mtd->size-mtd->erasesize; // board config
		dir_parts[5].size = mtd->erasesize;
		dir_parts[4].offset = dir_parts[5].offset-mtd->erasesize; //nvram
		dir_parts[4].size = mtd->erasesize;
		dir_parts[3].size = dir_parts[4].offset - dir_parts[3].offset;
		rootsize = dir_parts[4].offset-offset; //size of rootfs aligned to nvram offset

		dir_parts[1].offset=dir_parts[0].size;
		dir_parts[1].size=(dir_parts[2].offset-dir_parts[0].size)+rootsize;
		break;
		}
		//now scan for linux offset
	    offset+=4096;
	    buf+=4096;
	    }
	def:;
	dir_parts[6].offset=0; // linux + nvram = phy size
	dir_parts[6].size=mtd->size; // linux + nvram = phy size
	add_mtd_partitions(mtd, dir_parts, 7);
    return 0;
}


static void
__exit ar9100_flash_exit(void)
{
    /*
     * nothing to do
     */
}

/*
 * Primitives to implement flash operations
 */

static int
ar9100_pflash_write_buff(ar9100_flash_geom_t *geom, u_char * src, unsigned long addr, unsigned long cnt)
{
    unsigned long cp, wp, data;
    int i, l, rc;
//printk(KERN_INFO "%s", __func__);

    wp = (addr & ~3);   /* get lower word aligned address */

    /*
     * handle unaligned start bytes
     */
    if ((l = addr - wp) != 0) {
        data = 0;
        for (i = 0, cp = wp; i < l; ++i, ++cp) {
            data = (data << 8) | (*(unsigned char *) cp);
        }
        for (; i < 4 && cnt > 0; ++i) {
            data = (data << 8) | *src++;
            --cnt;
            ++cp;
        }
        for (; cnt == 0 && i < 4; ++i, ++cp) {
            data = (data << 8) | (*(unsigned char *) cp);
        }

        if ((rc = ar9100_pflash_write_word(geom, wp, data)) != 0) {
            return (rc);
        }
        wp += 4;
    }
    /*
     * handle word aligned part
     *
     */
    while (cnt >= 4) {
        data = 0;
        for (i = 0; i < 4; ++i) {
            data = (data << 8) | *src++;
        }
        if ((rc = ar9100_pflash_write_word(geom, wp, data)) != 0) {
            return (rc);
        }
        wp += 4;
        cnt -= 4;
    }

    if (cnt == 0) {
        return (0);
    }

    /*
     * handle unaligned tail bytes
     */
    data = 0;
    for (i = 0, cp = wp; i < 4 && cnt > 0; ++i, ++cp) {
        data = (data << 8) | *src++;
        --cnt;
    }
    for (; i < 4; ++i, ++cp) {
        data = (data << 8) | (*(unsigned char *) cp);
    }
    return (ar9100_pflash_write_word(geom, wp, data));
}

static int ar9100_pflash_write_word(ar9100_flash_geom_t *geom, unsigned long dest, unsigned long data)
{
//printk(KERN_INFO "%s", __func__);


    volatile CFG_FLASH_WORD_SIZE *dest2 = (CFG_FLASH_WORD_SIZE *) dest;
    unsigned long data_addr;

    data_addr = (unsigned long) &data;
    CFG_FLASH_WORD_SIZE *data2 = (CFG_FLASH_WORD_SIZE *) data_addr;
    int i;

    /* Check if Flash is (sufficiently) erased */
    if ((*((unsigned long *) dest) & data) != data) {
        return (2);
    }

    for (i = 0; i < 4 / sizeof(CFG_FLASH_WORD_SIZE); i++) {
        CFG_FLASH_WORD_SIZE state, prev_state;
        int timeout;

        ar9100_write(CFG_FLASH_ADDR0, FLASH_Setup_Code1);
        ar9100_write(CFG_FLASH_ADDR1, FLASH_Setup_Code2);
        ar9100_write(CFG_FLASH_ADDR0, FLASH_Program);
        dest2[i] = data2[i];

#if 1
            timeout = 10000000;
            while (timeout) {
                if (dest2[i] == data2[i]) {
                         break;
                }
                timeout--;
            }
#else
        /*  Wait for completion (bit 6 stops toggling) */
        	timeout = 5000000;
        	prev_state = (dest2[i] & FLASH_Busy);
        	while (timeout) {
            		state = (dest2[i] & FLASH_Busy);
            		if (prev_state == state) {
				break;
            		}
            	timeout--;
            	prev_state = state;
        	}
#endif
        	if (!timeout)
            		return -1;
     }

    return (0);
}

static int ar9100_pflash_erase(ar9100_flash_geom_t *geom, int s_first, int s_last)
{
//printk(KERN_INFO "%s", __func__);

    int i;
    int timeout;

    for (i = s_first; i < s_last; i++) {
        CFG_FLASH_WORD_SIZE state, prev_state,rd_data;

	 f_ptr addr_ptr = (f_ptr) (AR9100_PFLASH_CTRLR + (i * geom->sector_size * 16));
//	printk(KERN_INFO "%s %d", __func__,i);

        /* Program data [byte] - 6 step sequence */
        ar9100_write(CFG_FLASH_ADDR0, FLASH_Setup_Code1);
        ar9100_write(CFG_FLASH_ADDR1, FLASH_Setup_Code2);
        ar9100_write(CFG_FLASH_ADDR0, FLASH_Setup_Erase);
        ar9100_write(CFG_FLASH_ADDR0, FLASH_Setup_Code1);
        ar9100_write(CFG_FLASH_ADDR1, FLASH_Setup_Code2);

        *addr_ptr = FLASH_Block_Erase;
#if 1
        // Wait for erase completion.
        timeout = 10000000;
        while (timeout) {
        	state = *addr_ptr;
                if (FLASHWORD(0xffff) == state) {
                        break;
                }
                timeout--;
            }
#else
        /*  Wait for completion (bit 6 stops toggling) */
       		timeout = 5000000;
        	prev_state = (*addr_ptr & FLASH_Busy);
		while (timeout) {
                	rd_data = *addr_ptr;
                	state = rd_data & FLASH_Busy;
                	if ((prev_state == state) && (rd_data == FLASHWORD(0xffff))) {
                        	break;
			}
			timeout--;
                        prev_state = state;
                }
#endif
        if (!timeout) {
		printk("Erase operation failed\n");
        	return -1;
	}
     }
   	 return 0;
}

module_init(ar9100_flash_init);
module_exit(ar9100_flash_exit);
