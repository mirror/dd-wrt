// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * usbhid-dump - device list
 *
 * Copyright (C) 2010-2011 Nikolai Kondrashov <spbnick@gmail.com>
 */

#ifndef __UHD_DEV_LIST_H__
#define __UHD_DEV_LIST_H__

#include <stddef.h>
#include <stdint.h>
#include "dev.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Check if a device list is valid.
 *
 * @param list  Device list to check.
 *
 * @return True if the device list is valid, false otherwise.
 */
extern bool uhd_dev_list_valid(const uhd_dev *list);

/**
 * Close every device in a device list.
 *
 * @param list  The device list to close.
 */
extern void uhd_dev_list_close(uhd_dev *list);

/**
 * Iterate over a device list.
 *
 * @param _dev    Loop device variable.
 * @param _list     Device list to iterate over.
 */
#define UHD_DEV_LIST_FOR_EACH(_dev, _list) \
    for (_dev = _list; _dev != NULL; _dev = _dev->next)

/**
 * Open a list of devices optionally matching bus number/device address and
 * vendor/product IDs.
 *
 * @param ctx       Libusb context.
 * @param bus_num   Bus number, or 0 for any bus.
 * @param dev_addr  Device address, or 0 for any address.
 * @param vid       Vendor ID, or 0 for any vendor.
 * @param pid       Product ID, or 0 for any product.
 * @param plist     Location for the resulting device list head; could be
 *                  NULL.
 *
 * @return Libusb error code.
 */
extern enum libusb_error uhd_dev_list_open(libusb_context  *ctx,
                                           uint8_t          bus_num,
                                           uint8_t          dev_addr,
                                           uint16_t         vid,
                                           uint16_t         pid,
                                           uhd_dev        **plist);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __UHD_DEV_LIST_H__ */
