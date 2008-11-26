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

#ifndef __mvUsbDefs_h__
#define __mvUsbDefs_h__

#include "mvUsbTypes.h"

/* Host specific */
#define  USB_DEBOUNCE_DELAY                  (101)
#define  USB_RESET_RECOVERY_DELAY            (11)
#define  USB_RESET_DELAY                     (60)

/* Error codes */
#define  USB_OK                              (0x00)
#define  USBERR_ALLOC                        (0x81)
#define  USBERR_BAD_STATUS                   (0x82)
#define  USBERR_CLOSED_SERVICE               (0x83)
#define  USBERR_OPEN_SERVICE                 (0x84)
#define  USBERR_TRANSFER_IN_PROGRESS         (0x85)
#define  USBERR_ENDPOINT_STALLED             (0x86)
#define  USBERR_ALLOC_STATE                  (0x87)
#define  USBERR_DRIVER_INSTALL_FAILED        (0x88)
#define  USBERR_DRIVER_NOT_INSTALLED         (0x89)
#define  USBERR_INSTALL_ISR                  (0x8A)
#define  USBERR_INVALID_DEVICE_NUM           (0x8B)
#define  USBERR_ALLOC_SERVICE                (0x8C)
#define  USBERR_INIT_FAILED                  (0x8D)
#define  USBERR_SHUTDOWN                     (0x8E)
#define  USBERR_INVALID_PIPE_HANDLE          (0x8F)
#define  USBERR_OPEN_PIPE_FAILED             (0x90)
#define  USBERR_INIT_DATA                    (0x91)
#define  USBERR_SRP_REQ_INVALID_STATE        (0x92)
#define  USBERR_TX_FAILED                    (0x93)
#define  USBERR_RX_FAILED                    (0x94)
#define  USBERR_EP_INIT_FAILED               (0x95)
#define  USBERR_EP_DEINIT_FAILED             (0x96)
#define  USBERR_TR_FAILED                    (0x97)
#define  USBERR_BANDWIDTH_ALLOC_FAILED       (0x98)
#define  USBERR_INVALID_NUM_OF_ENDPOINTS     (0x99)

#define  USBERR_DEVICE_NOT_FOUND             (0xC0)
#define  USBERR_DEVICE_BUSY                  (0xC1)
#define  USBERR_NO_DEVICE_CLASS              (0xC3)
#define  USBERR_UNKNOWN_ERROR                (0xC4)
#define  USBERR_INVALID_BMREQ_TYPE           (0xC5)
#define  USBERR_GET_MEMORY_FAILED            (0xC6)
#define  USBERR_INVALID_MEM_TYPE             (0xC7)
#define  USBERR_NO_DESCRIPTOR                (0xC8)
#define  USBERR_NULL_CALLBACK                (0xC9)
#define  USBERR_NO_INTERFACE                 (0xCA)
#define  USBERR_INVALID_CFIG_NUM             (0xCB)
#define  USBERR_INVALID_ANCHOR               (0xCC)
#define  USBERR_INVALID_REQ_TYPE             (0xCD)

/* Error Codes for lower-layer */
#define  USBERR_ALLOC_EP_QUEUE_HEAD          (0xA8)
#define  USBERR_ALLOC_TR                     (0xA9)
#define  USBERR_ALLOC_DTD_BASE               (0xAA)
#define  USBERR_CLASS_DRIVER_INSTALL         (0xAB)


/* Pipe Types */
#define  USB_ISOCHRONOUS_PIPE                (0x01)
#define  USB_INTERRUPT_PIPE                  (0x02)
#define  USB_CONTROL_PIPE                    (0x03)
#define  USB_BULK_PIPE                       (0x04)

#define  ARC_USB_STATE_UNKNOWN               (0xff)
#define  ARC_USB_STATE_POWERED               (0x03)
#define  ARC_USB_STATE_DEFAULT               (0x02)
#define  ARC_USB_STATE_ADDRESS               (0x01)
#define  ARC_USB_STATE_CONFIG                (0x00)
#define  ARC_USB_STATE_SUSPEND               (0x80)

#define  ARC_USB_SELF_POWERED                (0x01)
#define  ARC_USB_REMOTE_WAKEUP               (0x02)

/* Bus Control values */
#define  ARC_USB_NO_OPERATION                (0x00)
#define  ARC_USB_ASSERT_BUS_RESET            (0x01)
#define  ARC_USB_DEASSERT_BUS_RESET          (0x02)
#define  ARC_USB_ASSERT_RESUME               (0x03)
#define  ARC_USB_DEASSERT_RESUME             (0x04)
#define  ARC_USB_SUSPEND_SOF                 (0x05)
#define  ARC_USB_RESUME_SOF                  (0x06)

/* possible values of XD->bStatus */
#define  ARC_USB_STATUS_IDLE                 (0)
#define  ARC_USB_STATUS_TRANSFER_ACCEPTED    (1)
#define  ARC_USB_STATUS_TRANSFER_PENDING     (2)
#define  ARC_USB_STATUS_TRANSFER_IN_PROGRESS (3)
#define  ARC_USB_STATUS_ERROR                (4)
#define  ARC_USB_STATUS_DISABLED             (5)
#define  ARC_USB_STATUS_STALLED              (6)
#define  ARC_USB_STATUS_TRANSFER_QUEUED      (7)

#define  ARC_USB_RECV                        (0)
#define  ARC_USB_SEND                        (1)

#define  ARC_USB_DEVICE_DONT_ZERO_TERMINATE  (0x1)

#define  ARC_USB_SETUP_DATA_XFER_DIRECTION   (0x80)

#define  ARC_USB_SPEED_FULL                  (0)
#define  ARC_USB_SPEED_LOW                   (1)
#define  ARC_USB_SPEED_HIGH                  (2)

#define  ARC_USB_MAX_PKTS_PER_UFRAME         (0x6)

/* USB 1.1 Setup Packet */
typedef struct setup_struct {
   uint_8      REQUESTTYPE;
   uint_8      REQUEST;
   uint_16     VALUE;
   uint_16     INDEX;
   uint_16     LENGTH;
} SETUP_STRUCT, _PTR_ SETUP_STRUCT_PTR;

#endif /* __mvUsbDefs_h__ */

/* EOF */
