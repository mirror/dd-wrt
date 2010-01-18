/* pcccan.h
 * Header file for the Linux CAN-bus driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

int pcccan_request_io(struct candevice_t *candev);
int pcccan_release_io(struct candevice_t *candev);
int pcccan_reset(struct candevice_t *candev); 
int pcccan_init_hw_data(struct candevice_t *candev);
int pcccan_init_chip_data(struct candevice_t *candev, int chipnr);
int pcccan_init_obj_data(struct canchip_t *chip, int objnr);
void pcccan_write_register(unsigned data, can_ioptr_t address);
unsigned pcccan_read_register(can_ioptr_t address);
int pcccan_program_irq(struct candevice_t *candev);

