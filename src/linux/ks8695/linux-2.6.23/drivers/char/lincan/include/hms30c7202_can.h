/* hms30c7202_can.h - Hynix HMS30c7202 ARM device specific code
 * Linux CAN-bus device driver.
 * Written by Sebastian Stolzenberg email:stolzi@sebastian-stolzenberg.de
 * Based on code from Arnaud Westenberg email:arnaud@wanadoo.nl
 * and Ake Hedman, eurosource, akhe@eurosource.se
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#ifndef __HMS30C7202_CAN__
# define __HMS30C7202_CAN__

int hms30c7202_init_hw_data(struct candevice_t *candev);
int hms30c7202_init_chip_data(struct candevice_t *candev, int chipnr);
int hms30c7202_request_io(struct candevice_t *candev);
int hms30c7202_release_io(struct candevice_t *candev);
int hms30c7202_reset(  struct candevice_t *candev);
void hms30c7202_write_register(unsigned data, can_ioptr_t address);
unsigned hms30c7202_read_register(can_ioptr_t address);



int hms30c7202_init_obj_data(struct canchip_t *chip, int objnr);
int hms30c7202_program_irq(struct candevice_t *candev);

#endif /* __HMS30C7202_CAN__ */
