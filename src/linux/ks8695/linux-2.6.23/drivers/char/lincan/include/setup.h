/* setup.h
 * Header file for the Linux CAN-bus driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

int init_hw_struct(void);
int list_hw(void);
void *can_checked_malloc(size_t size);
int can_checked_free(void *address_p);
int can_del_mem_list(void);
int can_chip_setup_irq(struct canchip_t *chip);
void can_chip_free_irq(struct canchip_t *chip);
