/* oscar.c
 * Linux CAN-bus device driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004

 NOTE: Please see the template.c file for the comments relating to this code.
       Herein is the modified copy of the functions

 */ 
#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/oscar.h"
#include "../include/sja1000p.h"

#define IO_RANGE 0x80 // allow both basic CAN and PeliCAN modes for sja1000

int oscar_request_io(struct candevice_t *candev)
{
	if (!can_request_io_region(candev->io_addr,IO_RANGE,DEVICE_NAME)) {
		CANMSG("Unable to open port: 0x%lx\n",candev->io_addr);
		return -ENODEV;
	}else {
		DEBUGMSG("Registered IO-memory: 0x%lx - 0x%lx\n", candev->io_addr, candev->io_addr + IO_RANGE - 1);
	}
	return 0;
}

int oscar_release_io(struct candevice_t *candev)
{
	can_release_io_region(candev->io_addr,IO_RANGE);

	return 0;
}

int oscar_reset(struct candevice_t *candev)
{
	int i;
	struct canchip_t *chip=candev->chip[0];
	unsigned cdr;

	oscar_write_register(sjaMOD_RM, chip->chip_base_addr+SJAMOD);
	udelay(1000);

	cdr=oscar_read_register(chip->chip_base_addr+SJACDR);
	oscar_write_register(cdr|sjaCDR_PELICAN, chip->chip_base_addr+SJACDR);

	oscar_write_register(0, chip->chip_base_addr+SJAIER);

	i=20;
	oscar_write_register(0, chip->chip_base_addr+SJAMOD);
	while (oscar_read_register(chip->chip_base_addr+SJAMOD)&sjaMOD_RM){
		if(!i--) return -ENODEV;
		udelay(1000);
		oscar_write_register(0, chip->chip_base_addr+SJAMOD);
	}

	cdr=oscar_read_register(chip->chip_base_addr+SJACDR);
	oscar_write_register(cdr|sjaCDR_PELICAN, chip->chip_base_addr+SJACDR);

	oscar_write_register(0, chip->chip_base_addr+SJAIER);

	return 0;
}

int oscar_init_hw_data(struct candevice_t *candev) 
{
    candev->res_addr = 0x0; // RESET address?
    candev->nr_82527_chips = 0;
    candev->nr_sja1000_chips = 1; // We've got a SJA1000 variant
    candev->nr_all_chips= 1;
    candev->flags |= CANDEV_PROGRAMMABLE_IRQ;

    return 0;
}

int oscar_init_chip_data(struct candevice_t *candev, int chipnr)
{
    // i82527_fill_chipspecops(candev->chip[chipnr]);
    // sja1000_fill_chipspecops(candev->chip[chipnr]);
    sja1000p_fill_chipspecops(candev->chip[chipnr]);
	
    candev->chip[chipnr]->chip_base_addr = can_ioport2ioptr(candev->io_addr);
    candev->chip[chipnr]->clock = 12000000;
    candev->chip[chipnr]->sja_cdr_reg = sjaCDR_CBP;  // we use an external tranceiver
    candev->chip[chipnr]->sja_ocr_reg = sjaOCR_MODE_NORMAL | sjaOCR_TX0_LH;
    // these three int_ registers are unused (we don't have this chip)
    candev->chip[chipnr]->int_cpu_reg = 0;
    candev->chip[chipnr]->int_clk_reg = 0;
    candev->chip[chipnr]->int_bus_reg = 0;
    
    return 0;
}

int oscar_init_obj_data(struct canchip_t *chip, int objnr)
{
    chip->msgobj[objnr]->obj_base_addr = chip->chip_base_addr;
    
    return 0;
}

int oscar_program_irq(struct candevice_t *candev)
{
    // CAN_IRQ_L (active low) interrupt: PF2 / INT2 on our LH7A400 SoC
    // This IRQ is set up already by the kernel.    

    return 0;
}

void oscar_write_register(unsigned data, can_ioptr_t address)
{
	can_outb(data,address);
}

unsigned oscar_read_register(can_ioptr_t address)
{
	return can_inb(address);
}

/* !!! Don't change this function !!! */
int oscar_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = oscar_request_io;
	hwspecops->release_io = oscar_release_io;
	hwspecops->reset = oscar_reset;
	hwspecops->init_hw_data = oscar_init_hw_data;
	hwspecops->init_chip_data = oscar_init_chip_data;
	hwspecops->init_obj_data = oscar_init_obj_data;
	hwspecops->write_register = oscar_write_register;
	hwspecops->read_register = oscar_read_register;
	hwspecops->program_irq = oscar_program_irq;
	return 0;
}
