/*
 * Sierra Wireless CDMA Wireless Serial USB drive
 * 
 * Current Copy modified by: Kevin Lloyd <linux@sierrawireless.com>
 * Original Copy written by: 2005 Greg Kroah-Hartman <gregkh <at> suse.de>
 *
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License version
 *	2 as published by the Free Software Foundation.
 *
 * Version history:
  Version 1.03 (Lloyd):
  Included support for DTR control and enhanced buffering (should help
  speed).
 */

#define USB_VENDER_REQUEST_SET_DEVICE_POWER_STATE 0

#define USB_DEVICE_POWER_STATE_D0       0x0000
#define USB_DEVICE_POWER_STATE_D1       0x0001
#define USB_DEVICE_POWER_STATE_D2       0x0002
#define USB_DEVICE_POWER_STATE_D3       0x0003

#define SET_CONTROL_LINE_STATE          0x22
/*
 * Output control lines.
 */

#define ACM_CTRL_DTR            0x01
#define ACM_CTRL_RTS            0x02

