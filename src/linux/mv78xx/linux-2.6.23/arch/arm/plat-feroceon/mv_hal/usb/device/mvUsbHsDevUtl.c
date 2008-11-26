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

#include "mvUsbDevApi.h"
#include "mvUsbDevPrv.h"
#include "mvUsbDefs.h"

/* Test packet for Test Mode : TEST_PACKET. USB 2.0 Specification section 7.1.20 */
uint_8 test_packet[USB_TEST_MODE_TEST_PACKET_LENGTH] = 
{
   /* Synch */
   /* DATA 0 PID */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
   0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 
   0xAA, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
   0xEE, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xBF, 0xDF, 
   0xEF, 0xF7, 0xFB, 0xFD, 0xFC, 0x7E, 0xBF, 0xDF, 
   0xEF, 0xF7, 0xFB, 0xFD, 0x7E
};

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_assert_resume
*  Returned Value : None
*  Comments       :
*        Resume signalling for remote wakeup
*
*END*-----------------------------------------------------------------*/
void _usb_dci_vusb20_assert_resume
   (
      /* [IN] the USB_dev_initialize state structure */
      _usb_device_handle         handle
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR                     usb_dev_ptr;
   VUSB20_REG_STRUCT_PTR             dev_ptr;
   uint_32                                      temp;
 
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
   dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;

   /* Assert the Resume signal */   
   temp = USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.PORTSCX[0]);
   temp &= ~EHCI_PORTSCX_W1C_BITS;
   temp |= EHCI_PORTSCX_PORT_FORCE_RESUME;
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.PORTSCX[0] = USB_32BIT_LE(temp);
   
   /* Port change interrupt will be asserted at the end of resume 
   ** operation 
   */

} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_stall_endpoint
*  Returned Value : None
*  Comments       :
*        Stalls the specified endpoint
*
*END*-----------------------------------------------------------------*/
void _usb_dci_vusb20_stall_endpoint
   (
      /* [IN] the USB_dev_initialize state structure */
      _usb_device_handle         handle,
            
      /* [IN] the Endpoint number */
      uint_8                     ep_num,
            
      /* [IN] direction */
      uint_8                     direction
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR             usb_dev_ptr;
   VUSB20_REG_STRUCT_PTR                dev_ptr;
   VUSB20_EP_QUEUE_HEAD_STRUCT _PTR_    ep_queue_head_ptr;
      
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
   dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;
   
   /* Get the endpoint queue head address */
   ep_queue_head_ptr = (VUSB20_EP_QUEUE_HEAD_STRUCT_PTR)usb_dev_ptr->EP_QUEUE_HEAD_PTR + 
                                                                    2*ep_num + direction;
   /* Stall the endpoint for Rx or Tx and set the endpoint type */
   if (ep_queue_head_ptr->MAX_PKT_LENGTH & USB_32BIT_LE(VUSB_EP_QUEUE_HEAD_IOS)) 
   {
      /* This is a control endpoint so STALL both directions */
      dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[ep_num] |= 
         USB_32BIT_LE((EHCI_EPCTRL_TX_EP_STALL | EHCI_EPCTRL_RX_EP_STALL));
   } 
   else 
   {   
       if(direction) 
       {
            dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[ep_num] |= 
                        USB_32BIT_LE(EHCI_EPCTRL_TX_EP_STALL);
       }
       else {
            dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[ep_num] |= 
                        USB_32BIT_LE(EHCI_EPCTRL_RX_EP_STALL);
       }
   } /* Endif */

   ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_STALL, 
                    "STALL ep=%d %s: EPCTRLX=0x%x, CURR_dTD=0x%x, NEXT_dTD=0x%x, SIZE=0x%x\n", 
                    ep_num, direction ? "SEND" : "RECV",
                    (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[ep_num]),
                    (unsigned)USB_32BIT_LE(ep_queue_head_ptr->CURR_DTD_PTR), 
                    (unsigned)USB_32BIT_LE(ep_queue_head_ptr->NEXT_DTD_PTR),
                    (unsigned)USB_32BIT_LE(ep_queue_head_ptr->SIZE_IOC_INT_STS));
      
} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_unstall_endpoint
*  Returned Value : None
*  Comments       :
*        Unstall the specified endpoint in the specified direction
*
*END*-----------------------------------------------------------------*/
void _usb_dci_vusb20_unstall_endpoint
   (
      /* [IN] the USB_dev_initialize state structure */
      _usb_device_handle         handle,
            
      /* [IN] the Endpoint number */
      uint_8                     ep_num,
            
      /* [IN] direction */
      uint_8                     direction
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR     usb_dev_ptr;
   VUSB20_REG_STRUCT_PTR        dev_ptr;

   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
   dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;
   
   /* Enable the endpoint for Rx or Tx and set the endpoint type */
   if(direction)
   {
        dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[ep_num] |= 
              USB_32BIT_LE(EHCI_EPCTRL_TX_DATA_TOGGLE_RST);

        dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[ep_num] &= 
                ~(USB_32BIT_LE(EHCI_EPCTRL_TX_EP_STALL));
   }
   else
   {
        dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[ep_num] |= 
              USB_32BIT_LE(EHCI_EPCTRL_RX_DATA_TOGGLE_RST);

        dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[ep_num] &= 
                ~(USB_32BIT_LE(EHCI_EPCTRL_RX_EP_STALL));
   }     

   ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_STALL, 
                    "UNSTALL ep=%d %s: EPCTRLX=0x%x\n", 
                    ep_num, direction ? "SEND" : "RECV",
                    (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[ep_num]));


} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : _usb_dci_vusb20_is_endpoint_stalled
* Returned Value : None
* Comments       :
*     Gets the endpoint status
* 
*END*--------------------------------------------------------------------*/
uint_8 _usb_dci_vusb20_is_endpoint_stalled
   (
      /* [IN] Handle to the USB device */
      _usb_device_handle   handle,
      
      /* [IN] Endpoint number */
      uint_8               ep,

      /* [IN] Endpoint direction */
      uint_8               dir
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR     usb_dev_ptr;
   VUSB20_REG_STRUCT_PTR        dev_ptr;
   uint_32                      value;

   ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_TRACE, "is_endpoint_stalled\n");
   
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
   dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;
  
   if(dir)
   {
        value = dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[ep] & 
                                    (USB_32BIT_LE(EHCI_EPCTRL_TX_EP_STALL));
   }
   else
   {
        value = dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[ep] & 
                                    (USB_32BIT_LE(EHCI_EPCTRL_RX_EP_STALL));
   }
   return (value) ? 1 : 0;
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : _usb_dci_vusb20_set_test_mode
* Returned Value : None
* Comments       :
*     sets/resets the test mode
* 
*END*--------------------------------------------------------------------*/
void _usb_dci_vusb20_set_test_mode
   (
      /* [IN] Handle to the USB device */
      _usb_device_handle handle,
      
      /* [IN] Test mode */
      uint_16 test_mode
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR                     usb_dev_ptr;
   VUSB20_REG_STRUCT_PTR                        dev_ptr;
   uint_32                                      temp;

   ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_ANY, "set_test_mode\n");
   
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
   dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;
   
   temp = USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[0]);
   
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[0] = 
                                USB_32BIT_LE((temp | EHCI_EPCTRL_TX_DATA_TOGGLE_RST));

   if (test_mode == ARC_USB_TEST_MODE_TEST_PACKET) 
   {
       USB_memcopy(test_packet, usb_dev_ptr->TEST_PKT_PTR, USB_TEST_MODE_TEST_PACKET_LENGTH); 
      _usb_device_send_data(handle, 0, usb_dev_ptr->TEST_PKT_PTR, USB_TEST_MODE_TEST_PACKET_LENGTH);

   } /* Endif */
   
   temp = USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.PORTSCX[0]);
   temp &= ~EHCI_PORTSCX_W1C_BITS;
   
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.PORTSCX[0] = 
                                    USB_32BIT_LE(temp | ((uint_32)test_mode << 8));
             
} /* EndBody */

