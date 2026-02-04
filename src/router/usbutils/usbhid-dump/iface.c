// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * usbhid-dump - interface
 *
 * Copyright (C) 2010 Nikolai Kondrashov <spbnick@gmail.com>
 */

#include "iface.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

bool
uhd_iface_valid(const uhd_iface *iface)
{
    return iface != NULL &&
           uhd_dev_valid(iface->dev) &&
           iface->number < UINT8_MAX &&
           strlen(iface->addr_str) == (sizeof(iface->addr_str) - 1);
}

uhd_iface *
uhd_iface_new(const uhd_dev    *dev,
              uint8_t           number,
              uint8_t           int_in_ep_addr,
              uint16_t          int_in_ep_maxp,
              uint16_t          rd_len)
{
    uhd_iface      *iface;
    libusb_device  *lusb_dev;
    int             rc;

    iface = malloc(sizeof(*iface));
    if (iface == NULL)
        return NULL;

    iface->next             = NULL;
    iface->dev              = dev;
    iface->number           = number;
    iface->int_in_ep_addr   = int_in_ep_addr;
    iface->int_in_ep_maxp   = int_in_ep_maxp;
    iface->rd_len           = rd_len;
    iface->detached         = false;
    iface->claimed          = false;
    iface->submitted        = false;

    /* Format address string */
    lusb_dev = libusb_get_device(dev->handle);
    rc = snprintf(iface->addr_str, sizeof(iface->addr_str),
                  "%.3hhu:%.3hhu:%.3hhu",
                  libusb_get_bus_number(lusb_dev),
                  libusb_get_device_address(lusb_dev),
                  number);
    (void)rc;
    assert(rc == (sizeof(iface->addr_str) - 1));

    assert(uhd_iface_valid(iface));

    return iface;
}


void
uhd_iface_free(uhd_iface *iface)
{
    if (iface == NULL)
        return;

    assert(uhd_iface_valid(iface));

    free(iface);
}


enum libusb_error
uhd_iface_detach(uhd_iface *iface)
{
    enum libusb_error   err;

    assert(uhd_iface_valid(iface));

    err = libusb_detach_kernel_driver(iface->dev->handle, iface->number);
    if (err == LIBUSB_SUCCESS)
        iface->detached = true;
    else if (err != LIBUSB_ERROR_NOT_FOUND)
        return err;

    return LIBUSB_SUCCESS;
}


enum libusb_error
uhd_iface_attach(uhd_iface *iface)
{
    enum libusb_error   err;

    assert(uhd_iface_valid(iface));

    if (iface->detached)
    {
        err = libusb_attach_kernel_driver(iface->dev->handle,
                                          iface->number);
        if (err != LIBUSB_SUCCESS)
            return err;
        iface->detached = false;
    }

    return LIBUSB_SUCCESS;
}


enum libusb_error
uhd_iface_claim(uhd_iface *iface)
{
    enum libusb_error   err;

    assert(uhd_iface_valid(iface));

    err = libusb_claim_interface(iface->dev->handle, iface->number);
    if (err != LIBUSB_SUCCESS)
        return err;

    iface->claimed = true;

    return LIBUSB_SUCCESS;
}


enum libusb_error
uhd_iface_release(uhd_iface *iface)
{
    enum libusb_error   err;

    assert(uhd_iface_valid(iface));

    err = libusb_release_interface(iface->dev->handle, iface->number);
    if (err != LIBUSB_SUCCESS)
        return err;

    iface->claimed = false;

    return LIBUSB_SUCCESS;
}


enum libusb_error
uhd_iface_clear_halt(uhd_iface *iface)
{
    enum libusb_error   err;

    assert(uhd_iface_valid(iface));

    err = libusb_clear_halt(iface->dev->handle, iface->int_in_ep_addr);
    if (err != LIBUSB_SUCCESS)
        return err;

    return LIBUSB_SUCCESS;
}


enum libusb_error
uhd_iface_set_idle(const uhd_iface    *iface,
                   uint8_t             duration,
                   unsigned int        timeout)
{
    int rc;

    assert(uhd_iface_valid(iface));

    rc = libusb_control_transfer(iface->dev->handle,
                                 /* host->device, class, interface */
                                 0x21,
                                 /* Set_Idle */
                                 0x0A,
                                 /* duration for all report IDs */
                                 duration << 8,
                                 /* interface */
                                 iface->number,
                                 NULL, 0,
                                 timeout);
    /*
     * Ignoring EPIPE, which means a STALL handshake, which is OK on
     * control pipes and indicates request is not supported.
     * See USB 2.0 spec, 8.4.5 Handshake Packets
     */
    if (rc < 0 && rc != LIBUSB_ERROR_PIPE)
        return rc;

    return LIBUSB_SUCCESS;
}


enum libusb_error
uhd_iface_set_protocol(const uhd_iface    *iface,
                       bool                report,
                       unsigned int        timeout)
{
    int rc;

    assert(uhd_iface_valid(iface));

    rc = libusb_control_transfer(iface->dev->handle,
                                 /* host->device, class, interface */
                                 0x21,
                                 /* Set_Protocol */
                                 0x0B,
                                 /* 0 - boot, 1 - report */
                                 report ? 1 : 0,
                                 /* interface */
                                 iface->number,
                                 NULL, 0,
                                 timeout);
    /*
     * Ignoring EPIPE, which means a STALL handshake, which is OK on
     * control pipes and indicates request is not supported.
     * See USB 2.0 spec, 8.4.5 Handshake Packets
     */
    if (rc < 0 && rc != LIBUSB_ERROR_PIPE)
        return rc;

    return LIBUSB_SUCCESS;
}


