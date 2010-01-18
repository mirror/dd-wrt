/* pccan.c
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
#include "../include/pccan.h"
#include "../include/i82527.h"
#include "../include/sja1000.h"

int pccanf_request_io(struct candevice_t *candev)
{
	if (!can_request_io_region(candev->io_addr+0x4000,0x20,DEVICE_NAME)) {
		CANMSG("Unable to open port: 0x%lx\n",candev->io_addr+0x4000);
		return -ENODEV;
	}
	else if (!can_request_io_region(candev->io_addr+0x6000,0x04,DEVICE_NAME)) {
		can_release_io_region(candev->io_addr+0x4000,0x20);
		CANMSG("Unable to open port: 0x%lx\n",candev->io_addr+0x6000);
		return -ENODEV;
	}
	else {
		DEBUGMSG("Registered IO-memory: 0x%lx - 0x%lx\n", candev->io_addr+0x4000, candev->io_addr+0x4000+0x20-1);
		DEBUGMSG("Registered IO-memory: 0x%lx - 0x%lx\n", candev->io_addr+0x6000, candev->io_addr+0x6000+0x04-1);
	}
	return 0;
}

int pccand_request_io(struct candevice_t *candev)
{
	if (pccanf_request_io(candev))
		return -ENODEV;

	if (!can_request_io_region(candev->io_addr+0x5000,0x20,DEVICE_NAME)) {
		pccanf_release_io(candev);
		CANMSG("Unable to open port: 0x%lx\n",candev->io_addr+0x5000);
		return -ENODEV;
	}
	else {
		DEBUGMSG("Registered IO-memory: 0x%lx - 0x%lx\n", candev->io_addr+0x5000, candev->io_addr+0x5000+0x20-1);
	}
	return 0;
}

int pccanq_request_io(struct candevice_t *candev)
{
	unsigned long io_addr;
	int i;
	
	if (pccand_request_io(candev))
		return -ENODEV;

	for(i=0, io_addr=candev->io_addr+0x2000; i<8; i++, io_addr+=0x400) {
		if (!can_request_io_region(io_addr,0x40,DEVICE_NAME)) {
			CANMSG("Unable to open port: 0x%lx\n",io_addr);
			while(i--){
				io_addr-=0x400;
				can_release_io_region(io_addr,0x40);
			}
			pccand_release_io(candev);
			return -ENODEV;
		}
		DEBUGMSG("Registered IO-memory: 0x%lx - 0x%lx\n", io_addr, io_addr+0x40-1);
	}
	return 0;
}

int pccanf_release_io(struct candevice_t *candev)
{
	can_release_io_region(candev->io_addr+0x4000,0x20);
	can_release_io_region(candev->io_addr+0x6000,0x04);

	return 0;
}

int pccand_release_io(struct candevice_t *candev)
{
	pccanf_release_io(candev);
	can_release_io_region(candev->io_addr+0x5000,0x20);

	return 0;
}

int pccanq_release_io(struct candevice_t *candev)
{
	unsigned long io_addr;
	int i;

	pccand_release_io(candev);

	for(i=0, io_addr=candev->io_addr+0x2000; i<8; i++, io_addr+=0x400) {
		can_release_io_region(io_addr,0x40);
	}

	return 0;
}

int pccanf_reset(struct candevice_t *candev)
{
	int i=0;

	DEBUGMSG("Resetting pccanf/s hardware ...\n");
	while (i < 1000000) {
		i++;
		can_outb(0x00,candev->res_addr);
	}
	can_outb(0x01,candev->res_addr);
	can_outb(0x00,candev->chip[0]->chip_base_addr+SJACR);

	/* Check hardware reset status */
	i=0;
	while ( (can_inb(candev->chip[0]->chip_base_addr+SJACR) & sjaCR_RR)
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

int pccand_reset(struct candevice_t *candev)
{
	int i=0,chip_nr=0;

	DEBUGMSG("Resetting pccan-d hardware ...\n");
	while (i < 1000000) {
		i++;
		can_outb(0x00,candev->res_addr);
	}
	can_outb(0x01,candev->res_addr);
	can_outb(0x00,candev->chip[0]->chip_base_addr+SJACR);
	can_outb(0x00,candev->chip[1]->chip_base_addr+SJACR);

	/* Check hardware reset status */
	i=0;
	for (chip_nr=0; chip_nr<2; chip_nr++) {
		i=0;
		while ( (can_inb(candev->chip[chip_nr]->chip_base_addr +
						SJACR) & sjaCR_RR) && (i<=15) ) {
			udelay(20000);
			i++;
		}
		if (i>=15) {
			CANMSG("Reset status timeout!\n");
			CANMSG("Please check your hardware.\n");
			return -ENODEV;
		}
		else
			DEBUGMSG("Chip%d reset status ok.\n",chip_nr);
	}
	return 0;
}

int pccanq_reset(struct candevice_t *candev)
{
	int i=0,chip_nr=0;

	for (i=0; i<4; i++)
		can_disable_irq(candev->chip[i]->chip_irq);

	DEBUGMSG("Resetting pccan-q hardware ...\n");
	while (i < 100000) {
		i++;
		can_outb(0x00,candev->res_addr);
	}
	outb_p(0x01,candev->res_addr);
		
	can_outb(0x00,candev->chip[2]->chip_base_addr+SJACR);
	can_outb(0x00,candev->chip[3]->chip_base_addr+SJACR);

	/* Check hardware reset status */
	for (chip_nr=0; chip_nr<2; chip_nr++) {
		i=0;
		while( (can_inb(candev->chip[chip_nr]->chip_base_addr +
						iCPU) & iCPU_RST) && (i<=15) ) {
			udelay(20000);
			i++;
		}
		if (i>=15) {
			CANMSG("Reset status timeout!\n");
			CANMSG("Please check your hardware.\n");
			return -ENODEV;
		}
		else 
			DEBUGMSG("Chip%d reset status ok.\n",chip_nr);
	}
	for (chip_nr=2; chip_nr<4; chip_nr++) {
		i=0;
		while( (can_inb(candev->chip[chip_nr]->chip_base_addr +
						SJACR) & sjaCR_RR) && (i<=15) ) {
			udelay(20000);
			i++;
		}
		if (i>=15) {
			CANMSG("Reset status timeout!\n");
			CANMSG("Please check your hardware.\n");
			return -ENODEV;
		}
		else
			DEBUGMSG("Chip%d reset status ok.\n",chip_nr);
	}

	for (i=0; i<4; i++)
		can_enable_irq(candev->chip[i]->chip_irq);

	return 0;
} 	

int pccan_init_hw_data(struct candevice_t *candev)
{
	candev->res_addr=candev->io_addr+0x6001;
	candev->flags |= CANDEV_PROGRAMMABLE_IRQ;

	if (!strcmp(candev->hwname,"pccan-q")) {
		candev->nr_82527_chips=2;
		candev->nr_sja1000_chips=2;
		candev->nr_all_chips=4;
	}
	if (!strcmp(candev->hwname,"pccan-f") |
	    !strcmp(candev->hwname,"pccan-s")) {
		candev->nr_82527_chips=0;
		candev->nr_sja1000_chips=1;
		candev->nr_all_chips=1;
	}
	if (!strcmp(candev->hwname,"pccan-d")) {
		candev->nr_82527_chips=0;
		candev->nr_sja1000_chips=2;
		candev->nr_all_chips=2;
	}

	return 0;
}

int pccan_init_chip_data(struct candevice_t *candev, int chipnr)
{
	if (!strcmp(candev->hwname,"pccan-q")) {
		if (chipnr<2) {
			i82527_fill_chipspecops(candev->chip[chipnr]);
			candev->chip[chipnr]->flags = CHIP_SEGMENTED;
			candev->chip[chipnr]->int_cpu_reg=iCPU_DSC;
			candev->chip[chipnr]->int_clk_reg=iCLK_SL1;
			candev->chip[chipnr]->int_bus_reg=iBUS_CBY;
			candev->chip[chipnr]->sja_cdr_reg = 0;
			candev->chip[chipnr]->sja_ocr_reg = 0;	
		}
		else{
			sja1000_fill_chipspecops(candev->chip[chipnr]);
			candev->chip[chipnr]->flags = 0;
			candev->chip[chipnr]->int_cpu_reg = 0;
			candev->chip[chipnr]->int_clk_reg = 0;
			candev->chip[chipnr]->int_bus_reg = 0;
			candev->chip[chipnr]->sja_cdr_reg =
								sjaCDR_CLK_OFF;
			candev->chip[chipnr]->sja_ocr_reg = 
						sjaOCR_MODE_NORMAL | sjaOCR_TX0_LH;	
		}
		candev->chip[chipnr]->chip_base_addr=can_ioport2ioptr(0x1000*chipnr+0x2000+candev->io_addr);
	}
	else {
		sja1000_fill_chipspecops(candev->chip[chipnr]);
		candev->chip[chipnr]->chip_base_addr=can_ioport2ioptr(0x1000*chipnr+0x4000+candev->io_addr);
		candev->chip[chipnr]->flags = 0;
		candev->chip[chipnr]->int_cpu_reg = 0;
		candev->chip[chipnr]->int_clk_reg = 0;
		candev->chip[chipnr]->int_bus_reg = 0;
		candev->chip[chipnr]->sja_cdr_reg = sjaCDR_CLK_OFF;
		candev->chip[chipnr]->sja_ocr_reg = 
						sjaOCR_MODE_NORMAL | sjaOCR_TX0_LH;	
	}

	candev->chip[chipnr]->clock = 16000000;

	return 0;
}	

int pccan_init_obj_data(struct canchip_t *chip, int objnr)
{
	if (!strcmp(chip->chip_type,"sja1000")) {
		chip->msgobj[objnr]->obj_base_addr=chip->chip_base_addr;
		}
	else {	/* The spacing for this card is 0x3c0 */
		chip->msgobj[objnr]->obj_base_addr=chip->chip_base_addr+(objnr+1)*0x10+(int)((objnr+1)/4)*0x3c0;
		}

	return 0;
}

int pccan_program_irq(struct candevice_t *candev)
{
	#define IRQ9 0x01
	#define IRQ3 0x02
	#define IRQ5 0x03

	unsigned char irq_reg_value=0;
	int i;

	for (i=0; i<4; i++) {
		switch (candev->chip[i]->chip_irq) {
			case 0: {
				break;
			}
			case 3: {
				irq_reg_value |= (IRQ3<<(i*2));
				break;
			}
			case 5: {
				irq_reg_value |= (IRQ5<<(i*2));
				break;
			}
			case 9: {
				irq_reg_value |= (IRQ9<<(i*2));
				break;
			}
			default: {
				CANMSG("Supplied interrupt is not supported by the hardware\n");
				return -ENODEV;
			}
		}
	}
	can_outb(irq_reg_value,0x6000+candev->io_addr);
	DEBUGMSG("Configured pccan hardware interrupts\n");
	can_outb(0x80,0x6000+candev->io_addr+0x02);
	DEBUGMSG("Selected pccan on-board 16 MHz oscillator\n");

	return 0;
}

inline void pccan_write_register(unsigned data, can_ioptr_t address)
{
	can_outb(data,address); 
}

unsigned pccan_read_register(can_ioptr_t address)
{
	return can_inb(address);
}

int pccanf_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = pccanf_request_io;
	hwspecops->release_io = pccanf_release_io;
	hwspecops->reset = pccanf_reset;
	hwspecops->init_hw_data = pccan_init_hw_data;
	hwspecops->init_chip_data = pccan_init_chip_data;
	hwspecops->init_obj_data = pccan_init_obj_data;
	hwspecops->write_register = pccan_write_register;
	hwspecops->read_register = pccan_read_register;
	hwspecops->program_irq = pccan_program_irq;
	return 0;
}


int pccand_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = pccand_request_io;
	hwspecops->release_io = pccand_release_io;
	hwspecops->reset = pccand_reset;
	hwspecops->init_hw_data = pccan_init_hw_data;
	hwspecops->init_chip_data = pccan_init_chip_data;
	hwspecops->init_obj_data = pccan_init_obj_data;
	hwspecops->write_register = pccan_write_register;
	hwspecops->read_register = pccan_read_register;
	hwspecops->program_irq = pccan_program_irq;
	return 0;
}


int pccanq_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = pccanq_request_io;
	hwspecops->release_io = pccanq_release_io;
	hwspecops->reset = pccanq_reset;
	hwspecops->init_hw_data = pccan_init_hw_data;
	hwspecops->init_chip_data = pccan_init_chip_data;
	hwspecops->init_obj_data = pccan_init_obj_data;
	hwspecops->write_register = pccan_write_register;
	hwspecops->read_register = pccan_read_register;
	hwspecops->program_irq = pccan_program_irq;
	return 0;
}
