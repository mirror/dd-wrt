/* sh7760.h
 * Header file for the Linux CAN-bus driver.
 * This software is released under the GPL-License.
 */

#define NR_82527	0
#define NR_SJA1000	0
#define NR_ALL		2

#define SH7760_CAN_IRQ 		56
#define SH7760_CAN_CHIP_OFFSET	0x10000
#define SH7760_CAN_CLOCK	33333330	  /* 33.3 MHz */

#define IO_RANGE 		0x10000

/* static CAN_DEFINE_SPINLOCK(sh7760_port_lock); */



int sh7760_request_io(struct candevice_t *candev);
int sh7760_release_io(struct candevice_t *candev);
int sh7760_reset(struct candevice_t *candev); 
int sh7760_init_hw_data(struct candevice_t *candev);
int sh7760_init_chip_data(struct candevice_t *candev, int chipnr);
int sh7760_init_obj_data(struct canchip_t *chip, int objnr);
int sh7760_program_irq(struct candevice_t *candev);
void sh7760_write_register(unsigned data, can_ioptr_t address);
unsigned sh7760_read_register(can_ioptr_t address);

