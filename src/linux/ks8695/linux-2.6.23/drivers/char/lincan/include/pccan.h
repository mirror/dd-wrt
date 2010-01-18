/* pccan.h
 * Header file for the Linux CAN-bus driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

int pccanf_request_io(struct candevice_t *candev);
int pccanf_release_io(struct candevice_t *candev);
int pccand_request_io(struct candevice_t *candev);
int pccand_release_io(struct candevice_t *candev);
int pccanq_request_io(struct candevice_t *candev);
int pccanq_release_io(struct candevice_t *candev);
int pccanf_reset(struct candevice_t *candev);
int pccand_reset(struct candevice_t *candev);
int pccanq_reset(struct candevice_t *candev); 
int pccan_init_hw_data(struct candevice_t *candev);
int pccan_init_chip_data(struct candevice_t *candev, int chipnr);
int pccan_init_obj_data(struct canchip_t *chip, int objnr);
void pccan_write_register(unsigned data, can_ioptr_t address);
unsigned pccan_read_register(can_ioptr_t address);
int pccan_program_irq(struct candevice_t *candev);


