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
*  Function Name  : _usb_device_unstall_endpoint
*  Returned Value : USB_OK or error code
*  Comments       :
*     Unstalls the endpoint in specified direction
*
*END*-----------------------------------------------------------------*/
void _usb_device_unstall_endpoint
   (
      /* [IN] the USB_USB_dev_initialize state structure */
      _usb_device_handle         handle,
            
      /* [IN] the Endpoint number */
      uint_8                     ep_num,
            
      /* [IN] direction */
      uint_8                     direction
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR   	usb_dev_ptr;
   int							lockKey;
   
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
 
   lockKey = USB_lock();
   
   _usb_dci_vusb20_unstall_endpoint(handle, ep_num, direction);

   USB_unlock(lockKey);
  
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : _usb_device_get_status
* Returned Value : USB_OK or error code
* Comments       :
*     Provides API to access the USB internal state.
* 
*END*--------------------------------------------------------------------*/
uint_8 _usb_device_get_status
   (
      /* [IN] Handle to the USB device */
      _usb_device_handle   handle,
      
      /* [IN] What to get the status of */
      uint_8               component,
      
      /* [OUT] The requested status */
      uint_16_ptr          status
   )
{ /* Body */
   	USB_DEV_STATE_STRUCT_PTR 	usb_dev_ptr;
	int							lockKey;

   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
  
   lockKey = USB_lock();

   switch (component) 
   {
      case ARC_USB_STATUS_DEVICE_STATE:
         *status = usb_dev_ptr->USB_STATE;
         break;

      case ARC_USB_STATUS_DEVICE:
         *status = usb_dev_ptr->USB_DEVICE_STATE;
         break;

      case ARC_USB_STATUS_INTERFACE:
          *status = 0;
         break;
         
      case ARC_USB_STATUS_ADDRESS:
         *status = usb_dev_ptr->DEVICE_ADDRESS;
         break;
         
      case ARC_USB_STATUS_CURRENT_CONFIG:
         *status = usb_dev_ptr->USB_CURR_CONFIG;
         break;

      case ARC_USB_STATUS_SOF_COUNT:
         *status = usb_dev_ptr->USB_SOF_COUNT;
         break;
   
      default:
            USB_unlock(lockKey);
            USB_printf("_usb_device_get_status, bad status\n");
            return USBERR_BAD_STATUS;
  
   } /* Endswitch */
   USB_unlock(lockKey);
  
   return USB_OK;   
} /* EndBody */  
 
/*FUNCTION*----------------------------------------------------------------
* 
* Function Name  : _usb_device_set_status
* Returned Value : USB_OK or error code
* Comments       :
*     Provides API to set internal state
* 
*END*--------------------------------------------------------------------*/
uint_8 _usb_device_set_status
   (
      /* [IN] Handle to the usb device */
      _usb_device_handle   handle,
      
      /* [IN] What to set the status of */
      uint_8               component,
      
      /* [IN] What to set the status to */
      uint_16              setting
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR usb_dev_ptr;
   int 					    lockKey;

   ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_STATUS, 
            "set_status: component=0x%x, value=0x%x\n", component, setting);
   
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
   lockKey = USB_lock();

   switch (component) 
   {
      case ARC_USB_STATUS_DEVICE_STATE:
         usb_dev_ptr->USB_STATE = setting;
         break;

      case ARC_USB_STATUS_DEVICE:
         usb_dev_ptr->USB_DEVICE_STATE = setting;
         break;

      case ARC_USB_STATUS_INTERFACE:
         break;

      case ARC_USB_STATUS_CURRENT_CONFIG:
         usb_dev_ptr->USB_CURR_CONFIG = setting;
         break;

      case ARC_USB_STATUS_SOF_COUNT:
         usb_dev_ptr->USB_SOF_COUNT = setting;
         break;

      case ARC_USB_FORCE_FULL_SPEED:
         _usb_dci_vusb20_set_speed_full((pointer)usb_dev_ptr, setting);
         break;

      case ARC_USB_PHY_LOW_POWER_SUSPEND:
         _usb_dci_vusb20_suspend_phy((pointer)usb_dev_ptr, setting);
         break;

      case ARC_USB_STATUS_ADDRESS:
         usb_dev_ptr->DEVICE_ADDRESS = setting;

         _usb_dci_vusb20_set_address((pointer)usb_dev_ptr, setting);
         break;
      
      case ARC_USB_STATUS_TEST_MODE:
         _usb_dci_vusb20_set_test_mode(handle, setting);
         break;
         
      default:
            USB_unlock(lockKey);
            USB_printf("_usb_device_set_status, bad status\n");
            return USBERR_BAD_STATUS;
  
   } /* Endswitch */

   USB_unlock(lockKey);
   
   return USB_OK;   
} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_device_stall_endpoint
*  Returned Value : USB_OK or error code
*  Comments       :
*     Stalls the endpoint.
*
*END*-----------------------------------------------------------------*/
void _usb_device_stall_endpoint
   (
      /* [IN] the USB_USB_dev_initialize state structure */
      _usb_device_handle         handle,
            
      /* [IN] the Endpoint number */
      uint_8                     ep_num,
            
      /* [IN] direction */
      uint_8                     direction
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR     usb_dev_ptr;
   int							lockKey;

   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;

   lockKey = USB_lock();

   _usb_dci_vusb20_stall_endpoint(handle, ep_num, direction);

   USB_unlock(lockKey);
   
} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_device_is_endpoint_stalled
*  Returned Value : USB_OK or error code
*  Comments       :
*     Stalls the endpoint.
*
*END*-----------------------------------------------------------------*/
uint_8 _usb_device_is_endpoint_stalled
   (
      /* [IN] the USB_USB_dev_initialize state structure */
      _usb_device_handle         handle,
            
      /* [IN] the Endpoint number */
      uint_8                     ep_num,
            
      /* [IN] direction */
      uint_8                     direction
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR     usb_dev_ptr;
   uint_8                       val;
   int							lockKey;

   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;

   lockKey = USB_lock();

   val = _usb_dci_vusb20_is_endpoint_stalled(handle, ep_num, direction);

   USB_unlock(lockKey);

   return val;
   
} /* EndBody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_device_process_resume
*  Returned Value : USB_OK or error code
*  Comments       :
*        Process Resume event
*
*END*-----------------------------------------------------------------*/
void _usb_device_assert_resume
   (
      /* [IN] the USB_USB_dev_initialize state structure */
      _usb_device_handle         handle
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR      usb_dev_ptr;
   
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;

   _usb_dci_vusb20_assert_resume(handle);

} /* EndBody */


/***************************/
/* ARC USB Debug functions */
/***************************/
void _usb_status(void* usbHandle)
{
    USB_DEV_STATE_STRUCT*   pUsbDev = (USB_DEV_STATE_STRUCT*)usbHandle;

    if(pUsbDev == NULL)
    {
        USB_printf("USB Device core is not initialized\n");
        return;
    }

    USB_printf("\n\tUSB Status\n\n");

    USB_printf("DEV_NUM=%d, DEV_ADDR=%d, CAP_REGS=0x%x, DEV_REGS=0x%x, MAX_EP=%d\n",
                pUsbDev->DEV_NUM,
                pUsbDev->DEVICE_ADDRESS,
                (unsigned)pUsbDev->CAP_REGS_PTR, 
                (unsigned)pUsbDev->DEV_PTR, 
                pUsbDev->MAX_ENDPOINTS);

    USB_printf("BUS_RESET=%s, USB_STATE=0x%02x, USB_DEV_STATE=0x%02x, SPEED=%d, ERRORS=0x%04x\n",
                pUsbDev->BUS_RESETTING ? "Yes" : "No",
                pUsbDev->USB_STATE,
                pUsbDev->USB_DEVICE_STATE,
                pUsbDev->SPEED, 
                pUsbDev->ERRORS);

    USB_printf("EP_QUEUE_HEAD: SIZE=%d, BASE=%p (0x%08x), ALIGNED=%p, SERVICE_HEAD=%p\n",
                pUsbDev->EP_QUEUE_HEAD_SIZE,
                pUsbDev->EP_QUEUE_HEAD_BASE, 
                pUsbDev->EP_QUEUE_HEAD_PHYS,
                pUsbDev->EP_QUEUE_HEAD_PTR,
                pUsbDev->SERVICE_HEAD_PTR);

    USB_printf("XD: BASE=%p, HEAD=%p, TAIL=%p, ENTRIES=%d, SCRATCH=%p, TEMP=%p\n",
                pUsbDev->XD_BASE,
                pUsbDev->XD_HEAD,
                pUsbDev->XD_TAIL,
                pUsbDev->XD_ENTRIES,
                pUsbDev->XD_SCRATCH_STRUCT_BASE,
                pUsbDev->TEMP_XD_PTR);

    USB_printf("DTD: SIZE=%d, BASE=%p (0x%08x), ALIGNED=%p, HEAD=0x%08x, TAIL=0x%08x, ENTRIES=%d, SCRATCH=%p\n",
                pUsbDev->DTD_SIZE,
                pUsbDev->DTD_BASE_PTR,
                pUsbDev->DTD_BASE_PHYS,
                pUsbDev->DTD_ALIGNED_BASE_PTR,
                pUsbDev->DTD_HEAD,
                pUsbDev->DTD_TAIL,
                pUsbDev->DTD_ENTRIES,
                pUsbDev->SCRATCH_STRUCT_BASE);
}

void _usb_stats(void* usbHandle)
{
    USB_DEV_STATE_STRUCT*   pUsbDev = (USB_DEV_STATE_STRUCT*)usbHandle;
    USB_STATS*              pUsbStats = &pUsbDev->STATS;
    int                     i;

    USB_printf("\n\tUSB Statistics\n\n");

    USB_printf("isr=%u, empty_isr=%u, reset=%u, setup=%u, read_setup=%u\n", 
                    pUsbStats->usb_isr_count, pUsbStats->usb_empty_isr_count, 
                    pUsbStats->usb_reset_count, pUsbStats->usb_setup_count, 
                    pUsbStats->usb_read_setup_count);

    USB_printf("recv=%u, send=%u, add=%u (%u), cancel=%u\n", 
                pUsbStats->usb_recv_count, pUsbStats->usb_send_count, 
                pUsbStats->usb_add_count, pUsbStats->usb_add_not_empty_count, 
                pUsbStats->usb_cancel_count);

    USB_printf("free_XD=%u, free_dTD=%u\n", 
            pUsbStats->free_XD_count, pUsbStats->free_dTD_count);

    USB_printf("complete_isr=%u, complete=%u, empty_complete=%u, max_complete=%u\n", 
                pUsbStats->usb_complete_isr_count, pUsbStats->usb_complete_count, 
                pUsbStats->usb_empty_complete_count, pUsbStats->usb_complete_max_count);

    USB_printf("port_change=%u, suspend=%u\n", 
                pUsbStats->usb_port_change_count, pUsbStats->usb_suspend_count);
    for(i=0; i<(pUsbDev->MAX_ENDPOINTS); i++)
    {
        if( (pUsbStats->usb_complete_ep_count[i*2] == 0) && 
            (pUsbStats->usb_complete_ep_count[i*2+1] == 0) )
            continue;

        USB_printf("EP #%d: RECV (OUT) = %3u, \tSEND (IN) = %u\n", i,
                    pUsbStats->usb_complete_ep_count[i*2], 
                    pUsbStats->usb_complete_ep_count[i*2+1]);
    }
    USB_printf("\n");
}

void _usb_clear_stats(void* usbHandle)
{
    USB_DEV_STATE_STRUCT*   pUsbDev = (USB_DEV_STATE_STRUCT*)usbHandle;

    USB_memzero(&pUsbDev->STATS, sizeof(pUsbDev->STATS));
}

void _usb_regs(void* usbHandle)
{
    USB_DEV_STATE_STRUCT*   pUsbDev = (USB_DEV_STATE_STRUCT*)usbHandle;
    VUSB20_REG_STRUCT*      cap_regs, *dev_regs;
    int                     dev_num;

    if(pUsbDev == NULL)
    {
        USB_printf("USB Device core is not initialized\n");
        return;
    }
    USB_printf("\n\tUSB Capability Registers\n\n");

    cap_regs = pUsbDev->CAP_REGS_PTR;
    USB_printf("CAPLENGTH_HCIVER (0x%08x) = 0x%08x\n", 
        (unsigned)&cap_regs->REGISTERS.CAPABILITY_REGISTERS.CAPLENGTH_HCIVER,
        (unsigned)USB_32BIT_LE(cap_regs->REGISTERS.CAPABILITY_REGISTERS.CAPLENGTH_HCIVER));

    USB_printf("DCI_VERSION      (0x%08x) = 0x%08x\n", 
        (unsigned)&cap_regs->REGISTERS.CAPABILITY_REGISTERS.DCI_VERSION,
        (unsigned)USB_32BIT_LE(cap_regs->REGISTERS.CAPABILITY_REGISTERS.DCI_VERSION));

    USB_printf("DCC_PARAMS       (0x%08x) = 0x%08x\n", 
        (unsigned)&cap_regs->REGISTERS.CAPABILITY_REGISTERS.DCC_PARAMS,
        (unsigned)USB_32BIT_LE(cap_regs->REGISTERS.CAPABILITY_REGISTERS.DCC_PARAMS));

    dev_regs = pUsbDev->DEV_PTR;
    dev_num = pUsbDev->DEV_NUM;
    USB_printf("\n\tUSB Device Operational Registers\n\n");

    USB_printf("USB_CMD          (0x%08x) = 0x%08x\n", 
        (unsigned)&dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_CMD,
        (unsigned)USB_32BIT_LE(dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_CMD));

    USB_printf("USB_STS          (0x%08x) = 0x%08x\n", 
        (unsigned)&dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_STS,
        (unsigned)USB_32BIT_LE(dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_STS));

    USB_printf("USB_INTR         (0x%08x) = 0x%08x\n", 
        (unsigned)&dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_INTR,
        (unsigned)USB_32BIT_LE(dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_INTR));

    USB_printf("USB_FRINDEX      (0x%08x) = 0x%08x\n", 
        (unsigned)&dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_FRINDEX,
        (unsigned)USB_32BIT_LE(dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_FRINDEX));

    /* Skip CTRLDSSEGMENT register */
    USB_printf("DEVICE_ADDR      (0x%08x) = 0x%08x\n", 
        (unsigned)&dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.DEVICE_ADDR,
        (unsigned)USB_32BIT_LE(dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.DEVICE_ADDR));

    USB_printf("EP_LIST_ADDR     (0x%08x) = 0x%08x\n", 
        (unsigned)&dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.EP_LIST_ADDR,
        (unsigned)USB_32BIT_LE(dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.EP_LIST_ADDR));

    /* Skip CONFIG_FLAG register */

    /* Skip PORTSCX[0..15] registers*/
    USB_printf("PORTSCX[0]       (0x%08x) = 0x%08x\n", 
        (unsigned)&dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.PORTSCX[0],
        (unsigned)USB_32BIT_LE(dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.PORTSCX[0]));

    /* Skip OTGSC register */

    USB_printf("USB_MODE         (0x%08x) = 0x%08x\n", 
        (unsigned)&dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_MODE,
        (unsigned)USB_32BIT_LE(dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_MODE));

    USB_printf("ENDPT_SETUP_STAT (0x%08x) = 0x%08x\n", 
        (unsigned)&dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPT_SETUP_STAT,
        (unsigned)USB_32BIT_LE(dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPT_SETUP_STAT));

    USB_printf("ENDPTPRIME       (0x%08x) = 0x%08x\n", 
        (unsigned)&dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTPRIME,
        (unsigned)USB_32BIT_LE(dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTPRIME));

    USB_printf("ENDPTFLUSH       (0x%08x) = 0x%08x\n", 
        (unsigned)&dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTFLUSH,
        (unsigned)USB_32BIT_LE(dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTFLUSH));

    USB_printf("ENDPTSTATUS      (0x%08x) = 0x%08x\n", 
        (unsigned)&dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTSTATUS,
        (unsigned)USB_32BIT_LE(dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTSTATUS));

    USB_printf("ENDPTCOMPLETE    (0x%08x) = 0x%08x\n", 
        (unsigned)&dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCOMPLETE,
        (unsigned)USB_32BIT_LE(dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCOMPLETE));
}

    
void _usb_ep_status(void* usbHandle, int ep_num, int direction)
{
    USB_DEV_STATE_STRUCT*               pUsbDev = (USB_DEV_STATE_STRUCT*)usbHandle;
    int                                 i, ep_idx;
    VUSB20_EP_QUEUE_HEAD_STRUCT_PTR     ep_queue_head_ptr;
    VUSB20_EP_TR_STRUCT_PTR             dTD_ptr, head_dTD_ptr, tail_dTD_ptr, next_dTD_ptr;
    XD_STRUCT_PTR                       xd_ptr, next_xd_ptr;
    VUSB20_REG_STRUCT_PTR               dev_regs;

    if(pUsbDev == NULL)
    {
        USB_printf("USB Device core is not initialized\n");
        return;
    }

    USB_printf("\n\tUSB Endpoint #%d - %s status\n\n", ep_num,
        (direction == ARC_USB_SEND) ? "SEND (IN)" : "RECV (OUT)" );

    ep_idx = ep_num*2 + direction;
    dev_regs = pUsbDev->DEV_PTR;

    USB_printf("ENDPTCTRLX[%d]    (0x%08x) = 0x%08x\n", ep_num,
        (unsigned)&dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[ep_num],
        (unsigned)USB_32BIT_LE(dev_regs->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[ep_num]));

    ep_queue_head_ptr = (VUSB20_EP_QUEUE_HEAD_STRUCT_PTR)pUsbDev->EP_QUEUE_HEAD_PTR + ep_idx;

    head_dTD_ptr = pUsbDev->EP_DTD_HEADS[ep_idx];
    tail_dTD_ptr = pUsbDev->EP_DTD_TAILS[ep_idx];

    USB_printf("EP_QH=0x%08x: MAX_PKT=0x%x, SIZE_IOC_INT_STS=0x%x, CURR_DTD=0x%x, NEXT_DTD=0x%x\n", 
                (unsigned)ep_queue_head_ptr, (unsigned)USB_32BIT_LE(ep_queue_head_ptr->MAX_PKT_LENGTH), 
                (unsigned)USB_32BIT_LE(ep_queue_head_ptr->SIZE_IOC_INT_STS), 
                (unsigned)USB_32BIT_LE(ep_queue_head_ptr->CURR_DTD_PTR), 
                (unsigned)USB_32BIT_LE(ep_queue_head_ptr->NEXT_DTD_PTR));

    USB_printf("\tBUF_0=0x%08x, BUF_1=0x%08x, BUF_2=0x%08x, BUF_3=0x%08x, BUF_4=0x%08x\n",
                (unsigned)USB_32BIT_LE(ep_queue_head_ptr->BUFF_PTR0), 
                (unsigned)USB_32BIT_LE(ep_queue_head_ptr->BUFF_PTR1), 
                (unsigned)USB_32BIT_LE(ep_queue_head_ptr->BUFF_PTR2), 
                (unsigned)USB_32BIT_LE(ep_queue_head_ptr->BUFF_PTR3), 
                (unsigned)USB_32BIT_LE(ep_queue_head_ptr->BUFF_PTR4));
    
    USB_printf("\tSETUP_BUFFER (%p): ", ep_queue_head_ptr->SETUP_BUFFER);
    for(i=0; i<sizeof(ep_queue_head_ptr->SETUP_BUFFER); i++)
        USB_printf("%02x", ep_queue_head_ptr->SETUP_BUFFER[i] & 0xFF);
    USB_printf("\n");

    USB_printf("\ndTD_HEAD=0x%08x, dTD_TAIL=0x%08x\n", 
                (unsigned)head_dTD_ptr, (unsigned)tail_dTD_ptr);

    dTD_ptr = head_dTD_ptr;
    i = 0;
    while(dTD_ptr != NULL)
    {
        USB_printf("%d. dTD=0x%08x (0x%08x), SIZE_IOC_STS=0x%08x, BUF_0=0x%08x, NEXT=0x%08x\n", 
                    i, (unsigned)dTD_ptr, USB_DTD_VIRT_TO_PHYS(pUsbDev, dTD_ptr),
                    (unsigned)USB_32BIT_LE(dTD_ptr->SIZE_IOC_STS), 
                    (unsigned)USB_32BIT_LE(dTD_ptr->BUFF_PTR0), 
                    (unsigned)USB_32BIT_LE(dTD_ptr->NEXT_TR_ELEM_PTR));

        xd_ptr = dTD_ptr->SCRATCH_PTR->XD_FOR_THIS_DTD;

        next_dTD_ptr = (VUSB20_EP_TR_STRUCT_PTR)USB_DTD_PHYS_TO_VIRT(pUsbDev, 
                           (uint_32)(USB_32BIT_LE(dTD_ptr->NEXT_TR_ELEM_PTR) & VUSBHS_TD_ADDR_MASK));
        if(next_dTD_ptr != NULL) 
            next_xd_ptr = next_dTD_ptr->SCRATCH_PTR->XD_FOR_THIS_DTD;
        else
            next_xd_ptr = NULL;

        if(next_xd_ptr != xd_ptr)
        {
            USB_printf("\tXD=0x%08x, ADDR=0x%08x, SIZE=%u, STATUS=0x%02x\n",
                (unsigned)xd_ptr, (unsigned)xd_ptr->WSTARTADDRESS, 
                (unsigned)xd_ptr->WTOTALLENGTH, xd_ptr->BSTATUS);
        }
        i++;
        dTD_ptr = next_dTD_ptr;
    }
}


/* DEBUG */
uint_32 usbDebugFlags = ARC_DEBUG_FLAG_STATS
                      | ARC_DEBUG_FLAG_INIT 
                      | ARC_DEBUG_FLAG_ERROR 
                      | ARC_DEBUG_FLAG_STALL
                      | ARC_DEBUG_FLAG_RESET;
                      /*| ARC_DEBUG_FLAG_TRANSFER;*/

void    _usb_debug_set_flags(uint_32 flags)     
{
    usbDebugFlags = (flags);                        
}

uint_32 _usb_debug_get_flags(void)     
{
    return usbDebugFlags;  
}

#if defined(MV_USB_TRACE_LOG)

uint_16 DEBUG_TRACE_ARRAY_COUNTER = 0;
char    DEBUG_TRACE_ARRAY[TRACE_ARRAY_SIZE][MAX_STRING_SIZE];

void  _usb_debug_init_trace_log(void)                   
{                                                  
    USB_memzero(DEBUG_TRACE_ARRAY, TRACE_ARRAY_SIZE*MAX_STRING_SIZE);
	DEBUG_TRACE_ARRAY_COUNTER =0;                  
}                               

void    _usb_debug_print_trace_log(void)                     
{                                                       
    int     i;                                          
                                                        
    USB_printf("USB Trace log: start=0x%x, end=0x%x, idx=%d, flags=0x%x\n\n", 
               &DEBUG_TRACE_ARRAY[0][0], &DEBUG_TRACE_ARRAY[TRACE_ARRAY_SIZE-1][0], 
               DEBUG_TRACE_ARRAY_COUNTER, usbDebugFlags);      

    for(i=DEBUG_TRACE_ARRAY_COUNTER; i<TRACE_ARRAY_SIZE; i++)                      
    {                                                 
        if(DEBUG_TRACE_ARRAY[i][0] == '\0')              
            continue;                     
        
        USB_printf("%3d. %s", i, DEBUG_TRACE_ARRAY[i]);
    }                                                 
    for(i=0; i<DEBUG_TRACE_ARRAY_COUNTER; i++)       
    {                                                 
        if(DEBUG_TRACE_ARRAY[i][0] == '\0')           
            continue;                                 
        USB_printf("%3d. %s", i, DEBUG_TRACE_ARRAY[i]);
    }                          
    _usb_debug_init_trace_log();
}
#else
void  _usb_debug_init_trace_log(void)                   
{
}

void    _usb_debug_print_trace_log(void)
{
    USB_printf("USB trace log is not supported\n");
}
#endif /* MV_USB_TRACE_LOG */




