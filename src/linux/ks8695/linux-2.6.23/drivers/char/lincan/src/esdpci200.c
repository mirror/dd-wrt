/* esdpci200.c - support for ESD Electronics' CAN/PCI-200 cards
 * Linux CAN-bus device driver.
 * The card support was added by Manuel Bessler <m.bessler@gmx.net>
 * Based on adlink7841.c and nsi_canpci.c
 * This software is released under the GPL-License.
 * Version lincan-0.3.3
 */ 

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/sja1000p.h"

#ifdef CAN_ENABLE_PCI_SUPPORT

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10))
	#define ioread32        can_readl
	#define iowrite32       can_writel
	#define ioread8         can_readb
	#define iowrite8        can_writeb
	#define wmb()
	#define rmb()
#else
#endif

#define ESDPCI200_PCI_VENDOR_ID	0x10B5
#define ESDPCI200_PCI_PRODUCT_ID	0x9050

/* PCI to local bus bridge PLX9052 */

#define PLX9052_INTCSR	0x4c	/* interrupt control register */
#define PLX9052_CNTRL	0x50	/* control register, for software reset */

/* The ESD PCI/200 uses (per default) just LINTi1 (Local Interrupt 1)
 * on the PLX. This means that both CAN channels' (SJA1000's) /INT pins 
 * are OR'ed to the LINTi1 pin (actually ANDed in the 74HC08 since both
 * the SJA1000's /INT pins and the LINTi1 pin are active low).
 *
 * The board does have an option to route the 2nd channel to LINTi2,
 * apparently just one or two resistors need to be added.
 *
 * LINTi2 is floating per default, so don't set its interrupt enable flag 
 * 'PLX9052_INTCSR_LI2EN', it'll just interrupt all the time.
 */
#define PLX9052_INTCSR_LI1EN 0x00000001 /* Local Interrupt 1 enable */
#define PLX9052_INTCSR_LI1S  0x00000004 /* Local Interrupt 1 status */
#define PLX9052_INTCSR_LI2EN 0x00000008 /* Local Interrupt 2 enable */
#define PLX9052_INTCSR_LI2S  0x00000020 /* Local Interrupt 2 status */
#define PLX9052_INTCSR_PIEN  0x00000040 /* PCI Interrupt enable */

#define PLX9052_CNTRL_SWRESET 0x40000000 /* PCI Adapter Software Reset to Local Bus */

#define IO_RANGE 0x100

// Standard value: Pushpull  (OCTP1|OCTN1|OCPOL1|OCTP0|OCTN0|OCM1)
#define ESDPCI200_OCR_DEFAULT_STD 0xFA
/* Setting the OCR register to 0xFA is a good idea. 
   This means  normal output mode , push-pull and the correct polarity. */


void esdpci200_pci_soft_reset(struct candevice_t *candev)
{
	unsigned long reg_reset;
	reg_reset = inl( candev->res_addr+PLX9052_CNTRL);
	reg_reset &= ~(PLX9052_CNTRL_SWRESET);
	rmb();
	/* PCI Adapter Software Reset plus reset local bus */
	outl( (reg_reset | PLX9052_CNTRL_SWRESET ), candev->res_addr+PLX9052_CNTRL);
	wmb();
	udelay(2500);
	outl(reg_reset, candev->res_addr+PLX9052_CNTRL);
	wmb();
	udelay(2500);
}

void esdpci200_disconnect_irq(struct candevice_t *candev)
{
    /* writing 0x0 into the PLX's INTCSR register disables interrupts */
	/* 0x0 is also the value in the register after a power-on reset */
	outl(0x0, candev->res_addr + PLX9052_INTCSR);
	DEBUGMSG("disabled interrupts on the PLX\n");
}

void esdpci200_connect_irq(struct candevice_t *candev)
{
        /* enable interrupts for the SJA1000's, enable PCI interrupts */
	outl(	PLX9052_INTCSR_LI1EN | PLX9052_INTCSR_PIEN,
		candev->res_addr+PLX9052_INTCSR);
	DEBUGMSG("enabled interrupts on the PLX\n"); 
}

int esdpci200_irq_handler(int irq, struct canchip_t *chip)
{
	int retcode;
	unsigned long it_reg;
	struct candevice_t *candev;
	candev = chip->hostdevice;
	retcode = CANCHIP_IRQ_NONE;
	//DEBUGMSG("Starting to handle an IRQ\n");
	it_reg = inl(candev->res_addr+PLX9052_INTCSR);
	rmb();
	if((it_reg & (PLX9052_INTCSR_LI1S | PLX9052_INTCSR_LI1EN) ) 
		== (PLX9052_INTCSR_LI1S | PLX9052_INTCSR_LI1EN) ) 
	{	/*interrupt enabled and active */
		int chipnum;
		for(chipnum=0; chipnum < candev->nr_sja1000_chips; chipnum++)
		{
			if(sja1000p_irq_handler(irq, candev->chip[chipnum]) == CANCHIP_IRQ_NONE)
			{ /* since both chips use the same IRQ and the same LINTi on the PLX,
			     we need manually do 'interrupt sharing' on the boardlevel
			     by checking all chips one-by-one */
				continue;
			}
			else
			{
				retcode=CANCHIP_IRQ_HANDLED;
			}
		}
		if( retcode != CANCHIP_IRQ_HANDLED )
		{/* None of the chips felt they were responsible for this IRQ... 
		    so it appears we have problems with the IRQ */
			it_reg &= ~(PLX9052_INTCSR_LI1EN);
			//Either we have a problem with IRQ malfunctions, or our IRQ is shared with some other device.
			//
			//not actually disabled, unless outl() below is uncommented
			//outl(it_reg,(void*)(candev->res_addr+PLX9052_INTCSR));
			//CANMSG("CAN Interrupt disabled due to malfunction\n");
		}
	}
	return retcode;
}

int esdpci200_request_io(struct candevice_t *candev)
{
	struct pci_dev *pcidev = candev->sysdevptr.pcidev;
	can_ioptr_t remap_addr;
	unsigned long bar2_addr;

	#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))
	if(pci_request_region(pcidev, 0, "esdpci200_plx9050") != 0){
		CANMSG("Request of esdpci200_plx9050 range failed\n");
		return -ENODEV;
	}else if(pci_request_region(pcidev, 1, "esdpci200_io") != 0){
		CANMSG("Request of esdpci200_io range failed\n");
		pci_release_region(pcidev, 0);
		return -ENODEV;
	}else if(pci_request_region(pcidev, 2, "esdpci200_sja") != 0){
		CANMSG("Request of esdpci200_sja range failed\n");
		pci_release_region(pcidev, 1);
		pci_release_region(pcidev, 0);
		return -ENODEV;
	}
	#else /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
	if(pci_request_regions(pcidev, "esdpci200") != 0){
		CANMSG("Request of esdpci200_plx9050 regions failed\n");
		return -ENODEV;
	}
	#endif /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/


	 /* ioports, PLX local configuration registers */
	candev->res_addr=pci_resource_start(pcidev,1);
	/*MEM window for SJA1000 chips*/
	bar2_addr = pci_resource_start(pcidev,2);
	candev->io_addr = bar2_addr;
	if( ! (remap_addr=ioremap(bar2_addr, 
		pci_resource_len(pcidev,2)))) /*MEM window for SJA1000 chips*/
	{
		CANMSG("Unable to access I/O memory at: 0x%lx\n", (unsigned long)bar2_addr);
		goto ioremap_error;
	}

	can_base_addr_fixup(candev, remap_addr);
	CANMSG("esdpci200_sja IO-memory: 0x%lx - 0x%lx (VMA 0x%lx)\n", 
		(unsigned long) bar2_addr,
		(unsigned long) bar2_addr + pci_resource_len(pcidev,2) - 1,
		(long) remap_addr);

	return 0;

    ioremap_error:
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))
	pci_release_region(pcidev, 2);
	pci_release_region(pcidev, 1);
	pci_release_region(pcidev, 0);
#else /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
	pci_release_regions(pcidev);
#endif /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
	return -ENODEV;
}

int esdpci200_release_io(struct candevice_t *candev)
{
	esdpci200_disconnect_irq(candev);
	esdpci200_pci_soft_reset(candev);

	iounmap(candev->dev_base_addr);
    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))
	pci_release_region(candev->sysdevptr.pcidev, 2);
	pci_release_region(candev->sysdevptr.pcidev, 1);
	pci_release_region(candev->sysdevptr.pcidev, 0);
    #else /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
	pci_release_regions(candev->sysdevptr.pcidev);
    #endif /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/

	return 0;
}

void esdpci200_write_register(unsigned data, can_ioptr_t address)
{
	iowrite8((u8)data,(void*)address);
	wmb();
}

unsigned esdpci200_read_register(can_ioptr_t address)
{
	return ioread8((void*)address);
}

int esdpci200_reset(struct candevice_t *candev)
{
	int i=0,chip_nr;
	struct canchip_t *chip;
	unsigned cdr;
	DEBUGMSG("Resetting esdpci200 hardware ...\n");

	esdpci200_disconnect_irq(candev);
	esdpci200_pci_soft_reset(candev);

	for(chip_nr=0;chip_nr<candev->nr_all_chips;chip_nr++){
		if(!candev->chip[chip_nr]) continue;
		chip=candev->chip[chip_nr];

		esdpci200_write_register(sjaMOD_RM, chip->chip_base_addr+SJAMOD);
		udelay(1000);

		cdr=esdpci200_read_register(chip->chip_base_addr+SJACDR);
		esdpci200_write_register(cdr|sjaCDR_PELICAN, chip->chip_base_addr+SJACDR);

		esdpci200_write_register(0, chip->chip_base_addr+SJAIER);

		i=20;
		esdpci200_write_register(0, chip->chip_base_addr+SJAMOD);
		while (esdpci200_read_register(chip->chip_base_addr+SJAMOD)&sjaMOD_RM){
			if(!i--) return -ENODEV;
			udelay(1000);
			esdpci200_write_register(0, chip->chip_base_addr+SJAMOD);
		}

		cdr=esdpci200_read_register(chip->chip_base_addr+SJACDR);
		esdpci200_write_register(cdr|sjaCDR_PELICAN, chip->chip_base_addr+SJACDR);

		esdpci200_write_register(0, chip->chip_base_addr+SJAIER);
		
		esdpci200_read_register(chip->chip_base_addr+SJAIR);
	}
	

	esdpci200_connect_irq(candev);

	return 0;
} 	

int esdpci200_init_hw_data(struct candevice_t *candev)
{
	struct pci_dev *pcidev = NULL;

	do {
		pcidev = pci_find_device(ESDPCI200_PCI_VENDOR_ID, ESDPCI200_PCI_PRODUCT_ID, pcidev);
		if(pcidev == NULL) return -ENODEV;
	} while(can_check_dev_taken(pcidev));
	
	if (pci_enable_device (pcidev)){
		printk(KERN_CRIT "Setup of ESDPCI200 failed\n");
		return -EIO;
	}
	candev->sysdevptr.pcidev=pcidev;

	can_spin_lock_init(&candev->device_lock);

	if(!(pci_resource_flags(pcidev, 0)&IORESOURCE_MEM))
	{
	   printk(KERN_CRIT "PCI200 region %d is not MEM\n",0);
	   return -EIO;
	}
	if(!(pci_resource_flags(pcidev, 1)&IORESOURCE_IO))
	{
	   printk(KERN_CRIT "PCI200 region %d is not IO\n",1);
	   return -EIO;
	}

	if(!(pci_resource_flags(pcidev,2)&IORESOURCE_MEM))
	{
	   printk(KERN_CRIT "PCI200 region %d is not MEM\n",2);
	   return -EIO;
	}

	 /* Reset/control field - used to store port of PLX9052 control region */
	candev->res_addr = pci_resource_start(pcidev,1);;

	/* Physical address of SJA1000 window, stored for debugging only */
	candev->io_addr = pci_resource_start(pcidev,2);
	
	candev->aux_base_addr=NULL; /* mapped dynamically in esdpci200_request_io() */
	candev->dev_base_addr=NULL; /* mapped dynamically in esdpci200_request_io() */
	/*candev->flags |= CANDEV_PROGRAMMABLE_IRQ;*/

	candev->nr_82527_chips=0;
	candev->nr_sja1000_chips=2;
	candev->nr_all_chips=2;

	return 0;
}

int esdpci200_init_chip_data(struct candevice_t *candev, int chipnr)
{

	if(candev->sysdevptr.pcidev==NULL)
		return -ENODEV;	

	CANMSG("initializing esdpci200 chip operations\n");


	sja1000p_fill_chipspecops(candev->chip[chipnr]);
	candev->chip[chipnr]->chip_base_addr=candev->dev_base_addr + chipnr * IO_RANGE;

	candev->chip[chipnr]->chipspecops->irq_handler=esdpci200_irq_handler;

	candev->chip[chipnr]->flags = 0;
	candev->chip[chipnr]->int_cpu_reg = 0; /* i82527 specific */
	candev->chip[chipnr]->int_clk_reg = 0; /* i82527 specific */
	candev->chip[chipnr]->int_bus_reg = 0; /* i82527 specific */
	candev->chip[chipnr]->sja_cdr_reg = sjaCDR_CBP | sjaCDR_CLK_OFF; /* hardware specific options for the Clock Divider register */
	candev->chip[chipnr]->sja_ocr_reg = ESDPCI200_OCR_DEFAULT_STD; /* hardware specific options for the Output Control register */
	candev->chip[chipnr]->clock = 16000000;
	candev->chip[chipnr]->chip_irq=candev->sysdevptr.pcidev->irq;
	candev->chip[chipnr]->flags |= CHIP_IRQ_PCI;
	if( chipnr > 0 ) /* only one IRQ used for both channels.
			    CHIP_IRQ_CUSTOM req'd for RTAI, since 
			    registering two handlers for the same IRQ 
			    returns an error */
		candev->chip[chipnr]->flags |= CHIP_IRQ_CUSTOM;

	return 0;
}	

int esdpci200_init_obj_data(struct canchip_t *chip, int objnr)
{
	chip->msgobj[objnr]->obj_base_addr=chip->chip_base_addr;
	return 0;
}

int esdpci200_program_irq(struct candevice_t *candev)
{

	return 0;
}

int esdpci200_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = esdpci200_request_io;
	hwspecops->release_io = esdpci200_release_io;
	hwspecops->reset = esdpci200_reset;
	hwspecops->init_hw_data = esdpci200_init_hw_data;
	hwspecops->init_chip_data = esdpci200_init_chip_data;
	hwspecops->init_obj_data = esdpci200_init_obj_data;
	hwspecops->write_register = esdpci200_write_register;
	hwspecops->read_register = esdpci200_read_register;
	hwspecops->program_irq = esdpci200_program_irq;
	return 0;
}


#endif /*CAN_ENABLE_PCI_SUPPORT*/
