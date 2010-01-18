/* can_iortl.h - RT-Linux Posix file IO interface
 * Linux CAN-bus device driver.
 * RT-Linux support by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#ifndef _CAN_IORTL_H
#define _CAN_IORTL_H

#ifdef CAN_WITH_RTL

int can_open_rtl_posix(struct rtl_file *fptr);

int can_release_rtl_posix(struct rtl_file *fptr);

ssize_t can_read_rtl_posix(struct rtl_file *fptr, char *buffer,
				size_t length, loff_t *ppos);

ssize_t can_write_rtl_posix(struct rtl_file *fptr, const char *buffer,
				 size_t length, loff_t *ppos);

int can_ioctl_rtl_posix(struct rtl_file *, unsigned int, unsigned long);

#endif /*CAN_WITH_RTL*/

#endif /*_CAN_IORTL_H*/
