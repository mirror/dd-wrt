/* devcommon.h - common device code
 * Linux CAN-bus device driver.
 * New CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#include "./canmsg.h"
#include "./can_sysdep.h"
#include "./constants.h"
#include "./can_queue.h"

int canqueue_ends_init_chip(struct canque_ends_t *qends, struct canchip_t *chip, struct msgobj_t *obj);
int canqueue_ends_done_chip(struct canque_ends_t *qends);
