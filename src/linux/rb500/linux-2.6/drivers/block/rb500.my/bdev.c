#include <linux/kernel.h>	/* printk() */
#include <linux/module.h>	/* module to be loadable */
#include <linux/init.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/fs.h>		/* register_blkdev() */
#include <linux/genhd.h>
#include <linux/blkdev.h>	/* blk_init_queue() */
#include <linux/blkpg.h>	/* blk_ioctl */
#include <linux/hdreg.h>	/* hd_geometry */

#include <asm/io.h>		/* ioremap() */
#include <asm/uaccess.h>	/* copy_from_user() */

#include <asm/rc32434/gpio.h>

#define DEBUG 0

#if DEBUG
#define DEBUGP printk
#define DLEVEL 1
#else
#define DEBUGP(format, args...)
#define DLEVEL 0
#endif

#define CF_MIPS_MAJOR 13
#define MAJOR_NR	CF_MIPS_MAJOR
#define CF_MAX_PART	4		/* max 16 partitions */

#undef DEVICE_REQUEST
#undef DEVICE_NAME
#undef DEVICE_NR
#define DEVICE_REQUEST cf_request
#define DEVICE_NAME	"xda"
#define DEVICE_NR(dev)	0		/* one drive only */

#include "ata.h"

extern struct block_device_operations cf_bdops;
extern struct gendisk cf_gendisk;

static struct hd_struct cf_parts[CF_MAX_PART];
static int cf_part_sizes[CF_MAX_PART];
static int cf_hsect_sizes[CF_MAX_PART];
static int cf_max_sectors[CF_MAX_PART];
static int cf_blksize_sizes[CF_MAX_PART];

static spinlock_t lock = SPIN_LOCK_UNLOCKED;

volatile int cf_busy = 0;

static int
cf_transfer(const struct request *req)
{
	unsigned minor = MINOR(req->rq_dev);
	struct hd_struct *hd = &cf_parts[minor];

	DEBUGP(KERN_INFO "cf_transfer dev %u at 0x%lx size %ld rw %d\n",
	       minor, req->sector, req->nr_sectors, req->cmd);

	if (req->sector + req->nr_sectors > hd->nr_sects) {
		static int count = 0;
		if (count++ < 5)
			printk(KERN_ERR
			       "cf-mips: request 0x%lx past end of device %d\n",
			       req->sector, minor);
		return CF_TRANS_FAILED;
	}

	/* Looks good, do the transfer. */
	switch (req->cmd) {
	case READ:
	case WRITE:
		return cf_do_transfer(req->bh->b_data, req->bh->b_size,
				      req->sector + hd->start_sect,
				      req->nr_sectors,
				      (req->cmd == READ ? 1 : 0));
	default:
		/* can't happen */
		return CF_TRANS_FAILED;
	}
}

static void
cf_request(request_queue_t * q)
{
	int status;
	long flags;

	if (cf_busy)
		return;

	while (1) {
		INIT_REQUEST;	/* returns when queue is empty */

		/* start the transfer */
		spin_lock_irqsave(&lock, flags);
		status = cf_transfer(CURRENT);
		spin_unlock_irqrestore(&lock, flags);

		if (status == CF_TRANS_IN_PROGRESS) {
			cf_busy = 1;
			return;
		}
		end_request(status);
	}
}

void *
cf_get_next_buf(unsigned *buf_size)
{
	struct request *req = CURRENT;

	if (!end_that_request_first(req, CF_TRANS_OK, DEVICE_NAME))
		BUG();

	*buf_size = req->bh->b_size;
	return req->bh->b_data;
}

void
cf_async_trans_done(int result)
{
	unsigned long flags;

	spin_lock_irqsave(&io_request_lock, flags);
	end_request(result);
	cf_busy = 0;
	if (!QUEUE_EMPTY)
		DEVICE_REQUEST(NULL);
	spin_unlock_irqrestore(&io_request_lock, flags);
}

static int
cf_ioctl(struct inode *inode, struct file *filp,
	 unsigned int cmd, unsigned long arg)
{
	unsigned minor = MINOR(inode->i_rdev);

	DEBUGP(KERN_INFO "cf_ioctl cmd %u\n", cmd);
	switch (cmd) {
	case BLKRRPART:	/* re-read partition table */
		if (!capable(CAP_SYS_ADMIN))
			return -EACCES;
		printk(KERN_INFO "cf-mips partition check: \n");
		register_disk(&cf_gendisk, inode->i_rdev,
			      MINOR(inode->i_rdev) ? 1 : CF_MAX_PART,
			      &cf_bdops, cf_sectors);
		return 0;

	case HDIO_GETGEO:{
			struct hd_geometry geo;
			geo.cylinders = cf_cyl;
			geo.heads = cf_head;
			geo.sectors = cf_spt;
			geo.start = cf_gendisk.part[minor].start_sect;
			if (copy_to_user((void *) arg, &geo, sizeof (geo)))
				return -EFAULT;
			return 0;
		}
	default:
		/*
		 * For ioctls we don't understand,
		 * let the block layer handle them.
		 */
		return blk_ioctl(inode->i_rdev, cmd, arg);
	}

	return -ENOTTY;		/* unknown command */
}

static int
cf_open(struct inode *inode, struct file *filp)
{
	int minor = MINOR(inode->i_rdev);
	if (minor >= CF_MAX_PART)
		return -ENODEV;
	DEBUGP(KERN_INFO "cf-mips module opened, minor %d\n", minor);
	MOD_INC_USE_COUNT;
		GPIO_t gpiot = (GPIO_t)GPIO_VirtualAddress;
		/* dirty workaround to set CFRDY GPIO as an input
		 * when some other program sets it as an output
		 */
		gpiot->gpiocfg &= ~(1 << 13);
	return 0;		/* success */
}

static int
cf_release(struct inode *inode, struct file *filp)
{
	int minor = MINOR(inode->i_rdev);
	DEBUGP(KERN_INFO "cf-mips module closed, minor %d\n", minor);
	MOD_DEC_USE_COUNT;
	return 0;
}

static struct block_device_operations cf_bdops = {
      owner:THIS_MODULE,
      open:cf_open,
      release:cf_release,
      ioctl:cf_ioctl,
      check_media_change:NULL,
      revalidate:0,
};

static struct gendisk cf_gendisk = {
      major:MAJOR_NR,
      major_name:"cf",
      minor_shift:8,		/* only one major device */
      max_p:CF_MAX_PART,
      part:cf_parts,
      sizes:cf_part_sizes,
      nr_real:1,
      fops:&cf_bdops,
};

int __init
cf_mips_init(void)
{
	int result;
	int i;

//	if (!is_rb500() && !is_rb100()) return -1;

	result = register_blkdev(MAJOR_NR, "cf-mips", &cf_bdops);
	if (result < 0) {
		printk(KERN_WARNING "cf-mips: can't get major %d\n", MAJOR_NR);
		return result;
	}

	result = cf_init();
	if (result) {
		unregister_blkdev(MAJOR_NR, "cf-mips");
		return result;
	}

	/* default cfg for all partitions */
	memset(cf_parts, 0, sizeof (cf_parts[0]) * CF_MAX_PART);
	memset(cf_part_sizes, 0, sizeof (cf_part_sizes[0]) * CF_MAX_PART);
	for (i = 0; i < CF_MAX_PART; ++i) {
		cf_hsect_sizes[i] = CF_SECT_SIZE;
		cf_max_sectors[i] = ATA_MAX_SECT_PER_CMD;
		cf_blksize_sizes[i] = BLOCK_SIZE;
	}

	/* setup info for whole disk (partition 0) */
	cf_part_sizes[0] = cf_sectors / 2;
	cf_parts[0].nr_sects = cf_sectors;

	blk_size[MAJOR_NR] = cf_part_sizes;
	blksize_size[MAJOR_NR] = cf_blksize_sizes;
	max_sectors[MAJOR_NR] = cf_max_sectors;
	hardsect_size[MAJOR_NR] = cf_hsect_sizes;
	read_ahead[MAJOR_NR] = 8;	/* (4kB) */

	blk_init_queue(BLK_DEFAULT_QUEUE(MAJOR_NR), DEVICE_REQUEST);

	add_gendisk(&cf_gendisk);
	printk(KERN_INFO "cf-mips partition check: \n");
	register_disk(&cf_gendisk, MKDEV(MAJOR_NR, 0), CF_MAX_PART,
		      &cf_bdops, cf_sectors);

	printk(KERN_INFO "cf-mips module loaded\n");
	return 0;
}

static void __exit
cf_mips_cleanup(void)
{
	int i;

	unregister_blkdev(MAJOR_NR, "cf-mips");
	blk_cleanup_queue(BLK_DEFAULT_QUEUE(MAJOR_NR));

	for (i = 0; i < CF_MAX_PART; ++i) {
		invalidate_device(MKDEV(MAJOR_NR, i), 1);
	}

	blk_size[MAJOR_NR] = NULL;
	blksize_size[MAJOR_NR] = NULL;
	max_sectors[MAJOR_NR] = NULL;
	hardsect_size[MAJOR_NR] = NULL;
	read_ahead[MAJOR_NR] = 0;

	del_gendisk(&cf_gendisk);

	cf_cleanup();
	printk(KERN_INFO "cf-mips module removed\n");
}

module_init(cf_mips_init);
module_exit(cf_mips_cleanup);

MODULE_LICENSE("GPL");
