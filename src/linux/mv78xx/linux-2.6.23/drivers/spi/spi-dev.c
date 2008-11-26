/* 
    spi-dev.c - spi-bus driver, char device interface   
 
    Copyright (C) 2005 Barnabas Kalman <barnik@sednet.hu> 
 
    This program is free software; you can redistribute it and/or modify 
    it under the terms of the GNU General Public License as published by 
    the Free Software Foundation; either version 2 of the License, or 
    (at your option) any later version. 
 
    This program is distributed in the hope that it will be useful, 
    but WITHOUT ANY WARRANTY; without even the implied warranty of 
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
    GNU General Public License for more details. 
 
    You should have received a copy of the GNU General Public License 
    along with this program; if not, write to the Free Software 
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 
*/ 
 
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/devfs_fs_kernel.h>
#include <linux/init.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi-dev.h>
#include <asm/uaccess.h>

static struct spi_client spidev_client_template;

struct spi_dev {
        int minor;
	struct spi_adapter *adap;
	struct class_device class_dev;
	struct completion released;     /* FIXME, we need a class_device_unregister */
};
#define to_spi_dev(d) container_of(d, struct spi_dev, class_dev)
				
#define SPI_MINORS 32
static struct spi_dev *spi_dev_array[SPI_MINORS];
static DEFINE_SPINLOCK(spi_dev_array_lock);
				
static struct spi_dev *spi_dev_get_by_minor(unsigned index)
{
	struct spi_dev *spi_dev;

	spin_lock(&spi_dev_array_lock);
	spi_dev = spi_dev_array[index];
	spin_unlock(&spi_dev_array_lock);
	return spi_dev;
}

static struct spi_dev *spi_dev_get_by_adapter(struct spi_adapter *adap)
{
        struct spi_dev *spi_dev;
	
    spin_lock(&spi_dev_array_lock);
    if ((spi_dev_array[adap->nr]) && (spi_dev_array[adap->nr]->adap == adap))
	spi_dev = spi_dev_array[adap->nr];
    else
	spi_dev = NULL;
    spin_unlock(&spi_dev_array_lock);
    return spi_dev;
}
								    
static struct spi_dev *get_free_spi_dev(struct spi_adapter *adap)
{
    struct spi_dev *spi_dev;
	
    spi_dev = kmalloc(sizeof(*spi_dev), GFP_KERNEL);
    if (!spi_dev)
	return ERR_PTR(-ENOMEM);
    memset(spi_dev, 0x00, sizeof(*spi_dev));

    spin_lock(&spi_dev_array_lock);
    if (spi_dev_array[adap->nr]) {
	spin_unlock(&spi_dev_array_lock);
	dev_err(&adap->dev, "spi-dev already has a device assigned to this adapter\n");
	goto error;
    }
    spi_dev->minor = adap->nr;
    spi_dev_array[adap->nr] = spi_dev;
    spin_unlock(&spi_dev_array_lock);
    return spi_dev;
error:
    kfree(spi_dev);
    return ERR_PTR(-ENODEV);
}

static void return_spi_dev(struct spi_dev *spi_dev)
{
    spin_lock(&spi_dev_array_lock);
    spi_dev_array[spi_dev->minor] = NULL;
    spin_unlock(&spi_dev_array_lock);
}

static ssize_t show_adapter_name(struct class_device *class_dev, char *buf)
{
    struct spi_dev *spi_dev = to_spi_dev(class_dev);
    return sprintf(buf, "%s\n", spi_dev->adap->name);
}

static CLASS_DEVICE_ATTR(name, S_IRUGO, show_adapter_name, NULL);
																																		    									 
static ssize_t spidev_read (struct file *file, char __user *buf, size_t count,
                            loff_t *offset)
{
    char *tmp;
    int ret;
    struct spi_client *client = (struct spi_client *)file->private_data;

    if (count > 8192)
	count = 8192;

    tmp = kmalloc(count,GFP_KERNEL);
    if (tmp==NULL)
	return -ENOMEM;

    pr_debug("spi-dev: spi-%d reading %zd bytes.\n",
				iminor(file->f_dentry->d_inode), count);

    ret = spi_master_recv(client,tmp,count);
    if (ret >= 0)
	ret = copy_to_user(buf,tmp,count)?-EFAULT:ret;
    kfree(tmp);
    return ret;
}
																						    
static ssize_t spidev_write (struct file *file, const char __user *buf,
			     size_t count, loff_t *offset)
{
    int ret;
    char *tmp;
    struct spi_client *client = (struct spi_client *)file->private_data;

    if (count > 8192)
	count = 8192;

    tmp = kmalloc(count,GFP_KERNEL);
    if (tmp==NULL)
	return -ENOMEM;
    if (copy_from_user(tmp,buf,count)) {
	kfree(tmp);
	return -EFAULT;
    }

    pr_debug("spi-dev: spi-%d writing %zd bytes.\n",
				iminor(file->f_dentry->d_inode), count);

    ret = spi_master_send(client,tmp,count);
    kfree(tmp);
    return ret;
}
																									      
static int spidev_ioctl(struct inode *inode, struct file *file,
                unsigned int cmd, unsigned long arg)
{
    struct spi_client *client = (struct spi_client *)file->private_data;
/*    
    struct spi_rdwr_ioctl_data rdwr_arg;
    struct spi_msg *rdwr_pa;
    u8 __user **data_ptrs;
    int i,datasize,res;
    unsigned long funcs;
*/
    dev_dbg(&client->adapter->dev, "spi-%d ioctl, cmd: 0x%x, arg: %lx.\n",
			iminor(inode),cmd, arg);

    switch ( cmd ) {
    default:
	return spi_control(client,cmd,arg);
    }
    return 0;
}    

static int spidev_open(struct inode *inode, struct file *file)
{
    unsigned int minor = iminor(inode);
    struct spi_client *client;
    struct spi_adapter *adap;
    struct spi_dev *spi_dev;

    spi_dev = spi_dev_get_by_minor(minor);
    if (!spi_dev)
		return -ENODEV;

    adap = spi_get_adapter(spi_dev->adap->nr);
    if (!adap)
		return -ENODEV;

    client = kmalloc(sizeof(*client), GFP_KERNEL);
    if (!client) {
		spi_put_adapter(adap);
        return -ENOMEM;
    }
    memcpy(client, &spidev_client_template, sizeof(*client));

    /* registered with adapter, passed as client to user */
    client->adapter = adap;
    file->private_data = client;

    return 0;
}
																								
																								
static int spidev_release(struct inode *inode, struct file *file)
{
    struct spi_client *client = file->private_data;
	
    spi_put_adapter(client->adapter);
    kfree(client);
    file->private_data = NULL;

    return 0;
}
					
					
static struct file_operations spidev_fops = {
        .owner          = THIS_MODULE,
	.llseek         = no_llseek,
	.read           = spidev_read,
	.write          = spidev_write,
	.ioctl          = spidev_ioctl,
	.open           = spidev_open,
	.release        = spidev_release,
};

static void release_spi_dev(struct class_device *dev)
{
    struct spi_dev *spi_dev = to_spi_dev(dev);
    complete(&spi_dev->released);
}

static struct class spi_dev_class = {
        .name           = "spi-dev",
        .release        = &release_spi_dev,
};
											
static int spidev_attach_adapter(struct spi_adapter *adap)
{
    struct spi_dev *spi_dev;
    int retval;
		
    spi_dev = get_free_spi_dev(adap);
    if (IS_ERR(spi_dev))
	return PTR_ERR(spi_dev);

    devfs_mk_cdev(MKDEV(SPI_MAJOR, spi_dev->minor),
		S_IFCHR|S_IRUSR|S_IWUSR, "spi/%d", spi_dev->minor);
    dev_dbg(&adap->dev, "Registered as minor %d\n", spi_dev->minor);
    /* register this spi device with the driver core */
    spi_dev->adap = adap;
    if (adap->dev.parent == &platform_bus)
		spi_dev->class_dev.dev = &adap->dev;
    else
		spi_dev->class_dev.dev = adap->dev.parent;
    spi_dev->class_dev.class = &spi_dev_class;
    spi_dev->class_dev.devt = MKDEV(SPI_MAJOR, spi_dev->minor);
    snprintf(spi_dev->class_dev.class_id, BUS_ID_SIZE, "spi-%d", spi_dev->minor);
    retval = class_device_register(&spi_dev->class_dev);
    if (retval)
		goto error;
    class_device_create_file(&spi_dev->class_dev, &class_device_attr_name);
    return 0;
    
error:
    return_spi_dev(spi_dev);
    kfree(spi_dev);
    return retval;
}
																															
																															
static int spidev_detach_adapter(struct spi_adapter *adap)
{
    struct spi_dev *spi_dev;
	
    spi_dev = spi_dev_get_by_adapter(adap);
    if (!spi_dev)
	return -ENODEV;

    init_completion(&spi_dev->released);
    devfs_remove("spi/%d", spi_dev->minor);
    return_spi_dev(spi_dev);
    class_device_unregister(&spi_dev->class_dev);
    wait_for_completion(&spi_dev->released);
    kfree(spi_dev);

    dev_dbg(&adap->dev, "Adapter unregistered\n");
    return 0;
}
													
													
static int spidev_detach_client(struct spi_client *client)
{
    return 0;
}
	
static int spidev_command(struct spi_client *client, unsigned int cmd,
                           void *arg)
{
    return -1;
}
				   
static struct spi_driver spidev_driver = {
        .owner          = THIS_MODULE,
	.name           = "dev_driver",
	.id             = SPI_DRIVERID_SPIDEV,
	.flags          = SPI_DF_NOTIFY,
	.attach_adapter = spidev_attach_adapter,
	.detach_adapter = spidev_detach_adapter,
	.detach_client  = spidev_detach_client,
	.command        = spidev_command,
};
								
static struct spi_client spidev_client_template = {
	.name           = "SPI /dev entry",
	.addr           = -1,
	.driver         = &spidev_driver,
};
											
											 
static int __init spi_dev_init(void) 
{ 
    int res;
	
    printk(KERN_INFO "spi /dev entries driver\n");
		
    res = register_chrdev(SPI_MAJOR, "spi", &spidev_fops);
    if (res)
	goto out;

    res = class_register(&spi_dev_class);
    if (res)
	goto out_unreg_chrdev;

    res = spi_add_driver(&spidev_driver);
    if (res)
	goto out_unreg_class;

    devfs_mk_dir("spi");

    return 0;

out_unreg_class:
    class_unregister(&spi_dev_class);
out_unreg_chrdev:
    unregister_chrdev(SPI_MAJOR, "spi");
out:
    printk(KERN_ERR "%s: Driver Initialisation failed\n", __FILE__);
    return res;
} 
 
static void spi_dev_exit(void) 
{ 
    spi_del_driver(&spidev_driver);
    class_unregister(&spi_dev_class);
    devfs_remove("spi");
    unregister_chrdev(SPI_MAJOR,"spi");
} 
 
MODULE_AUTHOR("Barnabas Kalman <b...@vescont.com>");
MODULE_DESCRIPTION("SPI /dev entries driver");
MODULE_LICENSE("GPL");
 
module_init(spi_dev_init); 
module_exit(spi_dev_exit); 
