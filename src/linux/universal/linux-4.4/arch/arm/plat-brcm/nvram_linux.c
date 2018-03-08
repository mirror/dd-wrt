/*
 * NVRAM variable manipulation (Linux kernel half)
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
 * $Id: nvram_linux.c,v 1.10 2010-09-17 04:51:19 $
 */

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
#include <linux/config.h>
#endif

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/bootmem.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/mtd/mtd.h>
#include <linux/vmalloc.h>

#include <typedefs.h>
#include <bcmendian.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmdefs.h>
#include <hndsoc.h>
#include <siutils.h>
#include <hndmips.h>
#include <hndsflash.h>
#ifdef CONFIG_MTD_NFLASH
#include <nflash.h>
#endif

int nvram_space = 0x10000;

/* Temp buffer to hold the nvram transfered romboot CFE */
char __initdata ram_nvram_buf[MAX_NVRAM_SPACE] __attribute__((aligned(PAGE_SIZE)));

/* In BSS to minimize text size and page aligned so it can be mmap()-ed */
static char nvram_buf[MAX_NVRAM_SPACE] __attribute__((aligned(PAGE_SIZE)));
static bool nvram_inram = FALSE;

#ifdef MODULE

#define early_nvram_get(name) nvram_get(name)

#else /* !MODULE */

static char *early_nvram_get(const char *name);

/* Global SB handle */
extern si_t *bcm947xx_sih;
extern spinlock_t bcm947xx_sih_lock;

/* Convenience */
#define sih bcm947xx_sih
#define sih_lock bcm947xx_sih_lock
#define KB * 1024
#define MB * 1024 * 1024

#ifndef	MAX_MTD_DEVICES
#define	MAX_MTD_DEVICES	32
#endif


void *MMALLOC(size_t size)
{
	void *ptr = kmalloc(size, GFP_ATOMIC);
	if (!ptr)
		ptr = vmalloc(size);
	return ptr;
}

void MMFREE(void *ptr)
{
	if (is_vmalloc_addr(ptr))
		vfree(ptr);
	else
		kfree(ptr);
}

static struct resource norflash_region = {
	.name = "norflash",
	.start = 0x1E000000,
	.end =  0x1FFFFFFF,
        .flags = IORESOURCE_MEM,
};

static int remap_cfe=0;
#ifdef CONFIG_MTD_NFLASH
static unsigned char nflash_nvh[MAX_NVRAM_SPACE];

static struct nvram_header *
BCMINITFN(nand_find_nvram)(hndnand_t *nfl, uint32 off)
{
	int blocksize = nfl->blocksize;
	unsigned char *buf = nflash_nvh;
	int rlen = sizeof(nflash_nvh);
	int len;

	for (; off < NFL_BOOT_SIZE; off += blocksize) {
		if (hndnand_checkbadb(nfl, off) != 0)
			continue;

		len = blocksize;
		if (len >= rlen)
			len = rlen;

		if (hndnand_read(nfl, off, len, buf) == 0)
			break;

		buf += len;
		rlen -= len;
		if (rlen == 0)
			return (struct nvram_header *)nflash_nvh;
	}

	return NULL;
}
#endif /* CONFIG_MTD_NFLASH */

/* Probe for NVRAM header */
static int
early_nvram_init(void)
{
	struct nvram_header *header;
	int i;
	u32 *src, *dst;
#ifdef CONFIG_MTD_NFLASH
	hndnand_t *nfl_info = NULL;
	uint32 blocksize;
#endif
	char *nvram_space_str;
	int bootdev;
	uint32 flash_base;
	uint32 lim = SI_FLASH_WINDOW;
	uint32 off;
	hndsflash_t *sfl_info;

	header = (struct nvram_header *)ram_nvram_buf;
	if (header->magic == NVRAM_MAGIC) {
		if (nvram_calc_crc(header) == (uint8)header->crc_ver_init) {
			nvram_inram = TRUE;
			goto found;
		}
	}
	bootdev = soc_boot_dev((void *)sih);
#ifdef CONFIG_MTD_NFLASH
	if (bootdev == SOC_BOOTDEV_NANDFLASH) {
		if ((nfl_info = hndnand_init(sih)) == NULL)
			return -1;
		flash_base = nfl_info->base;
		blocksize = nfl_info->blocksize;
		off = blocksize;
		for (; off < NFL_BOOT_SIZE; off += blocksize) {
			if (hndnand_checkbadb(nfl_info, off) != 0)
				continue;
			header = (struct nvram_header *)(flash_base + off);
			if (header->magic != NVRAM_MAGIC)
				continue;

			/* Read into the nand_nvram */
			if ((header = nand_find_nvram(nfl_info, off)) == NULL)
				continue;
			if (nvram_calc_crc(header) == (uint8)header->crc_ver_init)
			{
				printk(KERN_INFO "found nand nvram at %X\n",off);
				goto found;
			}
		}
	}
	else
#endif
	if (bootdev == SOC_BOOTDEV_SFLASH ||
	    bootdev == SOC_BOOTDEV_ROM) {
		/* Boot from SFLASH or ROM */
		if ((sfl_info = hndsflash_init(sih)) == NULL)
			return -1;

		lim = sfl_info->size;

		BUG_ON(request_resource(&iomem_resource, &norflash_region));
	
		flash_base = sfl_info->base;
	
		BUG_ON(IS_ERR_OR_NULL((void *)flash_base));
		
		off = FLASH_MIN;
		while (off <= lim) {
			/* Windowed flash access */
			header = (struct nvram_header *)(flash_base + off - (nvram_space*2));
			if (header->magic == NVRAM_MAGIC)
				if (nvram_calc_crc(header) == (uint8)header->crc_ver_init) {
					printk(KERN_INFO "found remapped nvram on sflash\n");
					remap_cfe=0;
					goto found;
				}

			header = (struct nvram_header *)(flash_base + off - nvram_space);
			if (header->magic == NVRAM_MAGIC)
				if (nvram_calc_crc(header) == (uint8)header->crc_ver_init) {
					printk(KERN_INFO "found cfe nvram on sflash\n");
					remap_cfe=1;
					goto found;
				}
			
			off += 0x10000;
		}
	}
	else {
		/* This is the case bootdev == SOC_BOOTDEV_PFLASH, not applied on NorthStar */
		ASSERT(0);
	}

	/* Try embedded NVRAM at 4 KB and 1 KB as last resorts */
	header = (struct nvram_header *)(flash_base + 4 KB);
	if (header->magic == NVRAM_MAGIC)
		if (nvram_calc_crc(header) == (uint8)header->crc_ver_init) {
			goto found;
		}

	header = (struct nvram_header *)(flash_base + 1 KB);
	if (header->magic == NVRAM_MAGIC)
		if (nvram_calc_crc(header) == (uint8)header->crc_ver_init) {
			goto found;
		}

	return -1;

found:
	src = (u32 *)header;
	dst = (u32 *)nvram_buf;
	for (i = 0; i < sizeof(struct nvram_header); i += 4)
		*dst++ = *src++;
	for (; i < header->len && i < MAX_NVRAM_SPACE; i += 4)
		*dst++ = ltoh32(*src++);

	nvram_space_str = early_nvram_get("nvram_space");
	if (nvram_space_str)
		nvram_space = bcm_strtoul(nvram_space_str, NULL, 0);

	return 0;
}

/* Early (before mm or mtd) read-only access to NVRAM */
static char *
early_nvram_get(const char *name)
{
	char *var, *value, *end, *eq;

	if (!name)
		return NULL;

	/* Too early? */
	if (sih == NULL)
		return NULL;

	if (!nvram_buf[0])
		if (early_nvram_init() != 0) {
			printk("early_nvram_get: Failed reading nvram var %s\n", name);
			return NULL;
		}

	/* Look for name=value and return value */
	var = &nvram_buf[sizeof(struct nvram_header)];
	end = nvram_buf + sizeof(nvram_buf) - 2;
	end[0] = end[1] = '\0';
	for (; *var; var = value + strlen(value) + 1) {
		if (!(eq = strchr(var, '=')))
			break;
		value = eq + 1;
		if ((eq - var) == strlen(name) && strncmp(var, name, (eq - var)) == 0)
			return value;
	}

	return NULL;
}

static int
early_nvram_getall(char *buf, int count)
{
	char *var, *end;
	int len = 0;

	/* Too early? */
	if (sih == NULL)
		return -1;

	if (!nvram_buf[0])
		if (early_nvram_init() != 0) {
			printk("early_nvram_getall: Failed reading nvram var\n");
			return -1;
		}

	bzero(buf, count);

	/* Write name=value\0 ... \0\0 */
	var = &nvram_buf[sizeof(struct nvram_header)];
	end = nvram_buf + sizeof(nvram_buf) - 2;
	end[0] = end[1] = '\0';
	for (; *var; var += strlen(var) + 1) {
		if ((count - len) <= (strlen(var) + 1))
			break;
		len += sprintf(buf + len, "%s", var) + 1;
	}

	return 0;
}
#endif /* !MODULE */

extern char * _nvram_get(const char *name);
extern int _nvram_set(const char *name, const char *value);
extern int _nvram_unset(const char *name);
extern int _nvram_getall(char *buf, int count);
extern int _nvram_commit(struct nvram_header *header);
extern int _nvram_init(si_t *sih);
extern void _nvram_exit(void);

/* Globals */
DEFINE_SPINLOCK(nvram_lock);
static struct mutex nvram_sem;
static unsigned long nvram_offset = 0;
static int nvram_major = -1;
static struct class *nvram_class = NULL;
static struct mtd_info *nvram_mtd = NULL;

int
_nvram_read(char *buf)
{
	struct nvram_header *header = (struct nvram_header *) buf;
	size_t len;
	int offset = 0;

	if (nvram_mtd) {
#ifdef CONFIG_MTD_NFLASH
		if (nvram_mtd->type == MTD_NANDFLASH)
			offset = 0;
		else
#endif
			offset = nvram_mtd->size - nvram_space;
	}
	printk(KERN_INFO "read %d bytes to offset %d\n",nvram_space, offset);
	if (nvram_inram || !nvram_mtd ||
	    mtd_read(nvram_mtd, offset, nvram_space, &len, buf) ||
	    len != nvram_space ||
	    header->magic != NVRAM_MAGIC) {
		/* Maybe we can recover some data from early initialization */
		if (nvram_inram)
			printk("Sourcing NVRAM from ram\n");
		memcpy(buf, nvram_buf, nvram_space);
	}

	return 0;
}

struct nvram_tuple *
_nvram_realloc(struct nvram_tuple *t, const char *name, const char *value)
{
	if ((nvram_offset + strlen(value) + 1) > nvram_space)
		return NULL;

	if (!t) {
		if (!(t = MMALLOC(sizeof(struct nvram_tuple) + strlen(name) + 1)))
			return NULL;

		/* Copy name */
		t->name = (char *) &t[1];
		strcpy(t->name, name);

		t->value = NULL;
	}

	/* Copy value */
	if (t->value == NULL || strlen(t->value) < strlen(value)) {
		/* Alloc value space */
		t->value = &nvram_buf[nvram_offset];
		strcpy(t->value, value);
		nvram_offset += strlen(value) + 1;
	} else if( 0 != strcmp(t->value, value)) {
		/* In place */
		strcpy(t->value, value);
	} 

	return t;
}

void
_nvram_free(struct nvram_tuple *t)
{
	if (!t) {
		nvram_offset = 0;
		memset( nvram_buf, 0, sizeof(nvram_buf) );
	} else {
		MMFREE(t);
	}
}

int
nvram_init(void *sih)
{
	return 0;
}

int
nvram_set(const char *name, const char *value)
{
	unsigned long flags;
	int ret;
	struct nvram_header *header;

	spin_lock_irqsave(&nvram_lock, flags);
	if ((ret = _nvram_set(name, value))) {
		printk( KERN_INFO "nvram: consolidating space!\n");
		/* Consolidate space and try again */
		if ((header = MMALLOC(nvram_space))) {
			if (_nvram_commit(header) == 0)
				ret = _nvram_set(name, value);
			MMFREE(header);
		}
	}
	spin_unlock_irqrestore(&nvram_lock, flags);

	return ret;
}

char *
real_nvram_get(const char *name)
{
	unsigned long flags;
	char *value;

	spin_lock_irqsave(&nvram_lock, flags);
	value = _nvram_get(name);
	spin_unlock_irqrestore(&nvram_lock, flags);

	return value;
}

char *
nvram_get(const char *name)
{
	if (nvram_major >= 0)
		return real_nvram_get(name);
	else
		return early_nvram_get(name);
}

int
nvram_unset(const char *name)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&nvram_lock, flags);
	ret = _nvram_unset(name);
	spin_unlock_irqrestore(&nvram_lock, flags);

	return ret;
}

static void
erase_callback(struct erase_info *done)
{
	wait_queue_head_t *wait_q = (wait_queue_head_t *) done->priv;
	wake_up(wait_q);
}

#ifdef CONFIG_MTD_NFLASH
int
nvram_nflash_commit(void)
{
	char *buf;
	size_t len;
	unsigned int i;
	int ret;
	struct nvram_header *header;
	unsigned long flags;
	u_int32_t offset;

	if (!(buf = MMALLOC(nvram_space))) {
		printk(KERN_WARNING "nvram_commit: out of memory\n");
		return -ENOMEM;
	}

	mutex_lock(&nvram_sem);

	offset = 0;
	header = (struct nvram_header *)buf;
	header->magic = NVRAM_MAGIC;
	/* reset MAGIC before we regenerate the NVRAM,
	 * otherwise we'll have an incorrect CRC
	 */
	/* Regenerate NVRAM */
	spin_lock_irqsave(&nvram_lock, flags);
	ret = _nvram_commit(header);
	spin_unlock_irqrestore(&nvram_lock, flags);
	if (ret)
		goto done;

	/* Write partition up to end of data area */
	i = header->len;
	ret = mtd_write(nvram_mtd, offset, i, &len, buf);
	if (ret || len != i) {
		printk(KERN_WARNING "nvram_commit: write error\n");
		ret = -EIO;
		goto done;
	}

done:
	mutex_unlock(&nvram_sem);
	MMFREE(buf);
	return ret;
}
#endif

int
nvram_commit(void)
{
	char *buf;
	size_t erasesize, len, magic_len;
	unsigned int i;
	int ret;
	struct nvram_header *header;
	unsigned long flags;
	u_int32_t offset;
	DECLARE_WAITQUEUE(wait, current);
	wait_queue_head_t wait_q;
	struct erase_info erase;
	u_int32_t magic_offset = 0; /* Offset for writing MAGIC # */

	if (!nvram_mtd) {
		printk(KERN_ERR "nvram_commit: NVRAM not found\n");
		return -ENODEV;
	}

	if (in_interrupt()) {
		printk(KERN_WARNING "nvram_commit: not committing in interrupt\n");
		return -EINVAL;
	}

#ifdef CONFIG_MTD_NFLASH
	if (nvram_mtd->type == MTD_NANDFLASH)
		return nvram_nflash_commit();
#endif
	/* Backup sector blocks to be erased */
	erasesize = ROUNDUP(nvram_space, nvram_mtd->erasesize);
	if (!(buf = MMALLOC(erasesize))) {
		printk(KERN_WARNING "nvram_commit: out of memory\n");
		return -ENOMEM;
	}

	mutex_lock(&nvram_sem);

	if ((i = erasesize - nvram_space) > 0) {
		offset = nvram_mtd->size - erasesize;
		len = 0;
		ret = mtd_read(nvram_mtd, offset, i, &len, buf);
		if (ret || len != i) {
			printk(KERN_ERR "nvram_commit: read error ret = %d, len = %d/%d\n", ret, len, i);
			ret = -EIO;
			goto done;
		}
		header = (struct nvram_header *)(buf + i);
		magic_offset = i + ((void *)&header->magic - (void *)header);
	} else {
		offset = nvram_mtd->size - nvram_space;
		header = (struct nvram_header *)buf;
		magic_offset = ((void *)&header->magic - (void *)header);
	}

	/* clear the existing magic # to mark the NVRAM as unusable 
	 * we can pull MAGIC bits low without erase
	 */
	header->magic = NVRAM_CLEAR_MAGIC; /* All zeros magic */
	/* Unlock sector blocks */
	mtd_unlock(nvram_mtd, offset, nvram_mtd->erasesize);
	ret = mtd_write(nvram_mtd, offset + magic_offset, sizeof(header->magic),
		&magic_len, (char *)&header->magic);
	if (ret || magic_len != sizeof(header->magic)) {
		printk(KERN_ERR "nvram_commit: clear MAGIC error\n");
		ret = -EIO;
		goto done;
	}

	header->magic = NVRAM_MAGIC;
	/* reset MAGIC before we regenerate the NVRAM,
	 * otherwise we'll have an incorrect CRC
	 */
	/* Regenerate NVRAM */
	spin_lock_irqsave(&nvram_lock, flags);
	ret = _nvram_commit(header);
	spin_unlock_irqrestore(&nvram_lock, flags);
	if (ret)
		goto done;

	/* Erase sector blocks */
	init_waitqueue_head(&wait_q);
	for (; offset < nvram_mtd->size - nvram_space + header->len;
		offset += nvram_mtd->erasesize) {

		erase.mtd = nvram_mtd;
		erase.addr = offset;
		erase.len = nvram_mtd->erasesize;
		erase.callback = erase_callback;
		erase.priv = (u_long) &wait_q;

		set_current_state(TASK_INTERRUPTIBLE);
		add_wait_queue(&wait_q, &wait);

		/* Unlock sector blocks */
		mtd_unlock(nvram_mtd, offset, nvram_mtd->erasesize);

		if ((ret = mtd_erase(nvram_mtd, &erase))) {
			set_current_state(TASK_RUNNING);
			remove_wait_queue(&wait_q, &wait);
			printk(KERN_ERR "nvram_commit: erase error\n");
			goto done;
		}

		/* Wait for erase to finish */
		schedule();
		remove_wait_queue(&wait_q, &wait);
	}

	/* Write partition up to end of data area */
	header->magic = NVRAM_INVALID_MAGIC; /* All ones magic */
	offset = nvram_mtd->size - erasesize;
	i = erasesize - nvram_space + header->len;
	ret = mtd_write(nvram_mtd, offset, i, &len, buf);
	if (ret || len != i) {
		printk(KERN_ERR "nvram_commit: write error\n");
		ret = -EIO;
		goto done;
	}

	/* Now mark the NVRAM in flash as "valid" by setting the correct
	 * MAGIC #
	 */
	header->magic = NVRAM_MAGIC;
	ret = mtd_write(nvram_mtd, offset + magic_offset, sizeof(header->magic),
		&magic_len, (char *)&header->magic);
	if (ret || magic_len != sizeof(header->magic)) {
		printk(KERN_ERR "nvram_commit: write MAGIC error\n");
		ret = -EIO;
		goto done;
	}

	offset = nvram_mtd->size - erasesize;
	ret = mtd_read(nvram_mtd, offset, 4, &len, buf);

done:
	mutex_unlock(&nvram_sem);
	MMFREE(buf);
	return ret;
}

int
nvram_getall(char *buf, int count)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&nvram_lock, flags);
	if (nvram_major >= 0)
		ret = _nvram_getall(buf, count);
	else
		ret = early_nvram_getall(buf, count);
	spin_unlock_irqrestore(&nvram_lock, flags);

	return ret;
}

EXPORT_SYMBOL(nvram_init);
EXPORT_SYMBOL(nvram_get);
EXPORT_SYMBOL(nvram_getall);
EXPORT_SYMBOL(nvram_set);
EXPORT_SYMBOL(nvram_unset);
EXPORT_SYMBOL(nvram_commit);

/* User mode interface below */

static ssize_t
dev_nvram_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	char tmp[100], *name = tmp, *value;
	ssize_t ret;
	unsigned long off;

	if ((count+1) > sizeof(tmp)) {
		if (!(name = MMALLOC(count+1)))
			return -ENOMEM;
	}

	if (copy_from_user(name, buf, count)) {
		ret = -EFAULT;
		goto done;
	}
	name[count] = '\0';

	if (*name == '\0') {
		/* Get all variables */
		ret = nvram_getall(name, count);
		if (ret == 0) {
			if (copy_to_user(buf, name, count)) {
				ret = -EFAULT;
				goto done;
			}
			ret = count;
		}
	} else {
		if (!(value = nvram_get(name))) {
			ret = 0;
			goto done;
		}

		/* Provide the offset into mmap() space */
		off = (unsigned long) value - (unsigned long) nvram_buf;

		if (copy_to_user(buf, &off, ret = sizeof(off))) {
			ret = -EFAULT;
			goto done;
		}
	}
#ifdef	_DEPRECATED
	flush_cache_all();
#endif
done:
	if (name != tmp)
		MMFREE(name);

	return ret;
}

static ssize_t
dev_nvram_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	char tmp[100], *name = tmp, *value;
	ssize_t ret;

	if (count >= sizeof(tmp)) {
		if (!(name = MMALLOC(count+1)))
			return -ENOMEM;
	}

	if (copy_from_user(name, buf, count)) {
		ret = -EFAULT;
		goto done;
	}
	name[ count ] = '\0';
	value = name;
	name = strsep(&value, "=");
	if (value)
		ret = nvram_set(name, value) ;
	else
		ret = nvram_unset(name) ;

	if( 0 == ret )
		ret = count;
done:
	if (name != tmp)
		MMFREE(name);

	return ret;
}

#define NVRAM_SPACE_MAGIC			0x50534341	/* 'SPAC' */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
static int
#else
static long
#endif
dev_nvram_ioctl(
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
	struct inode *inode, 
#endif
	struct file *file, 
	unsigned int cmd, 
	unsigned long arg)
{

	switch (cmd) {
	case NVRAM_MAGIC:
		nvram_commit();
		break;
	case NVRAM_SPACE_MAGIC:
		return nvram_space;
		break;
	default:
		return -EINVAL;
		break;

	}
}

static int
dev_nvram_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long offset = virt_to_phys(nvram_buf);
	if (remap_pfn_range(vma,vma->vm_start, offset>>PAGE_SHIFT, vma->vm_end-vma->vm_start,
			     vma->vm_page_prot))
		{
		return -EAGAIN;
		}

	return 0;
}

static int
dev_nvram_open(struct inode *inode, struct file * file)
{
	return 0;
}

static int
dev_nvram_release(struct inode *inode, struct file * file)
{
	return 0;
}

static struct file_operations dev_nvram_fops = {
	owner:		THIS_MODULE,
	open:		dev_nvram_open,
	release:	dev_nvram_release,
	read:		dev_nvram_read,
	write:		dev_nvram_write,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
	ioctl:		dev_nvram_ioctl,
#else
	unlocked_ioctl:	dev_nvram_ioctl,
#endif
	mmap:		dev_nvram_mmap
};

static void
dev_nvram_exit(void)
{
	int order = 0;
	struct page *page, *end;

	if (nvram_class) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
		class_device_destroy(nvram_class, MKDEV(nvram_major, 0));
#else /* 2.6.36 and up */
		device_destroy(nvram_class, MKDEV(nvram_major, 0));
#endif
		class_destroy(nvram_class);
	}

	if (nvram_major >= 0)
		unregister_chrdev(nvram_major, "nvram");

	if (nvram_mtd)
		put_mtd_device(nvram_mtd);

	while ((PAGE_SIZE << order) < MAX_NVRAM_SPACE)
		order++;
	end = virt_to_page(nvram_buf + (PAGE_SIZE << order) - 1);
	for (page = virt_to_page(nvram_buf); page <= end; page++)
		ClearPageReserved(page);

	_nvram_exit();
}

static int
dev_nvram_init(void)
{
	int order = 0, ret = 0;
	struct page *page, *end;
	osl_t *osh;
	struct mtd_info *nvram_mtd_cfe = NULL;
	struct mtd_info *nvram_mtd_temp = NULL;
	DECLARE_WAITQUEUE(wait, current);
	wait_queue_head_t wait_q;
	struct erase_info erase;
	struct nvram_header *header;
	u32 *src, *dst;
#if defined(CONFIG_MTD) || defined(CONFIG_MTD_MODULE)
	unsigned int i;
#endif

	/* Allocate and reserve memory to mmap() */
	while ((PAGE_SIZE << order) < nvram_space)
		order++;
	end = virt_to_page(nvram_buf + (PAGE_SIZE << order) - 1);
	for (page = virt_to_page(nvram_buf); page <= end; page++) {
		SetPageReserved(page);
	}

#if defined(CONFIG_MTD) || defined(CONFIG_MTD_MODULE)
	/* Find associated MTD device */
	for (i = 0; i < MAX_MTD_DEVICES; i++) {
		nvram_mtd_temp = get_mtd_device(NULL, i);
		if (!IS_ERR(nvram_mtd_temp)) {
			if (!strcmp(nvram_mtd_temp->name, "nvram_cfe")) {
				nvram_mtd_cfe = nvram_mtd_temp;
				printk(KERN_EMERG "found cfe nvram\n");
				continue;
			}
			if (!strcmp(nvram_mtd_temp->name, "nvram") && nvram_mtd_temp->size >= nvram_space) {
				nvram_mtd = nvram_mtd_temp;
				continue;
			}
			put_mtd_device(nvram_mtd_temp);
		}
	}

	if (nvram_mtd_cfe != NULL && remap_cfe && nvram_space != 0x20000) {
		printk(KERN_INFO "check if nvram copy is required CFE Size is %d\n", NVRAM_SPACE);
		int len;
		char *buf = MMALLOC(NVRAM_SPACE);
		if (buf == NULL) {
			printk(KERN_ERR "mem allocation error");
			goto done_nofree;
		}
		mtd_read(nvram_mtd, nvram_mtd->erasesize - NVRAM_SPACE,NVRAM_SPACE, &len, buf);
		header = (struct nvram_header *)buf;
		len = 0;
		printk(KERN_INFO "nvram copy magic is %X\n", header->magic);
		if (header->magic != NVRAM_MAGIC) {
			printk(KERN_EMERG "copy cfe nvram to base nvram\n");
			len = 0;
			memset(buf, 0, NVRAM_SPACE);
			mtd_read(nvram_mtd_cfe, nvram_mtd_cfe->erasesize - NVRAM_SPACE, NVRAM_SPACE, &len, buf + nvram_mtd->erasesize - NVRAM_SPACE);
			put_mtd_device(nvram_mtd_cfe);
			mtd_unlock(nvram_mtd, 0, nvram_mtd->erasesize);
			init_waitqueue_head(&wait_q);
			erase.mtd = nvram_mtd;
			erase.addr = 0;
			erase.len = nvram_mtd->erasesize;
			erase.callback = erase_callback;
			erase.priv = (u_long) & wait_q;
			set_current_state(TASK_INTERRUPTIBLE);
			add_wait_queue(&wait_q, &wait);
			if ((ret = mtd_erase(nvram_mtd, &erase))) {
				set_current_state(TASK_RUNNING);
				remove_wait_queue(&wait_q, &wait);
				printk("nvram_commit: erase error\n");
				goto done;
			}
			/* Wait for erase to finish */
			schedule();
			remove_wait_queue(&wait_q, &wait);
			len = 0;
			printk(KERN_INFO "remap nvram %d\n", header->len);

			src = (u32 *)buf + nvram_mtd->erasesize - NVRAM_SPACE;
			dst = (u32 *)nvram_buf;
			for (i = 0; i < sizeof(struct nvram_header); i += 4)
				*dst++ = *src++;
			for (; i < header->len && i < NVRAM_SPACE; i += 4)
				*dst++ = ltoh32(*src++);

			mtd_write(nvram_mtd, nvram_mtd->erasesize - NVRAM_SPACE, NVRAM_SPACE, &len, buf);

		      done:;
			MMFREE(buf);
		      done_nofree:;
		}
	}


#endif

	/* Initialize hash table lock */
	spin_lock_init(&nvram_lock);

	/* Initialize commit semaphore */
	mutex_init(&nvram_sem);

	/* Register char device */
	if ((nvram_major = register_chrdev(229, "nvram", &dev_nvram_fops)) < 0) {
		ret = nvram_major;
		goto err;
	}

	if (si_osh(sih) == NULL) {
		osh = osl_attach(NULL, SI_BUS, FALSE);
		if (osh == NULL) {
			printk("Error allocating osh\n");
			unregister_chrdev(nvram_major, "nvram");
			goto err;
		}
		si_setosh(sih, osh);
	}

	/* Initialize hash table */
	_nvram_init(sih);

	/* Create /dev/nvram handle */
	nvram_class = class_create(THIS_MODULE, "nvram");
	if (IS_ERR(nvram_class)) {
		printk("Error creating nvram class\n");
		goto err;
	}

	/* Add the device nvram0 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
	class_device_create(nvram_class, NULL, MKDEV(nvram_major, 0), NULL, "nvram");
#else /* Linux 2.6.36 and above */
	device_create(nvram_class, NULL, MKDEV(nvram_major, 0), NULL, "nvram");
#endif	/* Linux 2.6.36 */

	return 0;

err:
	dev_nvram_exit();
	return ret;
}

int nvram_match(char *name, char *match)
{
	const char *value = nvram_get(name);
	return (value && !strcmp(value, match));
}


char *nvram_safe_get(const char *name)
{
	return nvram_get(name) ? : "";
}


/*
* This is not a module, and is not unloadable.
* Also, this module must not be initialized before
* the Flash MTD partitions have been set up, in case
* the contents are stored in Flash.
* Thus, late_initcall() macro is used to insert this
* device driver initialization later.
* An alternative solution would be to initialize
* inside the xx_open() call.
* -LR
*/
late_initcall(dev_nvram_init);
