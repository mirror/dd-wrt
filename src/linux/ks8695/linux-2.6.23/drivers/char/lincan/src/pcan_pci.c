/* pcan_pci.c - support for PEAK System PCAN-PCI cards
 * Linux CAN-bus device driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/sja1000p.h"

#ifdef CAN_ENABLE_PCI_SUPPORT

/* This card is based on Infineon's PSB 4600 PITA bridge */

#define PITA_ICR         0x00        /* interrupt control register */
#define PITA_ICR_STAT    0x00        /*   - status/IRQ pending bits */
#define PITA_ICR_IEN     0x02        /*   - IRQ enable bits */
#define PITA_GPIOICR     0x18        /* general purpose IO interface control register */
#define PITA_MISC        0x1C        /* misc register */

#define PCAN_PCI_CONFIG_PORT_SIZE 0x1000  /* size of the config io-memory */
#define PCAN_PCI_PORT_SIZE        0x0400  /* size of a channel io-memory */

#define PCAN_PCI_VENDOR_ID	0x001C	/* PCAN-PCI and clones vednor id */
#define PCAN_PCI_PRODUCT_ID	0x0001	/* PCAN-PCI and clones device ID */

/* Standard value: Pushpull  (OCTP1|OCTN1|OCPOL1|OCTP0|OCTN0|OCM1) */
#define PCAN_PCI_OCR_DEFAULT_STD 0xFA

#define PCAN_PCI_BYTES_PER_CIRCUIT 0x400
#define PCAN_PCI_BYTES_PER_REG     4

/* Conversion of the chip index to IRQ register mask */
static unsigned int pcan_pci_idx2mask[4]={
	0x0002,
	0x0001,
	0x0040,
	0x0080
};

void pcan_pci_disconnect_irq(struct candevice_t *candev)
{
	u16 w;

	/* disable interrupts sources */
	w = can_readw(candev->aux_base_addr + PITA_ICR_IEN);
	can_writew(w & ~0xC3, candev->aux_base_addr + PITA_ICR_IEN);
}

void pcan_pci_connect_irq(struct candevice_t *candev)
{
	u16 w;
	int i;

	/* clear previous accumulated status */
	can_writew(0xC3, candev->aux_base_addr + PITA_ICR_STAT);

	/* enable interrupts sources */
	w = can_readw(candev->aux_base_addr + PITA_ICR_IEN);
	for(i = 0; i < candev->nr_all_chips; i++)
		w |= pcan_pci_idx2mask[i];
	can_writew(w, candev->aux_base_addr + PITA_ICR_IEN);
}


int pcan_pci_request_io(struct candevice_t *candev)
{
	unsigned long ctrl_addr;
	unsigned long io_addr;
	int i;

    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))
	if(pci_request_region(candev->sysdevptr.pcidev, 0, "pcan_pci_ctrl") != 0){
		CANMSG("Request of pcan_pci_ctrl range failed\n");
		return -ENODEV;
	}else if(pci_request_region(candev->sysdevptr.pcidev, 1, "pcan_pci_io") != 0){
		CANMSG("Request of pcan_pci_io range failed\n");
		goto error_io;
	}
    #else /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
	if(pci_request_regions(candev->sysdevptr.pcidev, "pcan_pci") != 0){
		CANMSG("Request of pcan_pci_bridge regions failed\n");
		return -ENODEV;
	}
    #endif /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/

	ctrl_addr=pci_resource_start(candev->sysdevptr.pcidev,0);
	if (!(candev->aux_base_addr = ioremap(ctrl_addr,
	      pci_resource_len(candev->sysdevptr.pcidev,0)))) {
		CANMSG("Unable to access I/O memory at: 0x%lx\n", ctrl_addr);
		goto error_ioremap_ctrl;
	}

	io_addr=pci_resource_start(candev->sysdevptr.pcidev,1);;
	if (!(candev->dev_base_addr = ioremap(io_addr,
	      pci_resource_len(candev->sysdevptr.pcidev,1)))) {
		CANMSG("Unable to access I/O memory at: 0x%lx\n", io_addr);
		goto error_ioremap_io;
	}

	candev->io_addr=io_addr;
	candev->res_addr=ctrl_addr;

	/*
	 * this is redundant with chip initialization, but remap address
	 * can change when resources are temporarily released
	 */
	for(i=0;i<candev->nr_all_chips;i++) {
		struct canchip_t *chip=candev->chip[i];
		if(!chip) continue;
		chip->chip_base_addr = candev->dev_base_addr +
			i * PCAN_PCI_BYTES_PER_CIRCUIT;
		if(!chip->msgobj[0]) continue;
		chip->msgobj[0]->obj_base_addr=chip->chip_base_addr;
	}

	pcan_pci_disconnect_irq(candev);

	/* Configure PITA */
	can_writew(0x0005, candev->aux_base_addr + PITA_GPIOICR + 2);	/* set GPIO control register */

	can_writeb(0x00, candev->aux_base_addr + PITA_GPIOICR);		/* enable all channels */

	can_writeb(0x05, candev->aux_base_addr + PITA_MISC + 3);	/* toggle reset */
	mdelay(5);
	writeb(0x04, candev->aux_base_addr + PITA_MISC + 3);		/* leave parport mux mode */
	wmb();

	return 0;

    error_ioremap_io:
	iounmap(candev->aux_base_addr);
    error_ioremap_ctrl:
    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))
	pci_release_region(candev->sysdevptr.pcidev, 1);
    error_io:
	pci_release_region(candev->sysdevptr.pcidev, 0);
    #else /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
	pci_release_regions(candev->sysdevptr.pcidev);
    #endif /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/

	return -ENODEV;
}

int pcan_pci_release_io(struct candevice_t *candev)
{
	pcan_pci_disconnect_irq(candev);

	iounmap(candev->dev_base_addr);
	iounmap(candev->aux_base_addr);
    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))
	pci_release_region(candev->sysdevptr.pcidev, 1);
	pci_release_region(candev->sysdevptr.pcidev, 0);
    #else /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
	pci_release_regions(candev->sysdevptr.pcidev);
    #endif /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/

	return 0;
}

void pcan_pci_write_register(unsigned data, can_ioptr_t address)
{
	address += ((can_ioptr2ulong(address)&(PCAN_PCI_BYTES_PER_CIRCUIT-1))
					     *(PCAN_PCI_BYTES_PER_REG-1));
	can_writeb(data,address);
}

unsigned pcan_pci_read_register(can_ioptr_t address)
{
	address += ((can_ioptr2ulong(address)&(PCAN_PCI_BYTES_PER_CIRCUIT-1))
					     *(PCAN_PCI_BYTES_PER_REG-1));
	return can_readb(address);
}

int pcan_pci_irq_handler(int irq, struct canchip_t *chip)
{
	struct candevice_t *candev=chip->hostdevice;
	int ret;
	unsigned int icr_stat;
	unsigned int chip_mask = pcan_pci_idx2mask[chip->chip_idx];

	icr_stat = can_readw(candev->aux_base_addr + PITA_ICR_STAT);

	if(!(icr_stat & chip_mask)) return CANCHIP_IRQ_NONE;

	ret = sja1000p_irq_handler(irq, chip);

	can_writew(chip_mask, candev->aux_base_addr + PITA_ICR_STAT);

	return ret;
}

int pcan_pci_reset(struct candevice_t *candev)
{
	int i=0,chip_nr;
	struct canchip_t *chip;
	unsigned cdr;

	DEBUGMSG("Resetting pcan_pci hardware ...\n");

	pcan_pci_disconnect_irq(candev);

	for(chip_nr=0;chip_nr<candev->nr_all_chips;chip_nr++){
		if(!candev->chip[chip_nr]) continue;
		chip=candev->chip[chip_nr];

		pcan_pci_write_register(sjaMOD_RM, chip->chip_base_addr+SJAMOD);
		udelay(1000);

		cdr=pcan_pci_read_register(chip->chip_base_addr+SJACDR);
		pcan_pci_write_register(cdr|sjaCDR_PELICAN, chip->chip_base_addr+SJACDR);

		pcan_pci_write_register(0, chip->chip_base_addr+SJAIER);

		i=20;
		pcan_pci_write_register(0, chip->chip_base_addr+SJAMOD);
		while (pcan_pci_read_register(chip->chip_base_addr+SJAMOD)&sjaMOD_RM){
			if(!i--) return -ENODEV;
			udelay(1000);
			pcan_pci_write_register(0, chip->chip_base_addr+SJAMOD);
		}

		cdr=pcan_pci_read_register(chip->chip_base_addr+SJACDR);
		pcan_pci_write_register(cdr|sjaCDR_PELICAN, chip->chip_base_addr+SJACDR);

		pcan_pci_write_register(0, chip->chip_base_addr+SJAIER);

		pcan_pci_read_register(chip->chip_base_addr+SJAIR);
	}


	pcan_pci_connect_irq(candev);

	return 0;
}

int pcan_pci_init_hw_data(struct candevice_t *candev)
{
	struct pci_dev *pcidev = NULL;
	int i;
	int nr_chips;
	u16 subsysid;

	i = 0;
	do {
		pcidev = pci_find_device(PCAN_PCI_VENDOR_ID, PCAN_PCI_PRODUCT_ID, pcidev);
		if(pcidev == NULL) {
			printk(KERN_ERR "No unused PCAN_PCI #%d card found\n", i);
			return -ENODEV;
		}
		i++;
	} while(can_check_dev_taken(pcidev));

	if (pci_enable_device (pcidev)){
		printk(KERN_ERR "Enable PCAN_PCI failed\n");
		return -EIO;
	}
	candev->sysdevptr.pcidev=pcidev;

        if(pci_read_config_word(pcidev, 0x2E, &subsysid))
		goto error_ret;

        if(pci_write_config_word(pcidev, 0x04, 2))
		goto error_ret;

        if(pci_write_config_word(pcidev, 0x44, 0))
		goto error_ret;

        wmb();

	for(i=0;i<2;i++){
		if(!(pci_resource_flags(pcidev,0)&IORESOURCE_MEM)){
			printk(KERN_ERR "PCAN_PCI region %d is not memory\n",i);
			goto error_ret;
		}
	}

	candev->res_addr=pci_resource_start(pcidev,0);	/* Control registers */
	candev->io_addr=pci_resource_start(pcidev,1);	/* SJA1000 chips are mapped here */
	candev->dev_base_addr=(can_ioptr_t)pci_resource_start(pcidev,1);

	/*candev->flags |= CANDEV_PROGRAMMABLE_IRQ;*/

	if(subsysid >= 12)
		nr_chips = 4;
	else if(subsysid >= 10)
		nr_chips = 3;
	else if(subsysid >= 4)
		nr_chips = 2;
	else
		nr_chips = 1;

	candev->nr_82527_chips=0;
	candev->nr_sja1000_chips=nr_chips;
	candev->nr_all_chips=nr_chips;

	printk(KERN_INFO "Found PCAN_PCI device with %d chip(s)\n", nr_chips);

	return 0;

error_ret:

	printk(KERN_CRIT "Setup of PCAN_PCI failed\n");
	pci_disable_device (pcidev);
	return -EIO;
}

int pcan_pci_init_chip_data(struct candevice_t *candev, int chipnr)
{

	if(candev->sysdevptr.pcidev==NULL)
		return -ENODEV;

	sja1000p_fill_chipspecops(candev->chip[chipnr]);

	/* special version of the IRQ handler is required for PCAN_PCI board */
	candev->chip[chipnr]->chipspecops->irq_handler=pcan_pci_irq_handler;

	candev->chip[chipnr]->chip_irq=candev->sysdevptr.pcidev->irq;

	candev->chip[chipnr]->chip_base_addr=
			can_ioport2ioptr(candev->io_addr+chipnr*PCAN_PCI_BYTES_PER_CIRCUIT);
	candev->chip[chipnr]->flags = 0;
	candev->chip[chipnr]->int_cpu_reg = 0;
	candev->chip[chipnr]->int_clk_reg = 0;
	candev->chip[chipnr]->int_bus_reg = 0;
	candev->chip[chipnr]->sja_cdr_reg = sjaCDR_CBP | sjaCDR_CLK_OFF;
	candev->chip[chipnr]->sja_ocr_reg = PCAN_PCI_OCR_DEFAULT_STD;
	candev->chip[chipnr]->clock = 16000000;
	candev->chip[chipnr]->flags |= CHIP_IRQ_PCI;

	return 0;
}

int pcan_pci_init_obj_data(struct canchip_t *chip, int objnr)
{
	chip->msgobj[objnr]->obj_base_addr=chip->chip_base_addr;
	return 0;
}

int pcan_pci_program_irq(struct candevice_t *candev)
{

	return 0;
}

int pcan_pci_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = pcan_pci_request_io;
	hwspecops->release_io = pcan_pci_release_io;
	hwspecops->reset = pcan_pci_reset;
	hwspecops->init_hw_data = pcan_pci_init_hw_data;
	hwspecops->init_chip_data = pcan_pci_init_chip_data;
	hwspecops->init_obj_data = pcan_pci_init_obj_data;
	hwspecops->write_register = pcan_pci_write_register;
	hwspecops->read_register = pcan_pci_read_register;
	hwspecops->program_irq = pcan_pci_program_irq;
	return 0;
}


#endif /*CAN_ENABLE_PCI_SUPPORT*/
