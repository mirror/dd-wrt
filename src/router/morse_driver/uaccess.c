/*
 * Copyright 2017-2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>

/* Char device */
#include <linux/cdev.h>
#include <linux/fs.h>
#include "debug.h"
#include "uaccess.h"
#include "bus.h"

#define MORSE_DEV_NAME			"morse"
#define MORSE_DEV_FILE			MORSE_DEV_NAME "_io"
#define MORSE_NUM_OF_UACCESS_DEVICES	4
#define MORSE_DEV_PERMISSIONS		0666
#define UACCESS_BUFFER_SIZE		((size_t)(64 * 512))

struct uaccess_file_descriptor {
	struct morse *mors;
	u8 *data;
	u32 address;
	/* Serialise read and write access */
	struct mutex lock;
};

int uaccess_major;
int uaccess_minor;
int uaccess_nr_devs = MORSE_NUM_OF_UACCESS_DEVICES;

/*
 * Open and release
 */
static int uaccess_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	struct uaccess_device *dev = NULL;	/*  device information */
	struct uaccess_file_descriptor *des = NULL;

	dev = container_of(inode->i_cdev, struct uaccess_device, cdev);
	des = kzalloc(sizeof(*des), GFP_KERNEL);
	if (!des) {
		ret = -ENOMEM;
		goto exit;
	}
	des->data = kmalloc(UACCESS_BUFFER_SIZE, GFP_KERNEL);
	if (!des->data) {
		ret = -ENOMEM;
		goto free_desc;
	}
	mutex_init(&des->lock);
	des->mors = dev->mors;
	filp->private_data = des;
	goto exit;

free_desc:
	kfree(des);
exit:
	return ret;
}

static int uaccess_release(struct inode *inode, struct file *filp)
{
	int ret = 0;
	struct uaccess_file_descriptor *des = filp->private_data;

	kfree(des->data);
	mutex_destroy(&des->lock);
	kfree(des);
	return ret;
}

/*
 * Data management: read and write
 */
static ssize_t uaccess_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	ssize_t ret = 0;
	struct uaccess_file_descriptor *des = filp->private_data;

	if (mutex_lock_interruptible(&des->lock))
		return -ERESTARTSYS;
	if (copy_from_user(des->data, buf, count)) {
		MORSE_PR_ERR(FEATURE_ID_DEFAULT, "copy_from_user failed\n");
		ret = -EFAULT;
	} else {
		count = min(count, UACCESS_BUFFER_SIZE);

		morse_claim_bus(des->mors);
		if (count == sizeof(u32))
			ret = morse_reg32_write(des->mors, des->address, *((u32 *)des->data));
		else
			ret = morse_dm_write(des->mors, des->address, (u8 *)des->data, count);

		morse_release_bus(des->mors);

		if (ret < 0) {
			MORSE_PR_ERR(FEATURE_ID_DEFAULT,
				     "write failed (errno=%zu, address=0x%04X, length=%zu bytes)\n",
				     ret, des->address, count);
			ret = -EFAULT;
		} else {
			ret = count;
		}
	}
	mutex_unlock(&des->lock);
	return ret;
}

static ssize_t uaccess_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	ssize_t ret = 0;
	struct uaccess_file_descriptor *des = filp->private_data;

	if (mutex_lock_interruptible(&des->lock))
		return -ERESTARTSYS;
	count = min(count, UACCESS_BUFFER_SIZE);

	morse_claim_bus(des->mors);
	if (count == sizeof(u32))
		ret = morse_reg32_read(des->mors, des->address, (u32 *)des->data);
	else
		ret = morse_dm_read(des->mors, des->address, (u8 *)des->data, count);
	morse_release_bus(des->mors);

	if (ret < 0) {
		MORSE_PR_ERR(FEATURE_ID_DEFAULT,
			     "read failed (errno=%zu, address=0x%04X, length=%zu bytes)\n",
			     ret, des->address, count);
		ret = -EFAULT;
	} else {
		ret = count;
		if (copy_to_user(buf, des->data, ret))
			ret = -EFAULT;
	}

	mutex_unlock(&des->lock);
	return ret;
}

/*
 * The ioctl() implementation
 */
static long uaccess_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int err = 0;
	int ret = 0;
	struct uaccess_file_descriptor *des = filp->private_data;

	/* Extract the type and number bitfields.
	 * Sanity check: return ENOTTY (inappropriate ioctl) before access_ok().
	 */
	if ((_IOC_TYPE(cmd) != UACCESS_IOC_MAGIC) || (_IOC_NR(cmd) > UACCESS_IOC_MAXNR)) {
		MORSE_PR_ERR(FEATURE_ID_DEFAULT, "Wrong ioctl command parameters\n");
		ret = -ENOTTY;
		goto exit;
	}

	/* The direction is a bitmask, and VERIFY_WRITE catches R/W transfers.
	 * `Type' is user-oriented while access_ok is kernel-oriented, so the concept of 'read'
	 * and 'write' is reversed.
	 */
	if (_IOC_DIR(cmd) & _IOC_READ) {
#if KERNEL_VERSION(5, 0, 0) > LINUX_VERSION_CODE
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
#else
		err = !access_ok((void __user *)arg, _IOC_SIZE(cmd));
#endif
	} else {
		if (_IOC_DIR(cmd) & _IOC_WRITE)
#if KERNEL_VERSION(5, 0, 0) > LINUX_VERSION_CODE
			err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
#else
			err =  !access_ok((void __user *)arg, _IOC_SIZE(cmd));
#endif
	}

	if (err) {
		MORSE_PR_ERR(FEATURE_ID_DEFAULT, "Wrong ioctl access direction\n");
		ret = -EFAULT;
		goto exit;
	}

	if (mutex_lock_interruptible(&des->lock))
		return -ERESTARTSYS;

	switch (cmd) {
	case UACCESS_IOC_SET_ADDRESS:
		des->address = (u32)arg;
		break;
	default:		/*  redundant, as cmd was checked against MAXNR */
		MORSE_PR_WARN(FEATURE_ID_DEFAULT, "Redundant IOCTL\n");
		ret = -ENOTTY;
	}

	mutex_unlock(&des->lock);
exit:
	return ret;
}

static const struct file_operations uaccess_fops = {
	.owner = THIS_MODULE,
	.read = uaccess_read,
	.write = uaccess_write,
	.unlocked_ioctl = uaccess_ioctl,
	.open = uaccess_open,
	.release = uaccess_release,
};

int uaccess_device_register(struct morse *mors, struct uaccess *uaccess, struct device *parent)
{
	int ret = 0;
	struct uaccess_device *dev = &mors->udev;
	struct cdev *char_dev = &dev->cdev;
	struct device *new_device = NULL;
	dev_t devno = MKDEV(uaccess_major, uaccess_minor);

	/*  Initialize uaccess device */
	cdev_init(char_dev, &uaccess_fops);
	char_dev->owner = THIS_MODULE;
	ret = cdev_add(char_dev, devno, 1);

	/*  Fail gracefully if need be */
	if (ret) {
		MORSE_PR_ERR(FEATURE_ID_DEFAULT,
			     "Error %d adding user access device '%s'", ret, MORSE_DEV_FILE);
		goto exit;
	}

	/*  create a /dev entry for uaccess drivers */
	new_device = device_create(uaccess->drv_class, parent, devno, NULL, "%s", MORSE_DEV_FILE);
	if (!new_device) {
		MORSE_PR_ERR(FEATURE_ID_DEFAULT,
			     "Can't create device node '/dev/%s'\n", MORSE_DEV_FILE);
		goto cleanup;
	}

	/* TODO: Allocate communication memory */
	dev->uaccess = uaccess;
	dev->device = new_device;
	dev->mors = mors;

	pr_info("%s: Device node '/dev/%s' created successfully\n", MORSE_DEV_FILE, MORSE_DEV_FILE);
	goto exit;
cleanup:
	dev->device = NULL;
	cdev_del(char_dev);
exit:
	return ret;
}

void uaccess_device_unregister(struct morse *mors)
{
	struct uaccess_device *dev = (struct uaccess_device *)&mors->udev;
	struct uaccess *uaccess = dev->uaccess;

	if (dev->device) {
		int devno = MKDEV(uaccess_major, uaccess_minor);

		device_destroy(uaccess->drv_class, devno);
		dev->device = NULL;
		cdev_del(&dev->cdev);
	}
}

struct uaccess *uaccess_alloc(void)
{
	struct uaccess *uaccess;

	uaccess = kzalloc(sizeof(*uaccess), GFP_KERNEL);
	if (!uaccess)
		return ERR_PTR(-ENOMEM);

	return uaccess;
}

int uaccess_init(struct uaccess *uaccess)
{
	int ret = 0;
	dev_t devno = 0;

	/*  Get a range of minor numbers to work with, asking for a dynamic */
	/*  major unless directed otherwise at load time. */
	if (uaccess_major) {
		devno = MKDEV(uaccess_major, uaccess_minor);
		ret = register_chrdev_region(devno, uaccess_nr_devs, MORSE_DEV_NAME);
	} else {
		ret = alloc_chrdev_region(&devno, uaccess_minor, uaccess_nr_devs, MORSE_DEV_NAME);
		uaccess_major = MAJOR(devno);
	}
	if (ret < 0) {
		MORSE_PR_ERR(FEATURE_ID_DEFAULT, "uaccess can't get major %d\n", uaccess_major);
		goto exit;
	}
	pr_info("uaccess char driver major number is %d\n", uaccess_major);

	/*  prepare create /dev/... instance */
#if KERNEL_VERSION(6, 4, 0) <= LINUX_VERSION_CODE
	uaccess->drv_class = class_create(MORSE_DEV_NAME);
#else
	uaccess->drv_class = class_create(THIS_MODULE, MORSE_DEV_NAME);
#endif
	if (IS_ERR(uaccess->drv_class)) {
		ret = -ENOMEM;
		MORSE_PR_ERR(FEATURE_ID_DEFAULT, MORSE_DEV_NAME " class_create failed\n");
		goto unregister_region;
	}
	return 0;
unregister_region:
	unregister_chrdev_region(devno, uaccess_nr_devs);
exit:
	return ret;
}

void uaccess_cleanup(struct uaccess *uaccess)
{
	if (uaccess && !IS_ERR(uaccess->drv_class)) {
		/* We have a valid init */
		dev_t devno = MKDEV(uaccess_major, uaccess_minor);

		class_destroy(uaccess->drv_class);
		unregister_chrdev_region(devno, uaccess_nr_devs);
	}
	kfree(uaccess);
}
