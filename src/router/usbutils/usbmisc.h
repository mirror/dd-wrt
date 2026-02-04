// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Misc USB routines
 *
 * Copyright (C) 2003 Aurelien Jarno (aurelien@aurel32.net)
 */

#ifndef _USBMISC_H
#define _USBMISC_H

#include <libusb.h>

/* ---------------------------------------------------------------------- */

extern libusb_device *get_usb_device(libusb_context *ctx, const char *path);

extern char *get_dev_string(libusb_device_handle *dev, uint8_t id);

/* ---------------------------------------------------------------------- */
#endif /* _USBMISC_H */
