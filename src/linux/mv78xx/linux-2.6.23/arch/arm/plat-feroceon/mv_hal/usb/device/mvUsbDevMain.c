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
#include "ctrlEnv/mvCtrlEnvLib.h"
#include "mvUsbDevApi.h"
#include "mvUsbDevPrv.h"

USB_IMPORT_FUNCS*           global_import_funcs = NULL;

#ifdef USB_UNDERRUN_WA
USB_WA_FUNCS*               global_wa_funcs = NULL;
int                         global_wa_threshold = 64;
int                         global_wa_sram_parts = 2;
#endif /* USB_UNDERRUN_WA */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_device_cleanup
*  Returned Value : void
*  Comments       :
*        Cleanup allocated structures.
*
*END*-----------------------------------------------------------------*/

static void    _usb_device_cleanup(USB_DEV_STATE_STRUCT_PTR usb_dev_ptr)
{
    /* Free all internal transfer descriptors */
    if(usb_dev_ptr->XD_BASE != NULL)
    {
        USB_memfree((pointer)usb_dev_ptr->XD_BASE);
    }
   
    /* Free all XD scratch memory */
    if(usb_dev_ptr->XD_SCRATCH_STRUCT_BASE != NULL)
    {
        USB_memfree((pointer)usb_dev_ptr->XD_SCRATCH_STRUCT_BASE);
    }
    /* Free the temp ep init XD */
    if(usb_dev_ptr->TEMP_XD_PTR != NULL)
    {
        USB_memfree((pointer)usb_dev_ptr->TEMP_XD_PTR);
    }

    if(usb_dev_ptr->STATUS_UNAIGNED_PTR != NULL)
        USB_memfree((pointer)usb_dev_ptr->STATUS_UNAIGNED_PTR);

    if(usb_dev_ptr->TEST_PKT_UNAIGNED_PTR != NULL)
        USB_memfree((pointer)usb_dev_ptr->TEST_PKT_UNAIGNED_PTR);

    /* Free the USB state structure */
    USB_memfree((pointer)usb_dev_ptr);
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_device_free_XD
*  Returned Value : void
*  Comments       :
*        Enqueues a XD onto the free XD ring.
*
*END*-----------------------------------------------------------------*/

void _usb_device_free_XD
   (
      /* [IN] the dTD to enqueue */
      pointer  xd_ptr
   )
{ /* Body */
    int                         lockKey;
    USB_DEV_STATE_STRUCT_PTR    usb_dev_ptr;
  
    usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)(((XD_STRUCT_PTR)xd_ptr)->SCRATCH_PTR->PRIVATE);

    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_TRACE, "free_XD: xd_ptr=0x%x\n", (unsigned)xd_ptr);

    ARC_DEBUG_CODE(ARC_DEBUG_FLAG_STATS, (usb_dev_ptr->STATS.free_XD_count++));

   /*
   ** This function can be called from any context, and it needs mutual
   ** exclusion with itself.
   */

   lockKey = USB_lock();

   /*
   ** Add the XD to the free XD queue (linked via PRIVATE) and
   ** increment the tail to the next descriptor
   */
   USB_XD_QADD(usb_dev_ptr->XD_HEAD, usb_dev_ptr->XD_TAIL, (XD_STRUCT_PTR)xd_ptr);
   usb_dev_ptr->XD_ENTRIES++;

   USB_unlock(lockKey);

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_device_set_bsp_funcs
*  Returned Value : NONE
*  Comments       :
*        Set pointer to structure of imported BSP functions
*
*END*-----------------------------------------------------------------*/
void _usb_device_set_bsp_funcs(USB_IMPORT_FUNCS* pBspFuncs)
{
    static  boolean isFirst = TRUE;
    
    if(isFirst)
    {
        global_import_funcs = pBspFuncs;
       _usb_debug_init_trace_log();
       isFirst = FALSE;
    }
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_device_get_max_endpoint
*  Returned Value : handle or NULL
*  Comments       :
*        Return maximum number of endpoints supportedby USB device 
*        (for DEBUG only)
*
*END*-----------------------------------------------------------------*/
uint_8 _usb_device_get_max_endpoint(_usb_device_handle handle)
{
    USB_DEV_STATE_STRUCT_PTR     usb_dev_ptr;

    usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;

    return usb_dev_ptr->MAX_ENDPOINTS;
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_device_get_dev_num
*  Returned Value : handle or NULL
*  Comments       :
*        Return unique USB device number
*        (for DEBUG only)
*
*END*-----------------------------------------------------------------*/
uint_8  _usb_device_get_dev_num(_usb_device_handle handle)
{
    USB_DEV_STATE_STRUCT_PTR     usb_dev_ptr;

    usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;

    return usb_dev_ptr->DEV_NUM;
}


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_device_init
*  Returned Value : USB_OK or error code
*  Comments       :
*        Initializes the USB device specific data structures and calls 
*  the low-level device controller chip initialization routine.
*
*END*-----------------------------------------------------------------*/
uint_8 _usb_device_init
   (
      /* [IN] the USB device controller to initialize */
      uint_8                    devnum,

      /* [OUT] the USB_USB_dev_initialize state structure */
      _usb_device_handle*       handle
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR         usb_dev_ptr;
   XD_STRUCT_PTR                    xd_ptr;
   uint_8                           i, error;
   SCRATCH_STRUCT_PTR               temp_scratch_ptr;

   /* global_import_funcs must be initailized before */
   if(global_import_funcs == NULL)
       return USBERR_INIT_FAILED;

   if (devnum > MAX_USB_DEVICES)
   {
        USB_printf("_usb_device_init, error invalid device number");
        return USBERR_INVALID_DEVICE_NUM;
   } /* Endif */
   
   /* Allocate memory for the state structure */
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)USB_memalloc(sizeof(USB_DEV_STATE_STRUCT));      
   if (usb_dev_ptr == NULL) 
   {
        USB_printf("_usb_device_init, malloc of %d bytes for USB_DEV_STATE_STRUCT failed\n", 
                    sizeof(USB_DEV_STATE_STRUCT));
        return USBERR_ALLOC_STATE;
   } /* Endif */
   
   /* Zero out the internal USB state structure */
   USB_memzero(usb_dev_ptr, sizeof(USB_DEV_STATE_STRUCT));
    
   usb_dev_ptr->DEV_NUM = devnum;

   /* Multiple devices will have different base addresses and 
   ** interrupt vectors (For future)
   */   
   usb_dev_ptr->USB_STATE = ARC_USB_STATE_UNKNOWN;
   
   /* Allocate MAX_XDS_FOR_TR_CALLS */
   xd_ptr = (XD_STRUCT_PTR)USB_memalloc(sizeof(XD_STRUCT) * MAX_XDS_FOR_TR_CALLS);      
   if (xd_ptr == NULL) 
   {
        _usb_device_cleanup(usb_dev_ptr);
        USB_printf("_usb_device_init, malloc of %d bytes for %d XD_STRUCT failed\n", 
                        sizeof(XD_STRUCT) * MAX_XDS_FOR_TR_CALLS, MAX_XDS_FOR_TR_CALLS);
        return USBERR_ALLOC_TR;
   } /* Endif */
   
   usb_dev_ptr->XD_BASE = xd_ptr;

   _usb_clear_stats(usb_dev_ptr);

   USB_memzero(xd_ptr, sizeof(XD_STRUCT) * MAX_XDS_FOR_TR_CALLS);

   /* Allocate memory for internal scratch structure */   
   usb_dev_ptr->XD_SCRATCH_STRUCT_BASE = (SCRATCH_STRUCT_PTR)
                    USB_memalloc(sizeof(SCRATCH_STRUCT) * MAX_XDS_FOR_TR_CALLS);
   if (usb_dev_ptr->XD_SCRATCH_STRUCT_BASE == NULL) 
   {
        _usb_device_cleanup(usb_dev_ptr);
        USB_printf("_usb_device_init, malloc of %d bytes for %d XD_STRUCT failed\n", 
                        sizeof(SCRATCH_STRUCT) * MAX_XDS_FOR_TR_CALLS, MAX_XDS_FOR_TR_CALLS);
        return USBERR_ALLOC;
   } /* Endif */

   temp_scratch_ptr = usb_dev_ptr->XD_SCRATCH_STRUCT_BASE;
   usb_dev_ptr->XD_HEAD = NULL;
   usb_dev_ptr->XD_TAIL = NULL;
   usb_dev_ptr->XD_ENTRIES = 0;

   /* Enqueue all the XDs */   
   for (i=0;i<MAX_XDS_FOR_TR_CALLS;i++) 
   {
      xd_ptr->SCRATCH_PTR = temp_scratch_ptr;
      xd_ptr->SCRATCH_PTR->FREE = _usb_device_free_XD;
      xd_ptr->SCRATCH_PTR->PRIVATE = (pointer)usb_dev_ptr;
      _usb_device_free_XD((pointer)xd_ptr);
      xd_ptr++;
      temp_scratch_ptr++;
   } /* Endfor */

   usb_dev_ptr->TEMP_XD_PTR = (XD_STRUCT_PTR)USB_memalloc(sizeof(XD_STRUCT));
   if(usb_dev_ptr->TEMP_XD_PTR == NULL)
   {
        USB_printf("_usb_device_init, malloc of %d bytes for TEMP_XD_STRUCT failed\n", 
                        sizeof(XD_STRUCT));
        _usb_device_cleanup(usb_dev_ptr);
        return USBERR_ALLOC;
   }
   USB_memzero(usb_dev_ptr->TEMP_XD_PTR, sizeof(XD_STRUCT));

   /* Allocate 2 bytes for USB_STATUS to be sent over USB, so Cache line aligned */
   usb_dev_ptr->STATUS_UNAIGNED_PTR = (uint_8*)USB_memalloc(sizeof(uint_16) + PSP_CACHE_LINE_SIZE);
   if(usb_dev_ptr->STATUS_UNAIGNED_PTR == NULL)
   {
        USB_printf("_usb_device_init, malloc of %d bytes for USB_STATUS failed\n", 
                        sizeof(uint_16) + PSP_CACHE_LINE_SIZE);
        _usb_device_cleanup(usb_dev_ptr);
        return USBERR_ALLOC;
   }
   USB_memzero(usb_dev_ptr->STATUS_UNAIGNED_PTR, sizeof(uint_16) + PSP_CACHE_LINE_SIZE);
   usb_dev_ptr->STATUS_PTR = (uint_16*)USB_CACHE_ALIGN((uint_32)usb_dev_ptr->STATUS_UNAIGNED_PTR);

   /* Allocate 53 bytes for USB Test packet to be sent over USB, so Cache line aligned */
   usb_dev_ptr->TEST_PKT_UNAIGNED_PTR = (uint_8*)USB_memalloc(USB_TEST_MODE_TEST_PACKET_LENGTH + PSP_CACHE_LINE_SIZE);
   if(usb_dev_ptr->TEST_PKT_UNAIGNED_PTR == NULL)
   {
        USB_printf("_usb_device_init, malloc of %d bytes for USB Test packet failed\n", 
                        USB_TEST_MODE_TEST_PACKET_LENGTH + PSP_CACHE_LINE_SIZE);
        _usb_device_cleanup(usb_dev_ptr);
        return USBERR_ALLOC;
   }
   USB_memzero(usb_dev_ptr->TEST_PKT_UNAIGNED_PTR, USB_TEST_MODE_TEST_PACKET_LENGTH + PSP_CACHE_LINE_SIZE);
   usb_dev_ptr->TEST_PKT_PTR = (uint_8*)USB_CACHE_ALIGN((uint_32)usb_dev_ptr->TEST_PKT_UNAIGNED_PTR);

   /* Initialize the USB controller chip */
   error = _usb_dci_vusb20_init(devnum, usb_dev_ptr);
   if (error) 
   {
        _usb_device_cleanup(usb_dev_ptr);
        USB_printf("_usb_device_init, init failed");
        return USBERR_INIT_FAILED;
   } /* Endif */

   USB_printf("device_init: pDev=0x%x, pXD(%d)=0x%x, pSCRATCH(%d)=0x%x, pTempXD=0x%x\n",
                (unsigned)usb_dev_ptr, MAX_XDS_FOR_TR_CALLS, (unsigned)usb_dev_ptr->XD_BASE,
                MAX_XDS_FOR_TR_CALLS, (unsigned)usb_dev_ptr->XD_SCRATCH_STRUCT_BASE,
                (unsigned)usb_dev_ptr->TEMP_XD_PTR);

   *handle = usb_dev_ptr;
   return USB_OK;   
} /* EndBody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_device_shutdown
*  Returned Value : USB_OK or error code
*  Comments       :
*        Shutdown an initialized USB device
*
*END*-----------------------------------------------------------------*/
void _usb_device_shutdown(_usb_device_handle handle)
{ /* Body */
    USB_DEV_STATE_STRUCT_PTR     usb_dev_ptr;
    SERVICE_STRUCT_PTR           service_ptr;
    int                          ep;

    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_CTRL, "shutdown\n");
   
    usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;

    for(ep=0; ep<(usb_dev_ptr->MAX_ENDPOINTS); ep++)
    {
        /* Cancel all transfers on all endpoints */
        while(_usb_device_get_transfer_status(handle, ep, ARC_USB_RECV) != 
                                                    ARC_USB_STATUS_IDLE)
        {
            _usb_device_cancel_transfer(handle, ep, ARC_USB_RECV);
        }
        while(_usb_device_get_transfer_status(handle, ep, ARC_USB_SEND) != 
                                                    ARC_USB_STATUS_IDLE)
        {
            _usb_device_cancel_transfer(handle, ep, ARC_USB_SEND);
        }
    }
    _usb_dci_vusb20_shutdown(usb_dev_ptr);
   
    /* Free all the Callback function structure memory */
    for( service_ptr = usb_dev_ptr->SERVICE_HEAD_PTR; service_ptr;
         service_ptr = service_ptr->NEXT) 
    {
        USB_printf("_usb_device_shutdown: free service_ptr = 0x%x\n", 
                            service_ptr);
        USB_memfree(service_ptr);
    }
    usb_dev_ptr->SERVICE_HEAD_PTR = NULL;

    _usb_device_cleanup(usb_dev_ptr);
} /* EndBody */


/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : _usb_device_register_service
* Returned Value : USB_OK or error code
* Comments       :
*     Registers a callback routine for a specified event or endpoint.
* 
*END*--------------------------------------------------------------------*/
uint_8 _usb_device_register_service
   (
      /* [IN] Handle to the USB device */
      _usb_device_handle         handle,
      
      /* [IN] type of event or endpoint number to service */
      uint_8                     type,
      
      /* [IN] Pointer to the service's callback function */
      void(_CODE_PTR_ service)(pointer, uint_8, boolean, uint_8, uint_8_ptr, uint_32, uint_8)
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR   usb_dev_ptr;
   SERVICE_STRUCT_PTR         service_ptr;
   SERVICE_STRUCT_PTR _PTR_   search_ptr;
   int                        lockKey;
 
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
   /* Needs mutual exclusion */
   lockKey = USB_lock();
   
   /* Search for an existing entry for type */
   for (search_ptr = &usb_dev_ptr->SERVICE_HEAD_PTR;
      *search_ptr;
      search_ptr = &(*search_ptr)->NEXT) 
   {
      if ((*search_ptr)->TYPE == type) 
      {
         /* Found an existing entry */
         USB_unlock(lockKey);
         USB_printf("_usb_device_register_service, service %d already opened\n");
         return USBERR_OPEN_SERVICE;
      } /* Endif */
   } /* Endfor */
   
   /* No existing entry found - create a new one */
   service_ptr = (SERVICE_STRUCT_PTR)USB_memalloc(sizeof(SERVICE_STRUCT));
   if (!service_ptr) 
   {
      USB_unlock(lockKey);
      USB_printf("_usb_device_register_service, malloc for %d bytes failed\n", 
                    sizeof(SERVICE_STRUCT));
      return USBERR_ALLOC;
   } /* Endif */

   service_ptr->TYPE = type;
   service_ptr->SERVICE = service;
   service_ptr->NEXT = NULL;
   *search_ptr = service_ptr;
   
   USB_unlock(lockKey);

   return USB_OK;
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : _usb_device_unregister_service
* Returned Value : USB_OK or error code
* Comments       :
*     Unregisters a callback routine for a specified event or endpoint.
* 
*END*--------------------------------------------------------------------*/
uint_8 _usb_device_unregister_service
   (
      /* [IN] Handle to the USB device */
      _usb_device_handle         handle,

      /* [IN] type of event or endpoint number to service */
      uint_8                     type
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR   usb_dev_ptr;
   SERVICE_STRUCT_PTR         service_ptr;
   SERVICE_STRUCT_PTR _PTR_   search_ptr;
   int                        lockKey;
 
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
   /* Needs mutual exclusion */
   lockKey = USB_lock();
   
   /* Search for an existing entry for type */
   for (search_ptr = &usb_dev_ptr->SERVICE_HEAD_PTR;
      *search_ptr;
      search_ptr = &(*search_ptr)->NEXT) 
   {
      if ((*search_ptr)->TYPE == type) {
         /* Found an existing entry - delete it */
         break;
      } /* Endif */
   } /* Endfor */
   
   /* No existing entry found */
   if (!*search_ptr) 
   {
      USB_unlock(lockKey);
      USB_printf("_usb_device_unregister_service, no service found\n");
      return USBERR_CLOSED_SERVICE;
   } /* Endif */
   
   service_ptr = *search_ptr;
   *search_ptr = service_ptr->NEXT;

   USB_memfree((pointer)service_ptr);
   
   USB_unlock(lockKey);

   return USB_OK;

} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : _usb_device_call_service
* Returned Value : USB_OK or error code
* Comments       :
*     Calls the appropriate service for the specified type, if one is
*     registered. Used internally only.
* 
*END*--------------------------------------------------------------------*/
uint_8 _usb_device_call_service
   (
      /* [IN] Handle to the USB device */
      _usb_device_handle   handle,

      /* [OUT] Type of service or endpoint */
      uint_8               type,
      
      /* [OUT] Is it a Setup transfer? */
      boolean              setup,
      
      /* [OUT] Direction of transmission; is it a Transmit? */
      boolean              direction,

      /* [OUT] Pointer to the data */
      uint_8_ptr           buffer_ptr,
      
      /* [OUT] Number of bytes in transmission */
      uint_32              length,

      /* [OUT] Any errors */
      uint_8               errors
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR     usb_dev_ptr;
   SERVICE_STRUCT _PTR_         service_ptr;
   int                          lockKey;

   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
   /* Needs mutual exclusion */
   lockKey = USB_lock();
   
   /* Search for an existing entry for type */
   for (service_ptr = usb_dev_ptr->SERVICE_HEAD_PTR;
        service_ptr;
        service_ptr = service_ptr->NEXT) 
   {
      if (service_ptr->TYPE == type) 
      {
         service_ptr->SERVICE(handle, type, setup, direction, buffer_ptr, length, errors);
         USB_unlock(lockKey);

         return USB_OK;
      } /* Endif */
      
   } /* Endfor */

   USB_unlock(lockKey);

   ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_CTRL, "_usb_device_call_service, service %d is closed\n", type);

   return USBERR_CLOSED_SERVICE;
} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_device_init_endpoint
*  Returned Value : USB_OK or error code
*  Comments       :
*     Initializes the endpoint and the data structures associated with the 
*  endpoint
*
*END*-----------------------------------------------------------------*/
uint_8 _usb_device_init_endpoint
   (
      /* [IN] the USB_USB_dev_initialize state structure */
      _usb_device_handle         handle,
            
      /* [IN] the Endpoint number */
      uint_8                     ep_num,
            
      /* [IN] MAX Packet size for this endpoint */
      uint_16                    max_pkt_size,
            
      /* [IN] Direction */
      uint_8                     direction,
            
      /* [IN] Type of Endpoint */
      uint_8                     type,
            
      /* [IN] After all data is transfered, should we terminate the transfer 
      ** with a zero length packet if the last packet size == MAX_PACKET_SIZE? 
      */
      uint_8                     flag   
   )
{ /* Body */

    int                         lockKey;
    uint_8                      error = 0;
    USB_DEV_STATE_STRUCT_PTR    usb_dev_ptr;
   
    usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;

    /* Initialize the transfer descriptor */
    usb_dev_ptr->TEMP_XD_PTR->EP_NUM = ep_num;
    usb_dev_ptr->TEMP_XD_PTR->BDIRECTION = direction;
    usb_dev_ptr->TEMP_XD_PTR->WMAXPACKETSIZE = max_pkt_size;
    usb_dev_ptr->TEMP_XD_PTR->EP_TYPE = type;
    usb_dev_ptr->TEMP_XD_PTR->DONT_ZERO_TERMINATE = flag;
    usb_dev_ptr->TEMP_XD_PTR->MAX_PKTS_PER_UFRAME = 
                    ((flag & ARC_USB_MAX_PKTS_PER_UFRAME) >> 1);

    lockKey = USB_lock();
    error = _usb_dci_vusb20_init_endpoint(handle, usb_dev_ptr->TEMP_XD_PTR);
    USB_unlock(lockKey);
   
   return error;

} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_device_deinit_endpoint
*  Returned Value : USB_OK or error code
*  Comments       :
*  Disables the endpoint and the data structures associated with the 
*  endpoint
*
*END*-----------------------------------------------------------------*/
uint_8 _usb_device_deinit_endpoint
   (
      /* [IN] the USB_USB_dev_initialize state structure */
      _usb_device_handle         handle,
            
      /* [IN] the Endpoint number */
      uint_8                    ep_num,
            
      /* [IN] Direction */
      uint_8                    direction
   )
{ /* Body */
   int                          lockKey;
   uint_8                       error = 0;
   lockKey = USB_lock();

   error = _usb_dci_vusb20_deinit_endpoint(handle, ep_num, direction);
   
   USB_unlock(lockKey);
   
   return error;
} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_device_get_transfer_status
*  Returned Value : Status of the transfer
*  Comments       :
*        returns the status of the transaction on the specified endpoint.
*
*END*-----------------------------------------------------------------*/
uint_8 _usb_device_get_transfer_status
   (
      /* [IN] the USB_USB_dev_initialize state structure */
      _usb_device_handle         handle,
            
      /* [IN] the Endpoint number */
      uint_8                     ep_num,
            
      /* [IN] direction */
      uint_8                     direction
   )
{ /* Body */
   uint_8                       status;
   int                          lockKey;

   lockKey = USB_lock();

   status = _usb_dci_vusb20_get_transfer_status(handle, ep_num, direction);

   USB_unlock(lockKey);

   /* Return the status of the last queued transfer */
   return (status);

} /* EndBody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_device_read_setup_data
*  Returned Value : USB_OK or error code
*  Comments       :
*        Reads the setup data from the hardware
*
*END*-----------------------------------------------------------------*/
void _usb_device_read_setup_data
   (
      /* [IN] the USB_USB_dev_initialize state structure */
      _usb_device_handle         handle,
            
      /* [IN] the Endpoint number */
      uint_8                     ep_num,
            
      /* [IN] buffer for receiving Setup packet */
      uint_8_ptr                  buff_ptr
   )
{ /* Body */
   int                           lockKey;
 
   lockKey = USB_lock();

   _usb_dci_vusb20_get_setup_data(handle, ep_num, buff_ptr);

   USB_unlock(lockKey);

} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_device_cancel_transfer
*  Returned Value : USB_OK or error code
*  Comments       :
*        returns the status of the transaction on the specified endpoint.
*
*END*-----------------------------------------------------------------*/
uint_8 _usb_device_cancel_transfer
   (
      /* [IN] the USB_USB_dev_initialize state structure */
      _usb_device_handle         handle,
            
      /* [IN] the Endpoint number */
      uint_8                     ep_num,
            
      /* [IN] direction */
      uint_8                     direction
   )
{ /* Body */
   uint_8                        error = USB_OK;
   int                           lockKey;
   
   lockKey = USB_lock();

   /* Cancel transfer on the specified endpoint for the specified 
   ** direction 
   */
   error = _usb_dci_vusb20_cancel_transfer(handle, ep_num, direction);

   USB_unlock(lockKey);

   return error;
} /* EndBody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_device_stop
*  Returned Value : None
*  Comments       :
*        Stop USB device
*
*END*-----------------------------------------------------------------*/
void _usb_device_stop(_usb_device_handle handle)
{
    int         lockKey;

    lockKey = USB_lock();
    _usb_dci_vusb20_stop(handle);
    USB_unlock(lockKey);
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_device_start
*  Returned Value : None
*  Comments       :
*        Start USB device
*
*END*-----------------------------------------------------------------*/
void _usb_device_start(_usb_device_handle handle)
{
    int         lockKey;

    lockKey = USB_lock();
    _usb_dci_vusb20_start(handle);
    USB_unlock(lockKey);
}
