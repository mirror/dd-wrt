/*
 * devpath.c
 * (C) Copyright 2008 Jiri Engelthaler
 *
 * $id$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *************************************************************
 *
 * 2008-07-25: Jiri Engelthaler (engy@centrum.cz)
 *
 * $Id: devpath.c,v 1.3 2000/01/11 13:58:24 tom Exp $
 */

#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/usb.h>
#include <linux/usbdevice_fs.h>
#include <asm/uaccess.h>
#include <linux/module.h>

LIST_HEAD(usb_devpath_list);
struct semaphore usb_devpath_list_lock;

struct usb_devpath {
  struct list_head devpath_list;
  struct usb_device *dev;
  char index;
  char devfs[16];
};

void usb_register_devpath(struct usb_device *dev, char index, char *devfs)
{
	struct usb_devpath *newdevpath;
	
	down(&usb_devpath_list_lock);
	newdevpath = kmalloc(sizeof(*newdevpath), GFP_KERNEL);
	newdevpath->dev = dev;
	newdevpath->index = index;
	strcpy(newdevpath->devfs, devfs);
	list_add_tail(&newdevpath->devpath_list, &usb_devpath_list);
	up(&usb_devpath_list_lock);
}

void usb_deregister_devpath(struct usb_device *dev)
{
	struct list_head *tmp;

	down(&usb_devpath_list_lock);
	for (tmp = usb_devpath_list.next; tmp != &usb_devpath_list; tmp = tmp->next) {
		struct usb_devpath *devpath = list_entry(tmp, struct usb_devpath, devpath_list);
		if (devpath->dev == dev)
		{
			tmp = tmp->prev;
			list_del(&devpath->devpath_list);
			kfree(devpath);
		}
	}	
	up(&usb_devpath_list_lock);
}

/*****************************************************************/

/*
 * Dump usb_devpath_list.
 *
 * We now walk the list of registered USB device paths.
 */
static ssize_t usb_devpath_read(struct file *file, char *buf, size_t nbytes, loff_t *ppos)
{
	struct list_head *tmp;
	char *page, *start, *end;
	ssize_t ret = 0;
	loff_t n = *ppos;
	unsigned int pos = n, len;
	char  *infobuf;
	
	if (pos != n)
		return -EINVAL;
	if (nbytes <= 0)
		return 0;
	if (!access_ok(VERIFY_WRITE, buf, nbytes))
		return -EFAULT;
        if (!(page = (char*) __get_free_page(GFP_KERNEL)))
                return -ENOMEM;
	start = page;
	end = page + (PAGE_SIZE - 100);

	infobuf = kmalloc(128, GFP_KERNEL);

	start += sprintf(start, "Device         : Location : Info\n");
	
	down(&usb_devpath_list_lock);
	
	for (tmp = usb_devpath_list.next; tmp != &usb_devpath_list; tmp = tmp->next) {
		struct usb_devpath *devpath = list_entry(tmp, struct usb_devpath, devpath_list);
		
		start += sprintf(start, "/dev/%s : %s.%d", devpath->devfs, devpath->dev->devpath, devpath->index);
		
		if (devpath->dev->descriptor.iManufacturer) {
			if (usb_string(devpath->dev, devpath->dev->descriptor.iManufacturer, infobuf, 128) > 0)
				start += sprintf(start, " : Manufacturer=\"%.100s\"", infobuf);
		}
		
		if (devpath->dev->descriptor.iProduct) {
			if (usb_string(devpath->dev, devpath->dev->descriptor.iProduct, infobuf, 128) > 0)
				start += sprintf(start, " : Product=\"%.100s\"", infobuf);
		}
#ifdef ALLOW_SERIAL_NUMBER
		if (devpath->dev->descriptor.iSerialNumber) {
			if (usb_string(devpath->dev, devpath->dev->descriptor.iSerialNumber, infobuf, 128) > 0)
				start += sprintf(start, " : SerialNumber=\"%.100s\"", infobuf);
		}
#endif
		start += sprintf(start, "\n");
/*		int minor = driver->fops ? driver->minor : -1;
		if (minor == -1)
			start += sprintf (start, "         %s\n", driver->name);
		else
			start += sprintf (start, "%3d-%3d: %s\n", minor, minor + 15, driver->name);*/
		if (start > end) {
			start += sprintf(start, "(truncated)\n");
			break;
		}
	}
	if (start == page)
		start += sprintf(start, "(none)\n");
	len = start - page;
	if (len > pos) {
		len -= pos;
		if (len > nbytes)
			len = nbytes;
		ret = len;
		if (copy_to_user(buf, page + pos, len))
			ret = -EFAULT;
		else
			*ppos = pos + len;
	}
	
	up(&usb_devpath_list_lock);
	
	kfree(infobuf);
	free_page((unsigned long)page);
	return ret;
}

static loff_t usb_devpath_lseek(struct file * file, loff_t offset, int orig)
{
	switch (orig) {
	case 0:
		file->f_pos = offset;
		return file->f_pos;

	case 1:
		file->f_pos += offset;
		return file->f_pos;

	case 2:
		return -EINVAL;

	default:
		return -EINVAL;
	}
}

struct file_operations usbdevfs_devpath_fops = {
	llseek:		usb_devpath_lseek,
	read:		usb_devpath_read,
};

EXPORT_SYMBOL(usb_register_devpath);
EXPORT_SYMBOL(usb_deregister_devpath);
