/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.


********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
*******************************************************************************/

#include <linux/autoconf.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/init.h>
#include <linux/platform_device.h>
                                                                                                                             
#include <asm/io.h>
#include <asm/irq.h>

#if defined(CONFIG_MV645xx)
#   include "marvell_pic.h"
#endif /* CONFIG_MV645xx */

#include "mvDebug.h"
#include "ctrlEnv/mvCtrlEnvLib.h"
#include "ctrlEnv/sys/mvSysUsb.h"
#include "usb/mvUsb.h"

extern u32 mvIsUsbHost;

#define MV_USB_DMA_MASK     0xffffffff

static char usb_dev_name[]  = "mv_udc";
static char usb_host_name[] = "ehci_marvell";
static char usb_bus_name[]  = "platform";


static void mv_usb_release(struct device *dev)
{
    struct platform_device  *pdev = to_platform_device(dev); 

    /* normally not freed */
    printk("mv_usb_release\n");

    kfree(pdev->resource);
    kfree(pdev->dev.dma_mask);
    kfree(pdev);
} 


static int __init   mv_usb_init(void)
{
    int                     status, dev, num, isHost;
    char*                   name_ptr;
    struct platform_device* mv_usb_dev_ptr;

    if(mvCtrlModelRevGet() == MV_5181L_A0_ID) {
        /* metal problem */
        return 0;
    }

    num = mvCtrlUsbMaxGet(); 
    for(dev=0; dev<num; dev++)
    {
		
		if (MV_FALSE == mvCtrlPwrClckGet(USB_UNIT_ID, dev))
		{
			printk("\nWarning Integrated USB %d is Powered Off\n",dev);
			continue;
			
		}

        isHost = mvIsUsbHost & (1 << dev);

        if(isHost)
        {        
            name_ptr = usb_host_name;
        }
        else
        {
            name_ptr = usb_dev_name;
        }

        status = mvUsbInit(dev, isHost);

        mv_usb_dev_ptr = kmalloc(sizeof(struct platform_device), GFP_KERNEL);
        if(mv_usb_dev_ptr == NULL)
        {
            printk("Can't allocate platform_device structure - %d bytes\n",
                    sizeof(struct platform_device) );
            return 1;
        }
        memset(mv_usb_dev_ptr, 0, sizeof(struct platform_device) );

        mv_usb_dev_ptr->name               = name_ptr;
        mv_usb_dev_ptr->id                 = PCI_VENDOR_ID_MARVELL | (MV_USB_VERSION << 16) | (dev << 24);

        mv_usb_dev_ptr->num_resources  = 2;

        mv_usb_dev_ptr->resource = (struct resource*)kmalloc(2*sizeof(struct resource), GFP_KERNEL);
        if(mv_usb_dev_ptr->resource == NULL)
        {
            printk("Can't allocate 2 resource structure - %d bytes\n",
                    2*sizeof(struct resource) );
            return 1;
        }
        memset(mv_usb_dev_ptr->resource, 0, 2*sizeof(struct resource));

        mv_usb_dev_ptr->resource[0].start = 
                        ( INTER_REGS_BASE | MV_USB_CORE_CAP_LENGTH_REG(dev));
        mv_usb_dev_ptr->resource[0].end   = 
                        ((INTER_REGS_BASE | MV_USB_CORE_CAP_LENGTH_REG(dev)) + 4096);
        mv_usb_dev_ptr->resource[0].flags = IORESOURCE_DMA;

        mv_usb_dev_ptr->resource[1].start = IRQ_USB_CTRL(dev);
        mv_usb_dev_ptr->resource[1].flags = IORESOURCE_IRQ;

        mv_usb_dev_ptr->dev.dma_mask           = kmalloc(sizeof(u64), GFP_KERNEL);
        *mv_usb_dev_ptr->dev.dma_mask          = MV_USB_DMA_MASK;

        mv_usb_dev_ptr->dev.coherent_dma_mask  = ~0;
        mv_usb_dev_ptr->dev.release            = mv_usb_release;
        strncpy(mv_usb_dev_ptr->dev.bus_id, usb_bus_name, BUS_ID_SIZE);

        printk("Marvell USB EHCI %s controller #%d: %p\n", 
                isHost ? "Host" : "Gadget", dev, mv_usb_dev_ptr); 
        
        status = platform_device_register(mv_usb_dev_ptr);
        if (status)
        {
            printk("Can't register Marvell USB EHCI controller #%d, status=%d\n", 
                        dev, status);
            return status;
        }
    }    
    return 0;
}

subsys_initcall(mv_usb_init);

