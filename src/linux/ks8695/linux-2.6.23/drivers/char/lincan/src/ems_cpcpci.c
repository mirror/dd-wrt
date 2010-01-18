/* ems_cpcpci.c - support for EMS-WUENSCHE CPC-PCI card
 * Linux CAN-bus device driver.
 * The card support added by Paolo Grisleri <grisleri@ce.unipr.it>
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */ 

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/sja1000p.h"

#ifdef CAN_ENABLE_PCI_SUPPORT


/* the only one supported: EMS CPC-PCI */
// PGX: check identifiers name
# define EMS_CPCPCI_PCICAN_VENDOR 0x110a
# define EMS_CPCPCI_PCICAN_ID 0x2104

/*The Infineon PSB4610 PITA-2 is used as PCI to local bus bridge*/
/*BAR0 - MEM - bridge control registers*/

/*BAR1 - MEM - parallel interface*/
/* 0 more EMS control registers
 * 0x400 the first SJA1000
 * 0x600 the second SJA1000
 * each register occupies 4 bytes
 */

/*PSB4610 PITA-2 bridge control registers*/   
#define PITA2_ICR  0x00  /* Interrupt Control Register */
#define   PITA2_ICR_INT0    0x00000002	/* [RC] INT0 Active/Clear */
#define   PITA2_ICR_GP0_INT 0x00000004	/* [RC] GP0 Interrupt */
					/* GP0_Int_En=1, GP0_Out_En=0 and low detected */
#define   PITA2_ICR_GP1_INT 0x00000008	/* [RC] GP1 Interrupt */
#define   PITA2_ICR_GP2_INT 0x00000010	/* [RC] GP2 Interrupt */
#define   PITA2_ICR_GP3_INT 0x00000020	/* [RC] GP2 Interrupt */
#define   PITA2_ICR_INT0_En 0x00020000	/* [RW] Enable INT0 */

#define PITA2_MISC 0x1C  /* Miscellaneous Register */
#define   PITA2_MISC_CONFIG 0x04000000
			 /* Multiplexed Parallel_interface_mode */

#define EMS_CPCPCI_BYTES_PER_CIRCUIT 0x200
/* Each CPC register occupies 4 bytes */
#define EMS_CPCPCI_BYTES_PER_REG     0x4

// Standard value: Pushpull  (OCTP1|OCTN1|OCTP0|OCTN0|OCM1)
#define EMS_CPCPCI_OCR_DEFAULT_STD 0xDA
// For Galathea piggyback.
#define EMS_CPCPCI_OCR_DEFAULT_GAL 0xDB

/*

The board configuration is probably following: 
" RX1 is connected to ground. 
" TX1 is not connected. 
" CLKO is not connected. 
" Setting the OCR register to 0xDA is a good idea. 
  This means  normal output mode , push-pull and the correct polarity. 
" In the CDR register, you should set CBP to 1. 
  You will probably also want to set the clock divider value to 7
  (meaning direct oscillator output) because the second SJA1000 chip 
  is driven by the first one CLKOUT output.

*/



void ems_cpcpci_disconnect_irq(struct candevice_t *candev)
{
	/* Disable interrupts from card */
	can_writel(0, candev->aux_base_addr + PITA2_ICR);
}

void ems_cpcpci_connect_irq(struct candevice_t *candev)
{
	/* Enable interrupts from card */
	can_writel(PITA2_ICR_INT0_En, candev->aux_base_addr + PITA2_ICR);
}


int ems_cpcpci_request_io(struct candevice_t *candev)
{
	unsigned long pita2_addr;
	unsigned long io_addr;
	int i;

    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))
	if(pci_request_region(candev->sysdevptr.pcidev, 0, "ems_cpcpci_pita2") != 0){
		CANMSG("Request of ems_cpcpci_pita2 range failed\n");
		return -ENODEV;
	}else if(pci_request_region(candev->sysdevptr.pcidev, 1, "ems_cpcpci_io") != 0){
		CANMSG("Request of ems_cpcpci_io range failed\n");
		goto error_io;
	}
    #else /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
	if(pci_request_regions(candev->sysdevptr.pcidev, "EMS_CPCPCI") != 0){
		CANMSG("Request of ems_cpcpci_s5920 regions failed\n");
		return -ENODEV;
	}
    #endif /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/

	pita2_addr=pci_resource_start(candev->sysdevptr.pcidev,0);
	if (!(candev->aux_base_addr = ioremap(pita2_addr, 
	      pci_resource_len(candev->sysdevptr.pcidev,0)))) {
		CANMSG("Unable to access I/O memory at: 0x%lx\n", pita2_addr);
		goto error_ioremap_pita2;
	}

	io_addr=pci_resource_start(candev->sysdevptr.pcidev,1);;
	if (!(candev->dev_base_addr = ioremap(io_addr,
	      pci_resource_len(candev->sysdevptr.pcidev,1)))) {
		CANMSG("Unable to access I/O memory at: 0x%lx\n", io_addr);
		goto error_ioremap_io;
	}

	candev->io_addr=io_addr;
	candev->res_addr=pita2_addr;
	
	/* 
	 * this is redundant with chip initialization, but remap address 
	 * can change when resources are temporarily released
	 */
	for(i=0;i<candev->nr_all_chips;i++) {
		struct canchip_t *chip=candev->chip[i];
		if(!chip) continue;
		chip->chip_base_addr = candev->dev_base_addr+
			0x400 + i*EMS_CPCPCI_BYTES_PER_CIRCUIT;
		if(!chip->msgobj[0]) continue;
		chip->msgobj[0]->obj_base_addr=chip->chip_base_addr;
	}

	/* Configure PITA-2 parallel interface */
	can_writel(PITA2_MISC_CONFIG, candev->aux_base_addr + PITA2_MISC);

	ems_cpcpci_disconnect_irq(candev);

	return 0;

    error_ioremap_io:
	iounmap(candev->aux_base_addr);
    error_ioremap_pita2:
    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))
	pci_release_region(candev->sysdevptr.pcidev, 1);
    error_io:
	pci_release_region(candev->sysdevptr.pcidev, 0);
    #else /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
	pci_release_regions(candev->sysdevptr.pcidev);
    #endif /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
	
	return -ENODEV;
}

int ems_cpcpci_release_io(struct candevice_t *candev)
{
	ems_cpcpci_disconnect_irq(candev);

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


void ems_cpcpci_write_register(unsigned data, can_ioptr_t address)
{
	address += ((can_ioptr2ulong(address)&(EMS_CPCPCI_BYTES_PER_CIRCUIT-1))
					     *(EMS_CPCPCI_BYTES_PER_REG-1));
	can_writeb(data,address); 
}

unsigned ems_cpcpci_read_register(can_ioptr_t address)
{
	address += ((can_ioptr2ulong(address)&(EMS_CPCPCI_BYTES_PER_CIRCUIT-1))
					     *(EMS_CPCPCI_BYTES_PER_REG-1));
	return can_readb(address);
}

int ems_cpcpci_irq_handler(int irq, struct canchip_t *chip)
{
	//struct canchip_t *chip=(struct canchip_t *)dev_id;
	struct candevice_t *candev=chip->hostdevice;
	int i;
	unsigned long icr;
	int test_irq_again;

	icr=can_readl(candev->aux_base_addr + PITA2_ICR);
	if(!(icr & PITA2_ICR_INT0)) return CANCHIP_IRQ_NONE;
	
	/* correct way to handle interrupts from all chips connected to the one PITA-2 */
	do {
		can_writel(PITA2_ICR_INT0_En | PITA2_ICR_INT0, candev->aux_base_addr + PITA2_ICR);
		test_irq_again=0;
		for(i=0;i<candev->nr_all_chips;i++){
			chip=candev->chip[i];
			if(!chip || !(chip->flags&CHIP_CONFIGURED))
				continue;
			if(sja1000p_irq_handler(irq, chip))
				test_irq_again=1;
		}
		icr=can_readl(candev->aux_base_addr + PITA2_ICR);
	} while((icr & PITA2_ICR_INT0)||test_irq_again);
	return CANCHIP_IRQ_HANDLED;
}

int ems_cpcpci_reset(struct candevice_t *candev)
{
	int i=0,chip_nr;
	struct canchip_t *chip;
	unsigned cdr;

	DEBUGMSG("Resetting EMS_CPCPCI hardware ...\n");

	/* Assert PTADR# - we're in passive mode so the other bits are not important */

	ems_cpcpci_disconnect_irq(candev);

	for(chip_nr=0;chip_nr<candev->nr_all_chips;chip_nr++){
		if(!candev->chip[chip_nr]) continue;
		chip=candev->chip[chip_nr];

		ems_cpcpci_write_register(sjaMOD_RM, chip->chip_base_addr+SJAMOD);
		udelay(1000);

		cdr=ems_cpcpci_read_register(chip->chip_base_addr+SJACDR);
		ems_cpcpci_write_register(cdr|sjaCDR_PELICAN, chip->chip_base_addr+SJACDR);

		ems_cpcpci_write_register(0, chip->chip_base_addr+SJAIER);

		i=20;
		ems_cpcpci_write_register(0, chip->chip_base_addr+SJAMOD);
		while (ems_cpcpci_read_register(chip->chip_base_addr+SJAMOD)&sjaMOD_RM){
			if(!i--) return -ENODEV;
			udelay(1000);
			ems_cpcpci_write_register(0, chip->chip_base_addr+SJAMOD);
		}

		cdr=ems_cpcpci_read_register(chip->chip_base_addr+SJACDR);
		ems_cpcpci_write_register(cdr|sjaCDR_PELICAN, chip->chip_base_addr+SJACDR);

		ems_cpcpci_write_register(0, chip->chip_base_addr+SJAIER);
		
		ems_cpcpci_read_register(chip->chip_base_addr+SJAIR);
	}
	

	ems_cpcpci_connect_irq(candev);

	return 0;
} 	

int ems_cpcpci_init_hw_data(struct candevice_t *candev)
{
	struct pci_dev *pcidev = NULL;
	int i;
	unsigned long l;

	pcidev = pci_find_device(EMS_CPCPCI_PCICAN_VENDOR, EMS_CPCPCI_PCICAN_ID, pcidev);
	if(pcidev == NULL) return -ENODEV;
	
	if (pci_enable_device (pcidev)){
		printk(KERN_CRIT "Setup of EMS_CPCPCI failed\n");
		return -EIO;
	}
	candev->sysdevptr.pcidev=pcidev;
	
	for(i=0;i<2;i++){
		if(!(pci_resource_flags(pcidev,0)&IORESOURCE_MEM)){
			printk(KERN_CRIT "EMS_CPCPCI region %d is not memory\n",i);
			return -EIO;
		}
	}

	/*request IO access temporarily to check card presence*/
	if(ems_cpcpci_request_io(candev)<0)
		return -ENODEV;

	/*** candev->aux_base_addr=pci_resource_start(pcidev,0); ***/
	/* some control registers */
	/*** candev->dev_base_addr=pci_resource_start(pcidev,1); ***/
	/* 0 more EMS control registers
         * 0x400 the first SJA1000
         * 0x600 the second SJA1000
         * each register occupies 4 bytes
         */
	
	/*candev->flags |= CANDEV_PROGRAMMABLE_IRQ;*/
	
	for(l=0,i=0;i<4;i++){
		l<<=8;
		l|=can_readb(candev->dev_base_addr + i*4);
	}
	i=can_readb(candev->dev_base_addr + i*5);
	
	CANMSG("EMS CPC-PCI check value %04lx, ID %d\n", l, i);
	
	if(l!=0x55aa01cb) {
		CANMSG("EMS CPC-PCI unexpected check values\n");
	}

	/*if (!strcmp(candev->hwname,"ems_cpcpci"))*/
	candev->nr_82527_chips=0;
	candev->nr_sja1000_chips=2;
	candev->nr_all_chips=2;

	ems_cpcpci_release_io(candev);
        
	return 0;
}

int ems_cpcpci_init_chip_data(struct candevice_t *candev, int chipnr)
{
	if(candev->sysdevptr.pcidev==NULL)
		return -ENODEV;

	/* initialize common routines for the SJA1000 chip */
	sja1000p_fill_chipspecops(candev->chip[chipnr]);
	
	/* special version of the IRQ handler is required for CPC-PCI board */
	candev->chip[chipnr]->chipspecops->irq_handler=ems_cpcpci_irq_handler;

	candev->chip[chipnr]->chip_irq=candev->sysdevptr.pcidev->irq;

	candev->chip[chipnr]->chip_base_addr = candev->dev_base_addr+
			0x400 + chipnr*EMS_CPCPCI_BYTES_PER_CIRCUIT;
	candev->chip[chipnr]->flags = 0;
	candev->chip[chipnr]->int_cpu_reg = 0;
	candev->chip[chipnr]->int_clk_reg = 0;
	candev->chip[chipnr]->int_bus_reg = 0;
	/* CLKOUT has to be equal to oscillator frequency to drive second chip */
	candev->chip[chipnr]->sja_cdr_reg = sjaCDR_CBP | 7;
	candev->chip[chipnr]->sja_ocr_reg = EMS_CPCPCI_OCR_DEFAULT_STD;
	candev->chip[chipnr]->clock = 16000000;
	candev->chip[chipnr]->flags |= CHIP_IRQ_PCI;

	return 0;
}	

int ems_cpcpci_init_obj_data(struct canchip_t *chip, int objnr)
{
	chip->msgobj[objnr]->obj_base_addr=chip->chip_base_addr;
	return 0;
}

int ems_cpcpci_program_irq(struct candevice_t *candev)
{

	return 0;
}

int ems_cpcpci_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = ems_cpcpci_request_io;
	hwspecops->release_io = ems_cpcpci_release_io;
	hwspecops->reset = ems_cpcpci_reset;
	hwspecops->init_hw_data = ems_cpcpci_init_hw_data;
	hwspecops->init_chip_data = ems_cpcpci_init_chip_data;
	hwspecops->init_obj_data = ems_cpcpci_init_obj_data;
	hwspecops->write_register = ems_cpcpci_write_register;
	hwspecops->read_register = ems_cpcpci_read_register;
	hwspecops->program_irq = ems_cpcpci_program_irq;
	return 0;
}


#endif /*CAN_ENABLE_PCI_SUPPORT*/
