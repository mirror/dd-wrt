/* ssv.c
 * Linux CAN-bus device driver.
 * Written by Arnaud Westenberg email:arnaud@casema.net
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */ 

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/ssv.h"
#include "../include/i82527.h"

int ssvcan_irq[2]={-1,-1};
can_ioptr_t ssvcan_base=0x0;

static CAN_DEFINE_SPINLOCK(ssv_port_lock);

/* IO_RANGE is the io-memory range that gets reserved, please adjust according
 * your hardware. Example: #define IO_RANGE 0x100 for i82527 chips or
 * #define IO_RANGE 0x20 for sja1000 chips.
 */
#define IO_RANGE 0x04

/* The function template_request_io is used to reserve the io-memory. If your
 * hardware uses a dedicated memory range as hardware control registers you
 * will have to add the code to reserve this memory as well.
 * The reserved memory starts at io_addr, wich is the module parameter io.
 */
int ssv_request_io(struct candevice_t *candev)
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
int ssv_release_io(struct candevice_t *candev)
{

	can_release_io_region(candev->io_addr,IO_RANGE);

	return 0;
}

/* The function template_reset is used to give a hardware reset. This is rather
 * hardware specific so I haven't included example code. Don't forget to check
 * the reset status of the chip before returning.
 */
int ssv_reset(struct candevice_t *candev)
{
    int i; 

    DEBUGMSG("Resetting ssv hardware ...\n");
    ssv_write_register(1,ssvcan_base+iCPU);
    ssv_write_register(0,ssvcan_base+iCPU);
    ssv_write_register(1,ssvcan_base+0x100+iCPU);
    ssv_write_register(0,ssvcan_base+0x100+iCPU);

    for (i = 1; i < 1000; i++)
	udelay (1000);

    /* Check hardware reset status */ 
    i=0;
    while ( (ssv_read_register(ssvcan_base+iCPU) & iCPU_RST) && (i<=15)) {
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

    /* Check hardware reset status */ 
    i=0;
    while ( (ssv_read_register(ssvcan_base+0x100+iCPU) & iCPU_RST) && (i<=15)) {
	udelay(20000);
	i++;
    }
    if (i>=15) {
	CANMSG("Reset status timeout!\n");
	CANMSG("Please check your hardware.\n");
	return -ENODEV;
    }
    else
	DEBUGMSG("Chip1 reset status ok.\n");



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
#define RESET_ADDR 0x02
#define NR_82527 2
#define NR_SJA1000 0

int ssv_init_hw_data(struct candevice_t *candev) 
{
    candev->res_addr=RESET_ADDR;
    candev->nr_82527_chips=NR_82527;
    candev->nr_sja1000_chips=0;
    candev->nr_all_chips=NR_82527;
    candev->flags |= CANDEV_PROGRAMMABLE_IRQ;

    return 0;
}

/* The function template_init_chip_data is used to initialize the hardware
 * structure containing information about the CAN chips.
 * CHIP_TYPE represents the type of CAN chip. CHIP_TYPE can be "i82527" or
 * "sja1000".
 * The chip_base_addr entry represents the start of the 'official' memory map
 * of the installed chip. It's likely that this is the same as the io_addr
 * argument supplied at module loading time.
 * The clock argument holds the chip clock value in Hz.
 */
int ssv_init_chip_data(struct candevice_t *candev, int chipnr)
{
    i82527_fill_chipspecops(candev->chip[chipnr]);
    candev->chip[chipnr]->chip_base_addr=
	can_ioport2ioptr(candev->io_addr+0x100*chipnr);
    candev->chip[chipnr]->clock = 16000000;
    ssvcan_irq[chipnr]=candev->chip[chipnr]->chip_irq;

    ssvcan_base=candev->io_addr;

    candev->chip[chipnr]->int_cpu_reg = iCPU_DSC;
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
int ssv_init_obj_data(struct canchip_t *chip, int objnr)
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
int ssv_program_irq(struct candevice_t *candev)
{
    return 0;
}

/* The function template_write_register is used to write to hardware registers
 * on the CAN chip. You should only have to edit this function if your hardware
 * uses some specific write process.
 */
void ssv_write_register(unsigned data, can_ioptr_t address)
{
    /* address is an absolute address */

    /* the ssv card has two registers, the address register at 0x0
       and the data register at 0x01 */

    /* write the relative address on the eight LSB bits 
     and the data on the eight MSB bits in one time */
    if((address-ssvcan_base)<0x100)
	can_outw(address-ssvcan_base + (256 * data), ssvcan_base);
    else
	can_outw(address-ssvcan_base-0x100 + (256 * data), ssvcan_base+0x02);
}

/* The function template_read_register is used to read from hardware registers
 * on the CAN chip. You should only have to edit this function if your hardware
 * uses some specific read process.
 */
unsigned ssv_read_register(can_ioptr_t address)
{
    /* this is the same thing that the function write_register.
       We use the two register, we write the address where we 
       want to read in a first time. In a second time we read the
       data */
    unsigned char ret;
    can_spin_irqflags_t flags;
    

    if((address-ssvcan_base)<0x100)
    {
	can_spin_lock_irqsave(&ssv_port_lock,flags);
	can_outb(address-ssvcan_base, ssvcan_base);
	ret=can_inb(ssvcan_base+1);
	can_spin_unlock_irqrestore(&ssv_port_lock,flags);
    }
    else
    {
	can_spin_lock_irqsave(&ssv_port_lock,flags);
	can_outb(address-ssvcan_base-0x100, ssvcan_base+0x02);
	ret=can_inb(ssvcan_base+1+0x02);
	can_spin_unlock_irqrestore(&ssv_port_lock,flags);
    }

    return ret;
}


 /* !!! Don't change this function !!! */
int ssv_register(struct hwspecops_t *hwspecops)
{
    hwspecops->request_io = ssv_request_io;
    hwspecops->release_io = ssv_release_io;
    hwspecops->reset = ssv_reset;
    hwspecops->init_hw_data = ssv_init_hw_data;
    hwspecops->init_chip_data = ssv_init_chip_data;
    hwspecops->init_obj_data = ssv_init_obj_data;
    hwspecops->write_register = ssv_write_register;
    hwspecops->read_register = ssv_read_register;
    hwspecops->program_irq = ssv_program_irq;
    return 0;
}
