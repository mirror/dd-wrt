/*
 * PCI Express Hot Plug Controller Driver
 *
 * Copyright (C) 1995,2001 Compaq Computer Corporation
 * Copyright (C) 2001 Greg Kroah-Hartman (greg@kroah.com)
 * Copyright (C) 2001 IBM Corp.
 * Copyright (C) 2003-2004 Intel Corporation
 *
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, GOOD TITLE or
 * NON INFRINGEMENT.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Send feedback to <greg@kroah.com>, <dely.l.sy@intel.com>
 *
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/tqueue.h>
#include <linux/proc_fs.h>
#include <linux/pci.h>
#include "pciehp.h"
#ifndef CONFIG_IA64
#include "../../arch/i386/kernel/pci-i386.h"    /* horrible hack showing how processor dependant we are... */
#endif

static int is_pci_dev_in_use(struct pci_dev* dev) 
{
	/* 
	 * dev->driver will be set if the device is in use by a new-style 
	 * driver -- otherwise, check the device's regions to see if any
	 * driver has claimed them
	 */

	int i, inuse=0;

	if (dev->driver) return 1; /* Assume driver feels responsible */

	for (i = 0; !dev->driver && !inuse && (i < 6); i++) {
		if (!pci_resource_start(dev, i))
			continue;

		if (pci_resource_flags(dev, i) & IORESOURCE_IO)
			inuse = check_region(pci_resource_start(dev, i),
					     pci_resource_len(dev, i));
		else if (pci_resource_flags(dev, i) & IORESOURCE_MEM)
			inuse = check_mem_region(pci_resource_start(dev, i),
						 pci_resource_len(dev, i));
	}

	return inuse;

}


static int pci_hp_remove_device(struct pci_dev *dev)
{
	if (is_pci_dev_in_use(dev)) {
		err("***Cannot safely power down device -- "
		       "it appears to be in use***\n");
		return -EBUSY;
	}
	dbg("%s: dev %p\n", __FUNCTION__, dev);
	pci_remove_device(dev);
	return 0;
}


static int configure_visit_pci_dev (struct pci_dev_wrapped *wrapped_dev, struct pci_bus_wrapped *wrapped_bus) 
{
	struct pci_bus* bus = wrapped_bus->bus;
	struct pci_dev* dev = wrapped_dev->dev;
	struct pci_func *temp_func;
	int i=0;

	/* We need to fix up the hotplug function representation with the linux representation */
	do {
		temp_func = pciehp_slot_find(dev->bus->number, dev->devfn >> 3, i++);
	} while (temp_func && (temp_func->function != (dev->devfn & 0x07)));

	if (temp_func) {
		temp_func->pci_dev = dev;
	} else {
		/* We did not even find a hotplug rep of the function, create it
		 * This code might be taken out if we can guarantee the creation of functions
		 * in parallel (hotplug and Linux at the same time).
		 */
		dbg("@@@@@@@@@@@ pciehp_slot_create in %s\n", __FUNCTION__);
		temp_func = pciehp_slot_create(bus->number);
		if (temp_func == NULL)
			return -ENOMEM;
		temp_func->pci_dev = dev;
	}

	/* Create /proc/bus/pci proc entry for this device and bus device is on */
	/* Notify the drivers of the change */
	if (temp_func->pci_dev) {
		dbg("%s: PCI_ID=%04X:%04X\n", __FUNCTION__, temp_func->pci_dev->vendor,
			temp_func->pci_dev->device); 
		dbg("%s: PCI BUS %x DEVFN %x\n", __FUNCTION__, 
			temp_func->pci_dev->bus->number, temp_func->pci_dev->devfn); 
		dbg("%s: PCI_SLOT_NAME=%s\n", __FUNCTION__, 
			temp_func->pci_dev->slot_name);
		pci_enable_device(temp_func->pci_dev);
		pci_proc_attach_device(temp_func->pci_dev);
		pci_announce_device_to_drivers(temp_func->pci_dev);
	}

	return 0;
}


static int unconfigure_visit_pci_dev_phase2 (struct pci_dev_wrapped *wrapped_dev, struct pci_bus_wrapped *wrapped_bus) 
{
	struct pci_dev* dev = wrapped_dev->dev;

	struct pci_func *temp_func;
	int i=0;

	dbg("%s: dev %p dev->bus->number %x bus->number %x\n", __FUNCTION__, wrapped_dev, 
		dev->bus->number, wrapped_bus->bus->number);

	/* We need to remove the hotplug function representation with the linux representation */
	do {
		temp_func = pciehp_slot_find(dev->bus->number, dev->devfn >> 3, i++);
		if (temp_func) {
			dbg("%s: temp_func->function = %d\n", __FUNCTION__, temp_func->function);
		}
	} while (temp_func && (temp_func->function != (dev->devfn & 0x07)));

	/* Now, remove the Linux Representation */
	if (dev) {
		if (pci_hp_remove_device(dev) == 0) {
			kfree(dev); /* Now, remove */
		} else {
			return -1; /* problems while freeing, abort visitation */
		}
	}

	if (temp_func) {
		temp_func->pci_dev = NULL;
	} else {
		dbg("No pci_func representation for bus, devfn = %d, %x\n", dev->bus->number, dev->devfn);
	}

	return 0;
}


static int unconfigure_visit_pci_bus_phase2 (struct pci_bus_wrapped *wrapped_bus, struct pci_dev_wrapped *wrapped_dev) 
{
	struct pci_bus* bus = wrapped_bus->bus;

	/* The cleanup code for proc entries regarding buses should be in the kernel...*/
	if (bus->procdir)
		dbg("detach_pci_bus %s\n", bus->procdir->name);
	pci_proc_detach_bus(bus);
	/* The cleanup code should live in the kernel... */
	bus->self->subordinate = NULL;
	/* unlink from parent bus */
	list_del(&bus->node);

	/* Now, remove */
	if (bus)
		kfree(bus);

	return 0;
}


static int unconfigure_visit_pci_dev_phase1 (struct pci_dev_wrapped *wrapped_dev, struct pci_bus_wrapped *wrapped_bus) 
{
	struct pci_dev* dev = wrapped_dev->dev;
	int rc;

	dbg("attempting removal of driver for device (%x, %x, %x)\n", dev->bus->number, PCI_SLOT(dev->devfn), PCI_FUNC(dev->devfn));
	/* Now, remove the Linux Driver Representation */
	if (dev->driver) {
		if (dev->driver->remove) {
			dev->driver->remove(dev);
			dbg("driver was properly removed\n");
		}
		dev->driver = NULL;
	}

	rc = is_pci_dev_in_use(dev);
	if (rc)
		info("%s: device still in use\n", __FUNCTION__);
	return rc;
}


static struct pci_visit configure_functions = {
	.visit_pci_dev =	configure_visit_pci_dev,
};


static struct pci_visit unconfigure_functions_phase1 = {
	.post_visit_pci_dev =	unconfigure_visit_pci_dev_phase1
};

static struct pci_visit unconfigure_functions_phase2 = {
	.post_visit_pci_bus =	unconfigure_visit_pci_bus_phase2,               
	.post_visit_pci_dev =	unconfigure_visit_pci_dev_phase2
};


int pciehp_configure_device (struct controller* ctrl, struct pci_func* func)  
{
	unsigned char bus;
	struct pci_dev dev0;
	struct pci_bus *child;
	struct pci_dev* temp;
	int rc = 0;

	struct pci_dev_wrapped wrapped_dev;
	struct pci_bus_wrapped wrapped_bus;
	memset(&wrapped_dev, 0, sizeof(struct pci_dev_wrapped));
	memset(&wrapped_bus, 0, sizeof(struct pci_bus_wrapped));

	memset(&dev0, 0, sizeof(struct pci_dev));
	
	dbg("%s: func->pci_dev %p bus %x dev %x func %x\n", __FUNCTION__,
		func->pci_dev, func->bus, func->device, func->function);
	if (func->pci_dev == NULL)
		func->pci_dev = pci_find_slot(func->bus, (func->device << 3) | (func->function & 0x7));
	dbg("%s: func->pci_dev %p bus %x dev %x func %x\n", __FUNCTION__,
		func->pci_dev, func->bus, func->device, func->function);
	if (func->pci_dev != NULL) {
		dbg("%s: pci_dev %p bus %x devfn %x\n", __FUNCTION__,
			func->pci_dev, func->pci_dev->bus->number, func->pci_dev->devfn);
	}
	/* Still NULL ? Well then scan for it ! */
	if (func->pci_dev == NULL) {
		dbg("%s: pci_dev still null. do pci_scan_slot\n", __FUNCTION__);
		dev0.bus = ctrl->pci_dev->subordinate;
		dbg("%s: dev0.bus %p\n", __FUNCTION__, dev0.bus);
		dev0.bus->number = func->bus;
		dbg("%s: dev0.bus->number %x\n", __FUNCTION__, func->bus);
		dev0.devfn = PCI_DEVFN(func->device, func->function);
		dev0.sysdata = ctrl->pci_dev->sysdata;

		/* This will generate pci_dev structures for all functions, 
		 * but we will only call this case when lookup fails
		 */
		func->pci_dev = pci_scan_slot(&dev0);
		dbg("%s: func->pci_dev %p\n", __FUNCTION__, func->pci_dev);
		if (func->pci_dev == NULL) {
			dbg("ERROR: pci_dev still null\n");
			return 0;
		}
	}

	if (func->pci_dev->hdr_type == PCI_HEADER_TYPE_BRIDGE) {
		pci_read_config_byte(func->pci_dev, PCI_SECONDARY_BUS, &bus);
		child = (struct pci_bus*) pci_add_new_bus(func->pci_dev->bus, (func->pci_dev), bus);
		dbg("%s: bridge device - func->pci_dev->bus %p func->pci_dev %p bus %x\n", 
			__FUNCTION__, func->pci_dev->bus,func->pci_dev,bus);
		pci_do_scan_bus(child);

	}

	temp = func->pci_dev;

	if (temp) {
		wrapped_dev.dev = temp;
		wrapped_bus.bus = temp->bus;
		rc = pci_visit_dev(&configure_functions, &wrapped_dev, &wrapped_bus);
	}
	return rc;
}


int pciehp_unconfigure_device(struct pci_func* func) 
{
	int rc = 0;
	int j;
	struct pci_dev_wrapped wrapped_dev;
	struct pci_bus_wrapped wrapped_bus;
	
	memset(&wrapped_dev, 0, sizeof(struct pci_dev_wrapped));
	memset(&wrapped_bus, 0, sizeof(struct pci_bus_wrapped));

	dbg("%s: bus/dev/func = %x/%x/%x\n", __FUNCTION__, func->bus, func->device, func->function);

	for (j=0; j<8 ; j++) {
		struct pci_dev* temp = pci_find_slot(func->bus, (func->device << 3) | j);
		dbg("%s: temp %p\n", __FUNCTION__, temp);
		if (temp) {
			wrapped_dev.dev = temp;
			wrapped_bus.bus = temp->bus;
			rc = pci_visit_dev(&unconfigure_functions_phase1, &wrapped_dev, &wrapped_bus);
			if (rc)
				break;

			rc = pci_visit_dev(&unconfigure_functions_phase2, &wrapped_dev, &wrapped_bus);
			if (rc)
				break;
		}
	}
	return rc;
}

/*
 * pciehp_set_irq
 *
 * @bus_num: bus number of PCI device
 * @dev_num: device number of PCI device
 * @slot: pointer to u8 where slot number will be returned
 */
int pciehp_set_irq (u8 bus_num, u8 dev_num, u8 int_pin, u8 irq_num)
{
#if defined(CONFIG_X86) && !defined(CONFIG_X86_IO_APIC) && !defined(CONFIG_X86_64)
	int rc;
	u16 temp_word;
	struct pci_dev fakedev;
	struct pci_bus fakebus;

	fakedev.devfn = dev_num << 3;
	fakedev.bus = &fakebus;
	fakebus.number = bus_num;
	dbg("%s: dev %d, bus %d, pin %d, num %d\n",
	    __FUNCTION__, dev_num, bus_num, int_pin, irq_num);
	rc = pcibios_set_irq_routing(&fakedev, int_pin - 0x0a, irq_num);
	dbg("%s: rc %d\n", __FUNCTION__, rc);
	if (!rc)
		return !rc;

	/* set the Edge Level Control Register (ELCR) */
	temp_word = inb(0x4d0);
	temp_word |= inb(0x4d1) << 8;

	temp_word |= 0x01 << irq_num;

	/* This should only be for x86 as it sets the Edge Level Control Register */
	outb((u8) (temp_word & 0xFF), 0x4d0);
	outb((u8) ((temp_word & 0xFF00) >> 8), 0x4d1);
#endif
	return 0;
}

/* More PCI configuration routines; this time centered around hotplug controller */


/*
 * pciehp_save_config
 *
 * Reads configuration for all slots in a PCI bus and saves info.
 *
 * Note:  For non-hot plug busses, the slot # saved is the device #
 *
 * returns 0 if success
 */
int pciehp_save_config(struct controller *ctrl, int busnumber, int num_ctlr_slots, int first_device_num) 
{
	int rc;
	u8 class_code;
	u8 header_type;
	u32 ID;
	u8 secondary_bus;
	struct pci_func *new_slot;
	int sub_bus;
	int max_functions;
	int function;
	u8 DevError;
	int device = 0;
	int cloop = 0;
	int stop_it;
	int index;
	int is_hot_plug = num_ctlr_slots || first_device_num; 
	struct pci_bus lpci_bus, *pci_bus;
	int FirstSupported, LastSupported;
	
	dbg("%s: Enter\n", __FUNCTION__);

	memcpy(&lpci_bus, ctrl->pci_dev->subordinate, sizeof(lpci_bus));
	pci_bus = &lpci_bus;

	dbg("%s: num_ctlr_slots = %d, first_device_num = %d\n", __FUNCTION__, num_ctlr_slots, 
		first_device_num);

	/*   Decide which slots are supported */
	if (is_hot_plug) {
		/*********************************
		 *  is_hot_plug is the slot mask
		 *********************************/
		FirstSupported = first_device_num;
		LastSupported = FirstSupported + num_ctlr_slots - 1;
	} else {
		FirstSupported = 0;
		LastSupported = 0x1F;
	}

	dbg("FirstSupported = %d, LastSupported = %d\n", FirstSupported, LastSupported);

	/*   Save PCI configuration space for all devices in supported slots */
	dbg("%s: pci_bus->number = %x\n", __FUNCTION__, pci_bus->number);
	pci_bus->number = busnumber;
	dbg("%s: bus = %x, dev = %x\n", __FUNCTION__, busnumber, device);
	for (device = FirstSupported; device <= LastSupported; device++) { 
		ID = 0xFFFFFFFF;
		rc = pci_bus_read_config_dword(pci_bus, PCI_DEVFN(device, 0), PCI_VENDOR_ID, &ID);
		/* dbg("%s: ID = %x\n", __FUNCTION__, ID);*/

		if (ID != 0xFFFFFFFF) {	  /*  device in slot */
			dbg("%s: ID = %x\n", __FUNCTION__, ID);
			rc = pci_bus_read_config_byte(pci_bus, PCI_DEVFN(device, 0), 0x0B, &class_code);
			if (rc)
				return rc;

			rc = pci_bus_read_config_byte(pci_bus, PCI_DEVFN(device, 0), PCI_HEADER_TYPE, 
				&header_type);
			if (rc)
				return rc;

			dbg("class_code = %x, header_type = %x\n", class_code, header_type);

			/* If multi-function device, set max_functions to 8 */
			if (header_type & 0x80)
				max_functions = 8;
			else
				max_functions = 1;

			function = 0;

			do {
				DevError = 0;
				dbg("%s: In do loop\n", __FUNCTION__);

				if ((header_type & 0x7F) == PCI_HEADER_TYPE_BRIDGE) {   /* P-P Bridge */
					/* Recurse the subordinate bus
					 * get the subordinate bus number
					 */
					rc = pci_bus_read_config_byte(pci_bus, PCI_DEVFN(device, function), 
						PCI_SECONDARY_BUS, &secondary_bus);
					if (rc) {
						return rc;
					} else {
						sub_bus = (int) secondary_bus;

						/* Save secondary bus cfg spc with this recursive call. */
						rc = pciehp_save_config(ctrl, sub_bus, 0, 0); 
						if (rc)
							return rc;
					}
				}

				index = 0;
				new_slot = pciehp_slot_find(busnumber, device, index++);

				dbg("%s: new_slot = %p bus %x dev %x fun %x\n",
				__FUNCTION__, new_slot, busnumber, device, index-1);

				while (new_slot && (new_slot->function != (u8) function)) {
					new_slot = pciehp_slot_find(busnumber, device, index++);
					dbg("%s: while loop, new_slot = %p bus %x dev %x fun %x\n",
					__FUNCTION__, new_slot, busnumber, device, index-1);
				}
				if (!new_slot) {
					/* Setup slot structure. */
					new_slot = pciehp_slot_create(busnumber);
					dbg("%s: if, new_slot = %p bus %x dev %x fun %x\n",
					__FUNCTION__, new_slot, busnumber, device, function);

					if (new_slot == NULL)
						return(1);
				}

				new_slot->bus = (u8) busnumber;
				new_slot->device = (u8) device;
				new_slot->function = (u8) function;
				new_slot->is_a_board = 1;
				new_slot->switch_save = 0x10;
				/* In case of unsupported board */
				new_slot->status = DevError;
				new_slot->pci_dev = pci_find_slot(new_slot->bus, (new_slot->device << 3) | new_slot->function);
				dbg("new_slot->pci_dev = %p\n", new_slot->pci_dev);

				for (cloop = 0; cloop < 0x20; cloop++) {
					rc = pci_bus_read_config_dword(pci_bus, PCI_DEVFN(device, function), cloop << 2, (u32 *) & (new_slot->config_space [cloop]));
					/* dbg("new_slot->config_space[%x] = %x\n", cloop, new_slot->config_space[cloop]); */
					if (rc)
						return rc;
				}

				function++;

				stop_it = 0;

				/*  This loop skips to the next present function
				 *  reading in Class Code and Header type.
				 */

				while ((function < max_functions)&&(!stop_it)) {
					dbg("%s: In while loop \n", __FUNCTION__);
					rc = pci_bus_read_config_dword(pci_bus, PCI_DEVFN(device, function), PCI_VENDOR_ID, &ID);

					if (ID == 0xFFFFFFFF) {  /* Nothing there. */
						function++;
						dbg("Nothing there\n");
					} else {  /* Something there */
						rc = pci_bus_read_config_byte(pci_bus, PCI_DEVFN(device, function), 0x0B, &class_code);
						if (rc)
							return rc;

						rc = pci_bus_read_config_byte(pci_bus, PCI_DEVFN(device, function), PCI_HEADER_TYPE, &header_type);
						if (rc)
							return rc;

						dbg("class_code = %x, header_type = %x\n", class_code, header_type);
						stop_it++;
					}
				}

			} while (function < max_functions);
		}		/* End of IF (device in slot?) */
		else if (is_hot_plug) {
			/* Setup slot structure with entry for empty slot */
			new_slot = pciehp_slot_create(busnumber);

			if (new_slot == NULL) {
				return(1);
			}
			dbg("new_slot = %p, bus = %x, dev = %x, fun = %x\n", new_slot,
				new_slot->bus, new_slot->device, new_slot->function);

			new_slot->bus = (u8) busnumber;
			new_slot->device = (u8) device;
			new_slot->function = 0;
			new_slot->is_a_board = 0;
			new_slot->presence_save = 0;
			new_slot->switch_save = 0;
		}
		/* dbg("%s: End of For loop\n", __FUNCTION__); */
	 } 			/* End of FOR loop */

	dbg("%s: Exit\n", __FUNCTION__);
	return(0);
}


/*
 * pciehp_save_slot_config
 *
 * Saves configuration info for all PCI devices in a given slot
 * including subordinate busses.
 *
 * returns 0 if success
 */
int pciehp_save_slot_config (struct controller *ctrl, struct pci_func * new_slot)
{
	int rc;
	u8 class_code;
	u8 header_type;
	u32 ID;
	u8 secondary_bus;
	int sub_bus;
	int max_functions;
	int function;
	int cloop = 0;
	int stop_it;
	struct pci_bus lpci_bus, *pci_bus;

	memcpy(&lpci_bus, ctrl->pci_dev->subordinate, sizeof(lpci_bus));
	pci_bus = &lpci_bus;
	pci_bus->number = new_slot->bus;

	ID = 0xFFFFFFFF;

	pci_bus_read_config_dword(pci_bus, PCI_DEVFN(new_slot->device, 0), PCI_VENDOR_ID, &ID);

	if (ID != 0xFFFFFFFF) {	  /*  Device in slot */
		pci_bus_read_config_byte(pci_bus, PCI_DEVFN(new_slot->device, 0), 0x0B, &class_code);

		pci_bus_read_config_byte(pci_bus, PCI_DEVFN(new_slot->device, 0), PCI_HEADER_TYPE, &header_type);

		if (header_type & 0x80)	/* Multi-function device */
			max_functions = 8;
		else 
			max_functions = 1;

		function = 0;

		do {
			if ((header_type & 0x7F) == PCI_HEADER_TYPE_BRIDGE) {	  /* PCI-PCI Bridge */
				/*  Recurse the subordinate bus */
				pci_bus_read_config_byte(pci_bus, PCI_DEVFN(new_slot->device, function), PCI_SECONDARY_BUS, &secondary_bus);

				sub_bus = (int) secondary_bus;

				/* Save the config headers for the secondary bus. */
				rc = pciehp_save_config(ctrl, sub_bus, 0, 0); 

				if (rc)
					return(rc);

			}	/* End of IF */

			new_slot->status = 0;

			for (cloop = 0; cloop < 0x20; cloop++) {
				pci_bus_read_config_dword(pci_bus, PCI_DEVFN(new_slot->device, function), cloop << 2, (u32 *) & (new_slot->config_space [cloop]));
			}

			function++;

			stop_it = 0;

			/*  this loop skips to the next present function
			 *  reading in the Class Code and the Header type.
			 */

			while ((function < max_functions) && (!stop_it)) {
				pci_bus_read_config_dword(pci_bus, PCI_DEVFN(new_slot->device, function), PCI_VENDOR_ID, &ID);

				if (ID == 0xFFFFFFFF) {	 /* Nothing there. */
					function++;
				} else {  /* Something there */
					pci_bus_read_config_byte(pci_bus, PCI_DEVFN(new_slot->device, function), 0x0B, &class_code);

					pci_bus_read_config_byte(pci_bus, PCI_DEVFN(new_slot->device, function), PCI_HEADER_TYPE, &header_type);

					stop_it++;
				}
			}

		} while (function < max_functions);
	}			/* End of IF (device in slot?) */
	else {
		return(2);
	}

	return(0);
}


/*
 * pciehp_save_used_resources
 *
 * Stores used resource information for existing boards.  this is
 * for boards that were in the system when this driver was loaded.
 * this function is for hot plug ADD
 *
 * returns 0 if success
 * if disable  == 1(DISABLE_CARD),
 *  it loops for all functions of the slot and disables them.
 * else, it just get resources of the function and return.
 */
int pciehp_save_used_resources (struct controller *ctrl, struct pci_func *func, int disable)
{
	u8 cloop;
	u8 header_type;
	u8 secondary_bus;
	u8 temp_byte;
	u16 command;
	u16 save_command;
	u16 w_base, w_length;
	u32 temp_register;
	u32 save_base;
	u32 base, length;
	u64 base64 = 0;
	int index = 0;
	unsigned int devfn;
	struct pci_resource *mem_node = NULL;
	struct pci_resource *p_mem_node = NULL;
	struct pci_resource *t_mem_node;
	struct pci_resource *io_node;
	struct pci_resource *bus_node;
	struct pci_bus lpci_bus, *pci_bus;

	memcpy(&lpci_bus, ctrl->pci_dev->subordinate, sizeof(lpci_bus));
	pci_bus = &lpci_bus;

	if (disable)
		func = pciehp_slot_find(func->bus, func->device, index++);

	while ((func != NULL) && func->is_a_board) {
		pci_bus->number = func->bus;
		devfn = PCI_DEVFN(func->device, func->function);

		/* Save the command register */
		pci_bus_read_config_word (pci_bus, devfn, PCI_COMMAND, &save_command);

		if (disable) {
			/* Disable card */
			command = 0x00;
			pci_bus_write_config_word(pci_bus, devfn, PCI_COMMAND, command);
		}

		/* Check for Bridge */
		pci_bus_read_config_byte (pci_bus, devfn, PCI_HEADER_TYPE, &header_type);

		if ((header_type & 0x7F) == PCI_HEADER_TYPE_BRIDGE) {     /* PCI-PCI Bridge */
			dbg("Save_used_res of PCI bridge b:d=0x%x:%x, sc=0x%x\n", func->bus, func->device, save_command);
			if (disable) {
				/* Clear Bridge Control Register */
				command = 0x00;
				pci_bus_write_config_word(pci_bus, devfn, PCI_BRIDGE_CONTROL, command);
			}

			pci_bus_read_config_byte (pci_bus, devfn, PCI_SECONDARY_BUS, &secondary_bus);
			pci_bus_read_config_byte (pci_bus, devfn, PCI_SUBORDINATE_BUS, &temp_byte);

			bus_node =(struct pci_resource *) kmalloc(sizeof(struct pci_resource), GFP_KERNEL);
			if (!bus_node)
				return -ENOMEM;

			bus_node->base = (ulong)secondary_bus;
			bus_node->length = (ulong)(temp_byte - secondary_bus + 1);

			bus_node->next = func->bus_head;
			func->bus_head = bus_node;

			/* Save IO base and Limit registers */
			pci_bus_read_config_byte (pci_bus, devfn, PCI_IO_BASE, &temp_byte);
			base = temp_byte;
			pci_bus_read_config_byte (pci_bus, devfn, PCI_IO_LIMIT, &temp_byte);
			length = temp_byte;

			if ((base <= length) && (!disable || (save_command & PCI_COMMAND_IO))) {
				io_node = (struct pci_resource *) kmalloc(sizeof(struct pci_resource), GFP_KERNEL);
				if (!io_node)
					return -ENOMEM;

				io_node->base = (ulong)(base & PCI_IO_RANGE_MASK) << 8;
				io_node->length = (ulong)(length - base + 0x10) << 8;

				io_node->next = func->io_head;
				func->io_head = io_node;
			}

			/* Save memory base and Limit registers */
			pci_bus_read_config_word (pci_bus, devfn, PCI_MEMORY_BASE, &w_base);
			pci_bus_read_config_word (pci_bus, devfn, PCI_MEMORY_LIMIT, &w_length);

			if ((w_base <= w_length) && (!disable || (save_command & PCI_COMMAND_MEMORY))) {
				mem_node = (struct pci_resource *) kmalloc(sizeof(struct pci_resource), GFP_KERNEL);
				if (!mem_node)
					return -ENOMEM;

				mem_node->base = (ulong)w_base << 16;
				mem_node->length = (ulong)(w_length - w_base + 0x10) << 16;

				mem_node->next = func->mem_head;
				func->mem_head = mem_node;
			}
			/* Save prefetchable memory base and Limit registers */
			pci_bus_read_config_word (pci_bus, devfn, PCI_PREF_MEMORY_BASE, &w_base);
			pci_bus_read_config_word (pci_bus, devfn, PCI_PREF_MEMORY_LIMIT, &w_length);

			if ((w_base <= w_length) && (!disable || (save_command & PCI_COMMAND_MEMORY))) {
				p_mem_node = (struct pci_resource *) kmalloc(sizeof(struct pci_resource), GFP_KERNEL);
				if (!p_mem_node)
					return -ENOMEM;

				p_mem_node->base = (ulong)w_base << 16;
				p_mem_node->length = (ulong)(w_length - w_base + 0x10) << 16;

				p_mem_node->next = func->p_mem_head;
				func->p_mem_head = p_mem_node;
			}
		} else if ((header_type & 0x7F) == PCI_HEADER_TYPE_NORMAL) {
			dbg("Save_used_res of PCI adapter b:d=0x%x:%x, sc=0x%x\n", func->bus, func->device, save_command);

			/* Figure out IO and memory base lengths */
			for (cloop = PCI_BASE_ADDRESS_0; cloop <= PCI_BASE_ADDRESS_5; cloop += 4) {
				pci_bus_read_config_dword (pci_bus, devfn, cloop, &save_base);

				temp_register = 0xFFFFFFFF;
				pci_bus_write_config_dword (pci_bus, devfn, cloop, temp_register);
				pci_bus_read_config_dword (pci_bus, devfn, cloop, &temp_register);

				if (!disable) {
					pci_bus_write_config_dword (pci_bus, devfn, cloop, save_base);
				}

				if (!temp_register)
					continue;

				base = temp_register;

				if ((base & PCI_BASE_ADDRESS_SPACE_IO) && (!disable || (save_command & PCI_COMMAND_IO))) {
					/* IO base */
					/* Set temp_register = amount of IO space requested */
					base = base & 0xFFFFFFFCL;
					base = (~base) + 1;

					io_node = (struct pci_resource *) kmalloc(sizeof (struct pci_resource), GFP_KERNEL);
					if (!io_node)
						return -ENOMEM;

					io_node->base = (ulong)save_base & PCI_BASE_ADDRESS_IO_MASK;
					io_node->length = (ulong)base;
					dbg("sur adapter: IO bar=0x%x(length=0x%x)\n", io_node->base, io_node->length);

					io_node->next = func->io_head;
					func->io_head = io_node;
				} else {  /* Map Memory */
					int prefetchable = 1;
					/* struct pci_resources **res_node; */
					char *res_type_str = "PMEM";
					u32 temp_register2;

					t_mem_node = (struct pci_resource *) kmalloc(sizeof (struct pci_resource), GFP_KERNEL);
					if (!t_mem_node)
						return -ENOMEM;

					if (!(base & PCI_BASE_ADDRESS_MEM_PREFETCH) && (!disable || (save_command & PCI_COMMAND_MEMORY))) {
						prefetchable = 0;
						mem_node = t_mem_node;
						res_type_str++;
					} else
						p_mem_node = t_mem_node;

					base = base & 0xFFFFFFF0L;
					base = (~base) + 1;

					switch (temp_register & PCI_BASE_ADDRESS_MEM_TYPE_MASK) {
					case PCI_BASE_ADDRESS_MEM_TYPE_32:
						if (prefetchable) {
							p_mem_node->base = (ulong)save_base & PCI_BASE_ADDRESS_MEM_MASK;
							p_mem_node->length = (ulong)base;
							dbg("sur adapter: 32 %s bar=0x%x(length=0x%x)\n", res_type_str, p_mem_node->base, p_mem_node->length);

							p_mem_node->next = func->p_mem_head;
							func->p_mem_head = p_mem_node;
						} else {
							mem_node->base = (ulong)save_base & PCI_BASE_ADDRESS_MEM_MASK;
							mem_node->length = (ulong)base;
							dbg("sur adapter: 32 %s bar=0x%x(length=0x%x)\n", res_type_str, mem_node->base, mem_node->length);

							mem_node->next = func->mem_head;
							func->mem_head = mem_node;
						}
						break;
					case PCI_BASE_ADDRESS_MEM_TYPE_64:
						pci_bus_read_config_dword(pci_bus, devfn, cloop+4, &temp_register2);
						base64 = temp_register2;
						base64 = (base64 << 32) | save_base;

						if (temp_register2) {
							dbg("sur adapter: 64 %s high dword of base64(0x%x:%x) masked to 0\n", res_type_str, temp_register2, (u32)base64);
							base64 &= 0x00000000FFFFFFFFL;
						}

						if (prefetchable) {
							p_mem_node->base = base64 & PCI_BASE_ADDRESS_MEM_MASK;
							p_mem_node->length = base;
							dbg("sur adapter: 64 %s base=0x%x(len=0x%x)\n", res_type_str, p_mem_node->base, p_mem_node->length);

							p_mem_node->next = func->p_mem_head;
							func->p_mem_head = p_mem_node;
						} else {
							mem_node->base = base64 & PCI_BASE_ADDRESS_MEM_MASK;
							mem_node->length = base;
							dbg("sur adapter: 64 %s base=0x%x(len=0x%x)\n", res_type_str, mem_node->base, mem_node->length);

							mem_node->next = func->mem_head;
							func->mem_head = mem_node;
						}
						cloop += 4;
						break;
					default:
						dbg("asur: reserved BAR type=0x%x\n", temp_register);
						break;
					}
				} 
			}	/* End of base register loop */
		} else {	/* Some other unknown header type */
			dbg("Save_used_res of PCI unknown type b:d=0x%x:%x. skip.\n", func->bus, func->device);
		}

		/* Find the next device in this slot */
		if (!disable)
			break;
		func = pciehp_slot_find(func->bus, func->device, index++);
	}

	return(0);
}


/*
 * pciehp_return_board_resources
 *
 * this routine returns all resources allocated to a board to
 * the available pool.
 *
 * returns 0 if success
 */
int pciehp_return_board_resources(struct pci_func * func, struct resource_lists * resources)
{
	int rc = 0;
	struct pci_resource *node;
	struct pci_resource *t_node;
	dbg("%s\n", __FUNCTION__);

	if (!func)
		return(1);

	node = func->io_head;
	func->io_head = NULL;
	while (node) {
		t_node = node->next;
		return_resource(&(resources->io_head), node);
		node = t_node;
	}

	node = func->mem_head;
	func->mem_head = NULL;
	while (node) {
		t_node = node->next;
		return_resource(&(resources->mem_head), node);
		node = t_node;
	}

	node = func->p_mem_head;
	func->p_mem_head = NULL;
	while (node) {
		t_node = node->next;
		return_resource(&(resources->p_mem_head), node);
		node = t_node;
	}

	node = func->bus_head;
	func->bus_head = NULL;
	while (node) {
		t_node = node->next;
		return_resource(&(resources->bus_head), node);
		node = t_node;
	}

	rc |= pciehp_resource_sort_and_combine(&(resources->mem_head));
	rc |= pciehp_resource_sort_and_combine(&(resources->p_mem_head));
	rc |= pciehp_resource_sort_and_combine(&(resources->io_head));
	rc |= pciehp_resource_sort_and_combine(&(resources->bus_head));

	return(rc);
}


/*
 * pciehp_destroy_resource_list
 *
 * Puts node back in the resource list pointed to by head
 */
void pciehp_destroy_resource_list (struct resource_lists * resources)
{
	struct pci_resource *res, *tres;

	res = resources->io_head;
	resources->io_head = NULL;

	while (res) {
		tres = res;
		res = res->next;
		kfree(tres);
	}

	res = resources->mem_head;
	resources->mem_head = NULL;

	while (res) {
		tres = res;
		res = res->next;
		kfree(tres);
	}

	res = resources->p_mem_head;
	resources->p_mem_head = NULL;

	while (res) {
		tres = res;
		res = res->next;
		kfree(tres);
	}

	res = resources->bus_head;
	resources->bus_head = NULL;

	while (res) {
		tres = res;
		res = res->next;
		kfree(tres);
	}
}


/*
 * pciehp_destroy_board_resources
 *
 * Puts node back in the resource list pointed to by head
 */
void pciehp_destroy_board_resources (struct pci_func * func)
{
	struct pci_resource *res, *tres;

	res = func->io_head;
	func->io_head = NULL;

	while (res) {
		tres = res;
		res = res->next;
		kfree(tres);
	}

	res = func->mem_head;
	func->mem_head = NULL;

	while (res) {
		tres = res;
		res = res->next;
		kfree(tres);
	}

	res = func->p_mem_head;
	func->p_mem_head = NULL;

	while (res) {
		tres = res;
		res = res->next;
		kfree(tres);
	}

	res = func->bus_head;
	func->bus_head = NULL;

	while (res) {
		tres = res;
		res = res->next;
		kfree(tres);
	}
}

