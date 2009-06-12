#ifndef CYGONCE_USB_H
# define CYGONCE_USB_H
//==========================================================================
//
//      include/usb.h
//
//      Data common to USB host and USB slave
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    bartv
// Contributors: bartv
// Date:         2000-10-04
//
//####DESCRIPTIONEND####
//==========================================================================

#ifdef __cplusplus
extern "C" {
#endif

// USB device requests, the setup packet.
//
// The structure is defined entirely in terms of bytes, eliminating
// any confusion about who is supposed to swap what when. This avoids
// endianness-related portability problems, and eliminates any need
// to worry about alignment. Also for some requests the value field
// is split into separate bytes anyway.
typedef struct usb_devreq {
    unsigned char       type;
    unsigned char       request;
    unsigned char       value_lo;
    unsigned char       value_hi;
    unsigned char       index_lo;
    unsigned char       index_hi;
    unsigned char       length_lo;
    unsigned char       length_hi;
} usb_devreq __attribute__((packed));

// Encoding of the request_type    
#define USB_DEVREQ_DIRECTION_OUT        0
#define USB_DEVREQ_DIRECTION_IN         (1 << 7)
#define USB_DEVREQ_DIRECTION_MASK       (1 << 7)

#define USB_DEVREQ_TYPE_STANDARD        0
#define USB_DEVREQ_TYPE_CLASS           (0x1 << 5)
#define USB_DEVREQ_TYPE_VENDOR          (0x2 << 5)
#define USB_DEVREQ_TYPE_RESERVED        (0x3 << 5)
#define USB_DEVREQ_TYPE_MASK            (0x3 << 5)

#define USB_DEVREQ_RECIPIENT_DEVICE     0x00
#define USB_DEVREQ_RECIPIENT_INTERFACE  0x01
#define USB_DEVREQ_RECIPIENT_ENDPOINT   0x02
#define USB_DEVREQ_RECIPIENT_OTHER      0x03
#define USB_DEVREQ_RECIPIENT_MASK       0x1F

// The standard request codes.
#define USB_DEVREQ_GET_STATUS            0
#define USB_DEVREQ_CLEAR_FEATURE         1
#define USB_DEVREQ_SET_FEATURE           3
#define USB_DEVREQ_SET_ADDRESS           5
#define USB_DEVREQ_GET_DESCRIPTOR        6
#define USB_DEVREQ_SET_DESCRIPTOR        7
#define USB_DEVREQ_GET_CONFIGURATION     8
#define USB_DEVREQ_SET_CONFIGURATION     9
#define USB_DEVREQ_GET_INTERFACE        10
#define USB_DEVREQ_SET_INTERFACE        11
#define USB_DEVREQ_SYNCH_FRAME          12

// Descriptor types. These are placed in value_hi for the
// GET_DESCRIPTOR and SET_DESCRIPTOR requests, with an index
// in value_lo. They also go into the type fields of the
// various descriptor structures.
#define USB_DEVREQ_DESCRIPTOR_TYPE_DEVICE               1
#define USB_DEVREQ_DESCRIPTOR_TYPE_CONFIGURATION        2
#define USB_DEVREQ_DESCRIPTOR_TYPE_STRING               3
#define USB_DEVREQ_DESCRIPTOR_TYPE_INTERFACE            4
#define USB_DEVREQ_DESCRIPTOR_TYPE_ENDPOINT             5

// Feature selectors. These go into value_lo for the CLEAR_FEATURE and
// SET_FEATURE requests, and in the first response byte for
// GET_STATUS.
#define USB_DEVREQ_FEATURE_DEVICE_REMOTE_WAKEUP         1
#define USB_DEVREQ_FEATURE_ENDPOINT_HALT                0

// Index decoding. When the CLEAR_FEATURE, SET_FEATURE and GET_STATUS
// requests is applied to an endpoint (as per the recipient field in
// the type field) index_lo identifies the endpoint.
#define USB_DEVREQ_INDEX_DIRECTION_OUT                  0
#define USB_DEVREQ_INDEX_DIRECTION_IN                   (1 << 7)
#define USB_DEVREQ_INDEX_DIRECTION_MASK                 (1 << 7)
#define USB_DEVREQ_INDEX_ENDPOINT_MASK                  0x0F

// Descriptors for the GET_DESCRIPTOR and SET_DESCRIPTOR requests.
typedef struct usb_device_descriptor {
    unsigned char       length;                 // USB_DEVICE_DESCRIPTOR_LENGTH == 18
    unsigned char       type;                   // USB_DEVREQ_DESCRIPTOR_TYPE
    unsigned char       usb_spec_lo;
    unsigned char       usb_spec_hi;
    unsigned char       device_class;
    unsigned char       device_subclass;
    unsigned char       device_protocol;
    unsigned char       max_packet_size;
    unsigned char       vendor_lo;
    unsigned char       vendor_hi;
    unsigned char       product_lo;
    unsigned char       product_hi;
    unsigned char       device_lo;
    unsigned char       device_hi;
    unsigned char       manufacturer_str;
    unsigned char       product_str;
    unsigned char       serial_number_str;
    unsigned char       number_configurations;
} usb_device_descriptor __attribute__((packed));

#define USB_DEVICE_DESCRIPTOR_LENGTH             18
#define USB_DEVICE_DESCRIPTOR_TYPE               USB_DEVREQ_DESCRIPTOR_TYPE_DEVICE
#define USB_DEVICE_DESCRIPTOR_USB11_LO           0x10
#define USB_DEVICE_DESCRIPTOR_USB11_HI           0x01

#define USB_DEVICE_DESCRIPTOR_CLASS_INTERFACE    0x00    
#define USB_DEVICE_DESCRIPTOR_CLASS_VENDOR       0x00FF
#define USB_DEVICE_DESCRIPTOR_SUBCLASS_INTERFACE 0x00
#define USB_DEVICE_DESCRIPTOR_SUBCLASS_VENDOR    0x00FF
#define USB_DEVICE_DESCRIPTOR_PROTOCOL_INTERFACE 0x00
#define USB_DEVICE_DESCRIPTOR_PROTOCOL_VENDOR    0x00FF
    
typedef struct usb_configuration_descriptor {
    unsigned char       length;
    unsigned char       type;
    unsigned char       total_length_lo;
    unsigned char       total_length_hi;
    unsigned char       number_interfaces;
    unsigned char       configuration_id;
    unsigned char       configuration_str;
    unsigned char       attributes;
    unsigned char       max_power;
} usb_configuration_descriptor __attribute__((packed));

#define USB_CONFIGURATION_DESCRIPTOR_LENGTH     9
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       USB_DEVREQ_DESCRIPTOR_TYPE_CONFIGURATION
#define USB_CONFIGURATION_DESCRIPTOR_ATTR_REQUIRED      (1 << 7)
#define USB_CONFIGURATION_DESCRIPTOR_ATTR_SELF_POWERED  (1 << 6)
#define USB_CONFIGURATION_DESCRIPTOR_ATTR_REMOTE_WAKEUP (1 << 5)
    
typedef struct usb_interface_descriptor {
    unsigned char       length;
    unsigned char       type;
    unsigned char       interface_id;
    unsigned char       alternate_setting;
    unsigned char       number_endpoints;
    unsigned char       interface_class;
    unsigned char       interface_subclass;
    unsigned char       interface_protocol;
    unsigned char       interface_str;
} usb_interface_descriptor __attribute__((packed));        

#define USB_INTERFACE_DESCRIPTOR_LENGTH          9
#define USB_INTERFACE_DESCRIPTOR_TYPE            USB_DEVREQ_DESCRIPTOR_TYPE_INTERFACE
#define USB_INTERFACE_DESCRIPTOR_CLASS_VENDOR    0x00FF
#define USB_INTERFACE_DESCRIPTOR_SUBCLASS_VENDOR 0x00FF
#define USB_INTERFACE_DESCRIPTOR_PROTOCOL_VENDOR 0x00FF

typedef struct usb_endpoint_descriptor {
    unsigned char       length;
    unsigned char       type;
    unsigned char       endpoint;
    unsigned char       attributes;
    unsigned char       max_packet_lo;
    unsigned char       max_packet_hi;
    unsigned char       interval;
} usb_endpoint_descriptor;

#define USB_ENDPOINT_DESCRIPTOR_LENGTH           7
#define USB_ENDPOINT_DESCRIPTOR_TYPE             USB_DEVREQ_DESCRIPTOR_TYPE_ENDPOINT
#define USB_ENDPOINT_DESCRIPTOR_ENDPOINT_OUT     0
#define USB_ENDPOINT_DESCRIPTOR_ENDPOINT_IN      (1 << 7)
#define USB_ENDPOINT_DESCRIPTOR_ATTR_CONTROL     0x00
#define USB_ENDPOINT_DESCRIPTOR_ATTR_ISOCHRONOUS 0x01
#define USB_ENDPOINT_DESCRIPTOR_ATTR_BULK        0x02
#define USB_ENDPOINT_DESCRIPTOR_ATTR_INTERRUPT   0x03

// String descriptors. If these are used at all then string 0
// must be a table of supported LANGID codes. For a simple device
// which only supports US English, the following sequence of
// four bytes should suffice for string 0. In practice string
// constants tend to be used which makes the use of these
// #define's difficult.    
#define USB_STRING_DESCRIPTOR_STRING0_LENGTH    4
#define USB_STRING_DESCRIPTOR_STRING0_TYPE      USB_DEVREQ_DESCRIPTOR_TYPE_STRING
#define USB_STRING_DESCRIPTOR_STRING0_LANGID_LO 0x09
#define USB_STRING_DESCRIPTOR_STRING0_LANGID_HI 0x04

// For subsequent strings the length and data will have to be
// determined by the application developer or by a suitable tool.
#define USB_STRING_DESCRIPTOR_TYPE              USB_DEVREQ_DESCRIPTOR_TYPE_STRING    

// Utility macros to calculate the total_length fields in a
// configuration descriptor.
#define USB_CONFIGURATION_DESCRIPTOR_TOTAL_LENGTH_LO(interfaces, endpoints) \
    (USB_CONFIGURATION_DESCRIPTOR_LENGTH +            \
     (interfaces * USB_INTERFACE_DESCRIPTOR_LENGTH) + \
     (endpoints  * USB_ENDPOINT_DESCRIPTOR_LENGTH)) % 256

#define USB_CONFIGURATION_DESCRIPTOR_TOTAL_LENGTH_HI(interfaces, endpoints) \
    (USB_CONFIGURATION_DESCRIPTOR_LENGTH +            \
     (interfaces * USB_INTERFACE_DESCRIPTOR_LENGTH) + \
     (endpoints  * USB_ENDPOINT_DESCRIPTOR_LENGTH)) / 256
    
#ifdef __cplusplus
} // extern "C" {
#endif

#endif // CYGONCE_USB_H

