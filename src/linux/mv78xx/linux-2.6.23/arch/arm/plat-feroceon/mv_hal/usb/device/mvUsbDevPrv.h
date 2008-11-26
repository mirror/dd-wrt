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

#ifndef __mvUsbDevPrv_h__
#define __mvUsbDevPrv_h__

#include "mvUsbCore.h"


#define USB_TEST_MODE_TEST_PACKET_LENGTH   (53)

 
#define USB_XD_QADD(head,tail,XD)      \
   if ((head) == NULL) {         \
      (head) = (XD);            \
   } else {                      \
      (tail)->SCRATCH_PTR->PRIVATE = (XD);   \
   } /* Endif */                 \
   (tail) = (XD);               \
   (XD)->SCRATCH_PTR->PRIVATE = NULL
   
#define USB_XD_QGET(head,tail,XD)      \
   (XD) = (head);               \
   if (head) {                   \
      (head) = (XD_STRUCT_PTR)((head)->SCRATCH_PTR->PRIVATE);  \
      if ((head) == NULL) {      \
         (tail) = NULL;          \
      } /* Endif */              \
   } /* Endif */

#define EHCI_DTD_QADD(head,tail,dTD)      \
   if ((head) == NULL) {         \
      (head) = (dTD);            \
   } else {                      \
      (tail)->SCRATCH_PTR->PRIVATE = (void *) (dTD);   \
   } /* Endif */                 \
   (tail) = (dTD);               \
   (dTD)->SCRATCH_PTR->PRIVATE = NULL
   
#define EHCI_DTD_QGET(head,tail,dTD)      \
   (dTD) = (head);               \
   if (head) {                   \
      (head) = (head)->SCRATCH_PTR->PRIVATE;  \
      if ((head) == NULL) {      \
         (tail) = NULL;          \
      } /* Endif */              \
   } /* Endif */

/***************************************
**
** Data structures
**
*/

typedef struct
{
    uint_32     usb_isr_count;
    uint_32     usb_reset_count;
    uint_32     usb_send_count;
    uint_32     usb_recv_count;
    uint_32     usb_setup_count;
    uint_32     free_XD_count;
    uint_32     free_dTD_count;
    uint_32     usb_cancel_count;
    uint_32     usb_add_count;
    uint_32     usb_add_not_empty_count;
    uint_32     usb_empty_isr_count;
    uint_32     usb_empty_complete_count;
    uint_32     usb_read_setup_count;
    uint_32     usb_complete_isr_count;
    uint_32     usb_complete_count;
    uint_32     usb_complete_max_count;
    uint_32     usb_port_change_count; 
    uint_32     usb_suspend_count;
    uint_32     usb_complete_ep_count[ARC_USB_MAX_ENDPOINTS*2];

} USB_STATS;



/* Callback function storage structure */
typedef struct service_struct 
{
    uint_8   TYPE;
    void     (_CODE_PTR_ SERVICE)(pointer, uint_8, boolean, uint_8, uint_8_ptr, uint_32, uint_8);
    struct service_struct _PTR_   NEXT;

} SERVICE_STRUCT, _PTR_ SERVICE_STRUCT_PTR;

typedef struct xd_struct 
{
    uint_8         EP_NUM;           /* Endpoint number */
    uint_8         BDIRECTION;       /* Direction : Send/Receive */
    uint_8         EP_TYPE;          /* Type of the endpoint: Ctrl, Isoch, Bulk, Int */
    uint_8         BSTATUS;          /* Current transfer status */
    uint_8_ptr     WSTARTADDRESS;    /* Address of first byte */
    uint_32        WTOTALLENGTH;     /* Number of bytes to send/recv */
    uint_32        WSOFAR;           /* Number of bytes recv'd so far */
    uint_16        WMAXPACKETSIZE;   /* Max Packet size */
    boolean        DONT_ZERO_TERMINATE;
    uint_8         MAX_PKTS_PER_UFRAME;
    SCRATCH_STRUCT *SCRATCH_PTR;
} XD_STRUCT, _PTR_ XD_STRUCT_PTR;

/* The USB Device State Structure */
typedef struct 
{
    boolean                          BUS_RESETTING;       /* Device is 
                                                         ** being reset */
    volatile VUSB20_REG_STRUCT_PTR   CAP_REGS_PTR;        /* Capabilities registers */

    volatile VUSB20_REG_STRUCT_PTR   DEV_PTR;            /* Device Controller 
                                                         ** Register base 
                                                         ** address */

    SERVICE_STRUCT_PTR               SERVICE_HEAD_PTR;   /* Head struct 
                                                         ** address of 
                                                         ** registered services 
                                                         */
    XD_STRUCT_PTR                    TEMP_XD_PTR;         /* Temp xd for ep init */
    XD_STRUCT_PTR                    XD_BASE;
    XD_STRUCT_PTR                    XD_HEAD;             /* Head Transaction 
                                                         ** descriptors 
                                                         */
    XD_STRUCT_PTR                    XD_TAIL;             /* Tail Transaction 
                                                         ** descriptors 
                                                         */
    uint_32                          XD_ENTRIES;
    uint_8*                          EP_QUEUE_HEAD_BASE;
    uint_32                          EP_QUEUE_HEAD_PHYS;
    uint_32                          EP_QUEUE_HEAD_SIZE;
    VUSB20_EP_QUEUE_HEAD_STRUCT_PTR  EP_QUEUE_HEAD_PTR;   /* Endpoint Queue head */   

    uint_8*                          DTD_BASE_PTR;        /* Device transfer descriptor pool address */
    uint_32                          DTD_BASE_PHYS;
    uint_32                          DTD_SIZE;
    VUSB20_EP_TR_STRUCT_PTR          DTD_ALIGNED_BASE_PTR;/* Aligned transfer descriptor pool address */

    VUSB20_EP_TR_STRUCT_PTR          DTD_HEAD;
    VUSB20_EP_TR_STRUCT_PTR          DTD_TAIL;
    VUSB20_EP_TR_STRUCT_PTR          EP_DTD_HEADS[ARC_USB_MAX_ENDPOINTS * 2];
    VUSB20_EP_TR_STRUCT_PTR          EP_DTD_TAILS[ARC_USB_MAX_ENDPOINTS * 2];
    SCRATCH_STRUCT_PTR               XD_SCRATCH_STRUCT_BASE;
   
   
    SCRATCH_STRUCT_PTR               SCRATCH_STRUCT_BASE;
   
    uint_16                          USB_STATE;
    uint_16                          USB_DEVICE_STATE;
    uint_16                          USB_SOF_COUNT;
    uint_16                          DTD_ENTRIES;
    uint_16                          ERRORS;
    uint_16                          ERROR_STATE;
    uint_16                          USB_DEV_STATE_B4_SUSPEND;
    uint_8                           DEV_NUM;             /* USB device number 
                                                         ** on the board 
                                                         */
    uint_8                           SPEED;               /* Low Speed, 
                                                         ** High Speed, 
                                                         ** Full Speed 
                                                         */
    uint_8                           MAX_ENDPOINTS;       /* Max endpoints
                                                         ** supported by this
                                                         ** device
                                                         */
                                                         
    uint_8                           USB_CURR_CONFIG;                                                         
    uint_8                           DEVICE_ADDRESS;
    uint_8                           FORCE_FS;
    USB_STATS                        STATS;

    uint_8*                          STATUS_UNAIGNED_PTR;
    uint_16*                         STATUS_PTR;

    uint_8*                          TEST_PKT_UNAIGNED_PTR;
    uint_8*                          TEST_PKT_PTR;

} USB_DEV_STATE_STRUCT, _PTR_ USB_DEV_STATE_STRUCT_PTR;

/* ONLY For data bases allocated by the driver (when PHYS and VIRT bases are known) */
#define USB_EP_QH_VIRT_TO_PHYS(handle, virtAddr)                                                    \
    (((virtAddr) == NULL) ? 0 : ((handle)->EP_QUEUE_HEAD_PHYS +                                \
                          ((uint_32)(virtAddr) - (uint_32)(handle)->EP_QUEUE_HEAD_BASE)))

#define USB_DTD_VIRT_TO_PHYS(handle, virtAddr)                                                    \
    (((virtAddr) == NULL) ? 0 : ((handle)->DTD_BASE_PHYS +                                \
                          ((uint_32)(virtAddr) - (uint_32)(handle)->DTD_BASE_PTR)))

#define USB_DTD_PHYS_TO_VIRT(handle, physAddr)                                                    \
    (((physAddr) == 0) ? NULL : ((handle)->DTD_BASE_PTR +                             \
                                ((physAddr) - (handle)->DTD_BASE_PHYS)))


/***************************************
**
** Prototypes
**
*/
#ifdef __cplusplus
extern "C" {
#endif

extern uint_8  _usb_device_call_service(void* handle, uint_8, boolean,
                                 boolean, uint_8_ptr, uint_32, uint_8);

extern uint_8   _usb_dci_vusb20_init(uint_8, _usb_device_handle);
extern void     _usb_device_free_XD(pointer);
extern void     _usb_dci_vusb20_free_dTD(pointer);
extern uint_8   _usb_dci_vusb20_add_dTD(_usb_device_handle, XD_STRUCT_PTR);
extern uint_8   _usb_dci_vusb20_cancel_transfer(_usb_device_handle, uint_8, uint_8);
extern uint_8   _usb_dci_vusb20_get_transfer_status(_usb_device_handle, uint_8, uint_8);
extern XD_STRUCT_PTR _usb_dci_vusb20_get_transfer_details(_usb_device_handle, uint_8, uint_8);
extern void     _usb_dci_vusb20_process_tr_complete(_usb_device_handle);
extern void     _usb_dci_vusb20_process_reset(_usb_device_handle);
extern void     _usb_dci_vusb20_process_tr_complete(_usb_device_handle);
extern void     _usb_dci_vusb20_process_suspend(_usb_device_handle);
extern void     _usb_dci_vusb20_process_SOF(_usb_device_handle);
extern void     _usb_dci_vusb20_process_port_change(_usb_device_handle);
extern void     _usb_dci_vusb20_process_error(_usb_device_handle);
extern void     _usb_dci_vusb20_shutdown(_usb_device_handle);
extern void     _usb_dci_vusb20_set_speed_full(_usb_device_handle, uint_8);
extern void     _usb_dci_vusb20_suspend_phy(_usb_device_handle, uint_8);
extern void     _usb_dci_vusb20_hnp_shutdown(void);
extern void     _usb_dci_vusb20_set_address(_usb_device_handle, uint_8);
extern void     _usb_dci_vusb20_get_setup_data(_usb_device_handle, uint_8, uint_8_ptr);
extern void     _usb_dci_vusb20_assert_resume(_usb_device_handle);
extern uint_8   _usb_dci_vusb20_init_endpoint(_usb_device_handle, XD_STRUCT_PTR);
extern void     _usb_dci_vusb20_stall_endpoint(_usb_device_handle, uint_8, uint_8);
extern void     _usb_dci_vusb20_unstall_endpoint(_usb_device_handle, uint_8, uint_8);
extern uint_8   _usb_dci_vusb20_is_endpoint_stalled(_usb_device_handle, uint_8, uint_8);
extern uint_8   _usb_dci_vusb20_deinit_endpoint(_usb_device_handle, uint_8, uint_8);
extern void     _usb_dci_vusb20_chip_initialize(_usb_device_handle);
extern void     _usb_dci_vusb20_stop(_usb_device_handle handle);
extern void     _usb_dci_vusb20_start(_usb_device_handle handle);

#if defined(USB_UNDERRUN_WA)

extern uint_8*  usbSramBase;
extern int      usbSramSize;

void    _usb_reset_send_queue(void);
void    usbSendComplete(void* handle, uint_8 type, boolean setup, uint_8 dir, 
                        uint_8_ptr buffer, uint_32 length, uint_8 error);
#endif /* USB_UNDERRUN_WA */
                         
#ifdef __cplusplus
}
#endif

#endif
