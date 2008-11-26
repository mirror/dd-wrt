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
#include "mvUsbCh9.h"

static volatile boolean  ENTER_TEST_MODE = FALSE; 
static volatile uint_16  test_mode_index = 0;


void    mvUsbCh9GetStatus(_usb_device_handle handle, boolean setup, 
                         SETUP_STRUCT* ctrl_req)
{ /* Body */
    uint_8                  endpoint, direction;
    uint_16                 usb_status;
    USB_DEV_STATE_STRUCT*   usb_dev_ptr = (USB_DEV_STATE_STRUCT*)handle; 

    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_SETUP, "%s: setup=%d\n", __FUNCTION__, (int)setup);

    if(!setup)
        return;

    switch (ctrl_req->REQUESTTYPE) 
    {
       case (REQ_DIR_IN | REQ_RECIP_DEVICE):
          /* Device request */
          _usb_device_get_status(handle, ARC_USB_STATUS_DEVICE, &usb_status);
          break;

       case (REQ_DIR_IN | REQ_RECIP_INTERFACE):
          /* Interface request */
          _usb_device_get_status(handle, ARC_USB_STATUS_INTERFACE, &usb_status);
          break;
    
       case (REQ_DIR_IN | REQ_RECIP_ENDPOINT):
          /* Endpoint request */
          endpoint = ctrl_req->INDEX & ARC_USB_STATUS_ENDPOINT_NUMBER_MASK;
          if( (ctrl_req->INDEX & (1 << REQ_DIR_OFFSET)) == REQ_DIR_IN)
            direction = ARC_USB_SEND;
          else
            direction = ARC_USB_RECV;

          usb_status = _usb_device_is_endpoint_stalled(handle, endpoint, direction);
          break;
       
       default:
          /* Unknown request */
           USB_printf("GetStatus: Unknown request type 0x%x\n", ctrl_req->REQUESTTYPE);
          _usb_device_stall_endpoint(handle, 0, ARC_USB_RECV);
          return;
    } /* Endswitch */

    /* Send the requested data */
    *usb_dev_ptr->STATUS_PTR = USB_16BIT_LE(usb_status);
    _usb_device_send_data(handle, 0, (uint_8_ptr)usb_dev_ptr->STATUS_PTR, sizeof(uint_16));
    
    /* status phase */
    _usb_device_recv_data(handle, 0, NULL, 0);
    
    return;
} /* Endbody */

void    mvUsbCh9ClearFeature(_usb_device_handle handle, boolean setup, 
                            SETUP_STRUCT* setup_ptr)
{ /* Body */
    uint_8   endpoint, direction;
    uint_16  usb_status;
     
    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_SETUP, "%s: setup=%d\n", __FUNCTION__, (int)setup);

    _usb_device_get_status(handle, ARC_USB_STATUS_DEVICE_STATE, &usb_status);
    if ((usb_status != ARC_USB_STATE_CONFIG) && (usb_status != ARC_USB_STATE_ADDRESS)) 
    {
        USB_printf("ClearFeature: Wrong USB state %d\n", usb_status);
        _usb_device_stall_endpoint(handle, 0, ARC_USB_RECV);
        return;
    } /* Endif */

    if(!setup)
        return;

    switch (setup_ptr->REQUESTTYPE) 
    {
        case (REQ_DIR_OUT | REQ_RECIP_DEVICE):
            /* DEVICE */
            switch(setup_ptr->VALUE)
            {
                case DEVICE_REMOTE_WAKEUP: 
                    /* clear remote wakeup */
                    _usb_device_get_status(handle, ARC_USB_STATUS_DEVICE, &usb_status);
                    usb_status &= ~ARC_USB_REMOTE_WAKEUP;
                    _usb_device_set_status(handle, ARC_USB_STATUS_DEVICE, usb_status);
                    USB_printf("Clear REMOTE_WAKEUP feature\n");
                    break;

                case DEVICE_TEST_MODE:
                    /* Exit Test Mode */
                    _usb_device_set_status(handle, ARC_USB_STATUS_TEST_MODE, 0);
                    break;

                default:
                    USB_printf("ClearFeature: Unknown Device feature %d\n", 
                                setup_ptr->VALUE);
                    _usb_device_stall_endpoint(handle, 0, ARC_USB_RECV);
                    return;
            } /* Endif */
            break;
         
        case (REQ_DIR_OUT | REQ_RECIP_ENDPOINT):
            /* ENDPOINT */
            if (setup_ptr->VALUE != ENDPOINT_HALT) 
            {
                USB_printf("ClearFeature: Wrong Endpoint feature %d\n", 
                            setup_ptr->VALUE);
                _usb_device_stall_endpoint(handle, 0, ARC_USB_RECV);
                return;
            } /* Endif */

            endpoint = setup_ptr->INDEX & ARC_USB_STATUS_ENDPOINT_NUMBER_MASK;
            if( (setup_ptr->INDEX & (1 << REQ_DIR_OFFSET)) == REQ_DIR_IN)
                direction = ARC_USB_SEND;
            else
                direction = ARC_USB_RECV;

            _usb_device_unstall_endpoint(handle, endpoint, direction);
            break;

        default:
            USB_printf("ClearFeature: Unknown REQUEST_TYPE %d\n", 
                                setup_ptr->REQUESTTYPE);

            _usb_device_stall_endpoint(handle, 0, ARC_USB_RECV);
            return;
    } /* Endswitch */
      
    /* status phase */
    _usb_device_send_data(handle, 0, 0, 0);
}

void    mvUsbCh9SetFeature(_usb_device_handle handle, boolean setup, 
                          SETUP_STRUCT* setup_ptr)
{
   uint_16                  usb_status;
   uint_8                   endpoint, direction;
   USB_DEV_STATE_STRUCT*    usb_dev_ptr = (USB_DEV_STATE_STRUCT*)handle; 

   ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_SETUP, "%s: setup=%d\n", __FUNCTION__, (int)setup);

   if (setup) 
   {
      switch (setup_ptr->REQUESTTYPE) 
      {
         case (REQ_DIR_OUT | REQ_RECIP_DEVICE):
            /* DEVICE */
            switch (setup_ptr->VALUE) 
            {
               case DEVICE_REMOTE_WAKEUP:
                  /* set remote wakeup */
                  _usb_device_get_status(handle, ARC_USB_STATUS_DEVICE, &usb_status);
                  usb_status |= ARC_USB_REMOTE_WAKEUP;
                  _usb_device_set_status(handle, ARC_USB_STATUS_DEVICE, usb_status);
                  USB_printf("Set REMOTE_WAKEUP feature\n");
                  break;

               case DEVICE_TEST_MODE:
                  /* Test Mode */
                  if( (setup_ptr->INDEX & 0x00FF) || (usb_dev_ptr->SPEED != ARC_USB_SPEED_HIGH) ) 
                  {
                     USB_printf("SetFeature: Wrong Test mode parameters: mode=%d, speed=%d\n", 
                                (setup_ptr->INDEX & 0x00FF), usb_dev_ptr->SPEED);
                     _usb_device_stall_endpoint(handle, 0, ARC_USB_RECV);
                     return;
                  } /* Endif */

                  _usb_device_get_status(handle, ARC_USB_STATUS_DEVICE_STATE, &usb_status);
                  if( (usb_status == ARC_USB_STATE_CONFIG)  || 
                      (usb_status == ARC_USB_STATE_ADDRESS) || 
                      (usb_status == ARC_USB_STATE_DEFAULT)) 
                  {
                      /* wait with Set Test mode */
                      ENTER_TEST_MODE = TRUE;
                      test_mode_index = (setup_ptr->INDEX & 0xFF00);
                      USB_printf("SetFeature: Prepare for Test mode 0x%x\n", test_mode_index);
                  } 
                  else 
                  {
                     USB_printf("SetFeature: Wrong USB state for Test mode: state=%d\n", 
                                usb_status);
                     _usb_device_stall_endpoint(handle, 0, ARC_USB_RECV);
                     return;
                  } /* Endif */
                  break;

               default:
                    USB_printf("SetFeature: Unknown Device feature %d\n", 
                                setup_ptr->VALUE);
                  _usb_device_stall_endpoint(handle, 0, ARC_USB_RECV);
                  return;
            } /* Endswitch */
            break;
            
         case (REQ_DIR_OUT | REQ_RECIP_ENDPOINT):
            /* ENDPOINT */
            if (setup_ptr->VALUE != ENDPOINT_HALT) 
            {
                USB_printf("SetFeature: Unknown Endpoint feature %d\n", 
                            setup_ptr->VALUE);
                _usb_device_stall_endpoint(handle, 0, ARC_USB_RECV);
                return;
            } /* Endif */

            endpoint = setup_ptr->INDEX & ARC_USB_STATUS_ENDPOINT_NUMBER_MASK;
            if( (setup_ptr->INDEX & (1 << REQ_DIR_OFFSET)) == REQ_DIR_IN)
                direction = ARC_USB_SEND;
            else
                direction = ARC_USB_RECV;

            _usb_device_stall_endpoint(handle, endpoint, direction);
            break;

         default:
            USB_printf("SetFeature: Unknown REQUEST_TYPE %d\n", 
                       setup_ptr->REQUESTTYPE);

            _usb_device_stall_endpoint(handle, 0, ARC_USB_RECV);
            return;
      } /* Endswitch */
      
      /* status phase */
      _usb_device_send_data(handle, 0, 0, 0);
   } 
   else 
   {
      if (ENTER_TEST_MODE) 
      {
         /* Enter Test Mode */
          USB_printf("SetFeature: Activate Test mode 0x%x\n", test_mode_index);
         _usb_device_set_status(handle, ARC_USB_STATUS_TEST_MODE, test_mode_index);
      } /* Endif */
   } /* Endif */
}

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : ch9SetAddress
* Returned Value : None
* Comments       :
*     Chapter 9 SetAddress command
*     We setup a TX packet of 0 length ready for the IN token
*     Once we get the TOK_DNE interrupt for the IN token, then
*     we change the ADDR register and go to the ADDRESS state.
* 
*END*--------------------------------------------------------------------*/
void    mvUsbCh9SetAddress(_usb_device_handle handle,
                        boolean setup, SETUP_STRUCT* setup_ptr)
{ /* Body */
   static uint_8            new_address;

   ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_ADDR, "usbDisk %s: setup=%d, address=%d\n", 
                                    __FUNCTION__, (int)setup, setup_ptr->VALUE);

   if (setup) 
   {
      new_address = setup_ptr->VALUE;
      /*******************************************************
       * if hardware assitance is enabled for set_address (see
       * hardware rev for details) we need to do the set_address
       * before queuing the status phase.
       *******************************************************/
#ifdef SET_ADDRESS_HARDWARE_ASSISTANCE      
       _usb_device_set_status(handle, ARC_USB_STATUS_ADDRESS, new_address);
#endif
      /* ack */
      _usb_device_send_data(handle, 0, 0, 0);
   } 
   else 
   {
#ifndef SET_ADDRESS_HARDWARE_ASSISTANCE
      _usb_device_set_status(handle, ARC_USB_STATUS_ADDRESS, new_address);
#endif
      _usb_device_set_status(handle, ARC_USB_STATUS_DEVICE_STATE, ARC_USB_STATE_ADDRESS);        
   }
}
