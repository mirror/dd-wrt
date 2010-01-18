/* ns_dev_can.h - FPGA version of C_CAN ARM device specific code
 * Linux CAN-bus device driver.
 * Written by Sebastian Stolzenberg email:stolzi@sebastian-stolzenberg.de
 * Based on code from Arnaud Westenberg email:arnaud@wanadoo.nl
 * and Ake Hedman, eurosource, akhe@eurosource.se
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * Ported to FS Forth-Systeme GmbH A9M9750DEVx development boards
 * email:nbryan@embebidos.com
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 * This port 19 May 2005
 *
 */

#ifndef __NS_DEV_CAN__
#define __NS_DEV_CAN__

int ns_dev_init_hw_data(struct candevice_t *candev);
int ns_dev_init_chip_data(struct candevice_t *candev, int chipnr);
int ns_dev_request_io(struct candevice_t *candev);
int ns_dev_release_io(struct candevice_t *candev);
int ns_dev_reset(struct candevice_t *candev);
void ns_dev_write_register(unsigned data, can_ioptr_t address);
unsigned ns_dev_read_register(can_ioptr_t address);

int ns_dev_init_obj_data(struct canchip_t *chip, int objnr);
int ns_dev_program_irq(struct candevice_t *candev);

#define NS9750_PERIPHERAL_BASE_ADDRESS    0xA0700000
#define NS9750_PERIPHERAL_MAP_SIZE        0x400
#define NS9750_SYSTEM_CONTROLLER_OFFSET   0x240

#define BUS_WIDTH_16BIT                   1
#define ACTIVE_LOW_CHIP_SELECT            (1<<7)

#define C_CAN_CLOCK_INPUT_FREQUENCY       20000000

#endif /* __NS_DEV_CAN__ */
