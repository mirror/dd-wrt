/*
  USB Driver for GSM modems

  Copyright (C) 2005  Matthias Urlichs <smurf@smurf.noris.de>

  This driver is free software; you can redistribute it and/or modify
  it under the terms of Version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  Portions copied from the Keyspan driver by Hugh Blemings <hugh@blemings.org>

  History: see the git log.

  Work sponsored by: Sigos GmbH, Germany <info@sigos.de>

  This driver exists because the "normal" serial driver doesn't work too well
  with GSM modems. Issues:
  - data loss -- one single Receive URB is not nearly enough
  - nonstandard flow (Option devices) control
  - controlling the baud rate doesn't make sense

  This driver is named "option" because the most common device it's
  used for is a PC-Card (with an internal OHCI-USB interface, behind
  which the GSM interface sits), made by Option Inc.

  Some of the "one port" devices actually exhibit multiple USB instances
  on the USB bus. This is not a bug, these ports are used for different
  device features.

  Backport to kernel 2.4.37 from 2.6.28.7, Leonid Lisovskiy <lly at sf.net>

*/

#define DRIVER_VERSION "v0.7.2a"
#define DRIVER_AUTHOR "Matthias Urlichs <smurf@smurf.noris.de>"
#define DRIVER_DESC "USB Driver for GSM modems"

#include <linux/config.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/usb.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>

#ifdef CONFIG_USB_SERIAL_DEBUG
	static int debug = 1;
#else
	static int debug;
#endif

#include "usb-serial.h"

/* Function prototypes */
static int  option_open(struct usb_serial_port *port, struct file *filp);
static void option_close(struct usb_serial_port *port, struct file *filp);
static int  option_startup(struct usb_serial *serial);
static void option_shutdown(struct usb_serial *serial);
static int  option_write_room(struct usb_serial_port *port);

static void option_indat_callback(struct urb *urb);
static void option_outdat_callback(struct urb *urb);
static void option_instat_callback(struct urb *urb);

static int option_write(struct usb_serial_port *port, int from_user,
			const unsigned char *buf, int count);

static int  option_chars_in_buffer(struct usb_serial_port *port);
static int  option_ioctl(struct usb_serial_port *port, struct file *file,
			unsigned int cmd, unsigned long arg);
static void option_set_termios(struct usb_serial_port *port,
				struct termios *old);
static int  option_tiocmget(struct usb_serial_port *port, struct file *file);
static int  option_tiocmset(struct usb_serial_port *port, struct file *file,
				unsigned int set, unsigned int clear);
static int  option_send_setup(struct usb_serial_port *port);

/* Vendor and product IDs */
#define OPTION_VENDOR_ID			0x0AF0
#define OPTION_PRODUCT_COLT			0x5000
#define OPTION_PRODUCT_RICOLA			0x6000
#define OPTION_PRODUCT_RICOLA_LIGHT		0x6100
#define OPTION_PRODUCT_RICOLA_QUAD		0x6200
#define OPTION_PRODUCT_RICOLA_QUAD_LIGHT	0x6300
#define OPTION_PRODUCT_RICOLA_NDIS		0x6050
#define OPTION_PRODUCT_RICOLA_NDIS_LIGHT	0x6150
#define OPTION_PRODUCT_RICOLA_NDIS_QUAD		0x6250
#define OPTION_PRODUCT_RICOLA_NDIS_QUAD_LIGHT	0x6350
#define OPTION_PRODUCT_COBRA			0x6500
#define OPTION_PRODUCT_COBRA_BUS		0x6501
#define OPTION_PRODUCT_VIPER			0x6600
#define OPTION_PRODUCT_VIPER_BUS		0x6601
#define OPTION_PRODUCT_GT_MAX_READY		0x6701
#define OPTION_PRODUCT_FUJI_MODEM_LIGHT		0x6721
#define OPTION_PRODUCT_FUJI_MODEM_GT		0x6741
#define OPTION_PRODUCT_FUJI_MODEM_EX		0x6761
#define OPTION_PRODUCT_KOI_MODEM		0x6800
#define OPTION_PRODUCT_SCORPION_MODEM		0x6901
#define OPTION_PRODUCT_ETNA_MODEM		0x7001
#define OPTION_PRODUCT_ETNA_MODEM_LITE		0x7021
#define OPTION_PRODUCT_ETNA_MODEM_GT		0x7041
#define OPTION_PRODUCT_ETNA_MODEM_EX		0x7061
#define OPTION_PRODUCT_ETNA_KOI_MODEM		0x7100

#define HUAWEI_VENDOR_ID			0x12D1
#define HUAWEI_PRODUCT_E600			0x1001
#define HUAWEI_PRODUCT_E220			0x1003
#define HUAWEI_PRODUCT_E220BIS			0x1004
#define HUAWEI_PRODUCT_E1401			0x1401
#define HUAWEI_PRODUCT_E1402			0x1402
#define HUAWEI_PRODUCT_E1403			0x1403
#define HUAWEI_PRODUCT_E1404			0x1404
#define HUAWEI_PRODUCT_E1405			0x1405
#define HUAWEI_PRODUCT_E1406			0x1406
#define HUAWEI_PRODUCT_E1407			0x1407
#define HUAWEI_PRODUCT_E1408			0x1408
#define HUAWEI_PRODUCT_E1409			0x1409
#define HUAWEI_PRODUCT_E140A			0x140A
#define HUAWEI_PRODUCT_E140B			0x140B
#define HUAWEI_PRODUCT_E140C			0x140C
#define HUAWEI_PRODUCT_E140D			0x140D
#define HUAWEI_PRODUCT_E140E			0x140E
#define HUAWEI_PRODUCT_E140F			0x140F
#define HUAWEI_PRODUCT_E1410			0x1410
#define HUAWEI_PRODUCT_E1411			0x1411
#define HUAWEI_PRODUCT_E1412			0x1412
#define HUAWEI_PRODUCT_E1413			0x1413
#define HUAWEI_PRODUCT_E1414			0x1414
#define HUAWEI_PRODUCT_E1415			0x1415
#define HUAWEI_PRODUCT_E1416			0x1416
#define HUAWEI_PRODUCT_E1417			0x1417
#define HUAWEI_PRODUCT_E1418			0x1418
#define HUAWEI_PRODUCT_E1419			0x1419
#define HUAWEI_PRODUCT_E141A			0x141A
#define HUAWEI_PRODUCT_E141B			0x141B
#define HUAWEI_PRODUCT_E141C			0x141C
#define HUAWEI_PRODUCT_E141D			0x141D
#define HUAWEI_PRODUCT_E141E			0x141E
#define HUAWEI_PRODUCT_E141F			0x141F
#define HUAWEI_PRODUCT_E1420			0x1420
#define HUAWEI_PRODUCT_E1421			0x1421
#define HUAWEI_PRODUCT_E1422			0x1422
#define HUAWEI_PRODUCT_E1423			0x1423
#define HUAWEI_PRODUCT_E1424			0x1424
#define HUAWEI_PRODUCT_E1425			0x1425
#define HUAWEI_PRODUCT_E1426			0x1426
#define HUAWEI_PRODUCT_E1427			0x1427
#define HUAWEI_PRODUCT_E1428			0x1428
#define HUAWEI_PRODUCT_E1429			0x1429
#define HUAWEI_PRODUCT_E142A			0x142A
#define HUAWEI_PRODUCT_E142B			0x142B
#define HUAWEI_PRODUCT_E142C			0x142C
#define HUAWEI_PRODUCT_E142D			0x142D
#define HUAWEI_PRODUCT_E142E			0x142E
#define HUAWEI_PRODUCT_E142F			0x142F
#define HUAWEI_PRODUCT_E1430			0x1430
#define HUAWEI_PRODUCT_E1431			0x1431
#define HUAWEI_PRODUCT_E1432			0x1432
#define HUAWEI_PRODUCT_E1433			0x1433
#define HUAWEI_PRODUCT_E1434			0x1434
#define HUAWEI_PRODUCT_E1435			0x1435
#define HUAWEI_PRODUCT_E1436			0x1436
#define HUAWEI_PRODUCT_E1437			0x1437
#define HUAWEI_PRODUCT_E1438			0x1438
#define HUAWEI_PRODUCT_E1439			0x1439
#define HUAWEI_PRODUCT_E143A			0x143A
#define HUAWEI_PRODUCT_E143B			0x143B
#define HUAWEI_PRODUCT_E143C			0x143C
#define HUAWEI_PRODUCT_E143D			0x143D
#define HUAWEI_PRODUCT_E143E			0x143E
#define HUAWEI_PRODUCT_E143F			0x143F

#define NOVATELWIRELESS_VENDOR_ID		0x1410

/* YISO PRODUCTS */

#define YISO_VENDOR_ID				0x0EAB
#define YISO_PRODUCT_U893			0xC893

/* MERLIN EVDO PRODUCTS */
#define NOVATELWIRELESS_PRODUCT_V640		0x1100
#define NOVATELWIRELESS_PRODUCT_V620		0x1110
#define NOVATELWIRELESS_PRODUCT_V740		0x1120
#define NOVATELWIRELESS_PRODUCT_V720		0x1130

/* MERLIN HSDPA/HSPA PRODUCTS */
#define NOVATELWIRELESS_PRODUCT_U730		0x1400
#define NOVATELWIRELESS_PRODUCT_U740		0x1410
#define NOVATELWIRELESS_PRODUCT_U870		0x1420
#define NOVATELWIRELESS_PRODUCT_XU870		0x1430
#define NOVATELWIRELESS_PRODUCT_X950D		0x1450

/* EXPEDITE PRODUCTS */
#define NOVATELWIRELESS_PRODUCT_EV620		0x2100
#define NOVATELWIRELESS_PRODUCT_ES720		0x2110
#define NOVATELWIRELESS_PRODUCT_E725		0x2120
#define NOVATELWIRELESS_PRODUCT_ES620		0x2130
#define NOVATELWIRELESS_PRODUCT_EU730		0x2400
#define NOVATELWIRELESS_PRODUCT_EU740		0x2410
#define NOVATELWIRELESS_PRODUCT_EU870D		0x2420

/* OVATION PRODUCTS */
#define NOVATELWIRELESS_PRODUCT_MC727		0x4100
#define NOVATELWIRELESS_PRODUCT_MC950D		0x4400

/* FUTURE NOVATEL PRODUCTS */
#define NOVATELWIRELESS_PRODUCT_EVDO_FULLSPEED	0X6000
#define NOVATELWIRELESS_PRODUCT_EVDO_HIGHSPEED	0X6001
#define NOVATELWIRELESS_PRODUCT_HSPA_FULLSPEED	0X7000
#define NOVATELWIRELESS_PRODUCT_HSPA_HIGHSPEED	0X7001
#define NOVATELWIRELESS_PRODUCT_EVDO_EMBEDDED_FULLSPEED	0X8000
#define NOVATELWIRELESS_PRODUCT_EVDO_EMBEDDED_HIGHSPEED	0X8001
#define NOVATELWIRELESS_PRODUCT_HSPA_EMBEDDED_FULLSPEED	0X9000
#define NOVATELWIRELESS_PRODUCT_HSPA_EMBEDDED_HIGHSPEED	0X9001
#define NOVATELWIRELESS_PRODUCT_GLOBAL		0XA001

/* AMOI PRODUCTS */
#define AMOI_VENDOR_ID				0x1614
#define AMOI_PRODUCT_H01			0x0800
#define AMOI_PRODUCT_H01A			0x7002
#define AMOI_PRODUCT_H02			0x0802

#define DELL_VENDOR_ID				0x413C

/* Dell modems */
#define DELL_PRODUCT_5700_MINICARD		0x8114
#define DELL_PRODUCT_5500_MINICARD		0x8115
#define DELL_PRODUCT_5505_MINICARD		0x8116
#define DELL_PRODUCT_5700_EXPRESSCARD		0x8117
#define DELL_PRODUCT_5510_EXPRESSCARD		0x8118

#define DELL_PRODUCT_5700_MINICARD_SPRINT	0x8128
#define DELL_PRODUCT_5700_MINICARD_TELUS	0x8129

#define DELL_PRODUCT_5720_MINICARD_VZW		0x8133
#define DELL_PRODUCT_5720_MINICARD_SPRINT	0x8134
#define DELL_PRODUCT_5720_MINICARD_TELUS	0x8135
#define DELL_PRODUCT_5520_MINICARD_CINGULAR	0x8136
#define DELL_PRODUCT_5520_MINICARD_GENERIC_L	0x8137
#define DELL_PRODUCT_5520_MINICARD_GENERIC_I	0x8138

#define DELL_PRODUCT_5730_MINICARD_SPRINT	0x8180
#define DELL_PRODUCT_5730_MINICARD_TELUS	0x8181
#define DELL_PRODUCT_5730_MINICARD_VZW		0x8182

#define KYOCERA_VENDOR_ID			0x0c88
#define KYOCERA_PRODUCT_KPC650			0x17da
#define KYOCERA_PRODUCT_KPC680			0x180a

#define ANYDATA_VENDOR_ID			0x16d5
#define ANYDATA_PRODUCT_ADU_620UW		0x6202
#define ANYDATA_PRODUCT_ADU_E100A		0x6501
#define ANYDATA_PRODUCT_ADU_500A		0x6502

#define AXESSTEL_VENDOR_ID			0x1726
#define AXESSTEL_PRODUCT_MV110H			0x1000

#define ONDA_VENDOR_ID				0x19d2
#define ONDA_PRODUCT_MSA501HS			0x0001
#define ONDA_PRODUCT_ET502HS			0x0002
#define ONDA_PRODUCT_MT503HS			0x0200

#define BANDRICH_VENDOR_ID			0x1A8D
#define BANDRICH_PRODUCT_C100_1			0x1002
#define BANDRICH_PRODUCT_C100_2			0x1003
#define BANDRICH_PRODUCT_1004			0x1004
#define BANDRICH_PRODUCT_1005			0x1005
#define BANDRICH_PRODUCT_1006			0x1006
#define BANDRICH_PRODUCT_1007			0x1007
#define BANDRICH_PRODUCT_1008			0x1008
#define BANDRICH_PRODUCT_1009			0x1009
#define BANDRICH_PRODUCT_100A			0x100a

#define BANDRICH_PRODUCT_100B			0x100b
#define BANDRICH_PRODUCT_100C			0x100c
#define BANDRICH_PRODUCT_100D			0x100d
#define BANDRICH_PRODUCT_100E			0x100e

#define BANDRICH_PRODUCT_100F			0x100f
#define BANDRICH_PRODUCT_1010			0x1010
#define BANDRICH_PRODUCT_1011			0x1011
#define BANDRICH_PRODUCT_1012			0x1012

#define AMOI_VENDOR_ID			0x1614
#define AMOI_PRODUCT_9508			0x0800

#define QUALCOMM_VENDOR_ID			0x05C6

#define MAXON_VENDOR_ID				0x16d8

#define TELIT_VENDOR_ID				0x1bc7
#define TELIT_PRODUCT_UC864E			0x1003

/* ZTE PRODUCTS */
#define ZTE_VENDOR_ID				0x19d2
#define ZTE_PRODUCT_MF622			0x0001
#define ZTE_PRODUCT_MF628			0x0015
#define ZTE_PRODUCT_MF626			0x0031
#define ZTE_PRODUCT_CDMA_TECH			0xfffe

/* Ericsson products */
#define ERICSSON_VENDOR_ID			0x0bdb
#define ERICSSON_PRODUCT_F3507G			0x1900

static struct usb_device_id option_ids[] = {
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_COLT) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_RICOLA) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_RICOLA_LIGHT) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_RICOLA_QUAD) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_RICOLA_QUAD_LIGHT) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_RICOLA_NDIS) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_RICOLA_NDIS_LIGHT) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_RICOLA_NDIS_QUAD) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_RICOLA_NDIS_QUAD_LIGHT) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_COBRA) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_COBRA_BUS) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_VIPER) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_VIPER_BUS) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_GT_MAX_READY) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_FUJI_MODEM_LIGHT) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_FUJI_MODEM_GT) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_FUJI_MODEM_EX) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_KOI_MODEM) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_SCORPION_MODEM) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_ETNA_MODEM) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_ETNA_MODEM_LITE) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_ETNA_MODEM_GT) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_ETNA_MODEM_EX) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_ETNA_KOI_MODEM) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E600, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E220, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E220BIS, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1401, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1402, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1403, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1404, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1405, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1406, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1407, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1408, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1409, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E140A, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E140B, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E140C, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E140D, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E140E, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E140F, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1410, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1411, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1412, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1413, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1414, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1415, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1416, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1417, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1418, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1419, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E141A, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E141B, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E141C, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E141D, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E141E, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E141F, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1420, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1421, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1422, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1423, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1424, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1425, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1426, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1427, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1428, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1429, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E142A, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E142B, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E142C, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E142D, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E142E, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E142F, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1430, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1431, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1432, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1433, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1434, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1435, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1436, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1437, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1438, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E1439, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E143A, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E143B, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E143C, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E143D, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E143E, 0xff, 0xff, 0xff) },
	{ USB_DEVICE_AND_INTERFACE_INFO(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E143F, 0xff, 0xff, 0xff) },
	{ USB_DEVICE(AMOI_VENDOR_ID, AMOI_PRODUCT_9508) },
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_V640) }, /* Novatel Merlin V640/XV620 */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_V620) }, /* Novatel Merlin V620/S620 */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_V740) }, /* Novatel Merlin EX720/V740/X720 */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_V720) }, /* Novatel Merlin V720/S720/PC720 */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_U730) }, /* Novatel U730/U740 (VF version) */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_U740) }, /* Novatel U740 */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_U870) }, /* Novatel U870 */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_XU870) }, /* Novatel Merlin XU870 HSDPA/3G */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_X950D) }, /* Novatel X950D */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_EV620) }, /* Novatel EV620/ES620 CDMA/EV-DO */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_ES720) }, /* Novatel ES620/ES720/U720/USB720 */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_E725) }, /* Novatel E725/E726 */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_ES620) }, /* Novatel Merlin ES620 SM Bus */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_EU730) }, /* Novatel EU730 and Vodafone EU740 */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_EU740) }, /* Novatel non-Vodafone EU740 */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_EU870D) }, /* Novatel EU850D/EU860D/EU870D */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_MC950D) }, /* Novatel MC930D/MC950D */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_MC727) }, /* Novatel MC727/U727/USB727 */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_EVDO_FULLSPEED) }, /* Novatel EVDO product */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_HSPA_FULLSPEED) }, /* Novatel HSPA product */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_EVDO_EMBEDDED_FULLSPEED) }, /* Novatel EVDO Embedded product */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_HSPA_EMBEDDED_FULLSPEED) }, /* Novatel HSPA Embedded product */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_EVDO_HIGHSPEED) }, /* Novatel EVDO product */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_HSPA_HIGHSPEED) }, /* Novatel HSPA product */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_EVDO_EMBEDDED_HIGHSPEED) }, /* Novatel EVDO Embedded product */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_HSPA_EMBEDDED_HIGHSPEED) }, /* Novatel HSPA Embedded product */
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID, NOVATELWIRELESS_PRODUCT_GLOBAL) }, /* Novatel Global product */

	{ USB_DEVICE(AMOI_VENDOR_ID, AMOI_PRODUCT_H01) },
	{ USB_DEVICE(AMOI_VENDOR_ID, AMOI_PRODUCT_H01A) },
	{ USB_DEVICE(AMOI_VENDOR_ID, AMOI_PRODUCT_H02) },

	{ USB_DEVICE(DELL_VENDOR_ID, DELL_PRODUCT_5700_MINICARD) },		/* Dell Wireless 5700 Mobile Broadband CDMA/EVDO Mini-Card == Novatel Expedite EV620 CDMA/EV-DO */
	{ USB_DEVICE(DELL_VENDOR_ID, DELL_PRODUCT_5500_MINICARD) },		/* Dell Wireless 5500 Mobile Broadband HSDPA Mini-Card == Novatel Expedite EU740 HSDPA/3G */
	{ USB_DEVICE(DELL_VENDOR_ID, DELL_PRODUCT_5505_MINICARD) },		/* Dell Wireless 5505 Mobile Broadband HSDPA Mini-Card == Novatel Expedite EU740 HSDPA/3G */
	{ USB_DEVICE(DELL_VENDOR_ID, DELL_PRODUCT_5700_EXPRESSCARD) },		/* Dell Wireless 5700 Mobile Broadband CDMA/EVDO ExpressCard == Novatel Merlin XV620 CDMA/EV-DO */
	{ USB_DEVICE(DELL_VENDOR_ID, DELL_PRODUCT_5510_EXPRESSCARD) },		/* Dell Wireless 5510 Mobile Broadband HSDPA ExpressCard == Novatel Merlin XU870 HSDPA/3G */
	{ USB_DEVICE(DELL_VENDOR_ID, DELL_PRODUCT_5700_MINICARD_SPRINT) },	/* Dell Wireless 5700 Mobile Broadband CDMA/EVDO Mini-Card == Novatel Expedite E720 CDMA/EV-DO */
	{ USB_DEVICE(DELL_VENDOR_ID, DELL_PRODUCT_5700_MINICARD_TELUS) },	/* Dell Wireless 5700 Mobile Broadband CDMA/EVDO Mini-Card == Novatel Expedite ET620 CDMA/EV-DO */
	{ USB_DEVICE(DELL_VENDOR_ID, DELL_PRODUCT_5720_MINICARD_VZW) }, 	/* Dell Wireless 5720 == Novatel EV620 CDMA/EV-DO */
	{ USB_DEVICE(DELL_VENDOR_ID, DELL_PRODUCT_5720_MINICARD_SPRINT) }, 	/* Dell Wireless 5720 == Novatel EV620 CDMA/EV-DO */
	{ USB_DEVICE(DELL_VENDOR_ID, DELL_PRODUCT_5720_MINICARD_TELUS) }, 	/* Dell Wireless 5720 == Novatel EV620 CDMA/EV-DO */
	{ USB_DEVICE(DELL_VENDOR_ID, DELL_PRODUCT_5520_MINICARD_CINGULAR) },	/* Dell Wireless HSDPA 5520 == Novatel Expedite EU860D */
	{ USB_DEVICE(DELL_VENDOR_ID, DELL_PRODUCT_5520_MINICARD_GENERIC_L) },	/* Dell Wireless HSDPA 5520 */
	{ USB_DEVICE(DELL_VENDOR_ID, DELL_PRODUCT_5520_MINICARD_GENERIC_I) },	/* Dell Wireless 5520 Voda I Mobile Broadband (3G HSDPA) Minicard */
	{ USB_DEVICE(DELL_VENDOR_ID, 0x8147) },					/* Dell Wireless 5530 Mobile Broadband (3G HSPA) Mini-Card */
	{ USB_DEVICE(DELL_VENDOR_ID, DELL_PRODUCT_5730_MINICARD_SPRINT) },	/* Dell Wireless 5730 Mobile Broadband EVDO/HSPA Mini-Card */
	{ USB_DEVICE(DELL_VENDOR_ID, DELL_PRODUCT_5730_MINICARD_TELUS) },	/* Dell Wireless 5730 Mobile Broadband EVDO/HSPA Mini-Card */
	{ USB_DEVICE(DELL_VENDOR_ID, DELL_PRODUCT_5730_MINICARD_VZW) }, 	/* Dell Wireless 5730 Mobile Broadband EVDO/HSPA Mini-Card */
	{ USB_DEVICE(ANYDATA_VENDOR_ID, ANYDATA_PRODUCT_ADU_E100A) },	/* ADU-E100, ADU-310 */
	{ USB_DEVICE(ANYDATA_VENDOR_ID, ANYDATA_PRODUCT_ADU_500A) },
	{ USB_DEVICE(ANYDATA_VENDOR_ID, ANYDATA_PRODUCT_ADU_620UW) },
	{ USB_DEVICE(AXESSTEL_VENDOR_ID, AXESSTEL_PRODUCT_MV110H) },
	{ USB_DEVICE(ONDA_VENDOR_ID, ONDA_PRODUCT_MSA501HS) },
	{ USB_DEVICE(ONDA_VENDOR_ID, ONDA_PRODUCT_ET502HS) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0003) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0004) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0005) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0006) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0007) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0008) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0009) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x000a) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x000b) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x000c) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x000d) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x000e) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x000f) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0010) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0011) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0012) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0013) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0014) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0015) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0016) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0017) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0018) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0019) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0020) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0021) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0022) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0023) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0024) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0025) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0026) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0027) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0028) },
	{ USB_DEVICE(ONDA_VENDOR_ID, 0x0029) },
	{ USB_DEVICE(ONDA_VENDOR_ID, ONDA_PRODUCT_MT503HS) },
	{ USB_DEVICE(YISO_VENDOR_ID, YISO_PRODUCT_U893) },
	{ USB_DEVICE(BANDRICH_VENDOR_ID, BANDRICH_PRODUCT_C100_1) },
	{ USB_DEVICE(BANDRICH_VENDOR_ID, BANDRICH_PRODUCT_C100_2) },
	{ USB_DEVICE(BANDRICH_VENDOR_ID, BANDRICH_PRODUCT_1004) },
	{ USB_DEVICE(BANDRICH_VENDOR_ID, BANDRICH_PRODUCT_1005) },
	{ USB_DEVICE(BANDRICH_VENDOR_ID, BANDRICH_PRODUCT_1006) },
	{ USB_DEVICE(BANDRICH_VENDOR_ID, BANDRICH_PRODUCT_1007) },
	{ USB_DEVICE(BANDRICH_VENDOR_ID, BANDRICH_PRODUCT_1008) },
	{ USB_DEVICE(BANDRICH_VENDOR_ID, BANDRICH_PRODUCT_1009) },
	{ USB_DEVICE(BANDRICH_VENDOR_ID, BANDRICH_PRODUCT_100A) },
	{ USB_DEVICE(BANDRICH_VENDOR_ID, BANDRICH_PRODUCT_100B) },
	{ USB_DEVICE(BANDRICH_VENDOR_ID, BANDRICH_PRODUCT_100C) },
	{ USB_DEVICE(BANDRICH_VENDOR_ID, BANDRICH_PRODUCT_100D) },
	{ USB_DEVICE(BANDRICH_VENDOR_ID, BANDRICH_PRODUCT_100E) },
	{ USB_DEVICE(BANDRICH_VENDOR_ID, BANDRICH_PRODUCT_100F) },
	{ USB_DEVICE(BANDRICH_VENDOR_ID, BANDRICH_PRODUCT_1010) },
	{ USB_DEVICE(BANDRICH_VENDOR_ID, BANDRICH_PRODUCT_1011) },
	{ USB_DEVICE(BANDRICH_VENDOR_ID, BANDRICH_PRODUCT_1012) },
	{ USB_DEVICE(KYOCERA_VENDOR_ID, KYOCERA_PRODUCT_KPC650) },
	{ USB_DEVICE(KYOCERA_VENDOR_ID, KYOCERA_PRODUCT_KPC680) },
	{ USB_DEVICE(QUALCOMM_VENDOR_ID, 0x6000)}, /* ZTE AC8700 */
	{ USB_DEVICE(QUALCOMM_VENDOR_ID, 0x6613)}, /* Onda H600/ZTE MF330 */
	{ USB_DEVICE(MAXON_VENDOR_ID, 0x6280) }, /* BP3-USB & BP3-EXT HSDPA */
	{ USB_DEVICE(TELIT_VENDOR_ID, TELIT_PRODUCT_UC864E) },
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_MF626) },
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_MF628) },
	{ USB_DEVICE(ZTE_VENDOR_ID, ZTE_PRODUCT_CDMA_TECH) },
	{ USB_DEVICE_AND_INTERFACE_INFO(ZTE_VENDOR_ID, ZTE_PRODUCT_MF622, 0xff, 0xff, 0xff) },
	{ USB_DEVICE(ERICSSON_VENDOR_ID, ERICSSON_PRODUCT_F3507G) },
	{ } /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, option_ids);

/* The card has three separate interfaces, which the serial driver
 * recognizes separately, thus num_port=1.
 */

static struct usb_serial_device_type option_1port_device = {
	.owner               = THIS_MODULE,
	.name                = "Option GSM modem",
	.id_table            = option_ids,
	.num_interrupt_in    = NUM_DONT_CARE,
	.num_bulk_in         = NUM_DONT_CARE,
	.num_bulk_out        = NUM_DONT_CARE,
	.num_ports           = 1,
	.open                = option_open,
	.close               = option_close,
	.write               = option_write,
	.write_room          = option_write_room,
	.chars_in_buffer     = option_chars_in_buffer,
	.read_bulk_callback  = option_indat_callback,
	.write_bulk_callback = option_outdat_callback,
	.ioctl               = option_ioctl,
	.set_termios         = option_set_termios,
//2.6	.tiocmget            = option_tiocmget,
//2.6	.tiocmset            = option_tiocmset,
	.startup             = option_startup,
	.shutdown            = option_shutdown,
	.read_int_callback   = option_instat_callback,
};

static int debug;

/* per port private data */

#define N_IN_URB	4
#define N_OUT_URB	1
#define IN_BUFLEN	4096		/* Must be equal to sizeof kernel page */
#define OUT_BUFLEN	1024

/* bitmap for set_line_state */
#define UART_DTR			0x01
#define UART_RTS			0x02

/* bitmap for update_line_state */
#define UART_DCD			0x01
#define UART_DSR			0x02
#define UART_BREAK_ERROR		0x04
#define UART_RING			0x08
#define UART_FRAME_ERROR		0x10
#define UART_PARITY_ERROR		0x20
#define UART_OVERRUN_ERROR		0x40
#define UART_CTS			0x80
#define UART_STATE_TRANSIENT_MASK	(~(UART_CTS|UART_RING|UART_DSR|UART_DCD))

struct option_port_private {
	spinlock_t lock;
	wait_queue_head_t delta_msr_wait;	 /* Used for TIOCMIWAIT */
	/* Input endpoints and buffer for this port */
	struct urb *in_urbs[N_IN_URB];
	u8         *in_buffer[N_IN_URB];
	/* Output endpoints and buffer for this port */
	struct urb *out_urbs[N_OUT_URB];
	u8         *out_buffer[N_OUT_URB];
	u32         out_busy;			/* Bit vector of URBs in use */
	unsigned long tx_start_time[N_OUT_URB];

	/* Settings for the port */
	u8 line_status;
	unsigned rts_state:1;	/* Handshaking pins (outputs) */
	unsigned dtr_state:1;
	unsigned cts_state:1;	/* Handshaking pins (inputs) */
	unsigned dsr_state:1;
	unsigned dcd_state:1;
	unsigned  ri_state:1;
};


/* Functions used by new usb-serial code. */
static void option_set_termios(struct usb_serial_port *port, struct termios *old_termios)
{
	dbg("%s", __FUNCTION__);

	/* Doesn't support option setting */
//2.6	tty_termios_copy_hw(tty->termios, old_termios);
	option_send_setup(port);
}

static int option_tiocmget(struct usb_serial_port *port, struct file *file)
{
	unsigned int value;
	struct option_port_private *portdata = usb_get_serial_port_data(port);

	value = ((portdata->rts_state) ? TIOCM_RTS : 0) |
		((portdata->dtr_state) ? TIOCM_DTR : 0) |
		((portdata->cts_state) ? TIOCM_CTS : 0) |
		((portdata->dsr_state) ? TIOCM_DSR : 0) |
		((portdata->dcd_state) ? TIOCM_CAR : 0) |
		((portdata->ri_state)  ? TIOCM_RNG : 0);

	return value;
}

static int option_tiocmset(struct usb_serial_port *port, struct file *file,
			unsigned int set, unsigned int clear)
{
	struct option_port_private *portdata = usb_get_serial_port_data(port);
	unsigned long flags;

	spin_lock_irqsave(&portdata->lock, flags);
	if (set & TIOCM_RTS)
		portdata->rts_state = 1;
	if (set & TIOCM_DTR)
		portdata->dtr_state = 1;
	if (clear & TIOCM_RTS)
		portdata->rts_state = 0;
	if (clear & TIOCM_DTR)
		portdata->dtr_state = 0;
	spin_unlock_irqrestore(&portdata->lock, flags);

	return option_send_setup(port);
}

static int wait_modem_info(struct usb_serial_port *port, unsigned int arg)
{
	struct option_port_private *portdata = usb_get_serial_port_data(port);
	unsigned long flags;
	u8 prevstatus, status, changed;

	spin_lock_irqsave(&portdata->lock, flags);
	prevstatus = portdata->line_status;
	spin_unlock_irqrestore(&portdata->lock, flags);

	while (1) {
		interruptible_sleep_on(&portdata->delta_msr_wait);
		/* see if a signal did it */
		if (signal_pending(current))
			return -ERESTARTSYS;

		spin_lock_irqsave(&portdata->lock, flags);
		status = portdata->line_status;
		spin_unlock_irqrestore(&portdata->lock, flags);

		changed = prevstatus^status;

		if (((arg & TIOCM_RNG) && (changed & UART_RING)) ||
		    ((arg & TIOCM_DSR) && (changed & UART_DSR)) ||
		    ((arg & TIOCM_CD)  && (changed & UART_DCD)) ||
		    ((arg & TIOCM_CTS) && (changed & UART_CTS)) ) {
			return 0;
		}
		prevstatus = status;
	}
	/* NOTREACHED */
	return 0;
}

static int option_ioctl(struct usb_serial_port *port, struct file *file,
			unsigned int cmd, unsigned long arg)
{
	int value;

	dbg("%s: port %d cmd = 0x%04x", __FUNCTION__, port->number, cmd);

	switch (cmd) {
		case TIOCMGET:
			value = option_tiocmget(port, file);
			if (copy_to_user((unsigned int *)arg, &value, sizeof(int)))
				return -EFAULT;
			return 0;

		case TIOCMSET:
			if (copy_from_user(&value, (unsigned int *)arg, sizeof(int)))
				return -EFAULT;
			/* turn off RTS and DTR and then only turn
			   on what was asked to */
			return option_tiocmset(port, file, value, value^(TIOCM_RTS|TIOCM_DTR));

		case TIOCMBIS:
			if (copy_from_user(&value, (unsigned int *)arg, sizeof(int)))
				return -EFAULT;
			return option_tiocmset(port, file, value, 0);

		case TIOCMBIC:
			if (copy_from_user(&value, (unsigned int *)arg, sizeof(int)))
				return -EFAULT;
			return option_tiocmset(port, file, 0, value);

		case TIOCMIWAIT:
			return wait_modem_info(port, arg);

		default:
			dbg("%s: not supported = 0x%04x", __FUNCTION__, cmd);
			break;
	}
	return -ENOIOCTLCMD;
}

/* Write */
static int option_write(struct usb_serial_port *port, int from_user, 
			const unsigned char *buf, int count)
{
	struct option_port_private *portdata = usb_get_serial_port_data(port);
	unsigned long flags;
	int i;
	int left, todo;
	struct urb *this_urb = NULL; /* spurious */
	int err;

	dbg("%s: write port %d (%d chars)", __FUNCTION__, port->number, count);

	left = count;
	spin_lock_irqsave(&portdata->lock, flags);
	for (i = 0; left > 0 && i < N_OUT_URB; i++) {
		todo = left;
		if (todo > OUT_BUFLEN)
			todo = OUT_BUFLEN;

		this_urb = portdata->out_urbs[i];
		if (test_and_set_bit(i, &portdata->out_busy)) {
			if (time_before(jiffies,
					portdata->tx_start_time[i] + 10 * HZ))
				continue;
			usb_unlink_urb(this_urb);
			continue;
		}
		if (this_urb->status != 0)
			dbg("usb_write %p failed (err=%d)",
				this_urb, this_urb->status);

		/* send the data */
                if (from_user) {
                        if (copy_from_user(this_urb->transfer_buffer, buf, todo))
                                return -EFAULT;
                }
                else {
			memcpy(this_urb->transfer_buffer, buf, todo);
                }
		usb_serial_debug_data (__FILE__, __FUNCTION__, todo, this_urb->transfer_buffer);

		this_urb->transfer_buffer_length = todo;
		this_urb->dev = port->serial->dev;
		err = usb_submit_urb(this_urb);
		if (err) {
			dbg("usb_submit_urb %p (write bulk) failed "
				"(%d, has %d)", this_urb,
				err, this_urb->status);
			clear_bit(i, &portdata->out_busy);
			continue;
		}
		portdata->tx_start_time[i] = jiffies;
		buf += todo;
		left -= todo;
	}

	spin_unlock_irqrestore(&portdata->lock, flags);
	count -= left;
	dbg("%s: wrote (did %d)", __FUNCTION__, count);
	return count;
}

static void option_indat_callback(struct urb *urb)
{
	int i, err;
	struct usb_serial_port *port = urb->context;
	struct option_port_private *portdata = usb_get_serial_port_data(port);
	struct tty_struct *tty;
	unsigned char *data = urb->transfer_buffer;
	int status = urb->status;
	unsigned long flags;
	char tty_flag;
	u8 line_status;

	dbg("%s: urb %p ep %d packets %d", __FUNCTION__, urb, usb_pipeendpoint(urb->pipe), urb->number_of_packets);

	switch (status) {
	case 0:
		/* success */
		break;
	case -ECONNRESET:
	case -ENOENT:
	case -ESHUTDOWN:
		/* this urb is terminated, clean up */
		dbg("%s - urb shutting down with status: %d", __FUNCTION__,
		    status);
		return;
	default:
		dbg("%s - nonzero urb status received: %d", __FUNCTION__,
		    status);
		return;
	}

	usb_serial_debug_data (__FILE__, __FUNCTION__, urb->actual_length, data);

	/* get tty_flag from status */
	tty_flag = TTY_NORMAL;

	spin_lock_irqsave(&portdata->lock, flags);
	line_status = portdata->line_status;
	portdata->line_status &= ~UART_STATE_TRANSIENT_MASK;
	spin_unlock_irqrestore(&portdata->lock, flags);
	wake_up_interruptible(&portdata->delta_msr_wait);

	/* break takes precedence over parity, */
	/* which takes precedence over framing errors */
	if (line_status & UART_BREAK_ERROR )
		tty_flag = TTY_BREAK;
	else if (line_status & UART_PARITY_ERROR)
		tty_flag = TTY_PARITY;
	else if (line_status & UART_FRAME_ERROR)
		tty_flag = TTY_FRAME;
	dbg("%s: tty_flag = %d", __FUNCTION__, tty_flag);

	tty = port->tty;
	if (tty && urb->actual_length>0) {
		/* overrun is special, not associated with a char */
		if (line_status & UART_OVERRUN_ERROR)
			tty_insert_flip_char(tty, 0, TTY_OVERRUN);

		for (i = 0; i < urb->actual_length; ++i) {
			if (tty->flip.count >= TTY_FLIPBUF_SIZE)
				tty_flip_buffer_push(tty);
			tty_insert_flip_char(tty, data[i], tty_flag);
		}
		tty_flip_buffer_push(tty);
	}

	/* Resubmit urb so we continue receiving */
	if (port->open_count && status != -ESHUTDOWN) {
		urb->dev = port->serial->dev;
		err = usb_submit_urb(urb);
		if (err)
			printk(KERN_ERR "%s: resubmit read urb failed. "
				"(%d)\n", __FUNCTION__, err);
	}
}

static void option_outdat_callback(struct urb *urb)
{
	struct usb_serial_port *port = urb->context;
	struct option_port_private *portdata = usb_get_serial_port_data(port);
	int status = urb->status;
	int i, result;

	dbg("%s: port %d count %d", __FUNCTION__, port->number, urb->actual_length);

	switch (status) {
	case 0:
		/* success */
		break;
	case -ECONNRESET:
	case -ENOENT:
	case -ESHUTDOWN:
		/* this urb is terminated, clean up */
		dbg("%s - urb shutting down with status: %d", __FUNCTION__,
		    status);
		break;
	default:
		/* error in the urb, so we have to resubmit it */
		dbg("%s - Overflow in write", __FUNCTION__);
		dbg("%s - nonzero urb status received: %d", __FUNCTION__,
		    status);
		urb->transfer_buffer_length = 1;
		urb->dev = port->serial->dev;
		result = usb_submit_urb(urb);
		if (result)
			err("%s - failed resubmitting write urb, error %d\n", __FUNCTION__, result);
		else
			return;
	}

//2.6	usb_serial_port_softint(port);

	for (i = 0; i < N_OUT_URB; ++i) {
		if (portdata->out_urbs[i] == urb) {
			smp_mb__before_clear_bit();
			clear_bit(i, &portdata->out_busy);
			break;
		}
	}

 	if (port->open_count) {
		queue_task(&port->tqueue, &tq_immediate);
		mark_bh(IMMEDIATE_BH);
	}
}

static void option_instat_callback(struct urb *urb)
{
	int status = urb->status;
	struct usb_serial_port *port =  urb->context;
	struct option_port_private *portdata = usb_get_serial_port_data(port);
	struct usb_ctrlrequest *req_pkt = (struct usb_ctrlrequest *)urb->transfer_buffer;

	dbg("%s: urb %p port %d req %p", __FUNCTION__, urb, port->number, req_pkt);

	switch (status) {
	case 0:
		if (!req_pkt) {
			dbg("%s: NULL req_pkt\n", __FUNCTION__);
			return;
		}
		if ((req_pkt->bRequestType == 0xA1) &&
				(req_pkt->bRequest == 0x20)) {
			unsigned old_dcd_state;
			u8 signals = *((u8 *)
					urb->transfer_buffer +
					sizeof(struct usb_ctrlrequest));

			dbg("%s: signal %x", __FUNCTION__, signals);

			old_dcd_state = portdata->dcd_state;
			portdata->cts_state = 1;
			portdata->dcd_state = ((signals & UART_DCD ) ? 1 : 0);
			portdata->dsr_state = ((signals & UART_DSR ) ? 1 : 0);
			portdata->ri_state  = ((signals & UART_RING) ? 1 : 0);
			portdata->line_status = signals;

			if (old_dcd_state && !portdata->dcd_state) {
				if (port->tty && !C_CLOCAL(port->tty))
					tty_hangup(port->tty);
			}
		} else {
			dbg("%s: type %x req %x", __FUNCTION__,
				req_pkt->bRequestType, req_pkt->bRequest);
		}
		break;

	case -ECONNRESET:
	case -ENOENT:
	case -ESHUTDOWN:
		/* this urb is terminated, clean up */
		dbg("%s - urb shutting down with status: %d", __FUNCTION__,
		    status);
		return;

	default:
		dbg("%s: error %d", __FUNCTION__, status);
	}

	usb_serial_debug_data (__FILE__, __FUNCTION__, urb->actual_length, urb->transfer_buffer);
	wake_up_interruptible(&portdata->delta_msr_wait);

	/* INT urbs are automatically re-submitted */
}

static int option_write_room(struct usb_serial_port *port)
{
	struct option_port_private *portdata = usb_get_serial_port_data(port);
	int i;
	int data_len = 0;
	struct urb *this_urb;
	unsigned long flags;

	spin_lock_irqsave(&portdata->lock, flags);
	for (i = 0; i < N_OUT_URB; i++) {
		this_urb = portdata->out_urbs[i];
		if (this_urb && !test_bit(i, &portdata->out_busy))
			data_len += OUT_BUFLEN;
	}
	spin_unlock_irqrestore(&portdata->lock, flags);

	dbg("%s: %d", __FUNCTION__, data_len);
	return data_len;
}

static int option_chars_in_buffer(struct usb_serial_port *port)
{
	struct option_port_private *portdata = usb_get_serial_port_data(port);
	int i;
	int data_len = 0;
	struct urb *this_urb;
	unsigned long flags;

	spin_lock_irqsave(&portdata->lock, flags);
	for (i = 0; i < N_OUT_URB; i++) {
		this_urb = portdata->out_urbs[i];
		/* FIXME: This locking is insufficient as this_urb may
		   go unused during the test */
		if (this_urb && test_bit(i, &portdata->out_busy))
			data_len += this_urb->transfer_buffer_length;
	}
	spin_unlock_irqrestore(&portdata->lock, flags);

	dbg("%s: returns %d", __FUNCTION__, data_len);
	return data_len;
}

static int option_open(struct usb_serial_port *port, struct file *filp)
{
	struct option_port_private *portdata = usb_get_serial_port_data(port);
	struct usb_serial *serial = port->serial;
	int i, err;
	struct urb *urb;

	dbg("%s", __FUNCTION__);

	/* Set some sane defaults */
	portdata->rts_state = 1;
	portdata->dtr_state = 1;

	/* Reset low level data toggle and start reading from endpoints */
	for (i = 0; i < N_IN_URB; i++) {
		urb = portdata->in_urbs[i];
		if (!urb)
			continue;
		if (urb->dev != serial->dev) {
			dbg("%s: dev %p != %p", __FUNCTION__,
				urb->dev, serial->dev);
			continue;
		}

		/*
		 * make sure endpoint data toggle is synchronized with the
		 * device
		 */
		usb_clear_halt(urb->dev, urb->pipe);

		err = usb_submit_urb(urb);
		if (err) {
			dbg("%s: submit urb %d failed (%d) %d",
				__FUNCTION__, i, err,
				urb->transfer_buffer_length);
		}
	}

	/* Reset low level data toggle on out endpoints */
	for (i = 0; i < N_OUT_URB; i++) {
		urb = portdata->out_urbs[i];
		if (!urb)
			continue;
		urb->dev = serial->dev;
		/* usb_settoggle(urb->dev, usb_pipeendpoint(urb->pipe),
				usb_pipeout(urb->pipe), 0); */
	}

	if (port->tty)
		port->tty->low_latency = 1;

	option_send_setup(port);

	return 0;
}

static void option_close(struct usb_serial_port *port, struct file *filp)
{
	int i;
	struct usb_serial *serial = port->serial;
	struct option_port_private *portdata = usb_get_serial_port_data(port);

	dbg("%s", __FUNCTION__);

	portdata->rts_state = 0;
	portdata->dtr_state = 0;

	if (serial->dev) {
		option_send_setup(port);

		/* Stop reading/writing urbs */
		for (i = 0; i < N_IN_URB; i++)
			usb_unlink_urb(portdata->in_urbs[i]);
		for (i = 0; i < N_OUT_URB; i++)
			usb_unlink_urb(portdata->out_urbs[i]);
	}
	port->tty = NULL;
}

/* Helper functions used by option_setup_urbs */
static struct urb *option_setup_urb(struct usb_device *dev, int endpoint,
		int dir, void *ctx, char *buf, int len,
		void (*callback)(struct urb *))
{
	struct urb *urb;

	if (endpoint == -1)
		return NULL;		/* endpoint not needed */

	urb = usb_alloc_urb(0);		/* No ISO */
	if (urb == NULL) {
		dbg("%s: alloc for endpoint %d failed.", __FUNCTION__, endpoint);
		return NULL;
	}

		/* Fill URB using supplied data. */
	usb_fill_bulk_urb(urb, dev,
		      (dir == USB_DIR_IN) ? usb_rcvbulkpipe(dev, endpoint) : usb_sndbulkpipe(dev, endpoint),
		      buf, len, callback, ctx);

	return urb;
}

/* Setup urbs */
static void option_setup_urbs(struct usb_serial *serial)
{
	int i, j;
	struct usb_serial_port *port;
	struct option_port_private *portdata;

	dbg("%s: num_ports %d", __FUNCTION__, serial->num_ports);

	for (i = 0; i < serial->num_ports; i++) {
		port = &serial->port[i];
		portdata = usb_get_serial_port_data(port);

		/* Do indat endpoints first */
		for (j = 0; j < N_IN_URB; ++j) {
			portdata->in_urbs[j] = option_setup_urb(serial->dev,
					port->bulk_in_endpointAddress,
					USB_DIR_IN, port,
					portdata->in_buffer[j],
					IN_BUFLEN, option_indat_callback);
		}

		/* outdat endpoints */
		for (j = 0; j < N_OUT_URB; ++j) {
			portdata->out_urbs[j] = option_setup_urb(serial->dev,
					port->bulk_out_endpointAddress,
					USB_DIR_OUT, port,
					portdata->out_buffer[j],
					OUT_BUFLEN, option_outdat_callback);
		}
	}
}


/** send RTS/DTR state to the port.
 *
 * This is exactly the same as SET_CONTROL_LINE_STATE from the PSTN
 * CDC.
*/
static int option_send_setup(struct usb_serial_port *port)
{
	struct usb_serial *serial = port->serial;
	struct option_port_private *portdata = usb_get_serial_port_data(port);
	int ifNum = serial->interface->altsetting->bInterfaceNumber;

	dbg("%s: port %d", __FUNCTION__, port->number);

	if (port->number != 0)
		return 0;

	if (port->tty) {
		int val = 0;
		if (portdata->dtr_state)
			val |= UART_DTR;
		if (portdata->rts_state)
			val |= UART_RTS;

		return usb_control_msg(serial->dev,
			usb_rcvctrlpipe(serial->dev, 0),
			0x22, 0x21, val, ifNum, NULL, 0, USB_CTRL_SET_TIMEOUT);
	}
	return 0;
}

static int option_startup(struct usb_serial *serial)
{
	int i, j, err;
	struct usb_serial_port *port;
	struct option_port_private *portdata;
	u8 *buffer;

	/* Now setup per port private data */
	for (i = 0; i < serial->num_ports; i++) {
		port = &serial->port[i];

		portdata = kmalloc(sizeof(*portdata), GFP_KERNEL);
		if (!portdata) {
			dbg("%s: kmalloc for option_port_private (%d) failed!.",
					__FUNCTION__, i);
			return 1;
		}

		init_waitqueue_head(&portdata->delta_msr_wait);

		for (j = 0; j < N_IN_URB; j++) {
			buffer = (u8 *)__get_free_page(GFP_KERNEL);
			if (!buffer)
				goto bail_out_error;
			portdata->in_buffer[j] = buffer;
		}

		for (j = 0; j < N_OUT_URB; j++) {
			buffer = kmalloc(OUT_BUFLEN, GFP_KERNEL);
			if (!buffer)
				goto bail_out_error2;
			portdata->out_buffer[j] = buffer;
		}

		usb_set_serial_port_data(port, portdata);

		if (!port->interrupt_in_urb)
			continue;
		err = usb_submit_urb(port->interrupt_in_urb);
		if (err)
			dbg("%s: submit irq_in urb failed %d",
				__FUNCTION__, err);
	}
	option_setup_urbs(serial);
	return 0;

bail_out_error2:
	for (j = 0; j < N_OUT_URB; j++)
		kfree(portdata->out_buffer[j]);
bail_out_error:
	for (j = 0; j < N_IN_URB; j++)
		if (portdata->in_buffer[j])
			free_page((unsigned long)portdata->in_buffer[j]);
	kfree(portdata);
	return -ENOMEM;
}

static void option_shutdown(struct usb_serial *serial)
{
	int i, j;
	struct usb_serial_port *port;
	struct option_port_private *portdata;

	dbg("%s", __FUNCTION__);

	/* Stop reading/writing urbs */
	for (i = 0; i < serial->num_ports; ++i) {
		port = &serial->port[i];
		portdata = usb_get_serial_port_data(port);

		for (j = 0; j < N_IN_URB; j++) {
			usb_unlink_urb(portdata->in_urbs[j]);
			usb_free_urb(portdata->in_urbs[j]);
			free_page((unsigned long)
					portdata->in_buffer[j]);
			portdata->in_urbs[j] = NULL;
		}
		for (j = 0; j < N_OUT_URB; j++) {
			usb_unlink_urb(portdata->out_urbs[j]);
			usb_free_urb(portdata->out_urbs[j]);
			kfree(portdata->out_buffer[j]);
			portdata->out_urbs[j] = NULL;
		}

		/* Now free per port private data */
		kfree(portdata);
	}
}

static int __init option_init(void)
{
	int retval;

	retval = usb_serial_register(&option_1port_device);
	if (retval)
		return retval;

	info(DRIVER_DESC ": " DRIVER_VERSION );

	return 0;
}

static void __exit option_exit(void)
{
	usb_serial_deregister(&option_1port_device);
}

module_init(option_init);
module_exit(option_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

MODULE_PARM(debug, "i");
MODULE_PARM_DESC(debug, "Debug messages");
