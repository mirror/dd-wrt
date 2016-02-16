/* Driver for USB Mass Storage compliant devices
 * Ununsual Devices File
 *
 * $Id: unusual_devs.h,v 1.1 2005/09/01 12:04:46 seg Exp $
 *
 * Current development and maintenance by:
 *   (c) 2000-2002 Matthew Dharm (mdharm-usb@one-eyed-alien.net)
 *
 * Initial work by:
 *   (c) 2000 Adam J. Richter (adam@yggdrasil.com), Yggdrasil Computing, Inc.
 *
 * Please see http://www.one-eyed-alien.net/~mdharm/linux-usb for more
 * information about this driver.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* IMPORTANT NOTE: This file must be included in another file which does
 * the following thing for it to work:
 * The macro UNUSUAL_DEV() must be defined before this file is included
 */
#include <linux/config.h>

/* If you edit this file, please try to keep it sorted first by VendorID,
 * then by ProductID.
 *
 * If you want to add an entry for this file, please send the following
 * to greg@kroah.com:
 *	- patch that adds the entry for your device which includes your
 *	  email address right above the entry.
 *	- a copy of /proc/bus/usb/devices with your device plugged in
 *	  running with this patch.
 *
 */

UNUSUAL_DEV(  0x03ee, 0x0000, 0x0000, 0x0245, 
		"Mitsumi",
		"CD-R/RW Drive",
		US_SC_8020, US_PR_CBI, NULL, 0), 

UNUSUAL_DEV(  0x03ee, 0x6901, 0x0000, 0x0200,
		"Mitsumi",
		"USB FDD",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN ),

UNUSUAL_DEV(  0x03f0, 0x0107, 0x0200, 0x0200, 
		"HP",
		"CD-Writer+",
		US_SC_8070, US_PR_CB, NULL, 0), 

#ifdef CONFIG_USB_STORAGE_HP8200e
UNUSUAL_DEV(  0x03f0, 0x0207, 0x0001, 0x0001, 
		"HP",
		"CD-Writer+ 8200e",
		US_SC_8070, US_PR_SCM_ATAPI, init_8200e, 0), 

UNUSUAL_DEV(  0x03f0, 0x0307, 0x0001, 0x0001, 
		"HP",
		"CD-Writer+ CD-4e",
		US_SC_8070, US_PR_SCM_ATAPI, init_8200e, 0), 
#endif


/* Reported by Gilles Espinasse <g.esp@free.fr> */
UNUSUAL_DEV(  0x03f0, 0x2001, 0x0001, 0x9999,
		"HP",
		"USB FDC GOLD-1.11",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN ),

/* Reported by Sebastian Kapfer <sebastian_kapfer@gmx.net>
 * and Olaf Hering <olh@suse.de> (different bcd's, same vendor/product)
 * for USB floppies that need the SINGLE_LUN enforcement.
 */
UNUSUAL_DEV(  0x0409, 0x0040, 0x0000, 0x9999,
		"NEC",
		"NEC USB UF000x",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN ),

/* Deduced by Jonathan Woithe <jwoithe@physics.adelaide.edu.au>
 * Entry needed for flags: US_FL_FIX_INQUIRY because initial inquiry message
 * always fails and confuses drive.
 */
UNUSUAL_DEV(  0x0411, 0x001c, 0x0113, 0x0113,
		"Buffalo",
		"DUB-P40G HDD",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),

/* Reported by Olaf Hering <olh@suse.de> from novell bug #105878 */
UNUSUAL_DEV(  0x0424, 0x0fdc, 0x0210, 0x0210,
		"SMSC",
		"FDC GOLD-2.30",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN ),

#ifdef CONFIG_USB_STORAGE_DPCM
UNUSUAL_DEV(  0x0436, 0x0005, 0x0100, 0x0100,
		"Microtech",
		"CameraMate (DPCM_USB)",
 		US_SC_SCSI, US_PR_DPCM_USB, NULL, 0 ),
#endif

/* Patch submitted by Philipp Friedrich <philipp@void.at> */
UNUSUAL_DEV(  0x0482, 0x0100, 0x0100, 0x0100,
		"Kyocera",
		"Finecam S3x",
		US_SC_8070, US_PR_CB, NULL, US_FL_FIX_INQUIRY),

/* Patch submitted by Philipp Friedrich <philipp@void.at> */
UNUSUAL_DEV(  0x0482, 0x0101, 0x0100, 0x0100,
		"Kyocera",
		"Finecam S4",
		US_SC_8070, US_PR_CB, NULL, US_FL_FIX_INQUIRY),

/* Patch submitted by Stephane Galles <stephane.galles@free.fr> */
UNUSUAL_DEV(  0x0482, 0x0103, 0x0100, 0x0100,
		"Kyocera",
		"Finecam S5",
		US_SC_DEVICE, US_PR_DEVICE, NULL, US_FL_FIX_INQUIRY),

/* Reported by Paul Stewart <stewart@wetlogic.net>
 * This entry is needed because the device reports Sub=ff */
UNUSUAL_DEV(  0x04a4, 0x0004, 0x0001, 0x0001,
		"Hitachi",
		"DVD-CAM DZ-MV100A Camcorder",
		US_SC_SCSI, US_PR_CB, NULL, US_FL_SINGLE_LUN),

/* Reported by Andreas Bockhold <andreas@bockionline.de> */
UNUSUAL_DEV(  0x04b0, 0x0405, 0x0100, 0x0100,
		"NIKON",
		"NIKON DSC D70",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY),

/* Reported by Jamie Kitson <jamie@staberinde.fsnet.co.uk> */
UNUSUAL_DEV(  0x04b0, 0x040d, 0x0100, 0x0100,
		"NIKON",
		"NIKON DSC D70s",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY),

/* Reported by Simon Levitt <simon@whattf.com>
 * This entry needs Sub and Proto fields */
UNUSUAL_DEV(  0x04b8, 0x0601, 0x0100, 0x0100,
		"Epson",
		"875DC Storage",
		US_SC_SCSI, US_PR_CB, NULL, US_FL_FIX_INQUIRY),

/* Reported by Khalid Aziz <khalid@gonehiking.org>
 * This entry is needed because the device reports Sub=ff */
UNUSUAL_DEV(  0x04b8, 0x0602, 0x0110, 0x0110,
		"Epson",
		"785EPX Storage",
		US_SC_SCSI, US_PR_BULK, NULL, US_FL_SINGLE_LUN),

/* Not sure who reported this originally but
 * Pavel Machek <pavel@ucw.cz> reported that the extra US_FL_SINGLE_LUN
 * flag be added */
UNUSUAL_DEV(  0x04cb, 0x0100, 0x0000, 0x2210,
		"Fujifilm",
		"FinePix 1400Zoom",
		US_SC_UFI, US_PR_DEVICE, NULL, US_FL_FIX_INQUIRY | US_FL_SINGLE_LUN),

/* Reported by Peter Wächtler <pwaechtler@loewe-komp.de>
 * The device needs the flags only.
 */
UNUSUAL_DEV(  0x04ce, 0x0002, 0x0074, 0x0074,
		"ScanLogic",
		"SL11R-IDE",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY),

/* Reported by Kriston Fincher <kriston@airmail.net>
 * Patch submitted by Sean Millichamp <sean@bruenor.org>
 * This is to support the Panasonic PalmCam PV-SD4090
 * This entry is needed because the device reports Sub=ff 
 */
UNUSUAL_DEV(  0x04da, 0x0901, 0x0100, 0x0200,
		"Panasonic",
		"LS-120 Camera",
		US_SC_UFI, US_PR_CBI, NULL, 0),

/* From Yukihiro Nakai, via zaitcev@yahoo.com.
 * This is needed for CB instead of CBI */
UNUSUAL_DEV(  0x04da, 0x0d05, 0x0000, 0x0000,
		"Sharp CE-CW05",
		"CD-R/RW Drive",
		US_SC_8070, US_PR_CB, NULL, 0),

/* Reported by Adriaan Penning <a.penning@luon.net>
 * Note that these cameras report "Medium not present" after
 * ALLOW_MEDIUM_REMOVAL, so they also need to be marked
 * NOT_LOCKABLE in the SCSI blacklist (and the vendor is MATSHITA). */
UNUSUAL_DEV(  0x04da, 0x2372, 0x0000, 0x9999,
		"Panasonic",
		"DMC-LCx Camera",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY ),

/* Most of the following entries were developed with the help of
 * Shuttle/SCM directly.
 */
UNUSUAL_DEV(  0x04e6, 0x0001, 0x0200, 0x0200, 
		"Matshita",
		"LS-120",
		US_SC_8020, US_PR_CB, NULL, 0),

UNUSUAL_DEV(  0x04e6, 0x0002, 0x0100, 0x0100, 
		"Shuttle",
		"eUSCSI Bridge",
		US_SC_SCSI, US_PR_BULK, usb_stor_euscsi_init, 
		US_FL_SCM_MULT_TARG ), 

#ifdef CONFIG_USB_STORAGE_SDDR09
UNUSUAL_DEV(  0x04e6, 0x0003, 0x0000, 0x9999, 
		"Sandisk",
		"ImageMate SDDR09",
		US_SC_SCSI, US_PR_EUSB_SDDR09, NULL,
		US_FL_SINGLE_LUN ),

/* This entry is from Andries.Brouwer@cwi.nl */
UNUSUAL_DEV(  0x04e6, 0x0005, 0x0100, 0x0208,
		"SCM Microsystems",
		"eUSB SmartMedia / CompactFlash Adapter",
		US_SC_SCSI, US_PR_DPCM_USB, sddr09_init, 
		0), 
#endif

UNUSUAL_DEV(  0x04e6, 0x0006, 0x0100, 0x0205, 
		"Shuttle",
		"eUSB MMC Adapter",
		US_SC_SCSI, US_PR_DEVICE, NULL, 
		US_FL_SINGLE_LUN), 

UNUSUAL_DEV(  0x04e6, 0x0007, 0x0100, 0x0200, 
		"Sony",
		"Hifd",
		US_SC_SCSI, US_PR_CB, NULL, 
		US_FL_SINGLE_LUN), 

UNUSUAL_DEV(  0x04e6, 0x0009, 0x0200, 0x0200, 
		"Shuttle",
		"eUSB ATA/ATAPI Adapter",
		US_SC_8020, US_PR_CB, NULL, 0),

UNUSUAL_DEV(  0x04e6, 0x000a, 0x0200, 0x0200, 
		"Shuttle",
		"eUSB CompactFlash Adapter",
		US_SC_8020, US_PR_CB, NULL, 0),

UNUSUAL_DEV(  0x04e6, 0x000B, 0x0100, 0x0100, 
		"Shuttle",
		"eUSCSI Bridge",
		US_SC_SCSI, US_PR_BULK, usb_stor_euscsi_init, 
		US_FL_SCM_MULT_TARG ), 

UNUSUAL_DEV(  0x04e6, 0x000C, 0x0100, 0x0100, 
		"Shuttle",
		"eUSCSI Bridge",
		US_SC_SCSI, US_PR_BULK, usb_stor_euscsi_init, 
		US_FL_SCM_MULT_TARG ), 

UNUSUAL_DEV(  0x04e6, 0x0101, 0x0200, 0x0200, 
		"Shuttle",
		"CD-RW Device",
		US_SC_8020, US_PR_CB, NULL, 0),

/* Reported by Bob Sass <rls@vectordb.com> -- only rev 1.33 tested */
UNUSUAL_DEV(  0x050d, 0x0115, 0x0133, 0x0133,
		"Belkin",
		"USB SCSI Adaptor",
		US_SC_SCSI, US_PR_BULK, usb_stor_euscsi_init,
		US_FL_SCM_MULT_TARG ),

/* Iomega Clik! Drive 
 * Reported by David Chatenay <dchatenay@hotmail.com>
 * The reason this is needed is not fully known.
 */
UNUSUAL_DEV(  0x0525, 0xa140, 0x0100, 0x0100,
		"Iomega",
		"USB Clik! 40",
		US_SC_8070, US_PR_BULK, NULL,
		US_FL_FIX_INQUIRY ),

/* This entry is needed because the device reports Sub=ff */
UNUSUAL_DEV(  0x054c, 0x0010, 0x0106, 0x0450, 
		"Sony",
		"DSC-S30/S70/S75/505V/F505/F707/F717/P8", 
		US_SC_SCSI, US_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN | US_FL_MODE_XLATE ),

/* Submitted by Lars Jacob <jacob.lars@googlemail.com>
 * This entry is needed because the device reports Sub=ff */
UNUSUAL_DEV(  0x054c, 0x0010, 0x0500, 0x0610,
		"Sony",
		"DSC-T1/T5/H5",
		US_SC_8070, US_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN ),

/* Reported by wim@geeks.nl */
UNUSUAL_DEV(  0x054c, 0x0025, 0x0100, 0x0100, 
		"Sony",
		"Memorystick NW-MS7",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN ),

#ifdef CONFIG_USB_STORAGE_ISD200
UNUSUAL_DEV(  0x054c, 0x002b, 0x0100, 0x0110,
		"Sony",
		"Portable USB Harddrive V2",
		US_SC_ISD200, US_PR_BULK, isd200_Initialization,
		0 ),
#endif

UNUSUAL_DEV(  0x054c, 0x002d, 0x0100, 0x0100, 
		"Sony",
		"Memorystick MSAC-US1",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN ),

/* Submitted by Klaus Mueller <k.mueller@intershop.de> */
UNUSUAL_DEV(  0x054c, 0x002e, 0x0106, 0x0310, 
		"Sony",
		"Handycam",
		US_SC_SCSI, US_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN | US_FL_MODE_XLATE),

/* Submitted by Klaus Mueller <k.mueller@intershop.de> */
UNUSUAL_DEV(  0x054c, 0x002e, 0x0106, 0x0310,
		"Sony",
		"Handycam",
		US_SC_SCSI, US_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN ),

/* Submitted by Rajesh Kumble Nayak <nayak@obs-nice.fr> */
UNUSUAL_DEV(  0x054c, 0x002e, 0x0500, 0x0500, 
		"Sony",
		"Handycam HC-85",
		US_SC_UFI, US_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN | US_FL_MODE_XLATE),

UNUSUAL_DEV(  0x054c, 0x0032, 0x0000, 0x9999,
		"Sony",
		"Memorystick MSC-U01N",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN ),

/* Submitted by Michal Mlotek <mlotek@foobar.pl> */
UNUSUAL_DEV(  0x054c, 0x0058, 0x0000, 0x9999,
		"Sony",
		"PEG N760c Memorystick",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),
		
UNUSUAL_DEV(  0x054c, 0x0069, 0x0000, 0x9999,
		"Sony",
		"Memorystick MSC-U03",
		US_SC_UFI, US_PR_CB, NULL,
		US_FL_SINGLE_LUN ),

/* Submitted by Nathan Babb <nathan@lexi.com> */
UNUSUAL_DEV(  0x054c, 0x006d, 0x0000, 0x9999,
		"Sony",
		"PEG Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),

/* Submitted by Mike Alborn <malborn@deandra.homeip.net> */
UNUSUAL_DEV(  0x054c, 0x016a, 0x0000, 0x9999,
		"Sony",
		"PEG Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),

/* Submitted by Frank Engel <frankie@cse.unsw.edu.au> */
UNUSUAL_DEV(  0x054c, 0x0099, 0x0000, 0x9999,
		"Sony",
		"PEG Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),

/* floppy reports multiple luns */
UNUSUAL_DEV(  0x055d, 0x2020, 0x0000, 0x0210,
		"SAMSUNG",
		"SFD-321U [FW 0C]",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN ),

UNUSUAL_DEV(  0x057b, 0x0000, 0x0000, 0x0299, 
		"Y-E Data",
		"Flashbuster-U",
		US_SC_DEVICE,  US_PR_CB, NULL,
		US_FL_SINGLE_LUN),

UNUSUAL_DEV(  0x057b, 0x0000, 0x0300, 0x9999, 
		"Y-E Data",
		"Flashbuster-U",
		US_SC_DEVICE,  US_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN),

/* Reported by Johann Cardon <johann.cardon@free.fr>
 * This entry is needed only because the device reports
 * bInterfaceClass = 0xff (vendor-specific)
 */
UNUSUAL_DEV(  0x057b, 0x0022, 0x0000, 0x9999, 
		"Y-E Data",
		"Silicon Media R/W",
		US_SC_DEVICE, US_PR_DEVICE, NULL, 0),

/* Fabrizio Fellini <fello@libero.it> */
UNUSUAL_DEV(  0x0595, 0x4343, 0x0000, 0x2210,
		"Fujifilm",
		"Digital Camera EX-20 DSC",
		US_SC_8070, US_PR_CBI, NULL, 0 ),

UNUSUAL_DEV(  0x059f, 0xa601, 0x0200, 0x0200, 
		"LaCie",
		"USB Hard Disk",
		US_SC_RBC, US_PR_CB, NULL, 0 ), 

#ifdef CONFIG_USB_STORAGE_ISD200
UNUSUAL_DEV(  0x05ab, 0x0031, 0x0100, 0x0110,
		"In-System",
		"USB/IDE Bridge (ATA/ATAPI)",
		US_SC_ISD200, US_PR_BULK, isd200_Initialization,
		0 ),

UNUSUAL_DEV(  0x05ab, 0x0301, 0x0100, 0x0110,
		"In-System",
		"Portable USB Harddrive V2",
		US_SC_ISD200, US_PR_BULK, isd200_Initialization,
		0 ),

UNUSUAL_DEV(  0x05ab, 0x0351, 0x0100, 0x0110,
		"In-System",
		"Portable USB Harddrive V2",
		US_SC_ISD200, US_PR_BULK, isd200_Initialization,
		0 ),

UNUSUAL_DEV(  0x05ab, 0x5701, 0x0100, 0x0110,
		"In-System",
		"USB Storage Adapter V2",
		US_SC_ISD200, US_PR_BULK, isd200_Initialization,
		0 ),
#endif

/* Submitted by Sven Anderson <sven-linux@anderson.de>
 * There are at least four ProductIDs used for iPods, so I added 0x1202 and
 * 0x1204. They just need the US_FL_FIX_CAPACITY. As the bcdDevice appears
 * to change with firmware updates, I changed the range to maximum for all
 * iPod entries.
 */
UNUSUAL_DEV( 0x05ac, 0x1202, 0x0000, 0x9999,
		"Apple",
		"iPod",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY ),

/* Reported by Avi Kivity <avi@argo.co.il> */
UNUSUAL_DEV( 0x05ac, 0x1203, 0x0000, 0x9999,
		"Apple",
		"iPod",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY ),

UNUSUAL_DEV( 0x05ac, 0x1205, 0x0000, 0x9999,
		"Apple",
		"iPod",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY ),

/*
 * Reported by Tyson Vinson <lornoss@gmail.com>
 * This particular productId is the iPod Nano
 */
UNUSUAL_DEV( 0x05ac, 0x120a, 0x0000, 0x9999,
		"Apple",
		"iPod",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY ),

#ifdef CONFIG_USB_STORAGE_JUMPSHOT
UNUSUAL_DEV(  0x05dc, 0x0001, 0x0000, 0x0001,
		"Lexar",
		"Jumpshot USB CF Reader",
		US_SC_SCSI, US_PR_JUMPSHOT, NULL,
		US_FL_MODE_XLATE ),
#endif

/* Reported by Blake Matheny <bmatheny@purdue.edu> */
UNUSUAL_DEV(  0x05dc, 0xb002, 0x0000, 0x0113,
		"Lexar",
		"USB CF Reader",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),

/* Reported by Carlos Villegas <cav@uniscope.co.jp>
 * This device needs an INQUIRY of exactly 36-bytes to function.
 * That is the only reason this entry is needed.
 */
UNUSUAL_DEV(  0x05e3, 0x0700, 0x0000, 0xffff,
		"Genesys Logic",
		"CompactFlash Card Reader",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),

/* The following two entries are for a Genesys USB to IDE
 * converter chip, but it changes its ProductId depending
 * on whether or not a disk or an optical device is enclosed
 * They were originally reported by Alexander Oltu
 * <alexander@all-2.com> and Peter Marks <peter.marks@turner.com>
 * respectively.
 */
UNUSUAL_DEV(  0x05e3, 0x0701, 0x0000, 0xffff, 
		"Genesys Logic",
		"USB to IDE Optical",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_MODE_XLATE ), 

/* Reported by Peter Marks <peter.marks@turner.com>
 * Like the Genesys Logic unit above, this unit needs an INQUIRY to ask for exactly
 * 36 bytes of data.  No more, no less. That is the only reason this entry
 * is needed.
 *
 * ST818 slim drives (rev 0.02) don't need special care.
*/
UNUSUAL_DEV(  0x05e3, 0x0702, 0x0000, 0x0001,
		"Genesys Logic",
		"USB to IDE Disk",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),

/* Reported by Hanno Boeck <hanno@gmx.de>
 * Taken from the Lycoris Kernel */
UNUSUAL_DEV(  0x0636, 0x0003, 0x0000, 0x9999,
		"Vivitar",
		"Vivicam 35Xx",
		US_SC_SCSI, US_PR_BULK, NULL,
		US_FL_FIX_INQUIRY | US_FL_MODE_XLATE),

UNUSUAL_DEV(  0x0644, 0x0000, 0x0100, 0x0100, 
		"TEAC",
		"Floppy Drive",
		US_SC_UFI, US_PR_CB, NULL, 0 ), 

#ifdef CONFIG_USB_STORAGE_SDDR09
UNUSUAL_DEV(  0x066b, 0x0105, 0x0100, 0x0100, 
		"Olympus",
		"Camedia MAUSB-2",
		US_SC_SCSI, US_PR_EUSB_SDDR09, NULL,
		US_FL_SINGLE_LUN ),
#endif

/* Reported by Darsen Lu <darsen@micro.ee.nthu.edu.tw> */
UNUSUAL_DEV( 0x066f, 0x8000, 0x0001, 0x0001,
		"SigmaTel",
		"USBMSC Audio Player",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY ),

/* Submitted by Benny Sjostrand <benny@hostmobility.com> */
UNUSUAL_DEV( 0x0686, 0x4011, 0x0001, 0x0001,
		"Minolta",
		"Dimage F300",
		US_SC_SCSI, US_PR_BULK, NULL, 0 ),

/* Reported by Miguel A. Fosas <amn3s1a@ono.com> */
UNUSUAL_DEV(  0x0686, 0x4017, 0x0001, 0x0001,
                "Minolta",
                "DIMAGE E223",
                US_SC_SCSI, US_PR_DEVICE, NULL, 0 ),

/* Following three Minolta cameras reported by Martin Pool
 * <mbp@sourcefrog.net>.  Originally discovered by Kedar Petankar,
 * Matthew Geier, Mikael Lofj"ard, Marcel de Boer.
 */
UNUSUAL_DEV( 0x0686, 0x4006, 0x0001, 0x0001,
             "Minolta",
             "DiMAGE 7",
             US_SC_SCSI, US_PR_DEVICE, NULL,
             0 ),

UNUSUAL_DEV( 0x0686, 0x400b, 0x0001, 0x0001,
             "Minolta",
             "DiMAGE 7i",
             US_SC_SCSI, US_PR_DEVICE, NULL,
             0 ),

UNUSUAL_DEV( 0x0686, 0x400f, 0x0001, 0x0001,
             "Minolta",
             "DiMAGE 7Hi",
             US_SC_SCSI, US_PR_DEVICE, NULL,
             0 ),

UNUSUAL_DEV(  0x0693, 0x0002, 0x0100, 0x0100, 
		"Hagiwara",
		"FlashGate SmartMedia",
		US_SC_SCSI, US_PR_BULK, NULL, 0 ),

UNUSUAL_DEV(  0x0693, 0x0005, 0x0100, 0x0100,
		"Hagiwara",
		"Flashgate",
		US_SC_SCSI, US_PR_BULK, NULL, 0 ), 

/* Reported by David Hamilton <niftimusmaximus@lycos.com> */
UNUSUAL_DEV(  0x069b, 0x3004, 0x0001, 0x0001,
		"Thomson Multimedia Inc.",
		"RCA RD1080 MP3 Player",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY ),

UNUSUAL_DEV(  0x0781, 0x0001, 0x0200, 0x0200, 
		"Sandisk",
		"ImageMate SDDR-05a",
		US_SC_SCSI, US_PR_CB, NULL,
		US_FL_SINGLE_LUN ),

UNUSUAL_DEV(  0x0781, 0x0002, 0x0009, 0x0009, 
		"Sandisk",
		"ImageMate SDDR-31",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_IGNORE_SER ),

UNUSUAL_DEV(  0x0781, 0x0100, 0x0100, 0x0100,
		"Sandisk",
		"ImageMate SDDR-12",
		US_SC_SCSI, US_PR_CB, NULL,
		US_FL_SINGLE_LUN ),

/* Reported by Kevin Lloyd <linux@sierrawireless.com>
 * Entry is needed for the initializer function override,
 * which instructs the device to load as a modem
 * device.
 */
UNUSUAL_DEV(  0x1199, 0x0fff, 0x0000, 0x9999,
		"Sierra Wireless",
		"USB MMC Storage",
		US_SC_DEVICE, US_PR_DEVICE, sierra_set_ms_mode,
		0),

UNUSUAL_DEV( 0x19d2, 0x2000, 0x0000, 0x0000,
                "HSDPA Support",
                "HSDPA Mass Storage",
                US_SC_DEVICE, US_PR_DEVICE,usb_stor_huawei_e220_init, 
                0),

UNUSUAL_DEV(  0x12d1, 0x1001, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1003, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1004, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1401, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1402, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1403, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1404, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1405, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1406, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1407, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1408, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1409, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x140A, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x140B, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x140C, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x140D, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x140E, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x140F, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1410, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1411, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1412, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1413, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1414, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1415, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1416, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1417, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1418, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1419, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x141A, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x141B, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x141C, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x141D, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x141E, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x141F, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1420, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1421, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1422, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1423, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1424, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1425, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1426, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1427, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1428, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1429, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x142A, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x142B, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x142C, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x142D, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x142E, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x142F, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1430, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1431, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1432, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1433, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1434, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1435, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1436, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1437, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1438, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1439, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x143A, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x143B, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x143C, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x143D, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x143E, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x143F, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_huawei_e220_init,
		0),


#ifdef CONFIG_USB_STORAGE_SDDR09
UNUSUAL_DEV(  0x0781, 0x0200, 0x0000, 0x9999, 
		"Sandisk",
		"ImageMate SDDR-09",
		US_SC_SCSI, US_PR_EUSB_SDDR09, NULL,
		US_FL_SINGLE_LUN ),
#endif

#ifdef CONFIG_USB_STORAGE_FREECOM
UNUSUAL_DEV(  0x07ab, 0xfc01, 0x0000, 0x9999,
		"Freecom",
		"USB-IDE",
		US_SC_QIC, US_PR_FREECOM, freecom_init, 0),
#endif

/* Reported by Eero Volotinen <eero@ping-viini.org> */
UNUSUAL_DEV(  0x07ab, 0xfccd, 0x0000, 0x9999,
		"Freecom Technologies",
		"FHD-Classic",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY),

UNUSUAL_DEV(  0x07af, 0x0004, 0x0100, 0x0133, 
		"Microtech",
		"USB-SCSI-DB25",
		US_SC_SCSI, US_PR_BULK, usb_stor_euscsi_init,
		US_FL_SCM_MULT_TARG ), 

UNUSUAL_DEV(  0x07af, 0x0005, 0x0100, 0x0100, 
		"Microtech",
		"USB-SCSI-HD50",
		US_SC_SCSI, US_PR_BULK, usb_stor_euscsi_init,
		US_FL_SCM_MULT_TARG ), 

#ifdef CONFIG_USB_STORAGE_DPCM
UNUSUAL_DEV(  0x07af, 0x0006, 0x0100, 0x0100,
		"Microtech",
		"CameraMate (DPCM_USB)",
 		US_SC_SCSI, US_PR_DPCM_USB, NULL, 0 ),
#endif

#ifdef CONFIG_USB_STORAGE_DATAFAB
UNUSUAL_DEV(  0x07c4, 0xa000, 0x0000, 0x0015,
		"Datafab",
		"MDCFE-B USB CF Reader",
		US_SC_SCSI, US_PR_DATAFAB, NULL,
		US_FL_MODE_XLATE ),

	/*
	 * The following Datafab-based devices may or may not work
	 * using the current driver...the 0xffff is arbitrary since I
	 * don't know what device versions exist for these guys.
	 *
	 * The 0xa003 and 0xa004 devices in particular I'm curious about.
	 * I'm told they exist but so far nobody has come forward to say that
	 * they work with this driver.  Given the success we've had getting
	 * other Datafab-based cards operational with this driver, I've decided
	 * to leave these two devices in the list.
	 */
UNUSUAL_DEV( 0x07c4, 0xa001, 0x0000, 0xffff,
		"SIIG/Datafab",
		"SIIG/Datafab Memory Stick+CF Reader/Writer",
		US_SC_SCSI, US_PR_DATAFAB, NULL,
		US_FL_MODE_XLATE ),

UNUSUAL_DEV( 0x07c4, 0xa003, 0x0000, 0xffff,
		"Datafab/Unknown",
		"Datafab-based Reader",
		US_SC_SCSI, US_PR_DATAFAB, NULL,
		US_FL_MODE_XLATE ),

UNUSUAL_DEV( 0x07c4, 0xa004, 0x0000, 0xffff,
		"Datafab/Unknown",
		"Datafab-based Reader",
		US_SC_SCSI, US_PR_DATAFAB, NULL,
		US_FL_MODE_XLATE ),

UNUSUAL_DEV( 0x07c4, 0xa005, 0x0000, 0xffff,
		"PNY/Datafab",
		"PNY/Datafab CF+SM Reader",
		US_SC_SCSI, US_PR_DATAFAB, NULL,
		US_FL_MODE_XLATE ),

UNUSUAL_DEV( 0x07c4, 0xa006, 0x0000, 0xffff,
		"Simple Tech/Datafab",
		"Simple Tech/Datafab CF+SM Reader",
		US_SC_SCSI, US_PR_DATAFAB, NULL,
		US_FL_MODE_XLATE ),
#endif
		
#ifdef CONFIG_USB_STORAGE_SDDR55
/* Contributed by Peter Waechtler */
UNUSUAL_DEV( 0x07c4, 0xa103, 0x0000, 0x9999,
		"Datafab",
		"MDSM-B reader",
		US_SC_SCSI, US_PR_SDDR55, NULL,
		US_FL_FIX_INQUIRY ),
#endif

#ifdef CONFIG_USB_STORAGE_DATAFAB
/* Submitted by Olaf Hering <olh@suse.de> */
UNUSUAL_DEV(  0x07c4, 0xa109, 0x0000, 0xffff,
		"Datafab Systems, Inc.",
		"USB to CF + SM Combo (LC1)",
		US_SC_SCSI, US_PR_DATAFAB, NULL,
		US_FL_MODE_XLATE ),
#endif
#ifdef CONFIG_USB_STORAGE_SDDR55
/* SM part - aeb <Andries.Brouwer@cwi.nl> */
UNUSUAL_DEV(  0x07c4, 0xa109, 0x0000, 0xffff,
		"Datafab Systems, Inc.",
		"USB to CF + SM Combo (LC1)",
		US_SC_SCSI, US_PR_SDDR55, NULL,
		US_FL_SINGLE_LUN ),
#endif

#ifdef CONFIG_USB_STORAGE_DATAFAB
/* Reported by Felix Moeller <felix@derklecks.de>
 * in Germany this is sold by Hama with the productnumber 46952
 * as "DualSlot CompactFlash(TM) & MStick Drive USB"
 */
UNUSUAL_DEV(  0x07c4, 0xa10b, 0x0000, 0xffff,
		"DataFab Systems Inc.",
		"USB CF+MS",
		US_SC_SCSI, US_PR_DATAFAB, NULL,
		0 ),

#endif

/* Datafab KECF-USB / Sagatek DCS-CF / Simpletech Flashlink UCF-100
 * Only revision 1.13 tested (same for all of the above devices,
 * based on the Datafab DF-UG-07 chip).  Needed for US_FL_FIX_INQUIRY.
 * Submitted by Marek Michalkiewicz <marekm@amelek.gda.pl>.
 * See also http://martin.wilck.bei.t-online.de/#kecf .
 */
UNUSUAL_DEV(  0x07c4, 0xa400, 0x0000, 0xffff,
		"Datafab",
		"KECF-USB",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),

/* Casio QV 2x00/3x00/4000/8000 digital still cameras are not conformant
 * to the USB storage specification in two ways:
 * - They tell us they are using transport protocol CBI. In reality they
 *   are using transport protocol CB.
 * - They don't like the INQUIRY command. So we must handle this command
 *   of the SCSI layer ourselves.
 */
UNUSUAL_DEV( 0x07cf, 0x1001, 0x1000, 0x9999,
		"Casio",
		"QV DigitalCamera",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),

/* Submitted by Hartmut Wahl <hwahl@hwahl.de>*/
UNUSUAL_DEV( 0x0839, 0x000a, 0x0001, 0x0001,
		"Samsung",
		"Digimax 410",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY),

/* Aiptek PocketCAM 3Mega
 * Nicolas DUPEUX <nicolas@dupeux.net> 
 */
UNUSUAL_DEV(  0x08ca, 0x2011, 0x0000, 0x9999,
		"AIPTEK",
		"PocketCAM 3Mega",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_MODE_XLATE ),

/* Submitted by Jan De Luyck <lkml@kcore.org> */
UNUSUAL_DEV(  0x08bd, 0x1100, 0x0000, 0x0000,
		"CITIZEN",
		"X1DE-USB",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN),

/*Medion 6047 Digital Camera
Davide Andrian <_nessuno_@katamail.com>
*/
UNUSUAL_DEV( 0x08ca, 0x2011, 0x0001, 0x0001,
		"3MegaCam",
		"3MegaCam",
		US_SC_DEVICE, US_PR_BULK, NULL,
		US_FL_MODE_XLATE ),

/* Entry needed for flags. Moreover, all devices with this ID use
 * bulk-only transport, but _some_ falsely report Control/Bulk instead.
 * One example is "Trumpion Digital Research MYMP3".
 * Submitted by Bjoern Brill <brill(at)fs.math.uni-frankfurt.de>
 */
UNUSUAL_DEV(  0x090a, 0x1001, 0x0100, 0x0100,
		"Trumpion",
		"t33520 USB Flash Card Controller",
		US_SC_DEVICE, US_PR_BULK, NULL,
		US_FL_MODE_XLATE),

/* Trumpion Microelectronics MP3 player (felipe_alfaro@linuxmail.org) */
UNUSUAL_DEV( 0x090a, 0x1200, 0x0000, 0x9999,
		"Trumpion",
		"MP3 player",
		US_SC_RBC, US_PR_BULK, NULL,
		US_FL_MODE_XLATE),

/* aeb */
UNUSUAL_DEV( 0x090c, 0x1132, 0x0000, 0xffff,
		"Feiya",
		"5-in-1 Card Reader",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY ),

UNUSUAL_DEV(  0x097a, 0x0001, 0x0000, 0x0001,
		"Minds@Work",
		"Digital Wallet",
 		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_MODE_XLATE ),

UNUSUAL_DEV(  0x0a16, 0x8888, 0x0100, 0x0100,
		"IBM",
		"IBM USB Memory Key",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),

/* This Pentax still camera is not conformant
 * to the USB storage specification: -
 * - It does not like the INQUIRY command. So we must handle this command
 *   of the SCSI layer ourselves.
 * Tested on Rev. 10.00 (0x1000)
 * Submitted by James Courtier-Dutton <James@superbug.demon.co.uk>
 */
UNUSUAL_DEV( 0x0a17, 0x0004, 0x1000, 0x1000,
                "Pentax",
                "Optio 2/3/400",
                US_SC_DEVICE, US_PR_DEVICE, NULL,
                US_FL_FIX_INQUIRY ),


/* Submitted by Per Winkvist <per.winkvist@uk.com> */
UNUSUAL_DEV( 0x0a17, 0x006, 0x0000, 0xffff,
                "Pentax",
                "Optio S/S4",
                US_SC_DEVICE, US_PR_DEVICE, NULL,
                US_FL_FIX_INQUIRY ),
		
#ifdef CONFIG_USB_STORAGE_ISD200
UNUSUAL_DEV(  0x0bf6, 0xa001, 0x0100, 0x0110,
		"ATI",
		"USB Cable 205",
		US_SC_ISD200, US_PR_BULK, isd200_Initialization,
		0 ),
#endif

#ifdef CONFIG_USB_STORAGE_DATAFAB
UNUSUAL_DEV( 0x0c0b, 0xa109, 0x0000, 0xffff,
		"Acomdata",
		"CF",
		US_SC_SCSI, US_PR_DATAFAB, NULL,
		US_FL_SINGLE_LUN ),
#endif
#ifdef CONFIG_USB_STORAGE_SDDR55
UNUSUAL_DEV( 0x0c0b, 0xa109, 0x0000, 0xffff,
		"Acomdata",
		"SM",
		US_SC_SCSI, US_PR_SDDR55, NULL,
		US_FL_SINGLE_LUN ),
#endif

/*
 * Pete Zaitcev <zaitcev@yahoo.com>, bz#164688.
 * The device blatantly ignores LUN and returns 1 in GetMaxLUN.
 */
UNUSUAL_DEV( 0x0c45, 0x1060, 0x0100, 0x0100,
		"Unknown",
		"Unknown",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN ),

/* Submitted by Joris Struyve <joris@struyve.be> */
UNUSUAL_DEV( 0x0d96, 0x410a, 0x0001, 0xffff,
		"Medion",
		"MD 7425",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY),

/*
 * Entry for Jenoptik JD 5200z3
 *
 * email: car.busse@gmx.de
 */
UNUSUAL_DEV(  0x0d96, 0x5200, 0x0001, 0x0200,
		"Jenoptik",
		"JD 5200 z3",
		US_SC_DEVICE, US_PR_DEVICE, NULL, US_FL_FIX_INQUIRY),

/* Reported by Lubomir Blaha <tritol@trilogic.cz>
 * I _REALLY_ don't know what 3rd, 4th number and all defines mean, but this
 * works for me. Can anybody correct these values? (I able to test corrected
 * version.)
 */
UNUSUAL_DEV( 0x0dd8, 0x1060, 0x0000, 0xffff,
		"Netac",
		"USB-CF-Card",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),

/* Submitted by Antoine Mairesse <antoine.mairesse@free.fr> */
UNUSUAL_DEV( 0x0ed1, 0x6660, 0x0100, 0x0300,
		"USB",
		"Solid state disk",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),

/* Reported by Jan Mate <mate@fiit.stuba.sk> */
UNUSUAL_DEV(  0x0fce, 0xe030, 0x0000, 0x0000,
		"Sony Ericsson",
		"P990i",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY ),

/* Reported by Emmanuel Vasilakis <evas@forthnet.gr> */
UNUSUAL_DEV(  0x0fce, 0xe031, 0x0000, 0x0000,
		"Sony Ericsson",
		"M600i",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY ),

/* Reported by Kevin Cernekee <kpc-usbdev@gelato.uiuc.edu>
 * Tested on hardware version 1.10.
 * Entry is needed only for the initializer function override.
 */
UNUSUAL_DEV(  0x1019, 0x0c55, 0x0000, 0x9999,
		"Desknote",
		"UCR-61S2B",
		US_SC_DEVICE, US_PR_DEVICE, usb_stor_ucr61s2b_init,
		0 ),

/* Reported by Dan Pilone <pilone@slac.com>
 * The device needs the flags only.
 * Also reported by Brian Hall <brihall@pcisys.net>, again for flags.
 * I also suspect this device may have a broken serial number.
 */
UNUSUAL_DEV(  0x1065, 0x2136, 0x0000, 0x9999,
		"CCYU TECHNOLOGY",
		"EasyDisk Portable Device",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_MODE_XLATE ),

/* Reported by Vilius Bilinkevicius <vilisas AT xxx DOT lt) */
UNUSUAL_DEV(  0x132b, 0x000b, 0x0001, 0x0001,
		"Minolta",
		"Dimage Z10",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		0 ),

#ifdef CONFIG_USB_STORAGE_SDDR55
UNUSUAL_DEV(  0x55aa, 0xa103, 0x0000, 0x9999, 
		"Sandisk",
		"ImageMate SDDR55",
		US_SC_SCSI, US_PR_SDDR55, NULL,
		US_FL_SINGLE_LUN),
#endif

/* Patch for Kyocera Finecam L3
 * Submitted by Michael Krauth <michael.krauth@web.de>
 */
UNUSUAL_DEV(  0x0482, 0x0105, 0x0100, 0x0100,
		"Kyocera",
		"Finecam L3",
		US_SC_SCSI, US_PR_BULK, NULL,
		US_FL_FIX_INQUIRY),

/* Reported by Thomas Baechler <thomas@archlinux.org>
 * Fixes I/O errors with Teac HD-35PU devices
 * svr: last param was US_FL_IGNORE_RESIDUE, but unknown in 2.4
 */
UNUSUAL_DEV(  0x1652, 0x6600, 0x0201, 0x0201,
		"Super Top",
		"USB 2.0  IDE DEVICE",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		0),
