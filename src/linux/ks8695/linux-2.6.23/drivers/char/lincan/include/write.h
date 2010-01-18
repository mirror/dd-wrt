/* write.h
 * Header file for the Linux CAN-bus driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

ssize_t can_write(struct file *file, const char *buffer, size_t length, loff_t *offset);
