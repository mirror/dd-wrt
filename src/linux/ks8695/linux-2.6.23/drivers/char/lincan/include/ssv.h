/* ssv.h
 * Header file for the Linux CAN-bus driver.
 * Written by Arnaud Westenberg email:arnaud@casema.net
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

int ssv_request_io(struct candevice_t *candev);
int ssv_release_io(struct candevice_t *candev);
int ssv_reset(struct candevice_t *candev);
int ssv_init_hw_data(struct candevice_t *candev);
int ssv_init_chip_data(struct candevice_t *candev, int chipnr);
int ssv_init_obj_data(struct canchip_t *chip, int objnr);
void ssv_write_register(unsigned data, can_ioptr_t address);
unsigned ssv_read_register(can_ioptr_t address);
int ssv_program_irq(struct candevice_t *candev);

