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

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_cancel_transfer
*  Returned Value : USB_OK or error code
*  Comments       :
*        Cancels a transfer
*
*END*-----------------------------------------------------------------*/
uint_8 _usb_dci_vusb20_cancel_transfer
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
   VUSB20_EP_TR_STRUCT_PTR              dTD_ptr, check_dTD_ptr;
   VUSB20_EP_QUEUE_HEAD_STRUCT_PTR      ep_queue_head_ptr;
   XD_STRUCT_PTR                        xd_ptr;
   uint_32                              temp, bit_pos;
   volatile unsigned long               timeout, status_timeout;
   
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
   dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;
   
   bit_pos = (1 << (16 * direction + ep_num));
   temp = (2*ep_num + direction);
   
   ep_queue_head_ptr = (VUSB20_EP_QUEUE_HEAD_STRUCT_PTR)usb_dev_ptr->EP_QUEUE_HEAD_PTR + temp;
   
   /* Unlink the dTD */
   dTD_ptr = usb_dev_ptr->EP_DTD_HEADS[temp];
   
   if (dTD_ptr) 
   {
      ARC_DEBUG_CODE(ARC_DEBUG_FLAG_STATS, (usb_dev_ptr->STATS.usb_cancel_count++));

      check_dTD_ptr = (VUSB20_EP_TR_STRUCT_PTR)USB_DTD_PHYS_TO_VIRT(usb_dev_ptr, 
                            ((uint_32)USB_32BIT_LE(dTD_ptr->NEXT_TR_ELEM_PTR) & VUSBHS_TD_ADDR_MASK));

      if (USB_32BIT_LE(dTD_ptr->SIZE_IOC_STS) & VUSBHS_TD_STATUS_ACTIVE) 
      {
         /* Flushing will halt the pipe */
         /* Write 1 to the Flush register */
         dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTFLUSH = USB_32BIT_LE(bit_pos);

         /* Wait until flushing completed */
         timeout = 0x1000000;
         while (USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTFLUSH) & bit_pos) 
         {
            /* ENDPTFLUSH bit should be cleared to indicate this operation is complete */
            timeout--;
            if(timeout == 0)
            {
                USB_printf("USB Cancel: - TIMEOUT for ENDPTFLUSH=0x%x, bit_pos=0x%x \n", 
                      (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTFLUSH), 
                      (unsigned)bit_pos);
                break;
            }
         } /* EndWhile */
         status_timeout = 0x100000;
         while (USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTSTATUS) & bit_pos) 
         {
            status_timeout--;
            if(status_timeout == 0)
            {
                USB_printf("USB Cancel: - TIMEOUT for ENDPTSTATUS=0x%x, bit_pos=0x%x\n", 
                      (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTSTATUS), 
                      (unsigned)bit_pos);
                break;
            }

            /* Write 1 to the Flush register */
            dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTFLUSH = USB_32BIT_LE(bit_pos);
         
            /* Wait until flushing completed */
            timeout = 0x1000000;
            while (USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTFLUSH) & bit_pos) 
            {
               /* ENDPTFLUSH bit should be cleared to indicate this operation is complete */
               timeout--;
               if(timeout == 0)
                {
                    USB_printf("USB Cancel: - TIMEOUT for ENDPTFLUSH=0x%x, ENDPTSTATUS=0x%x, bit_pos=0x%x\n", 
                            (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTFLUSH), 
                            (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTSTATUS),
                            (unsigned)bit_pos);
                    break;
                }
            } /* EndWhile */
         } /* EndWhile */
      } /* Endif */
      
      /* Retire the current dTD */
      dTD_ptr->SIZE_IOC_STS = 0;
      dTD_ptr->NEXT_TR_ELEM_PTR = USB_32BIT_LE(VUSBHS_TD_NEXT_TERMINATE);
      
      /* The transfer descriptor for this dTD */
      xd_ptr = (XD_STRUCT_PTR)dTD_ptr->SCRATCH_PTR->XD_FOR_THIS_DTD;
      dTD_ptr->SCRATCH_PTR->PRIVATE = (pointer)usb_dev_ptr;

      ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_TRANSFER,
                      "cncl_%d: fri=0x%x, ep=%d%s, buf=%p, size=%d, xd=%p, dTD=%p %p, bit=0x%x\n",
                       usb_dev_ptr->STATS.usb_cancel_count & 0xFFFF, 
                       USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_FRINDEX), 
                       ep_num, direction ? "in" : "out", 
                       xd_ptr->WSTARTADDRESS, xd_ptr->WTOTALLENGTH, xd_ptr, 
                       dTD_ptr, check_dTD_ptr, bit_pos);
       
      /* Free the dTD */
      _usb_dci_vusb20_free_dTD((pointer)dTD_ptr);
      
      /* Update the dTD head and tail for specific endpoint/direction */
      if (!check_dTD_ptr) 
      {
         usb_dev_ptr->EP_DTD_HEADS[temp] = NULL;
         usb_dev_ptr->EP_DTD_TAILS[temp] = NULL;
         if (xd_ptr) 
         {
            xd_ptr->SCRATCH_PTR->PRIVATE = (pointer)usb_dev_ptr;
            /* Free the transfer descriptor */
            _usb_device_free_XD((pointer)xd_ptr);
         } /* Endif */
         /* No other transfers on the queue */
         ep_queue_head_ptr->NEXT_DTD_PTR = USB_32BIT_LE(VUSB_EP_QUEUE_HEAD_NEXT_TERMINATE);
         ep_queue_head_ptr->SIZE_IOC_INT_STS = 0;
      } 
      else 
      {
         usb_dev_ptr->EP_DTD_HEADS[temp] = check_dTD_ptr;
            
         if (xd_ptr) 
         {
            if ((uint_32)check_dTD_ptr->SCRATCH_PTR->XD_FOR_THIS_DTD != (uint_32)xd_ptr) 
            {
               xd_ptr->SCRATCH_PTR->PRIVATE = (pointer)usb_dev_ptr;
               /* Free the transfer descriptor */
               _usb_device_free_XD((pointer)xd_ptr);
            } /* Endif */
         } /* Endif */
         
         if (USB_32BIT_LE(check_dTD_ptr->SIZE_IOC_STS) & VUSBHS_TD_STATUS_ACTIVE) 
         {         
            /* Start CR 1015 */
            /* Prime the Endpoint */
            dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTPRIME = USB_32BIT_LE(bit_pos);

            if (!(USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTSTATUS) & bit_pos)) 
            {
               timeout = 0x100000;
               while (USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTPRIME) & bit_pos) 
               {
                  /* Wait for the ENDPTPRIME to go to zero */
                  timeout--;
                  if(timeout == 0)
                  {
                      USB_printf("USB Cancel: - TIMEOUT for ENDPTPRIME=0x%x, bit_pos=0x%x\n", 
                                (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTPRIME), 
                                (unsigned)bit_pos);
                      break;
                  }
               } /* EndWhile */

               if (USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTSTATUS) & bit_pos) 
               {
                  /* The endpoint was not not primed so no other transfers on 
                  ** the queue 
                  */
                  goto done;
               } /* Endif */
            } 
            else 
            {
               goto done;
            } /* Endif */

            /* No other transfers on the queue */
            ep_queue_head_ptr->NEXT_DTD_PTR = (uint_32)USB_32BIT_LE((uint_32)check_dTD_ptr);
            ep_queue_head_ptr->SIZE_IOC_INT_STS = 0;
   
            /* Prime the Endpoint */
            dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTPRIME = USB_32BIT_LE(bit_pos);
         } /* Endif */
      } /* Endif */
   } /* Endif */
   
done:

   /* End CR 1015 */  
   return USB_OK;
} /* EndBody */

/* EOF */
