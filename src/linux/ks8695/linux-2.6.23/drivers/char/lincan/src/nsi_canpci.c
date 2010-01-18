/* nsi.c
 * Linux CAN-bus device driver.
 * nsi_canpci.c - support for NSI CAN PCI card
 * The card support added by Eric Pennamen <pennamen@gmail.com>
 * Based on code from Arnaud Westenberg email:arnaud@wanadoo.nl
 * Ake Hedman, eurosource, akhe@eurosource.se ,
 * and Pavel Pisa - OCERA team member email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */ 

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/nsi_canpci.h"
#include "../include/i82527.h"

extern int stdmask;
extern int extmask;
extern int mo15mask;

#define __NO_VERSION__
#include <linux/module.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10))
	#define ioread32	can_readl
	#define iowrite32	can_writel
	#define ioread8		can_readb
	#define iowrite8	can_writeb
#else
#endif



#define INT_CONF 0x00000040 	/* value for register INTCSR of PLX */
#define NSI_VENDOR_ID		0x1637
#define NSI_CANPCI_DEVICE_ID 	0x0001

enum PORT2 { P2_0=1, P2_1=1<<1, P2_2=1<<2, P2_3=1<<3, P2_4=1<<4, P2_5=1<<5, P2_6=1<<6, P2_7=1<<7 };

/*PLX register definition */
#define PLX_CNTRL	0x50	/* Controle register */
#define PLX_INTCSR	0x4C	/* Interruption controle register */

/* This value define the i82527 clock frequency */
#define iCLOCK		16000000


/* Il faut reserver 4 zones:
 *		BAR0: 128 octets memoire (32bits) pour les registres du PLX9052
 *		BAR1: 128 octets I/O pour les registres du PLX9052
 *		BAR2: 256 octets memoire(8bits) pour les registres du PLX9052
 *		BAR3: 256 octets memoire(8bits) pour les registres du PLX9052
 */
/* Variables globales contenant les @ des IO-Memory apres remap */
#define NB_VALID_BAR	4

typedef struct {
	void* addr_BAR_remap[NB_VALID_BAR];
}t_CardArray;

void nsi_canpci_connect_irq(struct candevice_t *candev)
{
//Not used
}
void nsi_canpci_disconnect_irq(struct candevice_t *candev)
{
//on disconnecting interrupt we need to disable interruption form PLX
	iowrite32(0x0,(void*)(candev->io_addr+PLX_INTCSR));
	DEBUGMSG("PLX interrupt disabled\n");
}

int nsi_canpci_config_irqs(struct canchip_t *chip, short irqs)
{
	
	unsigned long it_mask,it_reg;
	struct candevice_t *candev;
	it_mask=0;
	DEBUGMSG("Configuring NSI CANPCI interrupt\n");
	can_write_reg(chip,irqs,iCTL);
	if( (irqs&0x0E)!=0)
	{//At least one interrupt source requested
		if(chip->chip_idx==0)
		{
			DEBUGMSG("starting interrupt on chip 0\n");
			it_mask=1;
		}
		else
		{
			DEBUGMSG("starting interrupt on chip 1\n");
			it_mask=8;
		}
		candev=chip->hostdevice;
		it_reg = ioread32( (void*)(candev->io_addr+PLX_INTCSR));
		it_reg|=it_mask|0x40;
		iowrite32(it_reg,(void*)(candev->io_addr+PLX_INTCSR));
	}
	else
	{//No more interrupt source
		if(chip->chip_idx==0)
		{
			DEBUGMSG("stoping interrupt on chip 0\n");
			it_mask=1;
		}
		else
		{
			DEBUGMSG("stoping interrupt on chip 1\n");
			it_mask=8;
		}
		candev=chip->hostdevice;
		it_reg = ioread32( (void*)(candev->io_addr+PLX_INTCSR));
		it_reg&=~it_mask;
		iowrite32(it_reg,(void*)(candev->io_addr+PLX_INTCSR));
	}
	return 0;
}

int nsi_canpci_i82527_chip_config(struct canchip_t *chip)
{
	can_write_reg(chip,chip->int_cpu_reg,iCPU); // Configure cpu interface
	can_write_reg(chip,(iCTL_CCE|iCTL_INI),iCTL); // Enable configuration
	i82527_seg_write_reg(chip,chip->int_clk_reg,iCLK); // Set clock out slew rates 
	i82527_seg_write_reg(chip,chip->int_bus_reg,iBUS); /* Bus configuration */
	
	can_write_reg(chip,P2_2|P2_1,iP2C); // The pin P2_2,P2_1 of the 527 must be set as output
	can_write_reg(chip,P2_2|P2_1,iP2O); // and P2_2 must be set to 1
	
	can_write_reg(chip,0x00,iSTAT); /* Clear error status register */

	/* Check if we can at least read back some arbitrary data from the 
	 * card. If we can not, the card is not properly configured!
	 */
	canobj_write_reg(chip,chip->msgobj[1],0x25,iMSGDAT1);
	canobj_write_reg(chip,chip->msgobj[2],0x52,iMSGDAT3);
	canobj_write_reg(chip,chip->msgobj[10],0xc3,iMSGDAT6);
	if ( (canobj_read_reg(chip,chip->msgobj[1],iMSGDAT1) != 0x25) ||
	      (canobj_read_reg(chip,chip->msgobj[2],iMSGDAT3) != 0x52) ||
	      (canobj_read_reg(chip,chip->msgobj[10],iMSGDAT6) != 0xc3) ) {
		CANMSG("Could not read back from the hardware.\n");
		CANMSG("This probably means that your hardware is not correctly configured!\n");
		return -1;
	}
	else
		DEBUGMSG("Could read back, hardware is probably configured correctly\n");

	if (chip->baudrate == 0)
		chip->baudrate=1600000;

	if (i82527_baud_rate(chip,chip->baudrate,chip->clock,0,75,0)) {
		CANMSG("Error configuring baud rate\n");
		return -ENODEV;
	}
	if (i82527_standard_mask(chip,0x0000,stdmask)) {
		CANMSG("Error configuring standard mask\n");
		return -ENODEV;
	}
	if (i82527_extended_mask(chip,0x00000000,extmask)) {
		CANMSG("Error configuring extended mask\n");
		return -ENODEV;
	}
	if (i82527_message15_mask(chip,0x00000000,mo15mask)) {
		CANMSG("Error configuring message 15 mask\n");
		return -ENODEV;
	}
	if (i82527_clear_objects(chip)) {
		CANMSG("Error clearing message objects\n");
		return -ENODEV;
	}
	
	if (nsi_canpci_config_irqs(chip,iCTL_IE|iCTL_EIE)) { /* has been 0x0a */
		CANMSG("Error configuring interrupts\n");
		return -ENODEV;
	}	
	return 0;
}


int nsi_canpci_start_chip(struct canchip_t *chip)
{
	unsigned long it_mask,it_reg;
	struct candevice_t *candev;
	it_mask=0;
	if(chip->chip_idx==0)
	{
		DEBUGMSG("starting chip 0\n");
		it_mask=1;
	}
	else
	{
		DEBUGMSG("starting chip 1\n");
		it_mask=8;
	}
	candev=chip->hostdevice;
	it_reg = ioread32( (void*)(candev->io_addr+PLX_INTCSR));
	rmb();
	it_reg|=it_mask|0x40;
	iowrite32(it_reg,(void*)(candev->io_addr+PLX_INTCSR));
	wmb();	
	i82527_start_chip(chip);
	return 0;
}

int nsi_canpci_stop_chip(struct canchip_t *chip)
{
	unsigned long it_mask,it_reg;
	struct candevice_t *candev;
	it_mask=0;
	if(chip->chip_idx==0)
	{
		DEBUGMSG("stoping chip 0\n");
		it_mask=1;
	}
	else
	{
		DEBUGMSG("stoping chip 1\n");
		it_mask=8;
	}
	candev=chip->hostdevice;
	it_reg = ioread32( (void*)(candev->io_addr+PLX_INTCSR));
	rmb();
	it_reg&=~it_mask;
	iowrite32(it_reg,(void*)(candev->io_addr+PLX_INTCSR));
	wmb();	
	i82527_stop_chip(chip);
	return 0;
}

int nsi_canpci_irq_handler(int irq, struct canchip_t *chip)
{
	int retcode;
	unsigned long it_reg;
	struct candevice_t *candev;
	candev=chip->hostdevice;
	retcode = CANCHIP_IRQ_NONE;
	it_reg = ioread32( (void*)(candev->io_addr+PLX_INTCSR));
	rmb();
	if(chip->chip_idx==0)
	{
		if((it_reg &0x4)!=0) //interrupt active
		{
			if(i82527_irq_handler(irq,chip)==CANCHIP_IRQ_NONE)
			{//some trouble with IT
				it_reg&=~(0x01);
				CANMSG("Unexcepted interruption from canal0, interruption is canceled\n");

			}else
			{
				retcode=CANCHIP_IRQ_HANDLED;
			}
		
		}
	}
	else
	{
		if((it_reg &0x20)!=0) //interrupt is set
		{
			if(i82527_irq_handler(irq,chip)==CANCHIP_IRQ_NONE)
			{//soucis avec les IT
				it_reg&=~(0x08);
				CANMSG("Unexcepted interruption from canal1, interruption is canceled\n");
			}else
			{
				retcode=CANCHIP_IRQ_HANDLED;
			}	
		}
	}
	return retcode;
}

/* The function template_request_io is used to reserve the io-memory. If your
 * hardware uses a dedicated memory range as hardware control registers you
 * will have to add the code to reserve this memory as well.
 * The reserved memory starts at candev->io_addr, wich is the module parameter io.
 */
int nsi_canpci_request_io(struct candevice_t *candev)
{
  (void)candev;
  if(candev->dev_base_addr==0)
	return -EIO;  
  return 0;
}

/* The function template_release_io is used to free the previously reserved 
 * io-memory. In case you reserved more memory, don't forget to free it here.
 */
int nsi_canpci_release_io(struct candevice_t *candev)
{
	unsigned long reg_reset;
	struct pci_dev *pcidev = candev->sysdevptr.pcidev;
	DEBUGMSG("Releasing board io\n");
	
	nsi_canpci_disconnect_irq(candev);
	// First, set RESET signal to 0
	reg_reset = ioread32( (void*)(candev->io_addr+PLX_CNTRL));
	reg_reset&=(~(0x40000000));
	rmb();
	//Then set it to '1' for reseting the board
	iowrite32( (reg_reset | 0x40000000 ),(void*)(candev->io_addr+PLX_CNTRL));
	wmb();
	udelay(2500); /* This delay must be greater than 1ms for i82527 */
	iowrite32( (reg_reset ),(void*)(candev->io_addr+PLX_CNTRL)); //Releasing RESET signal
	wmb();
	udelay(2500); // Waiting for some additionnal time before writing in the 82527
  	iounmap(((t_CardArray*)(candev->dev_base_addr))->addr_BAR_remap[0]);
  	iounmap(((t_CardArray*)(candev->dev_base_addr))->addr_BAR_remap[1]);
  	iounmap(((t_CardArray*)(candev->dev_base_addr))->addr_BAR_remap[2]);
  	iounmap(((t_CardArray*)(candev->dev_base_addr))->addr_BAR_remap[3]);
  	kfree((void*)(candev->dev_base_addr));
	pci_release_region(pcidev,0);
	pci_release_region(pcidev,1);
	pci_release_region(pcidev,2);			
	pci_release_region(pcidev,3);			
	return 0;
}

/* The function template_reset is used to give a hardware reset. This is rather
 * hardware specific so I haven't included example code. Don't forget to check
 * the reset status of the chip before returning.
 */
int nsi_canpci_reset(struct candevice_t *candev)
{
	unsigned long reg_reset;
	
	DEBUGMSG("Board reset !!!\n");
	// Before reset disconnet interrupt to avoir freeze	
	nsi_canpci_disconnect_irq(candev);
	// First, set RESET signal to 0
	reg_reset = ioread32( (void*)(candev->io_addr+PLX_CNTRL));
	reg_reset&=(~(0x40000000));
	//Then set it to '1' for reseting the board
	iowrite32( (reg_reset | 0x40000000 ),(void*)(candev->io_addr+PLX_CNTRL));
	wmb();
	udelay(2500); /* This delay must be greater than 1ms for i82527 */
	iowrite32(reg_reset,(void*)(candev->io_addr+PLX_CNTRL)); //Releasing RESET signal
	wmb();
	udelay(2500); // Waiting for some additionnal time before writing in the 82527
	DEBUGMSG("Reset done !!!\n");
	
	nsi_canpci_connect_irq(candev);
	return 0;	
}

/* The function template_init_hw_data is used to initialize the hardware
 * structure containing information about the installed CAN-board.
 * RESET_ADDR represents the io-address of the hardware reset register.
 * NR_82527 represents the number of intel 82527 chips on the board.
 * NR_SJA1000 represents the number of philips sja1000 chips on the board.
 * The flags entry can currently only be CANDEV_PROGRAMMABLE_IRQ to indicate that
 * the hardware uses programmable interrupts.
 */

int nsi_canpci_init_hw_data(struct candevice_t *candev) 
     {
  struct pci_dev *pcidev = NULL;

  /* looking for NSI CANPCI ident on the pci bus*/
  do
  {
    pcidev = pci_find_device(NSI_VENDOR_ID, NSI_CANPCI_DEVICE_ID, pcidev);
  }
  while(can_check_dev_taken(pcidev));
  
  if(pcidev == NULL) 
  {
	do
	{
	pcidev = pci_find_device(NSI_VENDOR_ID, NSI_CANPCI_DEVICE_ID+1, pcidev);
	}
	while(can_check_dev_taken(pcidev));
	if(pcidev == NULL) 
	{
		CANMSG ("Error : NSI CAN PCI device not found\n");
		return -ENODEV;
	}
	else
	{
		CANMSG ("NSI CANPCI OPTO device found\n");
	}
  }
  else
  {
	CANMSG ("NSI CANPCI device found\n");  
  }
    
  /* enable it */
  if (pci_enable_device (pcidev))
  {
    CANMSG ("Cannot enable PCI device\n");
    return -EIO;
  }
  CANMSG ("NSI CANPCI device started\n");
  candev->sysdevptr.pcidev = pcidev;
  candev->res_addr=0;
  candev->nr_82527_chips=2;
  candev->nr_sja1000_chips=0;
  candev->nr_all_chips=2; 
  /* initialize device spinlock */
  can_spin_lock_init(&candev->device_lock);

  if(pci_request_region(pcidev,0,"nsi_canpci bar0")==0)
  {
  	if(pci_request_region(pcidev,1,"nsi_canpci bar1")==0)
	{
		if(pci_request_region(pcidev,2,"nsi_canpci bar2")==0)
		{
			if(pci_request_region(pcidev,3,"nsi_canpci bar3")==0)
			{
			}
			else
			{
			pci_release_region(pcidev,0);
			pci_release_region(pcidev,1);
			pci_release_region(pcidev,2);			
			return -EIO;
			}
		}
		else
		{
		pci_release_region(pcidev,0);
		pci_release_region(pcidev,1);
		return -EIO;
		}
	}
	else
	{
	pci_release_region(pcidev,0);
	return -EIO;
	}
  }  
  else
  {
	return -EIO;
  }
  candev->dev_base_addr=(unsigned long)(kmalloc(sizeof(t_CardArray),GFP_ATOMIC));  
  
  if((unsigned long)candev->dev_base_addr==0)
  	return -EIO;
  //PLX register 
  ((t_CardArray*)(candev->dev_base_addr))->addr_BAR_remap[0]=ioremap(pci_resource_start(pcidev,0),pci_resource_len(pcidev,0) );
  //PLX IO
  ((t_CardArray*)(candev->dev_base_addr))->addr_BAR_remap[1]=ioremap(pci_resource_start(pcidev,1),pci_resource_len(pcidev,1) );
  //Chip 0
  ((t_CardArray*)(candev->dev_base_addr))->addr_BAR_remap[2]=ioremap(pci_resource_start(pcidev,2),pci_resource_len(pcidev,2) );
  //Chip 1
  ((t_CardArray*)(candev->dev_base_addr))->addr_BAR_remap[3]=ioremap(pci_resource_start(pcidev,3),pci_resource_len(pcidev,3) );
  
  //Short acces to plx register
  candev->io_addr=(unsigned long)(((t_CardArray*)(candev->dev_base_addr))->addr_BAR_remap[0]);
  return 0;  
}

/* The function template_init_chip_data is used to initialize the hardware
 * structure containing information about the CAN chips.
 * CHIP_TYPE represents the type of CAN chip. CHIP_TYPE can be "i82527" or
 * "sja1000".
 * The chip_base_addr entry represents the start of the 'official' memory map
 * of the installed chip. It's likely that this is the same as the candev->io_addr
 * argument supplied at module loading time.
 * The clock argument holds the chip clock value in Hz.
 */

int nsi_canpci_init_chip_data(struct candevice_t *candev, int chipnr)
{
	//u8 irq_line;
 	CANMSG ("NSI chip data init %d\n",chipnr);
	i82527_fill_chipspecops(candev->chip[chipnr]);
	
	candev->chip[chipnr]->chipspecops->chip_config =nsi_canpci_i82527_chip_config;
	candev->chip[chipnr]->chipspecops->start_chip=nsi_canpci_start_chip;
	candev->chip[chipnr]->chipspecops->stop_chip=nsi_canpci_stop_chip;
	candev->chip[chipnr]->chipspecops->config_irqs=nsi_canpci_config_irqs;
	candev->chip[chipnr]->chipspecops->irq_handler=nsi_canpci_irq_handler;
	/*candev->chip[chipnr]->chip_data = NULL;*/
	
	candev->chip[chipnr]->chip_base_addr= (unsigned long) (((t_CardArray*)(candev->dev_base_addr))->addr_BAR_remap[chipnr+2]);
	candev->chip[chipnr]->clock = iCLOCK;
	candev->chip[chipnr]->chip_irq=candev->sysdevptr.pcidev->irq;
	candev->chip[chipnr]->flags=CHIP_IRQ_PCI;
	candev->chip[chipnr]->int_cpu_reg = iCPU_DSC+iCPU_CEN;
	candev->chip[chipnr]->int_clk_reg = iCLK_SL1+iCLK_CD0;
	candev->chip[chipnr]->int_bus_reg = iBUS_CBY;
	return 0;
}

 /* The function template_init_obj_data is used to initialize the hardware
 * structure containing information about the different message objects on the
 * CAN chip. In case of the sja1000 there's only one message object but on the
 * i82527 chip there are 15.
 * The code below is for a i82527 chip and initializes the object base addresses
 * The entry obj_base_addr represents the first memory address of the message 
 * object. In case of the sja1000 obj_base_addr is taken the same as the chips
 * base address.
 * Unless the hardware uses a segmented memory map, flags can be set zero.
 */
int nsi_canpci_init_obj_data(struct canchip_t *chip, int objnr)
{

	
	chip->msgobj[objnr]->obj_base_addr=
	    chip->chip_base_addr+(objnr+1)*0x10;
	
	return 0;
}

/* The function template_program_irq is used for hardware that uses programmable
 * interrupts. If your hardware doesn't use programmable interrupts you should
 * not set the candevices_t->flags entry to CANDEV_PROGRAMMABLE_IRQ and leave this
 * function unedited. Again this function is hardware specific so there's no
 * example code.
 */
int nsi_canpci_program_irq(struct candevice_t *candev)
{
	return 0;
}

/* The function template_write_register is used to write to hardware registers
 * on the CAN chip. You should only have to edit this function if your hardware
 * uses some specific write process.
 */
void nsi_canpci_write_register(unsigned data, can_ioptr_t address)
{
	iowrite8((u8)data,(void*)address);
}

/* The function template_read_register is used to read from hardware registers
 * on the CAN chip. You should only have to edit this function if your hardware
 * uses some specific read process.
 */
unsigned nsi_canpci_read_register(can_ioptr_t address)
{
	return ioread8((void*)address);
}


 /* !!! Don't change this function !!! */
int nsi_canpci_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = nsi_canpci_request_io;
	hwspecops->release_io = nsi_canpci_release_io;
	hwspecops->reset = nsi_canpci_reset;
	hwspecops->init_hw_data = nsi_canpci_init_hw_data;
	hwspecops->init_chip_data = nsi_canpci_init_chip_data;
	hwspecops->init_obj_data = nsi_canpci_init_obj_data;
	hwspecops->write_register = nsi_canpci_write_register;
	hwspecops->read_register = nsi_canpci_read_register;
	hwspecops->program_irq = nsi_canpci_program_irq;
	return 0;
}
