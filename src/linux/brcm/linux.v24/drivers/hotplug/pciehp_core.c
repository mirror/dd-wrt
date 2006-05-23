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
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/tqueue.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include "pciehp.h"
#include "pciehprm.h"

/* Global variables */
int pciehp_debug;
int pciehp_poll_mode;
int pciehp_poll_time;
struct controller *pciehp_ctrl_list;	/* = NULL */
struct pci_func *pciehp_slot_list[256];

#define DRIVER_VERSION	"0.5"
#define DRIVER_AUTHOR	"Dan Zink <dan.zink@compaq.com>, Greg Kroah-Hartman <greg@kroah.com>, Dely Sy <dely.l.sy@intel.com>"
#define DRIVER_DESC	"PCI Express Hot Plug Controller Driver"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

MODULE_PARM(pciehp_debug, "i");
MODULE_PARM(pciehp_poll_mode, "i");
MODULE_PARM(pciehp_poll_time, "i");
MODULE_PARM_DESC(pciehp_debug, "Debugging mode enabled or not");
MODULE_PARM_DESC(pciehp_poll_mode, "Using polling mechanism for hot-plug events or not");
MODULE_PARM_DESC(pciehp_poll_time, "Polling mechanism frequency, in seconds");

#define PCIE_MODULE_NAME "pciehp.o"

static int pcie_start_thread (void);
static int set_attention_status (struct hotplug_slot *slot, u8 value);
static int enable_slot		(struct hotplug_slot *slot);
static int disable_slot		(struct hotplug_slot *slot);
static int hardware_test	(struct hotplug_slot *slot, u32 value);
static int get_power_status	(struct hotplug_slot *slot, u8 *value);
static int get_attention_status	(struct hotplug_slot *slot, u8 *value);
static int get_latch_status	(struct hotplug_slot *slot, u8 *value);
static int get_adapter_status	(struct hotplug_slot *slot, u8 *value);
static int get_max_bus_speed	(struct hotplug_slot *slot, enum pci_bus_speed *value);
static int get_cur_bus_speed	(struct hotplug_slot *slot, enum pci_bus_speed *value);

static struct hotplug_slot_ops pciehp_hotplug_slot_ops = {
	.owner =		THIS_MODULE,
	.set_attention_status =	set_attention_status,
	.enable_slot =		enable_slot,
	.disable_slot =		disable_slot,
	.hardware_test =	hardware_test,
	.get_power_status =	get_power_status,
	.get_attention_status =	get_attention_status,
	.get_latch_status =		get_latch_status,
	.get_adapter_status =	get_adapter_status,
  	.get_max_bus_speed =	get_max_bus_speed,
  	.get_cur_bus_speed =	get_cur_bus_speed,
};

static int init_slots(struct controller *ctrl)
{
	struct slot *new_slot;
	u8 number_of_slots;
	u8 slot_device;
	u32 slot_number;
	int result;

	dbg("%s\n",__FUNCTION__);

	number_of_slots = ctrl->num_slots;
	slot_device = ctrl->slot_device_offset;
	slot_number = ctrl->first_slot;

	while (number_of_slots) {
		new_slot = (struct slot *) kmalloc(sizeof(struct slot), GFP_KERNEL);
		if (!new_slot)
			return -ENOMEM;

		memset(new_slot, 0, sizeof(struct slot));
		new_slot->hotplug_slot = kmalloc (sizeof (struct hotplug_slot), GFP_KERNEL);
		if (!new_slot->hotplug_slot) {
			kfree (new_slot);
			return -ENOMEM;
		}
		memset(new_slot->hotplug_slot, 0, sizeof (struct hotplug_slot));

		new_slot->hotplug_slot->info = kmalloc (sizeof (struct hotplug_slot_info), GFP_KERNEL);
		if (!new_slot->hotplug_slot->info) {
			kfree (new_slot->hotplug_slot);
			kfree (new_slot);
			return -ENOMEM;
		}
		memset(new_slot->hotplug_slot->info, 0, sizeof (struct hotplug_slot_info));
		new_slot->hotplug_slot->name = kmalloc (SLOT_NAME_SIZE, GFP_KERNEL);
		if (!new_slot->hotplug_slot->name) {
			kfree (new_slot->hotplug_slot->info);
			kfree (new_slot->hotplug_slot);
			kfree (new_slot);
			return -ENOMEM;
		}

		new_slot->magic = SLOT_MAGIC;
		new_slot->ctrl = ctrl;
		new_slot->bus = ctrl->slot_bus;
		new_slot->device = slot_device;
		new_slot->hpc_ops = ctrl->hpc_ops;

		new_slot->number = ctrl->first_slot;
		new_slot->hp_slot = slot_device - ctrl->slot_device_offset;

		/* register this slot with the hotplug pci core */
		new_slot->hotplug_slot->private = new_slot;
		make_slot_name (new_slot->hotplug_slot->name, SLOT_NAME_SIZE, new_slot);
		new_slot->hotplug_slot->ops = &pciehp_hotplug_slot_ops;

		new_slot->hpc_ops->get_power_status(new_slot, &(new_slot->hotplug_slot->info->power_status));
		new_slot->hpc_ops->get_attention_status(new_slot, &(new_slot->hotplug_slot->info->attention_status));
		new_slot->hpc_ops->get_latch_status(new_slot, &(new_slot->hotplug_slot->info->latch_status));
		new_slot->hpc_ops->get_adapter_status(new_slot, &(new_slot->hotplug_slot->info->adapter_status));

		dbg("Registering bus=%x dev=%x hp_slot=%x sun=%x slot_device_offset=%x\n", 
			new_slot->bus, new_slot->device, new_slot->hp_slot, new_slot->number, ctrl->slot_device_offset);
		result = pci_hp_register (new_slot->hotplug_slot);
		if (result) {
			err ("pci_hp_register failed with error %d\n", result);
			kfree (new_slot->hotplug_slot->info);
			kfree (new_slot->hotplug_slot->name);
			kfree (new_slot->hotplug_slot);
			kfree (new_slot);
			return result;
		}

		new_slot->next = ctrl->slot;
		ctrl->slot = new_slot;

		number_of_slots--;
		slot_device++;
		slot_number += ctrl->slot_num_inc;
	}

	return(0);
}


static int cleanup_slots (struct controller * ctrl)
{
	struct slot *old_slot, *next_slot;

	old_slot = ctrl->slot;
	ctrl->slot = NULL;

	while (old_slot) {
		next_slot = old_slot->next;
		pci_hp_deregister (old_slot->hotplug_slot);
		kfree(old_slot->hotplug_slot->info);
		kfree(old_slot->hotplug_slot->name);
		kfree(old_slot->hotplug_slot);
		kfree(old_slot);
		old_slot = next_slot;
	}


	return(0);
}

static int get_ctlr_slot_config(struct controller *ctrl)
{
	int num_ctlr_slots;		/* Not needed; PCI Express has 1 slot per port */
	int first_device_num;	 	/* Not needed */
	int physical_slot_num;
	int updown;			/* Not needed */
	int rc;
	int flags;			/* Not needed */

	rc = pcie_get_ctlr_slot_config(ctrl, &num_ctlr_slots, &first_device_num, &physical_slot_num, &updown, &flags);
	if (rc) {
		err("%s: get_ctlr_slot_config fail for b:d (%x:%x)\n", __FUNCTION__, ctrl->bus, ctrl->device);
		return (-1);
	}

	ctrl->num_slots = num_ctlr_slots;	/* PCI Express has 1 slot per port */
	ctrl->slot_device_offset = first_device_num;
	ctrl->first_slot = physical_slot_num;
	ctrl->slot_num_inc = updown; 		/* Not needed */	/* either -1 or 1 */

	dbg("%s: bus(0x%x) num_slot(0x%x) 1st_dev(0x%x) psn(0x%x) updown(%d) for b:d (%x:%x)\n",
		__FUNCTION__, ctrl->slot_bus, num_ctlr_slots, first_device_num, physical_slot_num, updown, 
		ctrl->bus, ctrl->device);

	return (0);
}


/*
 * set_attention_status - Turns the Amber LED for a slot on, off or blink
 */
static int set_attention_status (struct hotplug_slot *hotplug_slot, u8 status)
{
	struct slot *slot = get_slot (hotplug_slot, __FUNCTION__);

	if (slot == NULL)
		return -ENODEV;
	
	dbg("%s - physical_slot = %s\n", __FUNCTION__, hotplug_slot->name);

	hotplug_slot->info->attention_status = status;
	slot->hpc_ops->set_attention_status(slot, status);


	return 0;
}


static int enable_slot (struct hotplug_slot *hotplug_slot)
{
	struct slot *slot = get_slot (hotplug_slot, __FUNCTION__);
	
	if (slot == NULL)
		return -ENODEV;

	dbg("%s - physical_slot = %s\n", __FUNCTION__, hotplug_slot->name);

	return pciehp_enable_slot(slot);
}


static int disable_slot (struct hotplug_slot *hotplug_slot)
{
	struct slot *slot = get_slot (hotplug_slot, __FUNCTION__);
	
	if (slot == NULL)
		return -ENODEV;

	dbg("%s - physical_slot = %s\n", __FUNCTION__, hotplug_slot->name);

	return pciehp_disable_slot(slot);
}


static int hardware_test (struct hotplug_slot *hotplug_slot, u32 value)
{
	return 0;
}


static int get_power_status (struct hotplug_slot *hotplug_slot, u8 *value)
{
	struct slot *slot = get_slot (hotplug_slot, __FUNCTION__);
	int retval;
	
	if (slot == NULL)
		return -ENODEV;
	
	dbg("%s - physical_slot = %s\n", __FUNCTION__, hotplug_slot->name);

	retval = slot->hpc_ops->get_power_status(slot, value);
	if (retval < 0)
		*value = hotplug_slot->info->power_status;

	return 0;
}

static int get_attention_status (struct hotplug_slot *hotplug_slot, u8 *value)
{
	struct slot *slot = get_slot (hotplug_slot, __FUNCTION__);
	int retval;
	
	if (slot == NULL)
		return -ENODEV;
	
	dbg("%s - physical_slot = %s\n", __FUNCTION__, hotplug_slot->name);

	retval = slot->hpc_ops->get_attention_status(slot, value);
	if (retval < 0)
		*value = hotplug_slot->info->attention_status;

	return 0;
}

static int get_latch_status (struct hotplug_slot *hotplug_slot, u8 *value)
{
	struct slot *slot = get_slot (hotplug_slot, __FUNCTION__);
	int retval;
	
	if (slot == NULL)
		return -ENODEV;
	
	dbg("%s - physical_slot = %s\n", __FUNCTION__, hotplug_slot->name);

	retval = slot->hpc_ops->get_latch_status(slot, value);
	if (retval < 0)
		*value = hotplug_slot->info->latch_status;

	return 0;
}

static int get_adapter_status (struct hotplug_slot *hotplug_slot, u8 *value)
{
	struct slot *slot = get_slot (hotplug_slot, __FUNCTION__);
	int retval;
	
	if (slot == NULL)
		return -ENODEV;

	dbg("%s - physical_slot = %s\n", __FUNCTION__, hotplug_slot->name);

	retval = slot->hpc_ops->get_adapter_status(slot, value);

	if (retval < 0)
		*value = hotplug_slot->info->adapter_status;

	return 0;
}

static int get_max_bus_speed (struct hotplug_slot *hotplug_slot, enum pci_bus_speed *value)
{
	struct slot *slot = get_slot (hotplug_slot, __FUNCTION__);
	int retval;
	
	if (slot == NULL)
		return -ENODEV;

	dbg("%s - physical_slot = %s\n", __FUNCTION__, hotplug_slot->name);
	
	retval = slot->hpc_ops->get_max_bus_speed(slot, value);
	if (retval < 0)
		*value = PCI_SPEED_UNKNOWN;

	return 0;
}

static int get_cur_bus_speed (struct hotplug_slot *hotplug_slot, enum pci_bus_speed *value)
{
	struct slot *slot = get_slot (hotplug_slot, __FUNCTION__);
	int retval;
	
	if (slot == NULL)
		return -ENODEV;

	dbg("%s - physical_slot = %s\n", __FUNCTION__, hotplug_slot->name);
	
	retval = slot->hpc_ops->get_cur_bus_speed(slot, value);
	if (retval < 0)
		*value = PCI_SPEED_UNKNOWN;

	return 0;
}

static int pcie_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	int rc;
	struct controller *ctrl;
	struct slot *t_slot;
	int first_device_num = 0;	/* first PCI device number supported by this PCIE */  
	int num_ctlr_slots;		/* number of slots supported by this HPC */
	u8 value;

	ctrl = (struct controller *) kmalloc(sizeof(struct controller), GFP_KERNEL);
	if (!ctrl) {
		err("%s : out of memory\n", __FUNCTION__);
		goto err_out_none;
	}
	memset(ctrl, 0, sizeof(struct controller));

	dbg("%s: DRV_thread pid = %d\n", __FUNCTION__, current->pid);

	rc = pcie_init(ctrl, pdev,
		(php_intr_callback_t) pciehp_handle_attention_button,
		(php_intr_callback_t) pciehp_handle_switch_change,
		(php_intr_callback_t) pciehp_handle_presence_change,
		(php_intr_callback_t) pciehp_handle_power_fault);
	if (rc) {
		dbg("%s: controller initialization failed\n", PCIE_MODULE_NAME);
		goto err_out_free_ctrl;
	}

	ctrl->pci_dev = pdev;

	ctrl->pci_bus = kmalloc (sizeof (*ctrl->pci_bus), GFP_KERNEL);
	if (!ctrl->pci_bus) {
		err("%s: out of memory\n", __FUNCTION__);
		rc = -ENOMEM;
		goto err_out_unmap_mmio_region;
	}
	dbg("%s: ctrl->pci_bus %p\n", __FUNCTION__, ctrl->pci_bus);
	memcpy (ctrl->pci_bus, pdev->bus, sizeof (*ctrl->pci_bus));

	ctrl->bus = pdev->bus->number;  /* ctrl bus */
	ctrl->slot_bus = pdev->subordinate->number;  /* bus controlled by this HPC */

	ctrl->device = PCI_SLOT(pdev->devfn);
	ctrl->function = PCI_FUNC(pdev->devfn);
	dbg("%s: ctrl bus=0x%x, device=%x, function=%x, irq=%x\n", __FUNCTION__,
		ctrl->bus, ctrl->device, ctrl->function, pdev->irq);

	/*
	 *	Save configuration headers for this and subordinate PCI buses
	 */

	rc = get_ctlr_slot_config(ctrl);
	if (rc) {
		err(msg_initialization_err, rc);
		goto err_out_free_ctrl_bus;
	}
	
	first_device_num = ctrl->slot_device_offset; 
	num_ctlr_slots = ctrl->num_slots; 
	

	/* Store PCI Config Space for all devices on this bus */
	dbg("%s: Before calling pciehp_save_config, ctrl->bus %x,ctrl->slot_bus %x\n", 
		__FUNCTION__,ctrl->bus, ctrl->slot_bus);
	rc = pciehp_save_config(ctrl, ctrl->slot_bus, num_ctlr_slots, first_device_num); 
	if (rc) {
		err("%s: unable to save PCI configuration data, error %d\n", __FUNCTION__, rc);
		goto err_out_free_ctrl_bus;
	}

	/* Get IO, memory, and IRQ resources for new devices */
	rc = pciehprm_find_available_resources(ctrl);

	ctrl->add_support = !rc;
	if (rc) {
		dbg("pciehprm_find_available_resources = %#x\n", rc);
		err("unable to locate PCI configuration resources for hot plug add.\n");
		goto err_out_free_ctrl_bus;
	}
	/* Setup the slot information structures */
	rc = init_slots(ctrl);
	if (rc) {
		err(msg_initialization_err, 6);
		goto err_out_free_ctrl_slot;
	}

	t_slot = pciehp_find_slot(ctrl, first_device_num);
	dbg("%s: t_slot %p\n", __FUNCTION__, t_slot);

	/* Finish setting up the hot plug ctrl device */
	ctrl->next_event = 0;

	if (!pciehp_ctrl_list) {
		pciehp_ctrl_list = ctrl;
		ctrl->next = NULL;
	} else {
		ctrl->next = pciehp_ctrl_list;
		pciehp_ctrl_list = ctrl;
	}

	/* Wait for exclusive access to hardware */
	down(&ctrl->crit_sect);

	t_slot->hpc_ops->get_adapter_status(t_slot, &value); /* Check if slot is occupied */
	dbg("%s: adapter value %x\n", __FUNCTION__, value);
	if (!value) {
		rc = t_slot->hpc_ops->power_off_slot(t_slot);  /* Power off slot if not occupied*/
		if (rc) {
			up(&ctrl->crit_sect);
			goto err_out_free_ctrl_slot;
		} else
			/* Wait for the command to complete */
			wait_for_ctrl_irq (ctrl);
		}
	
	/* Done with exclusive hardware access */
	up(&ctrl->crit_sect);

	return 0;

err_out_free_ctrl_slot:
	cleanup_slots(ctrl);
err_out_free_ctrl_bus:
	kfree(ctrl->pci_bus);
err_out_unmap_mmio_region:
	ctrl->hpc_ops->release_ctlr(ctrl);
err_out_free_ctrl:
	kfree(ctrl);
err_out_none:
	return -ENODEV;
}


static int pcie_start_thread(void)
{
	int loop;
	int retval = 0;
	
	dbg("Initialize + Start the notification/polling mechanism \n");

	retval = pciehp_event_start_thread();
	if (retval) {
		dbg("pciehp_event_start_thread() failed\n");
		return retval;
	}

	dbg("Initialize slot lists\n");
	/* One slot list for each bus in the system */
	for (loop = 0; loop < 256; loop++) {
		pciehp_slot_list[loop] = NULL;
	}

	return retval;
}


static void unload_pciehpd(void)
{
	struct pci_func *next;
	struct pci_func *TempSlot;
	int loop;
	struct controller *ctrl;
	struct controller *tctrl;
	struct pci_resource *res;
	struct pci_resource *tres;

	ctrl = pciehp_ctrl_list;
	while (ctrl) {
		cleanup_slots(ctrl);

		res = ctrl->io_head;
		while (res) {
			tres = res;
			res = res->next;
			kfree(tres);
		}

		res = ctrl->mem_head;
		while (res) {
			tres = res;
			res = res->next;
			kfree(tres);
		}

		res = ctrl->p_mem_head;
		while (res) {
			tres = res;
			res = res->next;
			kfree(tres);
		}

		res = ctrl->bus_head;
		while (res) {
			tres = res;
			res = res->next;
			kfree(tres);
		}

		kfree (ctrl->pci_bus);

		ctrl->hpc_ops->release_ctlr(ctrl);

		tctrl = ctrl;
		ctrl = ctrl->next;

		kfree(tctrl);
	}

	for (loop = 0; loop < 256; loop++) {
		next = pciehp_slot_list[loop];
		while (next != NULL) {
			res = next->io_head;
			while (res) {
				tres = res;
				res = res->next;
				kfree(tres);
			}

			res = next->mem_head;
			while (res) {
				tres = res;
				res = res->next;
				kfree(tres);
			}

			res = next->p_mem_head;
			while (res) {
				tres = res;
				res = res->next;
				kfree(tres);
			}

			res = next->bus_head;
			while (res) {
				tres = res;
				res = res->next;
				kfree(tres);
			}

			TempSlot = next;
			next = next->next;
			kfree(TempSlot);
		}
	}

	/* Stop the notification mechanism */
	pciehp_event_stop_thread();

}


static struct pci_device_id pcied_pci_tbl[] __devinitdata = {
	{
	class:      	((PCI_CLASS_BRIDGE_PCI << 8) | 0x00),
	class_mask:	~0,
	vendor:		PCI_ANY_ID,
	device:     	PCI_ANY_ID,
	subvendor:   	PCI_ANY_ID,
	subdevice:    	PCI_ANY_ID,
	},
	{ /* end: all zeroes */ }
};

MODULE_DEVICE_TABLE(pci, pcied_pci_tbl);



static struct pci_driver pcie_driver = {
	.name		=	PCIE_MODULE_NAME,
	.id_table	=	pcied_pci_tbl,
	.probe		=	pcie_probe,
	/* remove:	pcie_remove_one, */
};



static int __init pcied_init(void)
{
	int retval = 0;

#ifdef CONFIG_HOTPLUG_PCI_PCIE_POLL_EVENT_MODE
	pciehp_poll_mode = 1;
#endif
	retval = pcie_start_thread();
	if (retval)
		goto error_hpc_init;

	retval = pciehprm_init(PCI);
	if (!retval) {
		retval = pci_module_init(&pcie_driver);
		dbg("pci_module_init = %d\n", retval);
		info(DRIVER_DESC " version: " DRIVER_VERSION "\n");
	}

error_hpc_init:
	if (retval) {
		pciehprm_cleanup();
		pciehp_event_stop_thread();
	} 

	return retval;
}

static void __exit pcied_cleanup(void)
{
	dbg("unload_pciehpd()\n");
	unload_pciehpd();

	pciehprm_cleanup();

	dbg("pci_unregister_driver\n");
	pci_unregister_driver(&pcie_driver);

	info(DRIVER_DESC " version: " DRIVER_VERSION " unloaded\n");
}


module_init(pcied_init);
module_exit(pcied_cleanup);


