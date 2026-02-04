// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * usbhid-dump - interface
 *
 * Copyright (C) 2010 Nikolai Kondrashov <spbnick@gmail.com>
 */

#ifndef __UHD_IFACE_H__
#define __UHD_IFACE_H__

#include <stdint.h>
#include "dev.h"

#ifdef __cplusplus
extern "C" {
#endif

/** usbhid-dump interface */
typedef struct uhd_iface uhd_iface;

struct uhd_iface {
    uhd_iface              *next;
    const uhd_dev          *dev;            /**< Device */
    uint8_t                 number;         /**< Interface number */
    char                    addr_str[12];   /**< Address string */
    uint8_t                 int_in_ep_addr; /**< Interrupt IN EP address */
    uint16_t                int_in_ep_maxp; /**< Interrupt IN EP maximum
                                                 packet size */
    uint16_t                rd_len;         /**< Report descriptor length */
    bool                    detached;       /**< True if the interface was
                                                 detached from the kernel
                                                 driver, false otherwise */
    bool                    claimed;        /**< True if the interface was
                                                 claimed */
    /*
     * This is somewhat hackish and doesn't belong here, since theoretically
     * there could be more than one transfer submitted for an interface.
     * However, we don't do it yet. This flag is used to track transfer
     * cancellation during stream dumping.
     */
    bool                    submitted;      /**< True if an asynchronous
                                                 transfer has been submitted
                                                 for the interface */
};

/**
 * Check if an interface is valid.
 *
 * @param iface Interface.
 *
 * @return True if the interface is valid, false otherwise.
 */
extern bool uhd_iface_valid(const uhd_iface *iface);

/**
 * Create a new interface.
 *
 * @param handle            Device handle.
 * @param number            Interface number.
 * @param int_in_ep_addr    Interrupt in endpoint address.
 * @param int_in_ep_maxp    Interrupt in endpoint maximum packet size.
 * @param rd_len            Report descriptor length.
 *
 * @return New interface or NULL, if failed to allocate.
 */
extern uhd_iface *uhd_iface_new(const uhd_dev  *dev,
                                uint8_t         number,
                                uint8_t         int_in_ep_addr,
                                uint16_t        int_in_ep_maxp,
                                uint16_t        rd_len);

/**
 * Free an interface.
 *
 * @param iface The interface to free, could be NULL.
 */
extern void uhd_iface_free(uhd_iface *iface);

/**
 * Detach an interface from its kernel driver (if any).
 *
 * @param iface The interface to detach.
 *
 * @return Libusb error code.
 */
extern enum libusb_error uhd_iface_detach(uhd_iface *iface);

/**
 * Attach an interface to its kernel driver (if detached before).
 *
 * @param iface The interface to attach.
 *
 * @return Libusb error code.
 */
extern enum libusb_error uhd_iface_attach(uhd_iface *iface);

/**
 * Claim an interface.
 *
 * @param iface The interface to claim.
 *
 * @return Libusb error code.
 */
extern enum libusb_error uhd_iface_claim(uhd_iface *iface);

/**
 * Set idle duration on an interface; ignore errors indicating missing
 * support.
 *
 * @param iface     The interface to set idle duration on.
 * @param duration  The duration in 4 ms steps starting from 4 ms.
 * @param timeout   The request timeout, ms.
 *
 * @return Libusb error code.
 */
extern enum libusb_error uhd_iface_set_idle(
                                       const uhd_iface    *iface,
                                       uint8_t             duration,
                                       unsigned int        timeout);

/**
 * Set HID protocol on an interface; ignore errors indicating missing
 * support.
 *
 * @param iface     The interface to set idle duration on.
 * @param report    True for "report" protocol, false for "boot" protocol.
 * @param timeout   The request timeout, ms.
 *
 * @return Libusb error code.
 */
extern enum libusb_error uhd_iface_set_protocol(
                                       const uhd_iface    *iface,
                                       bool                report,
                                       unsigned int        timeout);

/**
 * Clear halt condition on the input interrupt endpoint of an interface.
 *
 * @param iface The interface to clear halt condition on.
 *
 * @return Libusb error code.
 */
extern enum libusb_error uhd_iface_clear_halt(uhd_iface *iface);

/**
 * Release an interface (if claimed before).
 *
 * @param iface The interface to release.
 *
 * @return Libusb error code.
 */
extern enum libusb_error uhd_iface_release(uhd_iface *iface);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __UHD_IFACE_H__ */
