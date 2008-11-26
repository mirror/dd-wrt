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
*  Function Name  : _usb_device_recv_data
*  Returned Value : USB_OK or error code
*  Comments       :
*        Receives data on a specified endpoint.
*
*END*-----------------------------------------------------------------*/
uint_8 _usb_device_recv_data
   (
      /* [IN] the USB_USB_dev_initialize state structure */
      _usb_device_handle         handle,
            
      /* [IN] the Endpoint number */
      uint_8                     ep_num,
            
      /* [IN] buffer to receive data */
      uint_8_ptr                 buff_ptr,
            
      /* [IN] length of the transfer */
      uint_32                    size
   )
{ /* Body */
    int                              lockKey;
    uint_8                           error = USB_OK;
    XD_STRUCT_PTR                    xd_ptr;
    USB_DEV_STATE_STRUCT_PTR         usb_dev_ptr;
   
    usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;

    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_RX, "recv_data: ep=%d, buf_ptr=0x%x, size=%d\n",
                                       ep_num, (unsigned)buff_ptr, (int)size);

    ARC_DEBUG_CODE(ARC_DEBUG_FLAG_STATS, (usb_dev_ptr->STATS.usb_recv_count++));

    if(buff_ptr != NULL)
        USB_dcache_inv((pointer)buff_ptr,size);   
    
    lockKey = USB_lock();

    if (!usb_dev_ptr->XD_ENTRIES) 
    {
        USB_unlock(lockKey);
        USB_printf("_usb_device_recv_data, transfer in progress\n");
        return ARC_USB_STATUS_TRANSFER_IN_PROGRESS;
    } /* Endif */

    /* Get a transfer descriptor for the specified endpoint 
    ** and direction 
    */
    USB_XD_QGET(usb_dev_ptr->XD_HEAD, usb_dev_ptr->XD_TAIL, xd_ptr);
   
    usb_dev_ptr->XD_ENTRIES--;

    /* Initialize the new transfer descriptor */      
    xd_ptr->EP_NUM = ep_num;
    xd_ptr->BDIRECTION = ARC_USB_RECV;
    xd_ptr->WTOTALLENGTH = size;
    xd_ptr->WSOFAR = 0;
    xd_ptr->WSTARTADDRESS = buff_ptr;
   
    xd_ptr->BSTATUS = ARC_USB_STATUS_TRANSFER_ACCEPTED;

    error = _usb_dci_vusb20_add_dTD(handle, xd_ptr);

    USB_unlock(lockKey);
   
    if (error) 
    {
        USB_printf("_usb_device_recv_data, receive failed\n");
        return USBERR_RX_FAILED;
    } /* Endif */

    return error;

} /* EndBody */
