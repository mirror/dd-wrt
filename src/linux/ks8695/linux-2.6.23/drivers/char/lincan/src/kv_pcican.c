/* kv_pcican.c - support for KVASER PCIcan-S/D/Q cards
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

#define KV_PCICAN_PCICAN_VENDOR  0x10e8
#define KV_PCICAN_PCICAN_ID 0x8406

/*AMCC 5920*/

#define S5920_OMB    0x0C
#define S5920_IMB    0x1C
#define S5920_MBEF   0x34
#define S5920_INTCSR 0x38
#define S5920_RCR    0x3C
#define S5920_PTCR   0x60

#define INTCSR_ADDON_INTENABLE_M        0x2000
#define INTCSR_INTERRUPT_ASSERTED_M     0x800000

#define KV_PCICAN_BYTES_PER_CIRCUIT 0x20

// Standard value: Pushpull  (OCTP1|OCTN1|OCTP0|OCTN0|OCM1)
#define KV_PCICAN_OCR_DEFAULT_STD 0xDA
// For Galathea piggyback.
#define KV_PCICAN_OCR_DEFAULT_GAL 0xDB

/*

You need to know the following: 
" RX1 is connected to ground. 
" TX1 is not connected. 
" CLKO is not connected. 
" Setting the OCR register to 0xDA is a good idea. 
  This means  normal output mode , push-pull and the correct polarity. 
" In the CDR register, you should set CBP to 1. 
  You will probably also want to set the clock divider value to 0 (meaning divide-by-2),
  the Pelican bit, and the clock-off bit (you have no need for CLKOUT anyway.)

*/



void kv_pcican_disconnect_irq(struct candevice_t *candev)
{
	unsigned long tmp;
	/* Disable interrupts from card */
	tmp = can_inl(candev->dev_base_addr + S5920_INTCSR);
	tmp &= ~INTCSR_ADDON_INTENABLE_M;
	can_outl(tmp, candev->dev_base_addr + S5920_INTCSR);
}

void kv_pcican_connect_irq(struct candevice_t *candev)
{
	unsigned long tmp;
	/* Enable interrupts from card */
	tmp = can_inl(candev->dev_base_addr + S5920_INTCSR);
	tmp |= INTCSR_ADDON_INTENABLE_M;
	can_outl(tmp, candev->dev_base_addr + S5920_INTCSR);
}


int kv_pcican_request_io(struct candevice_t *candev)
{
    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))
	if(pci_request_region(candev->sysdevptr.pcidev, 0, "kv_pcican_s5920") != 0){
		CANMSG("Request of kv_pcican_s5920 range failed\n");
		return -ENODEV;
	}else if(pci_request_region(candev->sysdevptr.pcidev, 1, "kv_pcican_io") != 0){
		CANMSG("Request of kv_pcican_io range failed\n");
		goto error_io;
	}else if(pci_request_region(candev->sysdevptr.pcidev, 2, "kv_pcican_xilinx") != 0){
		CANMSG("Request of kv_pcican_xilinx range failed\n");
		goto error_xilinx;
	}
    #else /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
	if(pci_request_regions(candev->sysdevptr.pcidev, "kv_pcican") != 0){
		CANMSG("Request of kv_pcican_s5920 regions failed\n");
		return -ENODEV;
	}
    #endif /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/

	kv_pcican_disconnect_irq(candev);

	return 0;

    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))
    error_xilinx:
	pci_release_region(candev->sysdevptr.pcidev, 1);
    error_io:
	pci_release_region(candev->sysdevptr.pcidev, 0);
    #endif /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
	
	return -ENODEV;
}

int kv_pcican_release_io(struct candevice_t *candev)
{
	kv_pcican_disconnect_irq(candev);

    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))
	pci_release_region(candev->sysdevptr.pcidev, 2);
	pci_release_region(candev->sysdevptr.pcidev, 1);
	pci_release_region(candev->sysdevptr.pcidev, 0);
    #else /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
	pci_release_regions(candev->sysdevptr.pcidev);
    #endif /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/

	return 0;
}


void kv_pcican_write_register(unsigned data, can_ioptr_t address)
{
	can_outb(data,address); 
}

unsigned kv_pcican_read_register(can_ioptr_t address)
{
	return can_inb(address);
}

int kv_pcican_reset(struct candevice_t *candev)
{
	int i=0,chip_nr;
	struct canchip_t *chip;
	unsigned cdr;

	DEBUGMSG("Resetting kv_pcican hardware ...\n");

	/* Assert PTADR# - we're in passive mode so the other bits are not important */
	can_outl(0x80808080L, candev->dev_base_addr + S5920_PTCR);

	kv_pcican_disconnect_irq(candev);

	for(chip_nr=0;chip_nr<candev->nr_all_chips;chip_nr++){
		if(!candev->chip[chip_nr]) continue;
		chip=candev->chip[chip_nr];

		kv_pcican_write_register(sjaMOD_RM, chip->chip_base_addr+SJAMOD);
		udelay(1000);

		cdr=kv_pcican_read_register(chip->chip_base_addr+SJACDR);
		kv_pcican_write_register(cdr|sjaCDR_PELICAN, chip->chip_base_addr+SJACDR);

		kv_pcican_write_register(0, chip->chip_base_addr+SJAIER);

		i=20;
		kv_pcican_write_register(0, chip->chip_base_addr+SJAMOD);
		while (kv_pcican_read_register(chip->chip_base_addr+SJAMOD)&sjaMOD_RM){
			if(!i--) return -ENODEV;
			udelay(1000);
			kv_pcican_write_register(0, chip->chip_base_addr+SJAMOD);
		}

		cdr=kv_pcican_read_register(chip->chip_base_addr+SJACDR);
		kv_pcican_write_register(cdr|sjaCDR_PELICAN, chip->chip_base_addr+SJACDR);

		kv_pcican_write_register(0, chip->chip_base_addr+SJAIER);
		
		kv_pcican_read_register(chip->chip_base_addr+SJAIR);
	}
	

	kv_pcican_connect_irq(candev);

	return 0;
} 	

int kv_pcican_init_hw_data(struct candevice_t *candev)
{
	struct pci_dev *pcidev = NULL;
	int i;

	do {
		pcidev = pci_find_device(KV_PCICAN_PCICAN_VENDOR, KV_PCICAN_PCICAN_ID, pcidev);
		if(pcidev == NULL) return -ENODEV;
	} while(can_check_dev_taken(pcidev));
	
	if (pci_enable_device (pcidev)){
		printk(KERN_CRIT "Setup of PCICAN failed\n");
		return -EIO;
	}
	candev->sysdevptr.pcidev=pcidev;
	
	for(i=0;i<3;i++){
		if(!(pci_resource_flags(pcidev,i)&IORESOURCE_IO)){
			printk(KERN_CRIT "PCICAN region %d is not IO\n",i);
			return -EIO;
		}
	}
	candev->dev_base_addr=pci_resource_start(pcidev,0); /*S5920*/
	candev->io_addr=pci_resource_start(pcidev,1); /*IO window for SJA1000 chips*/
	candev->res_addr=pci_resource_start(pcidev,2); /*XILINX board wide address*/
	
	/*candev->flags |= CANDEV_PROGRAMMABLE_IRQ;*/

	if (!strcmp(candev->hwname,"pcican-s")) {
		candev->nr_82527_chips=0;
		candev->nr_sja1000_chips=1;
		candev->nr_all_chips=1;
	}
	if (!strcmp(candev->hwname,"pcican-d")) {
		candev->nr_82527_chips=0;
		candev->nr_sja1000_chips=2;
		candev->nr_all_chips=2;
	}
	if (!strcmp(candev->hwname,"pcican-q")) {
		candev->nr_82527_chips=0;
		candev->nr_sja1000_chips=4;
		candev->nr_all_chips=4;
	}

	return 0;
}

int kv_pcican_init_chip_data(struct candevice_t *candev, int chipnr)
{

	if(candev->sysdevptr.pcidev==NULL)
		return -ENODEV;
	
	candev->chip[chipnr]->chip_irq=candev->sysdevptr.pcidev->irq;

	sja1000p_fill_chipspecops(candev->chip[chipnr]);
	candev->chip[chipnr]->chip_base_addr=
			candev->io_addr+chipnr*KV_PCICAN_BYTES_PER_CIRCUIT;
	candev->chip[chipnr]->flags = 0;
	candev->chip[chipnr]->int_cpu_reg = 0;
	candev->chip[chipnr]->int_clk_reg = 0;
	candev->chip[chipnr]->int_bus_reg = 0;
	candev->chip[chipnr]->sja_cdr_reg = sjaCDR_CBP | sjaCDR_CLK_OFF;
	candev->chip[chipnr]->sja_ocr_reg = KV_PCICAN_OCR_DEFAULT_STD;
	candev->chip[chipnr]->clock = 16000000;
	candev->chip[chipnr]->flags |= CHIP_IRQ_PCI;

	return 0;
}	

int kv_pcican_init_obj_data(struct canchip_t *chip, int objnr)
{
	chip->msgobj[objnr]->obj_base_addr=chip->chip_base_addr;
	return 0;
}

int kv_pcican_program_irq(struct candevice_t *candev)
{

	return 0;
}

int kv_pcican_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = kv_pcican_request_io;
	hwspecops->release_io = kv_pcican_release_io;
	hwspecops->reset = kv_pcican_reset;
	hwspecops->init_hw_data = kv_pcican_init_hw_data;
	hwspecops->init_chip_data = kv_pcican_init_chip_data;
	hwspecops->init_obj_data = kv_pcican_init_obj_data;
	hwspecops->write_register = kv_pcican_write_register;
	hwspecops->read_register = kv_pcican_read_register;
	hwspecops->program_irq = kv_pcican_program_irq;
	return 0;
}


#endif /*CAN_ENABLE_PCI_SUPPORT*/
