/*****************************************************************************

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

*****************************************************************************/


/* Linux GCU Driver main header file */

#ifndef _GCU_H_
#define _GCU_H_

#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <asm/bitops.h>
#include <linux/slab.h>
#include <linux/reboot.h>
#include <asm/delay.h>

#define BAR_0		0

#include "kcompat.h"

#define INTEL_GCU_DEVICE(device_id) {\
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, device_id)}

#define GCU_DEV_NAME_SIZE 16

#ifdef DBG
#define GCU_DBG(args...) printk(KERN_DEBUG "gcu: " args)
#else
#define GCU_DBG(args...)
#endif

#define GCU_ERR(args...) printk(KERN_ERR "gcu: " args)

#define PFX "gcu: "
#define DPRINTK(nlevel, klevel, fmt, args...) \
	(void)((NETIF_MSG_##nlevel & adapter->msg_enable) && \
	printk(KERN_##klevel PFX "%s: %s: " fmt, adapter->name, \
		__FUNCTION__ , ## args))

struct gcu_adapter {
    struct pci_dev *pdev;
    uint32_t mem_start;
    uint32_t mem_end;
    uint32_t base_addr;
    uint8_t *hw_addr;
    char name[GCU_DEV_NAME_SIZE];
    uint32_t pci_state[16];
    int32_t msg_enable;
    uint16_t device_id;
    uint16_t vendor_id;
    uint16_t subsystem_id;
    uint16_t subsystem_vendor_id;
    uint16_t pci_cmd_word;
    uint8_t revision_id;
    /* open/release and usage marking */
    struct module *owner;

};

/* 
 * Exported interface functions need access to the modules 
 * gcu_adapter struct 
 */
const struct gcu_adapter *gcu_get_adapter(void);
void gcu_release_adapter(const struct gcu_adapter **adapter);

#endif /* _GCU_H_ */
 
