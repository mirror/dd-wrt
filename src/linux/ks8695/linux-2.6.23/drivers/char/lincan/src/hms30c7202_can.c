/* hms30c7202_can.c - Hynix HMS30c7202 ARM device specific code
 * Linux CAN-bus device driver.
 * Written by Sebastian Stolzenberg email:stolzi@sebastian-stolzenberg.de
 * Based on code from Arnaud Westenberg email:arnaud@wanadoo.nl
 * and Ake Hedman, eurosource, akhe@eurosource.se
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#include <linux/delay.h>

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/c_can.h"
#include "../include/hms30c7202_can.h"

/*
 * IO_RANGE is the io-memory range that gets reserved, please adjust according
 * your hardware. Example: #define IO_RANGE 0x100 for i82527 chips or
 * #define IO_RANGE 0x20 for sja1000 chips in basic CAN mode.
 */
#define IO_RANGE 0x17E

/**
 * hms30c7202_request_io: - reserve io or memory range for can board
 * @candev: pointer to candevice/board which asks for io. Field @io_addr
 *	of @candev is used in most cases to define start of the range
 *
 * The function hms30c7202_request_io() is used to reserve the io-memory. If your
 * hardware uses a dedicated memory range as hardware control registers you
 * will have to add the code to reserve this memory as well. 
 * %IO_RANGE is the io-memory range that gets reserved, please adjust according
 * your hardware. Example: #define IO_RANGE 0x100 for i82527 chips or
 * #define IO_RANGE 0x20 for sja1000 chips in basic CAN mode.
 * Return Value: The function returns zero on success or %-ENODEV on failure
 * File: src/template.c
 */
int hms30c7202_request_io(struct candevice_t *candev)
{
	DEBUGMSG("(c%d)calling hms30c7202_request_io(...)\n", candev->chip[0]->chip_idx);

	if(!can_request_mem_region(candev->io_addr, IO_RANGE, DEVICE_NAME )) {
		CANMSG("hmsc30c7202_can failed to request mem region %lx.\n",
		(unsigned long)candev->io_addr );
	}
	
	if (!( candev->dev_base_addr = ioremap( candev->io_addr, IO_RANGE ))) {
		DEBUGMSG( "Failed to map IO-memory: 0x%lx - 0x%lx, mapped to 0x%lx\n",
			(unsigned long)candev->io_addr,
			(unsigned long)candev->io_addr + IO_RANGE - 1,
			(unsigned long)candev->dev_base_addr);
		can_release_mem_region(candev->io_addr, IO_RANGE);
		return -ENODEV;
	} else {
	
		DEBUGMSG( "Mapped IO-memory: 0x%lx - 0x%lx, mapped to 0x%lx\n",
			(unsigned long)candev->io_addr,
			(unsigned long)candev->io_addr + IO_RANGE - 1,
			(unsigned long)candev->dev_base_addr);
	
	}
	
	candev->chip[0]->chip_base_addr=candev->dev_base_addr;
	
	//pchip->write_register(0, pchip->vbase_addr + CCCR);
	//DEBUGMSG("C-CAN Control Register : 0x%.4lx\n",
	//	(unsigned long)(c_can_read_reg_w( pchip->vbase_addr + CCCR)));
	candev->chip[0]->chipspecops->start_chip(candev->chip[0]);
	//DEBUGMSG("C-CAN Control Register : 0x%.4lx\n",
	//	(unsigned long)(c_can_read_reg_w( pchip->vbase_addr + CCCR)));
	
	//DEBUGMSG("hms30c7202_can request i/o, leaving.\n");
	return 0;
}


/**
 * hms30c7202_release_io - free reserved io memory range
 * @candev: pointer to candevice/board which releases io
 *
 * The function hms30c7202_release_io() is used to free reserved io-memory.
 * In case you have reserved more io memory, don't forget to free it here.
 * IO_RANGE is the io-memory range that gets released, please adjust according
 * your hardware. Example: #define IO_RANGE 0x100 for i82527 chips or
 * #define IO_RANGE 0x20 for sja1000 chips in basic CAN mode.
 * Return Value: The function always returns zero
 * File: src/template.c
 */
int hms30c7202_release_io(struct candevice_t *candev)
{
	u16 tempReg;
	
	//disable IRQ generation
	tempReg = c_can_read_reg_w(candev->chip[0], CCCR);

	c_can_config_irqs(candev->chip[0], 0);
	
	/*  // clear all message objects
	for (i=1; i<=15; i++) {
	ccscan_write_register(
			INTPD_RES |
			RXIE_RES |
			TXIE_RES |
			MVAL_RES,
			pchip->vbase_addr +
			i*0x10 + iMSGCTL0 );
	ccscan_write_register(
			NEWD_RES |
			MLST_RES |
			CPUU_RES |
			TXRQ_RES |
			RMPD_RES,
			pchip->vbase_addr +
			i*0x10 + iMSGCTL1 );
	}
	*/
	// power down HMS30c7202 - C_CAN
	candev->chip[0]->chipspecops->stop_chip(candev->chip[0]);
	
	// release I/O memory mapping
	iounmap(candev->dev_base_addr);
	
	// Release the memory region
	can_release_mem_region(candev->io_addr, IO_RANGE);
	
	return 0;
}

/**
 * hms30c7202_reset - hardware reset routine
 * @card: Number of the hardware card.
 *
 * The function hms30c7202_reset() is used to give a hardware reset. This is
 * rather hardware specific so I haven't included example code. Don't forget to
 * check the reset status of the chip before returning.
 * Return Value: The function returns zero on success or %-ENODEV on failure
 * File: src/template.c
 */
int hms30c7202_reset(  struct candevice_t *candev)
{
	int i=0;
	int enableTest=0, disableTest=0;
	struct canchip_t *pchip = candev->chip[0];
	
	enableTest = pchip->chipspecops->enable_configuration(pchip);
	disableTest = pchip->chipspecops->disable_configuration(pchip);
	if( enableTest || disableTest) {
		CANMSG("Reset status timeout!\n");
		CANMSG("Please check your hardware.\n");
		return -ENODEV;
	}
	
	/* Check busoff status */
	
	while ( (c_can_read_reg_w(pchip, CCSR) & SR_BOFF) && (i<=15)) {
		udelay(20000);
		i++;
	}
	if (i>=15) {
		CANMSG("Reset status timeout!\n");
		CANMSG("Please check your hardware.\n");
		return -ENODEV;
	}
	else
		DEBUGMSG("Chip0 reset status ok.\n");
	
	//pchip->config_irqs(pchip, CR_MIE | CR_SIE | CR_EIE);
	return 0;
}

#define RESET_ADDR 0x0
#define NR_C_CAN 1
#define NR_MSGOBJ 32

/**
 * hms30c7202_init_hw_data - Initialize hardware cards
 * @candev: Pointer to candevice/board structure
 *
 * The function hms30c7202_init_hw_data() is used to initialize the hardware
 * structure containing information about the installed CAN-board.
 * %RESET_ADDR represents the io-address of the hardware reset register.
 * %NR_82527 represents the number of intel 82527 chips on the board.
 * %NR_SJA1000 represents the number of philips sja1000 chips on the board.
 * The flags entry can currently only be %CANDEV_PROGRAMMABLE_IRQ to indicate that
 * the hardware uses programmable interrupts.
 * Return Value: The function always returns zero
 * File: src/template.c
 */
int hms30c7202_init_hw_data(struct candevice_t *candev) 
/*( struct canchip_t *pchip, u16 chip_nr, u16 startminor, u32 baseaddr, u8 irq )*/
{
	//	u32 intCntrVAddr = 0;
	can_ioptr_t gpioVAddr = 0;
	u32 tempReg = 0;
	u32 baseaddr=candev->io_addr;
	
	//	if ( (!( intCntrVAddr = ioremap( 0x80024000, 0xCD ) ))
	//		& (! ( gpioVAddr = ioremap( 0x80023000, 0xAD ) ))) {
	//		DEBUGMSG("Failed to map Int and GPIO memory\n");
	//		return -EIO;
	//	}
	if ( ! ( gpioVAddr = ioremap( 0x80023000, 0xAD ) )) {
		DEBUGMSG("Failed to map GPIO memory\n");
		return -EIO;
	} else {
	
		//   DEBUGMSG( "Mapped Interrupt Controller IO-memory: 0x%lx - 0x%lx to 0x%lx\n",
		//	      (unsigned long)0X80024000,
		//	      (unsigned long)0X800240CC,
		//	      (unsigned long)intCntrVAddr);
		DEBUGMSG( "Mapped GPIO IO-memory: 0x%lx - 0x%lx to 0x%lx\n",
			(unsigned long)0X80023000,
			(unsigned long)0X800240AC,
			(unsigned long)gpioVAddr);
	}
	
	if (baseaddr == 0x8002f000) {
		//		tempReg = can_readl(intCntrVAddr);
		//		DEBUGMSG("Read Interrupt Enable Register : 0x%.4lx\n",(long)tempReg);
		//		DEBUGMSG("Trying to activate CAN0 Interrupt (Bit 18)\n");
		//		can_writel((tempReg | (1<<18)), intCntrVAddr);
		//		tempReg = can_readl(intCntrVAddr);
		//		DEBUGMSG("Read changed Interrupt Enable Register : 0x%.4lx\n",(long)tempReg);
		tempReg = can_readl(gpioVAddr + 0x5C);
		DEBUGMSG("Read GPIO-C Enable Register : 0x%.4lx\n",(long)tempReg);
		DEBUGMSG("Trying to activate CAN0 (Bit 1 = 0 for CANTx0, Bit 2 = 0 for CANRx0,)\n");
		can_writel(tempReg & ~0x6, gpioVAddr + 0x5C);
		tempReg = can_readl(gpioVAddr + 0x5C);
		DEBUGMSG("Read changed GPIO-C Enable Register : 0x%.4lx\n",(long)tempReg);
		tempReg = can_readl(gpioVAddr + 0x44);
		DEBUGMSG("Read GPIO-C Direction Register : 0x%.4lx\n",(long)tempReg);
		DEBUGMSG("Trying to set CAN0 directions (Bit 1 = 0 for CANTx0 as OUT, Bit 2 = 1 for CANRx0 as IN,)\n");
		can_writel((tempReg & ~0x2) | 0x4, gpioVAddr + 0x44);
		tempReg = can_readl(gpioVAddr + 0x44);
		DEBUGMSG("Read changed GPIO-C Direction Register : 0x%.4lx\n",(long)tempReg);
	}
	else if (baseaddr == 0x80030000) {
		//		tempReg = can_readl(intCntrVAddr);
		//		can_writel((tempReg | (1<<19)), intCntrVAddr);
		tempReg = can_readl(gpioVAddr + 0x9C);
		DEBUGMSG("Read GPIO-E Enable Register : 0x%.8lx\n",(long)tempReg);
		DEBUGMSG("Trying to activate CAN1 (Bit 22 = 0 for CANRx1, Bit 23 = 0 for CANTx1,)\n");
		can_writel(tempReg & 0xFF3FFFFF, gpioVAddr + 0x9C);
		tempReg = can_readl(gpioVAddr + 0x9C);
		DEBUGMSG("Read changed GPIO-E Enable Register : 0x%.8lx\n",(long)tempReg);
		tempReg = can_readl(gpioVAddr + 0x84);
		DEBUGMSG("Read GPIO-E Direction Register : 0x%.8lx\n",(long)tempReg);
		DEBUGMSG("Trying to set CAN1 directions (Bit 22 = 1 for CANRx1 as IN, Bit 23 = 0 for CANTx1 as OUT,)\n");
		can_writel((tempReg & ~(1<<23)) | 1<<22, gpioVAddr + 0x84);
		tempReg = can_readl(gpioVAddr + 0x84);
		DEBUGMSG("Read changed GPIO-E Direction Register : 0x%.8lx\n",(long)tempReg);
	}

	//DEBUGMSG("Current Interrupt Status Register (ISR): 0x%4.4lx\n",
	//			(long)can_readl(intCntrVAddr + 4));
	//DEBUGMSG("Current Interrupt ID: %d\n",
	//			(int)(can_readl(intCntrVAddr + 0x90) & 0xF));
	//	iounmap( (void*)intCntrVAddr);
	iounmap( gpioVAddr );
	//	DEBUGMSG( "Unmapped Interrupt Controller IO-memory: 0x%lx\n",
	//	      (unsigned long)intCntrVAddr);
	DEBUGMSG( "Unmapped GPIO IO-memory: 0x%lx\n",
		(unsigned long)gpioVAddr);

	// Initialize chip data ( only one chip )
	//  pcandev->pchip[ 0 ]->powner = pcandev;
	/*pchip->ntype = CAN_CHIPTYPE_C_CAN;*/
	
	candev->nr_82527_chips=0;
	candev->nr_sja1000_chips=0;
	candev->nr_all_chips=NR_C_CAN;

	return 0;
}


#define CHIP_TYPE "c_can"
/**
 * hms30c7202_init_chip_data - Initialize chips
 * @candev: Pointer to candevice/board structure
 * @chipnr: Number of the CAN chip on the hardware card
 *
 * The function hms30c7202_init_chip_data() is used to initialize the hardware
 * structure containing information about the CAN chips.
 * %CHIP_TYPE represents the type of CAN chip.
 * The @chip_base_addr entry represents the start of the 'official' memory map
 * of the installed chip. It's likely that this is the same as the @io_addr
 * argument supplied at module loading time.
 * The @clock entry holds the chip clock value in Hz.
 * File: src/template.c
 */
int hms30c7202_init_chip_data(struct candevice_t *candev, int chipnr)
{
	// Register chip operations
	c_can_fill_chipspecops(candev->chip[chipnr]);
	/* override chip provided default value */
	candev->chip[chipnr]->max_objects = NR_MSGOBJ;

	candev->chip[chipnr]->chip_base_addr=candev->io_addr;
	
	candev->chip[chipnr]->clock = 16000000/2;
	
	/*candev->chip[chipnr]->int_clk_reg = 0x0;
	candev->chip[chipnr]->int_bus_reg = 0x0;
	candev->chip[chipnr]->sja_cdr_reg = 0x0;
	candev->chip[chipnr]->sja_ocr_reg = 0x0;*/
	

	return 0;
}


/**
 * hms30c7202_init_obj_data - Initialize message buffers
 * @chip: Pointer to chip specific structure
 * @objnr: Number of the message buffer
 *
 * The function hms30c7202_init_obj_data() is used to initialize the hardware
 * structure containing information about the different message objects on the
 * CAN chip. 
 * The entry @obj_base_addr represents the first memory address of the message 
 * object. 
 * Unless the hardware uses a segmented memory map, flags can be set zero.
 * Return Value: The function always returns zero
 * File: src/template.c
 */
int hms30c7202_init_obj_data(struct canchip_t *chip, int objnr)
{

	DEBUGMSG("(c%d)calling hms30c7202_init_obj_data( ...)\n", chip->chip_idx);

	/* It seems, that there is no purpose to setup object base address */
	chip->msgobj[objnr]->obj_base_addr=0;
	
	/*can_msgobj_test_fl(pmsgobj,RX_MODE_EXT);*/
	return 0;
}

/**
 * hms30c7202_write_register - Low level write register routine
 * @data: data to be written
 * @address: memory address to write to
 *
 * The function hms30c7202_write_register() is used to write to hardware registers
 * on the CAN chip. The registers are mapped on 32 bit bus on hms30c7202
 * and thus registers span is twice as one defined by C_CAN manual and defines.
 * This function compensates this difference.
 * Return Value: The function does not return a value
 * File: src/template.c
 */

void hms30c7202_write_register(unsigned data, can_ioptr_t address)
{
	unsigned long addr=can_ioptr2ulong(address);
	int i;
	//unsigned long usecs = 1;

	address = can_ulong2ioptr(((addr & C_CAN_REGOFFS_MASK) << 1) |
		                  (addr & ~C_CAN_REGOFFS_MASK));
	
	//DEBUGMSG("Trying to write 0x%u16x to address 0x%lx\n",data,address);
	
	can_writew(data,address);
	//udelay( usecs );
	for (i=0; i<5; i++);
}

/**
 * hms30c7202_read_register - Low level read register routine
 * @address: memory address to read from
 *
 * The function hms30c7202_read_register() is used to read from hardware registers
 * on the CAN chip. The registers are mapped on 32 bit bus on hms30c7202
 * and thus registers span is twice as one defined by C_CAN manual and defines.
 * This function compensates this difference.
 * Return Value: The function returns the value stored in @address
 * File: src/template.c
 */
unsigned hms30c7202_read_register(can_ioptr_t address)
{
	unsigned long addr=can_ioptr2ulong(address);
	u16 value, i;
	
	address = can_ulong2ioptr(((addr & C_CAN_REGOFFS_MASK) << 1) |
		                  (addr & ~C_CAN_REGOFFS_MASK));

	//DEBUGMSG("Trying to read from address 0x%lx :",address);
	
	value = can_readw(address);
	//udelay( usecs );
	for (i=0;i<5;i++);
	value = can_readw(address);
		//udelay( usecs );
	for (i=0;i<5;i++);
	
	//DEBUGMSG("0x%u16x\n",value);
	return value;

}

/**
 * hms30c7202_program_irq - program interrupts
 * @candev: Pointer to candevice/board structure
 *
 * The function hms30c7202_program_irq() is used for hardware that uses 
 * programmable interrupts. If your hardware doesn't use programmable interrupts
 * you should not set the @candevices_t->flags entry to %CANDEV_PROGRAMMABLE_IRQ and 
 * leave this function unedited. Again this function is hardware specific so 
 * there's no example code.
 * Return value: The function returns zero on success or %-ENODEV on failure
 * File: src/template.c
 */
int hms30c7202_program_irq(struct candevice_t *candev)
{
	return 0;
}

int hms30c7202_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = hms30c7202_request_io;
	hwspecops->release_io = hms30c7202_release_io;
	hwspecops->reset = hms30c7202_reset;
	hwspecops->init_hw_data = hms30c7202_init_hw_data;
	hwspecops->init_chip_data = hms30c7202_init_chip_data;
	hwspecops->init_obj_data = hms30c7202_init_obj_data;
	hwspecops->write_register = hms30c7202_write_register;
	hwspecops->read_register = hms30c7202_read_register;
	hwspecops->program_irq = hms30c7202_program_irq;
	return 0;
}

