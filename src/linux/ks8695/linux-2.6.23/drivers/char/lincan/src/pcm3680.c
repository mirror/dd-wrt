/* pcm3680.c
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
#include "../include/pcm3680.h"
#include "../include/sja1000p.h"

/*
 * IO_RANGE is the io-memory range that gets reserved, please adjust according
 * your hardware. Example: #define IO_RANGE 0x100 for i82527 chips or
 * #define IO_RANGE 0x20 for sja1000 chips in basic CAN mode.
 */
#define IO_RANGE 0x400

/**
 * template_request_io: - reserve io or memory range for can board
 * @candev: pointer to candevice/board which asks for io. Field @io_addr
 *	of @candev is used in most cases to define start of the range
 *
 * The function template_request_io() is used to reserve the io-memory. If your
 * hardware uses a dedicated memory range as hardware control registers you
 * will have to add the code to reserve this memory as well. 
 * %IO_RANGE is the io-memory range that gets reserved, please adjust according
 * your hardware. Example: #define IO_RANGE 0x100 for i82527 chips or
 * #define IO_RANGE 0x20 for sja1000 chips in basic CAN mode.
 * Return Value: The function returns zero on success or %-ENODEV on failure
 * File: src/template.c
 */
int pcm3680_request_io(struct candevice_t *candev)
{
        can_ioptr_t remap_addr;
	if (!can_request_mem_region(candev->io_addr,IO_RANGE,DEVICE_NAME " - pcm3680")) {
		CANMSG("Unable to request IO-memory: 0x%lx\n",candev->io_addr);
		return -ENODEV;
	}
	if ( !( remap_addr = ioremap( candev->io_addr, IO_RANGE ) ) ) {
		CANMSG("Unable to access I/O memory at: 0x%lx\n", candev->io_addr);
		can_release_mem_region(candev->io_addr,IO_RANGE);
		return -ENODEV;
	
	}
	can_base_addr_fixup(candev, remap_addr);
	DEBUGMSG("Registered IO-memory: 0x%lx - 0x%lx\n", candev->io_addr, candev->io_addr + IO_RANGE - 1);
	return 0;
}

/**
 * template_release_io - free reserved io memory range
 * @candev: pointer to candevice/board which releases io
 *
 * The function template_release_io() is used to free reserved io-memory.
 * In case you have reserved more io memory, don't forget to free it here.
 * IO_RANGE is the io-memory range that gets released, please adjust according
 * your hardware. Example: #define IO_RANGE 0x100 for i82527 chips or
 * #define IO_RANGE 0x20 for sja1000 chips in basic CAN mode.
 * Return Value: The function always returns zero
 * File: src/template.c
 */
int pcm3680_release_io(struct candevice_t *candev)
{
	iounmap((void*)candev->dev_base_addr);
	can_release_mem_region(candev->io_addr,IO_RANGE);
	return 0;
}

/**
 * template_reset - hardware reset routine
 * @candev: Pointer to candevice/board structure
 *
 * The function template_reset() is used to give a hardware reset. This is 
 * rather hardware specific so I haven't included example code. Don't forget to 
 * check the reset status of the chip before returning.
 * Return Value: The function returns zero on success or %-ENODEV on failure
 * File: src/template.c
 */
int pcm3680_reset(struct candevice_t *candev)
{
	int i=0;
	struct canchip_t *chip;
	int chipnr;
	
	DEBUGMSG("Resetting pcm3680 hardware ...\n");
	for(chipnr=0;chipnr<candev->nr_sja1000_chips;chipnr++) {
		chip=candev->chip[chipnr];
		pcm3680_write_register(sjaMOD_RM, chip->chip_base_addr+SJAMOD);
		udelay(1000);
		pcm3680_write_register(0x00, chip->chip_base_addr + SJAIER);
		/* Write arbitrary data to reset chip */
		pcm3680_write_register(0x01, chip->chip_base_addr + 0x100);
		udelay(1000);
		i=20;
		while (pcm3680_read_register(chip->chip_base_addr+SJAMOD)&sjaMOD_RM){
			if(!i--) return -ENODEV;
			udelay(1000);
			pcm3680_write_register(0, chip->chip_base_addr+SJAMOD);
		}
		udelay(1000);
		pcm3680_write_register(sjaCDR_PELICAN, chip->chip_base_addr+SJACDR);
		pcm3680_write_register(0x00, chip->chip_base_addr + SJAIER);
	}

	return 0;
}

#define RESET_ADDR 0x100
#define NR_82527 0
#define NR_SJA1000 2

/**
 * template_init_hw_data - Initialize hardware cards
 * @candev: Pointer to candevice/board structure
 *
 * The function template_init_hw_data() is used to initialize the hardware
 * structure containing information about the installed CAN-board.
 * %RESET_ADDR represents the io-address of the hardware reset register.
 * %NR_82527 represents the number of intel 82527 chips on the board.
 * %NR_SJA1000 represents the number of philips sja1000 chips on the board.
 * The flags entry can currently only be %CANDEV_PROGRAMMABLE_IRQ to indicate that
 * the hardware uses programmable interrupts.
 * Return Value: The function always returns zero
 * File: src/template.c
 */
int pcm3680_init_hw_data(struct candevice_t *candev) 
{
	candev->res_addr=RESET_ADDR;
	candev->nr_82527_chips=NR_82527;
	candev->nr_sja1000_chips=NR_SJA1000;
	candev->nr_all_chips=NR_82527+NR_SJA1000;
	candev->flags &= ~CANDEV_PROGRAMMABLE_IRQ;

	return 0;
}

/**
 * template_init_chip_data - Initialize chips
 * @candev: Pointer to candevice/board structure
 * @chipnr: Number of the CAN chip on the hardware card
 *
 * The function template_init_chip_data() is used to initialize the hardware
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
 * File: src/template.c
 */
int pcm3680_init_chip_data(struct candevice_t *candev, int chipnr)
{
	sja1000p_fill_chipspecops(candev->chip[chipnr]);
	candev->chip[chipnr]->chip_base_addr=
			candev->dev_base_addr + 0x200*chipnr;
	candev->chip[chipnr]->clock = 16000000;
	candev->chip[chipnr]->int_clk_reg = 0x0;
	candev->chip[chipnr]->int_bus_reg = 0x0;
	candev->chip[chipnr]->sja_cdr_reg = sjaCDR_CBP | sjaCDR_CLK_OFF;
	candev->chip[chipnr]->sja_ocr_reg = sjaOCR_MODE_NORMAL |
								sjaOCR_TX0_LH;

	return 0;
}

/**
 * template_init_obj_data - Initialize message buffers
 * @chip: Pointer to chip specific structure
 * @objnr: Number of the message buffer
 *
 * The function template_init_obj_data() is used to initialize the hardware
 * structure containing information about the different message objects on the
 * CAN chip. In case of the sja1000 there's only one message object but on the
 * i82527 chip there are 15.
 * The code below is for a i82527 chip and initializes the object base addresses
 * The entry @obj_base_addr represents the first memory address of the message 
 * object. In case of the sja1000 @obj_base_addr is taken the same as the chips
 * base address.
 * Unless the hardware uses a segmented memory map, flags can be set zero.
 * Return Value: The function always returns zero
 * File: src/template.c
 */
int pcm3680_init_obj_data(struct canchip_t *chip, int objnr)
{
	chip->msgobj[objnr]->obj_base_addr=chip->chip_base_addr;
	
	return 0;
}

/**
 * template_program_irq - program interrupts
 * @candev: Pointer to candevice/board structure
 *
 * The function template_program_irq() is used for hardware that uses 
 * programmable interrupts. If your hardware doesn't use programmable interrupts
 * you should not set the @candevices_t->flags entry to %CANDEV_PROGRAMMABLE_IRQ and 
 * leave this function unedited. Again this function is hardware specific so 
 * there's no example code.
 * Return value: The function returns zero on success or %-ENODEV on failure
 * File: src/template.c
 */
int pcm3680_program_irq(struct candevice_t *candev)
{
	return 0;
}

/**
 * template_write_register - Low level write register routine
 * @data: data to be written
 * @address: memory address to write to
 *
 * The function template_write_register() is used to write to hardware registers
 * on the CAN chip. You should only have to edit this function if your hardware
 * uses some specific write process.
 * Return Value: The function does not return a value
 * File: src/template.c
 */
void pcm3680_write_register(unsigned data, can_ioptr_t address)
{
	can_writeb(data,address);
}

/**
 * template_read_register - Low level read register routine
 * @address: memory address to read from
 *
 * The function template_read_register() is used to read from hardware registers
 * on the CAN chip. You should only have to edit this function if your hardware
 * uses some specific read process.
 * Return Value: The function returns the value stored in @address
 * File: src/template.c
 */
unsigned pcm3680_read_register(can_ioptr_t address)
{
	return can_readb(address);
}

/* !!! Don't change this function !!! */
int pcm3680_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = pcm3680_request_io;
	hwspecops->release_io = pcm3680_release_io;
	hwspecops->reset = pcm3680_reset;
	hwspecops->init_hw_data = pcm3680_init_hw_data;
	hwspecops->init_chip_data = pcm3680_init_chip_data;
	hwspecops->init_obj_data = pcm3680_init_obj_data;
	hwspecops->write_register = pcm3680_write_register;
	hwspecops->read_register = pcm3680_read_register;
	hwspecops->program_irq = pcm3680_program_irq;
	return 0;
}
