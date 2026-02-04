// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * usbhid-dump - device
 *
 * Copyright (C) 2010-2011 Nikolai Kondrashov <spbnick@gmail.com>
 */

#ifndef __UHD_DEV_H__
#define __UHD_DEV_H__

#include <stdbool.h>
#include <libusb.h>

#ifdef __cplusplus
extern "C" {
#endif

/** usbhid-dump device */
typedef struct uhd_dev uhd_dev;

struct uhd_dev {
    uhd_dev                *next;       /**< Next device in the list */
    libusb_device_handle   *handle;     /**< Handle */
};

/**
 * Check if a device is valid.
 *
 * @param dev Device to check.
 *
 * @return True if the device is valid, false otherwise.
 */
extern bool uhd_dev_valid(const uhd_dev *dev);

/**
 * Open a device.
 *
 * @param lusb_dev  Libusb device.
 * @param pdev      Location for the opened device pointer.
 *
 * @return Libusb error code.
 */
extern enum libusb_error uhd_dev_open(libusb_device    *lusb_dev,
                                      uhd_dev         **pdev);

/**
 * Close a device.
 *
 * @param dev   The device to close.
 */
extern void uhd_dev_close(uhd_dev *dev);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __UHD_DEV_H__ */
