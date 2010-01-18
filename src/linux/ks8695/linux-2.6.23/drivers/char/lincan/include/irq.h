/* irq.h
 * Header file for the Linux CAN-bus driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

int i82527_irq_handler(int irq, struct canchip_t *chip);
int sja1000_irq_handler(int irq, struct canchip_t *chip);
int sja1000p_irq_handler(int irq, struct canchip_t *chip);
int dummy_irq_handler(int irq, struct canchip_t *chip);
