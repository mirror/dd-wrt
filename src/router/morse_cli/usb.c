/*
 * Copyright 2024 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <errno.h>
#include "utilities.h"
#include "usb.h"

static int usb_is_morse_dev(libusb_device *dev)
{
    int ret;
    struct libusb_device_descriptor desc;

    ret = libusb_get_device_descriptor(dev, &desc);

    if (ret < 0)
    {
        mctrl_err("Failed to get usb device descriptor: %d\n", ret);
    }
    else if (desc.idVendor == MORSE_ID_VENDOR)
    {
        ret = 1;
    }
    else
    {
        ret = 0;
    }

    return ret;
}

static int usb_ndr_reset_cmd(libusb_device_handle *handle)
{
    int ret;
    int size;
    unsigned char cmd_buf[MORSE_CMD_SIZE] = {0};

    cmd_buf[0] = MORSE_CMD_RESET;

    ret = libusb_bulk_transfer(handle, MORSE_BULK_OUT_EP,
        cmd_buf, MORSE_CMD_SIZE, &size, 0);

    if (ret < 0)
    {
        mctrl_err("Failed to send reset command: %d\n", ret);
    }

    return ret;
}

int usb_ndr_reset(void)
{
    int i;
    int ret;
    ssize_t cnt;
    int found_morse_dev = 0;
    libusb_device_handle *morse_usb_handle = NULL;
    libusb_device **devs;

    ret = libusb_init(NULL);

    if (ret < 0)
    {
        mctrl_err("Failed to init libusb: %d\n", ret);
        goto exit;
    }

    cnt = libusb_get_device_list(NULL, &devs);

    if (cnt < 0)
    {
        mctrl_err("Failed to get usb device list: %d\n", ret);
        goto exit;
    }

    for (i = 0; devs[i]; i++)
    {
        found_morse_dev = usb_is_morse_dev(devs[i]);

        if (found_morse_dev)
        {
            break;
        }
    }

    if (found_morse_dev < 0)
    {
        goto cleanup;
    }
    else if (found_morse_dev)
    {
        morse_usb_handle = libusb_open_device_with_vid_pid(NULL,
            MORSE_ID_VENDOR, MORSE_MM810X_PRODUCT_ID);

        if (morse_usb_handle == NULL)
        {
            mctrl_err("Failed to open device %04x:%04x\n",
                MORSE_ID_VENDOR, MORSE_MM810X_PRODUCT_ID);
            goto cleanup;
        }

        ret = libusb_claim_interface(morse_usb_handle, MORSE_INTF_NUM);

        if (ret < 0)
        {
            mctrl_err("Failed to claim interface %d: %d\n", MORSE_INTF_NUM, ret);
        }
        else
        {
            ret = usb_ndr_reset_cmd(morse_usb_handle);
        }
        libusb_close(morse_usb_handle);
    }
    else
    {
        ret = -ENODEV;
    }
cleanup:
    libusb_free_device_list(devs, 1);
exit:
    libusb_exit(NULL);

    return ret;
}
