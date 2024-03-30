
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/bootmem.h>
#include <linux/vmalloc.h>
#include "wrapper.h"
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
//#include <asm/addrspace.h>
#include <asm/io.h>
#include <linux/uaccess.h>

#include "nvram_linux.h"

/* In BSS to minimize text size and page aligned so it can be mmap()-ed */
static char nvram_buf[NVRAM_SPACE] __attribute__((aligned(PAGE_SIZE)));

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

#ifdef MODULE

#define early_nvram_get(name) nvram_get(name)

#else				/* !MODULE */

/* Convenience */
#define KB * 1024
#define MB * 1024 * 1024

/* Probe for NVRAM header */
static void early_nvram_init(void)
{
	struct file *srcf;
	struct nvram_header *header;
	int i;
	int len;
	char *wr;
	loff_t offs;
	mm_segment_t old_fs = get_fs();
	set_fs(get_ds());
	srcf = filp_open("/usr/local/nvram/nvram.bin", O_RDONLY, 0);
	if (IS_ERR(srcf)) {
		printk("nvram not yet ready\n");
		set_fs(old_fs);
		return;
	}
	if ((srcf->f_op == NULL) || (srcf->f_op->read == NULL) || (srcf->f_op->write == NULL)) {
		printk("nvram not yet ready\n");
		filp_close(srcf, NULL);
		set_fs(old_fs);
		return;
	}			/* End of if */
	char *buffer = MALLOC(NVRAM_SPACE);
	wr = buffer;
	offs = 0;
	for (i = 0; i < (NVRAM_SPACE / PAGE_SIZE); i++) {
		len = kernel_read(srcf, srcf->f_pos + offs, wr, PAGE_SIZE);
		offs += len;
		wr += PAGE_SIZE;
	}

	/* windowed flash access */
	header = (struct nvram_header *)buffer;
	if (header->magic == NVRAM_MAGIC && header->len > 0 && header->len <= NVRAM_SPACE) {
		u32 *src = (u32 *)header;
		u32 *dst = (u32 *)nvram_buf;
		for (i = 0; i < sizeof(struct nvram_header); i += 4)
			*dst++ = *src++;
		for (; i < header->len && i < NVRAM_SPACE; i += 4)
			*dst++ = ltoh32(*src++);
		filp_close(srcf, NULL);
		MFREE(buffer);
		return;
	}
	filp_close(srcf, NULL);
	MFREE(buffer);
	set_fs(old_fs);
}

/* Early (before mm or mtd) read-only access to NVRAM */
static char *early_nvram_get(const char *name)
{
	char *var, *value, *end, *eq;

	if (!name)
		return NULL;

	if (!nvram_buf[0])
		early_nvram_init();

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

#endif				/* !MODULE */

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

int _nvram_read(char *buf)
{
	struct nvram_header *header = (struct nvram_header *)buf;
	size_t len;
	struct file *srcf;
	char *wr;
	loff_t offs;
	int i;

	mm_segment_t old_fs = get_fs();
	set_fs(get_ds());

	srcf = filp_open("/usr/local/nvram/nvram.bin", O_RDONLY, 0);
	if (IS_ERR(srcf)) {
		printk(KERN_INFO "Broken NVRAM found, recovering it (filesystem error)\n");
		/* Maybe we can recover some data from early initialization */
		memcpy(buf, nvram_buf, NVRAM_SPACE);
		memset(buf, 0, NVRAM_SPACE);
		header->magic = NVRAM_MAGIC;
		header->len = 0;
		set_fs(old_fs);
		nvram_commit();
		return 0;
	}
	wr = buf;
	offs = 0;
	for (i = 0; i < (NVRAM_SPACE / PAGE_SIZE); i++) {
		len = kernel_read(srcf, srcf->f_pos + offs, wr, PAGE_SIZE);
		if (!len)
			break;
		offs += len;
		wr += PAGE_SIZE;
	}
	filp_close(srcf, NULL);
	set_fs(old_fs);
	if (!offs || header->magic != NVRAM_MAGIC) {
		printk(KERN_INFO "Broken NVRAM found, recovering it (header error) len = %llu\n", offs);
		/* Maybe we can recover some data from early initialization */
		memcpy(buf, nvram_buf, NVRAM_SPACE);
		memset(buf, 0, NVRAM_SPACE);
		header->magic = NVRAM_MAGIC;
		header->len = 0;
		nvram_commit();
	}
	return 0;
}

struct nvram_tuple *_nvram_realloc(struct nvram_tuple *t, const char *name, const char *value)
{
	if ((nvram_offset + strlen(value) + 1) > NVRAM_SPACE)
		return NULL;

	if (!t) {
		if (!(t = MALLOC(sizeof(struct nvram_tuple) + strlen(name) + 1)))
			return NULL;

		/* Copy name */
		t->name = (char *)&t[1];
		strcpy(t->name, name);

		t->value = NULL;
	}

	/* Copy value */
	if (!t->value || strcmp(t->value, value)) {
		t->value = &nvram_buf[nvram_offset];
		strcpy(t->value, value);
		nvram_offset += strlen(value) + 1;
	}

	return t;
}

void _nvram_free(struct nvram_tuple *t)
{
	if (!t) {
		nvram_offset = 0;
		memset(nvram_buf, 0, sizeof(nvram_buf));
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
		return early_nvram_get(name);
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

int nvram_commit(void)
{
	char *buf, *wr;
	size_t erasesize, len;
	int ret;
	int i;
	loff_t offs;
	struct nvram_header *header;
	unsigned long flags;
	u_int32_t offset;
	static int waiting=0;
	struct file *srcf;

	if (in_interrupt()) {
		printk(KERN_INFO "nvram_commit: not committing in interrupt\n");
		return -EINVAL;
	}

	if (waiting > 1) {
		printk(KERN_INFO "nvram_commit: commit still pending, cancle new one\n");
		return 0; // we can ignore it, since another commit is still waiting
	}
	waiting++;
	mutex_lock(&nvram_sem);
	/* Backup sector blocks to be erased */
	erasesize = NVRAM_SPACE;
	if (!(buf = MALLOC(erasesize))) {
		printk(KERN_ERR "nvram_commit: out of memory\n");
		mutex_unlock(&nvram_sem);
		waiting--;
		return -ENOMEM;
	}


	offset = 0;
	header = (struct nvram_header *)buf;

	/* Regenerate NVRAM */
	spin_lock_irqsave(&nvram_lock, flags);
	ret = _nvram_commit(header);
	spin_unlock_irqrestore(&nvram_lock, flags);
	if (ret) {
		printk(KERN_INFO "error on nvram_commit\n");
		goto done;
	}

	mm_segment_t old_fs = get_fs();
	set_fs(get_ds());
	srcf = filp_open("/usr/local/nvram/nvram.bin", O_CREAT | O_RDWR | O_LARGEFILE, 0600);
	if (IS_ERR(srcf)) {
		printk(KERN_INFO "Open error\n");
		ret = -EIO;
		set_fs(old_fs);
		goto done;
	}

	wr = buf;
	offs = 0;
	for (i = 0; i < (NVRAM_SPACE / PAGE_SIZE); i++) {
		len = kernel_write(srcf, wr, PAGE_SIZE, srcf->f_pos + offs);
		offs += len;
		wr += PAGE_SIZE;
	}
	printk(KERN_INFO "nvram_commit: %lld bytes written\n", offs);
	filp_close(srcf, NULL);
	set_fs(old_fs);
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

static ssize_t dev_nvram_read(struct file *file, char *buf, size_t count, loff_t * ppos)
{
	char tmp[128], *name = tmp, *value;
	ssize_t ret;
	unsigned long off;

	if (count > sizeof(tmp)) {
		if (!(name = MALLOC(count)))
			return -ENOMEM;
	}

	if (copy_from_user(name, buf, count)) {
		ret = -EFAULT;
		goto done;
	}

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

		if (put_user(off, (unsigned long *)buf)) {
			ret = -EFAULT;
			goto done;
		}

		ret = sizeof(unsigned long);
	}

//      flush_cache_all();      

done:
	if (name != tmp)
		MFREE(name);

	return ret;
}

static ssize_t dev_nvram_write(struct file *file, const char *buf, size_t count, loff_t * ppos)
{
	char tmp[128], *name = tmp, *value;
	ssize_t ret;

	if (count >= sizeof(tmp)) {
		if (!(name = MALLOC(count + 1)))
			return -ENOMEM;
	}

	if (copy_from_user(name, buf, count)) {
		ret = -EFAULT;
		goto done;
	}

	value = name;
	name = strsep(&value, "=");
	if (value)
		ret = nvram_set(name, value) ? : count;
	else
		ret = nvram_unset(name) ? : count;

done:
	if (name != tmp)
		MFREE(name);

	return ret;
}

static long dev_nvram_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
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

static DEFINE_MUTEX(mtd_mutex);

static long nvram_unlocked_ioctl(struct file *file, u_int cmd, u_long arg)
{
	long ret;

	mutex_lock(&mtd_mutex);
	ret = dev_nvram_ioctl(file, cmd, arg);
	mutex_unlock(&mtd_mutex);
	return ret;
}

//int remap_pfn_range(struct vm_area_struct *vma, unsigned long addr,
//                  unsigned long pfn, unsigned long size, pgprot_t prot)

static int dev_nvram_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long offset = virt_to_phys(nvram_buf);
	if (remap_pfn_range(vma, vma->vm_start, offset >> PAGE_SHIFT, vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
		return -EAGAIN;
	}
	return 0;
}

static int isinit = 0;
static int dev_nvram_open(struct inode *inode, struct file *file)
{
	/* Initialize hash table */
	int ret = 0;
	if (!isinit)
		ret = _nvram_init();
	if (ret == 0)
		isinit = 1;
	return 0;
}

static int dev_nvram_release(struct inode *inode, struct file *file)
{
	return 0;
}

static struct file_operations dev_nvram_fops = {
      owner:THIS_MODULE,
      open:dev_nvram_open,
      release:dev_nvram_release,
      read:dev_nvram_read,
      write:dev_nvram_write,
#ifdef CONFIG_COMPAT
      compat_ioctl:dev_nvram_ioctl,
#endif
      unlocked_ioctl:nvram_unlocked_ioctl,
      mmap:dev_nvram_mmap,
};

static void dev_nvram_exit(void)
{
	int order = 0;
	struct page *page, *end;

	while ((PAGE_SIZE << order) < NVRAM_SPACE)
		order++;
	end = virt_to_page(nvram_buf + (PAGE_SIZE << order) - 1);
	for (page = virt_to_page(nvram_buf); page <= end; page++)
		mem_map_unreserve(page);

	_nvram_exit();
}

static int __init dev_nvram_init(void)
{
	int order = 0, ret = 0;
	struct page *page, *end;

	/* Allocate and reserve memory to mmap() */
	while ((PAGE_SIZE << order) < NVRAM_SPACE)
		order++;
	end = virt_to_page(nvram_buf + (PAGE_SIZE << order) - 1);
	for (page = virt_to_page(nvram_buf); page <= end; page++)
		mem_map_reserve(page);

	/* Initialize hash table lock */
	spin_lock_init(&nvram_lock);

	/* Initialize commit semaphore */
	mutex_init(&nvram_sem);

	/* Register char device */
	if ((nvram_major = register_chrdev(229, "nvram", &dev_nvram_fops)) < 0) {
		ret = nvram_major;
		goto err;
	}

	return 0;

err:
	dev_nvram_exit();
	return ret;
}

late_initcall(dev_nvram_init);
module_exit(dev_nvram_exit);
