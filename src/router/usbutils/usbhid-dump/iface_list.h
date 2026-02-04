// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * usbhid-dump - interface list
 *
 * Copyright (C) 2010 Nikolai Kondrashov <spbnick@gmail.com>
 */

#ifndef __UHD_IFACE_LIST_H__
#define __UHD_IFACE_LIST_H__

#include <stdint.h>
#include "dev_list.h"
#include "iface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Check if an interface list is valid.
 *
 * @param list  Interface list to check.
 *
 * @return True if the interface list is valid, false otherwise.
 */
extern bool uhd_iface_list_valid(const uhd_iface *list);

/**
 * Check if an interface list is empty.
 *
 * @param list  Interface list to check.
 *
 * @return True if the interface list is empty, false otherwise.
 */
static inline bool
uhd_iface_list_empty(const uhd_iface *list)
{
    return list == NULL;
}

/**
 * Calculate length of an interface list.
 *
 * @param list  The list to calculate length of.
 *
 * @return The list length.
 */
extern size_t uhd_iface_list_len(const uhd_iface *list);

/**
 * Free an interface list.
 *
 * @param list  The interface list to free.
 */
extern void uhd_iface_list_free(uhd_iface *list);

/**
 * Iterate over an interface list.
 *
 * @param _iface    Loop interface variable.
 * @param _list     Interface list to iterate over.
 */
#define UHD_IFACE_LIST_FOR_EACH(_iface, _list) \
    for (_iface = _list; _iface != NULL; _iface = _iface->next)

/**
 * Fetch a list of HID interfaces from a device list.
 *
 * @param dev_list      The device list to fetch interface list from.
 * @param plist         Location for the resulting list head; could be NULL.
 *
 * @return Libusb error code.
 */
extern enum libusb_error uhd_iface_list_new(uhd_dev    *dev_list,
                                            uhd_iface **plist);

/**
 * Filter an interface list by an interface number.
 *
 * @param plist     The original list head.
 * @param number    The interface number to match against.
 *
 * @return The resulting list head.
 */
extern uhd_iface *uhd_iface_list_fltr_by_num(uhd_iface *list,
                                             uint8_t    number);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __UHD_IFACE_LIST_H__ */
