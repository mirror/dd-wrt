/* irq.c
 * Linux CAN-bus device driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/irq.h"


int dummy_irq_handler(int irq, struct canchip_t *chip) {
	CANMSG("dummy_irq_handler called irq %d \n", irq);
	return CANCHIP_IRQ_NONE;
}
