
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include "wrapper.h"
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/mtd/mtd.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
//#include <asm/addrspace.h>
#include <asm/io.h>
#include <linux/uaccess.h>
#include <linux/lzma.h>

#include "nvram_linux.h"

void *MALLOC(size_t size)
{
	void *ptr = kmalloc(size, GFP_ATOMIC | __GFP_NOWARN);
	if (!ptr)
		ptr = vmalloc(size);
	return ptr;
}

void MFREE(void *ptr)
{
	if (is_vmalloc_addr(ptr))
		vfree(ptr);
	else
		kfree(ptr);
}

/* In BSS to minimize text size and page aligned so it can be mmap()-ed */
static char *nvram_buf;

//static char nvram_buf[NVRAM_SPACE] __attribute__((aligned(PAGE_SIZE)));

extern char *_nvram_get(const char *name);
extern int _nvram_set(const char *name, const char *value);
extern int _nvram_unset(const char *name);
extern int _nvram_getall(char *buf, int count);
extern int _nvram_commit(struct nvram_header *header);
extern int _nvram_init(void);
extern void _nvram_exit(void);

/* Globals */
DEFINE_SPINLOCK(nvram_lock);
static struct mutex nvram_sem;
static unsigned long nvram_offset = 0;
static int nvram_major = -1;
//static devfs_handle_t nvram_handle = NULL;
static struct mtd_info *nvram_mtd = NULL;
static size_t nvram_off = -1;
static int magic64_dict = 0;
static void decompress(void *src, void *dst, size_t len);
extern struct nvram_tuple *_nvram_malloc(const char *name, const char *value);
extern struct nvram_tuple *_nvram_realloc(struct nvram_tuple *t,
					  const char *name, const char *value);
extern void _nvram_free(struct nvram_tuple *t);
extern int _nvram_read(void *buf);

int _nvram_read(void *_buf)
{
	size_t len, i;
	char *buf = (char *)_buf;
	struct nvram_header *header = (struct nvram_header *)buf;
	void *lzma;
	int found = 0;
	unsigned char magic[4] = { 0x00, 0x23, 0x14, 0x0e }; // 0x2000
	unsigned char magic64[4] = { 0x00, 0x24, 0x15, 0xff }; // 0x10000
	unsigned char check[4];
	if (nvram_mtd) {
		if (nvram_off == -1) {
			nvram_off = nvram_mtd->size - NVRAM_SPACE_OLD;
			if (nvram_mtd->size > NVRAM_SPACE_OLD) {
				for (i = 0; i < nvram_mtd->size; i += 0x1000) {
					mtd_read(nvram_mtd, i, NVRAM_SPACE_OLD,
						 &len, buf);
					if (header->magic == NVRAM_MAGIC) {
						nvram_off = i;
						found = 1;
						break;
					}
				}
			}
		}
		mtd_read(nvram_mtd, nvram_off, NVRAM_SPACE_OLD, &len, buf);
		if (header->magic != NVRAM_MAGIC) {
			mtd_read(nvram_mtd,
				 nvram_mtd->size - (NVRAM_SPACE_OLD / 2),
				 (NVRAM_SPACE_OLD / 2), &len, buf);
			if (header->magic == NVRAM_MAGIC) {
				found = 1;
				printk(KERN_INFO
				       "nvram: convert old nvram to new one\n");
			}
		} else {
			printk(KERN_INFO "nvram: found nvram at 0x%zx\n",
			       nvram_off);
		}
	}
	if (found)
		return 0;
	nvram_off = 0;
	for (i = 0; i < nvram_mtd->size - 0x1000; i += 0x1000) {
		mtd_read(nvram_mtd, i, 4, &len, check);
		if (!memcmp(check, magic, 4) || !memcmp(check, magic64, 4)) {
			if (!memcmp(check, magic64, 4))
				magic64_dict = 1;
			nvram_off = i;
			printk(KERN_INFO
			       "nvram: found compressed nvram at 0x%zx\n",
			       i);
			break;
		}
	}
	lzma = vmalloc(nvram_mtd->size - nvram_off);
	if (!lzma)
		return 0;
	memset(lzma, 0, nvram_mtd->size - nvram_off);
	mtd_read(nvram_mtd, nvram_off, nvram_mtd->size - nvram_off, &len, lzma);
	decompress(lzma, buf, nvram_mtd->size - nvram_off);
	vfree(lzma);
	return 0;
}

struct nvram_tuple *_nvram_realloc(struct nvram_tuple *t, const char *name,
				   const char *value)
{
	if ((nvram_offset + strlen(value) + 1) > NVRAM_SPACE)
		return NULL;

	if (!t) {
		if (!(t = MALLOC(sizeof(struct nvram_tuple) + strlen(name) +
				 1))) {
			printk("nvram: MALLOC failed\n");
			return NULL;
		}

		/* Copy name */
		t->name = (char *)&t[1];
		strcpy(t->name, name);

		t->value = NULL;
	}

	/* Copy value */
	if (t->value == NULL || strlen(t->value) < strlen(value)) {
		/* Alloc value space */
		t->value = &nvram_buf[nvram_offset];
		strcpy(t->value, value);
		nvram_offset += strlen(value) + 1;
	} else if (0 != strcmp(t->value, value)) {
		/* In place */
		strcpy(t->value, value);
	}

	return t;
}

void _nvram_free(struct nvram_tuple *t)
{
	if (!t) {
		nvram_offset = 0;
		memset(nvram_buf, 0, NVRAM_SPACE);
	} else {
		MFREE(t);
	}
}

int nvram_set(const char *name, const char *value)
{
	unsigned long flags;
	int ret;
	struct nvram_header *header;

	spin_lock_irqsave(&nvram_lock, flags);
	if ((ret = _nvram_set(name, value))) {
		/* Consolidate space and try again */
		if ((header = MALLOC(NVRAM_SPACE))) {
			if (_nvram_commit(header) == 0)
				ret = _nvram_set(name, value);
			MFREE(header);
		}
	}
	spin_unlock_irqrestore(&nvram_lock, flags);

	return ret;
}

char *real_nvram_get(const char *name);
char *real_nvram_get(const char *name)
{
	unsigned long flags;
	char *value;

	spin_lock_irqsave(&nvram_lock, flags);
	value = _nvram_get(name);
	spin_unlock_irqrestore(&nvram_lock, flags);

	return value;
}

char *nvram_get(const char *name)
{
	if (nvram_major >= 0)
		return real_nvram_get(name);
	else
		return NULL;
}

int nvram_unset(const char *name)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&nvram_lock, flags);
	ret = _nvram_unset(name);
	spin_unlock_irqrestore(&nvram_lock, flags);

	return ret;
}

static CLzmaEncHandle *p;
static Byte propsEncoded[LZMA_PROPS_SIZE];
static SizeT propsSize = sizeof(propsEncoded);

static void lzma_free_workspace(void)
{
	LzmaEnc_Destroy(p, &lzma_alloc, &lzma_alloc);
}

static int lzma_alloc_workspace(CLzmaEncProps *props)
{
	if ((p = (CLzmaEncHandle *)LzmaEnc_Create(&lzma_alloc)) == NULL) {
		printk(KERN_ERR
		       "nvram: Failed to allocate lzma deflate workspace\n");
		return -ENOMEM;
	}

	if (LzmaEnc_SetProps(p, props) != SZ_OK) {
		lzma_free_workspace();
		return -1;
	}

	if (LzmaEnc_WriteProperties(p, propsEncoded, &propsSize) != SZ_OK) {
		lzma_free_workspace();
		return -1;
	}

	return 0;
}

static void *compress(void *src, size_t len, SizeT *compress_size)
{
	void *dst;
	int ret;
	CLzmaEncProps props;
	LzmaEncProps_Init(&props);

	props.dictSize = LZMA_BEST_DICT(0x10000);
	props.level = LZMA_BEST_LEVEL;
	props.lc = LZMA_BEST_LC;
	props.lp = LZMA_BEST_LP;
	props.pb = LZMA_BEST_PB;
	props.fb = LZMA_BEST_FB;

	ret = lzma_alloc_workspace(&props);
	if (ret < 0)
		return NULL;
	dst = vmalloc(nvram_mtd->size);
	if (!dst)
		return NULL;
	memset(dst, 0, nvram_mtd->size);
	ret = LzmaEnc_MemEncode(p, dst, compress_size, src, len, 0, NULL,
				&lzma_alloc, &lzma_alloc);
	lzma_free_workspace();
	return dst;
}

static void decompress(void *src, void *dst, size_t len)
{
	SizeT dl = (SizeT)NVRAM_SPACE;
	SizeT sl = (SizeT)len;
	ELzmaStatus status;
	int ret;
	unsigned int magic = NVRAM_MAGIC;
	CLzmaEncProps props;
	if (!memcmp(src, &magic, 4)) {
		memcpy(dst, src, len);
		return;
	}
	LzmaEncProps_Init(&props);

	props.dictSize = LZMA_BEST_DICT(0x10000);
	props.level = LZMA_BEST_LEVEL;
	props.lc = LZMA_BEST_LC;
	props.lp = LZMA_BEST_LP;
	props.pb = LZMA_BEST_PB;
	props.fb = LZMA_BEST_FB;

	ret = lzma_alloc_workspace(&props);
	if (ret < 0) {
		printk(KERN_INFO "nvram: alloc workspace failed\n");
		return;
	}

	ret = LzmaDecode(dst, &dl, src, &sl, propsEncoded, propsSize,
			 LZMA_FINISH_ANY, &status, &lzma_alloc);
	if (ret != SZ_OK || status == LZMA_STATUS_NOT_FINISHED) {
		printk(KERN_INFO
		       "nvram: decompress failed %zu ret %d status %d\n",
		       dl, ret, status);
		return;
	}
	lzma_free_workspace();
	return;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 00)
static void erase_callback(struct erase_info *done)
{
	wait_queue_head_t *wait_q = (wait_queue_head_t *) done->priv;
	wake_up(wait_q);
}
#endif

static u_int32_t bad[256];
int nvram_commit(void)
{
	char *buf;
	void *lzma;
	size_t erasesize, len;
	u_int32_t alternate;
	int i;
	int ret, counts, esize;
	int errorfound = 0;
	struct nvram_header *header;
	unsigned long flags;
	static int waiting = 0;
	u_int32_t offset, cnt = 0;
	struct erase_info erase;
	size_t target_size;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 00)
	DECLARE_WAITQUEUE(wait, current);
	wait_queue_head_t wait_q;
#endif
	if (!nvram_mtd) {
		printk("nvram_commit: NVRAM not found\n");
		return -ENODEV;
	}

	if (in_interrupt()) {
		printk("nvram_commit: not committing in interrupt\n");
		return -EINVAL;
	}

	if (waiting > 1) {
		printk("nvram_commit: commit still pending, cancle new one\n");
		return 0; // we can ignore it, since another commit is still waiting
	}
	waiting++;
	/* Backup sector blocks to be erased */
	mutex_lock(&nvram_sem);
	erasesize = nvram_mtd->erasesize;
	if (!(buf = MALLOC(NVRAM_SPACE))) {
		printk("nvram_commit: out of memory\n");
		mutex_unlock(&nvram_sem);
		waiting--;
		return -ENOMEM;
	}
	nvram_off = 0; // recalculate
	offset = nvram_off;
	header = (struct nvram_header *)buf;

	/* Regenerate NVRAM */
	spin_lock_irqsave(&nvram_lock, flags);
	ret = _nvram_commit(header);
	spin_unlock_irqrestore(&nvram_lock, flags);
	if (ret)
		goto done;
	/* Erase sector blocks */
	memset(&bad[0], -1, 256 * sizeof(int));
	esize = nvram_mtd->erasesize;
	counts = ((unsigned int)nvram_mtd->size / esize);
	if (!counts)
		counts = 1;
fullerase:;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 00)
	init_waitqueue_head(&wait_q);
#endif
	for (; offset < nvram_mtd->size; offset += nvram_mtd->erasesize) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 00)
		erase.mtd = nvram_mtd;
#endif
		erase.addr = offset;
		erase.len = nvram_mtd->erasesize;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 00)
		erase.callback = erase_callback;
		erase.priv = (u_long)&wait_q;
		set_current_state(TASK_INTERRUPTIBLE);
		add_wait_queue(&wait_q, &wait);
#endif 

		/* Unlock sector blocks */
		mtd_unlock(nvram_mtd, offset, nvram_mtd->erasesize);
		if ((ret = mtd_erase(nvram_mtd, &erase))) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 00)
			set_current_state(TASK_RUNNING);
			remove_wait_queue(&wait_q, &wait);
#endif
			printk("nvram_commit: erase error offset %X, skipping\n",
			       offset);
			goto done;
			for (i = 0; i < counts; i++) {
				if ((cnt + i) < 256)
					bad[cnt + i] = offset;
			}
			cnt++;
			if (!errorfound) {
				errorfound = 1;
				cnt = 0;
				offset = 0;
				goto fullerase;
			}
			continue;
		}
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 00)
		schedule();
		remove_wait_queue(&wait_q, &wait);
#endif
		cnt++;
		/* Wait for erase to finish */
	}
	offset = nvram_off;
	alternate = 0;
	//	printk(KERN_INFO "counts %d\n", counts);
	for (cnt = 0; cnt < 256; cnt++) {
		//		if (bad[cnt]!=-1)
		//		    printk(KERN_INFO "bad table idx %d: %X\n", cnt, bad[cnt]);
		if (bad[cnt] == -1) {
			for (i = 0; i < counts; i++) {
				if (bad[cnt + i] != -1)
					goto next;
			}
			//			printk(KERN_INFO "alternate option %X\n", cnt * esize);
			alternate = cnt * esize;
		}
next:;
		if (bad[cnt] == offset) {
			offset = alternate;
			printk("nvram_commit: use alternate offset %X\n",
			       offset);
			break;
		}
	}
	target_size = nvram_mtd->size;
	lzma = compress(buf, header->len, &target_size);
	if (!lzma) {
		printk(KERN_ERR "nvram: compress failed\n");
		goto done;
	}
	target_size = ROUNDUP(target_size, (unsigned int)nvram_mtd->erasesize);
	ret = mtd_write(nvram_mtd, offset, target_size, &len, lzma);
	vfree(lzma);
	if (ret || len != target_size) {
		printk("nvram_commit: write error (offset %d size %zu)\n",
		       offset, len);
		ret = -EIO;
		goto done;
	}
done:
	mutex_unlock(&nvram_sem);
	waiting--;
	MFREE(buf);
	return ret;
}

int nvram_getall(char *buf, int count)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&nvram_lock, flags);
	ret = _nvram_getall(buf, count);
	spin_unlock_irqrestore(&nvram_lock, flags);

	return ret;
}

EXPORT_SYMBOL(nvram_get);
EXPORT_SYMBOL(nvram_getall);
EXPORT_SYMBOL(nvram_set);
EXPORT_SYMBOL(nvram_unset);
EXPORT_SYMBOL(nvram_commit);

/* User mode interface below */

static ssize_t dev_nvram_read(struct file *file, char *buf, size_t count,
			      loff_t *ppos)
{
	char tmp[100], *name = tmp, *value;
	ssize_t ret;
	unsigned long off;

	if ((count + 1) > sizeof(tmp)) {
		if (!(name = MALLOC(count + 1)))
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
		off = (unsigned long)value - (unsigned long)nvram_buf;

		if (copy_to_user(buf, &off, ret = sizeof(off))) {
			ret = -EFAULT;
			goto done;
		}
	}
#ifdef _DEPRECATED
	flush_cache_all();
#endif
done:
	if (name != tmp)
		MFREE(name);

	return ret;
}

static ssize_t dev_nvram_write(struct file *file, const char *buf, size_t count,
			       loff_t *ppos)
{
	char tmp[100], *name = tmp, *value;
	ssize_t ret;

	if (count >= sizeof(tmp)) {
		if (!(name = MALLOC(count + 1)))
			return -ENOMEM;
	}

	if (copy_from_user(name, buf, count)) {
		ret = -EFAULT;
		goto done;
	}
	name[count] = '\0';
	value = name;
	name = strsep(&value, "=");
	if (value)
		ret = nvram_set(name, value);
	else
		ret = nvram_unset(name);

	if (0 == ret)
		ret = count;
done:
	if (name != tmp)
		MFREE(name);

	return ret;
}

static long dev_nvram_ioctl(struct file *file, unsigned int cmd,
			    unsigned long arg)
{
	switch (cmd) {
	case NVRAM_MAGIC:
		nvram_commit();
		return 0;
		break;
	case NVRAM_SPACE_MAGIC:
		return NVRAM_SPACE;
		break;
	default:
		return -EINVAL;
		break;
	}
}

//int remap_pfn_range(struct vm_area_struct *vma, unsigned long addr,
//                  unsigned long pfn, unsigned long size, pgprot_t prot)

static DEFINE_MUTEX(mtd_mutex);

static long nvram_unlocked_ioctl(struct file *file, u_int cmd, u_long arg)
{
	int ret;

	mutex_lock(&mtd_mutex);
	ret = dev_nvram_ioctl(file, cmd, arg);
	mutex_unlock(&mtd_mutex);

	return ret;
}

static int dev_nvram_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long offset = virt_to_phys(nvram_buf);
	//      vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	//      printk(KERN_EMERG "vma size %d\n",offset);
	if (remap_pfn_range(vma, vma->vm_start, offset >> PAGE_SHIFT,
			    vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
		return -EAGAIN;
	}
	return 0;
}

static int dev_nvram_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int dev_nvram_release(struct inode *inode, struct file *file)
{
	return 0;
}

static struct file_operations dev_nvram_fops = {
	owner: THIS_MODULE,
	open: dev_nvram_open,
	release: dev_nvram_release,
	read: dev_nvram_read,
	write: dev_nvram_write,
#ifdef CONFIG_COMPAT
	compat_ioctl: dev_nvram_ioctl,
#endif
	unlocked_ioctl: nvram_unlocked_ioctl,
	mmap: dev_nvram_mmap,
};

static void dev_nvram_exit(void)
{
	int order = 0;
	struct page *page, *end;

	//      if (nvram_handle)
	//              devfs_unregister(nvram_handle);

	//      if (nvram_major >= 0)
	//              devfs_unregister_chrdev(nvram_major, "nvram");

	if (nvram_mtd)
		put_mtd_device(nvram_mtd);

	while ((PAGE_SIZE << order) < NVRAM_SPACE)
		order++;
	end = virt_to_page(nvram_buf + (PAGE_SIZE << order) - 1);
	for (page = virt_to_page(nvram_buf); page <= end; page++)
		mem_map_unreserve(page);

	_nvram_exit();
	kfree(nvram_buf);
}

static int __init dev_nvram_init(void)
{
	int order = 0, ret = 0;
	struct page *page, *end;
	unsigned int i;
	/* Allocate and reserve memory to mmap() */
	nvram_buf = kmalloc(NVRAM_SPACE, GFP_KERNEL);
	/* Allocate and reserve memory to mmap() */
	while ((PAGE_SIZE << order) < NVRAM_SPACE)
		order++;
	end = virt_to_page(nvram_buf + (PAGE_SIZE << order) - 1);
	for (page = virt_to_page(nvram_buf); page <= end; page++)
		mem_map_reserve(page);

#ifdef CONFIG_MTD
	/* Find associated MTD device */
	for (i = 0; i < 32; i++) {
		nvram_mtd = get_mtd_device(NULL, i);
		if (nvram_mtd) {
			if (!strcmp(nvram_mtd->name, "nvram")) {
				printk(KERN_INFO "nvram size = %llu\n",
				       nvram_mtd->size);
				break;
			}
			put_mtd_device(nvram_mtd);
		}
	}
	if (i >= 32) {
		printk(KERN_EMERG "no nvram partition found\n");
		nvram_mtd = NULL;
		return -1;
	}
#endif
	/* Initialize hash table lock */
	spin_lock_init(&nvram_lock);

	/* Initialize commit semaphore */
	mutex_init(&nvram_sem);

	/* Register char device */
	if ((nvram_major = register_chrdev(229, "nvram", &dev_nvram_fops)) <
	    0) {
		ret = nvram_major;
		goto err;
	}

	/* Initialize hash table */
	if (_nvram_init())
		return -1;

	/* Create /dev/nvram handle */

	//      nvram_handle = devfs_register(NULL, "nvram", DEVFS_FL_NONE, nvram_major, 0,
	//                                    S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP, &dev_nvram_fops, NULL);

	return 0;

err:
	dev_nvram_exit();
	return ret;
}

late_initcall(dev_nvram_init);
module_exit(dev_nvram_exit);
