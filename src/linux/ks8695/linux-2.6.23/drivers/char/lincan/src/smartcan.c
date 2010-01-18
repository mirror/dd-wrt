/* smartcan.c
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
#include "../include/smartcan.h"
#include "../include/i82527.h"

int smartcan_irq=-1;
unsigned long smartcan_base=0x0;

static CAN_DEFINE_SPINLOCK(smartcan_port_lock);

int smartcan_request_io(struct candevice_t *candev)
{
	if (!can_request_io_region(candev->io_addr,0x04,DEVICE_NAME)) {
		CANMSG("Unable to open port: 0x%lx\n",candev->io_addr);
		return -ENODEV;
	}else {
		DEBUGMSG("Registered IO-memory: 0x%lx - 0x%lx\n", candev->io_addr, candev->io_addr + 0x04 - 1);
	}
	return 0;
}

int smartcan_release_io(struct candevice_t *candev)
{
	can_release_io_region(candev->io_addr,0x04);

	return 0;
}

int smartcan_reset(struct candevice_t *candev)
{
	int i=0;

	DEBUGMSG("Resetting smartcan hardware ...\n");
	can_outb(0x00,candev->res_addr);
	while (i < 1000000) {
		i++;
		can_outb(0x01,candev->res_addr);
	}
	can_outb(0x00,candev->res_addr); 

	/* Check hardware reset status */
	i=0;
	can_outb(candev->io_addr+iCPU,candev->io_addr);
	while ( (can_inb(candev->io_addr+1)&0x80) && (i<=15) ) {
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

	return 0;
} 

int smartcan_init_hw_data(struct candevice_t *candev)
{
	candev->res_addr=candev->io_addr+0x02;
	candev->nr_82527_chips=1;
	candev->nr_sja1000_chips=0;
	candev->nr_all_chips=1;
	
	return 0;
}

int smartcan_init_chip_data(struct candevice_t *candev, int chipnr)
{
	i82527_fill_chipspecops(candev->chip[chipnr]);
	candev->chip[chipnr]->chip_base_addr=can_ioport2ioptr(candev->io_addr);
	candev->chip[chipnr]->clock = 16000000;
	candev->chip[chipnr]->int_cpu_reg = iCPU_DSC;
	candev->chip[chipnr]->int_clk_reg = iCLK_SL1;
	candev->chip[chipnr]->int_bus_reg = iBUS_CBY;
	candev->chip[chipnr]->sja_cdr_reg = 0;
	candev->chip[chipnr]->sja_ocr_reg = 0;
	smartcan_irq=candev->chip[chipnr]->chip_irq;
	smartcan_base=candev->chip[chipnr]->chip_base_addr;

	return 0;
}

int smartcan_init_obj_data(struct canchip_t *chip, int objnr)
{
	chip->msgobj[objnr]->obj_base_addr=(objnr+1)*0x10;

	return 0;
}


void smartcan_write_register(unsigned data, can_ioptr_t address)
{
	can_spin_irqflags_t flags;
	can_spin_lock_irqsave(&smartcan_port_lock,flags);
	can_outb(address-smartcan_base,smartcan_base);
	can_outb(data,smartcan_base+1);
	can_spin_unlock_irqrestore(&smartcan_port_lock,flags);
}

unsigned smartcan_read_register(can_ioptr_t address)
{
	unsigned ret;
	can_spin_irqflags_t flags;
	can_spin_lock_irqsave(&smartcan_port_lock,flags);
	can_outb(address-smartcan_base,smartcan_base);
	ret=can_inb(smartcan_base+1);
	can_spin_unlock_irqrestore(&smartcan_port_lock,flags);
	return ret;
}

int smartcan_program_irq(struct candevice_t *candev)
{
	CANMSG("The 'smartcan' card doesn't have programmable interrupts\n");
	return 0;
}

/* !!! Don't change this function !!! */
int smartcan_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = smartcan_request_io;
	hwspecops->release_io = smartcan_release_io;
	hwspecops->reset = smartcan_reset;
	hwspecops->init_hw_data = smartcan_init_hw_data;
	hwspecops->init_chip_data = smartcan_init_chip_data;
	hwspecops->init_obj_data = smartcan_init_obj_data;
	hwspecops->write_register = smartcan_write_register;
	hwspecops->read_register = smartcan_read_register;
	hwspecops->program_irq = smartcan_program_irq;
	return 0;
}
