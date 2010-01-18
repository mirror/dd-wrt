/* adlink7841.c - support for ADLINK 7841 cards
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

#define ADLINK7841_PCI_VENDOR_ID	0x144A	/* ADLINK vednor id */
#define ADLINK7841_PCI_PRODUCT_ID	0x7841	/* PCI 7841 device ID */

/* PCI to local bus bridge PLX9050 */

#define PLX9050_INTCSR	0x4c	/* interrupt control register */

#define ADLINK7841_BYTES_PER_CIRCUIT 0x80

// Standard value: Pushpull  (OCTP1|OCTN1|OCPOL1|OCTP0|OCTN0|OCM1)
#define ADLINK7841_OCR_DEFAULT_STD 0xFA

/*

You need to know the following: 
" RX1 is connected to ground. 
" TX1 is not connected. 
" CLKO is not connected. 
" Setting the OCR register to 0xFA is a good idea. 
  This means  normal output mode , push-pull and the correct polarity. 
" In the CDR register, you should set CBP to 1. 
  You will probably also want to set the clock divider value to 0 (meaning divide-by-2),
  the Pelican bit, and the clock-off bit (you have no need for CLKOUT anyway.)

*/



void adlink7841_disconnect_irq(struct candevice_t *candev)
{
}

void adlink7841_connect_irq(struct candevice_t *candev)
{
}


int adlink7841_request_io(struct candevice_t *candev)
{
    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))
	if(pci_request_region(candev->sysdevptr.pcidev, 1, "adlink7841_plx9050") != 0){
		CANMSG("Request of adlink7841_plx9050 range failed\n");
		return -ENODEV;
	}else if(pci_request_region(candev->sysdevptr.pcidev, 2, "adlink7841_io") != 0){
		CANMSG("Request of adlink7841_io range failed\n");
		goto error_io;
	}
    #else /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
	if(pci_request_regions(candev->sysdevptr.pcidev, "adlink7841") != 0){
		CANMSG("Request of adlink7841_plx9050 regions failed\n");
		return -ENODEV;
	}
    #endif /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/

	adlink7841_disconnect_irq(candev);

	return 0;

    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))
    error_io:
	pci_release_region(candev->sysdevptr.pcidev, 1);
    #endif /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
	
	return -ENODEV;
}

int adlink7841_release_io(struct candevice_t *candev)
{
	adlink7841_disconnect_irq(candev);

    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))
	pci_release_region(candev->sysdevptr.pcidev, 2);
	pci_release_region(candev->sysdevptr.pcidev, 1);
    #else /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
	pci_release_regions(candev->sysdevptr.pcidev);
    #endif /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/

	return 0;
}


void adlink7841_write_register(unsigned data, can_ioptr_t address)
{
	can_outb(data,address); 
}

unsigned adlink7841_read_register(can_ioptr_t address)
{
	return can_inb(address);
}

int adlink7841_reset(struct candevice_t *candev)
{
	int i=0,chip_nr;
	struct canchip_t *chip;
	unsigned cdr;

	DEBUGMSG("Resetting adlink7841 hardware ...\n");

	adlink7841_disconnect_irq(candev);

	for(chip_nr=0;chip_nr<candev->nr_all_chips;chip_nr++){
		if(!candev->chip[chip_nr]) continue;
		chip=candev->chip[chip_nr];

		adlink7841_write_register(sjaMOD_RM, chip->chip_base_addr+SJAMOD);
		udelay(1000);

		cdr=adlink7841_read_register(chip->chip_base_addr+SJACDR);
		adlink7841_write_register(cdr|sjaCDR_PELICAN, chip->chip_base_addr+SJACDR);

		adlink7841_write_register(0, chip->chip_base_addr+SJAIER);

		i=20;
		adlink7841_write_register(0, chip->chip_base_addr+SJAMOD);
		while (adlink7841_read_register(chip->chip_base_addr+SJAMOD)&sjaMOD_RM){
			if(!i--) return -ENODEV;
			udelay(1000);
			adlink7841_write_register(0, chip->chip_base_addr+SJAMOD);
		}

		cdr=adlink7841_read_register(chip->chip_base_addr+SJACDR);
		adlink7841_write_register(cdr|sjaCDR_PELICAN, chip->chip_base_addr+SJACDR);

		adlink7841_write_register(0, chip->chip_base_addr+SJAIER);
		
		adlink7841_read_register(chip->chip_base_addr+SJAIR);
	}
	

	adlink7841_connect_irq(candev);

	return 0;
} 	

int adlink7841_init_hw_data(struct candevice_t *candev)
{
	struct pci_dev *pcidev = NULL;
	int i;

	do {
		pcidev = pci_find_device(ADLINK7841_PCI_VENDOR_ID, ADLINK7841_PCI_PRODUCT_ID, pcidev);
		if(pcidev == NULL) return -ENODEV;
	} while(can_check_dev_taken(pcidev));
	
	if (pci_enable_device (pcidev)){
		printk(KERN_CRIT "Setup of ADLINK7841 failed\n");
		return -EIO;
	}
	candev->sysdevptr.pcidev=pcidev;
	
	for(i=1;i<3;i++){
		if(!(pci_resource_flags(pcidev,i)&IORESOURCE_IO)){
			printk(KERN_CRIT "ADLINK7841 region %d is not IO\n",i);
			return -EIO;
		}
	}
	candev->dev_base_addr=pci_resource_start(pcidev,1); /* PLX 9050 BASE*/
	candev->io_addr=pci_resource_start(pcidev,2); /*IO window for SJA1000 chips*/
	candev->res_addr=pci_resource_start(pcidev,1); /*reserved*/
	
	/*candev->flags |= CANDEV_PROGRAMMABLE_IRQ;*/

	candev->nr_82527_chips=0;
	candev->nr_sja1000_chips=2;
	candev->nr_all_chips=2;

	return 0;
}

int adlink7841_init_chip_data(struct candevice_t *candev, int chipnr)
{

	if(candev->sysdevptr.pcidev==NULL)
		return -ENODEV;
	
	candev->chip[chipnr]->chip_irq=candev->sysdevptr.pcidev->irq;

	sja1000p_fill_chipspecops(candev->chip[chipnr]);
	candev->chip[chipnr]->chip_base_addr=
			can_ioport2ioptr(candev->io_addr+chipnr*ADLINK7841_BYTES_PER_CIRCUIT);
	candev->chip[chipnr]->flags = 0;
	candev->chip[chipnr]->int_cpu_reg = 0;
	candev->chip[chipnr]->int_clk_reg = 0;
	candev->chip[chipnr]->int_bus_reg = 0;
	candev->chip[chipnr]->sja_cdr_reg = sjaCDR_CBP | sjaCDR_CLK_OFF;
	candev->chip[chipnr]->sja_ocr_reg = ADLINK7841_OCR_DEFAULT_STD;
	candev->chip[chipnr]->clock = 16000000;
	candev->chip[chipnr]->flags |= CHIP_IRQ_PCI;

	return 0;
}	

int adlink7841_init_obj_data(struct canchip_t *chip, int objnr)
{
	chip->msgobj[objnr]->obj_base_addr=chip->chip_base_addr;
	return 0;
}

int adlink7841_program_irq(struct candevice_t *candev)
{

	return 0;
}

int adlink7841_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = adlink7841_request_io;
	hwspecops->release_io = adlink7841_release_io;
	hwspecops->reset = adlink7841_reset;
	hwspecops->init_hw_data = adlink7841_init_hw_data;
	hwspecops->init_chip_data = adlink7841_init_chip_data;
	hwspecops->init_obj_data = adlink7841_init_obj_data;
	hwspecops->write_register = adlink7841_write_register;
	hwspecops->read_register = adlink7841_read_register;
	hwspecops->program_irq = adlink7841_program_irq;
	return 0;
}


#endif /*CAN_ENABLE_PCI_SUPPORT*/
