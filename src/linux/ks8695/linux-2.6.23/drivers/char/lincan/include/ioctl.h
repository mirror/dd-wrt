/* ioctl.h
 * Header file for the Linux CAN-bus driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

int can_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
int can_ioctl_query(struct canuser_t *canuser, unsigned long what);
int can_ioctl_remote_read(struct canuser_t *canuser, struct canmsg_t *rtr_msg,
                          unsigned long rtr_id, int options);
