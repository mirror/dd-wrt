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

#if defined(USB_UNDERRUN_WA)

typedef struct
{
    uint_8*     buff_ptr[MAX_XDS_FOR_TR_CALLS];
    uint_32     size[MAX_XDS_FOR_TR_CALLS];
    uint_8      ep_num[MAX_XDS_FOR_TR_CALLS];
    int         head;
    int         tail;
    int         tail_dma;
    int         num;
    int         num_dma;

} USB_SEND_QUEUE;

uint_8*         usbSramBase;
int             usbSramSize;
int		        usbSramPartSize;
USB_SEND_QUEUE  usbSendQueue;

uint_32         usbSentSize = 0;
uint_32         usbDmaSize = 0;

#define S_FREE	    0
#define S_BUSY	    1

uint_32		    dma_index = 0;
uint_32		    sent_index = 0;
uint_32		    sram_parts[USB_SRAM_MAX_PARTS];


void    _usb_reset_send_queue(void)
{
    int     i;

    usbSendQueue.num = 0;
	usbSendQueue.num_dma = 0;
    usbSendQueue.head = 0;
    usbSendQueue.tail = 0;
	usbSendQueue.tail_dma = 0;
    for(i=0; i<MAX_XDS_FOR_TR_CALLS; i++)
    {
        usbSendQueue.size[i] = 0;
        usbSendQueue.buff_ptr[i] = NULL;
        usbSendQueue.ep_num[i] = 0;
    }        
	usbSramPartSize = usbSramSize/global_wa_sram_parts;

    for(i=0; i<global_wa_sram_parts; i++)
    {
		sram_parts[i] = S_FREE;
    }
}

uint_8 _usb_prepare_to_send(void*   handle)
{
    XD_STRUCT_PTR               xd_ptr;
    USB_DEV_STATE_STRUCT_PTR    usb_dev_ptr;
    uint_8*                     buff_ptr;
    uint_8*			            tmp_buff;
    uint_32                     size;
    int                         num_dma, tail_dma, i;
    uint_8			            error = 0;

    usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;

    tail_dma = usbSendQueue.tail_dma;
    num_dma = usbSendQueue.num_dma;
    buff_ptr = usbSendQueue.buff_ptr[tail_dma];
    size = usbSendQueue.size[tail_dma];

    if(num_dma == 0)
	    return 0;

/*
    USB_printf("_usb_prepare_to_send: num=%d, tail=%d, sentSize=%d, size=%d, buff=%p\n", 
                num_dma, tail_dma, usbSentSize, size, buff_ptr);
*/
    for(i=0; i<global_wa_sram_parts; i++)
    {
	    if(sram_parts[dma_index] != S_FREE)
	        break;
	
	    if(usbDmaSize >= usbSendQueue.size[tail_dma])
	    {
	        /* Remove from the usbSendQueues */
	        num_dma--;
            tail_dma++;
            if(tail_dma == MAX_XDS_FOR_TR_CALLS)
                tail_dma = 0;

            usbSendQueue.tail_dma = tail_dma;
            usbSendQueue.num_dma = num_dma;
            usbDmaSize = 0;

            if(num_dma == 0)
                break;
        }

	    buff_ptr = usbSendQueue.buff_ptr[tail_dma] + usbDmaSize;
	    size = MIN(usbSramPartSize, (usbSendQueue.size[tail_dma] - usbDmaSize) ); 

	    usbDmaSize += size;

	    if(size > global_wa_threshold)
	    {
	        tmp_buff = buff_ptr;
	        buff_ptr = (uint_8*)((int)usbSramBase + (dma_index * usbSramPartSize));
	        USB_idma_copy(buff_ptr, tmp_buff, size);

	        sram_parts[dma_index] = S_BUSY;
            dma_index++;
            if(dma_index == global_wa_sram_parts)
                dma_index = 0;
	    }
        

	    /* Get a transfer descriptor */
	    USB_XD_QGET(usb_dev_ptr->XD_HEAD, usb_dev_ptr->XD_TAIL, xd_ptr);

	    usb_dev_ptr->XD_ENTRIES--;
	    USB_dcache_flush((pointer)buff_ptr, size);   

	    /* Initialize the new transfer descriptor */      
	    xd_ptr->EP_NUM = usbSendQueue.ep_num[tail_dma];
	    xd_ptr->BDIRECTION = ARC_USB_SEND;
	    xd_ptr->WTOTALLENGTH = size;
	    xd_ptr->WSOFAR = 0;
	    xd_ptr->WSTARTADDRESS = buff_ptr;   
	    xd_ptr->BSTATUS = ARC_USB_STATUS_TRANSFER_ACCEPTED;

	    error = _usb_dci_vusb20_add_dTD(handle, xd_ptr);

	    if(error)
	        break;
    }

    return error;    
}


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : usbSendComplete
*  Returned Value : None
*  Comments       :
*        Callback for send transfer complete event.
*
*END*-----------------------------------------------------------------*/
void    usbSendComplete(void* handle, uint_8 type, boolean setup, uint_8 dir, 
                        uint_8_ptr buffer, uint_32 length, uint_8 error)
{
    /* Check if this complete is one from the sendQueue */
    if( (usbSendQueue.ep_num[usbSendQueue.tail] == type) &&
        (usbSendQueue.num > 0) )
    {
        USB_DEV_STATE_STRUCT_PTR    usb_dev_ptr;
        uint_8*                     buff_ptr;
        uint_32                     size;
        int                         num, tail;

        usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;

        tail = usbSendQueue.tail;
        num = usbSendQueue.num;
        buff_ptr = usbSendQueue.buff_ptr[tail];
        size = usbSendQueue.size[tail];
/*
        USB_printf("usbSendComplete: num=%d, tail=%d, usbSentSize=%d, type=%d, length=%d (%d), buff=%p (%p)\n", 
                num, tail, usbSentSize, type, length, usbSendQueue.size[tail], 
                buffer, usbSendQueue.buff_ptr[tail]);
*/
        usbSentSize += length;

	    /* if the buffer was on the SRAM */
	    if( ((unsigned)buffer >= (unsigned)usbSramBase) &&
	    ((unsigned)buffer < ((unsigned)usbSramBase + (usbSramPartSize * global_wa_sram_parts))) )
	    {
	        sram_parts[sent_index] = S_FREE;
            sent_index++;
            if(sent_index == global_wa_sram_parts)
                sent_index = 0;
	    }

        if(usbSentSize >= usbSendQueue.size[tail])
        {
            /* Remove from the usbSendQueues */
            num--;
            tail++;
            if(tail == MAX_XDS_FOR_TR_CALLS)
                tail = 0;

            usbSendQueue.tail = tail;
            usbSendQueue.num = num;
            usbSentSize = 0;

            /* Call complete callback */
            _usb_device_call_service(handle, type, setup, dir, 
                         buff_ptr, size, error);

            if(num == 0)
                return;
        }

	    error = _usb_prepare_to_send(handle);	
        if (error) 
        {
            USB_printf("usbSendComplete, add_dTD failed\n");
        }	

    }
    else
    {
        /* Call complete callback */
        _usb_device_call_service(handle, type, setup, dir, 
                        buffer, length, error);
    }
}
#endif /* USB_UNDERRUN_WA */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_device_send_data
*  Returned Value : USB_OK or error code
*  Comments       :
*        Sends data on a specified endpoint.
*
*END*-----------------------------------------------------------------*/
uint_8 _usb_device_send_data
   (
      /* [IN] the USB_USB_dev_initialize state structure */
      _usb_device_handle         handle,
            
      /* [IN] the Endpoint number */
      uint_8                     ep_num,
            
      /* [IN] buffer to send */
      uint_8_ptr                 buff_ptr,
            
      /* [IN] length of the transfer */
      uint_32                    size
   )
{ /* Body */
   int 	                        lockKey;
   uint_8                       error = 0;
   XD_STRUCT_PTR                xd_ptr;
   USB_DEV_STATE_STRUCT_PTR     usb_dev_ptr;
   boolean                      toSend = TRUE;

   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;

   ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_TX,
       "send_data: handle=%p, ep=%d, pBuf=0x%x, size=%d, EP_QH=%p\n", 
       handle, ep_num, (unsigned)buff_ptr, (int)size, usb_dev_ptr->EP_QUEUE_HEAD_PTR);

   ARC_DEBUG_CODE(ARC_DEBUG_FLAG_STATS, (usb_dev_ptr->STATS.usb_send_count++));
      
   lockKey = USB_lock();

   if (!usb_dev_ptr->XD_ENTRIES) 
   {
      USB_unlock(lockKey);
      USB_printf("_usb_device_send_data, transfer in progress\n");
      return ARC_USB_STATUS_TRANSFER_IN_PROGRESS;
   } /* Endif */

#if defined(USB_UNDERRUN_WA)
    {
        int 			                head;
       	VUSB20_EP_QUEUE_HEAD_STRUCT* 	ep_queue_head_ptr;

		ep_queue_head_ptr = (VUSB20_EP_QUEUE_HEAD_STRUCT_PTR)usb_dev_ptr->EP_QUEUE_HEAD_PTR + 
                       						                  2*ep_num + ARC_USB_SEND;

        if( ((ep_queue_head_ptr->MAX_PKT_LENGTH >> 16) & 0x7FF) > global_wa_threshold) 
        {
            /* Only Endpoints with maxPktSize more than 128 bytes need special processing */
            if( (size > global_wa_threshold) || 
                (usbSendQueue.num != 0) )
            {
/*
                USB_printf("_usb_device_send_data: ep_num=%d, maxPktSize=%d, size=%d\n",
                        ep_num, (ep_queue_head_ptr->MAX_PKT_LENGTH >> 16) & 0x7FF, size);
*/
                /* Check if usbSendQueue is not Full */
                if(usbSendQueue.num == MAX_XDS_FOR_TR_CALLS)
                {
                    USB_printf("ep=%d: usbSendQueue is FULL\n", ep_num);
                    USB_unlock(lockKey);
                    return USBERR_TX_FAILED;
                }

                /* Add to usbSendQueu */
                head = usbSendQueue.head;

                usbSendQueue.num++;
		        usbSendQueue.num_dma++;
                usbSendQueue.size[head] = size;
                usbSendQueue.buff_ptr[head] = buff_ptr;
                usbSendQueue.ep_num[head] = ep_num;

                head++;
                if(head == MAX_XDS_FOR_TR_CALLS)
                    head = 0;

                usbSendQueue.head = head;

                /* Process first usbSendQueue element if possible */
                if(usbSendQueue.num == 1)
                {
		            error = _usb_prepare_to_send(handle);
		        }
		        toSend = FALSE;
            }
        }
    }
#endif /* USB_UNDERRUN_WA */

    if(toSend == TRUE)
    {
        /* Get a transfer descriptor */
        USB_XD_QGET(usb_dev_ptr->XD_HEAD, usb_dev_ptr->XD_TAIL, xd_ptr);

        usb_dev_ptr->XD_ENTRIES--;

        if(buff_ptr != NULL)
            USB_dcache_flush((pointer)buff_ptr, size);   

        /* Initialize the new transfer descriptor */      
        xd_ptr->EP_NUM = ep_num;
        xd_ptr->BDIRECTION = ARC_USB_SEND;
        xd_ptr->WTOTALLENGTH = size;
        xd_ptr->WSOFAR = 0;
        xd_ptr->WSTARTADDRESS = buff_ptr;   
        xd_ptr->BSTATUS = ARC_USB_STATUS_TRANSFER_ACCEPTED;

        error = _usb_dci_vusb20_add_dTD(handle, xd_ptr);
    }
    USB_unlock(lockKey);
   
    if (error) 
    {
        USB_printf("_usb_device_send_data, transfer failed\n");
        return USBERR_TX_FAILED;
    } /* Endif */
    return error;

} /* EndBody */

