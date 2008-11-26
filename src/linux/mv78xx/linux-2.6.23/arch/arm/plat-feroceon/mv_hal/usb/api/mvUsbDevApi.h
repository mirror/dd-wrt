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

#ifndef __mvUsbDevApi_h__
#define __mvUsbDevApi_h__

#include "mvUsbTypes.h"
#include "mvUsbDebug.h"
#include "mvUsbDefs.h"
#include "ctrlEnv/mvCtrlEnvSpec.h"


#define ARC_USB_MAX_ENDPOINTS               (16)

#define MAX_EP_TR_DESCRS                    (48)
#define MAX_XDS_FOR_TR_CALLS                (32)
#define MAX_USB_DEVICES                     MV_USB_MAX_CHAN

/*Assumption here is that all control endpoints are sequential 0,1,..
if they are not you need to modify the tr_complete routine to handle that */
#define USB_MAX_CONTROL_ENDPOINTS           (1)

#define USB_MAX_CTRL_PAYLOAD                (64)


/* Endpoint types */
#define  ARC_USB_CONTROL_ENDPOINT             (0)
#define  ARC_USB_ISOCHRONOUS_ENDPOINT         (1)
#define  ARC_USB_BULK_ENDPOINT                (2)
#define  ARC_USB_INTERRUPT_ENDPOINT           (3)

/* Informational Request/Set Types */
#define  ARC_USB_STATUS_DEVICE_STATE          (0x01)
#define  ARC_USB_STATUS_INTERFACE             (0x02)
#define  ARC_USB_STATUS_ADDRESS               (0x03)
#define  ARC_USB_STATUS_CURRENT_CONFIG        (0x04)
#define  ARC_USB_STATUS_SOF_COUNT             (0x05)
#define  ARC_USB_STATUS_DEVICE                (0x06)
#define  ARC_USB_STATUS_TEST_MODE             (0x07)
#define  ARC_USB_FORCE_FULL_SPEED             (0x08)
#define  ARC_USB_PHY_LOW_POWER_SUSPEND        (0x09)

#define  ARC_USB_STATUS_ENDPOINT_NUMBER_MASK  (0x000F)
#define  ARC_USB_STATUS_ENDPOINT_DIR_MASK     (0x0080)

#define  ARC_USB_TEST_MODE_TEST_PACKET        (0x0400)

/* Available service types */
/* Services 0 through 15 are reserved for endpoints */
#define  ARC_USB_SERVICE_EP0                  (0x00)
#define  ARC_USB_SERVICE_EP1                  (0x01)
#define  ARC_USB_SERVICE_EP2                  (0x02)
#define  ARC_USB_SERVICE_EP3                  (0x03)
#define  ARC_USB_SERVICE_BUS_RESET            (0x10)
#define  ARC_USB_SERVICE_SUSPEND              (0x11)
#define  ARC_USB_SERVICE_SOF                  (0x12)
#define  ARC_USB_SERVICE_RESUME               (0x13)
#define  ARC_USB_SERVICE_SLEEP                (0x14)
#define  ARC_USB_SERVICE_SPEED_DETECTION      (0x15)
#define  ARC_USB_SERVICE_ERROR                (0x16)
#define  ARC_USB_SERVICE_STALL                (0x17)

typedef pointer _usb_device_handle;
typedef void (*USB_SERVICE_FUNC)(void* handle, uint_8, boolean, uint_8, 
                                                 uint_8_ptr, uint_32, uint_8);

#ifdef __cplusplus
extern "C" {
#endif

void    _usb_dci_vusb20_isr(void* handle);

void    _usb_device_set_bsp_funcs(USB_IMPORT_FUNCS* pBspFuncs);

uint_8  _usb_device_init(uint_8 devNo, void** pHandle);

uint_8  _usb_device_get_max_endpoint(void* handle);

uint_8  _usb_device_get_dev_num(void* handle);

void    _usb_device_shutdown(void* handle);

void    _usb_device_stop(void* handle);
void    _usb_device_start(void* handle);

uint_8  _usb_device_init_endpoint(void* handle, uint_8 ep_num, uint_16 max_pkt_size, 
                                  uint_8 direction, uint_8 type, uint_8 flag);
uint_8  _usb_device_deinit_endpoint(void* handle, uint_8 ep_num, uint_8 direction);

uint_8  _usb_device_recv_data(void* handle, uint_8 ep_num, uint_8* buf_ptr, uint_32 size);
uint_8  _usb_device_send_data(void* handle, uint_8 ep_num, uint_8* buf_ptr, uint_32 size);
uint_8  _usb_device_cancel_transfer(void* handle, uint_8 ep_num, uint_8 direction);
uint_8  _usb_device_get_transfer_status(void* handle, uint_8 ep_num, uint_8 direction);
void    _usb_device_stall_endpoint(void* handle, uint_8 ep_num, uint_8 direction);
void    _usb_device_unstall_endpoint(void* handle, uint_8 ep_num, uint_8 direction);
uint_8  _usb_device_is_endpoint_stalled(void* handle, uint_8 ep_num, uint_8 direction);
void    _usb_device_assert_resume(void* handle);
uint_8  _usb_device_get_status(void* handle, uint_8 component, uint_16* status_ptr);
uint_8  _usb_device_set_status(void* handle, uint_8 component, uint_16 setting);
void    _usb_device_read_setup_data(void* handle, uint_8 ep_num, uint_8* buf_ptr);

uint_8  _usb_device_register_service(void* handle, uint_8 type, USB_SERVICE_FUNC serviceFunc);

uint_8  _usb_device_unregister_service(void* handle, uint_8 type);



/* These functions that implement USB 2.0 standard Chapter 9 Setup requests */
void    mvUsbCh9GetStatus(void* handle, boolean setup, 
                                  SETUP_STRUCT* ctrl_req);

void    mvUsbCh9ClearFeature(void* handle, boolean setup, 
                                   SETUP_STRUCT* setup_ptr);

void    mvUsbCh9SetFeature(void* handle, boolean setup, 
                                 SETUP_STRUCT* setup_ptr);

void    mvUsbCh9SetAddress(void* handle, boolean setup, 
                                   SETUP_STRUCT* setup_ptr);

/* DEBUG Functions */
void    _usb_dci_vusb20_set_test_mode(void* handle, uint_16 testMode);   

void    _usb_debug_set_flags(uint_32 flags);     
uint_32 _usb_debug_get_flags(void);
     
void    _usb_debug_init_trace_log(void);
void    _usb_debug_print_trace_log(void);

void    _usb_regs(void* usbHandle);
void    _usb_status(void* usbHandle);
void    _usb_stats(void* usbHandle);
void    _usb_clear_stats(void* usbHandle);
void    _usb_ep_status(void* usbHandle, int ep_num, int direction);

#ifdef __cplusplus
}
#endif

#endif /* __mvUsbDevApi_h__ */
/* EOF */

