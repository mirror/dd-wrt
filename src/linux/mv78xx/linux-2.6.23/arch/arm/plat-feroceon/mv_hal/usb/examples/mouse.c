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
Include the USB stack header files.
**************************************************************************/
#include "mvUsbDefs.h"
#include "mvUsbDebug.h"
#include "mvUsbCh9.h"
#include "mvUsbDevApi.h"

#include "mouse.h"


/**************************************************************************
global variables and some defines for device.
**************************************************************************/

#define CONTROL_MAX_PACKET_SIZE         (64)
#define INTERRUPT_MAX_PACKET_SIZE       (0x0008)
#define DEV_DESC_MAX_PACKET_SIZE        (7)
#define INTERRUPT_EP                    (1)
#define FRAME_INTERVAL                  (15)

/**************************************************************************
Include the OS and BSP dependent files that define IO functions and
basic types. You may like to change these files for your board and RTOS 
**************************************************************************/

int frame_interval = FRAME_INTERVAL;
int mouseCntr = 0;
int mouseDelay = 2;

static volatile boolean TEST_ENABLED = FALSE;
static volatile boolean USB_SUSPENDED = FALSE;

#define EP1_RECV_BUFFER_SIZE 10
static uint_8_ptr hid_test_rep_data;
static uint_8_ptr hid_test_rep_data_unaligned;

/********************************
Buffers for sending data to stack
********************************/
#define EP0_SEND_BUFFER_SIZE 200
static uint_8_ptr Send_Buffer_Unaligned;
static uint_8_ptr Send_Buffer_aligned;

static uint_8         data_to_send;
static uint_16        sof_count;
static SETUP_STRUCT   local_setup_packet;


/*************************************************************************
Device descriptors are always 18 bytes 

Offset|       Field        | Value |  Description  
------|--------------------|-------|--------------------
  0   |      bLength       |  0x12 |The size of this 
      |                    |       |descriptor is 18 bytes
------|--------------------|-------|--------------------
  1   |  bDescriptorType   |  0x01 |DEVICE Descriptor Type
------|--------------------|-------|--------------------
  2   |       bcdUSB       | 0x0100|Device compliant to 
      |                    |       |the USB 
      |                    |       |specification 
      |                    |       |version 1.00   
------|--------------------|-------|--------------------
  4   |    bDeviceClass    |  0x00 |Each interface 
      |                    |       |specifies its own 
      |                    |       |class information
------|--------------------|-------|--------------------
  5   |  bDeviceSubClass   |  0x00 |Each interface 
      |                    |       |specifies its own 
      |                    |       |subclass information
------|--------------------|-------|--------------------
  6   |  bDeviceProtocol   |  0x00 |No protocols on the 
      |                    |       |device basis
------|--------------------|-------|--------------------
  7   |  bMaxPacketSize0   |  0x08 |Maximum packet size 
      |                    |       |for endpoint zero is 8
------|--------------------|-------|--------------------
  8   |      idVendor      | 0x0261|Vendor ID is 609: 
      |                    |       
------|--------------------|-------|--------------------
  10  |     idProduct      | 0x4D03|The Product ID is 0x4D03
------|--------------------|-------|--------------------
  12  |     bcdDevice      | 0x0441|The device release 
      |                    |       |number is 4.41
------|--------------------|-------|--------------------
  14  |   iManufacturer    |  0x00 |The device doesn't 
      |                    |       |have the string 
      |                    |       |descriptor 
      |                    |       |describing the manufacturer
------|--------------------|-------|--------------------
  15  |      iProduct      |  0x00 |The device doesn't 
      |                    |       |have the string 
      |                    |       |descriptor 
      |                    |       |describing the product
------|--------------------|-------|--------------------
  16  |   iSerialNumber    |  0x00 | 
------|--------------------|-------|--------------------
  17  | bNumConfigurations |  0x01 | 
------|--------------------|-------|--------------------
*************************************************************************/
#define DEVICE_DESCRIPTOR_SIZE 18
static uint_8_ptr DevDesc;
static uint_8  DevDescData[DEVICE_DESCRIPTOR_SIZE] =
{
   DEVICE_DESCRIPTOR_SIZE,
   0x01,
   0x0,2,
   
   0x00,
   0x00,
   0x00,
   CONTROL_MAX_PACKET_SIZE,
   /* Vendor ID = MARVELL */
   USB_uint_16_low(0x1286), USB_uint_16_high(0x1286),
   /* Product ID */
   USB_uint_16_low(0x1), USB_uint_16_high(0x1),
   /* BCD Device version */
   USB_uint_16_low(0x0002), USB_uint_16_high(0x0002),

   0x01,                      /* iManufacturer */
   0x02,                      /* iProduct */
   0x00,                      /* iSerialNumber */
   0x01                       /* bNumConfigurations */
   
};

/* USB 2.0 specific descriptor */
#define DEVICE_QUALIFIER_DESCRIPTOR_SIZE 10
static uint_8_ptr DevQualifierDesc;
static uint_8  DevQualifierDescData[DEVICE_QUALIFIER_DESCRIPTOR_SIZE] =
{
   DEVICE_QUALIFIER_DESCRIPTOR_SIZE,  /* bLength Length of this descriptor */
   6,                         /* bDescType This is a DEVICE Qualifier descr */
   0,2,                       /* bcdUSB USB revision 2.0 */
   0,                         /* bDeviceClass */
   0,                         /* bDeviceSubClass */
   0,                         /* bDeviceProtocol */
   CONTROL_MAX_PACKET_SIZE,  /* bMaxPacketSize0 */
   0x01,                      /* bNumConfigurations */
   0
};


/*******************************************************************
   CONFIG DESCRIPTOR

Data stage (34 bytes) :
------------------------------------

       CONFIGURATION Descriptor
       ------------------------
Offset|        Field        | Value |  Description  
------|---------------------|-------|--------------------
  0   |       bLength       |  0x09 |The size of this 
      |                     |       |descriptor is 9 bytes
------|---------------------|-------|--------------------
  1   |   bDescriptorType   |  0x02 |CONFIGURATION 
      |                     |       |Descriptor Type
------|---------------------|-------|--------------------
  2   |    wTotalLength     | 0x0022|The total length of 
      |                     |       |data for this 
      |                     |       |configuration is 34. 
      |                     |       |This includes the 
      |                     |       |combined length of 
      |                     |       |all the descriptors returned
------|---------------------|-------|--------------------
  4   |   bNumInterfaces    |  0x01 |This configuration 
      |                     |       |supports 1 interfaces
------|---------------------|-------|--------------------
  5   | bConfigurationValue |  0x01 |The value 1 should 
      |                     |       |be used to select 
      |                     |       |this configuration
------|---------------------|-------|--------------------
  6   |   iConfiguration    |  0x00 |The device doesn't 
      |                     |       |have the string 
      |                     |       |descriptor 
      |                     |       |describing this configuration
------|---------------------|-------|--------------------
  7   |    bmAttributes     |  0x80 |Configuration characteristics :
      |                     |       |Bit 7: Reserved (set to one) 1 
      |                     |       |Bit 6: Self-powered          0 
      |                     |       |Bit 5: Remote Wakeup         1 
------|---------------------|-------|--------------------
  8   |      MaxPower       |  0x32 |Maximum power 
      |                     |       |consumption of the 
      |                     |       |device in this 
      |                     |       |configuration is 100 mA
------|---------------------|-------|--------------------

       INTERFACE Descriptor
       --------------------
Offset|       Field        | Value |  Description  
------|--------------------|-------|--------------------
  0   |      bLength       |  0x09 |The size of this 
      |                    |       |descriptor is 9 bytes
------|--------------------|-------|--------------------
  1   |  bDescriptorType   |  0x04 |INTERFACE Descriptor Type
------|--------------------|-------|--------------------
  2   |  bInterfaceNumber  |  0x00 |The number of this 
      |                    |       |interface is 0
------|--------------------|-------|--------------------
  3   | bAlternateSetting  |  0x00 |The value used to 
      |                    |       |select alternate 
      |                    |       |setting for this 
      |                    |       |interface is 0
------|--------------------|-------|--------------------
  4   |   bNumEndpoints    |  0x01 |The number of 
      |                    |       |endpoints used by 
      |                    |       |this interface is 1 
      |                    |       |(excluding endpoint zero)
------|--------------------|-------|--------------------
  5   |  bInterfaceClass   |  0x03 |The interface 
      |                    |       |implements HID class
------|--------------------|-------|--------------------
  6   | bInterfaceSubClass |  0x01 |The subclass code is 0x01
------|--------------------|-------|--------------------
  7   | bInterfaceProtocol |  0x02 |The protocol code is 0x02
------|--------------------|-------|--------------------
  8   |     iInterface     |  0x00 |The device doesn't 
      |                    |       |have the string 
      |                    |       |descriptor 
      |                    |       |describing this interface
------|--------------------|-------|--------------------

       HID Descriptor
       --------------
Offset|       Field       | Value |  Description  
------|-------------------|-------|--------------------
  0   |      bLength      |  0x09 |The size of this 
      |                   |       |descriptor is 9 bytes
------|-------------------|-------|--------------------
  1   |  bDescriptorType  |  0x21 |HID Descriptor Type
------|-------------------|-------|--------------------
  2   |      bcdHID       | 0x0100|Device compliant to 
      |                   |       |the HID 
      |                   |       |specification 
      |                   |       |version 1.00   
------|-------------------|-------|--------------------
  4   |   bCountryCode    |  0x00 |The country code is 0x00
------|-------------------|-------|--------------------
  5   |  bNumDescriptors  |  0x01 |The number of class 
      |                   |       |descriptors is 1
------|-------------------|-------|--------------------
  6   |  bDescriptorType  |  0x22 |The class descriptor 
      |                   |       |is Report descriptor
------|-------------------|-------|--------------------
  7   | wDescriptorlength | 0x0034|The total size of 
      |                   |       |the class descriptor 
      |                   |       |is 52
------|-------------------|-------|--------------------

       ENDPOINT Descriptor
       -------------------
Offset|      Field       | Value |  Description  
------|------------------|-------|--------------------
  0   |     bLength      |  0x07 |The size of this 
      |                  |       |descriptor is 7 bytes
------|------------------|-------|--------------------
  1   | bDescriptorType  |  0x05 |ENDPOINT Descriptor Type
------|------------------|-------|--------------------
  2   | bEndpointAddress |  0x81 |This is an IN 
      |                  |       |endpoint with 
      |                  |       |address (endpoint 
      |                  |       |number) 1
------|------------------|-------|--------------------
  3   |   bmAttributes   |  0x03 |Types - 
      |                  |       |Transfer:INTERRUPT 
      |                  |       |Sync:No Sync 
      |                  |       |Usage:Data EP
------|------------------|-------|--------------------
  4   |  wMaxPacketSize  | 0x0004|Maximum packet size 
      |                  |       |value for this 
      |                  |       |endpoint is 0x4 
      |                  |       |(Bits 12-11: Addtl. Transactions/frame)
------|------------------|-------|--------------------
  6   |    bInterval     |  0x0A |bInterval:10. The 
      |                  |       |polling interval 
      |                  |       |value is bInterval 
      |                  |       |or 2**(bInterval-1)
------|------------------|-------|--------------------

*******************************************************************/


#define CONFIG_DESC_NUM_INTERFACES  (4)

/* This must be counted manually and updated with the descriptor */
/* 1*Config(9) + 1*Interface(9) + 1*HID(9) + 1* Endpoint (7)= 34 bytes */
#define CONFIG_DESC_SIZE            (34)

static uint_8_ptr ConfigDesc;

static uint_8 ConfigDescData[CONFIG_DESC_SIZE] =
{
   /*Config Descriptor */
   0x09,
   0x02,
   USB_uint_16_low(CONFIG_DESC_SIZE),
   USB_uint_16_high(CONFIG_DESC_SIZE),
   0x01,
   0x01,
   0x00,
   0xE0, /* 0x80, */
   0x0,
   /* Interface Descriptor */	
   0x09,
   0x04,
   0x00,
   0x00,
   0x01,
   0x03,
   0x01,
   0x02,
   0x00,

   /* HID descriptor */
   0x09,
   0x21,
   USB_uint_16_low(0x0100),
   USB_uint_16_high(0x0100),
   0x00,
   0x01,
   0x22,
   USB_uint_16_low(0x0034),
   USB_uint_16_high(0x0034),
 
   /*Endpoint descriptor */
   0x07,
   0x05,
   (0x80+INTERRUPT_EP),
   0x03,
   USB_uint_16_low(INTERRUPT_MAX_PACKET_SIZE), 
   USB_uint_16_high(INTERRUPT_MAX_PACKET_SIZE),
   FRAME_INTERVAL
};

#define OTHER_SPEED_CONFIG_DESC_SIZE  CONFIG_DESC_SIZE
static uint_8_ptr  other_speed_config;
static uint_8  other_speed_config_data[CONFIG_DESC_SIZE] =
{
   /*Config Descriptor */
   0x09,
   0x07,
   USB_uint_16_low(CONFIG_DESC_SIZE),
   USB_uint_16_high(CONFIG_DESC_SIZE),
   0x01,
   0x01,
   0x00,
   0xE0, /* 0x80, */
   0x0,
   /* Interface Descriptor */	
   0x09,
   0x04,
   0x00,
   0x00,
   0x01,
   0x03,
   0x01,
   0x02,
   0x00,

   /* HID descriptor */
   0x09,
   0x21,
   USB_uint_16_low(0x0100),
   USB_uint_16_high(0x0100),
   0x00,
   0x01,
   0x22,
   USB_uint_16_low(0x0034),
   USB_uint_16_high(0x0034),
 
   /*Endpoint descriptor */
   0x07,
   0x05,
   (0x80+INTERRUPT_EP),
   0x03,
   USB_uint_16_low(INTERRUPT_MAX_PACKET_SIZE), 
   USB_uint_16_high(INTERRUPT_MAX_PACKET_SIZE),
   FRAME_INTERVAL

};
/************************************************************************

HID Class Report Descriptor :

Item						Value(Hex)
------------------------------------------------------------------------------------------------------------
Usage Page (Generic Desktop Control)			05 01 
Usage (Mouse)					09 02 
Collection (Application)				A1 01 
  Usage (Pointer)					09 01 
  Collection (Physical)				A1 00 
    Usage Page (Button)				05 09 
    Usage Minimum (1)				19 01 
    Usage Maximum (3)				29 03 
    Logical Minimum (0)				15 00 
    Logical Maximum (1)				25 01 
    Report Count (3)					95 03 
    Report Size (1)					75 01 
    Input (Data, Variable, Absolute)			81 02 
    Report Count (1)					95 01 
    Report Size (5)					75 05 
    Input (Constant)					81 01 
    Usage Page (Generic Desktop Control)		05 01 
    Usage (X)					09 30 
    Usage (Y)					09 31 
    Usage (Wheel)					09 38 
    Logical Minimum (-127)				15 81 
    Logical Maximum (127)				25 7F 
    Report Size (8)					75 08 
    Report Count (3)					95 03 
    Input (Data, Variable, Relative)			81 06 
  End Collection					C0 
End Collection					C0 


************************************************************************/

#define REPORT_DESC_SIZE            (52)
static uint_8_ptr ReportDesc;

static uint_8 ReportDescData[REPORT_DESC_SIZE] =
{
   0x05,
   0x01,
   0x09,
   0x02,
   0xA1,
   0x01,
   0x09,
   0x01,
   
   0xA1,
   0x00,
   0x05,
   0x09,
   0x19,
   0x01,
   0x29,
   0x03,
   
   0x15,
   0x00,
   0x25,
   0x01,
   0x95,
   0x03,
   0x75,
   0x01,
   
   0x81,
   0x02,
   0x95,
   0x01,
   0x75,
   0x05,
   0x81,
   0x01,
   
   0x05,
   0x01,
   0x09,
   0x30,
   0x09,
   0x31,
   0x09,
   0x38,
   
   0x15,
   0x81,
   0x25,
   0x7F,
   0x75,
   0x08,
   0x95,
   0x03,
   
   0x81,
   0x06,
   0xC0,
   0xC0   
};

/**************************************************************
This report descriptor can be used to report the set_report
and get_report capability to host. When this is used, modify
the config descriptor to reflect the size of report descriptor.
The following lines should be changed,

USB_uint_16_low(0x0038),  //   Changed from USB_uint_16_low(0x0034),
USB_uint_16_high(0x0038), //   Changed from USB_uint_16_high(0x0034),
  



uint_8  ReportDesc[56] = {
    0x06, 0x00, 0xff,              		// USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                    		// USAGE (Vendor Usage 1)
    0xa1, 0x01,                    		// COLLECTION (Application)

    0x09, 0x02,                    		//   USAGE (Vendor Usage 2)
    0x15, 0x80,                    		//   LOGICAL_MINIMUM (-128)																			
    0x25, 0x7f,                    		//   LOGICAL_MAXIMUM (127) 																			
    0x95, 0x01,                    		//   REPORT_COUNT (1)      																			
    0x75, 0x08,                    		//   REPORT_SIZE (8)       																			
    0xb1, 0x02,                    		//   FEATURE (Data,Var,Abs)																			

    0x09, 0x03,                    		//   USAGE (Vendor Usage 3)																			
    0x15, 0x80,                    		//   LOGICAL_MINIMUM (-128)																			
    0x25, 0x7f,                    		//   LOGICAL_MAXIMUM (127) 																			
    0x95, 0x01,                    		//   REPORT_COUNT (1)      																			
    0x75, 0x08,                    		//   REPORT_SIZE (8)
    0xb1, 0x02,                    		//   FEATURE (Data,Var,Abs)																			

    0x09, 0x04,                    		//   USAGE (Vendor Usage 4)																			
    0x15, 0x80,                    		//   LOGICAL_MINIMUM (-128)																			
    0x25, 0x7f,                    		//   LOGICAL_MAXIMUM (127) 																			
    0x95, 0x01,                    		//   REPORT_COUNT (1)      																			
    0x75, 0x08,                    		//   REPORT_SIZE (8)
    0xb1, 0x02,                    		//   FEATURE (Data,Var,Abs)																			

    0x09, 0x05,                    		//   USAGE (Vendor Usage 5)																			
    0x15, 0x80,                    		//   LOGICAL_MINIMUM (-128)																			
    0x25, 0x7f,                    		//   LOGICAL_MAXIMUM (127) 																			
    0x95, 0x01,                    		//   REPORT_COUNT (1)      																			
    0x75, 0x08,                    		//   REPORT_SIZE (8)
    0xb1, 0x02,                    		//   FEATURE (Data,Var,Abs)																			
        
    0xc0                           		// END_COLLECTION          																			
};
***************************************************************/

/**********************************************************************
Mouse data (this structure is used to send mouse movement information)
**********************************************************************/
typedef struct   _MOUSE_DATA {
   char  a;
   char  b;
   char  c;
   char  d;

} MOUSE_DATA_STRUCT;

static MOUSE_DATA_STRUCT mouse_data = {0,0,0,0};

static uint_8 USB_IF_ALT[4] = { 0, 0, 0, 0};

/* number of strings in the table not including 0 or n. */
static const uint_8 USB_STR_NUM  = 6;

/*
** if the number of strings changes, look for USB_STR_0 everywhere and make 
** the obvious changes.  It should be found in 3 places.
*/

static const uint_16 USB_STR_0[ 2] = {0x0300 + sizeof(USB_STR_0),0x0409};
static const uint_16 USB_STR_1[26] = {0x0300 + sizeof(USB_STR_1),
      'M','a','r','v','e','l','l',' ','S','e','m','i','c','o','n','d','u','c','t','o','r',' ','L','t','d'};       
static const uint_16 USB_STR_2[28] = {0x0300 + sizeof(USB_STR_2),
      'M','A','R','V','E','L','L',' ','U','S','B',' ','h','i','d','m','o','u','s','e',' ',\
      'D','e','v','i','c','e'};
static const uint_16 USB_STR_3[ 5] = {0x0300 + sizeof(USB_STR_3),
      'B','E','T','A'};
static const uint_16 USB_STR_4[ 4] = {0x0300 + sizeof(USB_STR_4),
      '#','0','2'};
static const uint_16 USB_STR_5[ 4] = {0x0300 + sizeof(USB_STR_5),
      '_','A','1'};
static const uint_16 USB_STR_6[15] = {0x0300 + sizeof(USB_STR_6),
      'Y','o','u','r',' ','n','a','m','e',' ','h','e','r','e'};
static const uint_16 USB_STR_n[17] = {0x0300 + sizeof(USB_STR_n),
      'B','A','D',' ','S','T','R','I','N','G',' ','I','n','d','e','x'};

#define USB_STRING_ARRAY_SIZE  8
static const uint_8_ptr USB_STRING_DESC[USB_STRING_ARRAY_SIZE] =
{
   (uint_8_ptr)USB_STR_0,
   (uint_8_ptr)USB_STR_1,
   (uint_8_ptr)USB_STR_2,
   (uint_8_ptr)USB_STR_3,
   (uint_8_ptr)USB_STR_4,
   (uint_8_ptr)USB_STR_5,
   (uint_8_ptr)USB_STR_6,
   (uint_8_ptr)USB_STR_n
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
*     See section 9.4.3 (page 189) of the USB 1.1 Specification.
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
   if (setup) {
      /* Load the appropriate string depending on the descriptor requested.*/
      switch (setup_ptr->VALUE & 0xFF00) {

         case 0x0100:
            _usb_device_send_data(handle, 0, DevDesc,
               MIN(setup_ptr->LENGTH, DEVICE_DESCRIPTOR_SIZE));

            break;

         case 0x0200:
             *(ConfigDesc + 33) = frame_interval;
            _usb_device_send_data(handle, 0, ConfigDesc,
               MIN(setup_ptr->LENGTH, CONFIG_DESC_SIZE));
                           
            break;

         case 0x2200:
         _usb_device_send_data(handle, 0, ReportDesc,
            MIN(setup_ptr->LENGTH, REPORT_DESC_SIZE));

         /*send some data for the mouse in the interrupt pipe queue */
         _usb_device_send_data(handle, INTERRUPT_EP, (uint_8_ptr)((pointer)&mouse_data),
                                sizeof(MOUSE_DATA_STRUCT));
               
            break;      
   
         case 0x0300:
            if ((setup_ptr->VALUE & 0x00FF) > USB_STR_NUM) {
               _usb_device_send_data(handle, 0, USB_STRING_DESC[USB_STR_NUM+1],
                  MIN(setup_ptr->LENGTH, USB_STRING_DESC[USB_STR_NUM+1][0]));
            } else {
               _usb_device_send_data(handle, 0,
                  USB_STRING_DESC[setup_ptr->VALUE & 0x00FF],
                  MIN(setup_ptr->LENGTH, USB_STRING_DESC[setup_ptr->VALUE & 0x00FF][0]));
            } /* Endif */      
            break;

         case 0x600:
            _usb_device_send_data(handle, 0, (uint_8_ptr)DevQualifierDesc, 
               MIN(setup_ptr->LENGTH, DEVICE_QUALIFIER_DESCRIPTOR_SIZE));
            break;
            
         case 0x700:      
            *(other_speed_config + 33) = frame_interval;

            _usb_device_send_data(handle, 0, (uint_8_ptr)other_speed_config, 
               MIN(setup_ptr->LENGTH, OTHER_SPEED_CONFIG_DESC_SIZE));
            break;
        
         default:
            USB_printf("usbMouse_%d, %s: Unexpected VALUE=0x%04x\n", 
                _usb_device_get_dev_num(handle), __FUNCTION__, setup_ptr->VALUE);                
            _usb_device_stall_endpoint(handle, 0, 0);
            return;
      } /* Endswitch */
      /* status phase */
      _usb_device_recv_data(handle, 0, 0, 0);
   } /* Endif */
   return;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : ch9SetDescription
* Returned Value : None
* Comments       :
*     Chapter 9 SetDescription command
*     See section 9.4.8 (page 193) of the USB 1.1 Specification.
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
    USB_printf("usbMouse_%d, %s: setup=%d\n", 
            _usb_device_get_dev_num(handle), __FUNCTION__, (int)setup);
   _usb_device_stall_endpoint(handle, 0, 0);
   return;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : ch9GetConfig
* Returned Value : None
* Comments       :
*     Chapter 9 GetConfig command
*     See section 9.4.2 (page 189) of the USB 1.1 Specification.
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
   uint_16 current_config;
   /* Return the currently selected configuration */
   if (setup){ 
      _usb_device_get_status(handle, ARC_USB_STATUS_CURRENT_CONFIG,
         &current_config);
      data_to_send = (uint_8)current_config;      
      _usb_device_send_data(handle, 0, (pointer) &data_to_send, sizeof(data_to_send));
      /* status phase */
      _usb_device_recv_data(handle, 0, 0, 0);
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
    uint_16 usb_state;
   
    if (setup) 
    {
        if ((setup_ptr->VALUE & 0x00FF) > 1) 
        {
            /* generate stall */
            USB_printf("usbMouse_%d, %s: Wrong VALUE=0x%04x\n", 
                    _usb_device_get_dev_num(handle), __FUNCTION__, setup_ptr->VALUE);
            _usb_device_stall_endpoint(handle, 0, 0);
            return;
        } /* Endif */

        /* 0 indicates return to unconfigured state */
        if ((setup_ptr->VALUE & 0x00FF) == 0) 
        {
            _usb_device_get_status(handle, ARC_USB_STATUS_DEVICE_STATE, &usb_state);
            if ((usb_state == ARC_USB_STATE_CONFIG) || 
                (usb_state == ARC_USB_STATE_ADDRESS)) 
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
                USB_printf("usbMouse_%d, %s: Wrong usb_state=%d\n", 
                        _usb_device_get_dev_num(handle), __FUNCTION__, usb_state);

                _usb_device_stall_endpoint(handle, 0, 0);
            } /* Endif */
            return;
        } /* Endif */

        /*
        ** If the configuration value (setup_ptr->VALUE & 0x00FF) differs
        ** from the current configuration value, then endpoints must be
        ** reconfigured to match the new device configuration
        */
        _usb_device_get_status(handle, ARC_USB_STATUS_CURRENT_CONFIG, &usb_state);
        ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_SETUP, 
                        "usbMouse: Set configuration: old=%d, new=%d\n", 
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
        /* Init Interrupt endpoint */
        _usb_device_init_endpoint(handle,INTERRUPT_EP, INTERRUPT_MAX_PACKET_SIZE, 
                                   ARC_USB_SEND, ARC_USB_INTERRUPT_ENDPOINT, 
                                   ARC_USB_DEVICE_DONT_ZERO_TERMINATE);

        TEST_ENABLED = TRUE;

        _usb_device_set_status(handle, ARC_USB_STATUS_DEVICE_STATE,
                             ARC_USB_STATE_CONFIG);
        /* status phase */
        _usb_device_send_data(handle, 0, 0, 0);
    } /* Endif */
    return;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : ch9GetInterface
* Returned Value : None
* Comments       :
*     Chapter 9 GetInterface command
*     See section 9.4.4 (page 190) of the USB 1.1 Specification.
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
   
    _usb_device_get_status(handle, ARC_USB_STATUS_DEVICE_STATE, &usb_state);
    if (usb_state != ARC_USB_STATE_CONFIG) 
    {
        USB_printf("usbMouse_%d, %s: Wrong usb_state=%d\n", 
                    _usb_device_get_dev_num(handle), __FUNCTION__, usb_state);
        _usb_device_stall_endpoint(handle, 0, 0);
        return;
    } /* Endif */

    if (setup) 
    {
        _usb_device_send_data(handle, 0, &USB_IF_ALT[setup_ptr->INDEX & 0x00FF],
                                MIN(setup_ptr->LENGTH, sizeof(uint_8)));
        /* status phase */      
        _usb_device_recv_data(handle, 0, 0, 0);
    } /* Endif */
    return;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : ch9SetInterface
* Returned Value : None
* Comments       :
*     Chapter 9 SetInterface command
*     See section 9.4.10 (page 195) of the USB 1.1 Specification.
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
    if (setup) 
    {
        if (setup_ptr->REQUESTTYPE != 0x01) 
        {
            USB_printf("usbDisk_%d, %s: Wrong REQUESTTYPE=0x%02x\n", 
                        _usb_device_get_dev_num(handle), __FUNCTION__, 
                        setup_ptr->REQUESTTYPE);

            _usb_device_stall_endpoint(handle, 0, 0);
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
*     See section 9.4.11 (page 195) of the USB 1.1 Specification.
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
   
    if (setup) 
    {
        if (setup_ptr->REQUESTTYPE != 0x02) 
        {
            USB_printf("usbMouse_%d, %s: Wrong REQUESTTYPE=0x%02x\n", 
                        _usb_device_get_dev_num(handle), __FUNCTION__, 
                        setup_ptr->REQUESTTYPE);
            _usb_device_stall_endpoint(handle, 0, 0);
            return;
        } /* Endif */

        if ((setup_ptr->INDEX & 0x00FF) >=
                ConfigDesc[CONFIG_DESC_NUM_INTERFACES])
        {
            USB_printf("usbMouse_%d, %s: Wrong INDEX=0x%04x\n", 
                        _usb_device_get_dev_num(handle), __FUNCTION__, setup_ptr->INDEX);
            _usb_device_stall_endpoint(handle, 0, 0);
            return;
        } /* Endif */

        _usb_device_get_status(handle, ARC_USB_STATUS_SOF_COUNT, &sof_count);

        sof_count = USB_16BIT_LE(sof_count);
        _usb_device_send_data(handle, 0, (uint_8_ptr)&sof_count,
                        MIN(setup_ptr->LENGTH, sizeof(sof_count)));
        /* status phase */      
        _usb_device_recv_data(handle, 0, 0, 0);
    } /* Endif */
    return;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : get_report
* Returned Value : 
* Comments       :
*     Chapter 9 Class specific request
*     See section 9.4.11 (page 195) of the USB 1.1 Specification.
* 
*END*--------------------------------------------------------------------*/

void get_report
	(
	/* USB handle */
	_usb_device_handle handle,
	
	/* Is it a Setup phase? */
	boolean     setup,
   
   /* [IN] Direction of the transfer. (1 for USB IN token)*/
   uint_8      direction,
	
	/* The setup packet pointer */
	SETUP_STRUCT_PTR setup_ptr

	)
{ 
    int i;
      
   for(i=0;i<10;i++)	
	{
		hid_test_rep_data[i] = (uint_8) i;
	}

	if (setup)
	{
		_usb_device_send_data(handle, 0, (uint_8_ptr)hid_test_rep_data, MIN(setup_ptr->LENGTH,4));
	}
         
   _usb_device_recv_data(handle, 0, 0, 0);
         

	return;
} 

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : set_report
* Returned Value : 
* Comments       :
*     Chapter 9 Class specific request
*     See section 9.4.11 (page 195) of the USB 1.1 Specification.
* 
*END*--------------------------------------------------------------------*/

void set_report
	(
	/* USB handle */
	_usb_device_handle handle,
	
	/* Is it a Setup phase? */
	boolean setup,
   
   /* [IN] Direction of the transfer. (1 for USB IN token)*/
   uint_8               direction,

	
	/* The setup packet pointer */
	SETUP_STRUCT_PTR setup_ptr
	)
{ 
	if (setup)      /*on a SetUP packet*/
   {
	  _usb_device_recv_data(handle, 0, (uint_8_ptr)hid_test_rep_data, MIN(setup_ptr->LENGTH,4));
 
	}
   else if(direction == ARC_USB_RECV)   /*on a OUT packet*/
   {
	_usb_device_recv_data(handle, 0, (uint_8_ptr)hid_test_rep_data, MIN(setup_ptr->LENGTH,4));
     _usb_device_send_data(handle, 0, 0, 0);
   }

	return;
} 

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : set_idle
* Returned Value : 
* Comments       :
*     Chapter 9 Class specific request
*     See section 9.4.11 (page 195) of the USB 1.1 Specification.
* 
*END*--------------------------------------------------------------------*/

void set_idle
	(
	/* USB handle */
	_usb_device_handle handle,
	
	/* Is it a Setup phase? */
	boolean setup,
   
   /* [IN] Direction of the transfer. (1 for USB IN token)*/
   uint_8               direction,

	
	/* The setup packet pointer */
	SETUP_STRUCT_PTR setup_ptr
	)
{ 
   /* SET_IDLE is a No data phase transaction from HID class. All it needs
   is a terminating IN token */
	if (setup)      /*on a SetUP packet*/
   {
	  _usb_device_send_data(handle, 0, 0, 0);  
	}
	return;
} 

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
      /* USB handle */
      _usb_device_handle handle,
      
      /* Is it a Setup phase? */
      boolean setup,
      
      /* [IN] Direction of the transfer. (1 for USB IN token)*/
      uint_8               direction,

      /* The setup packet pointer */
      SETUP_STRUCT_PTR setup_ptr
   )
{ /* Body */
   
   switch (setup_ptr->REQUEST) 
   {
   
      case 0x01:
         get_report(handle, setup, direction, setup_ptr);
         break;
      
      case 0x09:
         set_report(handle, setup, direction, setup_ptr);
         break;

      case 0x0A:
         set_idle(handle, setup, direction, setup_ptr);
         break;
     
      default:
        USB_printf("usbMouse_%d, %s: Wrong REQUEST=0x%02x\n", 
              _usb_device_get_dev_num(handle), __FUNCTION__, setup_ptr->REQUEST);

         _usb_device_stall_endpoint(handle, 0, 0);
         break;

   } /* EndSwitch */

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
   boolean  class_request = FALSE;
   
   if (setup) 
   {
      _usb_device_read_setup_data(handle, 0, (uint_8_ptr)&local_setup_packet);
      local_setup_packet.VALUE = USB_16BIT_LE(local_setup_packet.VALUE);
      local_setup_packet.INDEX = USB_16BIT_LE(local_setup_packet.INDEX);
      local_setup_packet.LENGTH = USB_16BIT_LE(local_setup_packet.LENGTH);
   } 
   else if (class_request) {
      class_request = FALSE;
      /* Finish your class or vendor request here */
      
      return;
   } /* Endif */
   
   switch (local_setup_packet.REQUESTTYPE & 0x60) {

      case 0x00:
         switch (local_setup_packet.REQUEST) {

            case 0x0:
               mvUsbCh9GetStatus(handle, setup, &local_setup_packet);
               break;

            case 0x1:
               mvUsbCh9ClearFeature(handle, setup, &local_setup_packet);
               break;

            case 0x3:
               mvUsbCh9SetFeature(handle, setup, &local_setup_packet);
               break;

            case 0x5:
               mvUsbCh9SetAddress(handle, setup, &local_setup_packet);
               break;

            case 0x6:
               ch9GetDescription(handle, setup, &local_setup_packet);
               break;

            case 0x7:
               ch9SetDescription(handle, setup, &local_setup_packet);
               break;

            case 0x8:
               ch9GetConfig(handle, setup, &local_setup_packet);
               break;

            case 0x9:
               ch9SetConfig(handle, setup, &local_setup_packet);
               break;

            case 0xa:
               ch9GetInterface(handle, setup, &local_setup_packet);
               break;

            case 0xb:
               ch9SetInterface(handle, setup, &local_setup_packet);
               break;

            case 0xc:
               ch9SynchFrame(handle, setup, &local_setup_packet);
               break;

            default:
                USB_printf("usbMouse_%d, %s: Wrong REQUEST = 0x%02x\n", 
                        _usb_device_get_dev_num(handle), __FUNCTION__, local_setup_packet.REQUEST);
               _usb_device_stall_endpoint(handle, 0, 0);
               break;

         } /* Endswitch */
         
         break;

      case 0x20:
         ch9Class(handle, setup, direction, &local_setup_packet);

         break;

      case 0x40:
         /* vendor specific request */
         break;
      
      default:
         USB_printf("usbMouse_%d, %s: Unexpected REQUESTTYPE = 0x%x\n", 
                _usb_device_get_dev_num(handle), __FUNCTION__, 
                local_setup_packet.REQUESTTYPE);

         _usb_device_stall_endpoint(handle, 0, 0);
         break;
         
   } /* Endswitch */
   
   return;
} /* Endbody */

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

/********************************************************************
   The following code will move the mouse right and left on the screen.
   Comment this out if this behaviour is not desired.
********************************************************************/

    static int       x = 0;  
    static boolean   right = FALSE;
    static int       wait = 0;

    mouseCntr++;
    if(wait == 0)
    {
        if (right == FALSE)  
        {
            mouse_data.b = 1; 
            x++;
            right = (x > 200) ? TRUE : FALSE;     
        }
   
        if (right == TRUE)  
        {
            mouse_data.b = -1; 
            x--;
            right = (x < 0) ? FALSE : TRUE;     
        }
        wait = mouseDelay;
    }
    else
    {
        wait--;
        mouse_data.b = 0; 
    }
   
   _usb_device_send_data(handle, INTERRUPT_EP, (uint_8_ptr)((pointer)&mouse_data),
                            sizeof(MOUSE_DATA_STRUCT));
 
   return;
} /* Endbody */

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

   /*on a reset always cancel all transfers all EP 0 */
   _usb_device_cancel_transfer(handle, 0, ARC_USB_RECV);
   _usb_device_cancel_transfer(handle, 0, ARC_USB_SEND);

   _usb_device_start(handle);

   /* Initialize the endpoint 0 in both directions */
   _usb_device_init_endpoint(handle, 0, DevDesc[DEV_DESC_MAX_PACKET_SIZE], 0,
      ARC_USB_CONTROL_ENDPOINT, 0);
   _usb_device_init_endpoint(handle, 0, DevDesc[DEV_DESC_MAX_PACKET_SIZE], 1,
      ARC_USB_CONTROL_ENDPOINT, 0);

   if (TEST_ENABLED) 
   {
      _usb_device_cancel_transfer(handle, INTERRUPT_EP, ARC_USB_SEND);
   } /* Endif */
   
   TEST_ENABLED = FALSE;
   mouseCntr = 0;
         
   return;
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : service_suspend
* Returned Value : None
* Comments       :
*     Called when host suspend the USB port. Do remote wake up if desired.
* 
*END*--------------------------------------------------------------------*/
static void service_suspend
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
   uint_16      usb_status;
   int          lockKey;            
   
   _usb_device_get_status(handle, ARC_USB_STATUS_DEVICE, &usb_status);
   if (usb_status & ARC_USB_REMOTE_WAKEUP) 
   { 
       lockKey = USB_lock();

       USB_printf("Mouse Suspended: type=%d, usbStatus=0x%x\n", type, usb_status);
       USB_SUSPENDED = TRUE;

       USB_unlock(lockKey);
   } 
          
   return;
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : usbMouseLoad
* Returned Value : None
* Comments       :
*     First function called.  Initialises the USB and registers Chapter 9
*     callback functions.
* 
*END*--------------------------------------------------------------------*/
_usb_device_handle  usbMouseLoad(int devNo)
{ /* Body */
   _usb_device_handle   handle;
   uint_8               error;
   uint_32              send_data_buffer_size=0;
   uint_8_ptr           temp;
   int                  lockKey, i, j;
   static boolean       isFirst = TRUE;

    if(isFirst)
    {
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

   lockKey = USB_lock();
   
    /* Initialize the USB interface */
    error = _usb_device_init(devNo, &handle);   
    if (error != USB_OK) 
    {
        USB_printf("\nUSB Initialization failed. Error: %x\n", error);
        return NULL;
    } /* Endif */
   
    /* Self Power, Remote wakeup disable */
    _usb_device_set_status(handle, ARC_USB_STATUS_DEVICE, (1 << DEVICE_SELF_POWERED));    

    error = _usb_device_register_service(handle, ARC_USB_SERVICE_EP0, service_ep0);
    if (error != USB_OK) 
    {
        USB_printf("\nUSB EP0 Service Registration failed. Error: %x\n", error);
        return NULL;
    } /* Endif */
   
    error = _usb_device_register_service(handle, ARC_USB_SERVICE_BUS_RESET, reset_ep0);
    if (error != USB_OK) 
    {
        USB_printf("\nUSB BUS_RESET Service Registration failed. Error: %x\n", error);
        return NULL;
    } /* Endif */
   
    error = _usb_device_register_service(handle, INTERRUPT_EP, service_ep1);   
    if (error != USB_OK) 
    {
        USB_printf("\nUSB EP1 Service Registration failed. Error: %x\n", error);
        return NULL;
    } /* Endif */

    error = _usb_device_register_service(handle, ARC_USB_SERVICE_SUSPEND, service_suspend);
    if (error != USB_OK) 
    {
        USB_printf("\nUSB SUSPEND Service Registration failed. Error: %x\n", error);
        return NULL;
    } /* Endif */

    error = _usb_device_register_service(handle, ARC_USB_SERVICE_SLEEP, service_suspend);
    if (error != USB_OK) 
    {
        USB_printf("\nUSB SUSPEND Service Registration failed. Error: %x\n", error);
        return NULL;
    } /* Endif */

    /***********************************************************************
    Allocate memory to receive data at endpoint 0. Ensure that buffers are
    cache aligned.
    ***********************************************************************/
    hid_test_rep_data_unaligned   = (uint_8_ptr) USB_memalloc((EP1_RECV_BUFFER_SIZE + PSP_CACHE_LINE_SIZE));
    if(hid_test_rep_data_unaligned == NULL)
    {
        USB_printf("mouseLoad: Buffer allocation of %d bytes is failed\n", 
                    (unsigned)EP1_RECV_BUFFER_SIZE + PSP_CACHE_LINE_SIZE);
        return NULL;
    }

    hid_test_rep_data = (uint_8_ptr) USB_CACHE_ALIGN((uint_32) hid_test_rep_data_unaligned);
    
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
                             (REPORT_DESC_SIZE + PSP_CACHE_LINE_SIZE) +
                             (DEVICE_QUALIFIER_DESCRIPTOR_SIZE + PSP_CACHE_LINE_SIZE) +
                             (OTHER_SPEED_CONFIG_DESC_SIZE + PSP_CACHE_LINE_SIZE);
                               
    Send_Buffer_Unaligned   = (uint_8_ptr) USB_memalloc(send_data_buffer_size);
    if (Send_Buffer_Unaligned == NULL) 
    {
        USB_printf("\nMouse: %d bytes Buffer allocation failed\n", send_data_buffer_size);
        return NULL;
    }
   
    Send_Buffer_aligned = (uint_8_ptr) USB_CACHE_ALIGN((uint_32) Send_Buffer_Unaligned);
    /* keep a temporary copy of the aligned address */
    temp = Send_Buffer_aligned;
   

    /**************************************************************************
    Assign pointers to different descriptors from it and copy descriptors inside.
    ***************************************************************************/
    DevDesc =  (uint_8_ptr) Send_Buffer_aligned;
    USB_memcopy(DevDescData, DevDesc, DEVICE_DESCRIPTOR_SIZE);
    Send_Buffer_aligned += ((DEVICE_DESCRIPTOR_SIZE/PSP_CACHE_LINE_SIZE) + 1)* PSP_CACHE_LINE_SIZE; 
   
    ConfigDesc =  (uint_8_ptr) Send_Buffer_aligned;
    USB_memcopy(ConfigDescData, ConfigDesc, CONFIG_DESC_SIZE);

    Send_Buffer_aligned += ((CONFIG_DESC_SIZE/PSP_CACHE_LINE_SIZE) + 1)* PSP_CACHE_LINE_SIZE; 

    ReportDesc =  (uint_8_ptr) Send_Buffer_aligned;
    USB_memcopy(ReportDescData, ReportDesc, REPORT_DESC_SIZE);
    Send_Buffer_aligned += ((REPORT_DESC_SIZE/PSP_CACHE_LINE_SIZE) + 1)* PSP_CACHE_LINE_SIZE; 

   
    DevQualifierDesc =  (uint_8_ptr) Send_Buffer_aligned;
    USB_memcopy(DevQualifierDescData, DevQualifierDesc, DEVICE_QUALIFIER_DESCRIPTOR_SIZE);
    Send_Buffer_aligned += ((DEVICE_QUALIFIER_DESCRIPTOR_SIZE/PSP_CACHE_LINE_SIZE) + \
                           1)* PSP_CACHE_LINE_SIZE;

    other_speed_config =  (uint_8_ptr) Send_Buffer_aligned;
    USB_memcopy(other_speed_config_data, other_speed_config, OTHER_SPEED_CONFIG_DESC_SIZE);
    Send_Buffer_aligned += ((OTHER_SPEED_CONFIG_DESC_SIZE/PSP_CACHE_LINE_SIZE) + 1)* PSP_CACHE_LINE_SIZE; 
                           
    /**************************************************************************
    Flush the cache to ensure main memory is updated.
    ***************************************************************************/
    USB_dcache_flush(temp,send_data_buffer_size);

     /* Initialize the endpoint 0 in both directions */
    _usb_device_init_endpoint(handle, 0, DevDesc[DEV_DESC_MAX_PACKET_SIZE], 0,
                            ARC_USB_CONTROL_ENDPOINT, 0);
    _usb_device_init_endpoint(handle, 0, DevDesc[DEV_DESC_MAX_PACKET_SIZE], 1,
                            ARC_USB_CONTROL_ENDPOINT, 0);

    USB_unlock(lockKey);

    USB_printf("USB Mouse example is READY\n");
   
    return handle; 
} /* Endbody */

void    usbMouseUnload(_usb_device_handle handle)
{
    int     lockKey;

    if(handle == NULL)
        return;

    /*lock interrupts */
    lockKey = USB_lock();
    
    /* ensure all transfers are cancelled */
    _usb_device_cancel_transfer(handle, INTERRUPT_EP,  ARC_USB_SEND);

    /* Stop Endpoints */
    _usb_device_deinit_endpoint(handle, INTERRUPT_EP, ARC_USB_SEND);

    _usb_device_deinit_endpoint(handle, 0, ARC_USB_RECV);
    _usb_device_deinit_endpoint(handle, 0, ARC_USB_SEND);

    _usb_device_stop(handle);

    /* Deregister all services */
    _usb_device_unregister_service(handle, ARC_USB_SERVICE_EP0);   
    _usb_device_unregister_service(handle, ARC_USB_SERVICE_BUS_RESET);   
    _usb_device_unregister_service(handle, ARC_USB_SERVICE_SUSPEND);
    _usb_device_unregister_service(handle, ARC_USB_SERVICE_SLEEP);
    _usb_device_unregister_service(handle, INTERRUPT_EP);   

    _usb_device_shutdown(handle);

    /* Free memory allocated for Disk device */
    if(Send_Buffer_Unaligned != NULL)
    {
        USB_memfree(Send_Buffer_Unaligned);
        Send_Buffer_Unaligned = NULL;
    }

    if(hid_test_rep_data_unaligned != NULL)
    {
        USB_memfree(hid_test_rep_data_unaligned);
        hid_test_rep_data_unaligned  = NULL;
    }

    /* Clear gloabal variables */
    TEST_ENABLED = FALSE;
    USB_SUSPENDED = FALSE;    
    
    USB_unlock(lockKey);
}

void    usbMousePeriodicResume(_usb_device_handle handle)
{
    if (USB_SUSPENDED) 
    {
       /*
        * Send RESUME signal whenever host suspends the USB port. In real case, we should
        *  send RESUME signal only when a mouse button being clicked.
        */
        USB_printf("Mouse Resumed\n");

        _usb_device_assert_resume(handle);
        USB_SUSPENDED = FALSE;     
    } /* Endbody */
}
/* EOF */
