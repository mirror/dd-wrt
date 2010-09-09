#include <linux/init.h>
#include <linux/pci.h>
#include <linux/ath9k_platform.h>
#include "ar7240.h"

/*
 * PCI IRQ map
 */
int __init pcibios_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
    //printk(KERN_EMERG "fixing irq for slot %d pin %d\n", slot, pin);

#if 1
    printk("Returning IRQ %d\n", AR7240_PCI_IRQ_DEV0);
    return AR7240_PCI_IRQ_DEV0;
#else
    switch(slot)
    {
        case 0:
            return AR7240_PCI_IRQ_DEV0;
        case 1:
            return AR7240_PCI_IRQ_DEV1;
        case 2:
            return AR7240_PCI_IRQ_DEV2;
        default:
            printk("unknown slot!\n");
            return -1;
    }
#endif
}

static struct ath9k_platform_data ap91_wmac0_data;

#define CALDATA0_OFFSET		0x1000

int 
pcibios_plat_dev_init(struct pci_dev *dev)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ap91_wmac0_data.slot = 0;

	printk(KERN_EMERG "init ATH9k WMAC %d\n",PCI_SLOT(dev->devfn));
	switch(PCI_SLOT(dev->devfn)) {
	case 0:
		memcpy(ap91_wmac0_data.eeprom_data, art + CALDATA0_OFFSET,sizeof(ap91_wmac0_data.eeprom_data));
		dev->dev.platform_data = &ap91_wmac0_data;
		break;
	}
        return 0;
}

