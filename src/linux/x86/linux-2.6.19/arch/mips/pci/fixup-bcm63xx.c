/*
<:copyright-gpl 
 Copyright 2002 Broadcom Corp. All Rights Reserved. 
 
 This program is free software; you can distribute it and/or modify it 
 under the terms of the GNU General Public License (Version 2) as 
 published by the Free Software Foundation. 
 
 This program is distributed in the hope it will be useful, but WITHOUT 
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
 for more details. 
 
 You should have received a copy of the GNU General Public License along 
 with this program; if not, write to the Free Software Foundation, Inc., 
 59 Temple Place - Suite 330, Boston MA 02111-1307, USA. 
:>
*/
#include <linux/init.h>
#include <linux/types.h>
#include <linux/pci.h>

#include <bcmpci.h>
#include <bcm_intr.h>
#include <bcm_map_part.h>


static char irq_tab_bcm63xx[] __initdata = {
    [0] = INTERRUPT_ID_MPI,
    [1] = INTERRUPT_ID_MPI,
    [2] = INTERRUPT_ID_MPI,    
#if defined(CONFIG_USB)
    [USB_HOST_SLOT] = INTERRUPT_ID_USBH,
#if !defined(CONFIG_BCM96348)
    [USB20_HOST_SLOT] = INTERRUPT_ID_USBH20
#endif
#endif
};

int __init pcibios_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
    return irq_tab_bcm63xx[slot];
}

static void bcm63xx_fixup(struct pci_dev *dev)
{
    uint32 memaddr;
    uint32 size;
    uint32 resno;

    memaddr = pci_resource_start(dev, 0);
    size = pci_resource_len(dev, 0);

    switch (PCI_SLOT(dev->devfn)) {
        case 0:
            // Move device in slot 0 to a different memory range
            // In case this is a CB device, it will be accessed via l2pmremap1
            // which will have CARDBUS_MEM bit set
            for (resno = 0; resno < 6; resno++) {
                if (dev->resource[resno].end && (dev->resource[resno].start < BCM_CB_MEM_BASE)) {
                    dev->resource[resno].start += (BCM_CB_MEM_BASE - BCM_PCI_MEM_BASE);
                    dev->resource[resno].end += (BCM_CB_MEM_BASE - BCM_PCI_MEM_BASE);
                }
            }
            break;

#if defined(CONFIG_USB)
        case USB_HOST_SLOT:
            dev->resource[0].start = USB_OHCI_BASE;
            dev->resource[0].end = USB_OHCI_BASE+USB_BAR0_MEM_SIZE-1;
            break;

#if !defined(CONFIG_BCM96348)
        case USB20_HOST_SLOT:
            dev->resource[0].start = USB_EHCI_BASE;
            dev->resource[0].end = USB_EHCI_BASE+USB_BAR0_MEM_SIZE-1;
            break;
#endif
#endif
    }
}
/*
struct pci_fixup pcibios_fixups[] = {
    { PCI_FIXUP_FINAL, PCI_ANY_ID, PCI_ANY_ID, bcm63xx_fixup },
    {0}
};*/
DECLARE_PCI_FIXUP_FINAL(PCI_ANY_ID, PCI_ANY_ID,
	 bcm63xx_fixup );
