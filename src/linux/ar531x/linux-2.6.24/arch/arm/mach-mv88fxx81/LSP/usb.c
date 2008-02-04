/*
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
 */
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
                                                                                                                             
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/mach/pci.h>
#include <asm/arch/irqs.h>

#include "mvDebug.h"
#include "mvCtrlEnvLib.h"
#include "mvUsb.h"

extern u32 mvIsUsbHost;

#define MV_USB_DEV_REV      0
#define MV_USB_DMA_MASK     0xffffffff

static char usb_dev_name[]  = "mv_gadget";
static char usb_host_name[] = "ehci_platform";
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
    int		                status, dev, num, isHost;
    char*                   name_ptr;
    struct platform_device* mv_usb_dev_ptr;

    if(mvCtrlModelRevGet() == MV_5181L_A0_ID) {
        /* metal problem */
        return 0;
    }

    num = mvCtrlUsbMaxGet(); 
    for(dev=0; dev<num; dev++)
    {
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
        mv_usb_dev_ptr->id                 = PCI_VENDOR_ID_MARVELL | (MV_USB_DEV_REV << 16);

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

