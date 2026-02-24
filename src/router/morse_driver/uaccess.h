#ifndef _MORSE_UACCESS_H_
#define _MORSE_UACCESS_H_

/*
 * Copyright 2017-2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include <linux/cdev.h>

#define UACCESS_IOC_MAGIC	'k'
#define UACCESS_IOC_MAXNR	1
#define UACCESS_IOC_SET_ADDRESS	_IO(UACCESS_IOC_MAGIC, 1)

struct uaccess {
	struct class *drv_class;
};

struct uaccess_device {
	struct cdev cdev;
	struct device *device;
	struct uaccess *uaccess;
	struct morse *mors;
};

struct uaccess *uaccess_alloc(void);
int uaccess_init(struct uaccess *uaccess);
void uaccess_cleanup(struct uaccess *uaccess);

int uaccess_device_register(struct morse *mors, struct uaccess *uaccess, struct device *parent);
void uaccess_device_unregister(struct morse *mors);

#endif /* !_MORSE_UACCESS_H_ */
