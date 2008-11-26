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

/**************************************************************************
Include the USB stack and local header files.
**************************************************************************/

#include "mvUsbDefs.h"
#include "mvUsbCh9.h"
#include "mvUsbDebug.h"
#include "mvUsbDevApi.h"
#include "disk.h"

/* MSB of debug flags for USB device usage */
#define ARC_DEBUG_FLAG_DISK         0x01000000
#define ARC_DEBUG_FLAG_DISK_READ    0x02000000
#define ARC_DEBUG_FLAG_DISK_WRITE   0x04000000
#define ARC_DEBUG_FLAG_DISK_CAP     0x08000000
#define ARC_DEBUG_FLAG_DISK_DATA    0x10000000
#define ARC_DEBUG_FLAG_DISK_DUMP    0x20000000


/**************************************************************************
Include the OS and BSP dependent files that define IO functions and
basic types. You may like to change these files for your board and RTOS 
**************************************************************************/
   

/**************************************************************************
Global variables and some defines for device.
**************************************************************************/

#define BUFFERSIZE                      (2048)

#define EP_TEMP_BUFFERSIZE              (32)
#define MASS_STORAGE_INTERFACE          (0)

#define APP_CONTROL_MAX_PKT_SIZE        (64)
#define DEV_DESC_MAX_PACKET_SIZE        (7)
#define DISK_FS_MAX_PACKET_SIZE         (64)
#define DISK_HS_MAX_PACKET_SIZE         (512)

#define CFG_DESC_EP_IN_TYPE_OFFSET              (21)
#define CFG_DESC_EP_IN_MAX_PACKET_SIZE_OFFSET   (22)
#define CFG_DESC_EP_OUT_TYPE_OFFSET             (28)
#define CFG_DESC_EP_OUT_MAX_PACKET_SIZE_OFFSET  (29)

#define TOTAL_LOGICAL_ADDRESS_BLOCKS    (4096)
#define LENGTH_OF_EACH_LAB              (512)

#define DISK_IN_EP_NO       1
#define DISK_OUT_EP_NO      2

#define DISK_IN_EP_TYPE     2 /* Bulk */
#define DISK_OUT_EP_TYPE    2 /* Bulk */

typedef struct
{
    _usb_device_handle  usbDevHandle;     /* Must be first field */
    uint_32             devNo;
    uint_8_ptr          Send_Buffer_Unaligned;

    uint_8_ptr          DevDesc;
    uint_8_ptr          DevQualifierDesc;
    uint_8_ptr          ConfigDesc;
    uint_8_ptr          other_speed_config;
    uint_8_ptr          ep1_buf;
    uint_8_ptr          epTemp_buf;
    DISK_READ_CAPACITY* pReadCapacity;
    CSW_STRUCT*         pCSW;
    uint_8_ptr          MASS_STORAGE_DISK;

    SETUP_STRUCT        local_setup_packet;

    volatile boolean    TEST_ENABLED;
    volatile boolean    ENTER_TEST_MODE; 
    volatile uint_16    test_mode_index;
    volatile uint_8     speed;
    uint_16             logicalBlocks;
    uint_32             hsMaxPktSize;
    uint_32             fsMaxPktSize;

    uint_32             inEpType;
    uint_32             outEpType;

    uint_32             inEpNo;
    uint_32             outEpNo;
    boolean             CBW_PROCESSED;
    boolean             ZERO_TERMINATE;

} USB_DISK_STRUCT;

uint_32             diskHsMaxPktSize = DISK_HS_MAX_PACKET_SIZE;
uint_32             diskFsMaxPktSize = DISK_FS_MAX_PACKET_SIZE;

uint_32             diskInEpType = DISK_IN_EP_TYPE;
uint_32             diskOutEpType = DISK_OUT_EP_TYPE;

uint_32             diskInEpNo = DISK_IN_EP_NO;
uint_32             diskOutEpNo = DISK_OUT_EP_NO;

static USB_DISK_STRUCT*  usbDisksPtr[MAX_USB_DEVICES] = { NULL, NULL };

/**************************************************************************
DESCRIPTORS DESCRIPTORS DESCRIPTORS DESCRIPTORS DESCRIPTORS DESCRIPTORS
**************************************************************************/

#define DEVICE_DESCRIPTOR_SIZE 18
static const uint_8  DevDescData[DEVICE_DESCRIPTOR_SIZE] =
{
   /* Length of DevDesc */
   DEVICE_DESCRIPTOR_SIZE,
   /* "Device" Type of descriptor */
   1,
   /* BCD USB version */
   0, 2,
   /* Device Class is indicated in the interface descriptors */
   0x00,
   /* Device Subclass is indicated in the interface descriptors */
   0x00,
   /* Mass storage devices do not use class-specific protocols */
   0x00,
   /* Max packet size */
   APP_CONTROL_MAX_PKT_SIZE,
   /* Vendor ID */
   USB_uint_16_low(0x1286), USB_uint_16_high(0x1286),
   /* Product ID */
   USB_uint_16_low(0x1), USB_uint_16_high(0x1),
   /* BCD Device version */
   USB_uint_16_low(0x0002), USB_uint_16_high(0x0002),
   /* Manufacturer string index */
   0x1,
   /* Product string index */
   0x2,
   /* Serial number string index */
   0x6,
   /* Number of configurations available */
   0x1
};

/* USB 2.0 specific descriptor */
#define DEVICE_QUALIFIER_DESCRIPTOR_SIZE 10
static const uint_8  DevQualifierDescData[DEVICE_QUALIFIER_DESCRIPTOR_SIZE] =
{
   DEVICE_QUALIFIER_DESCRIPTOR_SIZE,  /* bLength Length of this descriptor */
   6,                         /* bDescType This is a DEVICE Qualifier descr */
   0,2,                       /* bcdUSB USB revision 2.0 */
   0,                         /* bDeviceClass */
   0,                         /* bDeviceSubClass */
   0,                         /* bDeviceProtocol */
   APP_CONTROL_MAX_PKT_SIZE,  /* bMaxPacketSize0 */
   0x01,                      /* bNumConfigurations */
   0
};

#define CONFIG_DESC_NUM_INTERFACES  (4)
/* This must be counted manually and updated with the descriptor */
/* 1*Config(9) + 1*Interface(9) + 2*Endpoint(7) = 32 bytes */
#define CONFIG_DESC_SIZE            (32)

/**************************************************************
we declare the config desc as USB_Uncached because this descriptor
is updated on the fly for max packet size during enumeration. Making
it uncached ensures that main memory is updated whenever this
descriptor pointer is used.
**************************************************************/
static const uint_8 ConfigDescData[CONFIG_DESC_SIZE] =
{
   /* Configuration Descriptor - always 9 bytes */
   9,
   /* "Configuration" type of descriptor */
   2,
   /* Total length of the Configuration descriptor */
   USB_uint_16_low(CONFIG_DESC_SIZE), 
   USB_uint_16_high(CONFIG_DESC_SIZE),
   /* NumInterfaces */
   1,
   /* Configuration Value */
   1,
   /* Configuration Description String Index*/
   4,
   /* Attributes.  Self-powered. */
   0xc0,
   /* Current draw from bus */
   0,
   /* Interface 0 Descriptor - always 9 bytes */
   9,
   /* "Interface" type of descriptor */
   4,
   /* Number of this interface */
   MASS_STORAGE_INTERFACE,
   /* Alternate Setting */
   0,
   /* Number of endpoints on this interface */
   2,
   /* Interface Class */
   0x08,
   /* Interface Subclass: SCSI transparent command set */
   0x06,
   /* Interface Protocol: Bulk only protocol */
   0x50,
   /* Interface Description String Index */
   0,
   /* Endpoint 1 (Bulk In Endpoint), Interface 0 Descriptor - always 7 bytes*/
   7,
   /* "Endpoint" type of descriptor */
   5,
   /*
   ** Endpoint address.  The low nibble contains the endpoint number and the
   ** high bit indicates TX(1) or RX(0).
   */
   ((ARC_USB_SEND<<7) | DISK_IN_EP_NO) /*0x81*/,
   /* Attributes.  0=Control 1=Isochronous 2=Bulk 3=Interrupt */
   DISK_IN_EP_TYPE,
   /* Max Packet Size for this endpoint */
   USB_uint_16_low(DISK_FS_MAX_PACKET_SIZE), 
   USB_uint_16_high(DISK_FS_MAX_PACKET_SIZE),
   /* Polling Interval (ms) */
   0,
   /* Endpoint 2 (Bulk Out Endpoint), Interface 0 Descriptor - always 7 bytes*/
   7,
   /* "Endpoint" type of descriptor */
   5,
   /*
   ** Endpoint address.  The low nibble contains the endpoint number and the
   ** high bit indicates TX(1) or RX(0).
   */
   ((ARC_USB_RECV<<7) | DISK_OUT_EP_NO), /*0x02*/
   /* Attributes.  0=Control 1=Isochronous 2=Bulk 3=Interrupt */
   DISK_OUT_EP_TYPE,
   /* Max Packet Size for this endpoint */
   USB_uint_16_low(DISK_FS_MAX_PACKET_SIZE), 
   USB_uint_16_high(DISK_FS_MAX_PACKET_SIZE),
   /* Polling Interval (ms) */
   0
};

#define OTHER_SPEED_CONFIG_DESC_SIZE  CONFIG_DESC_SIZE
static const uint_8  other_speed_config_data[CONFIG_DESC_SIZE] =
{
   9,                         /* bLength Length of this descriptor */
   7,                         /* bDescType This is a Other speed config descr */
   USB_uint_16_low(OTHER_SPEED_CONFIG_DESC_SIZE), 
   USB_uint_16_high(OTHER_SPEED_CONFIG_DESC_SIZE),
   1,
   1,
   4,
   0xc0,
   0,
   /* Interface 0 Descriptor - always 9 bytes */
   9,
   /* "Interface" type of descriptor */
   4,
   /* Number of this interface */
   MASS_STORAGE_INTERFACE,
   /* Alternate Setting */
   0,
   /* Number of endpoints on this interface */
   2,
   /* Interface Class */
   0x08,
   /* Interface Subclass: SCSI transparent command set */
   0x06,
   /* Interface Protocol: Bulk only protocol */
   0x50,
   /* Interface Description String Index */
   0,
   /* Endpoint 1 (Bulk In Endpoint), Interface 0 Descriptor - always 7 bytes*/
   7,
   /* "Endpoint" type of descriptor */
   5,
   /*
   ** Endpoint address.  The low nibble contains the endpoint number and the
   ** high bit indicates TX(1) or RX(0).
   */
   ((ARC_USB_SEND<<7) | DISK_IN_EP_NO), /*0x81*/
   /* Attributes.  0=Control 1=Isochronous 2=Bulk 3=Interrupt */
   DISK_IN_EP_TYPE,
   /* Max Packet Size for this endpoint */
   USB_uint_16_low(DISK_HS_MAX_PACKET_SIZE), 
   USB_uint_16_high(DISK_HS_MAX_PACKET_SIZE),
   /* Polling Interval (ms) */
   0,
   /* Endpoint 2 (Bulk Out Endpoint), Interface 0 Descriptor - always 7 bytes*/
   7,
   /* "Endpoint" type of descriptor */
   5,
   /*
   ** Endpoint address.  The low nibble contains the endpoint number and the
   ** high bit indicates TX(1) or RX(0).
   */
   ((ARC_USB_RECV<<7) | DISK_OUT_EP_NO), /*0x02*/

   /* Attributes.  0=Control 1=Isochronous 2=Bulk 3=Interrupt */
   DISK_OUT_EP_TYPE,
   /* Max Packet Size for this endpoint */
   USB_uint_16_low(DISK_HS_MAX_PACKET_SIZE), 
   USB_uint_16_high(DISK_HS_MAX_PACKET_SIZE),
   /* Polling Interval (ms) */
   0
};

static uint_8 USB_IF_ALT[4] = { 0, 0, 0, 0};

/* number of strings in the table not including 0 or n. */
static const uint_8 USB_STR_NUM = 7;

/*
** if the number of strings changes, look for USB_STR_0 everywhere and make 
** the obvious changes.  It should be found in 3 places.
*/

static uint_16 USB_STR_0[ 2] = {(0x300 + sizeof(USB_STR_0)),(0x0409)};
static uint_16 USB_STR_1[26] = {(0x300 + sizeof(USB_STR_1)),
      'M','a','r','v','e','l','l',' ','S','e','m','i','c','o','n','d','u','c','t','o','r',' ','L','t','d'};       
static uint_16 USB_STR_2[28] = {(0x300 + sizeof(USB_STR_2)),
      'M','A','R','V','E','L','L',' ','M','a','s','s',' ','S','t','o','r','a','g','e',' ',\
      'D','e','v','i','c','e'};
static uint_16 USB_STR_3[ 5] = {(0x300 + sizeof(USB_STR_3)),
      'B','E','T','A'};
static uint_16 USB_STR_4[ 4] = {(0x300 + sizeof(USB_STR_4)),
      '#','0','2'};
static uint_16 USB_STR_5[ 4] = {(0x300 + sizeof(USB_STR_5)),
      '_','A','1'};
      /* Serial number has to be at least 12 bytes */
static uint_16 USB_STR_6[ 13] = {(0x300 + sizeof(USB_STR_6)),
      '0','0','0','0','0','0','0','0','0','0','0','1'};     
static uint_16 USB_STR_7[15] = {(0x300 + sizeof(USB_STR_7)),
      'Y','o','u','r',' ','n','a','m','e',' ','h','e','r','e'};
static uint_16 USB_STR_n[17] = {(0x300 + sizeof(USB_STR_n)),
      'B','A','D',' ','S','T','R','I','N','G',' ','I','n','d','e','x'};

#define USB_STRING_ARRAY_SIZE  9
static uint_8_ptr USB_STRING_DESC[USB_STRING_ARRAY_SIZE] =
{
   (uint_8_ptr)((pointer)USB_STR_0),
   (uint_8_ptr)((pointer)USB_STR_1),
   (uint_8_ptr)((pointer)USB_STR_2),
   (uint_8_ptr)((pointer)USB_STR_3),
   (uint_8_ptr)((pointer)USB_STR_4),
   (uint_8_ptr)((pointer)USB_STR_5),
   (uint_8_ptr)((pointer)USB_STR_6),
   (uint_8_ptr)((pointer)USB_STR_7),
   (uint_8_ptr)((pointer)USB_STR_n)
};

/*****************************************************************
MASS STORAGE SPECIFIC GLOBALS
*****************************************************************/

static const DISK_DEVICE_INFO device_information_data = 
{
   0, 0x80, 0, 0x01, 0x1F, 
   /* Reserved */
   {0, 0, 0},
   /* Vendor information: "MARVELL  " */
   {0x4D, 0x41, 0x52, 0x56, 0x45, 0x4C, 0x4C, 0x20,},
   /* Product information: "Disk            " */
   {0x44, 0x69, 0x73, 0x6B, 0x20, 0x20, 0x20, 0x20,
   0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20}, 
   /* Product Revision level: "Demo" */
   {0x44, 0x65, 0x6D, 0x6F}
}; 

static const DISK_READ_CAPACITY read_capacity = 
{
   /* Data for the capacity */
   {
      0x00, 0x00, USB_uint_16_high(TOTAL_LOGICAL_ADDRESS_BLOCKS-14), 
      USB_uint_16_low(TOTAL_LOGICAL_ADDRESS_BLOCKS-14)
   }, 
   {
      0x00, 0x00, USB_uint_16_high(LENGTH_OF_EACH_LAB), 
      USB_uint_16_low(LENGTH_OF_EACH_LAB)
   }
};

static const uint_8 BOOT_SECTOR_AREA[512] = 
{
   /* Block 0 is the boot sector. Following is the data in the boot sector */
   /* 80x86 "short: jump instruction, indicating that the disk is formatted */
    0xEB, 
    /* 8-bit displacement */
    0x3C, 
    /* NOP OPCode */
    0x90, 
    /* 8-bytes for OEM identification: "ARC 4.3 " */
    0x41, 0x52, 0x43, 0x20, 0x34, 0x2E, 0x33, 0x20, 
    /* bytes/sector: 512 bytes (0x0200) */
    0x00, 0x02, 
    /* Sectors/allocation unit */
    0x01,
    /* Reserved sectors: 0x0001 */
    0x01, 0x00, 
    /* Number of File Allocation Tables (FATs): 2 */
    0x02,
    /* Number of root directory entries */
    0x00, 0x02, 
    /* Total Small sectors in logical volume */
    USB_uint_16_low(TOTAL_LOGICAL_ADDRESS_BLOCKS), 
    USB_uint_16_high(TOTAL_LOGICAL_ADDRESS_BLOCKS),
    /* Media descriptor byte: 0xF8: Fixed disk */
    0xF8,
    /* Sectors/FAT: 3 (Each FAT starts at a new sector) */
    0x80, 0x00, 
    /* Sectors/track: 9 */
    0x09, 0x00, 
    /* Number of heads */
    0x02, 0x00, 
    /* Number of hidden sectors: 0 */
    0x00, 0x00, 0x00, 0x00, 
    /* Total Large sectors in logical volume */
    0x00, 0x00, 0x00, 0x00, 
    /* Physical drive number */
    0x00, 
    /* Reserved */
    0x00, 
    /* Extended boot signature record: 0x29 */
    0x29,
    /* 32-bit binary volume ID */
    0x01, 0x02, 0x03, 0x04, 
    /* Volume label */
    0x53, 0x54, 0x55, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
    /* Reserved FAT-16*/
    0x46, 0x41, 0x54, 0x31, 0x36, 0x00, 0x00, 0x00,
    /* Bootstrap */
    0x33, 0xC0, 0x8E, 0xD0, 0xBC, 0x00, 0x7C, 0xFC, 0xE8, 0x45, 0x00, 
    /* String: \r\nNon-System disk\r\nPress any key to reboot\r\n" */
    0x0D, 0x0A, 0x4E, 0x6F, 0x6E, 0x2D, 0x53, 0x79, 0x73, 0x74, 0x65,
    0x6D, 0x20, 0x64, 0x69, 0x73, 0x6B, 0x0D, 0x0A, 0x50, 0x72, 0x65, 
    0x73, 0x73, 0x20, 0x61, 0x6E, 0x79, 0x20, 0x6B, 0x65, 0x79, 0x20, 
    0x74, 0x6F, 0x20, 0x72, 0x65, 0x62, 0x6F, 0x6F, 0x74, 0x0D, 0x0A, 
    0x5E, 0xEB, 0x02, 0xCD, 0x10, 0xB4, 0x0E, 0xBB, 0x07, 0x00, 0x2E, 
    0xAC, 0x84, 0xC0, 0x75, 0xF3, 0x98, 0xCD, 0x16, 0xCD, 0x19, 0xEB, 
    0xB1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    /* Partition descriptors */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xAA
};


static const uint_8 FAT16_SPECIAL_BYTES[3] = 
{
   /* FAT ID: Same as Media descriptor */
   0xF8, 0xFF, 0xFF
};


/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : ch9GetDescription
* Returned Value : None
* Comments       :
*     Chapter 9 GetDescription command
*     The Device Request can ask for Device/Config/string/interface/endpoint
*     descriptors (via wValue). We then post an IN response to return the
*     requested descriptor.
*     And then wait for the OUT which terminates the control transfer.
* 
*END*--------------------------------------------------------------------*/
static void ch9GetDescription
   (
      /* USB handle */
      _usb_device_handle handle,
      
      /* Is it a Setup phase? */
      boolean setup,
            
      /* The setup packet pointer */
      SETUP_STRUCT_PTR setup_ptr
   )
{ /* Body */
    int                 devNo = _usb_device_get_dev_num(handle);
    USB_DISK_STRUCT*    pDiskCtrl = usbDisksPtr[devNo];
    uint_32             max_pkt_size;

    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_SETUP, "usbDisk %s: setup=%d, value=0x%x, length=%d\n", 
                __FUNCTION__, (int)setup, setup_ptr->VALUE, setup_ptr->LENGTH);

    if (setup) 
    {
        /* Load the appropriate string depending on the descriptor requested.*/
        switch (setup_ptr->VALUE & 0xFF00) 
        {
            case 0x0100:
                _usb_device_send_data(handle, 0, pDiskCtrl->DevDesc,
                    MIN(setup_ptr->LENGTH, DEVICE_DESCRIPTOR_SIZE));
                break;

            case 0x0200:
                /* Set the Max Packet Size in the config and other speed config */
                if(pDiskCtrl->speed == ARC_USB_SPEED_HIGH) 
                {
                    max_pkt_size = pDiskCtrl->hsMaxPktSize;
                } 
                else 
                {
                    max_pkt_size = pDiskCtrl->fsMaxPktSize;
                } /* Endif */
        
                *(pDiskCtrl->ConfigDesc + CFG_DESC_EP_IN_TYPE_OFFSET) = (uint_8)pDiskCtrl->inEpType;

                *(pDiskCtrl->ConfigDesc + CFG_DESC_EP_IN_MAX_PACKET_SIZE_OFFSET) = 
                                                USB_uint_16_low(max_pkt_size);
                *(pDiskCtrl->ConfigDesc + CFG_DESC_EP_IN_MAX_PACKET_SIZE_OFFSET+1) = 
                                                USB_uint_16_high(max_pkt_size);

                *(pDiskCtrl->ConfigDesc + CFG_DESC_EP_OUT_TYPE_OFFSET) = (uint_8)pDiskCtrl->outEpType;

                *(pDiskCtrl->ConfigDesc + CFG_DESC_EP_OUT_MAX_PACKET_SIZE_OFFSET) = 
                                                USB_uint_16_low(max_pkt_size);
                *(pDiskCtrl->ConfigDesc + CFG_DESC_EP_OUT_MAX_PACKET_SIZE_OFFSET+1) = 
                                                USB_uint_16_high(max_pkt_size);

                _usb_device_send_data(handle, 0, pDiskCtrl->ConfigDesc,
                            MIN(setup_ptr->LENGTH, CONFIG_DESC_SIZE));
                break;

            case 0x0300:
                if ((setup_ptr->VALUE & 0x00FF) > USB_STR_NUM) {
                    _usb_device_send_data(handle, 0, USB_STRING_DESC[USB_STR_NUM+1],
                            MIN(setup_ptr->LENGTH, USB_STRING_DESC[USB_STR_NUM+1][0]));
                } 
                else 
                {
                    _usb_device_send_data(handle, 0, USB_STRING_DESC[setup_ptr->VALUE & 0x00FF],
                            MIN(setup_ptr->LENGTH, USB_STRING_DESC[setup_ptr->VALUE & 0x00FF][0]));
                } /* Endif */      
                break;
            
            case 0x600:
                _usb_device_send_data(handle, 0, (uint_8_ptr)pDiskCtrl->DevQualifierDesc, 
                        MIN(setup_ptr->LENGTH, DEVICE_QUALIFIER_DESCRIPTOR_SIZE));
                break;
            
            case 0x700:
                if(pDiskCtrl->speed == ARC_USB_SPEED_HIGH) 
                {
                    max_pkt_size = pDiskCtrl->fsMaxPktSize;
                } 
                else 
                {
                    max_pkt_size = pDiskCtrl->hsMaxPktSize;
                } /* Endif */
            
                *(pDiskCtrl->other_speed_config + CFG_DESC_EP_IN_TYPE_OFFSET) = (uint_8)pDiskCtrl->inEpType;

                *(pDiskCtrl->other_speed_config + CFG_DESC_EP_IN_MAX_PACKET_SIZE_OFFSET) = 
                    USB_uint_16_low(max_pkt_size);
                *(pDiskCtrl->other_speed_config + CFG_DESC_EP_IN_MAX_PACKET_SIZE_OFFSET+1) = 
                    USB_uint_16_high(max_pkt_size);

                *(pDiskCtrl->other_speed_config + CFG_DESC_EP_OUT_TYPE_OFFSET) = (uint_8)pDiskCtrl->outEpType;

                *(pDiskCtrl->other_speed_config + CFG_DESC_EP_OUT_MAX_PACKET_SIZE_OFFSET) = 
                    USB_uint_16_low(max_pkt_size);
                *(pDiskCtrl->other_speed_config + CFG_DESC_EP_OUT_MAX_PACKET_SIZE_OFFSET+1) = 
                    USB_uint_16_high(max_pkt_size);
            
                _usb_device_send_data(handle, 0, (uint_8_ptr)pDiskCtrl->other_speed_config, 
                            MIN(setup_ptr->LENGTH, OTHER_SPEED_CONFIG_DESC_SIZE));
               
                break;

            default:
                USB_printf("usbDisk_%d, %s: Unexpected VALUE=0x%04x\n", 
                        _usb_device_get_dev_num(handle), __FUNCTION__, setup_ptr->VALUE);
                _usb_device_stall_endpoint(handle, 0, ARC_USB_RECV);
                return;
        } /* Endswitch */
        /* status phase */
        _usb_device_recv_data(handle, 0, NULL, 0);
    } /* Endif */
    return;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : ch9SetDescription
* Returned Value : None
* Comments       :
*     Chapter 9 SetDescription command
* 
*END*--------------------------------------------------------------------*/
static void ch9SetDescription
   (
      /* USB handle */
      _usb_device_handle handle,
      
      /* Is it a Setup phase? */
      boolean setup,
            
      /* The setup packet pointer */
      SETUP_STRUCT_PTR setup_ptr
   )
{ /* Body */
   USB_printf("usbDisk_%d, %s: setup=%d\n", 
        _usb_device_get_dev_num(handle), __FUNCTION__, (int)setup);
   _usb_device_stall_endpoint(handle, 0, ARC_USB_RECV);

   return;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : ch9GetConfig
* Returned Value : None
* Comments       :
*     Chapter 9 GetConfig command
* 
*END*--------------------------------------------------------------------*/
static void ch9GetConfig
   (
      /* USB handle */
      _usb_device_handle handle,
      
      /* Is it a Setup phase? */
      boolean setup,
            
      /* The setup packet pointer */
      SETUP_STRUCT_PTR setup_ptr
   )
{ /* Body */
    uint_16             current_config;
    int                 devNo = _usb_device_get_dev_num(handle);
    USB_DISK_STRUCT*    pDiskCtrl = usbDisksPtr[devNo];

    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_SETUP, "usbDisk %s: setup=%d\n", __FUNCTION__, (int)setup);

    /* Return the currently selected configuration */
    if (setup)
    { 
        _usb_device_get_status(handle, ARC_USB_STATUS_CURRENT_CONFIG,
                                &current_config);
        *pDiskCtrl->epTemp_buf = (current_config & 0xFF);      
        _usb_device_send_data(handle, 0, pDiskCtrl->epTemp_buf, sizeof(uint_8));
        /* status phase */
        _usb_device_recv_data(handle, 0, NULL, 0);
    } /* Endif */
    return;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : ch9SetConfig
* Returned Value : None
* Comments       :
*     Chapter 9 SetConfig command
* 
*END*--------------------------------------------------------------------*/
static void ch9SetConfig
   (
      /* USB handle */
      _usb_device_handle handle,
      
      /* Is it a Setup phase? */
      boolean setup,
            
      /* The setup packet pointer */
      SETUP_STRUCT_PTR setup_ptr
   )
{ /* Body */
    int                 devNo = _usb_device_get_dev_num(handle);
    USB_DISK_STRUCT*    pDiskCtrl = usbDisksPtr[devNo];
    uint_16             usb_state;
    uint_32             max_pkt_size;
   
    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_SETUP, "usbDisk %s: setup=%d, value=0x%x\n", 
                    __FUNCTION__, (int)setup, setup_ptr->VALUE);

    if (setup) 
    {
        if ((setup_ptr->VALUE & 0x00FF) > 1) 
        {
            /* generate stall */
            USB_printf("usbDisk_%d, %s: Wrong VALUE=0x%04x\n", 
                    _usb_device_get_dev_num(handle), __FUNCTION__, setup_ptr->VALUE);
            _usb_device_stall_endpoint(handle, 0, ARC_USB_RECV);
            return;
        } /* Endif */

        /* 0 indicates return to unconfigured state */
        if ((setup_ptr->VALUE & 0x00FF) == 0) 
        {
            _usb_device_get_status(handle, ARC_USB_STATUS_DEVICE_STATE, &usb_state);
            if( (usb_state == ARC_USB_STATE_CONFIG) || 
                (usb_state == ARC_USB_STATE_ADDRESS) ) 
            {
                /* clear the currently selected config value */
                _usb_device_set_status(handle, ARC_USB_STATUS_CURRENT_CONFIG, 0);
                _usb_device_set_status(handle, ARC_USB_STATUS_DEVICE_STATE,
                                                    ARC_USB_STATE_ADDRESS);
                /* status phase */      
                _usb_device_send_data(handle, 0, 0, 0);
            } 
            else 
            {
                USB_printf("usbDisk_%d, %s: Wrong usb_state=%d\n", 
                    _usb_device_get_dev_num(handle), __FUNCTION__, usb_state);
                _usb_device_stall_endpoint(handle, 0, ARC_USB_RECV);
            } /* Endif */
            return;
        } /* Endif */

        /*
        ** If the configuration value (setup_ptr->VALUE & 0x00FF) differs
        ** from the current configuration value, then endpoints must be
        ** reconfigured to match the new device configuration
        */
        _usb_device_get_status(handle, ARC_USB_STATUS_CURRENT_CONFIG,
                                                        &usb_state);

        ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_SETUP, "usbDisk: Set configuration: old=%d, new=%d\n", 
                    usb_state, setup_ptr->VALUE & 0x00FF);

        if (usb_state != (setup_ptr->VALUE & 0x00FF)) 
        {
            /* Reconfigure endpoints here */
            switch (setup_ptr->VALUE & 0x00FF) 
            {
                default:
                break;
            } /* Endswitch */

            _usb_device_set_status(handle, ARC_USB_STATUS_CURRENT_CONFIG,
                                    setup_ptr->VALUE & 0x00FF);
        } /* Endif */

        if (pDiskCtrl->speed == ARC_USB_SPEED_HIGH) 
        {
            max_pkt_size = pDiskCtrl->hsMaxPktSize;
        } 
        else 
        {
            max_pkt_size = pDiskCtrl->fsMaxPktSize;
        } /* Endif */
      
        _usb_device_init_endpoint(handle, pDiskCtrl->outEpNo, max_pkt_size,
                ARC_USB_RECV, ARC_USB_BULK_ENDPOINT, ARC_USB_DEVICE_DONT_ZERO_TERMINATE);
        _usb_device_init_endpoint(handle, pDiskCtrl->inEpNo, max_pkt_size,
                ARC_USB_SEND, ARC_USB_BULK_ENDPOINT, ARC_USB_DEVICE_DONT_ZERO_TERMINATE);
    
        if (_usb_device_get_transfer_status(handle, pDiskCtrl->outEpNo, ARC_USB_RECV) == USB_OK) 
        {
            _usb_device_recv_data(handle, pDiskCtrl->outEpNo, pDiskCtrl->ep1_buf, 31);
        } /* Endif */      
      
        pDiskCtrl->TEST_ENABLED = TRUE;

        _usb_device_set_status(handle, ARC_USB_STATUS_DEVICE_STATE,
                                ARC_USB_STATE_CONFIG);
        /* status phase */
        _usb_device_send_data(handle, 0, 0, 0);

        USB_printf("USB %s speed disk: config = %d\n", 
            (pDiskCtrl->speed == ARC_USB_SPEED_HIGH) ? "High" : "Full", 
            setup_ptr->VALUE & 0x00FF);

    } /* Endif */
    return;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : ch9GetInterface
* Returned Value : None
* Comments       :
*     Chapter 9 GetInterface command
* 
*END*--------------------------------------------------------------------*/
static void ch9GetInterface
   (
      /* USB handle */
      _usb_device_handle handle,
      
      /* Is it a Setup phase? */
      boolean setup,
            
      /* The setup packet pointer */
      SETUP_STRUCT_PTR setup_ptr
   )
{ /* Body */
    uint_16 usb_state;
   
    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_SETUP, "usbDisk %s: setup=%d\n", __FUNCTION__, (int)setup);

    _usb_device_get_status(handle, ARC_USB_STATUS_DEVICE_STATE, &usb_state);
    if (usb_state != ARC_USB_STATE_CONFIG) 
    {
        USB_printf("usbDisk_%d, %s: Wrong usb_state=%d\n", 
                    _usb_device_get_dev_num(handle), __FUNCTION__, usb_state);
        _usb_device_stall_endpoint(handle, 0, ARC_USB_RECV);
        return;
    } /* Endif */

    if (setup) 
    {
        _usb_device_send_data(handle, 0, &USB_IF_ALT[setup_ptr->INDEX & 0x00FF],
                            MIN(setup_ptr->LENGTH, sizeof(uint_8)));
        /* status phase */      
        _usb_device_recv_data(handle, 0, NULL, 0);
    } /* Endif */
    return;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : ch9SetInterface
* Returned Value : None
* Comments       :
*     Chapter 9 SetInterface command
* 
*END*--------------------------------------------------------------------*/
static void ch9SetInterface
   (
      /* USB handle */
      _usb_device_handle handle,
      
      /* Is it a Setup phase? */
      boolean setup,
            
      /* The setup packet pointer */
      SETUP_STRUCT_PTR setup_ptr
   )
{ /* Body */
    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_SETUP, "usbDisk %s: setup=%d\n", __FUNCTION__, (int)setup);

    if (setup) 
    {
        if (setup_ptr->REQUESTTYPE != 0x01) 
        {
            USB_printf("usbDisk_%d, %s: Wrong REQUESTTYPE=0x%02x\n", 
                        _usb_device_get_dev_num(handle), __FUNCTION__, 
                        setup_ptr->REQUESTTYPE);
            _usb_device_stall_endpoint(handle, 0, ARC_USB_RECV);
            return;
        } /* Endif */

        /*
        ** If the alternate value (setup_ptr->VALUE & 0x00FF) differs
        ** from the current alternate value for the specified interface,
        ** then endpoints must be reconfigured to match the new alternate
        */
        if (USB_IF_ALT[setup_ptr->INDEX & 0x00FF]
                        != (setup_ptr->VALUE & 0x00FF))
        {
            USB_IF_ALT[setup_ptr->INDEX & 0x00FF] = (setup_ptr->VALUE & 0x00FF);
            /* Reconfigure endpoints here. */
         
        } /* Endif */

        /* status phase */
        _usb_device_send_data(handle, 0, 0, 0);
    } /* Endif */
    return;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : ch9SynchFrame
* Returned Value : 
* Comments       :
*     Chapter 9 SynchFrame command
* 
*END*--------------------------------------------------------------------*/
static void ch9SynchFrame
   (
      /* USB handle */
      _usb_device_handle handle,
      
      /* Is it a Setup phase? */
      boolean setup,
            
      /* The setup packet pointer */
      SETUP_STRUCT_PTR setup_ptr
   )
{ /* Body */

    uint_16             usbStatus;
    int                 devNo = _usb_device_get_dev_num(handle);
    USB_DISK_STRUCT*    pDiskCtrl = usbDisksPtr[devNo];

    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_SETUP, "usbDisk %s: setup=%d\n", 
                                        __FUNCTION__, (int)setup);

    if (setup) 
    {
        if (setup_ptr->REQUESTTYPE != (REQ_RECIP_ENDPOINT | REQ_TYPE_STANDARD | REQ_DIR_OUT) )
        {
            USB_printf("usbDisk_%d, %s: Wrong REQUESTTYPE=0x%02x\n", 
                        _usb_device_get_dev_num(handle), __FUNCTION__, 
                        setup_ptr->REQUESTTYPE);
            _usb_device_stall_endpoint(handle, 0, ARC_USB_RECV);
            return;
        } /* Endif */

        if ((setup_ptr->INDEX & 0x00FF) >=
            pDiskCtrl->ConfigDesc[CONFIG_DESC_NUM_INTERFACES])
        {
            USB_printf("usbDisk_%d, %s: Wrong INDEX=0x%02x\n", 
                        _usb_device_get_dev_num(handle), __FUNCTION__, setup_ptr->INDEX);
            _usb_device_stall_endpoint(handle, 0, ARC_USB_RECV);
            return;
        } /* Endif */

        _usb_device_get_status(handle, ARC_USB_STATUS_SOF_COUNT, &usbStatus);
        pDiskCtrl->epTemp_buf[0] = USB_uint_16_low(usbStatus);
        pDiskCtrl->epTemp_buf[1] = USB_uint_16_high(usbStatus);
        _usb_device_send_data(handle, 0, pDiskCtrl->epTemp_buf, MIN(setup_ptr->LENGTH, sizeof(uint_16)));
        /* status phase */      
        _usb_device_recv_data(handle, 0, NULL, 0);
    } /* Endif */
    return;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : ch9Class
* Returned Value : 
* Comments       :
*     Chapter 9 Class specific request
*     See section 9.4.11 (page 195) of the USB 1.1 Specification.
* 
*END*--------------------------------------------------------------------*/
static void ch9Class
   (
      _usb_device_handle handle,
      boolean setup,
      uint_8  direction,
      SETUP_STRUCT_PTR setup_ptr
   )
{ /* Body */
    int                 devNo = _usb_device_get_dev_num(handle);
    USB_DISK_STRUCT*    pDiskCtrl = usbDisksPtr[devNo];
   
    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_CLASS, 
                "usbDisk %s: setup=%d, request=0x%x, value=%d, index=%d, size=%d\n", 
                __FUNCTION__, (int)setup, setup_ptr->REQUEST,
                setup_ptr->VALUE, setup_ptr->INDEX, setup_ptr->LENGTH);

    if (setup) 
    {
        switch (setup_ptr->REQUEST) 
        {
            case 0xFF:
                /* Bulk-Only Mass Storage Reset: Ready the device for the next 
                ** CBW from the host 
                */
                if ((setup_ptr->VALUE != 0) || 
                    (setup_ptr->INDEX != MASS_STORAGE_INTERFACE) ||
                    (setup_ptr->LENGTH != 0)) 
                {
                    USB_printf("usbDisk_%d, %s: Wrong Setup: VALUE=%d, INDEX=%d, LENGTH=%d\n", 
                        _usb_device_get_dev_num(handle), __FUNCTION__, setup_ptr->VALUE, 
                        setup_ptr->INDEX, setup_ptr->LENGTH);

                    _usb_device_stall_endpoint(handle, 0, 0);
                } 
                else 
                { /* Body */
                    pDiskCtrl->CBW_PROCESSED = FALSE;
                    pDiskCtrl->ZERO_TERMINATE = FALSE;
                    _usb_device_cancel_transfer(handle, pDiskCtrl->outEpNo, ARC_USB_RECV);
                    _usb_device_cancel_transfer(handle, pDiskCtrl->inEpNo, ARC_USB_SEND);
               
                    /* unstall bulk endpoint */
                    _usb_device_unstall_endpoint(handle, pDiskCtrl->outEpNo, ARC_USB_RECV);
                    _usb_device_unstall_endpoint(handle, pDiskCtrl->inEpNo, ARC_USB_SEND);
               
                    _usb_device_recv_data(handle, pDiskCtrl->outEpNo, pDiskCtrl->ep1_buf, 31);
                    /* send zero packet to control pipe */
                    _usb_device_send_data(handle, 0, NULL, 0);
                } /* Endbody */
                break;

            case 0xFE:
                /* For Get Max LUN use any of these responses*/
                if (setup_ptr->LENGTH == 0) 
                { /* Body */

                    USB_printf("usbDisk_%d, %s: Wrong Length: LENGTH=%d\n", 
                        _usb_device_get_dev_num(handle), __FUNCTION__, setup_ptr->LENGTH);

                    _usb_device_stall_endpoint(handle, 0, 0);
                } 
                else
                {
                    if ((setup_ptr->VALUE != 0) ||
                        (setup_ptr->INDEX != MASS_STORAGE_INTERFACE) ||
                        (setup_ptr->LENGTH != 1)) 
                    { /* Body */
                        USB_printf("usbDisk_%d, %s: Wrong Setup: VALUE=%d, INDEX=%d, LENGTH=%d\n", 
                                    _usb_device_get_dev_num(handle), __FUNCTION__, setup_ptr->VALUE, 
                                    setup_ptr->INDEX, setup_ptr->LENGTH);
                        _usb_device_stall_endpoint(handle, 0, 0);
                    } 
                    else 
                    { /* Body */
                        /* Send Max LUN = 0 to the the control pipe */
                        *pDiskCtrl->epTemp_buf = 0;
                        _usb_device_send_data(handle, 0, pDiskCtrl->epTemp_buf, 1);
                        /* status phase */
                        _usb_device_recv_data(handle, 0, 0, 0);
                    } /* Endbody */     
                }
                break;

            default :
                USB_printf("usbDisk_%d, %s: Wrong REQUEST=0x%02x\n", 
                    _usb_device_get_dev_num(handle), __FUNCTION__, setup_ptr->REQUEST);
                _usb_device_stall_endpoint(handle, 0, 0);
                return;
        } /* EndSwitch */
    } 
    return;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : service_ep0
* Returned Value : None
* Comments       :
*     Called upon a completed endpoint 0 (USB 1.1 Chapter 9) transfer
* 
*END*--------------------------------------------------------------------*/
static void service_ep0
   (
      /* [IN] Handle of the USB device */
      _usb_device_handle   handle,

      /* [IN] request type as registered */
      uint_8               type,

      /* [IN] Is it a setup packet? */
      boolean              setup,
      
      /* [IN] Direction of the transfer.  Is it transmit? */
      uint_8               direction,
      
      /* [IN] Pointer to the data buffer */
      uint_8_ptr           buffer,
      
      /* [IN] Length of the transfer */
      uint_32              length,
      
      /* [IN] Error, if any */
      uint_8               error
            
   )
{ /* Body */
    int                 devNo = _usb_device_get_dev_num(handle);
    USB_DISK_STRUCT*    pDiskCtrl = usbDisksPtr[devNo];
    SETUP_STRUCT*       pSetupPacket = &pDiskCtrl->local_setup_packet;

   if (setup) 
   {
      _usb_device_read_setup_data(handle, 0, (uint_8_ptr)pSetupPacket);
        pSetupPacket->VALUE = USB_16BIT_LE(pSetupPacket->VALUE);
        pSetupPacket->INDEX = USB_16BIT_LE(pSetupPacket->INDEX);
        pSetupPacket->LENGTH = USB_16BIT_LE(pSetupPacket->LENGTH);
   }
   
   ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_EP0,
              "disk %s: setup=%s, dir=%s, pBuf=0x%x, length=%d, reqType=0x%x, req=0x%x\n",
                    __FUNCTION__, (setup ? "YES" : "NO"), 
                    (direction == ARC_USB_RECV) ? "RECV" : "SEND", 
                    (unsigned)buffer, (int)length, pSetupPacket->REQUESTTYPE, 
                    pSetupPacket->REQUEST);

   switch (pSetupPacket->REQUESTTYPE & REQ_TYPE_MASK) 
   {
      case REQ_TYPE_STANDARD:
         switch (pSetupPacket->REQUEST) 
         {
            case REQ_GET_STATUS:
               mvUsbCh9GetStatus(handle, setup, pSetupPacket);
               break;

            case REQ_CLEAR_FEATURE:
               mvUsbCh9ClearFeature(handle, setup, pSetupPacket);
               break;

            case REQ_SET_FEATURE:
               mvUsbCh9SetFeature(handle, setup, pSetupPacket);
               break;

            case REQ_SET_ADDRESS:
               mvUsbCh9SetAddress(handle, setup, pSetupPacket);
               break;

            case REQ_GET_DESCRIPTOR:
               ch9GetDescription(handle, setup, pSetupPacket);
               break;

            case REQ_SET_DESCRIPTOR:
               ch9SetDescription(handle, setup, pSetupPacket);
               break;

            case REQ_GET_CONFIGURATION:
               ch9GetConfig(handle, setup, pSetupPacket);
               break;

            case REQ_SET_CONFIGURATION:
               ch9SetConfig(handle, setup, pSetupPacket);
               break;

            case REQ_GET_INTERFACE:
               ch9GetInterface(handle, setup, pSetupPacket);
               break;

            case REQ_SET_INTERFACE:
               ch9SetInterface(handle, setup, pSetupPacket);
               break;

            case REQ_SYNCH_FRAME:
               ch9SynchFrame(handle, setup, pSetupPacket);
               break;

            default:
                USB_printf("usbDisk_%d, %s: Wrong REQUEST = 0x%02x\n", 
                        _usb_device_get_dev_num(handle), __FUNCTION__, pSetupPacket->REQUEST);
               _usb_device_stall_endpoint(handle, 0, 0);
               break;

         } /* Endswitch */
         
         break;

      case REQ_TYPE_CLASS:
         /* class specific request */
         ch9Class(handle, setup, direction, pSetupPacket);
         return;

      case REQ_TYPE_VENDOR:
         /* vendor specific request can be handled here*/
         USB_printf("usbDisk_%d, %s: Vendor REQUESTTYPE (%d) not supported\n", 
               _usb_device_get_dev_num(handle), __FUNCTION__, REQ_TYPE_VENDOR);
         
         _usb_device_stall_endpoint(handle, 0, 0);
         break;
      
      default:
         USB_printf("usbDisk_%d, %s: Unexpected REQUESTTYPE = 0x%x\n", 
                _usb_device_get_dev_num(handle), __FUNCTION__, 
                pSetupPacket->REQUESTTYPE);
         
         _usb_device_stall_endpoint(handle, 0, 0);
         break;
         
   } /* Endswitch */
   
   return;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : _process_inquiry_command
* Returned Value : None
* Comments       :
*     Process a Mass storage class Inquiry command
* 
*END*--------------------------------------------------------------------*/
void _process_inquiry_command
   (
      /* [IN] Handle of the USB device */
      _usb_device_handle   handle,
            
      /* [IN] Endpoint number */
      uint_8               ep_num,
      
      /* [IN] Pointer to the data buffer */
      CBW_STRUCT_PTR       cbw_ptr
   )
{ /* Body */
    int                 devNo = _usb_device_get_dev_num(handle);
    USB_DISK_STRUCT*    pDiskCtrl = usbDisksPtr[devNo];

    if (cbw_ptr->DCBWDATALENGTH) 
    {
        if (cbw_ptr->BMCBWFLAGS & USB_CBW_DIRECTION_BIT) 
        {      
            /* Send the device information */
            _usb_device_send_data(handle, pDiskCtrl->inEpNo, (uint_8_ptr)&device_information_data, 36);
        } /* Endif */
    } /* Endif */
   
    /* The actual length will never exceed the DCBWDATALENGTH */            
    pDiskCtrl->pCSW->DCSWDATARESIDUE = USB_32BIT_LE(cbw_ptr->DCBWDATALENGTH - 36);
    pDiskCtrl->pCSW->BCSWSTATUS = 0;
   
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : _process_unsupported_command
* Returned Value : None
* Comments       :
*     Responds appropriately to unsupported commands
* 
*END*--------------------------------------------------------------------*/
void _process_unsupported_command
   (
      /* [IN] Handle of the USB device */
      _usb_device_handle   handle,
            
      /* [IN] Endpoint number */
      uint_8               ep_num,
      
      /* [IN] Pointer to the data buffer */
      CBW_STRUCT_PTR       cbw_ptr
   )
{ /* Body */
    int                 devNo = _usb_device_get_dev_num(handle);
    USB_DISK_STRUCT*    pDiskCtrl = usbDisksPtr[devNo];
   
    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_DISK, 
                    "disk unsupported command: BMCBWFLAGS = 0x%02x\n", cbw_ptr->BMCBWFLAGS);

    /* The actual length will never exceed the DCBWDATALENGTH */
    pDiskCtrl->pCSW->DCSWDATARESIDUE = 0;
    pDiskCtrl->pCSW->BCSWSTATUS = 0;

    if (cbw_ptr->BMCBWFLAGS & USB_CBW_DIRECTION_BIT) 
    {      
        /* Send a zero-length packet */
        _usb_device_send_data(handle, pDiskCtrl->inEpNo, (uint_8_ptr)NULL, 0);
    } 
    else 
    {
        pDiskCtrl->CBW_PROCESSED = FALSE;
        /* Send the command status information */
        _usb_device_send_data(handle, pDiskCtrl->inEpNo, (uint_8_ptr)pDiskCtrl->pCSW, 13);
        _usb_device_recv_data(handle, pDiskCtrl->outEpNo, pDiskCtrl->ep1_buf, 31);
    } /* Endif */
   
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : _process_report_capacity
* Returned Value : None
* Comments       :
*     Reports the media capacity as a response to READ CAPACITY Command.
* 
*END*--------------------------------------------------------------------*/
void _process_report_capacity
   (
      /* [IN] Handle of the USB device */
      _usb_device_handle   handle,
            
      /* [IN] Endpoint number */
      uint_8               ep_num,
      
      /* [IN] Pointer to the data buffer */
      CBW_STRUCT_PTR       cbw_ptr
   )
{ /* Body */
    int                 devNo = _usb_device_get_dev_num(handle);
    USB_DISK_STRUCT*    pDiskCtrl = usbDisksPtr[devNo];

    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_DISK_CAP, 
                    "disk read_capacity: BMCBWFLAGS = 0x%02x\n", cbw_ptr->BMCBWFLAGS);

    if (cbw_ptr->BMCBWFLAGS & USB_CBW_DIRECTION_BIT) 
    {      
        /* Send a zero-length packet */
        _usb_device_send_data(handle, pDiskCtrl->inEpNo, (uint_8_ptr)pDiskCtrl->pReadCapacity, 8);
      
    } /* Endif */
   
    /* The actual length will never exceed the DCBWDATALENGTH */            
    pDiskCtrl->pCSW->DCSWDATARESIDUE = 0;
    pDiskCtrl->pCSW->BCSWSTATUS = 0;
   
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : _process_read_command
* Returned Value : None
* Comments       :
*     Sends data as a response to READ Command.
* 
*END*--------------------------------------------------------------------*/
void _process_read_command
   (
      /* [IN] Handle of the USB device */
      _usb_device_handle   handle,
            
      /* [IN] Endpoint number */
      uint_8               ep_num,
      
      /* [IN] Pointer to the data buffer */
      CBW_STRUCT_PTR       cbw_ptr
   )
{ /* Body */
    uint_32             index1 = 0, index2 = 0;
    uint_32             max_pkt_size, byteSize;
    int                 devNo = _usb_device_get_dev_num(handle);
    USB_DISK_STRUCT*    pDiskCtrl = usbDisksPtr[devNo];

    if (cbw_ptr->BMCBWFLAGS & USB_CBW_DIRECTION_BIT) 
    {                
      /* Send a zero-length packet */
      index1  = ((uint_32)cbw_ptr->CBWCB[4] << 8);
      index1  |= cbw_ptr->CBWCB[5];
      index2 = ((uint_32)cbw_ptr->CBWCB[7] << 8);
      index2 |= (uint_32)cbw_ptr->CBWCB[8];

      ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_DISK_READ, 
                    "disk read: FLAGS=0x%02x, LENGTH=0x%x, index1=0x%x, index2=0x%x\n", 
                    cbw_ptr->BMCBWFLAGS, cbw_ptr->DCBWDATALENGTH, index1, index2);

      if(cbw_ptr->CBWCB[0] != 0x3E)
      {
          byteSize = index2 * LENGTH_OF_EACH_LAB;
      }
      else
      {
          byteSize = index2;
          index2 = (USB_MEM_ALIGN(byteSize, LENGTH_OF_EACH_LAB) / LENGTH_OF_EACH_LAB);
      }

      /* Check index validities */
      if( (index1 + index2) >= pDiskCtrl->logicalBlocks)
      {
          USB_printf("USB disk read: invalid indexes - addr=%d, size=%d\n",
                        index1, index2);
          pDiskCtrl->pCSW->DCSWDATARESIDUE = USB_32BIT_LE(cbw_ptr->DCBWDATALENGTH);
          pDiskCtrl->pCSW->BCSWSTATUS = 1;
          /* Send zero size packet */
          _usb_device_send_data(handle, pDiskCtrl->inEpNo, (uint_8_ptr)NULL, 0);
          return;
      }
 
      if (cbw_ptr->DCBWDATALENGTH == 0) 
      { /* Body */
         pDiskCtrl->pCSW->DCSWDATARESIDUE = 0;
         pDiskCtrl->pCSW->BCSWSTATUS = 2;
         pDiskCtrl->CBW_PROCESSED = FALSE;
         /* Send the command status information */
         _usb_device_send_data(handle, pDiskCtrl->inEpNo, (uint_8_ptr)pDiskCtrl->pCSW, 13);
         _usb_device_recv_data(handle, pDiskCtrl->outEpNo, pDiskCtrl->ep1_buf, 31);
         return;
      } 
      else 
      { /* Body */
         pDiskCtrl->pCSW->DCSWDATARESIDUE = 0;
         pDiskCtrl->pCSW->BCSWSTATUS = 0;         
         if (byteSize > cbw_ptr->DCBWDATALENGTH) 
         { /* Body */
            byteSize = cbw_ptr->DCBWDATALENGTH;
            pDiskCtrl->pCSW->DCSWDATARESIDUE = USB_32BIT_LE(cbw_ptr->DCBWDATALENGTH);
            pDiskCtrl->pCSW->BCSWSTATUS = 2;
         } 
         else 
         {
            if (byteSize < cbw_ptr->DCBWDATALENGTH) 
            { /* Body */
                pDiskCtrl->pCSW->DCSWDATARESIDUE = USB_32BIT_LE(cbw_ptr->DCBWDATALENGTH - index2);
                if (byteSize > 0) 
                { /* Body */
                    if (pDiskCtrl->speed == ARC_USB_SPEED_HIGH) 
                    {
                        max_pkt_size = pDiskCtrl->hsMaxPktSize;
                    } 
                    else 
                    {
                        max_pkt_size = pDiskCtrl->fsMaxPktSize;
                    }

                    if( (byteSize % max_pkt_size) == 0) 
                    { /* Body */
                        /* Need send a zero terminate packet to host */
                        pDiskCtrl->ZERO_TERMINATE = TRUE;
                    } /* Endbody */
                } /* Endbody */  
            } /* Endbody */
         }

         _usb_device_send_data(handle, pDiskCtrl->inEpNo, 
            pDiskCtrl->MASS_STORAGE_DISK + (index1*LENGTH_OF_EACH_LAB), byteSize);
      } /* Endbody */
   } 
   else 
   { /* Body */
      USB_printf("disk read incorrect: FLAGS=0x%02x, LENGTH=0x%x\n", 
                    cbw_ptr->BMCBWFLAGS, cbw_ptr->DCBWDATALENGTH);

      /* Incorrect but valid CBW */
      if (cbw_ptr->DCBWDATALENGTH > BUFFERSIZE)
         byteSize = BUFFERSIZE;
      else
         byteSize = cbw_ptr->DCBWDATALENGTH;

      pDiskCtrl->pCSW->DCSWDATARESIDUE = USB_32BIT_LE(cbw_ptr->DCBWDATALENGTH);
      pDiskCtrl->pCSW->BCSWSTATUS = 2;
       _usb_device_recv_data(handle, pDiskCtrl->outEpNo, pDiskCtrl->ep1_buf, index2);   
   } /* Endbody */     
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : _process_write_command
* Returned Value : None
* Comments       :
*     Sends data as a response to WRITE Command.
* 
*END*--------------------------------------------------------------------*/
void _process_write_command
   (
      /* [IN] Handle of the USB device */
      _usb_device_handle   handle,
            
      /* [IN] Endpoint number */
      uint_8               ep_num,
      
      /* [IN] Pointer to the data buffer */
      CBW_STRUCT_PTR       cbw_ptr
   )
{ /* Body */
    uint_32             index1 = 0, index2 = 0;
    uint_32             byteSize;
    int                 devNo = _usb_device_get_dev_num(handle);
    USB_DISK_STRUCT*    pDiskCtrl = usbDisksPtr[devNo];

    if (!(cbw_ptr->BMCBWFLAGS & USB_CBW_DIRECTION_BIT)) 
    {
        index1  = ((uint_32)cbw_ptr->CBWCB[4] << 8);
        index1  |= cbw_ptr->CBWCB[5];
        index2 = ((uint_32)cbw_ptr->CBWCB[7] << 8);
        index2 |= (uint_32)cbw_ptr->CBWCB[8];
        
        ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_DISK_WRITE, 
                      "disk write: FLAGS=0x%02x, LENGTH=0x%x, index1=0x%x, index2=0x%x\n", 
                      cbw_ptr->BMCBWFLAGS, cbw_ptr->DCBWDATALENGTH, index1, index2);
        
        if(cbw_ptr->CBWCB[0] != 0x3F)
        {
            byteSize = index2 * LENGTH_OF_EACH_LAB;
        }
        else
        {
            byteSize = index2;
            index2 = (USB_MEM_ALIGN(byteSize, LENGTH_OF_EACH_LAB) / LENGTH_OF_EACH_LAB);
        }
        
        /* Check index validities */
        if( (index1 + index2) >= pDiskCtrl->logicalBlocks)
        {
            USB_printf("USB disk write: invalid indexes - addr=%d, size=%d\n",
                          index1, index2);
            pDiskCtrl->pCSW->DCSWDATARESIDUE = 0;
            pDiskCtrl->pCSW->BCSWSTATUS = 1;
            pDiskCtrl->CBW_PROCESSED = FALSE;
            /* Send the command status information */
            _usb_device_send_data(handle, pDiskCtrl->inEpNo, (uint_8_ptr)pDiskCtrl->pCSW, 13);
            _usb_device_recv_data(handle, pDiskCtrl->outEpNo, pDiskCtrl->ep1_buf, 31);
            return;
        }

        if (cbw_ptr->DCBWDATALENGTH == 0) 
        { /* Body */
            /* Zero transfer length */
            pDiskCtrl->pCSW->DCSWDATARESIDUE = 0;
            pDiskCtrl->pCSW->BCSWSTATUS = 2;
            pDiskCtrl->CBW_PROCESSED = FALSE;
            
            /* Send the command status information */
            _usb_device_send_data(handle, pDiskCtrl->inEpNo, (uint_8_ptr)pDiskCtrl->pCSW, 13);
            
            _usb_device_recv_data(handle, pDiskCtrl->outEpNo, pDiskCtrl->ep1_buf, 31);
            return;
        } 
        else 
        { /* Body */
            pDiskCtrl->pCSW->DCSWDATARESIDUE = 0;
            pDiskCtrl->pCSW->BCSWSTATUS = 0;
         
            if (byteSize < cbw_ptr->DCBWDATALENGTH) 
            { /* Body */
                /* The actual length will never exceed the DCBWDATALENGTH */
                pDiskCtrl->pCSW->DCSWDATARESIDUE = USB_32BIT_LE(cbw_ptr->DCBWDATALENGTH - byteSize);
                byteSize = cbw_ptr->DCBWDATALENGTH;
            } 
            else if (byteSize > cbw_ptr->DCBWDATALENGTH) 
            { /* Body */
                pDiskCtrl->pCSW->DCSWDATARESIDUE = USB_32BIT_LE(cbw_ptr->DCBWDATALENGTH);
                pDiskCtrl->pCSW->BCSWSTATUS = 2;
                byteSize = cbw_ptr->DCBWDATALENGTH;
            } /* Endbody */
            
            if (_usb_device_get_transfer_status(handle, pDiskCtrl->outEpNo, ARC_USB_RECV) != USB_OK) 
            {
                _usb_device_cancel_transfer(handle, ep_num, ARC_USB_RECV);
            } /* Endif */

            _usb_device_recv_data(handle, pDiskCtrl->outEpNo, 
                    pDiskCtrl->MASS_STORAGE_DISK + (index1*LENGTH_OF_EACH_LAB), byteSize);
        }
    } 
    else 
    { /* Body */
        USB_printf("disk write incorrect: FLAGS=0x%02x, LENGTH=0x%x\n", 
                    cbw_ptr->BMCBWFLAGS, cbw_ptr->DCBWDATALENGTH);

        /* Incorrect but valid CBW */
        pDiskCtrl->pCSW->DCSWDATARESIDUE = USB_32BIT_LE(cbw_ptr->DCBWDATALENGTH);
        pDiskCtrl->pCSW->BCSWSTATUS = 2;
        _usb_device_send_data(handle, pDiskCtrl->inEpNo, 0, 0);
        return;
    } /* Endbody */
   
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : _process_test_unit_ready
* Returned Value : None
* Comments       :
*     Responds appropriately to unit ready query
* 
*END*--------------------------------------------------------------------*/
void _process_test_unit_ready
   (
      /* [IN] Handle of the USB device */
      _usb_device_handle   handle,
            
      /* [IN] Endpoint number */
      uint_8               ep_num,
      
      /* [IN] Pointer to the data buffer */
      CBW_STRUCT_PTR       cbw_ptr
   )
{ /* Body */
    uint_32             bufSize;
    int                 devNo = _usb_device_get_dev_num(handle);
    USB_DISK_STRUCT*    pDiskCtrl = usbDisksPtr[devNo];
   
    if ((cbw_ptr->BMCBWFLAGS & USB_CBW_DIRECTION_BIT) ||
        (cbw_ptr->DCBWDATALENGTH == 0)) 
    {
        /* The actual length will never exceed the DCBWDATALENGTH */
        pDiskCtrl->pCSW->DCSWDATARESIDUE = 0;
        pDiskCtrl->pCSW->BCSWSTATUS = 0;
   
        pDiskCtrl->CBW_PROCESSED = FALSE;

        /* Send the command status information */
        _usb_device_send_data(handle, pDiskCtrl->inEpNo, (uint_8_ptr)pDiskCtrl->pCSW, 13);
        _usb_device_recv_data(handle, pDiskCtrl->outEpNo, pDiskCtrl->ep1_buf, 31);
    } 
    else 
    { /* Body */
      /* Incorrect but valid CBW */
        if (cbw_ptr->DCBWDATALENGTH > BUFFERSIZE)
            bufSize = BUFFERSIZE;
        else
            bufSize = cbw_ptr->DCBWDATALENGTH;

        pDiskCtrl->pCSW->DCSWDATARESIDUE = USB_32BIT_LE(cbw_ptr->DCBWDATALENGTH);
        pDiskCtrl->pCSW->BCSWSTATUS = 1;
        _usb_device_recv_data(handle, pDiskCtrl->outEpNo, pDiskCtrl->ep1_buf, bufSize);
    } /* Endbody */
   
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : _process_prevent_allow_medium_removal
* Returned Value : None
* Comments       :
*     Responds appropriately to unit ready query
* 
*END*--------------------------------------------------------------------*/
void _process_prevent_allow_medium_removal
   (
      /* [IN] Handle of the USB device */
      _usb_device_handle   handle,
            
      /* [IN] Endpoint number */
      uint_8               ep_num,
      
      /* [IN] Pointer to the data buffer */
      CBW_STRUCT_PTR       cbw_ptr
   )
{ /* Body */
    int                 devNo = _usb_device_get_dev_num(handle);
    USB_DISK_STRUCT*    pDiskCtrl = usbDisksPtr[devNo];

    /* The actual length will never exceed the DCBWDATALENGTH */
    pDiskCtrl->pCSW->DCSWDATARESIDUE = 0;
    pDiskCtrl->pCSW->BCSWSTATUS = 0;
   
    pDiskCtrl->CBW_PROCESSED = FALSE;

    /* Send the command status information */
    _usb_device_send_data(handle, pDiskCtrl->inEpNo, (uint_8_ptr)pDiskCtrl->pCSW, 13);
    _usb_device_recv_data(handle, pDiskCtrl->outEpNo, pDiskCtrl->ep1_buf, 31);
   
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : _process_mass_storage_command
* Returned Value : None
* Comments       :
*     Process a Mass storage class command
* 
*END*--------------------------------------------------------------------*/
void _process_mass_storage_command
   (
      /* [IN] Handle of the USB device */
      _usb_device_handle   handle,
            
      /* [IN] Endpoint number */
      uint_8               ep_num,
      
      /* [IN] Pointer to the data buffer */
      CBW_STRUCT_PTR       cbw_ptr
   )
{ /* Body */

    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_DISK, 
                    "disk command: CBWCB[0]=0x%02x, FLAGS=0x%02x, LENGTH=0x%x\n", 
                    cbw_ptr->CBWCB[0], cbw_ptr->BMCBWFLAGS, cbw_ptr->DCBWDATALENGTH);

   switch (cbw_ptr->CBWCB[0]) 
   {
      case 0x00: /* Request the device to report if it is ready */
         _process_test_unit_ready(handle, ep_num, cbw_ptr);
         break;

      case 0x12: /* Inquity command. Get device information */
         _process_inquiry_command(handle, ep_num, cbw_ptr);
         break;

      case 0x1A:
         _process_unsupported_command(handle, ep_num, cbw_ptr);
         break;

      case 0x1E: /* Prevent or allow the removal of media from a removable media device */
         _process_prevent_allow_medium_removal(handle, ep_num, cbw_ptr);
         break;

      case 0x23: /* Read Format Capacities. Report current media capacity and 
                 ** formattable capacities supported by media 
                  */
         /* We bahave like already installed medium. No need to send any data */
         _process_unsupported_command(handle, ep_num, cbw_ptr);
         break;

      case 0x25: /* Report current media capacity */
         _process_report_capacity(handle, ep_num, cbw_ptr);
         break;

      case 0x28: /* Read (10) Transfer binary data from media to the host */
      case 0x3E:
          _process_read_command(handle, ep_num, cbw_ptr);
         break;

      case 0x2A: /* Write (10) Transfer binary data from the host to the media */
      case 0x3F:
         _process_write_command(handle, ep_num, cbw_ptr);
         break;

      case 0x01: /* Position a head of the drive to zero track */
      case 0x03: /* Transfer status sense data to the host */
      case 0x04: /* Format unformatted media */
      case 0x1B: /* Request a request a removable-media device to load or 
                 ** unload its media 
                 */
      case 0x1D: /* Perform a hard reset and execute diagnostics */
      case 0x2B: /* Seek the device to a specified address */
      case 0x2E: /* Transfer binary data from the host to the media and 
                 ** verify data 
                 */
      case 0x2F: /* Verify data on the media */
      case 0x55: /* Allow the host to set parameters in a peripheral */
      case 0x5A: /* Report parameters to the host */
      case 0xA8: /* Read (12) Transfer binary data from the media to the host */
      case 0xAA: /* Write (12) Transfer binary data from the host to the 
                 ** media 
                 */
      default:
         _process_unsupported_command(handle, ep_num, cbw_ptr);
         break;
   } /* Endswitch */

} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : service_ep1
* Returned Value : None
* Comments       :
*     Called upon a completed endpoint 1 (USB 1.1 Chapter 9) transfer
* 
*END*--------------------------------------------------------------------*/
static void service_ep1
   (
      /* [IN] Handle of the USB device */
      _usb_device_handle   handle,

      /* [IN] Service type as registered */
      uint_8               type,
      
      /* [IN] Is it a setup packet? */
      boolean              setup,
      
      /* [IN] Direction of the transfer.  Is it transmit? */
      uint_8               direction,
      
      /* [IN] Pointer to the data buffer */
      uint_8_ptr           buffer,
      
      /* [IN] Length of the transfer */
      uint_32              length,

      /* [IN] Error, if any */
      uint_8               error
            
   )
{ /* Body */
    int                 devNo = _usb_device_get_dev_num(handle);
    USB_DISK_STRUCT*    pDiskCtrl = usbDisksPtr[devNo];
    CBW_STRUCT_PTR cbw_ptr = (CBW_STRUCT_PTR)((pointer)buffer);      

    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_EP1, 
                    "disk %s: ep=%d, dir=%s, pBuf=0x%x, length=%d, error=0x%x\n",
                    __FUNCTION__, type, (direction == ARC_USB_RECV) ? "RECV" : "SEND", 
                    (unsigned)buffer, (int)length, error);

    if ((!direction) && (!pDiskCtrl->CBW_PROCESSED) && (length == 31) && 
        (cbw_ptr->DCBWSIGNATURE == USB_32BIT_LE(USB_DCBWSIGNATURE))) 
    {
        /* A valid CBW was received */
        pDiskCtrl->pCSW->DCSWSIGNATURE = USB_32BIT_LE(USB_DCSWSIGNATURE);
        pDiskCtrl->pCSW->DCSWTAG = cbw_ptr->DCBWTAG;
        pDiskCtrl->CBW_PROCESSED = TRUE;

        /* Swap 32 bit fields if neccessary */
        cbw_ptr->DCBWDATALENGTH = USB_32BIT_LE(cbw_ptr->DCBWDATALENGTH);

        /* Process the command */
        _process_mass_storage_command(handle, type, cbw_ptr);
    } 
    else 
    {
        /* If a CBW was processed then send the status information and 
        ** queue another cbw receive request, else just queue another CBW receive
        ** request if we received an invalid CBW 
        */
        if (pDiskCtrl->CBW_PROCESSED) 
        {
            int     i;

            ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_DISK_DATA, 
                    "disk %s: ep=%d, dir=%s, pBuf=0x%x, length=%d, error=0x%x\n",
                    __FUNCTION__, type, (direction == ARC_USB_RECV) ? "RECV" : "SEND", 
                    (unsigned)buffer, (int)length, error);

            for(i=0; i<64; i++)
            {
                if( (i % 16) == 0)
                {
                    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_DISK_DUMP, "\n0x%08x: ", &buffer[i]);
                }
                ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_DISK_DUMP, "%02x ", buffer[i]);
                if( (i % 3) == 0)
                    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_DISK_DUMP, " ");                
            }

            if (pDiskCtrl->ZERO_TERMINATE) 
            { /* Body */
                pDiskCtrl->ZERO_TERMINATE = FALSE;
                _usb_device_send_data(handle, pDiskCtrl->inEpNo, 0, 0);
            } 
            else 
            { /* Body */
                pDiskCtrl->CBW_PROCESSED = FALSE;

                /* Send the command status information */
                _usb_device_send_data(handle, pDiskCtrl->inEpNo, (uint_8_ptr)pDiskCtrl->pCSW, 13);        
                _usb_device_recv_data(handle, pDiskCtrl->outEpNo, pDiskCtrl->ep1_buf, 31);
            }
        } 
        else
        {
            if (!direction) 
            {
                USB_printf("usbDisk_%d, %s: Wrong direction = %d\n", 
                    _usb_device_get_dev_num(handle), __FUNCTION__, direction);
                _usb_device_stall_endpoint(handle, pDiskCtrl->outEpNo, ARC_USB_RECV);
                _usb_device_stall_endpoint(handle, pDiskCtrl->inEpNo, ARC_USB_SEND);

                /* Invalid CBW received. Queue another receive buffer */
                _usb_device_recv_data(handle, pDiskCtrl->outEpNo, pDiskCtrl->ep1_buf, 31);  
            }
        } /* Endif */
    } /* Endif */
   
    return;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : service_speed
* Returned Value : None
* Comments       :
*     Called upon a speed detection event.
* 
*END*--------------------------------------------------------------------*/
static void service_speed
   (
      /* [IN] Handle of the USB device */
      _usb_device_handle   handle,

      /* [IN] request type as registered */
      uint_8               type,

      /* [IN] Unused */
      boolean              setup,
   
      /* [IN] Unused */
      uint_8               direction,
   
      /* [IN] Unused */
      uint_8_ptr           buffer,
   
      /* [IN] Unused */
      uint_32              length,

      /* [IN] Error, if any */
      uint_8               error
            
   )
{ /* EndBody */
    int                 devNo = _usb_device_get_dev_num(handle);
    USB_DISK_STRUCT*    pDiskCtrl = usbDisksPtr[devNo];
    
    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_SPEED, "disk %s: speed = %d\n", __FUNCTION__, (unsigned)length);

    pDiskCtrl->speed = length;
    return;
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : reset_ep0
* Returned Value : None
* Comments       :
*     Called upon a bus reset event.  Initialises the control endpoint.
* 
*END*--------------------------------------------------------------------*/
static void reset_ep0
   (
      /* [IN] Handle of the USB device */
      _usb_device_handle   handle,
   
      /* [IN] request type as registered */
      uint_8               type,

      /* [IN] Unused */
      boolean              setup,
   
      /* [IN] Unused */
      uint_8               direction,
   
      /* [IN] Unused */
      uint_8_ptr           buffer,
   
      /* [IN] Unused */
      uint_32              length,

      /* [IN] Error, if any */
      uint_8               error
            
   )
{ /* Body */
    int                 devNo = _usb_device_get_dev_num(handle);
    USB_DISK_STRUCT*    pDiskCtrl = usbDisksPtr[devNo];
    
    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_RESET, "disk-%d %s: pDiskCtrl=%p, handle=%p\n", 
                    devNo, __FUNCTION__, pDiskCtrl, handle);

    /* on a reset always ensure all transfers are cancelled on control EP*/
    _usb_device_cancel_transfer(handle, 0, ARC_USB_RECV);
    _usb_device_cancel_transfer(handle, 0, ARC_USB_SEND);

    _usb_device_start(handle);
    /* Initialize the endpoint 0 in both directions */
    _usb_device_init_endpoint(handle, 0, pDiskCtrl->DevDesc[DEV_DESC_MAX_PACKET_SIZE], 
                                ARC_USB_RECV, ARC_USB_CONTROL_ENDPOINT, 0);
    _usb_device_init_endpoint(handle, 0, pDiskCtrl->DevDesc[DEV_DESC_MAX_PACKET_SIZE], 
                                ARC_USB_SEND, ARC_USB_CONTROL_ENDPOINT, 0);


    if (pDiskCtrl->TEST_ENABLED) 
    {
        int out_ep_count=0, in_ep_count=0;

        while(_usb_device_get_transfer_status(handle, pDiskCtrl->outEpNo, ARC_USB_RECV) != 
                                                    ARC_USB_STATUS_IDLE)
        {
            out_ep_count++;
            _usb_device_cancel_transfer(handle, pDiskCtrl->outEpNo, ARC_USB_RECV);
        }
        while(_usb_device_get_transfer_status(handle, pDiskCtrl->inEpNo, ARC_USB_SEND) != 
                                                    ARC_USB_STATUS_IDLE)
        {
            in_ep_count++;
            _usb_device_cancel_transfer(handle, pDiskCtrl->inEpNo, ARC_USB_SEND);
        }
        ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_RESET, "disk %s: out_ep_count=%d, in_ep_count=%d\n", 
                        __FUNCTION__, out_ep_count, in_ep_count);
    } /* Endif */

    pDiskCtrl->TEST_ENABLED = FALSE;

    return;
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : usbDiskLoad - main task
* Inputs:
*   int diskSize  - size of created disk in KBytes    
* Returned Value : None
* Comments       :
*     First function called.  Initialises the USB and registers Chapter 9
*     callback functions.
* 
*END*--------------------------------------------------------------------*/
_usb_device_handle  usbDiskLoad(int devNo, int diskSize)
{ /* Body */
    _usb_device_handle  handle;
    USB_DISK_STRUCT*    pDiskCtrl;
    uint_8_ptr          Send_Buffer_aligned;
    uint_8              error;
    uint_32             send_data_buffer_size=0;
    uint_8_ptr          temp;
    int                 lockKey, i, j;
    static boolean      isFirst = TRUE;

    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_INIT, "%s: devNo=%d, diskSize=%d\n", 
                        __FUNCTION__, devNo, diskSize);

    if(devNo >= MAX_USB_DEVICES)
    {
        USB_printf("USB disk: devNo=%d too large\n", devNo);
        return NULL;
    }

    /*lock interrupts */
    lockKey = USB_lock();

    if(isFirst)
    {
        for(i=0; i<MAX_USB_DEVICES; i++)
        {
            usbDisksPtr[i] = NULL;
        }
        /* Swap all USB_STRING_DESC */
        for(i=0; i<(sizeof(USB_STRING_DESC)/sizeof(USB_STRING_DESC[0])); i++)
        {
            uint_16* usbStr = (uint_16*)(USB_STRING_DESC[i]);
            uint_16 size = (usbStr[0]-0x300)/sizeof(uint_16);

            for(j=0; j<size; j++)
            {
                usbStr[j] = USB_16BIT_LE(usbStr[j]);
            }
        }
        isFirst = FALSE;
    }

    if(usbDisksPtr[devNo] != NULL)
    {
        USB_printf("USB disk: devNo=%d is busy\n", devNo);
        USB_unlock(lockKey);
        return NULL;
    }

    /* Allocate Disk control structure */
    pDiskCtrl = USB_memalloc(sizeof(USB_DISK_STRUCT));
    if(pDiskCtrl == NULL)
    {
        USB_printf("USB disk #%d: Can't allocate USB_DISK_STRUCT (%d bytes)\n", 
                    devNo, sizeof(USB_DISK_STRUCT));
        USB_unlock(lockKey);
        return NULL;
    }
    USB_memzero(pDiskCtrl, sizeof(USB_DISK_STRUCT));

    if(diskSize == 0)
        pDiskCtrl->logicalBlocks = TOTAL_LOGICAL_ADDRESS_BLOCKS;
    else
        pDiskCtrl->logicalBlocks = (diskSize*1024)/LENGTH_OF_EACH_LAB;

    if(pDiskCtrl->logicalBlocks < 16)
    {
        USB_printf("USB disk size (%d) is too small. Minimum is 8 Kbytes\n",
                    diskSize);
        USB_unlock(lockKey);
        return NULL;
    }
    
    pDiskCtrl->devNo = devNo;
    pDiskCtrl->hsMaxPktSize = diskHsMaxPktSize;
    pDiskCtrl->fsMaxPktSize = diskFsMaxPktSize;

    pDiskCtrl->inEpType     = diskInEpType;
    pDiskCtrl->outEpType    = diskOutEpType;

    pDiskCtrl->inEpNo       = diskInEpNo;
    pDiskCtrl->outEpNo      = diskOutEpNo;

    /* Initialize the USB interface */
    error = _usb_device_init(devNo, &handle);
    if (error != USB_OK) 
    {
        USB_unlock(lockKey);
        USB_printf("\nUSB Initialization failed. Error: %x", error);
        return NULL;
    } /* Endif */

    /* Self Power, Remote wakeup disable */
    _usb_device_set_status(handle, ARC_USB_STATUS_DEVICE, (1 << DEVICE_SELF_POWERED));    

    error = _usb_device_register_service(handle, ARC_USB_SERVICE_EP0, service_ep0);   
    if (error != USB_OK) 
    {
        USB_unlock(lockKey);
        USB_printf("\nUSB Service Registration failed. Error: %x", error);
        return NULL;
    } /* Endif */
   
    error = _usb_device_register_service(handle, ARC_USB_SERVICE_BUS_RESET, reset_ep0);   
    if (error != USB_OK) 
    {
        USB_unlock(lockKey);
        USB_printf("\nUSB Service Registration failed. Error: %x", error);
        return NULL;
    } /* Endif */
   
    error = _usb_device_register_service(handle, ARC_USB_SERVICE_SPEED_DETECTION, 
                                                        service_speed);
    if (error != USB_OK) 
    {
        USB_unlock(lockKey);
        USB_printf("\nUSB Service Registration failed. Error: %x", error);
        return NULL;
    } /* Endif */
         
    error = _usb_device_register_service(handle, pDiskCtrl->outEpNo, service_ep1);   
    if (error != USB_OK) 
    {
        USB_unlock(lockKey);
        USB_printf("\nUSB Service Registration failed. Error: %x", error);
        return NULL;
    } /* Endif */

    if(pDiskCtrl->outEpNo != pDiskCtrl->inEpNo)
    {
        error = _usb_device_register_service(handle, pDiskCtrl->inEpNo, service_ep1);   
        if (error != USB_OK) 
        {
            USB_unlock(lockKey);
            USB_printf("\nUSB Service Registration failed. Error: %x", error);
            return NULL;
        } /* Endif */
    }

    /**************************************************************************
    Best way to handle the Data cache is to allocate a large buffer that is
    cache aligned and keep all data inside it. Flush the line of the cache
    that you have changed. In this program, we have static data such as 
    descriptors which never changes. Such data can be kept in this buffer
    and flushed only once. Note that you can reduce the size of this buffer
    by aligning the addresses in a different way.   
    ***************************************************************************/
    send_data_buffer_size =  (DEVICE_DESCRIPTOR_SIZE +  PSP_CACHE_LINE_SIZE) +
                            (CONFIG_DESC_SIZE + PSP_CACHE_LINE_SIZE) +
                            (DEVICE_QUALIFIER_DESCRIPTOR_SIZE + PSP_CACHE_LINE_SIZE) +
                            (OTHER_SPEED_CONFIG_DESC_SIZE + PSP_CACHE_LINE_SIZE) +
                            (BUFFERSIZE + PSP_CACHE_LINE_SIZE) +
                            (EP_TEMP_BUFFERSIZE + PSP_CACHE_LINE_SIZE) + 
                            (sizeof(DISK_READ_CAPACITY) + PSP_CACHE_LINE_SIZE) +
                            (sizeof(CSW_STRUCT) + PSP_CACHE_LINE_SIZE) +
                            (pDiskCtrl->logicalBlocks*LENGTH_OF_EACH_LAB + PSP_CACHE_LINE_SIZE);

    pDiskCtrl->Send_Buffer_Unaligned   = (uint_8_ptr) USB_memalloc(send_data_buffer_size);
    if (pDiskCtrl->Send_Buffer_Unaligned == NULL) 
    {
        USB_unlock(lockKey);
        USB_printf("diskLoad: Buffer allocation of %d bytes is failed\n", 
                    (unsigned)send_data_buffer_size);
        return NULL;
    }
   
    Send_Buffer_aligned = (uint_8_ptr) USB_CACHE_ALIGN((uint_32)pDiskCtrl->Send_Buffer_Unaligned);
    /* keep a temporary copy of the aligned address */
    temp = Send_Buffer_aligned;
   
    /**************************************************************************
    Assign pointers to different buffers from it and copy data inside.
    ***************************************************************************/
    pDiskCtrl->DevDesc =  (uint_8_ptr) Send_Buffer_aligned;
    USB_memcopy(DevDescData, pDiskCtrl->DevDesc, DEVICE_DESCRIPTOR_SIZE);
    Send_Buffer_aligned += ((DEVICE_DESCRIPTOR_SIZE/PSP_CACHE_LINE_SIZE) + 1)* PSP_CACHE_LINE_SIZE; 
   
    pDiskCtrl->ConfigDesc =  (uint_8_ptr) Send_Buffer_aligned;
    USB_memcopy(ConfigDescData, pDiskCtrl->ConfigDesc, CONFIG_DESC_SIZE);
    Send_Buffer_aligned += ((CONFIG_DESC_SIZE/PSP_CACHE_LINE_SIZE) + 1)* PSP_CACHE_LINE_SIZE; 
   
    pDiskCtrl->DevQualifierDesc =  (uint_8_ptr) Send_Buffer_aligned;
    USB_memcopy(DevQualifierDescData, pDiskCtrl->DevQualifierDesc, DEVICE_QUALIFIER_DESCRIPTOR_SIZE);
    Send_Buffer_aligned += ((DEVICE_QUALIFIER_DESCRIPTOR_SIZE/PSP_CACHE_LINE_SIZE) + 1) * PSP_CACHE_LINE_SIZE;

    pDiskCtrl->other_speed_config =  (uint_8_ptr) Send_Buffer_aligned;
    USB_memcopy(other_speed_config_data, pDiskCtrl->other_speed_config, OTHER_SPEED_CONFIG_DESC_SIZE);
    Send_Buffer_aligned += ((OTHER_SPEED_CONFIG_DESC_SIZE/PSP_CACHE_LINE_SIZE) + 1)* PSP_CACHE_LINE_SIZE; 

    /*buffer to receive data from Bulk OUT */
    pDiskCtrl->ep1_buf =  (uint_8_ptr) Send_Buffer_aligned;
    USB_memzero(pDiskCtrl->ep1_buf, BUFFERSIZE);
    Send_Buffer_aligned += ((BUFFERSIZE/PSP_CACHE_LINE_SIZE) + 1)* PSP_CACHE_LINE_SIZE;
   
    /*buffer for control endpoint to send data */
    pDiskCtrl->epTemp_buf =  (uint_8_ptr) Send_Buffer_aligned;
    USB_memzero(pDiskCtrl->epTemp_buf, EP_TEMP_BUFFERSIZE);
    
    Send_Buffer_aligned += ((EP_TEMP_BUFFERSIZE/PSP_CACHE_LINE_SIZE) + 1)* PSP_CACHE_LINE_SIZE; 

    /* Buffer for read Capacity message */
    pDiskCtrl->pReadCapacity = (DISK_READ_CAPACITY*)Send_Buffer_aligned;
    USB_memcopy((void*)&read_capacity, pDiskCtrl->pReadCapacity, sizeof(DISK_READ_CAPACITY));

    /* Update read_capacity */
    pDiskCtrl->pReadCapacity->LAST_LOGICAL_BLOCK_ADDRESS[2] = 
                            USB_uint_16_high(pDiskCtrl->logicalBlocks-14);
    pDiskCtrl->pReadCapacity->LAST_LOGICAL_BLOCK_ADDRESS[3] = 
                            USB_uint_16_low(pDiskCtrl->logicalBlocks-14);

    Send_Buffer_aligned += ((sizeof(DISK_READ_CAPACITY)/PSP_CACHE_LINE_SIZE) + 1) * PSP_CACHE_LINE_SIZE; 

    /* Buffer for CSW message */
    pDiskCtrl->pCSW = (CSW_STRUCT*)Send_Buffer_aligned;
    USB_memzero(pDiskCtrl->pCSW , sizeof(CSW_STRUCT));
    
    Send_Buffer_aligned += ((sizeof(CSW_STRUCT)/PSP_CACHE_LINE_SIZE) + 1) * PSP_CACHE_LINE_SIZE; 

    /*buffer for storage disk */
    pDiskCtrl->MASS_STORAGE_DISK = (uint_8_ptr)Send_Buffer_aligned;
    
    USB_printf("usbDisk-%d: pDiskCtrl=%p, %d bytes allocated addr=0x%x\n", 
                 devNo, pDiskCtrl, (unsigned)send_data_buffer_size, 
                 (unsigned)pDiskCtrl->Send_Buffer_Unaligned);
    USB_printf("usbDisk-%d: DevDesc=0x%x, ConfigDesc=0x%x, QualifierDesc=0x%x, otherSpeedDesc=0x%x\n",
                 devNo, (unsigned)pDiskCtrl->DevDesc, (unsigned)pDiskCtrl->ConfigDesc, 
                 (unsigned)pDiskCtrl->DevQualifierDesc, (unsigned)pDiskCtrl->other_speed_config);
    USB_printf("usbDisk-%d: ep1_buf=0x%x, epTemp_buf=0x%x, MASS_STORAGE_DISK=0x%x\n",
                 devNo, (unsigned)pDiskCtrl->ep1_buf, (unsigned)pDiskCtrl->epTemp_buf, 
                 (unsigned)pDiskCtrl->MASS_STORAGE_DISK);
    
    USB_memzero(pDiskCtrl->MASS_STORAGE_DISK, (pDiskCtrl->logicalBlocks*LENGTH_OF_EACH_LAB));

    /* Format the "disk" */      
    USB_memcopy(BOOT_SECTOR_AREA, pDiskCtrl->MASS_STORAGE_DISK, 512);

    /* Update BOOT Sector "Total Small sectors" field */
    pDiskCtrl->MASS_STORAGE_DISK[19] = USB_uint_16_low(pDiskCtrl->logicalBlocks);
    pDiskCtrl->MASS_STORAGE_DISK[20] = USB_uint_16_high(pDiskCtrl->logicalBlocks);

    USB_memcopy((void *)FAT16_SPECIAL_BYTES, pDiskCtrl->MASS_STORAGE_DISK + 512, 3);
    USB_memcopy((void *)FAT16_SPECIAL_BYTES, pDiskCtrl->MASS_STORAGE_DISK + 512*4, 3);
                         
    /**************************************************************************
    Flush the cache to ensure main memory is updated.
    ***************************************************************************/
    USB_dcache_flush(temp, send_data_buffer_size);

    pDiskCtrl->usbDevHandle = handle;
    usbDisksPtr[devNo] = pDiskCtrl;

    USB_unlock(lockKey);

    USB_printf("USB Disk is READY: diskSize=%d KBytes, blockSize=%d Bytes, numBlocks=%d\n",
                diskSize, LENGTH_OF_EACH_LAB, pDiskCtrl->logicalBlocks);

    return pDiskCtrl->usbDevHandle;
} /* Endbody */

void    usbDiskUnload(_usb_device_handle handle)
{
    int                 lockKey;
    int                 devNo = _usb_device_get_dev_num(handle);
    USB_DISK_STRUCT*    pDiskCtrl = usbDisksPtr[devNo];

    if(pDiskCtrl == NULL)
    {
        USB_printf("USB disk #%d: Disk is not loaded\n", pDiskCtrl->devNo);
        return;
    }
    /*lock interrupts */
    lockKey = USB_lock();
    
    /* ensure all transfers are cancelled */
    _usb_device_cancel_transfer(handle, pDiskCtrl->outEpNo, ARC_USB_RECV);
    _usb_device_cancel_transfer(handle, pDiskCtrl->inEpNo,  ARC_USB_SEND);

    /* Stop Endpoints */
    _usb_device_deinit_endpoint(handle, pDiskCtrl->outEpNo, ARC_USB_RECV);
    _usb_device_deinit_endpoint(handle, pDiskCtrl->inEpNo, ARC_USB_SEND);

    _usb_device_deinit_endpoint(handle, 0, ARC_USB_RECV);
    _usb_device_deinit_endpoint(handle, 0, ARC_USB_SEND);

    _usb_device_stop(handle);

    /* Deregister all services */
    _usb_device_unregister_service(handle, ARC_USB_SERVICE_EP0);   
    _usb_device_unregister_service(handle, ARC_USB_SERVICE_BUS_RESET);   
    _usb_device_unregister_service(handle, ARC_USB_SERVICE_SPEED_DETECTION);
    _usb_device_unregister_service(handle, pDiskCtrl->outEpNo);   
    if(pDiskCtrl->outEpNo != pDiskCtrl->inEpNo)
    {
        _usb_device_unregister_service(handle, pDiskCtrl->inEpNo);   
    }

    _usb_device_shutdown(handle);

    /* Free memory allocated for Disk device */
    if(pDiskCtrl->Send_Buffer_Unaligned != NULL)
    {
        USB_memfree(pDiskCtrl->Send_Buffer_Unaligned);
    }

    /* Free Control structure */
    USB_memfree(pDiskCtrl);
    usbDisksPtr[devNo] = NULL;

    USB_unlock(lockKey);
}

/* EOF */

