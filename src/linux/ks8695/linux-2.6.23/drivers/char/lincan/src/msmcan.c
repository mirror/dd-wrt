/* msmcan.c
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
#include "../include/msmcan.h"
#include "../include/i82527.h"

static CAN_DEFINE_SPINLOCK(msmcan_port_lock);

/* IO_RANGE is the io-memory range that gets reserved, please adjust according
 * your hardware. Example: #define IO_RANGE 0x100 for i82527 chips or
 * #define IO_RANGE 0x20 for sja1000 chips.
 */
#define IO_RANGE 0x04

/* The function template_request_io is used to reserve the io-memory. If your
 * hardware uses a dedicated memory range as hardware control registers you
 * will have to add the code to reserve this memory as well.
 * The reserved memory starts at candev->io_addr, wich is the module parameter io.
 */
int msmcan_request_io(struct candevice_t *candev)
{

	if (!can_request_io_region(candev->io_addr,IO_RANGE,DEVICE_NAME)) {
		CANMSG("Unable to open port: 0x%lx\n",candev->io_addr);
		return -ENODEV;
	} else {
		DEBUGMSG("Registered IO-memory: 0x%lx - 0x%lx\n", candev->io_addr, 
			 candev->io_addr + IO_RANGE - 1);
	}
	return 0;
}

/* The function template_release_io is used to free the previously reserved 
 * io-memory. In case you reserved more memory, don't forget to free it here.
 */
int msmcan_release_io(struct candevice_t *candev)
{

	can_release_io_region(candev->io_addr,IO_RANGE);

	return 0;
}

/* The function template_reset is used to give a hardware reset. This is rather
 * hardware specific so I haven't included example code. Don't forget to check
 * the reset status of the chip before returning.
 */
int msmcan_reset(struct candevice_t *candev)
{
	struct canchip_t *chip=candev->chip[0];

	DEBUGMSG("Resetting msmcan hardware ...\n");
	/* we don't use template_write_register because we don't use the two first
	   registers of the card but the third in order to make a hard reset */
	/* can_outb (1, msmcan_base + candev->res_addr); */


	/* terrible MSMCAN reset design - best to comment out */
	if(0) {
		int tic=jiffies;
		int tac;
		
		msmcan_write_register(iCTL_INI, chip->chip_base_addr+iCTL);
		/*CLKOUT stopped (iCPU_CEN=0) */
		msmcan_write_register(iCPU_DSC, chip->chip_base_addr+iCPU);
		while(!(msmcan_read_register(chip->chip_base_addr+iCPU)&iCPU_CEN)){
			tac=jiffies;
			if((tac-tic)>HZ*2){
				CANMSG("Unable to reset board\n");
				return -EIO;
			}
			schedule();
		}
		
	
	}

	can_disable_irq(chip->chip_irq);
	msmcan_write_register(iCTL_INI, chip->chip_base_addr+iCTL);
	can_enable_irq(chip->chip_irq);

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
#define NR_82527 1
#define NR_SJA1000 0

int msmcan_init_hw_data(struct candevice_t *candev) 
{
	candev->res_addr=0;
	candev->nr_82527_chips=1;
	candev->nr_sja1000_chips=0;
        candev->nr_all_chips=1;
	candev->flags |= CANDEV_PROGRAMMABLE_IRQ*0;

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

int msmcan_init_chip_data(struct candevice_t *candev, int chipnr)
{
	i82527_fill_chipspecops(candev->chip[chipnr]);
	/* device uses indexed access */
	candev->chip[chipnr]->chip_base_addr=
	    can_ioport2ioptr(candev->io_addr << 16);
	candev->chip[chipnr]->clock = 16000000;
	/* The CLKOUT has to be enabled to reset MSMCAN MAX1232 watchdog */
	candev->chip[chipnr]->int_cpu_reg = iCPU_DSC | iCPU_CEN;
	candev->chip[chipnr]->int_clk_reg = iCLK_SL1;
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
int msmcan_init_obj_data(struct canchip_t *chip, int objnr)
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
int msmcan_program_irq(struct candevice_t *candev)
{
	return 0;
}

/* The function template_write_register is used to write to hardware registers
 * on the CAN chip. You should only have to edit this function if your hardware
 * uses some specific write process.
 */
void msmcan_write_register(unsigned data, can_ioptr_t address)
{
	/* address is combination of base address shifted left by 16 and index */
	can_spin_irqflags_t flags;
	unsigned long addr=can_ioptr2ulong(address);

	/* the msmcan card has two registers, the data register at 0x0
	   and the address register at 0x01 */

	can_spin_lock_irqsave(&msmcan_port_lock,flags);
	can_outb(addr & 0xff, (addr>>16)+1);
	can_outb(data, addr>>16); 
	can_spin_unlock_irqrestore(&msmcan_port_lock,flags);
}

/* The function template_read_register is used to read from hardware registers
 * on the CAN chip. You should only have to edit this function if your hardware
 * uses some specific read process.
 */
unsigned msmcan_read_register(can_ioptr_t address)
{
	/* this is the same thing that the function write_register.
	   We use the two register, we write the address where we 
	   want to read in a first time. In a second time we read the
	   data */
	unsigned char ret;
	can_spin_irqflags_t flags;
	unsigned long addr=can_ioptr2ulong(address);

	can_spin_lock_irqsave(&msmcan_port_lock,flags);
	can_outb(addr & 0xff, (addr>>16)+1);
	ret=can_inb(addr>>16); 
	can_spin_unlock_irqrestore(&msmcan_port_lock,flags);
	return ret;
}


 /* !!! Don't change this function !!! */
int msmcan_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = msmcan_request_io;
	hwspecops->release_io = msmcan_release_io;
	hwspecops->reset = msmcan_reset;
	hwspecops->init_hw_data = msmcan_init_hw_data;
	hwspecops->init_chip_data = msmcan_init_chip_data;
	hwspecops->init_obj_data = msmcan_init_obj_data;
	hwspecops->write_register = msmcan_write_register;
	hwspecops->read_register = msmcan_read_register;
	hwspecops->program_irq = msmcan_program_irq;
	return 0;
}
