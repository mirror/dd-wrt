/* 
 *  linux/drivers/spi/spi-core.c 
 * 
 *  Copyright (C) 2001 Russell King 
 *  Copyright (C) 2002 Compaq Computer Corporation 
 * 
 *  Adapted from l3-core.c by Jamey Hicks <jamey.hi...@compaq.com> 
 * 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License. 
 * 
 */ 

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/init.h>
#include <linux/idr.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>
 
static LIST_HEAD(adapters);
static LIST_HEAD(drivers);
static DECLARE_MUTEX(core_lists);
static DEFINE_IDR(spi_adapter_idr);

/* 
 * Bus declaration, init/exit functions 
 */ 
 
/* match always succeeds, as we want the probe() to tell if we really accept this match */
static int spi_device_match(struct device *dev, struct device_driver *drv)
{
    return 1;
}
	
static int spi_bus_suspend(struct device * dev, pm_message_t state)
{
    int rc = 0;
		
    if (dev->driver && dev->driver->suspend)
	rc = dev->driver->suspend(dev,state);
    return rc;
}

static int spi_bus_resume(struct device * dev)
{
    int rc = 0;

    if (dev->driver && dev->driver->resume)
	rc = dev->driver->resume(dev);
    return rc;
}
											
struct bus_type spi_bus_type = { 
    .name 	= "spi", 
    .match 	= spi_device_match,
    .suspend	= spi_bus_suspend,
    .resume	= spi_bus_resume,
};


static int spi_device_probe(struct device *dev)
{
    return -ENODEV;
}
	
static int spi_device_remove(struct device *dev)
{
    return 0;
}
		
static void spi_adapter_dev_release(struct device *dev)
{
    struct spi_adapter *adap = dev_to_spi_adapter(dev);
    complete(&adap->dev_released);
}
		
				
static struct device_driver spi_adapter_driver = {
    .name 	= "spi_adapter",
    .bus 	= &spi_bus_type,
    .probe	= spi_device_probe,
    .remove	= spi_device_remove,
};
				

static void spi_adapter_class_dev_release(struct class_device *dev)
{
    struct spi_adapter *adap = class_dev_to_spi_adapter(dev);
    complete(&adap->class_dev_released);
}
		
		
static struct class spi_adapter_class = {
    .name 	= "spi-adapter",
    .release 	= &spi_adapter_class_dev_release,
};
		
static ssize_t show_adapter_name(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct spi_adapter *adap = dev_to_spi_adapter(dev);
    return sprintf(buf, "%s\n", adap->name);
}
static DEVICE_ATTR(name, S_IRUGO, show_adapter_name, NULL);

static void spi_client_release(struct device *dev)
{
	struct spi_client *client = to_spi_client(dev);
	complete(&client->released);
}

		
static ssize_t show_client_name(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct spi_client *client = to_spi_client(dev);
    return sprintf(buf, "%s\n", client->name);
}
		
/*
 * We can't use the DEVICE_ATTR() macro here as we want the same filename for a
 * different type of a device.  So beware if the DEVICE_ATTR() macro ever
 * changes, this definition will also have to change.
 */
static struct device_attribute dev_attr_client_name = {
    .attr   = {.name = "name", .mode = S_IRUGO, .owner = THIS_MODULE },
    .show   = &show_client_name,
};
				    


/* ---------------------------------------------------
 * registering functions 
 * --------------------------------------------------- 
 */

/* -----
 * spi_add_adapter is called from within the algorithm layer,
 * when a new hw adapter registers. A new device is register to be
 * available for clients.
 */
int spi_add_adapter(struct spi_adapter *adap)
{
	int id, res = 0;
	struct list_head   *item;
	struct spi_driver  *driver;

	down(&core_lists);

	if (idr_pre_get(&spi_adapter_idr, GFP_KERNEL) == 0) {
		res = -ENOMEM;
		goto out_unlock;
	}

	res = idr_get_new(&spi_adapter_idr, adap, &id);
	if (res < 0) {
		if (res == -EAGAIN)
			res = -ENOMEM;
		goto out_unlock;
	}

	adap->nr =  id & MAX_ID_MASK;
	init_MUTEX(&adap->bus_lock);
	init_MUTEX(&adap->clist_lock);
	list_add_tail(&adap->list,&adapters);
	INIT_LIST_HEAD(&adap->clients);

	/* Add the adapter to the driver core.
	 * If the parent pointer is not set up,
	 * we add this adapter to the host bus.
	 */
	if (adap->dev.parent == NULL)
		adap->dev.parent = &platform_bus;
	sprintf(adap->dev.bus_id, "spi-%d", adap->nr);
	adap->dev.driver = &spi_adapter_driver;
	adap->dev.release = &spi_adapter_dev_release;
	device_register(&adap->dev);
	device_create_file(&adap->dev, &dev_attr_name);

	/* Add this adapter to the spi_adapter class */
	memset(&adap->class_dev, 0x00, sizeof(struct class_device));
	adap->class_dev.dev = &adap->dev;
	adap->class_dev.class = &spi_adapter_class;
	strlcpy(adap->class_dev.class_id, adap->dev.bus_id, BUS_ID_SIZE);
	class_device_register(&adap->class_dev);

	/* inform drivers of new adapters */
	list_for_each(item,&drivers) {
		driver = list_entry(item, struct spi_driver, list);
		if (driver->flags & SPI_DF_NOTIFY)
			/* We ignore the return code; if it fails, too bad */
			driver->attach_adapter(adap);
	}

	dev_dbg(&adap->dev, "registered as adapter #%d\n", adap->nr);

out_unlock:
	up(&core_lists);
	return res;
}


int spi_del_adapter(struct spi_adapter *adap)
{
	struct list_head  *item, *_n;
	struct spi_adapter *adap_from_list;
	struct spi_driver *driver;
	struct spi_client *client;
	int res = 0;

	down(&core_lists);

	/* First make sure that this adapter was ever added */
	list_for_each_entry(adap_from_list, &adapters, list) {
		if (adap_from_list == adap)
			break;
	}
	if (adap_from_list != adap) {
		pr_debug("SPI: Attempting to delete an unregistered "
			 "adapter\n");
		res = -EINVAL;
		goto out_unlock;
	}

	list_for_each(item,&drivers) {
		driver = list_entry(item, struct spi_driver, list);
		if (driver->detach_adapter)
			if ((res = driver->detach_adapter(adap))) {
				dev_warn(&adap->dev, "can't detach adapter "
					 "while detaching driver %s: driver "
					 "not detached!\n", driver->name);
				goto out_unlock;
			}
	}

	/* detach any active clients. This must be done first, because
	 * it can fail; in which case we give up. */
	list_for_each_safe(item, _n, &adap->clients) {
		client = list_entry(item, struct spi_client, list);

		/* detaching devices is unconditional of the set notify
		 * flag, as _all_ clients that reside on the adapter
		 * must be deleted, as this would cause invalid states.
		 */
		if ((res=client->driver->detach_client(client))) {
			dev_err(&adap->dev, "adapter not "
				"unregistered, because client at "
				"address %02x can't be detached. ",
				client->addr);
			goto out_unlock;
		}
	}

	/* clean up the sysfs representation */
	init_completion(&adap->dev_released);
	init_completion(&adap->class_dev_released);
	class_device_unregister(&adap->class_dev);
	device_remove_file(&adap->dev, &dev_attr_name);
	device_unregister(&adap->dev);
	list_del(&adap->list);

	/* wait for sysfs to drop all references */
	wait_for_completion(&adap->dev_released);
	wait_for_completion(&adap->class_dev_released);

	/* free dynamically allocated bus id */
	idr_remove(&spi_adapter_idr, adap->nr);

	dev_dbg(&adap->dev, "adapter unregistered\n");

 out_unlock:
	up(&core_lists);
	return res;
}


/* -----
 * What follows is the "upwards" interface: commands for talking to clients,
 * which implement the functions to access the physical information of the
 * chips.
 */

int spi_add_driver(struct spi_driver *driver)
{
	struct list_head   *item;
	struct spi_adapter *adapter;
	int res = 0;

	down(&core_lists);

	/* add the driver to the list of spi drivers in the driver core */
	driver->driver.name = driver->name;
	driver->driver.bus = &spi_bus_type;
	driver->driver.probe = spi_device_probe;
	driver->driver.remove = spi_device_remove;

	res = driver_register(&driver->driver);
	if (res)
		goto out_unlock;
	
	list_add_tail(&driver->list,&drivers);
	pr_debug("spi-core: driver %s registered.\n", driver->name);

	/* now look for instances of driver on our adapters */
	if (driver->flags & SPI_DF_NOTIFY) {
		list_for_each(item,&adapters) {
			adapter = list_entry(item, struct spi_adapter, list);
			driver->attach_adapter(adapter);
		}
	}

 out_unlock:
	up(&core_lists);
	return res;
}

int spi_del_driver(struct spi_driver *driver)
{
	struct list_head   *item1, *item2, *_n;
	struct spi_client  *client;
	struct spi_adapter *adap;
	
	int res = 0;

	down(&core_lists);

	/* Have a look at each adapter, if clients of this driver are still
	 * attached. If so, detach them to be able to kill the driver 
	 * afterwards.
	 */
	pr_debug("spi-core: unregister_driver - looking for clients.\n");
	/* removing clients does not depend on the notify flag, else 
	 * invalid operation might (will!) result, when using stale client
	 * pointers.
	 */
	list_for_each(item1,&adapters) {
		adap = list_entry(item1, struct spi_adapter, list);
		dev_dbg(&adap->dev, "examining adapter\n");
		if (driver->detach_adapter) {
			if ((res = driver->detach_adapter(adap))) {
				dev_warn(&adap->dev, "while unregistering "
				       "dummy driver %s, adapter could "
				       "not be detached properly; driver "
				       "not unloaded!",driver->name);
				goto out_unlock;
			}
		} else {
			list_for_each_safe(item2, _n, &adap->clients) {
				client = list_entry(item2, struct spi_client, list);
				if (client->driver != driver)
					continue;
				pr_debug("spi-core.o: detaching client %s:\n", client->name);
				if ((res = driver->detach_client(client))) {
					dev_err(&adap->dev, "while "
						"unregistering driver "
						"`%s', the client at "
						"address %02x of "
						"adapter could not "
						"be detached; driver "
						"not unloaded!",
						driver->name,
						client->addr);
					goto out_unlock;
				}
			}
		}
	}

	driver_unregister(&driver->driver);
	list_del(&driver->list);
	pr_debug("spi-core: driver unregistered: %s\n", driver->name);

 out_unlock:
	up(&core_lists);
	return 0;
}

static int __spi_check_addr(struct spi_adapter *adapter, unsigned int addr)
{
	struct list_head   *item;
	struct spi_client  *client;

	list_for_each(item,&adapter->clients) {
		client = list_entry(item, struct spi_client, list);
		if (client->addr == addr)
			return -EBUSY;
	}
	return 0;
}

int spi_check_addr(struct spi_adapter *adapter, int addr)
{
	int rval;

	down(&adapter->clist_lock);
	rval = __spi_check_addr(adapter, addr);
	up(&adapter->clist_lock);

	return rval;
}


int spi_attach_client(struct spi_client *client)
{
	struct spi_adapter *adapter = client->adapter;

	down(&adapter->clist_lock);
	if (__spi_check_addr(client->adapter, client->addr)) {
		up(&adapter->clist_lock);
		return -EBUSY;
	}
	list_add_tail(&client->list,&adapter->clients);
	up(&adapter->clist_lock);
	
	if (adapter->client_register)  {
		if (adapter->client_register(client))  {
			dev_warn(&adapter->dev, "warning: client_register "
				"seems to have failed for client %02x\n",
				client->addr);
		}
	}

	dev_dbg(&adapter->dev, "client [%s] registered to adapter\n",
		client->name);

	if (client->flags & SPI_CLIENT_ALLOW_USE)
		client->usage_count = 0;

	client->dev.parent = &client->adapter->dev;
	client->dev.driver = &client->driver->driver;
	client->dev.bus = &spi_bus_type;
	client->dev.release = &spi_client_release;
	
	snprintf(&client->dev.bus_id[0], sizeof(client->dev.bus_id),
		"%d-%04x", spi_adapter_id(adapter), client->addr);
	pr_debug("registering %s\n", client->dev.bus_id);
	device_register(&client->dev);
	device_create_file(&client->dev, &dev_attr_client_name);
	
	return 0;
}


int spi_detach_client(struct spi_client *client)
{
	struct spi_adapter *adapter = client->adapter;
	int res = 0;
	
	if ((client->flags & SPI_CLIENT_ALLOW_USE) && (client->usage_count > 0))
		return -EBUSY;

	if (adapter->client_unregister)  {
		res = adapter->client_unregister(client);
		if (res) {
			dev_err(&client->dev,
				"client_unregister [%s] failed, "
				"client not detached\n", client->name);
			goto out;
		}
	}

	down(&adapter->clist_lock);
	list_del(&client->list);
	init_completion(&client->released);
	device_remove_file(&client->dev, &dev_attr_client_name);
	device_unregister(&client->dev);
	up(&adapter->clist_lock);
	wait_for_completion(&client->released);

 out:
	return res;
}

/* ----------------------------------------------------
 * the functional interface to the spi busses.
 * ----------------------------------------------------
 */

int spi_transfer(struct spi_adapter * adap, struct spi_msg *msgs, int num)
{
	int ret;

	if (adap->algo->master_xfer) {
#ifdef DEBUG
		for (ret = 0; ret < num; ret++) {
			dev_dbg(&adap->dev, "master_xfer[%d] %c, addr=0x%02x, "
				"len=%d\n", ret, msgs[ret].flags & SPI_M_RD ?
				'R' : 'W', msgs[ret].addr, msgs[ret].len);
		}
#endif

		down(&adap->bus_lock);
		ret = adap->algo->master_xfer(adap,msgs,num);
		up(&adap->bus_lock);

		return ret;
	} else {
		dev_dbg(&adap->dev, "SPI level transfers not supported\n");
		return -ENOSYS;
	}
}

int spi_master_send(struct spi_client *client,const char *buf ,int count)
{
	int ret;
	struct spi_adapter *adap=client->adapter;
	struct spi_msg msg;

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.len = count;
	msg.buf = (char *)buf;
	
	ret = spi_transfer(adap, &msg, 1);

	/* If everything went ok (i.e. 1 msg transmitted), return #bytes
	   transmitted, else error code. */
	return (ret == 1) ? count : ret;
}

int spi_master_recv(struct spi_client *client, char *buf ,int count)
{
	struct spi_adapter *adap=client->adapter;
	struct spi_msg msg;
	int ret;

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.flags |= SPI_M_RD;
	msg.len = count;
	msg.buf = buf;

	ret = spi_transfer(adap, &msg, 1);

	/* If everything went ok (i.e. 1 msg transmitted), return #bytes
	   transmitted, else error code. */
	return (ret == 1) ? count : ret;
}


int spi_control(struct spi_client *client,
	unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct spi_adapter *adap = client->adapter;

	dev_dbg(&client->adapter->dev, "spi ioctl, cmd: 0x%x, arg: %#lx\n", cmd, arg);
	switch (cmd) {
		case SPI_RETRIES:
			adap->retries = arg;
			break;
		case SPI_TIMEOUT:
			adap->timeout = arg;
			break;
		default:
			if (adap->algo->algo_control!=NULL)
				ret = adap->algo->algo_control(adap,cmd,arg);
	}
	return ret;
}


/*
 * return id number for a specific adapter
 */
int spi_adapter_id(struct spi_adapter *adap)
{
	return adap->nr;
}

struct spi_adapter* spi_get_adapter(int id)
{
	struct spi_adapter *adapter;
	
	down(&core_lists);
	adapter = (struct spi_adapter *)idr_find(&spi_adapter_idr, id);
	if (adapter && !try_module_get(adapter->owner))
		adapter = NULL;

	up(&core_lists);
	return adapter;
}

void spi_put_adapter(struct spi_adapter *adap)
{
	module_put(adap->owner);
}












		 
int __init spi_init(void) 
{ 
    int retval;
	
    retval = bus_register(&spi_bus_type);
    if (retval)
	return retval;
    retval = driver_register(&spi_adapter_driver);
    if (retval)
	return retval;
    retval = class_register(&spi_adapter_class);
    
    pr_debug("SPI bus driver loaded.\n");

    return retval;
} 
 
void __exit spi_exit(void) 
{ 
    class_unregister(&spi_adapter_class);
    driver_unregister(&spi_adapter_driver);
    bus_unregister(&spi_bus_type);
    pr_debug("SPI: leaving\n"); 
} 
 
subsys_initcall(spi_init); 
module_exit(spi_exit); 

EXPORT_SYMBOL(spi_add_adapter);
EXPORT_SYMBOL(spi_del_adapter);
EXPORT_SYMBOL(spi_add_driver);
EXPORT_SYMBOL(spi_del_driver);
EXPORT_SYMBOL(spi_attach_client);
EXPORT_SYMBOL(spi_detach_client);
//EXPORT_SYMBOL(spi_use_client);
//EXPORT_SYMBOL(spi_release_client);
//EXPORT_SYMBOL(spi_clients_command);
EXPORT_SYMBOL(spi_check_addr);

EXPORT_SYMBOL(spi_master_send);
EXPORT_SYMBOL(spi_master_recv);
EXPORT_SYMBOL(spi_control);
EXPORT_SYMBOL(spi_transfer);
EXPORT_SYMBOL(spi_adapter_id);
EXPORT_SYMBOL(spi_get_adapter);
EXPORT_SYMBOL(spi_put_adapter);
//EXPORT_SYMBOL(spi_probe);

MODULE_AUTHOR( "Barnabas Kalman <ba...@sednet.hu>" );
MODULE_DESCRIPTION( "SPI-Bus main module" );
MODULE_LICENSE( "GPL" ); 
 
