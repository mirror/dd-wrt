/* nutscan-usb.h - header with USB identifiers known to NUT drivers
 * This file was automatically generated during NUT build by 'tools/nut-usbinfo.pl'
 *
 *  Copyright (C) 2011 - Arnaud Quette <arnaud.quette@free.fr>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef DEVSCAN_USB_H
#define DEVSCAN_USB_H

#include "nut_stdint.h"	/* for uint16_t etc. */

#include <limits.h>	/* for PATH_MAX in usb.h etc. */

#include <sys/param.h>	/* for MAXPATHLEN etc. */

/* libusb header file */
#if (!WITH_LIBUSB_1_0) && (!WITH_LIBUSB_0_1)
#error "configure script error: Neither WITH_LIBUSB_1_0 nor WITH_LIBUSB_0_1 is set"
#endif

#if (WITH_LIBUSB_1_0) && (WITH_LIBUSB_0_1)
#error "configure script error: Both WITH_LIBUSB_1_0 and WITH_LIBUSB_0_1 are set"
#endif

#if WITH_LIBUSB_1_0
# include <libusb.h>
#endif
#if WITH_LIBUSB_0_1
# ifdef HAVE_USB_H
#  include <usb.h>
# else
#  ifdef HAVE_LUSB0_USB_H
#   include <lusb0_usb.h>
#  else
#   error "configure script error: Neither HAVE_USB_H nor HAVE_LUSB0_USB_H is set for the WITH_LIBUSB_0_1 build"
#  endif
# endif
 /* simple remap to avoid bloating structures */
 typedef usb_dev_handle libusb_device_handle;
#endif
typedef struct {
	uint16_t	vendorID;
	uint16_t	productID;
	char*	driver_name;
	char*	alt_driver_names;
} usb_device_id_t;

/* USB IDs device table */
static usb_device_id_t usb_device_table[] = {

	{ 0x0001, 0x0000, "nutdrv_qx", "blazer_usb nutdrv_atcl_usb" },
	{ 0x03f0, 0x0001, "usbhid-ups", NULL },
	{ 0x03f0, 0x1f01, "bcmxcp_usb", NULL },
	{ 0x03f0, 0x1f02, "bcmxcp_usb", NULL },
	{ 0x03f0, 0x1f06, "usbhid-ups", NULL },
	{ 0x03f0, 0x1f08, "usbhid-ups", NULL },
	{ 0x03f0, 0x1f09, "usbhid-ups", NULL },
	{ 0x03f0, 0x1f0a, "usbhid-ups", NULL },
	{ 0x03f0, 0x1fe0, "usbhid-ups", NULL },
	{ 0x03f0, 0x1fe1, "usbhid-ups", NULL },
	{ 0x03f0, 0x1fe2, "usbhid-ups", NULL },
	{ 0x03f0, 0x1fe3, "usbhid-ups", NULL },
	{ 0x03f0, 0x1fe5, "usbhid-ups", NULL },
	{ 0x03f0, 0x1fe6, "usbhid-ups", NULL },
	{ 0x03f0, 0x1fe7, "usbhid-ups", NULL },
	{ 0x03f0, 0x1fe8, "usbhid-ups", NULL },
	{ 0x0463, 0x0001, "usbhid-ups", NULL },
	{ 0x0463, 0xffff, "usbhid-ups", NULL },
	{ 0x047c, 0xffff, "usbhid-ups", NULL },
	{ 0x0483, 0x0035, "nutdrv_qx", NULL },
	{ 0x0483, 0xa113, "usbhid-ups", NULL },
	{ 0x0483, 0xa430, "usbhid-ups", NULL },
	{ 0x04b3, 0x0001, "usbhid-ups", NULL },
	{ 0x04b4, 0x5500, "riello_usb", NULL },
	{ 0x04d8, 0xd004, "usbhid-ups", NULL },
	{ 0x04d8, 0xd005, "usbhid-ups", NULL },
	{ 0x050d, 0x0375, "usbhid-ups", NULL },
	{ 0x050d, 0x0551, "usbhid-ups", NULL },
	{ 0x050d, 0x0750, "usbhid-ups", NULL },
	{ 0x050d, 0x0751, "usbhid-ups", NULL },
	{ 0x050d, 0x0900, "usbhid-ups", NULL },
	{ 0x050d, 0x0910, "usbhid-ups", NULL },
	{ 0x050d, 0x0912, "usbhid-ups", NULL },
	{ 0x050d, 0x0980, "usbhid-ups", NULL },
	{ 0x050d, 0x0f51, "usbhid-ups", NULL },
	{ 0x050d, 0x1100, "usbhid-ups", NULL },
	{ 0x051d, 0x0000, "usbhid-ups", NULL },
	{ 0x051d, 0x0002, "usbhid-ups", NULL },
	{ 0x051d, 0x0003, "usbhid-ups", "apc_modbus" },
	{ 0x051d, 0x0004, "usbhid-ups", NULL },
	{ 0x0592, 0x0002, "bcmxcp_usb", NULL },
	{ 0x0592, 0x0004, "usbhid-ups", NULL },
	{ 0x05b8, 0x0000, "nutdrv_qx", "blazer_usb" },
	{ 0x05dd, 0x041b, "usbhid-ups", NULL },
	{ 0x05dd, 0xa011, "usbhid-ups", NULL },
	{ 0x05dd, 0xa0a0, "usbhid-ups", NULL },
	{ 0x0665, 0x5161, "nutdrv_qx", "blazer_usb" },
	{ 0x06da, 0x0002, "nutdrv_qx", "bcmxcp_usb blazer_usb" },
	{ 0x06da, 0x0003, "nutdrv_qx", "blazer_usb" },
	{ 0x06da, 0x0004, "nutdrv_qx", "blazer_usb" },
	{ 0x06da, 0x0005, "nutdrv_qx", "blazer_usb" },
	{ 0x06da, 0x0201, "nutdrv_qx", "blazer_usb" },
	{ 0x06da, 0x0601, "nutdrv_qx", "blazer_usb" },
	{ 0x06da, 0xffff, "usbhid-ups", "usbhid-ups" },
	{ 0x075d, 0x0300, "usbhid-ups", NULL },
	{ 0x0764, 0x0005, "usbhid-ups", NULL },
	{ 0x0764, 0x0501, "usbhid-ups", NULL },
	{ 0x0764, 0x0601, "usbhid-ups", NULL },
	{ 0x0925, 0x1234, "nutdrv_qx", "richcomm_usb" },
	{ 0x09ae, 0x0001, "tripplite_usb", NULL },
	{ 0x09ae, 0x1003, "usbhid-ups", NULL },
	{ 0x09ae, 0x1007, "usbhid-ups", NULL },
	{ 0x09ae, 0x1008, "usbhid-ups", NULL },
	{ 0x09ae, 0x1009, "usbhid-ups", NULL },
	{ 0x09ae, 0x1010, "usbhid-ups", NULL },
	{ 0x09ae, 0x1330, "usbhid-ups", NULL },
	{ 0x09ae, 0x2005, "usbhid-ups", NULL },
	{ 0x09ae, 0x2007, "usbhid-ups", NULL },
	{ 0x09ae, 0x2008, "usbhid-ups", NULL },
	{ 0x09ae, 0x2009, "usbhid-ups", NULL },
	{ 0x09ae, 0x2010, "usbhid-ups", NULL },
	{ 0x09ae, 0x2011, "usbhid-ups", NULL },
	{ 0x09ae, 0x2012, "usbhid-ups", NULL },
	{ 0x09ae, 0x2013, "usbhid-ups", NULL },
	{ 0x09ae, 0x2014, "usbhid-ups", NULL },
	{ 0x09ae, 0x3008, "usbhid-ups", NULL },
	{ 0x09ae, 0x3009, "usbhid-ups", NULL },
	{ 0x09ae, 0x3010, "usbhid-ups", NULL },
	{ 0x09ae, 0x3011, "usbhid-ups", NULL },
	{ 0x09ae, 0x3012, "usbhid-ups", NULL },
	{ 0x09ae, 0x3013, "usbhid-ups", NULL },
	{ 0x09ae, 0x3014, "usbhid-ups", NULL },
	{ 0x09ae, 0x3015, "usbhid-ups", NULL },
	{ 0x09ae, 0x3016, "usbhid-ups", NULL },
	{ 0x09ae, 0x3024, "usbhid-ups", NULL },
	{ 0x09ae, 0x4001, "usbhid-ups", NULL },
	{ 0x09ae, 0x4002, "usbhid-ups", NULL },
	{ 0x09ae, 0x4003, "usbhid-ups", NULL },
	{ 0x09ae, 0x4004, "usbhid-ups", NULL },
	{ 0x09ae, 0x4005, "usbhid-ups", NULL },
	{ 0x09ae, 0x4006, "usbhid-ups", NULL },
	{ 0x09ae, 0x4007, "usbhid-ups", NULL },
	{ 0x09ae, 0x4008, "usbhid-ups", NULL },
	{ 0x0d9f, 0x0001, "usbhid-ups", NULL },
	{ 0x0d9f, 0x0004, "usbhid-ups", NULL },
	{ 0x0d9f, 0x00a2, "usbhid-ups", NULL },
	{ 0x0d9f, 0x00a3, "usbhid-ups", NULL },
	{ 0x0d9f, 0x00a4, "usbhid-ups", NULL },
	{ 0x0d9f, 0x00a5, "usbhid-ups", NULL },
	{ 0x0d9f, 0x00a6, "usbhid-ups", NULL },
	{ 0x0f03, 0x0001, "nutdrv_qx", "blazer_usb" },
	{ 0x10af, 0x0000, "usbhid-ups", NULL },
	{ 0x10af, 0x0001, "usbhid-ups", NULL },
	{ 0x10af, 0x0002, "usbhid-ups", NULL },
	{ 0x10af, 0x0004, "usbhid-ups", NULL },
	{ 0x10af, 0x0008, "usbhid-ups", NULL },
	{ 0x14f0, 0x00c9, "nutdrv_qx", "blazer_usb" },
	{ 0x1cb0, 0x0032, "usbhid-ups", NULL },
	{ 0x1cb0, 0x0035, "nutdrv_qx", NULL },
	{ 0x1cb0, 0x0038, "usbhid-ups", NULL },
	{ 0x2341, 0x0036, "usbhid-ups", NULL },
	{ 0x2341, 0x8036, "usbhid-ups", NULL },
	{ 0x2a03, 0x0036, "usbhid-ups", NULL },
	{ 0x2a03, 0x0040, "usbhid-ups", NULL },
	{ 0x2a03, 0x8036, "usbhid-ups", NULL },
	{ 0x2a03, 0x8040, "usbhid-ups", NULL },
	{ 0x2b2d, 0xffff, "usbhid-ups", NULL },
	{ 0x2e51, 0x0000, "usbhid-ups", NULL },
	{ 0x2e51, 0xffff, "usbhid-ups", NULL },
	{ 0x2e66, 0x0101, "usbhid-ups", NULL },
	{ 0x2e66, 0x0201, "usbhid-ups", NULL },
	{ 0x2e66, 0x0202, "usbhid-ups", NULL },
	{ 0x2e66, 0x0203, "usbhid-ups", NULL },
	{ 0x2e66, 0x0300, "usbhid-ups", NULL },
	{ 0x2e66, 0x0302, "usbhid-ups", NULL },
	{ 0x4234, 0x0002, "usbhid-ups", NULL },
	{ 0xffff, 0x0000, "nutdrv_qx", "blazer_usb" },

	/* Terminating entry */
	{ 0, 0, NULL, NULL }
};
#endif /* DEVSCAN_USB_H */

