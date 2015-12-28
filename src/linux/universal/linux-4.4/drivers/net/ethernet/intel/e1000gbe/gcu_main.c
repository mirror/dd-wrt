/******************************************************************************

GPL LICENSE SUMMARY

  Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.

  This program is free software; you can redistribute it and/or modify 
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
  General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this program; if not, write to the Free Software 
  Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
  The full GNU General Public License is included in this distribution 
  in the file called LICENSE.GPL.

  Contact Information:
  Intel Corporation

 version: Security.L.1.0.3-98

  Contact Information:

  Intel Corporation, 5000 W Chandler Blvd, Chandler, AZ 85226

******************************************************************************/
/**************************************************************************
 * @ingroup GCU_GENERAL
 *
 * @file gcu_main.c
 *
 * @description
 *   This module contains the upper-edge routines of the driver
 *   interface that handle initialization, resets, and shutdowns
 *   of the GCU.
 *
 **************************************************************************/

#include "gcu.h"

char gcu_driver_name[] = "GCU";
char gcu_driver_string[] = "Global Configuration Unit Driver";
#define DRV_VERSION "1.0.0"
char gcu_driver_version[] = DRV_VERSION;
char gcu_copyright[] = "Copyright (c) 1999-2007 Intel Corporation.";

/* gcu_pci_tbl - PCI Device ID Table
 *
 * Last entry must be all 0s
 *
 * Macro expands to...
 *   {PCI_DEVICE(PCI_VENDOR_ID_INTEL, device_id)}
 */
static struct pci_device_id gcu_pci_tbl[] = {
    INTEL_GCU_DEVICE(0x503E),
    /* required last entry */
    {0,}
};

MODULE_DEVICE_TABLE(pci, gcu_pci_tbl);

enum gcu_err_type {err_ioremap, err_alloc_gcu_adapter};

static int gcu_init_module(void);
static void gcu_exit_module(void);
static int gcu_probe(struct pci_dev *pdev, const struct pci_device_id *ent);
static void gcu_probe_err(enum gcu_err_type err, struct pci_dev *pdev,
                          struct gcu_adapter *adapter);
static void gcu_remove(struct pci_dev *pdev);
static int gcu_notify_reboot(struct notifier_block *, unsigned long event,
                             void *ptr);
static int gcu_suspend(struct pci_dev *pdev, uint32_t state);
static struct gcu_adapter *alloc_gcu_adapter(void);
static void free_gcu_adapter(struct gcu_adapter *adapter);


struct notifier_block gcu_notifier_reboot = {
    .notifier_call    = gcu_notify_reboot,
    .next        = NULL,
    .priority    = 0
};

static struct pci_driver gcu_driver = {
    .name     = gcu_driver_name,
    .id_table = gcu_pci_tbl,
    .probe    = gcu_probe,
    .remove   = gcu_remove,
};

static struct gcu_adapter *global_adapter = 0;
DEFINE_SPINLOCK(global_adapter_spinlock);
//static spinlock_t global_adapter_spinlock = SPIN_LOCK_UNLOCKED;
static unsigned long g_intflags = 0;

MODULE_AUTHOR("Intel(R) Corporation");
MODULE_DESCRIPTION("Global Configuration Unit Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.0");

/*
 * AFU: debug values are pulled originally from netdevice.h
 * where in the orig iegbe driver, the enums were used.
 * Probably want to come up with our own enum set
 */
static int debug = 0x1UL | 0x2UL;
module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "Debug level (0=none,...,16=all)");

/**
 * gcu_init_module - Driver Registration Routine
 *
 * gcu_init_module is the first routine called when the driver is
 * loaded. All it does is register with the PCI subsystem.
 **/
static int __init
gcu_init_module(void)
{
    int ret;
    printk(KERN_INFO "%s - version %s\n",
           gcu_driver_string, gcu_driver_version);

    printk(KERN_INFO "%s\n", gcu_copyright);

    ret = pci_register_driver(&gcu_driver);
    if(ret >= 0) {
        register_reboot_notifier(&gcu_notifier_reboot);
    }
    return ret;
}

module_init(gcu_init_module);

/**
 * gcu_exit_module - Driver Exit Cleanup Routine
 *
 * gcu_exit_module is called just before the driver is removed
 * from memory.
 **/
static void __exit
gcu_exit_module(void)
{
    GCU_DBG("%s\n", __func__);

    unregister_reboot_notifier(&gcu_notifier_reboot);
    pci_unregister_driver(&gcu_driver);
}

module_exit(gcu_exit_module);


/**
 * gcu_probe - Device Initialization Routine
 * @pdev: PCI device information struct
 * @ent: entry in gcu_pci_tbl
 *
 * Returns 0 on success, negative on failure
 *
 * gcu_probe initializes an adapter identified by a pci_dev structure.
 * The OS initialization, configuring of the adapter private structure,
 * and a hardware reset occur.
 **/
static int
gcu_probe(struct pci_dev *pdev,
          const struct pci_device_id *ent)
{
    struct gcu_adapter *adapter=0;
    uint32_t mmio_start, mmio_len;
    int err;

    GCU_DBG("%s\n", __func__);

    if((err = pci_enable_device(pdev)))
    {
        GCU_DBG("Unable to enable PCI Device\n");
        return err;
    }

    if((err = pci_request_regions(pdev, gcu_driver_name)))
    {
        GCU_DBG("Unable to acquire requested memory regions\n");
        return err;
    }

    /*
     * acquire the adapter spinlock. Once the module is loaded, it is possible for
     * someone to access the adapter struct via the interface functions exported
     * in gcu_if.c
     */
    spin_lock(&global_adapter_spinlock);

    adapter = alloc_gcu_adapter();
    if(!adapter)
    {
        gcu_probe_err(err_alloc_gcu_adapter, pdev, adapter);
        spin_unlock(&global_adapter_spinlock);
        return -ENOMEM;
    }

    pci_set_drvdata(pdev, adapter);

    adapter->pdev = pdev;
    adapter->msg_enable = (1 << debug) - 1;

    mmio_start = pci_resource_start(pdev, BAR_0);
    mmio_len = pci_resource_len(pdev, BAR_0);

    adapter->hw_addr = ioremap(mmio_start, mmio_len);
    if(!adapter->hw_addr) {
        GCU_DBG("Unable to map mmio\n");
        gcu_probe_err(err_ioremap, pdev, adapter);
        spin_unlock(&global_adapter_spinlock);
        return -EIO;
    }

    strncpy(adapter->name, pci_name(pdev), sizeof(adapter->name)-1);
    adapter->mem_start = mmio_start;
    adapter->mem_end = mmio_start + mmio_len;

    adapter->vendor_id = pdev->vendor;
    adapter->device_id = pdev->device;
    adapter->subsystem_vendor_id = pdev->subsystem_vendor;
    adapter->subsystem_id = pdev->subsystem_device;

    pci_read_config_byte(pdev, PCI_REVISION_ID, &adapter->revision_id);

    pci_read_config_word(pdev, PCI_COMMAND, &adapter->pci_cmd_word);

    global_adapter = adapter;
    spin_unlock(&global_adapter_spinlock);

    DPRINTK(PROBE, INFO, "Intel(R) GCU Initialized\n");

    return 0;
}

/**
 * gcu_probe_err - gcu_probe error handler
 * @err: gcu_err_type
 *
 * encapsulated error handling for gcu_probe
 **/
static void
gcu_probe_err(enum gcu_err_type err, struct pci_dev *pdev,
              struct gcu_adapter *adapter)
{

    switch(err) {
    case err_ioremap:
        iounmap(adapter->hw_addr);
        pci_release_regions(pdev);
    case err_alloc_gcu_adapter:
    default:
        free_gcu_adapter(adapter);
        break;
    }
}


/**
 * gcu_remove - Device Removal Routine
 * @pdev: PCI device information struct
 *
 * gcu_remove is called by the PCI subsystem to alert the driver
 * that it should release a PCI device.  The could be caused by a
 * Hot-Plug event, or because the driver is going to be removed from
 * memory.
 **/
static void
gcu_remove(struct pci_dev *pdev)
{
    struct gcu_adapter *adapter = pci_get_drvdata(pdev);

    GCU_DBG("%s\n", __func__);

    iounmap(adapter->hw_addr);
    pci_release_regions(pdev);
    free_gcu_adapter(adapter);
    pci_set_drvdata(pdev, NULL);
}

static int
gcu_notify_reboot(struct notifier_block *nb, unsigned long event, void *p)
{
    struct pci_dev *pdev = NULL;

    GCU_DBG("%s\n", __func__);

    switch(event) {
    case SYS_DOWN:
    case SYS_HALT:
    case SYS_POWER_OFF:
        while((pdev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, pdev))) {
            if(pci_dev_driver(pdev) == &gcu_driver){
                gcu_suspend(pdev, 0x3);
            }
        }
    }
    return NOTIFY_DONE;
}


/**
 * gcu_suspend - device sleep function
 * @pdev: PCI device information struct
 *
 * gcu_supend is generally called to place a device in sleep mode,
 * however the GCU doesn't support power mangement. For this case,
 * it is part of the gcu_notify_reboot() call chain to quiese the
 * device before a reboot.
 **/
static int
gcu_suspend(struct pci_dev *pdev, uint32_t state)
{
    /*struct gcu_adapter *adapter = pci_get_drvdata(pdev); */
#if ( ( LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,6) ) && \
      ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10) ) )
    struct net_device *netdev = pci_get_drvdata(pdev);
    struct gcu_adapter *adapter = netdev_priv(netdev);
#endif

    GCU_DBG("%s\n", __func__);

    pci_save_state(pdev);

    pci_disable_device(pdev);

    state = (state > 0) ? 0 : 0;

    /*
     * GCU doesn't support power management, but want to
     * leave a hook incase that situation changes in the future
     *
     * pci_set_power_state(pdev, state);
     *
     */

    return state;
}

/**
 * alloc_gcu_adapter
 *
 * alloc_gcu_adapter is a wrapper for the kmalloc call for the
 * device specific data block plus inits the global_adapter variable.
 *
 * Note that this function assumes that the spinlock for the global
 * gcu_adapter struct as been acquired.
 **/
static struct gcu_adapter *
alloc_gcu_adapter()
{
    struct gcu_adapter *adapter;

    GCU_DBG("%s\n", __func__);

    adapter = (struct gcu_adapter*) kmalloc(sizeof(*adapter), GFP_KERNEL);

    global_adapter = adapter;

    if(!adapter)
    {
        GCU_DBG("Unable to allocate space for global gcu_adapter");
        return 0;
    }

    memset(adapter, 0, sizeof(*adapter));

    return adapter;
}


/**
 * free_gcu_adapter
 * @adapter: gcu_adapter struct to be free'd
 *
 * free_gcu_adapter is a wrapper for the kfree call for the
 * device specific data block plus clears the global_adapter variable
 *
 * Note that this function assumes that the spinlock for the global
 * gcu_adapter struct as been acquired.
 **/
static void
free_gcu_adapter(struct gcu_adapter *adapter)
{
    GCU_DBG("%s\n", __func__);

    global_adapter = 0;

    if(adapter){
        kfree(adapter);
    }
}


/**
 * gcu_get_adapter
 *
 * gcu_get_adapter is used by the functions exported in gcu_if.c to get
 * access to the memory addresses needed to access the MMIO registers
 * of the GCU
 **/
const struct gcu_adapter *
gcu_get_adapter(void)
{
    GCU_DBG("%s\n", __func__);

    if(global_adapter == NULL)
    {
        GCU_DBG("global gcu_adapter is not available\n");
        return NULL;
    }

    spin_lock_irqsave(&global_adapter_spinlock, g_intflags);

    return global_adapter;
}

/**
 * gcu_release_adapter
 *
 * gcu_release_adapter is used by the functions exported in gcu_if.c to get
 * release the adapter spinlock and the handle to the adapter
 **/
void
gcu_release_adapter(const struct gcu_adapter **adapter)
{
    GCU_DBG("%s\n", __func__);

    if(adapter == NULL)
    {
        GCU_ERR("global gcu_adapter handle is invalid\n");
    }
    else
    {
        *adapter = 0;
    }

    spin_unlock_irqrestore(&global_adapter_spinlock, g_intflags);

    return;
}

/* gcu_main.c */

