/* select.h
 * Header file for the Linux CAN-bus driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Added by Pavel Pisa pisa@cmp.felk.cvut.cz
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

unsigned int can_poll(struct file *file, poll_table *wait);
