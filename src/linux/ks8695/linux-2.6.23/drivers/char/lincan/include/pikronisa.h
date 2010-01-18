/* pikronisa.h
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

int pikronisa_request_io(struct candevice_t *candev);
int pikronisa_release_io(struct candevice_t *candev);
int pikronisa_reset(struct candevice_t *candev); 
int pikronisa_init_hw_data(struct candevice_t *candev);
int pikronisa_init_chip_data(struct candevice_t *candev, int chipnr);
int pikronisa_init_obj_data(struct canchip_t *chip, int objnr);
int pikronisa_program_irq(struct candevice_t *candev);
void pikronisa_write_register(unsigned data, can_ioptr_t address);
unsigned pikronisa_read_register(can_ioptr_t address);

