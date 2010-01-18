/* pc-i03.c
 * Linux CAN-bus device driver.
 * Written by Arnaud Westenberg email:arnaud@wnadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */ 

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/pc-i03.h"
#include "../include/sja1000.h"

/* Basic hardware io address. This is also stored in the hardware structure but
 * we need it global, else we have to change many internal functions.
 * pc-i03_base_addr is initialized in pc-i03_init_chip_data().
 */
unsigned int pci03_base_addr; 

/*
 * IO_RANGE is the io-memory range that gets reserved, please adjust according
 * your hardware. Example: #define IO_RANGE 0x100 for i82527 chips or
 * #define IO_RANGE 0x20 for sja1000 chips in basic CAN mode.
 */
#define IO_RANGE 0x200	// The pc-i03 uses an additional 0x100 bytes reset space

/**
 * pci03_request_io: - reserve io or memory range for can board
 * @candev: pointer to candevice/board which asks for io. Field @io_addr
 *	of @candev is used in most cases to define start of the range
 *
 * The function pci03_request_io() is used to reserve the io-memory. If your
 * hardware uses a dedicated memory range as hardware control registers you
 * will have to add the code to reserve this memory as well. 
 * %IO_RANGE is the io-memory range that gets reserved, please adjust according
 * your hardware. Example: #define IO_RANGE 0x100 for i82527 chips or
 * #define IO_RANGE 0x20 for sja1000 chips in basic CAN mode.
 * Return Value: The function returns zero on success or %-ENODEV on failure
 * File: src/pc-i03.c
 */
int pci03_request_io(struct candevice_t *candev)
{
	if (!can_request_io_region(candev->io_addr,IO_RANGE,DEVICE_NAME)) {
		CANMSG("Unable to open port: 0x%lx\n",candev->io_addr);
		return -ENODEV;
	} else {
		DEBUGMSG("Registered IO-memory: 0x%lx - 0x%lx\n", candev->io_addr, candev->io_addr + IO_RANGE - 1);
	}
	return 0;
}

/**
 * pci03_elease_io - free reserved io memory range
 * @candev: pointer to candevice/board which releases io
 *
 * The function pci03_release_io() is used to free reserved io-memory.
 * In case you have reserved more io memory, don't forget to free it here.
 * IO_RANGE is the io-memory range that gets released, please adjust according
 * your hardware. Example: #define IO_RANGE 0x100 for i82527 chips or
 * #define IO_RANGE 0x20 for sja1000 chips in basic CAN mode.
 * Return Value: The function always returns zero
 * File: src/pc-i03.c
 */
int pci03_release_io(struct candevice_t *candev)
{
	can_release_io_region(candev->io_addr,IO_RANGE);

	return 0;
}

/**
 * pci03_reset - hardware reset routine
 * @candev: Pointer to candevice/board structure
 *
 * The function pci03_reset() is used to give a hardware reset. This is 
 * rather hardware specific so I haven't included example code. Don't forget to 
 * check the reset status of the chip before returning.
 * Return Value: The function returns zero on success or %-ENODEV on failure
 * File: src/pc-i03.c
 */
int pci03_reset(struct candevice_t *candev)
{
	int i=0;

	DEBUGMSG("Resetting pc-i03 hardware ...\n");
	pci03_write_register(0x01,pci03_base_addr +
				0x100); // Write arbitrary data to reset mem
	udelay(20000);

	pci03_write_register(0x00, pci03_base_addr + SJACR);
									
	/* Check hardware reset status */
	i=0;
	while ( (pci03_read_register(pci03_base_addr + SJACR) & sjaCR_RR)
								 && (i<=15) ) {
		udelay(20000);
		i++;
	}
	if (i>=15) {
		CANMSG("Reset status timeout!\n");
		CANMSG("Please check your hardware.\n");
		return -ENODEV;
	}
	else
		DEBUGMSG("Chip[0] reset status ok.\n");

	return 0;
}

#define RESET_ADDR 0x100
#define NR_82527 0
#define NR_SJA1000 1

/**
 * pci03_init_hw_data - Initialize hardware cards
 * @candev: Pointer to candevice/board structure
 *
 * The function pci03_init_hw_data() is used to initialize the hardware
 * structure containing information about the installed CAN-board.
 * %RESET_ADDR represents the io-address of the hardware reset register.
 * %NR_82527 represents the number of intel 82527 chips on the board.
 * %NR_SJA1000 represents the number of philips sja1000 chips on the board.
 * The flags entry can currently only be %CANDEV_PROGRAMMABLE_IRQ to indicate that
 * the hardware uses programmable interrupts.
 * Return Value: The function always returns zero
 * File: src/pc-i03.c
 */
int pci03_init_hw_data(struct candevice_t *candev) 
{
	candev->res_addr=RESET_ADDR;
	candev->nr_82527_chips=NR_82527;
        candev->nr_sja1000_chips=NR_SJA1000;
	candev->nr_all_chips=NR_82527+NR_SJA1000;
	return 0;
}

/**
 * pci03_init_chip_data - Initialize chips
 * @candev: Pointer to candevice/board structure
 * @chipnr: Number of the CAN chip on the hardware card
 *
 * The function pci03_init_chip_data() is used to initialize the hardware
 * structure containing information about the CAN chips.
 * %CHIP_TYPE represents the type of CAN chip. %CHIP_TYPE can be "i82527" or
 * "sja1000".
 * The @chip_base_addr entry represents the start of the 'official' memory map
 * of the installed chip. It's likely that this is the same as the @io_addr
 * argument supplied at module loading time.
 * The @clock entry holds the chip clock value in Hz.
 * The entry @sja_cdr_reg holds hardware specific options for the Clock Divider
 * register. Options defined in the %sja1000.h file:
 * %sjaCDR_CLKOUT_MASK, %sjaCDR_CLK_OFF, %sjaCDR_RXINPEN, %sjaCDR_CBP, %sjaCDR_PELICAN
 * The entry @sja_ocr_reg holds hardware specific options for the Output Control
 * register. Options defined in the %sja1000.h file:
 * %sjaOCR_MODE_BIPHASE, %sjaOCR_MODE_TEST, %sjaOCR_MODE_NORMAL, %sjaOCR_MODE_CLOCK,
 * %sjaOCR_TX0_LH, %sjaOCR_TX1_ZZ.
 * The entry @int_clk_reg holds hardware specific options for the Clock Out
 * register. Options defined in the %i82527.h file:
 * %iCLK_CD0, %iCLK_CD1, %iCLK_CD2, %iCLK_CD3, %iCLK_SL0, %iCLK_SL1.
 * The entry @int_bus_reg holds hardware specific options for the Bus 
 * Configuration register. Options defined in the %i82527.h file:
 * %iBUS_DR0, %iBUS_DR1, %iBUS_DT1, %iBUS_POL, %iBUS_CBY.
 * Return Value: The function always returns zero
 * File: src/pc-i03.c
 */
int pci03_init_chip_data(struct candevice_t *candev, int chipnr)
{
	sja1000_fill_chipspecops(candev->chip[chipnr]);
	pci03_base_addr = candev->io_addr;
	candev->chip[chipnr]->chip_base_addr=candev->io_addr;
	candev->chip[chipnr]->clock = 16000000;
	candev->chip[chipnr]->sja_cdr_reg = sjaCDR_CBP | sjaCDR_CLK_OFF;
	candev->chip[chipnr]->sja_ocr_reg = sjaOCR_MODE_NORMAL | 
							sjaOCR_TX0_HL | sjaOCR_TX1_LZ;

	return 0;
}

/**
 * pci03_init_obj_data - Initialize message buffers
 * @chip: Pointer to chip specific structure
 * @objnr: Number of the message buffer
 *
 * The function pci03_init_obj_data() is used to initialize the hardware
 * structure containing information about the different message objects on the
 * CAN chip. In case of the sja1000 there's only one message object but on the
 * i82527 chip there are 15.
 * The code below is for a i82527 chip and initializes the object base addresses
 * The entry @obj_base_addr represents the first memory address of the message 
 * object. In case of the sja1000 @obj_base_addr is taken the same as the chips
 * base address.
 * Unless the hardware uses a segmented memory map, flags can be set zero.
 * Return Value: The function always returns zero
 * File: src/pc-i03.c
 */
int pci03_init_obj_data(struct canchip_t *chip, int objnr)
{
	chip->msgobj[objnr]->obj_base_addr=chip->chip_base_addr;
	
	return 0;
}

/**
 * pci03_program_irq - program interrupts
 * @candev: Pointer to candevice/board structure
 *
 * The function pci03_program_irq() is used for hardware that uses 
 * programmable interrupts. If your hardware doesn't use programmable interrupts
 * you should not set the @candevices_t->flags entry to %CANDEV_PROGRAMMABLE_IRQ and 
 * leave this function unedited. Again this function is hardware specific so 
 * there's no example code.
 * Return value: The function returns zero on success or %-ENODEV on failure
 * File: src/pc-i03.c
 */
int pci03_program_irq(struct candevice_t *candev)
{
	return 0;
}

/**
 * pci03_write_register - Low level write register routine
 * @data: data to be written
 * @address: memory address to write to
 *
 * The function pci03_write_register() is used to write to hardware registers
 * on the CAN chip. You should only have to edit this function if your hardware
 * uses some specific write process.
 * Return Value: The function does not return a value
 * File: src/pc-i03.c
 */
void pci03_write_register(unsigned data, can_ioptr_t address)
{
	unsigned int *pci03_base_ptr;
	unsigned short address_to_write;

	/* The read/write functions are called by an extra abstract function.
	 * This extra function adds the basic io address of the card to the
	 * memory address we want to write to, so we substract the basic io
	 * address again to obtain the offset into the hardware's memory map.
	 */
	address_to_write = address - pci03_base_addr; // Offset
	pci03_base_ptr = (unsigned int *)(pci03_base_addr * 0x100001);
	(*(pci03_base_ptr+address_to_write)) = data;
}

/**
 * pci03_read_register - Low level read register routine
 * @address: memory address to read from
 *
 * The function pci03_read_register() is used to read from hardware registers
 * on the CAN chip. You should only have to edit this function if your hardware
 * uses some specific read process.
 * Return Value: The function returns the value stored in @address
 * File: src/pc-i03.c
 */
unsigned pci03_read_register(can_ioptr_t address)
{
	unsigned int *pci03_base_ptr;
	unsigned short address_to_read;

	/* The read/write functions are called by an extra abstract function.
	 * This extra function adds the basic io address of the card to the
	 * memory address we want to write to, so we substract the basic io
	 * address again to obtain the offset into the hardware's memory map.
	 */
	address_to_read = address - pci03_base_addr;
	pci03_base_ptr = (unsigned int *)(pci03_base_addr * 0x100001);
	return (*(pci03_base_ptr+address_to_read));
}

int pci03_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = pci03_request_io;
	hwspecops->release_io = pci03_release_io;
	hwspecops->reset = pci03_reset;
	hwspecops->init_hw_data = pci03_init_hw_data;
	hwspecops->init_chip_data = pci03_init_chip_data;
	hwspecops->init_obj_data = pci03_init_obj_data;
	hwspecops->write_register = pci03_write_register;
	hwspecops->read_register = pci03_read_register;
	hwspecops->program_irq = pci03_program_irq;
	return 0;
}
