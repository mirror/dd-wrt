/* template.h
 * Header file for the Linux CAN-bus driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

//Ids SECTION
#define TSCAN1_ID0		0xF6
#define TSCAN1_ID1		0xB9
#define TS7KV_ID0		0x41
#define TS7KV_ID1		0x20

//MEMORY SECTION
#ifdef CONFIG_ARM
#include <asm-arm/arch-ep93xx/regmap.h>
#define TSXXX_BASE_IO	0x01E00000
#endif

#define TSCAN1_BASE_IO	0x150
#define TS7KV_BASE_IO	0xE0
#define TSXXX_IO_RANGE	0x8
#define TSXXX_CAN_RANGE	0x20

#define TSXXX_ID0_REG	0x0
#define TSXXX_ID1_REG	0x1
#define TSXXX_PLD_REG	0x2

#define TSCAN1_WIN_REG	0x4
#define TSCAN1_MOD_REG	0x5
#define TSCAN1_JMP_REG	0x6

#define TS7KV_CTR1_REG	0x4
#define TS7KV_CTR2_REG	0x5
#define TS7KV_FPGA_REG	0x6
#define TS7KV_JMP_REG	0x7
#define TS7KV_WIN_REG	0x1E

//IRQs
#ifdef CONFIG_ARM
#define TSXXX_IRQ5		5
#define TSXXX_IRQ6		33
#define TSXXX_IRQ7		40
#endif

#ifdef CONFIG_X86
#define TSXXX_IRQ5		5
#define TSXXX_IRQ6		6
#define TSXXX_IRQ7		7
#endif


int tscan1_request_io(struct candevice_t *candev);
int tscan1_release_io(struct candevice_t *candev);
int tscan1_reset(struct candevice_t *candev);
int tscan1_init_hw_data(struct candevice_t *candev);
int tscan1_init_chip_data(struct candevice_t *candev, int chipnr);
int tscan1_init_obj_data(struct canchip_t *chip, int objnr);
void tscan1_write_register(unsigned data, can_ioptr_t address);
unsigned tscan1_read_register(can_ioptr_t address);
int tscan1_program_irq(struct candevice_t *candev);

unsigned long tscan1_getmappedaddr(unsigned long address);
unsigned short tscan1_getcandevidx(unsigned long address);
unsigned long tscan1_setpage_getaddr(unsigned long address, signed short *nwinbak, unsigned long *winaddr);
