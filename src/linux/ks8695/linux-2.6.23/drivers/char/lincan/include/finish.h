/* finish.h
 * Header file for the Linux CAN-bus driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

void msgobj_done(struct msgobj_t *obj);
void canchip_done(struct canchip_t *chip);
void candevice_done(struct candevice_t *candev);
void canhardware_done(struct canhardware_t *candev);
