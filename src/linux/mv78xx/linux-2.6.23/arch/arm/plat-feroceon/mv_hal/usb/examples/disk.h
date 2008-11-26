/*******************************************************************************

This software file (the "File") is distributed by Marvell International Ltd. 
or its affiliate(s) under the terms of the GNU General Public License Version 2, 
June 1991 (the "License").  You may use, redistribute and/or modify this File 
in accordance with the terms and conditions of the License, a copy of which 
is available along with the File in the license.txt file or by writing to the 
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 
or on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.

(C) Copyright 2004 - 2007 Marvell Semiconductor Israel Ltd. All Rights Reserved.
(C) Copyright 1999 - 2004 Chipidea Microelectronica, S.A. All Rights Reserved.

*******************************************************************************/

#ifndef __disk_h__
#define __disk_h__

#define  USB_DCBWSIGNATURE       (0x43425355)
#define  USB_DCSWSIGNATURE       (0x53425355)
#define  USB_CBW_DIRECTION_BIT   (0x80)

/* USB Command Block Wrapper */
typedef struct cbw_struct {
   uint_32  DCBWSIGNATURE;
   uint_32  DCBWTAG;
   uint_32  DCBWDATALENGTH;
   uint_8   BMCBWFLAGS;
   /* 4 MSBs bits reserved */
   uint_8   BCBWCBLUN;
   /* 3 MSB reserved */
   uint_8   BCBWCBLENGTH;
   uint_8   CBWCB[16];
} CBW_STRUCT, _PTR_ CBW_STRUCT_PTR;

/* USB Command Status Wrapper */
typedef struct csw_struct {
   uint_32  DCSWSIGNATURE;
   uint_32  DCSWTAG;
   uint_32  DCSWDATARESIDUE;
   uint_8   BCSWSTATUS;
} CSW_STRUCT, _PTR_ CSW_STRUCT_PTR;

/* USB Mass storage Inquiry Command */
typedef struct mass_storage_inquiry {
   uint_8   OPCODE;
   uint_8   LUN;
   uint_8   PAGE_CODE;
   uint_8   RESERVED1;
   uint_8   ALLOCATION_LENGTH;
   uint_8   RESERVED2[7];
} DISK_INQUIRY, _PTR_ DISK_INQUIRY_PTR;

/* USB Mass storage READ CAPACITY Data */
typedef struct mass_storage_read_capacity {
   uint_8   LAST_LOGICAL_BLOCK_ADDRESS[4];
   uint_8   BLOCK_LENGTH_IN_BYTES[4];
} DISK_READ_CAPACITY, _PTR_ DISK_READ_CAPACITY_PTR;

/* USB Mass storage Device information */
typedef struct mass_storage_device_info {
   uint_8   PERIPHERAL_DEVICE_TYPE;    /* Bits 0-4. All other bits reserved */
   uint_8   RMB;                       /* Bit 7. All other bits reserved */
   uint_8   ANSI_ECMA_ISO_VERSION;     /* ANSI: bits 0-2, ECMA: bits 3-5, 
                                       ** ISO: bits 6-7 
                                       */
   uint_8   RESPONSE_DATA_FORMAT;      /* bits 0-3. All other bits reserved */
   uint_8   ADDITIONAL_LENGTH;         /* For UFI device: always set to 0x1F */
   uint_8   RESERVED1[3];
   uint_8   VENDOR_INFORMATION[8];
   uint_8   PRODUCT_ID[16];
   uint_8   PRODUCT_REVISION_LEVEL[4];
} DISK_DEVICE_INFO, _PTR_ DISK_DEVICE_INFO_PTR;


extern _usb_device_handle  usbDiskLoad(int devNo, int diskSize);
extern void                usbDiskUnload(_usb_device_handle  handle);

#endif /* __disk_h__ */

/* EOF */
