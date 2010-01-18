/* pcm3680.h
 * Header file for the Linux CAN-bus driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

int pcm3680_request_io(struct candevice_t *candev);
int pcm3680_release_io(struct candevice_t *candev);
int pcm3680_reset(struct candevice_t *candev); 
int pcm3680_init_hw_data(struct candevice_t *candev);
int pcm3680_init_chip_data(struct candevice_t *candev, int chipnr);
int pcm3680_init_obj_data(struct canchip_t *chip, int objnr);
void pcm3680_write_register(unsigned data, can_ioptr_t address);
unsigned pcm3680_read_register(can_ioptr_t address);
int pcm3680_program_irq(struct candevice_t *candev);

