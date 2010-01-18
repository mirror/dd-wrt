/* ns_dev_can.c - FPGA version of C_CAN ARM device specific code
 * Linux CAN-bus device driver.
 * Written by Sebastian Stolzenberg email:stolzi@sebastian-stolzenberg.de
 * Based on code from Arnaud Westenberg email:arnaud@wanadoo.nl
 * and Ake Hedman, eurosource, akhe@eurosource.se
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * Ported to FS Forth-Systeme GmbH A9M9750DEVx development boards
 * email:nbryan@embebidos.com
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 * This port 19 May 2005
 *
 */

#include <linux/delay.h>

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/c_can.h"
#include "../include/ns_dev_can.h"

/*
 * IO range for the C_CAN 1.2 memory map is 0x100 (256bytes)
 */
#define IO_RANGE 0x100

/**
 * ns_dev_request_io: - reserve io or memory range for can board
 * @candev: pointer to candevice/board which asks for io. Field @io_addr
 *	of @candev is used in most cases to define start of the range
 *
 */
int ns_dev_request_io(struct candevice_t *candev)
{
	/* Note hard-coded index for the chip number as this 
	 * only supports a single instance of the C_CAN controller.
	 */
	DEBUGMSG("(c%d)ns_dev_request_io (...)\n", candev->chip[0]->chip_idx);

	if (!can_request_mem_region(candev->io_addr, IO_RANGE, DEVICE_NAME)) {
		CANMSG("ns_dev failed to  mem region %lx.\n",
		       (unsigned long)candev->io_addr);
	}

	if (!(candev->dev_base_addr = ioremap(candev->io_addr, IO_RANGE))) {
		DEBUGMSG
		    ("Failed to map IO-memory: 0x%lx - 0x%lx, mapped to 0x%lx\n",
		     (unsigned long)candev->io_addr,
		     (unsigned long)candev->io_addr + IO_RANGE - 1,
		     (unsigned long)candev->dev_base_addr);
		can_release_mem_region(candev->io_addr, IO_RANGE);
		return -ENODEV;
	} else {

		DEBUGMSG("Mapped IO-memory: 0x%lx - 0x%lx, mapped to 0x%lx\n",
			 (unsigned long)candev->io_addr,
			 (unsigned long)candev->io_addr + IO_RANGE - 1,
			 (unsigned long)candev->dev_base_addr);

	}

	candev->chip[0]->chip_base_addr = candev->dev_base_addr;
	candev->chip[0]->chipspecops->start_chip(candev->chip[0]);

	return 0;
}

/**
 * ns_dev_release_io - free reserved io memory range
 * @candev: pointer to candevice/board which releases io
 *
 * The function ns_dev_release_io() is used to free reserved io-memory.
 * In case you have reserved more io memory, don't forget to free it here.
 *
 */
int ns_dev_release_io(struct candevice_t *candev)
{
	DEBUGMSG("(c%d)ns_dev_release_io (...)\n", candev->chip[0]->chip_idx);

	/* Release I/O memory mapping */
	iounmap(candev->dev_base_addr);

	/* Release the memory region */
	can_release_mem_region(candev->io_addr, IO_RANGE);

	return 0;
}

/**
 * ns_dev_reset - hardware reset routine
 * @card: Number of the hardware card.
 *
 * The function ns_dev_reset() is used to give a hardware reset. This is
 * rather hardware specific so I haven't included example code. Don't forget to
 * check the reset status of the chip before returning.
 * Return Value: The function returns zero on success or %-ENODEV on failure
 *
 */
int ns_dev_reset(struct candevice_t *candev)
{
	int i = 0;
	int enableTest = 0;
	int disableTest = 0;

	struct canchip_t *pchip = candev->chip[0];

	enableTest = pchip->chipspecops->enable_configuration(pchip);
	disableTest = pchip->chipspecops->disable_configuration(pchip);

	if (enableTest || disableTest) {
		CANMSG("Enable or Disable status failed!\n");
		CANMSG("Please check your hardware.\n");
		return -ENODEV;
	}

	/* Check busoff status */
	while ((c_can_read_reg_w(pchip, CCSR) & SR_BOFF) && (i <= 15)) {
		udelay(2000);
		i++;
	}

	if (i >= 15) {
		CANMSG("Reset status timeout!\n");
		CANMSG("Please check your hardware.\n");
		return -ENODEV;
	}
	//pchip->config_irqs(pchip, CR_MIE | CR_SIE | CR_EIE);
	return 0;
}

#define RESET_ADDR 0x0
#define NR_C_CAN 1
#define NR_MSGOBJ 32

/**
 * ns_dev_hw_data - Initialize hardware cards
 * @candev: Pointer to candevice/board structure
 *
 * The function ns_dev_init_hw_data() is used to initialize the hardware
 * structure containing information about the installed CAN-board.
 * The flags entry can currently only be %CANDEV_PROGRAMMABLE_IRQ to indicate that
 * the hardware uses programmable interrupts.
 * Return Value: The function always returns zero
 */
int ns_dev_init_hw_data(struct candevice_t *candev)
{
	can_ioptr_t sys_contVA = NULL;

	/* LUCAN : Magic numbers */
	if (!(sys_contVA = ioremap(NS9750_PERIPHERAL_BASE_ADDRESS,
					 NS9750_PERIPHERAL_MAP_SIZE))) {
		DEBUGMSG("Failed to map FPGA memory\n");
		return -EIO;
	} else {
		DEBUGMSG("Writing to NS9750 sys cont\n");
		can_writel((BUS_WIDTH_16BIT | ACTIVE_LOW_CHIP_SELECT),
		       sys_contVA + NS9750_SYSTEM_CONTROLLER_OFFSET);
	}

	/* We have finished with this mapping */
	iounmap(sys_contVA);

	candev->nr_82527_chips = 0;
	candev->nr_sja1000_chips = 0;
	candev->nr_all_chips = NR_C_CAN;
	candev->flags |= CANDEV_PROGRAMMABLE_IRQ;

	return 0;
}

/**
 * ns_dev_init_chip_data - Initialize chips
 * @candev: Pointer to candevice/board structure
 * @chipnr: Number of the CAN chip on the hardware card
 *
 * The function ns_dev_init_chip_data() is used to initialize the hardware
 * structure containing information about the CAN chips.
 * %CHIP_TYPE represents the type of CAN chip.
 * The @chip_base_addr entry represents the start of the 'official' memory map
 * of the installed chip. It's likely that this is the same as the @io_addr
 * argument supplied at module loading time.
 * The @clock entry holds the chip clock value in Hz.
 * File: src/template.c
 */
int ns_dev_init_chip_data(struct candevice_t *candev, int chipnr)
{
	/* Register chip operations */
	c_can_fill_chipspecops(candev->chip[chipnr]);

	/* override chip provided default value */
	candev->chip[chipnr]->max_objects = MAX_MSGOBJS;
	candev->chip[chipnr]->chip_base_addr = candev->io_addr;
	candev->chip[chipnr]->clock = C_CAN_CLOCK_INPUT_FREQUENCY;

	return 0;
}

/**
 * ns_dev_init_obj_data - Initialize message buffers
 * @chip: Pointer to chip specific structure
 * @objnr: Number of the message buffer
 *
 * The function ns_dev_init_obj_data() is used to initialize the hardware
 * structure containing information about the different message objects on the
 * CAN chip. 
 * The entry @obj_base_addr represents the first memory address of the message 
 * object. 
 * Unless the hardware uses a segmented memory map, flags can be set zero.
 * Return Value: The function always returns zero
 * File: src/template.c
 */
int ns_dev_init_obj_data(struct canchip_t *chip, int objnr)
{

	DEBUGMSG("(c%d)calling ns_dev_init_obj_data( ...)\n", chip->chip_idx);

	/* It seems, that there is no purpose to setup object base address */
	chip->msgobj[objnr]->obj_base_addr = 0;

	/*can_msgobj_test_fl(pmsgobj,RX_MODE_EXT); */
	return 0;
}

/**
 * ns_dev_write_register - Low level write register routine
 * @data: data to be written
 * @address: memory address to write to
 *
 * The function ns_dev_write_register() is used to write to hardware registers
 * on the CAN chip. You should only have to edit this function if your hardware
 * uses some specific write process.
 * Return Value: The function does not return a value
 * File: src/template.c
 */
void ns_dev_write_register(unsigned data, can_ioptr_t address)
{
	int i;
	//unsigned long usecs = 1;

	can_writew(data, address);
	//udelay( usecs );
	for (i = 0; i < 5; i++) ;
}

/**
 * ns_dev_read_register - Low level read register routine
 * @address: memory address to read from
 *
 * The function ns_dev_read_register() is used to read from hardware registers
 * on the CAN chip. You should only have to edit this function if your hardware
 * uses some specific read process.
 * Return Value: The function returns the value stored in @address
 * File: src/template.c
 */
unsigned ns_dev_read_register(can_ioptr_t address)
{
	u16 value, i;

	value = can_readw(address);
	//udelay( usecs );
	for (i = 0; i < 5; i++) ;
	value = can_readw(address);
	//udelay( usecs );
	for (i = 0; i < 5; i++) ;

	return value;
}

/**
 * ns_dev_program_irq - program interrupts
 * @candev: Pointer to candevice/board structure
 *
 * The function ns_dev_program_irq() is used for hardware that uses 
 * programmable interrupts. If your hardware doesn't use programmable interrupts
 * you should not set the @candevices_t->flags entry to %CANDEV_PROGRAMMABLE_IRQ and 
 * leave this function unedited. Again this function is hardware specific so 
 * there's no example code.
 * Return value: The function returns zero on success or %-ENODEV on failure
 * File: src/template.c
 */
int ns_dev_program_irq(struct candevice_t *candev)
{
	return 0;
}

int ns_dev_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = ns_dev_request_io;
	hwspecops->release_io = ns_dev_release_io;
	hwspecops->reset = ns_dev_reset;
	hwspecops->init_hw_data = ns_dev_init_hw_data;
	hwspecops->init_chip_data = ns_dev_init_chip_data;
	hwspecops->init_obj_data = ns_dev_init_obj_data;
	hwspecops->write_register = ns_dev_write_register;
	hwspecops->read_register = ns_dev_read_register;
	hwspecops->program_irq = ns_dev_program_irq;
	return 0;
}
