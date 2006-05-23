/* CF-mips driver
   This is a block driver for the direct (mmaped) interface to the CF-slot,
   found in Routerboard.com's RB532 board
   See SDK provided from routerboard.com.
   
   Module adapted By P.Christeas <p_christeas@yahoo.com>, 2005-6.
   This work is redistributed under the terms of the GNU General Public License.
*/

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

#define DEBUG

#ifdef DEBUG
#define DEBUGP printk
#define DLEVEL 1
#else
#define DEBUGP(format, args...)
#define DLEVEL 0
#endif

#define CF_MIPS_MAJOR 13
#define MAJOR_NR	CF_MIPS_MAJOR
#define CF_MAX_PART	16		/* max 15 partitions */


// #undef DEVICE_REQUEST
// #undef DEVICE_NAME
// #undef DEVICE_NR
// #define DEVICE_REQUEST cf_request
// #define DEVICE_NAME	"xda"
// #define DEVICE_NR(dev)	0		/* one drive only */

#include "ata.h"

//extern struct block_device_operations cf_bdops;

// static struct hd_struct cf_parts[CF_MAX_PART];
// static int cf_part_sizes[CF_MAX_PART];
// static int cf_hsect_sizes[CF_MAX_PART];
// static int cf_max_sectors[CF_MAX_PART];
// static int cf_blksize_sizes[CF_MAX_PART];

// static spinlock_t lock = SPIN_LOCK_UNLOCKED;

// volatile int cf_busy = 0;

struct cf_mips_dev *the_dev = NULL;

static int cf_open (struct inode *, struct file *);
static int cf_release (struct inode *, struct file *);
static int cf_ioctl (struct inode *, struct file *, unsigned, unsigned long);

static void cf_request(request_queue_t * q);
static int cf_transfer(const struct request *req);

/*long (*unlocked_ioctl) (struct file *, unsigned, unsigned long);
long (*compat_ioctl) (struct file *, unsigned, unsigned long);*/
// int (*direct_access) (struct block_device *, sector_t, unsigned long *);
// int (*media_changed) (struct gendisk *);
// int (*revalidate_disk) (struct gendisk *);

static struct block_device_operations cf_bdops = {
      .owner = THIS_MODULE,
      .open = cf_open,
      .release = cf_release,
      .ioctl = cf_ioctl,
      .media_changed = NULL,
      .unlocked_ioctl = NULL,
      .revalidate_disk = NULL,
      .compat_ioctl = NULL,
      .direct_access = NULL
};


int __init cf_mips_init(void)
{
	struct gendisk* cf_gendisk=NULL;
	int reg_result;
	//int i;

	reg_result = register_blkdev(MAJOR_NR, "cf-mips");
	if (reg_result < 0) {
		printk(KERN_WARNING "cf-mips: can't get major %d\n", MAJOR_NR);
		return reg_result;
	}

	the_dev=(struct cf_mips_dev *)kmalloc(sizeof(struct cf_mips_dev),GFP_KERNEL);
	if (!the_dev) goto out_err;
	memset(the_dev,0,sizeof(struct cf_mips_dev));
	
	if (cf_init(the_dev)) goto out_err;
	
	spin_lock_init(&the_dev->lock);
	the_dev->queue = blk_init_queue(cf_request,&the_dev->lock);
	if (!the_dev->queue){
		printk(KERN_ERR "cf-mips: no mem for queue\n");
		goto out_err;
	}
	blk_queue_max_sectors(the_dev->queue,ATA_MAX_SECT_PER_CMD);

// 	blk_queue_max_segment_size(the_dev->queue,the_dev->block_size*512);
	
	/* For memory devices, it is always better to avoid crossing segments
	inside the same request. */
/*	if (the_dev->dtype==0x848A){
		printk(KERN_INFO "Setting boundary for cf to 0x%x",(the_dev->block_size*512)-1);
		blk_queue_segment_boundary(the_dev->queue, (the_dev->block_size*512)-1);
	}*/

	the_dev->gd = alloc_disk(CF_MAX_PART);
	cf_gendisk = the_dev->gd;
	if (!cf_gendisk) goto out_err; /* Last of these goto's */
	
	cf_gendisk->major = MAJOR_NR;
	cf_gendisk->first_minor = 0;
	cf_gendisk->queue=the_dev->queue;
	BUG_ON(cf_gendisk->minors != CF_MAX_PART);
	strcpy(cf_gendisk->disk_name,"cfa");
	cf_gendisk->fops = &cf_bdops;
	cf_gendisk->flags = 0 ; /* is not yet GENHD_FL_REMOVABLE */
	cf_gendisk->private_data=the_dev;
	
	set_capacity(cf_gendisk,the_dev->sectors * CF_KERNEL_MUL);
	
	/* Let the disk go live */
	add_disk(cf_gendisk);
#if 0
	result = cf_init();
	
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
#endif
// 	printk(KERN_INFO "cf-mips partition check: \n");
// 	register_disk(cf_gendisk, MKDEV(MAJOR_NR, 0), CF_MAX_PART,
// 		      &cf_bdops, the_dev->sectors);
	printk(KERN_INFO "cf-mips module loaded\n");
	return 0;

out_err:
	if (the_dev->queue){
		blk_cleanup_queue(the_dev->queue);
	}
	if (reg_result) {
		unregister_blkdev(MAJOR_NR, "cf-mips");
		return reg_result;
	}
	if (the_dev){
		cf_cleanup(the_dev);
		kfree(the_dev);
	}
	return 1;
}

static void __exit
cf_mips_cleanup(void)
{
	//int i;

	unregister_blkdev(MAJOR_NR, "cf-mips");
	blk_cleanup_queue(the_dev->queue);

// 	for (i = 0; i < CF_MAX_PART; ++i) {
// 		invalidate_device(MKDEV(MAJOR_NR, i), 1);
// 	}

	/*blk_size[MAJOR_NR] = NULL;
	blksize_size[MAJOR_NR] = NULL;
	max_sectors[MAJOR_NR] = NULL;
	hardsect_size[MAJOR_NR] = NULL;
	read_ahead[MAJOR_NR] = 0; */

	del_gendisk(the_dev->gd);

	cf_cleanup(the_dev);
	printk(KERN_INFO "cf-mips module removed\n");
}

module_init(cf_mips_init);
module_exit(cf_mips_cleanup);

MODULE_LICENSE("GPL");
MODULE_ALIAS_BLOCKDEV_MAJOR(CF_MIPS_MAJOR);


static int cf_open(struct inode *inode, struct file *filp)
{
	struct cf_mips_dev  *dev=inode->i_bdev->bd_disk->private_data;
	int minor = MINOR(inode->i_rdev);
	BUG_ON(dev!=the_dev);
	
	if (minor >= CF_MAX_PART)
		return -ENODEV;
	//DEBUGP(KERN_INFO "cf-mips module opened, minor %d\n", minor);
	spin_lock(&dev->lock);
	dev->users++;
	spin_unlock(&dev->lock);
	filp->private_data=dev;
	/* dirty workaround to set CFRDY GPIO as an input when some other
	   program sets it as an output */
	*(volatile unsigned *)(0xb8050004) &= ~(1 << 13);
	return 0;		/* success */
}

static int cf_release(struct inode *inode, struct file *filp)
{
	int minor = MINOR(inode->i_rdev);
	struct cf_mips_dev  *dev=inode->i_bdev->bd_disk->private_data;
	BUG_ON(dev!=the_dev);
	//DEBUGP(KERN_INFO "cf-mips module closed, minor %d\n", minor);
	spin_lock(&dev->lock);
	dev->users--;
	spin_unlock(&dev->lock);
	return 0;
}

static int cf_ioctl(struct inode *inode, struct file *filp,
	 unsigned int cmd, unsigned long arg)
{
	unsigned minor = MINOR(inode->i_rdev);
	struct cf_mips_dev  *dev=inode->i_bdev->bd_disk->private_data;
	BUG_ON(dev!=the_dev);

	DEBUGP(KERN_INFO "cf_ioctl cmd %u\n", cmd);
	switch (cmd) {
	case BLKRRPART:	/* re-read partition table */
		if (!capable(CAP_SYS_ADMIN))
			return -EACCES;
		printk(KERN_INFO "cf-mips partition check: \n");
		register_disk(dev->gd);
		return 0;

	case HDIO_GETGEO:
		{
			struct hd_geometry geo;
			geo.cylinders = dev->cyl;
			geo.heads = dev->head;
			geo.sectors = dev->spt;
			geo.start = (*dev->gd->part)[minor].start_sect;
			if (copy_to_user((void *) arg, &geo, sizeof (geo)))
				return -EFAULT;
		}
		return 0;
	}

	return -ENOTTY;		/* unknown command */
}

static void cf_request(request_queue_t * q)
{
	struct cf_mips_dev* dev;
	
	struct request * req;
	int status;

	/* We could have q->queuedata = dev , but haven't yet. */
	if (the_dev->active_req)
		return;

	while ((req=elv_next_request(q))!=NULL){
		dev=req->rq_disk->private_data;
		BUG_ON(dev!=the_dev);
		status=cf_transfer(req);
		if (status==CF_TRANS_IN_PROGRESS){
			dev->active_req=req;
			return;
		}
		end_request(req,status);
	}
}

static int cf_transfer(const struct request *req)
{
	struct cf_mips_dev* dev=req->rq_disk->private_data;
	BUG_ON(dev!=the_dev);

// 	if (printk_ratelimit())
// 	DEBUGP(KERN_INFO "cf_transfer dev at 0x%lx size %ld rw 0x%x\n", req->sector, req->nr_sectors, req->cmd[0]);

	if (!blk_fs_request(req)){	
		if (printk_ratelimit())
			printk(KERN_WARNING "cf-mips: skipping non-fs request 0x%x\n",req->cmd[0]);
		return CF_TRANS_FAILED;
	}
	
	return cf_do_transfer(dev,req->sector,req->current_nr_sectors,req->buffer,rq_data_dir(req));

}

#if 0 /* that is so 2.4 .. */
void * cf_get_next_buf(unsigned *buf_size)
{
	struct request *req = CURRENT;

	if (!end_that_request_first(req, CF_TRANS_OK, DEVICE_NAME))
		BUG();

	*buf_size = req->bh->b_size;
	return req->bh->b_data;
}
#endif

void cf_async_trans_done(struct cf_mips_dev * dev,int result)
{
	struct request *req;
	BUG_ON(dev!=the_dev);
	spin_lock(&dev->lock);
	req=dev->active_req;
	dev->active_req=NULL;
	end_request(req,result);
	spin_unlock(&dev->lock);

	{
		/*DEBUGP("cf_mips: queue next");*/
		spin_lock(&dev->lock);
		cf_request(dev->queue);
		spin_unlock(&dev->lock);
	}
}

/*eof */
