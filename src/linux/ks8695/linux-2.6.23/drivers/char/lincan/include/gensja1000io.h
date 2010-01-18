/* gensja1000io.h
 * Header file for the Linux CAN-bus driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Added by Pavel Pisa pisa@cmp.felk.cvut.cz
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 *
 * Support for PiKRON Ltd ISA CAN card using 
 * memory mapped SJA1000 controller
 */

int gensja1000io_request_io(struct candevice_t *candev);
int gensja1000io_release_io(struct candevice_t *candev);
int gensja1000io_reset(struct candevice_t *candev); 
int gensja1000io_init_hw_data(struct candevice_t *candev);
int gensja1000io_init_chip_data(struct candevice_t *candev, int chipnr);
int gensja1000io_init_obj_data(struct canchip_t *chip, int objnr);
int gensja1000io_program_irq(struct candevice_t *candev);
void gensja1000io_write_register(unsigned data, can_ioptr_t address);
unsigned gensja1000io_read_register(can_ioptr_t address);

