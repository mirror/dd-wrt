// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * usbhid-dump - device
 *
 * Copyright (C) 2010 Nikolai Kondrashov <spbnick@gmail.com>
 */

#include "dev.h"
#include <assert.h>
#include <stdlib.h>

bool
uhd_dev_valid(const uhd_dev *dev)
{
    return dev != NULL &&
           dev->handle != NULL;
}


enum libusb_error
uhd_dev_open(libusb_device     *lusb_dev,
             uhd_dev          **pdev)
{
    enum libusb_error   err;
    uhd_dev            *dev;

    assert(lusb_dev != NULL);

    dev = malloc(sizeof(*dev));
    if (dev == NULL)
        return LIBUSB_ERROR_NO_MEM;

    dev->next       = NULL;

    err = libusb_open(lusb_dev, &dev->handle);
    if (err != LIBUSB_SUCCESS)
    {
        free(dev);
        return err;
    }

    assert(uhd_dev_valid(dev));

    if (pdev != NULL)
        *pdev = dev;
    else
    {
        libusb_close(dev->handle);
        free(dev);
    }

    return LIBUSB_SUCCESS;
}


void
uhd_dev_close(uhd_dev *dev)
{
    if (dev == NULL)
        return;

    assert(uhd_dev_valid(dev));

    libusb_close(dev->handle);
    dev->handle = NULL;

    free(dev);
}


