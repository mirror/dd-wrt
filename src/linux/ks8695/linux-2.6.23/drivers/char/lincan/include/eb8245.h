/* eb8245.h
 * Header file for the Linux CAN-bus driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Added by Pavel Pisa pisa@cmp.felk.cvut.cz
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 *
 * Support for Kontron EB8245 onboard CAN using 
 * memory mapped SJA1000 controller
 */

int eb8245_request_io(struct candevice_t *candev);
int eb8245_release_io(struct candevice_t *candev);
int eb8245_reset(struct candevice_t *candev); 
int eb8245_init_hw_data(struct candevice_t *candev);
int eb8245_init_chip_data(struct candevice_t *candev, int chipnr);
int eb8245_init_obj_data(struct canchip_t *chip, int objnr);
int eb8245_program_irq(struct candevice_t *candev);
void eb8245_write_register(unsigned data, can_ioptr_t address);
unsigned eb8245_read_register(can_ioptr_t address);
