// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * usbhid-dump - interface list
 *
 * Copyright (C) 2010 Nikolai Kondrashov <spbnick@gmail.com>
 */

#include "iface_list.h"
#include "misc.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

bool
uhd_iface_list_valid(const uhd_iface *list)
{
    UHD_IFACE_LIST_FOR_EACH(list, list)
        if (!uhd_iface_valid(list))
            return false;

    return true;
}


size_t
uhd_iface_list_len(const uhd_iface *list)
{
    size_t  len = 0;

    UHD_IFACE_LIST_FOR_EACH(list, list)
        len++;

    return len;
}


void
uhd_iface_list_free(uhd_iface *list)
{
    uhd_iface *next;

    for (; list != NULL; list = next)
    {
        next = list->next;
        uhd_iface_free(list);
    }
}


enum libusb_error
uhd_iface_list_new(uhd_dev     *dev_list,
                   uhd_iface  **plist)
{
    enum libusb_error   err = LIBUSB_ERROR_OTHER;

    uhd_dev                                    *dev;
    struct libusb_config_descriptor            *config          = NULL;
    const struct libusb_interface              *lusb_iface;
    const struct libusb_interface_descriptor   *iface_desc;
    const uhd_hid_descriptor                   *hid_desc;
    const uhd_hid_descriptor_extra             *hid_desc_extra;
    uint16_t                                    rd_len;
    const struct libusb_endpoint_descriptor    *ep_list;
    uint8_t                                     ep_num;
    const struct libusb_endpoint_descriptor    *ep;
    uhd_iface                                  *list            = NULL;
    uhd_iface                                  *iface;

    assert(uhd_dev_list_valid(dev_list));

    UHD_DEV_LIST_FOR_EACH(dev, dev_list)
    {
        /* Retrieve active configuration descriptor */
        err = libusb_get_active_config_descriptor(libusb_get_device(dev->handle),
                                                  &config);
        if (err != LIBUSB_SUCCESS)
            goto cleanup;

        /*
         * Build the matching interface list
         */

        /* For each interface */
        for (lusb_iface = config->interface;
             lusb_iface - config->interface < config->bNumInterfaces;
             lusb_iface++)
        {
            /* Skip interfaces with altsettings */
            if (lusb_iface->num_altsetting != 1)
                continue;

            iface_desc = lusb_iface->altsetting;

            /* Skip non-HID interfaces */
            if (iface_desc->bInterfaceClass != LIBUSB_CLASS_HID)
                continue;

            /*
             * Try to retrieve report descriptor length
             */
            rd_len = UHD_MAX_DESCRIPTOR_SIZE;
            /* If interface descriptor has space for a HID descriptor */
            if (iface_desc->extra_length >= (int)sizeof(uhd_hid_descriptor))
            {
                hid_desc = (const uhd_hid_descriptor *)iface_desc->extra;
                /* If this is truly a HID class descriptor */
                if (hid_desc->bDescriptorType == LIBUSB_DT_HID)
                {
                    /* For each extra HID descriptor entry */
                    for (hid_desc_extra = hid_desc->extra;
                         hid_desc_extra <
                            hid_desc->extra + hid_desc->bNumDescriptors &&
                         (uint8_t *)hid_desc_extra <
                            (uint8_t *)hid_desc + hid_desc->bLength &&
                         (unsigned char *)hid_desc_extra <
                            iface_desc->extra +
                            iface_desc->extra_length;
                         hid_desc_extra++) {
                        /* If this is a report descriptor entry */
                        if (hid_desc_extra->bDescriptorType ==
                                LIBUSB_DT_REPORT)
                        {
                            rd_len = hid_desc_extra->wDescriptorLength;
                            break;
                        }
                    }
                }
            }

            /* Retrieve endpoint list */
            ep_list = iface_desc->endpoint;
            ep_num = iface_desc->bNumEndpoints;

            /* For each endpoint */
            for (ep = ep_list; (ep - ep_list) < ep_num; ep++)
            {
                /* Skip non-interrupt and non-in endpoints */
                if ((ep->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) !=
                    LIBUSB_TRANSFER_TYPE_INTERRUPT ||
                    (ep->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) !=
                    LIBUSB_ENDPOINT_IN)
                    continue;

                /* Create the interface */
                iface = uhd_iface_new(
                            dev,
                            iface_desc->bInterfaceNumber,
                            ep->bEndpointAddress, ep->wMaxPacketSize,
                            rd_len);
                if (iface == NULL)
                {
                    err = LIBUSB_ERROR_NO_MEM;
                    goto cleanup;
                }

                /* Add the interface */
                iface->next = list;
                list = iface;

                break;
            }
        }

        /* Free the config descriptor */
        libusb_free_config_descriptor(config);
        config = NULL;
    }

    /* Output the resulting list, if requested */
    assert(uhd_iface_list_valid(list));
    if (plist != NULL)
    {
        *plist = list;
        list = NULL;
    }

    /* Done! */
    err = LIBUSB_SUCCESS;

cleanup:

    libusb_free_config_descriptor(config);
    uhd_iface_list_free(list);

    return err;
}


uhd_iface *
uhd_iface_list_fltr_by_num(uhd_iface   *list,
                           uint8_t      number)
{
    uhd_iface  *prev;
    uhd_iface  *iface;
    uhd_iface  *next;

    assert(uhd_iface_list_valid(list));
    assert(number < UINT8_MAX);

    for (prev = NULL, iface = list; iface != NULL; iface = next)
    {
        next = iface->next;
        if (iface->number == number)
            prev = iface;
        else
        {
            if (prev == NULL)
                list = next;
            else
                prev->next = next;
            uhd_iface_free(iface);
        }
    }

    return list;
}


