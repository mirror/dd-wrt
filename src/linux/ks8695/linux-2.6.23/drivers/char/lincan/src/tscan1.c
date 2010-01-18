/* tscan1.c
 * Linux CAN-bus device driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 *
 * The support for TS-CAN1 and TS-7KV provided by Ronald Gomes
 * from Technologic Systems <http://www.embeddedarm.com/>
 */

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/pcm3680.h"
#include "../include/sja1000p.h"

#include <linux/module.h>
#include "../include/modparms.h"
#include "../include/devcommon.h"

#include "../include/tscan1.h"

static CAN_DEFINE_SPINLOCK(ts7kv_win_lock);

unsigned long tscanio[MAX_HW_CARDS]={-1,-1,-1,-1,-1,-1,-1,-1};
unsigned int tscanio_specified;

#if defined(TS7XXX_IO8_BASE)&&defined(TSXXX_BASE_IO)
int tsxxx_base=TS7XXX_IO8_BASE+TSXXX_BASE_IO;
#elif defined(TS7XXX_IO8_BASE)
int tsxxx_base=TS7XXX_IO8_BASE;
#else /*TS7XXX_IO8_BASE*/
unsigned long tsxxx_base=0;
#endif /*TS7XXX_IO8_BASE*/

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,12))
MODULE_PARM(tscanio, "1-" __MODULE_STRING(MAX_HW_CARDS)"i");
MODULE_PARM(tsxxx_base, "1i");
#else /* LINUX_VERSION_CODE >= 2,6,12 */
module_param_array(tscanio, int, &tscanio_specified, 0);
module_param(tsxxx_base, ulong, 0);
#endif /* LINUX_VERSION_CODE >= 2,6,12 */

MODULE_PARM_DESC(tscanio,"TSCAN CAN controller IO address for each board");
MODULE_PARM_DESC(tsxxx_base,"The base of the ISA/8-bit IO space for TSxxx CAN peripherals in the system");


/**
 * tscan1_request_io: - reserve io or memory range for can board
 * @candev: pointer to candevice/board which asks for io. Field @io_addr
 *	of @candev is used in most cases to define start of the range
 *
 * The function tscan1_request_io() is used to reserve the io-memory. If your
 * hardware uses a dedicated memory range as hardware control registers you
 * will have to add the code to reserve this memory as well.
 * %IO_RANGE is the io-memory range that gets reserved, please adjust according
 * your hardware. Example: #define IO_RANGE 0x100 for i82527 chips or
 * #define IO_RANGE 0x20 for sja1000 chips in basic CAN mode.
 * Return Value: The function returns zero on success or %-ENODEV on failure
 * File: src/tscan1.c
 */
int tscan1_request_io(struct candevice_t *candev)
{
	unsigned long can_io_addr;
	unsigned long remap_can_io_addr = 0;
	unsigned char mode;
	int i, j;


	if (!can_request_io_region(candev->io_addr, TSXXX_IO_RANGE, "tscan1-base")) {
		CANMSG("Unable to request base IO port: 0x%lx\n", candev->io_addr);
		return -ENODEV;
	} else {
		DEBUGMSG("Registered base IO port: 0x%lx - 0x%lx\n",
			candev->io_addr, candev->io_addr+TSXXX_IO_RANGE-1);
	}

	can_io_addr = tscanio[candev->candev_idx];

	if(can_io_addr && (can_io_addr != (unsigned long)-1)) {
		remap_can_io_addr = tsxxx_base + can_io_addr;

		if (!can_request_io_region(remap_can_io_addr, TSXXX_CAN_RANGE, "tscan1-can")) {
			CANMSG("Unable to request CAN IO port: 0x%lx\n", remap_can_io_addr);
			can_release_io_region(candev->io_addr, TSXXX_IO_RANGE);
			return -ENODEV;
		} else {
			DEBUGMSG("Registered CAN IO port: 0x%lx - 0x%lx\n",
			remap_can_io_addr, remap_can_io_addr+TSXXX_CAN_RANGE-1);
		}
	} else {
		for(i = 0; 1; i++) {

			if(i>=8) {
				CANMSG("Unable find range for CAN IO port\n");
				can_release_io_region(candev->io_addr, TSXXX_IO_RANGE);
				return -ENODEV;
			}

			can_io_addr = 0x100 + i * TSXXX_CAN_RANGE;
			for (j = 0; j < MAX_HW_CARDS; j++) {
				if(tscanio[j] == can_io_addr) {
					j = -1;
					break;
				}
			}
			if(j<0)
				continue;

			remap_can_io_addr = tsxxx_base + can_io_addr;

			if (can_request_io_region(remap_can_io_addr, TSXXX_CAN_RANGE, "tscan1-can"))
				break;
		}

		tscanio[candev->candev_idx] = can_io_addr;

		DEBUGMSG("Found free range and registered CAN IO port: 0x%lx - 0x%lx\n",
			remap_can_io_addr, remap_can_io_addr+TSXXX_CAN_RANGE-1);
	}

	candev->chip[0]->chip_base_addr = remap_can_io_addr;
	candev->chip[0]->msgobj[0]->obj_base_addr = remap_can_io_addr;

	switch(can_io_addr) {
		case 0x100:	mode=0x60; break;
		case 0x120:	mode=0x61; break;
		case 0x180:	mode=0x62; break;
		case 0x1A0:	mode=0x63; break;
		case 0x200:	mode=0x64; break;
		case 0x240:	mode=0x65; break;
		case 0x280:	mode=0x66; break;
		case 0x320:	mode=0x67; break;
		default:	mode=0x60; break;
	}

	can_outb(0x00, candev->io_addr+TSCAN1_WIN_REG);
	can_outb(mode, candev->io_addr+TSCAN1_MOD_REG);

	return 0;
}

int ts7kv_request_io(struct candevice_t *candev)
{
	if (!can_request_io_region(candev->io_addr, TSXXX_CAN_RANGE, "ts7kv-can")) {
		CANMSG("Unable to request CAN IO port: 0x%lx\n", candev->io_addr);
		return -ENODEV;
	} else {
		DEBUGMSG("Registered CAN IO port: 0x%lx - 0x%lx\n",
			candev->io_addr, candev->io_addr+TSXXX_CAN_RANGE-1);
	}

	return 0;
}

/**
 * tscan1_release_io - free reserved io memory range
 * @candev: pointer to candevice/board which releases io
 *
 * The function tscan1_release_io() is used to free reserved io-memory.
 * In case you have reserved more io memory, don't forget to free it here.
 * IO_RANGE is the io-memory range that gets released, please adjust according
 * your hardware. Example: #define IO_RANGE 0x100 for i82527 chips or
 * #define IO_RANGE 0x20 for sja1000 chips in basic CAN mode.
 * Return Value: The function always returns zero
 * File: src/tscan1.c
 */
int tscan1_release_io(struct candevice_t *candev)
{
	unsigned long remap_can_io_addr;

	if(candev->chip[0]){
		remap_can_io_addr = candev->chip[0]->chip_base_addr;
		if(remap_can_io_addr != (unsigned long)-1)
			can_release_io_region(remap_can_io_addr, TSXXX_CAN_RANGE);
	}

	can_outb(0x20, candev->io_addr+TSCAN1_MOD_REG);

	can_release_io_region(candev->io_addr, TSXXX_IO_RANGE);
	return 0;
}

int ts7kv_release_io(struct candevice_t *candev)
{
	can_release_io_region(candev->io_addr, TSXXX_CAN_RANGE);
	return 0;
}

/**
 * tscan1_reset - hardware reset routine
 * @candev: Pointer to candevice/board structure
 *
 * The function tscan1_reset() is used to give a hardware reset. This is
 * rather hardware specific so I haven't included example code. Don't forget to
 * check the reset status of the chip before returning.
 * Return Value: The function returns zero on success or %-ENODEV on failure
 * File: src/tscan1.c
 */
int tscan1_reset(struct candevice_t *candev)
{
	unsigned short i=0, chipnr;
	struct canchip_t *chip;

	DEBUGMSG("Resetting %s hardware ...\n", candev->hwname);

	for(chipnr=0;chipnr<candev->nr_sja1000_chips;chipnr++) {
		chip=candev->chip[chipnr];
		can_write_reg(chip, sjaMOD_RM, SJAMOD);
		udelay(1000);
		can_write_reg(chip, 0x00, SJAIER);
		udelay(1000);
		i=20;
		while (can_read_reg(chip, SJAMOD)&sjaMOD_RM){
			if(!i--) return -ENODEV;
			udelay(1000);
			can_write_reg(chip, 0, SJAMOD);
		}
		udelay(1000);
		can_write_reg(chip, sjaCDR_PELICAN, SJACDR);
		can_write_reg(chip, 0x00, SJAIER);
	}

	return 0;
}

#define RESET_ADDR 0x100
#define NR_82527 0
#define NR_SJA1000 1

int tscan1_check_presence(unsigned long remap_io_addr, int *pjmp)
{
	int result = -ENODEV;

	if (!can_request_io_region(remap_io_addr, TSXXX_IO_RANGE, "tscan1-probe"))
		return -ENODEV;

	do {
		if (can_inb(remap_io_addr+TSXXX_ID0_REG)!=TSCAN1_ID0 ||
			can_inb(remap_io_addr+TSXXX_ID1_REG)!=TSCAN1_ID1)
			break;

		can_outb(0x00, remap_io_addr+TSCAN1_WIN_REG);
		can_outb(0x20, remap_io_addr+TSCAN1_MOD_REG);

		if(pjmp)
			*pjmp = can_inb(remap_io_addr+TSCAN1_JMP_REG);

		result = 0;
	} while (0);

	can_release_io_region(remap_io_addr, TSXXX_IO_RANGE);

	return result;
}


/**
 * tscan1_init_hw_data - Initialize hardware cards
 * @candev: Pointer to candevice/board structure
 *
 * The function tscan1_init_hw_data() is used to initialize the hardware
 * structure containing information about the installed CAN-board.
 * %RESET_ADDR represents the io-address of the hardware reset register.
 * %NR_82527 represents the number of intel 82527 chips on the board.
 * %NR_SJA1000 represents the number of philips sja1000 chips on the board.
 * The flags entry can currently only be %CANDEV_PROGRAMMABLE_IRQ to indicate that
 * the hardware uses programmable interrupts.
 * Return Value: The function always returns zero
 * File: src/tscan1.c
 */

int tscan1_init_hw_data(struct candevice_t *candev)
{
	int i, j, jmp;
	unsigned long io_addr;
	unsigned long remap_io_addr;

	io_addr = candev->io_addr;

	if(io_addr && (io_addr != (unsigned long)-1)) {
		remap_io_addr = io_addr + tsxxx_base;

		if(tscan1_check_presence(remap_io_addr, &jmp)){
			CANMSG("No TSCAN1 card found at address 0xlx\n");
			return -ENODEV;
		}
	} else {
		DEBUGMSG("Scanning bus for TS-CAN1 boards...\n");

		for (i=0; 1;i++)
		{
			if(i >= 4) {
				CANMSG("No TS-CAN1 boards found for slot %d\n", candev->candev_idx);
				return -ENODEV;
			}

			io_addr = TSCAN1_BASE_IO + i*TSXXX_IO_RANGE;
			remap_io_addr = io_addr + tsxxx_base;

			for (j = 0; j < MAX_HW_CARDS; j++) {
				if(io[j] == io_addr){
					j = -1;
					break;
				}
			}
			if(j<0)
				continue;

			if(!tscan1_check_presence(remap_io_addr, &jmp))
				break;

		}
		DEBUGMSG("TS-CAN1 board was found at 0x%lx for driver slot %d\n",
					io_addr, candev->candev_idx);

		io[candev->candev_idx] = io_addr;
	}

	candev->io_addr = remap_io_addr;
	/* unused reset address is used to store jumper setting */
	candev->res_addr = jmp;

	candev->nr_82527_chips=NR_82527;
	candev->nr_sja1000_chips=NR_SJA1000;
	candev->nr_all_chips=NR_82527+NR_SJA1000;
	candev->flags &= ~CANDEV_PROGRAMMABLE_IRQ;

	DEBUGMSG("Memory region at 0x%lx assigned to sja1000 of driver %d/%s\n",
		candev->io_addr, candev->candev_idx, candev->hwname);

	return 0;
}


int ts7kv_check_presence(unsigned long remap_io_addr, int *pjmp)
{
	int result = -ENODEV;

	if (!can_request_io_region(remap_io_addr, TSXXX_IO_RANGE, "ts7kv-probe"))
		return -ENODEV;

	do {
		if (can_inb(remap_io_addr+TSXXX_ID0_REG)!=TS7KV_ID0 ||
			can_inb(remap_io_addr+TSXXX_ID1_REG)!=TS7KV_ID1)
			break;

		if(pjmp)
			*pjmp = can_inb(remap_io_addr+TS7KV_JMP_REG);

		result = 0;
	} while (0);

	can_release_io_region(remap_io_addr, TSXXX_IO_RANGE);

	return result;
}

int ts7kv_init_hw_data(struct candevice_t *candev)
{
	int i, j, jmp;
	unsigned long io_addr;
	unsigned long remap_io_addr;
	unsigned long can_io_addr;

	io_addr = candev->io_addr;

	if(io_addr && (io_addr != (unsigned long)-1)) {
		remap_io_addr = io_addr + tsxxx_base;

		if(ts7kv_check_presence(remap_io_addr, &jmp)){
			CANMSG("No TS7KV card found at address 0xlx\n");
			return -ENODEV;
		}
	} else {
		DEBUGMSG("Scanning bus for TS7KV boards...\n");

		for (i=0; 1;i++)
		{
			if(i >= 4) {
				CANMSG("No TS7KV boards found for slot %d\n", candev->candev_idx);
				return -ENODEV;
			}

			io_addr = TS7KV_BASE_IO + i*TSXXX_IO_RANGE;
			remap_io_addr = io_addr + tsxxx_base;

			for (j = 0; j < MAX_HW_CARDS; j++) {
				if(io[j] == io_addr){
					j = -1;
					break;
				}
			}
			if(j<0)
				continue;

			if(!ts7kv_check_presence(remap_io_addr, &jmp))
				break;

		}
		DEBUGMSG("TS7KV board was found at 0x%lx for driver slot %d\n",
					io_addr, candev->candev_idx);

		io[candev->candev_idx] = io_addr;
	}

	can_io_addr = ((io_addr>>3)&0x03)*0x20;
	tscanio[candev->candev_idx] = can_io_addr;

	/* dev_base_addr address is used to store remapped PLD base address */
	candev->dev_base_addr = remap_io_addr;

	/* dev_base_addr address is used to store remapped slave window address */
	candev->io_addr = can_io_addr+tsxxx_base;

	/* unused reset address is used to store jumper setting */
	candev->res_addr = jmp;

	candev->nr_82527_chips=NR_82527;
	candev->nr_sja1000_chips=NR_SJA1000;
	candev->nr_all_chips=NR_82527+NR_SJA1000;
	candev->flags &= ~CANDEV_PROGRAMMABLE_IRQ;

	DEBUGMSG("Memory region at 0x%lx assigned to sja1000 of driver %d/%s\n",
		candev->io_addr, candev->candev_idx, candev->hwname);

	return 0;
}

/**
 * tscan1_init_chip_data - Initialize chips
 * @candev: Pointer to candevice/board structure
 * @chipnr: Number of the CAN chip on the hardware card
 *
 * The function tscan1_init_chip_data() is used to initialize the hardware
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
 * File: src/tscan1.c
 */
int tscan1_init_chip_data(struct candevice_t *candev, int chipnr)
{
	unsigned long default_clk = 16000 * 1000;
	int jmp;
	int irq = -1;

	/* unused reset address is used to store jumper setting */
	jmp = candev->res_addr;

	if (jmp&0x10 && jmp&0x20) irq=TSXXX_IRQ5;
	else if (jmp&0x10) irq=TSXXX_IRQ6;
	else if (jmp&0x20) irq=TSXXX_IRQ7;

	if(irq<0) {
		CANMSG("Jumpers select no IRQ for TSCAN1 at 0x%lx of driver %d/%s\n",
			candev->io_addr, candev->candev_idx, candev->hwname);
		return -ENODEV;
	}
	candev->chip[chipnr]->chip_irq = irq;

	sja1000p_fill_chipspecops(candev->chip[chipnr]);

	if(candev->chip[chipnr]->clock <= 0)
		candev->chip[chipnr]->clock = default_clk;
	candev->chip[chipnr]->int_clk_reg = 0x0;
	candev->chip[chipnr]->int_bus_reg = 0x0;
	candev->chip[chipnr]->sja_cdr_reg = sjaCDR_CBP | sjaCDR_CLK_OFF;
	candev->chip[chipnr]->sja_ocr_reg = sjaOCR_MODE_NORMAL | sjaOCR_TX0_LH;
	/* 
	 * The address is assigned during tscan1_request_io()
	 * according to found free ranges or tscanio option
	 */
	candev->chip[chipnr]->chip_base_addr = (unsigned long)-1;

	return 0;
}

int ts7kv_init_chip_data(struct candevice_t *candev, int chipnr)
{
	unsigned long default_clk = 16000 * 1000;
	int jmp;
	int irq = -1;

	/* unused reset address is used to store jumper setting */
	jmp = candev->res_addr;

	if (jmp&0x10 && jmp&0x20) irq=TSXXX_IRQ5;
	else if (jmp&0x10) irq=TSXXX_IRQ6;
	else if (jmp&0x20) irq=TSXXX_IRQ7;

	if(irq<0) {
		CANMSG("Jumpers select no IRQ for TS7KV CAN at 0x%lx of driver %d/%s\n",
			candev->io_addr, candev->candev_idx, candev->hwname);
		return -ENODEV;
	}

	candev->chip[chipnr]->chip_irq = irq;

	sja1000p_fill_chipspecops(candev->chip[chipnr]);

	if(candev->chip[chipnr]->clock <= 0)
		candev->chip[chipnr]->clock = default_clk;
	candev->chip[chipnr]->int_clk_reg = 0x0;
	candev->chip[chipnr]->int_bus_reg = 0x0;
	candev->chip[chipnr]->sja_cdr_reg = sjaCDR_CBP | sjaCDR_CLK_OFF;
	candev->chip[chipnr]->sja_ocr_reg = sjaOCR_MODE_NORMAL | sjaOCR_TX0_LH;
	candev->chip[chipnr]->chip_base_addr = candev->io_addr;

	return 0;
}

/**
 * tscan1_init_obj_data - Initialize message buffers
 * @chip: Pointer to chip specific structure
 * @objnr: Number of the message buffer
 *
 * The function tscan1_init_obj_data() is used to initialize the hardware
 * structure containing information about the different message objects on the
 * CAN chip. In case of the sja1000 there's only one message object but on the
 * i82527 chip there are 15.
 * The code below is for a i82527 chip and initializes the object base addresses
 * The entry @obj_base_addr represents the first memory address of the message
 * object. In case of the sja1000 @obj_base_addr is taken the same as the chips
 * base address.
 * Unless the hardware uses a segmented memory map, flags can be set zero.
 * Return Value: The function always returns zero
 * File: src/tscan1.c
 */
int tscan1_init_obj_data(struct canchip_t *chip, int objnr)
{
	chip->msgobj[objnr]->obj_base_addr = chip->chip_base_addr;
	return 0;
}

/**
 * tscan1_program_irq - program interrupts
 * @candev: Pointer to candevice/board structure
 *
 * The function tscan1_program_irq() is used for hardware that uses
 * programmable interrupts. If your hardware doesn't use programmable interrupts
 * you should not set the @candevices_t->flags entry to %CANDEV_PROGRAMMABLE_IRQ and
 * leave this function unedited. Again this function is hardware specific so
 * there's no example code.
 * Return value: The function returns zero on success or %-ENODEV on failure
 * File: src/tscan1.c
 */
int tscan1_program_irq(struct candevice_t *candev)
{
	return 0;
}

/**
 * tscan1_write_register - Low level write register routine
 * @data: data to be written
 * @address: memory address to write to
 *
 * The function tscan1_write_register() is used to write to hardware registers
 * on the CAN chip. You should only have to edit this function if your hardware
 * uses some specific write process.
 * Return Value: The function does not return a value
 * File: src/tscan1.c
 */
void tscan1_write_register(unsigned data, can_ioptr_t address)
{
	can_outb(data, address);
}

void ts7kv_write_register(unsigned data, can_ioptr_t address)
{
	unsigned long addr=can_ioptr2ulong(address);
	can_ioptr_t base = can_ulong2ioptr(addr & ~0x1f);
	unsigned char nwin = 0x10;
	unsigned char savewin;
	
	can_spin_irqflags_t flags;

	if((addr&0x1f) > 0x1d) {
		nwin++;
		address -= 0x10;
	}

	can_spin_lock_irqsave(&ts7kv_win_lock,flags);
	savewin = can_inb(base+TS7KV_WIN_REG);
	if(nwin == savewin) {
		can_outb(data, address);
	}else{
		can_outb(nwin, base+TS7KV_WIN_REG);
		can_outb(data, address);
		can_outb(savewin, base+TS7KV_WIN_REG);
	}
	can_spin_unlock_irqrestore(&ts7kv_win_lock,flags);
}

/**
 * tscan1_read_register - Low level read register routine
 * @address: memory address to read from
 *
 * The function tscan1_read_register() is used to read from hardware registers
 * on the CAN chip. You should only have to edit this function if your hardware
 * uses some specific read process.
 * Return Value: The function returns the value stored in @address
 * File: src/tscan1.c
 */
unsigned tscan1_read_register(can_ioptr_t address)
{
	return can_inb(address);
}

unsigned ts7kv_read_register(can_ioptr_t address)
{
	unsigned long addr=can_ioptr2ulong(address);
	can_ioptr_t base = can_ulong2ioptr(addr & ~0x1f);
	unsigned char nwin = 0x10;
	unsigned char savewin;
	unsigned val;
	
	can_spin_irqflags_t flags;

	if((addr&0x1f) > 0x1d) {
		nwin++;
		address -= 0x10;
	}

	can_spin_lock_irqsave(&ts7kv_win_lock,flags);
	savewin = can_inb(base+TS7KV_WIN_REG);
	if(nwin == savewin) {
		val = can_inb(address);
	}else{
		can_outb(nwin, base+TS7KV_WIN_REG);
		val = can_inb(address);
		can_outb(savewin, base+TS7KV_WIN_REG);
	}
	can_spin_unlock_irqrestore(&ts7kv_win_lock,flags);

	return val;
}

int tscan1_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = tscan1_request_io;
	hwspecops->release_io = tscan1_release_io;
	hwspecops->reset = tscan1_reset;
	hwspecops->init_hw_data = tscan1_init_hw_data;
	hwspecops->init_chip_data = tscan1_init_chip_data;
	hwspecops->init_obj_data = tscan1_init_obj_data;
	hwspecops->write_register = tscan1_write_register;
	hwspecops->read_register = tscan1_read_register;
	hwspecops->program_irq = tscan1_program_irq;
	return 0;
}

extern int ts7kv_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = ts7kv_request_io;
	hwspecops->release_io = ts7kv_release_io;
	hwspecops->reset = tscan1_reset;
	hwspecops->init_hw_data = ts7kv_init_hw_data;
	hwspecops->init_chip_data = ts7kv_init_chip_data;
	hwspecops->init_obj_data = tscan1_init_obj_data;
	hwspecops->write_register = ts7kv_write_register;
	hwspecops->read_register = ts7kv_read_register;
	hwspecops->program_irq = tscan1_program_irq;
	return 0;
}
