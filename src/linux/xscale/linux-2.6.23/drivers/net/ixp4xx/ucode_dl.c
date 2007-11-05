/*
 * ucode_dl.c - provide an NPE device and a char-dev for microcode download
 *
 * Copyright (C) 2006 Christian Hohnstaedt <chohnstaedt@innominate.com>
 *
 * This file is released under the GPLv2
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/firmware.h>
#include <linux/dma-mapping.h>
#include <linux/byteorder/swab.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include <linux/ixp_npe.h>

#define IXNPE_VERSION "IXP4XX NPE driver Version 0.3.0"

#define DL_MAGIC 0xfeedf00d
#define DL_MAGIC_SWAP 0x0df0edfe

#define EOF_BLOCK 0xf
#define IMG_SIZE(image) (((image)->size * sizeof(u32)) + \
		sizeof(struct dl_image))

#define BT_INSTR 0
#define BT_DATA 1

enum blk_type {
	instruction,
	data,
};

struct dl_block {
	u32 type;
	u32 offset;
};

struct dl_image {
	u32 magic;
	u32 id;
	u32 size;
	union {
		u32 data[0];
		struct dl_block block[0];
	} u;
};

struct dl_codeblock {
	u32 npe_addr;
	u32 size;
	u32 data[0];
};

static struct platform_driver ixp4xx_npe_driver;

static int match_by_npeid(struct device *dev, void *id)
{
	struct npe_info *npe = dev_get_drvdata(dev);
	if (!npe->plat)
		return 0;
	return (npe->plat->id == *(int*)id);
}

struct device *get_npe_by_id(int id)
{
	struct device *dev = driver_find_device(&ixp4xx_npe_driver.driver,
			NULL, &id, match_by_npeid);
	if (dev) {
		struct npe_info *npe = dev_get_drvdata(dev);
		if (!try_module_get(THIS_MODULE)) {
			 put_device(dev);
			 return NULL;
		}
		npe->usage++;
	}
	return dev;
}

void return_npe_dev(struct device *dev)
{
	struct npe_info *npe = dev_get_drvdata(dev);
	put_device(dev);
	module_put(THIS_MODULE);
	npe->usage--;
}

static int
download_block(struct npe_info *npe, struct dl_codeblock *cb, unsigned type)
{
	int i;
	int cmd;

	switch (type) {
	case BT_DATA:
		cmd = IX_NPEDL_EXCTL_CMD_WR_DATA_MEM;
		if (cb->npe_addr + cb->size > npe->plat->data_size) {
			printk(KERN_INFO "Data size too large: %d+%d > %d\n",
				cb->npe_addr, cb->size, npe->plat->data_size);
			return -EIO;
		}
		break;
	case BT_INSTR:
		cmd = IX_NPEDL_EXCTL_CMD_WR_INS_MEM;
		if (cb->npe_addr + cb->size > npe->plat->inst_size) {
			printk(KERN_INFO "Instr size too large: %d+%d > %d\n",
				cb->npe_addr, cb->size, npe->plat->inst_size);
			return -EIO;
		}
		break;
	default:
		printk(KERN_INFO "Unknown CMD: %d\n", type);
		return -EIO;
	}

	for (i=0; i < cb->size; i++) {
		npe_write_cmd(npe, cb->npe_addr + i, cb->data[i], cmd);
	}

	return 0;
}

static int store_npe_image(struct dl_image *image, struct device *dev)
{
	struct dl_block *blk;
	struct dl_codeblock *cb;
	struct npe_info *npe;
	int ret=0;

	if (!dev) {
		dev = get_npe_by_id( (image->id >> 24) & 0xf);
		return_npe_dev(dev);
	}
	if (!dev)
		return -ENODEV;

	npe = dev_get_drvdata(dev);
	if (npe->loaded && (npe->usage > 0)) {
		printk(KERN_INFO "Cowardly refusing to reload an Image "
			"into the used and running %s\n", npe->plat->name);
		return 0; /* indicate success anyway... */
	}
	if (!cpu_is_ixp46x() && ((image->id >> 28) & 0xf)) {
		printk(KERN_INFO "IXP46x NPE image ignored on IXP42x\n");
		return -EIO;
	}

	npe_stop(npe);
	npe_reset(npe);

	for (blk = image->u.block; blk->type != EOF_BLOCK; blk++) {
		if (blk->offset > image->size) {
			printk(KERN_INFO "Block offset out of range\n");
			return -EIO;
		}
		cb = (struct dl_codeblock*)&image->u.data[blk->offset];
		if (blk->offset + cb->size + 2 > image->size) {
			printk(KERN_INFO "Codeblock size out of range\n");
			return -EIO;
		}
		if ((ret = download_block(npe, cb, blk->type)))
			return ret;
	}
	*(u32*)npe->img_info = cpu_to_be32(image->id);
	npe_start(npe);

	printk(KERN_INFO "Image loaded to %s Func:%x, Rel: %x:%x, Status: %x\n",
			npe->plat->name, npe->img_info[1], npe->img_info[2],
			npe->img_info[3], npe_status(npe));
	if (npe_mh_status(npe)) {
		printk(KERN_ERR "%s not responding\n", npe->plat->name);
	}
	npe->loaded = 1;
	return 0;
}

static int ucode_open(struct inode *inode, struct file *file)
{
	file->private_data = kmalloc(sizeof(struct dl_image), GFP_KERNEL);
	if (!file->private_data)
		return -ENOMEM;
	return 0;
}

static int ucode_close(struct inode *inode, struct file *file)
{
	kfree(file->private_data);
	return 0;
}

static ssize_t ucode_write(struct file *file, const char __user *buf,
		size_t count, loff_t *ppos)
{
	union {
		char *data;
		struct dl_image *image;
	} u;
	const char __user *cbuf = buf;

	u.data = file->private_data;

	while (count) {
		int len;
		if (*ppos < sizeof(struct dl_image)) {
			len = sizeof(struct dl_image) - *ppos;
			len = len > count ? count : len;
			if (copy_from_user(u.data + *ppos, cbuf, len))
				return -EFAULT;
			count -= len;
			*ppos += len;
			cbuf += len;
			continue;
		} else if (*ppos == sizeof(struct dl_image)) {
			void *data;
			if (u.image->magic == DL_MAGIC_SWAP) {
				printk(KERN_INFO "swapped image found\n");
				u.image->id = swab32(u.image->id);
				u.image->size = swab32(u.image->size);
			} else if (u.image->magic != DL_MAGIC) {
				printk(KERN_INFO "Bad magic:%x\n",
						u.image->magic);
				return -EFAULT;
			}
			len = IMG_SIZE(u.image);
			data = kmalloc(len, GFP_KERNEL);
			if (!data)
				return -ENOMEM;
			memcpy(data, u.data, *ppos);
			kfree(u.data);
			u.data = (char*)data;
			file->private_data = data;
		}
		len = IMG_SIZE(u.image) - *ppos;
		len = len > count ? count : len;
		if (copy_from_user(u.data + *ppos, cbuf, len))
			return -EFAULT;
		count -= len;
		*ppos += len;
		cbuf += len;
		if (*ppos == IMG_SIZE(u.image)) {
			int ret, i;
			*ppos = 0;
			if (u.image->magic == DL_MAGIC_SWAP) {
				for (i=0; i<u.image->size; i++) {
					u.image->u.data[i] =
						swab32(u.image->u.data[i]);
				}
				u.image->magic = swab32(u.image->magic);
			}
			ret = store_npe_image(u.image, NULL);
			if (ret) {
				printk(KERN_INFO "Error in NPE image: %x\n",
					u.image->id);
				return ret;
			}
		}
	}
	return (cbuf-buf);
}

static void npe_firmware_probe(struct device *dev)
{
#if (defined(CONFIG_FW_LOADER) || defined(CONFIG_FW_LOADER_MODULE)) \
	&& defined(MODULE)
	const struct firmware *fw_entry;
	struct npe_info *npe = dev_get_drvdata(dev);
	struct dl_image *image;
	int ret = -1, i;

	if (request_firmware(&fw_entry, npe->plat->name, dev) != 0) {
		return;
	}
	image = (struct dl_image*)fw_entry->data;
	/* Sanity checks */
	if (fw_entry->size < sizeof(struct dl_image)) {
		printk(KERN_ERR "Firmware error: too small\n");
		goto out;
	}
	if (image->magic == DL_MAGIC_SWAP) {
		printk(KERN_INFO "swapped image found\n");
		image->id = swab32(image->id);
		image->size = swab32(image->size);
	} else if (image->magic != DL_MAGIC) {
		printk(KERN_ERR "Bad magic:%x\n", image->magic);
		goto out;
	}
	if (IMG_SIZE(image) != fw_entry->size) {
		printk(KERN_ERR "Firmware error: bad size\n");
		goto out;
	}
	if (((image->id >> 24) & 0xf) != npe->plat->id) {
		printk(KERN_ERR "NPE id missmatch\n");
		goto out;
	}
	if (image->magic == DL_MAGIC_SWAP) {
		for (i=0; i<image->size; i++) {
			image->u.data[i] = swab32(image->u.data[i]);
		}
		image->magic = swab32(image->magic);
	}

	ret = store_npe_image(image, dev);
out:
	if (ret) {
		printk(KERN_ERR "Error downloading Firmware for %s\n",
				npe->plat->name);
	}
	release_firmware(fw_entry);
#endif
}

static void disable_npe_irq(struct npe_info *npe)
{
	u32 reg;
	reg = npe_reg_read(npe, IX_NPEDL_REG_OFFSET_CTL);
	reg &= ~(IX_NPEMH_NPE_CTL_OFE | IX_NPEMH_NPE_CTL_IFE);
	reg |= IX_NPEMH_NPE_CTL_OFEWE | IX_NPEMH_NPE_CTL_IFEWE;
	npe_reg_write(npe, IX_NPEDL_REG_OFFSET_CTL, reg);
}

static ssize_t show_npe_state(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct npe_info *npe = dev_get_drvdata(dev);

	strcpy(buf, npe_status(npe) & IX_NPEDL_EXCTL_STATUS_RUN ?
			"start\n" : "stop\n");
	return strlen(buf);
}

static ssize_t set_npe_state(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct npe_info *npe = dev_get_drvdata(dev);

	if (npe->usage) {
		printk("%s in use: read-only\n", npe->plat->name);
		return count;
	}
	if (!strncmp(buf, "start", 5)) {
		npe_start(npe);
	}
	if (!strncmp(buf, "stop", 4)) {
		npe_stop(npe);
	}
	if (!strncmp(buf, "reset", 5)) {
		npe_stop(npe);
		npe_reset(npe);
	}
	return count;
}

static DEVICE_ATTR(state, S_IRUGO | S_IWUSR, show_npe_state, set_npe_state);

static int npe_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct npe_info *npe;
	struct npe_plat_data *plat = pdev->dev.platform_data;
	int err, size, ret=0;

	if (!(res = platform_get_resource(pdev, IORESOURCE_MEM, 0)))
		return -EIO;

	if (!(npe = kzalloc(sizeof(struct npe_info), GFP_KERNEL)))
		return -ENOMEM;

	size = res->end - res->start +1;
	npe->res = request_mem_region(res->start, size, plat->name);
	if (!npe->res) {
		ret = -EBUSY;
		printk(KERN_ERR "Failed to get memregion(%x, %x)\n",
				res->start, size);
		goto out_free;
	}

	npe->addr = ioremap(res->start, size);
	if (!npe->addr) {
		ret = -ENOMEM;
		printk(KERN_ERR "Failed to ioremap(%x, %x)\n",
				res->start, size);
		goto out_rel;
	}

	pdev->dev.coherent_dma_mask = DMA_32BIT_MASK;

	platform_set_drvdata(pdev, npe);

	err = device_create_file(&pdev->dev, &dev_attr_state);
	if (err)
		goto out_rel;

	npe->plat = plat;
	disable_npe_irq(npe);
	npe->usage = 0;
	npe_reset(npe);
	npe_firmware_probe(&pdev->dev);

	return 0;

out_rel:
	release_resource(npe->res);
out_free:
	kfree(npe);
	return ret;
}

static struct file_operations ucode_dl_fops = {
	.owner		= THIS_MODULE,
	.write		= ucode_write,
	.open		= ucode_open,
	.release	= ucode_close,
};

static struct miscdevice ucode_dl_dev = {
	.minor	= MICROCODE_MINOR,
	.name	= "ixp4xx_ucode",
	.fops	= &ucode_dl_fops,
};

static int npe_remove(struct platform_device *pdev)
{
	struct npe_info *npe = platform_get_drvdata(pdev);

	device_remove_file(&pdev->dev, &dev_attr_state);

	iounmap(npe->addr);
	release_resource(npe->res);
	kfree(npe);
	return 0;
}

static struct platform_driver ixp4xx_npe_driver = {
	.driver = {
		.name	= "ixp4xx_npe",
		.owner	= THIS_MODULE,
	},
	.probe	= npe_probe,
	.remove	= npe_remove,
};

static int __init init_npedriver(void)
{
	int ret;
	if ((ret = misc_register(&ucode_dl_dev))){
		printk(KERN_ERR "Failed to register misc device %d\n",
				MICROCODE_MINOR);
		return ret;
	}
	if ((ret = platform_driver_register(&ixp4xx_npe_driver)))
		misc_deregister(&ucode_dl_dev);
	else
		printk(KERN_INFO IXNPE_VERSION " initialized\n");

	return ret;

}

static void __exit finish_npedriver(void)
{
	misc_deregister(&ucode_dl_dev);
	platform_driver_unregister(&ixp4xx_npe_driver);
}

module_init(init_npedriver);
module_exit(finish_npedriver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Christian Hohnstaedt <chohnstaedt@innominate.com>");

EXPORT_SYMBOL(get_npe_by_id);
EXPORT_SYMBOL(return_npe_dev);
