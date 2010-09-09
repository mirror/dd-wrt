#include <linux/init.h>
#include <linux/pci.h>
#include <linux/ath9k_platform.h>
#include "ar7100.h"

/*
 * PCI IRQ map
 */
int __init pcibios_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
    pr_debug("fixing irq for slot %d pin %d\n", slot, pin);

#ifdef CONFIG_AR7100_EMULATION
    printk("Returning IRQ %d\n", AR7100_PCI_IRQ_DEV0);
    return AR7100_PCI_IRQ_DEV0;
#else
    switch(slot)
    {
        case 0:
            return AR7100_PCI_IRQ_DEV0;
        case 1:
            return AR7100_PCI_IRQ_DEV1;
        case 2:
            return AR7100_PCI_IRQ_DEV2;
        default:
            printk("unknown slot!\n");
            return -1;
    }
#endif
}
static struct ath9k_platform_data ap94_wmac0_data;
static struct ath9k_platform_data ap94_wmac1_data;

#define CALDATA0_OFFSET		0x1000
#define CALDATA1_OFFSET		0x5000
 

int 
pcibios_plat_dev_init(struct pci_dev *dev)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ap94_wmac0_data.slot = 0;
	ap94_wmac1_data.slot = 1;

	printk(KERN_EMERG "init ATH9k WMAC %d\n",PCI_SLOT(dev->devfn));
	switch(PCI_SLOT(dev->devfn)) {
	case 0:
		memcpy(ap94_wmac0_data.eeprom_data, art + CALDATA0_OFFSET,sizeof(ap94_wmac0_data.eeprom_data));
		dev->dev.platform_data = &ap94_wmac0_data;
		break;

	case 1:
		memcpy(ap94_wmac1_data.eeprom_data, art + CALDATA1_OFFSET,sizeof(ap94_wmac1_data.eeprom_data));
		dev->dev.platform_data = &ap94_wmac1_data;
		break;
	}
        return 0;
}

