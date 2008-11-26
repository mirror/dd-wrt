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
*  Function Name  : _usb_dci_vusb20_init
*  Returned Value : USB_OK or error code
*  Comments       :
*        Initializes the USB device controller.
*
*END*-----------------------------------------------------------------*/
uint_8 _usb_dci_vusb20_init
   (
      /* [IN] the USB device controller to initialize */
      uint_8                     devnum,

      /* [OUT] the USB_dev_initialize state structure */
      _usb_device_handle         handle
   )
{ /* Body */
    USB_DEV_STATE_STRUCT_PTR    usb_dev_ptr;
    uint_32                     temp;
    uint_8*                     pBuf;
    unsigned long               phyAddr;
      
    usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;

    usb_dev_ptr->CAP_REGS_PTR = 
       (VUSB20_REG_STRUCT_PTR)USB_get_cap_reg_addr(devnum);
      
    /* Get the base address of the VUSB_HS registers */
    usb_dev_ptr->DEV_PTR = 
      (VUSB20_REG_STRUCT_PTR)(((uint_32)usb_dev_ptr->CAP_REGS_PTR) + 
       (USB_32BIT_LE(usb_dev_ptr->CAP_REGS_PTR->REGISTERS.CAPABILITY_REGISTERS.CAPLENGTH_HCIVER) & 
                                                                EHCI_CAP_LEN_MASK));

    /* Get the maximum number of endpoints supported by this USB controller */
    usb_dev_ptr->MAX_ENDPOINTS = 
      (USB_32BIT_LE(usb_dev_ptr->CAP_REGS_PTR->REGISTERS.CAPABILITY_REGISTERS.DCC_PARAMS) & 
                                                VUSB20_DCC_MAX_ENDPTS_SUPPORTED);

    USB_printf("USB init: CAP_REGS=0x%x, DEV_REGS=0x%x, MAX_EP=%d\n",
                (unsigned)usb_dev_ptr->CAP_REGS_PTR, (unsigned)usb_dev_ptr->DEV_PTR, 
                usb_dev_ptr->MAX_ENDPOINTS);

    temp = (usb_dev_ptr->MAX_ENDPOINTS * 2);
   
    pBuf = (uint_8*)USB_uncached_memalloc(temp*sizeof(VUSB20_EP_QUEUE_HEAD_STRUCT), 
                                          2048, &phyAddr);      
    if (pBuf == NULL) 
    {
        USB_printf("_usb_dci_vusb20_init, malloc of %d bytes in Uncached area failed\n",
                        temp*sizeof(VUSB20_EP_QUEUE_HEAD_STRUCT));
        return USBERR_ALLOC;
    }

    /****************************************************************   
      Assign QH base
    ****************************************************************/   
    usb_dev_ptr->EP_QUEUE_HEAD_BASE = pBuf;
    usb_dev_ptr->EP_QUEUE_HEAD_PHYS = (uint_32)phyAddr;

    /* Align the endpoint queue head to 2K boundary */   
    usb_dev_ptr->EP_QUEUE_HEAD_PTR = (VUSB20_EP_QUEUE_HEAD_STRUCT_PTR)
                USB_MEM2048_ALIGN((uint_32)usb_dev_ptr->EP_QUEUE_HEAD_BASE);

    usb_dev_ptr->EP_QUEUE_HEAD_SIZE = temp*sizeof(VUSB20_EP_QUEUE_HEAD_STRUCT) +
                                      ((uint_32)usb_dev_ptr->EP_QUEUE_HEAD_PTR - 
                                       (uint_32)usb_dev_ptr->EP_QUEUE_HEAD_BASE);

    /****************************************************************   
      Zero out the memory allocated
    ****************************************************************/   
    USB_memzero( (void*)usb_dev_ptr->EP_QUEUE_HEAD_PTR, 
                 temp*sizeof(VUSB20_EP_QUEUE_HEAD_STRUCT));

    USB_printf("USB EP_QH: Base=%p (0x%x), Aligned(%d)=%p, Size=%d\n",
                usb_dev_ptr->EP_QUEUE_HEAD_BASE, usb_dev_ptr->EP_QUEUE_HEAD_PHYS,
                2048, usb_dev_ptr->EP_QUEUE_HEAD_PTR, usb_dev_ptr->EP_QUEUE_HEAD_SIZE);

    /****************************************************************   
      Assign DTD base
    ****************************************************************/   
    pBuf = (uint_8*)USB_uncached_memalloc(MAX_EP_TR_DESCRS*sizeof(VUSB20_EP_TR_STRUCT), 
                                          32, &phyAddr);      
    if (pBuf == NULL) 
    {
        USB_printf("_usb_dci_vusb20_init, malloc of %d bytes in Uncached area failed\n",
                        MAX_EP_TR_DESCRS*sizeof(VUSB20_EP_TR_STRUCT));
        return USBERR_ALLOC;
    }

    usb_dev_ptr->DTD_BASE_PTR = pBuf;
    usb_dev_ptr->DTD_BASE_PHYS = (uint_32)phyAddr;
    
    /* Align the dTD base to 32 byte boundary */   
    usb_dev_ptr->DTD_ALIGNED_BASE_PTR = (VUSB20_EP_TR_STRUCT_PTR)
                        USB_MEM32_ALIGN((uint_32)usb_dev_ptr->DTD_BASE_PTR);

    usb_dev_ptr->DTD_SIZE = MAX_EP_TR_DESCRS*sizeof(VUSB20_EP_TR_STRUCT) +
                                ((uint_32)usb_dev_ptr->EP_QUEUE_HEAD_PTR - 
                                 (uint_32)usb_dev_ptr->EP_QUEUE_HEAD_BASE);

    /****************************************************************   
      Zero out the memory allocated
    ****************************************************************/   
    USB_memzero((void*)usb_dev_ptr->DTD_ALIGNED_BASE_PTR, 
                MAX_EP_TR_DESCRS*sizeof(VUSB20_EP_TR_STRUCT));

    /****************************************************************   
      Assign SCRATCH Structure base
    ****************************************************************/   
    /* Allocate memory for internal scratch structure */   
    pBuf = USB_memalloc(MAX_EP_TR_DESCRS*sizeof(SCRATCH_STRUCT));
    if (pBuf == NULL) 
    {
        USB_printf("_usb_dci_vusb20_init, malloc of %d bytes failed\n",
                        MAX_EP_TR_DESCRS*sizeof(SCRATCH_STRUCT));
        return USBERR_ALLOC;
    }
    usb_dev_ptr->SCRATCH_STRUCT_BASE = (SCRATCH_STRUCT_PTR)pBuf;
    USB_memzero(usb_dev_ptr->SCRATCH_STRUCT_BASE, 
            MAX_EP_TR_DESCRS*sizeof(SCRATCH_STRUCT));

    USB_printf("USB dTD(%d): Base=%p (0x%x), Aligned(%d)=%p, Size=%d, Scratch=%p\n",
                MAX_EP_TR_DESCRS, usb_dev_ptr->DTD_BASE_PTR, usb_dev_ptr->DTD_BASE_PHYS,
                32, usb_dev_ptr->DTD_ALIGNED_BASE_PTR, usb_dev_ptr->DTD_SIZE,
                usb_dev_ptr->SCRATCH_STRUCT_BASE);

#ifdef USB_UNDERRUN_WA
    usbSramBase = (uint_8*)USB_get_sram_addr(&usbSramSize);
    if (usbSramBase == NULL) 
    {
        USB_printf("_usb_dci_vusb20_init, SRAM is not available\n");                        
        return USBERR_ALLOC;
    }
    USB_memzero(usbSramBase, usbSramSize);
    USB_printf("USB WA_Queue: base=%p, size=%d, parts=%d\n", 
                    usbSramBase, usbSramSize, global_wa_sram_parts);
#endif /* USB_UNDERRUN_WA */

    usb_dev_ptr->USB_STATE = ARC_USB_STATE_UNKNOWN;

    /* Initialize the VUSB_HS controller */   
    _usb_dci_vusb20_chip_initialize((pointer)usb_dev_ptr);

    return USB_OK;   
} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_chip_initialize
*  Returned Value : USB_OK or error code
*  Comments       :
*        Initializes the USB device controller.
*
*END*-----------------------------------------------------------------*/
void _usb_dci_vusb20_chip_initialize
   (
      /* [IN] the USB_dev_initialize state structure */
      _usb_device_handle         handle
   )
{ /* Body */
    USB_DEV_STATE_STRUCT_PTR         usb_dev_ptr;
    VUSB20_REG_STRUCT_PTR            dev_ptr;
    VUSB20_EP_QUEUE_HEAD_STRUCT_PTR  ep_queue_head_ptr;
    VUSB20_EP_TR_STRUCT_PTR          dTD_ptr;
    uint_32                          i, port_control;
    SCRATCH_STRUCT_PTR               temp_scratch_ptr;
    volatile unsigned long           delay;

    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_INIT, "chip_initialize\n");
   
    usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;

    dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;
   
    /* Stop the controller */
    dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_CMD &= ~(USB_32BIT_LE(EHCI_CMD_RUN_STOP));
      
    /* Reset the controller to get default values */
    dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_CMD = USB_32BIT_LE(EHCI_CMD_CTRL_RESET);

    USB_printf("USB Init: Wait for RESET completed\n");
   
    delay = 0x100000;
    while (dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_CMD & 
                                    (USB_32BIT_LE(EHCI_CMD_CTRL_RESET))) 
    {
        /* Wait for the controller reset to complete */
        delay--;
        if(delay == 0)
            break;
    } /* EndWhile */

    if(delay == 0)
    {
        USB_printf("USB Init: Wait for RESET completed TIMEOUT\n");
    }
    else
    {
        USB_printf("USB Init: RESET completed\n");
    }
    /* Call BSP callback to complete reset process */
    USB_reset_complete(usb_dev_ptr->DEV_NUM);

    /* Initialize the internal dTD head and tail to NULL */   
    usb_dev_ptr->DTD_HEAD = NULL;
    usb_dev_ptr->DTD_TAIL = NULL;
    usb_dev_ptr->DTD_ENTRIES = 0; 
    usb_dev_ptr->ERROR_STATE = 0;

   /* Make sure the 16 MSBs of this register are 0s */
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPT_SETUP_STAT = USB_32BIT_LE(0);
   
   ep_queue_head_ptr = usb_dev_ptr->EP_QUEUE_HEAD_PTR;

   /* Initialize all device queue heads */
   for (i=0; i<(usb_dev_ptr->MAX_ENDPOINTS*2); i++) 
   {
      /* Interrupt on Setup packet */
      (ep_queue_head_ptr + i)->MAX_PKT_LENGTH = (USB_32BIT_LE(
          ((uint_32)USB_MAX_CTRL_PAYLOAD << VUSB_EP_QUEUE_HEAD_MAX_PKT_LEN_POS) | 
            VUSB_EP_QUEUE_HEAD_IOS));

      (ep_queue_head_ptr + i)->NEXT_DTD_PTR = (USB_32BIT_LE(VUSB_EP_QUEUE_HEAD_NEXT_TERMINATE));
   } /* Endfor */

   /* Configure the Endpoint List Address */
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.EP_LIST_ADDR = 
                            USB_32BIT_LE(USB_EP_QH_VIRT_TO_PHYS(usb_dev_ptr, ep_queue_head_ptr));
      
   port_control = USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.PORTSCX[0]);
   if (usb_dev_ptr->CAP_REGS_PTR->REGISTERS.CAPABILITY_REGISTERS.HCS_PARAMS & 
                                        USB_32BIT_LE(VUSB20_HCS_PARAMS_PORT_POWER_CONTROL_FLAG)) 
   {
      port_control &= (~EHCI_PORTSCX_W1C_BITS | ~EHCI_PORTSCX_PORT_POWER);
   } /* Endif */
   
   if(usb_dev_ptr->FORCE_FS == TRUE)
   {
       port_control |= EHCI_PORTSCX_FORCE_FULL_SPEED_CONNECT;
   }
   else
   {
       port_control &= (~EHCI_PORTSCX_FORCE_FULL_SPEED_CONNECT);
   }
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.PORTSCX[0] = USB_32BIT_LE(port_control);

   dTD_ptr = usb_dev_ptr->DTD_ALIGNED_BASE_PTR;

   temp_scratch_ptr = usb_dev_ptr->SCRATCH_STRUCT_BASE;

   /* Enqueue all the dTDs */   
   for (i=0; i<MAX_EP_TR_DESCRS; i++) 
   {
      dTD_ptr->SCRATCH_PTR = temp_scratch_ptr;
      dTD_ptr->SCRATCH_PTR->FREE = _usb_dci_vusb20_free_dTD;
      /* Set the dTD to be invalid */
      dTD_ptr->NEXT_TR_ELEM_PTR = USB_32BIT_LE(VUSBHS_TD_NEXT_TERMINATE);
      /* Set the Reserved fields to 0 */
      dTD_ptr->SIZE_IOC_STS &= ~(USB_32BIT_LE(VUSBHS_TD_RESERVED_FIELDS));
      dTD_ptr->SCRATCH_PTR->PRIVATE = (pointer)usb_dev_ptr;
      _usb_dci_vusb20_free_dTD((pointer)dTD_ptr);
      dTD_ptr++;
      temp_scratch_ptr++;
   } /* Endfor */
} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_free_dTD
*  Returned Value : void
*  Comments       :
*        Enqueues a dTD onto the free DTD ring.
*
*END*-----------------------------------------------------------------*/

void _usb_dci_vusb20_free_dTD
   (
      /* [IN] the dTD to enqueue */
      pointer  dTD_ptr
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR     usb_dev_ptr;
   int                          lockKey;

   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)(((VUSB20_EP_TR_STRUCT_PTR)dTD_ptr)->SCRATCH_PTR->PRIVATE);

   ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_TRACE, "free_dTD: dTD_ptr=0x%x\n", (unsigned)dTD_ptr);

   ARC_DEBUG_CODE(ARC_DEBUG_FLAG_STATS, (usb_dev_ptr->STATS.free_dTD_count++));


   /*
   ** This function can be called from any context, and it needs mutual
   ** exclusion with itself.
   */
   lockKey = USB_lock();

   /*
   ** Add the dTD to the free dTD queue (linked via PRIVATE) and
   ** increment the tail to the next descriptor
   */
   EHCI_DTD_QADD(usb_dev_ptr->DTD_HEAD, usb_dev_ptr->DTD_TAIL, (VUSB20_EP_TR_STRUCT_PTR)dTD_ptr);
   usb_dev_ptr->DTD_ENTRIES++;

   USB_unlock(lockKey);

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_add_dTD
*  Returned Value : USB_OK or error code
*  Comments       :
*        Adds a device transfer desriptor(s) to the queue.
*
*END*-----------------------------------------------------------------*/
uint_8 _usb_dci_vusb20_add_dTD
   (
      /* [IN] the USB_dev_initialize state structure */
      _usb_device_handle         handle,
     
      /* [IN] The transfer descriptor address */
      XD_STRUCT_PTR              xd_ptr
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR         usb_dev_ptr;
   VUSB20_REG_STRUCT_PTR            dev_ptr;
   VUSB20_EP_TR_STRUCT_PTR          dTD_ptr, temp_dTD_ptr, first_dTD_ptr = NULL;
   VUSB20_EP_QUEUE_HEAD_STRUCT_PTR  ep_queue_head_ptr;
   uint_32                          curr_pkt_len, remaining_len; 
   uint_32                          curr_offset, temp, bit_pos;
   volatile unsigned long           timeout;
   
   /*********************************************************************
   For a optimal implementation, we need to detect the fact that
   we are adding DTD to an empty list. If list is empty, we can
   actually skip several programming steps esp. those for ensuring
   that there is no race condition.The following boolean will be useful
   in skipping some code here.
   *********************************************************************/
   boolean           list_empty = FALSE;
   
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
   dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;

   remaining_len = xd_ptr->WTOTALLENGTH;
   
   curr_offset = 0;
   temp = (2*xd_ptr->EP_NUM + xd_ptr->BDIRECTION);
   bit_pos = (1 << (16 * xd_ptr->BDIRECTION + xd_ptr->EP_NUM));
   
   ep_queue_head_ptr = (VUSB20_EP_QUEUE_HEAD_STRUCT_PTR)usb_dev_ptr->EP_QUEUE_HEAD_PTR + temp;

   /*********************************************************************
   This loops iterates through the length of the transfer and divides
   the data in to DTDs each handling the a max of 0x4000 bytes of data.
   The first DTD in the list is stored in a pointer called first_dTD_ptr.
   This pointer is later linked in to QH for processing by the hardware.   
   *********************************************************************/

    do 
    {
        /* Check if we need to split the transfer into multiple dTDs */
        if (remaining_len > VUSB_EP_MAX_LENGTH_TRANSFER) 
        {
            curr_pkt_len = VUSB_EP_MAX_LENGTH_TRANSFER;
        } 
        else 
        {
            curr_pkt_len = remaining_len;
        } /* Endif */
   
        /* Get a dTD from the queue */   
        EHCI_DTD_QGET(usb_dev_ptr->DTD_HEAD, usb_dev_ptr->DTD_TAIL, dTD_ptr);
   
        if (!dTD_ptr) 
        {
            USB_printf("Error: Can't get dTD\n");
            return USBERR_TR_FAILED;
        } /* Endif */

        ARC_DEBUG_CODE(ARC_DEBUG_FLAG_STATS, (usb_dev_ptr->STATS.usb_add_count++));

        remaining_len -= curr_pkt_len;

        usb_dev_ptr->DTD_ENTRIES--;

        if (curr_offset == 0) 
        {
            first_dTD_ptr = dTD_ptr;
        } /* Endif */

        /* Zero the dTD. Leave the last 4 bytes as that is the scratch pointer */
        USB_memzero((void *) dTD_ptr,(sizeof(VUSB20_EP_TR_STRUCT) - 4));

        /* Initialize the dTD */
        dTD_ptr->SCRATCH_PTR->PRIVATE = handle;
   
        /* Set the Terminate bit */
        dTD_ptr->NEXT_TR_ELEM_PTR = USB_32BIT_LE(VUSB_EP_QUEUE_HEAD_NEXT_TERMINATE);

        /*************************************************************
        FIX ME: For hig-speed and high-bandwidth ISO IN endpoints,
        we must initialize the multiplied field so that Host can issues
        multiple IN transactions on the endpoint. See the DTD data
        structure for MultiIO field.
      
        S Garg 11/06/2003
        *************************************************************/
      
        /* Fill in the transfer size */
        if (!remaining_len) 
        {
            dTD_ptr->SIZE_IOC_STS = USB_32BIT_LE((curr_pkt_len << 
                    VUSBHS_TD_LENGTH_BIT_POS) | (VUSBHS_TD_IOC) | (VUSBHS_TD_STATUS_ACTIVE));
        } 
        else 
        {
            dTD_ptr->SIZE_IOC_STS = USB_32BIT_LE((curr_pkt_len << VUSBHS_TD_LENGTH_BIT_POS) 
                                                   | VUSBHS_TD_STATUS_ACTIVE);
        } /* Endif */
   
        /* Set the reserved field to 0 */
        dTD_ptr->SIZE_IOC_STS &= ~USB_32BIT_LE(VUSBHS_TD_RESERVED_FIELDS);

        /* 4K apart buffer page pointers */
        if(xd_ptr->WSTARTADDRESS != NULL)
        {
            uint_32 physAddr = USB_virt_to_phys((uint_8*)xd_ptr->WSTARTADDRESS + curr_offset);

            dTD_ptr->BUFF_PTR0 = USB_32BIT_LE(physAddr);
            
            physAddr += 4096;
            dTD_ptr->BUFF_PTR1 = USB_32BIT_LE(physAddr);

            physAddr += 4096;
            dTD_ptr->BUFF_PTR2 = USB_32BIT_LE(physAddr);

            physAddr += 4096;
            dTD_ptr->BUFF_PTR3 = USB_32BIT_LE(physAddr);

            physAddr += 4096;
            dTD_ptr->BUFF_PTR4 = USB_32BIT_LE(physAddr);
        }
        else
        {
            dTD_ptr->BUFF_PTR0 = dTD_ptr->BUFF_PTR1 = dTD_ptr->BUFF_PTR2 = 0;
            dTD_ptr->BUFF_PTR3 = dTD_ptr->BUFF_PTR4 = 0;
        }
        curr_offset += curr_pkt_len;

      /* Maintain the first and last device transfer descriptor per 
      ** endpoint and direction 
      */
      if (!usb_dev_ptr->EP_DTD_HEADS[temp]) 
      {
         usb_dev_ptr->EP_DTD_HEADS[temp] = dTD_ptr;
         /***********************************************
         If list does not have a head, it means that list
         is empty. An empty condition is detected.
         ***********************************************/
         list_empty = TRUE;
      } /* Endif */ 
   
      /* Check if the transfer is to be queued at the end or beginning */
      temp_dTD_ptr = usb_dev_ptr->EP_DTD_TAILS[temp];
      
      /* Remember which XD to use for this dTD */
      dTD_ptr->SCRATCH_PTR->XD_FOR_THIS_DTD = (pointer)xd_ptr;
      
      /* New tail */
      usb_dev_ptr->EP_DTD_TAILS[temp] = dTD_ptr;
      if (temp_dTD_ptr) 
      {
         /* Should not do |=. The Terminate bit should be zero */
         temp_dTD_ptr->NEXT_TR_ELEM_PTR = USB_32BIT_LE(USB_DTD_VIRT_TO_PHYS(usb_dev_ptr, dTD_ptr));
      } /* Endif */
   } while (remaining_len); /* EndWhile */


   /**************************************************************
   In the loop above DTD has already been added to the list
   However endpoint has not been primed yet. If list is not empty 
   we need safter ways to add DTD to the existing list. 
   Else we just skip to adding DTD to QH safely.
   **************************************************************/
   
    if(list_empty == FALSE)
    {
        volatile boolean    read_safe = FALSE;                      
        uint_32             prime, temp_ep_stat=0;

        /*********************************************************
        Hardware v3.2+ require the use of semaphore to ensure that
        QH is safely updated.
        *********************************************************/
        ARC_DEBUG_CODE(ARC_DEBUG_FLAG_STATS, (usb_dev_ptr->STATS.usb_add_not_empty_count++));

        /*********************************************************
        Check the prime bit. If set goto done
        *********************************************************/
        prime = USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTPRIME);
/*
        USB_printf("%03d: Add not empty: bit_pos=%x, prime=0x%x, status=0x%x\n",
                    usb_dev_ptr->STATS.usb_add_not_empty_count, prime, bit_pos,
                    USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTSTATUS),
                    USB_32BIT_LE(ep_queue_head_ptr->SIZE_IOC_INT_STS) );
*/
        if(prime & bit_pos)
        {
            timeout = 0x1000;
            while( dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTPRIME
                    & USB_32BIT_LE(bit_pos) ) 
            {
               /* Wait for the ENDPTPRIME to go to zero */
                timeout--;
                if(timeout <= 0)
                {
                    USB_printf(
                        "timeout: CTRL=%x, PRIME=%x, STAT=%x, INTR=%x, ADDR=%x, PORTSC=%x, dTD=%p, temp_dTD=%p\n",
                        USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[0]),
                        USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTPRIME), 
                        USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTSTATUS), 
                        USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_STS),
                        USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.DEVICE_ADDR),
                        USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.PORTSCX[0]), 
                        first_dTD_ptr, temp_dTD_ptr);

                    _usb_ep_status(handle, xd_ptr->EP_NUM, xd_ptr->BDIRECTION);

                    return USBERR_TR_FAILED;
                }
            } /* EndWhile */

            /*ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_TRANSFER,*/
            if(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTSTATUS & USB_32BIT_LE(bit_pos))
            {
                goto done;
            }
        }

        read_safe = FALSE;
        timeout = 1000000;
        while(read_safe == FALSE)
        {
            timeout--;
            if(timeout <= 0)
            {
                USB_printf("%s: Timeout for ATDTW_TRIPWIRE reg = 0x%x\n", __FUNCTION__, 
                    (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_CMD));
                return USBERR_TR_FAILED;
            }

           /*********************************************************
           start with setting the semaphores
           *********************************************************/
           dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_CMD |= 
                                          USB_32BIT_LE(EHCI_CMD_ATDTW_TRIPWIRE_SET);
               
           /*********************************************************
           Read the endpoint status
           *********************************************************/
           temp_ep_stat = USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTSTATUS) 
                                        & bit_pos; 

           /*********************************************************
           Reread the ATDTW semaphore bit to check if it is cleared.
           When hardware see a hazard, it will clear the bit or
           else we remain set to 1 and we can proceed with priming
           of endpoint if not already primed.
           *********************************************************/
           if( dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_CMD & 
                                          USB_32BIT_LE(EHCI_CMD_ATDTW_TRIPWIRE_SET))
           {
               read_safe = TRUE;
           }

        }/*end while loop */

        /*********************************************************
        Clear the semaphore
        *********************************************************/
        dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_CMD &= 
                                       USB_32BIT_LE(EHCI_CMD_ATDTW_TRIPWIRE_CLEAR);

        /*********************************************************
         * If endpoint is not active, we activate it now.               
         *********************************************************/
         if(!temp_ep_stat)
         {
            /* No other transfers on the queue */
            ep_queue_head_ptr->NEXT_DTD_PTR = USB_32BIT_LE(
                        USB_DTD_VIRT_TO_PHYS(usb_dev_ptr, first_dTD_ptr));
            ep_queue_head_ptr->SIZE_IOC_INT_STS = 0;

            /* Prime the Endpoint */
            dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTPRIME = USB_32BIT_LE(bit_pos);
         }
    }
    else
    {
         /* No other transfers on the queue */
         ep_queue_head_ptr->NEXT_DTD_PTR = USB_32BIT_LE(
                            USB_DTD_VIRT_TO_PHYS(usb_dev_ptr, first_dTD_ptr));
         ep_queue_head_ptr->SIZE_IOC_INT_STS = 0;
   
         /* Prime the Endpoint */
         dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTPRIME = USB_32BIT_LE(bit_pos);
         /* delay */
         timeout = 0x100;
         while(timeout > 0)
             timeout--;

         dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTPRIME = USB_32BIT_LE(bit_pos);
    }

done:
   if(first_dTD_ptr == NULL)
       USB_printf("ERROR !!!! first_dTD_ptr=NULL\n");

   ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_TRANSFER,
               " add_%d: fri=0x%x, ep=%d%s, buf=%p, size=%d, xd=%p, dTD=%p %p, empty=%d\n",
               usb_dev_ptr->STATS.usb_add_count & 0xFFFF, 
               USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_FRINDEX), 
               xd_ptr->EP_NUM, xd_ptr->BDIRECTION ? "in" : "out", 
               xd_ptr->WSTARTADDRESS, (int)xd_ptr->WTOTALLENGTH, 
               xd_ptr, (unsigned)first_dTD_ptr, 
               usb_dev_ptr->EP_DTD_HEADS[temp], list_empty);


   return USB_OK;
   /* End CR 1015 */
} /* EndBody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_process_tr_complete
*  Returned Value : None
*  Comments       :
*        Services transaction complete interrupt
*
*END*-----------------------------------------------------------------*/
void _usb_dci_vusb20_process_tr_complete
   (
      /* [IN] the USB_dev_initialize state structure */
      _usb_device_handle         handle
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR                     usb_dev_ptr;
   volatile VUSB20_REG_STRUCT_PTR               dev_ptr;
   volatile VUSB20_EP_TR_STRUCT_PTR             dTD_ptr; 
   VUSB20_EP_TR_STRUCT_PTR                      temp_dTD_ptr;
   VUSB20_EP_QUEUE_HEAD_STRUCT_PTR              ep_queue_head_ptr;
   uint_32                                      temp, i, ep_num = 0, direction = 0, bit_pos;
   uint_32                                      remaining_length = 0;
   uint_32                                      actual_transfer_length = 0;
   uint_32                                      counter, errors = 0;
   XD_STRUCT_PTR                                xd_ptr;
   XD_STRUCT_PTR                                temp_xd_ptr = NULL;
   uint_8_ptr                                   buff_start_address = NULL;
   boolean                                      endpoint_detected = FALSE;
   
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
   dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;

   ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_ISR, "process_tr_complete_isr\n");
   ARC_DEBUG_CODE(ARC_DEBUG_FLAG_STATS, (usb_dev_ptr->STATS.usb_complete_isr_count++));

   /* We use separate loops for ENDPTSETUPSTAT and ENDPTCOMPLETE because the 
   ** setup packets are to be read ASAP 
   */
   
   /* Process all Setup packet received interrupts */
   bit_pos = USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPT_SETUP_STAT);
   
   if (bit_pos) 
   {
      ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_ISR, "setup_isr: bit_pos=0x%x\n", (unsigned)bit_pos);
      for(i=0; i<USB_MAX_CONTROL_ENDPOINTS; i++) 
      {
         if (bit_pos & (1 << i)) 
         {
            ARC_DEBUG_CODE(ARC_DEBUG_FLAG_STATS, (usb_dev_ptr->STATS.usb_setup_count++));
            _usb_device_call_service(handle, i, TRUE, 0, 0, 8, 0);
         } /* Endif */
      } /* Endfor */
   } /* Endif */
   
   /* Don't clear the endpoint setup status register here. It is cleared as a 
   ** setup packet is read out of the buffer 
   */

   /* Process non-setup transaction complete interrupts */   
   bit_pos = USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCOMPLETE);

   /* Clear the bits in the register */
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCOMPLETE = USB_32BIT_LE(bit_pos);
   
   ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_ISR, "tr_complete: bit_pos = 0x%x\n", (unsigned)bit_pos);

   if (bit_pos) 
   {
        ARC_DEBUG_CODE(ARC_DEBUG_FLAG_STATS, (usb_dev_ptr->STATS.usb_complete_count++));

        /* Get the endpoint number and the direction of transfer */
        counter = 0;
        for (i=0; i<(ARC_USB_MAX_ENDPOINTS*2); i++) 
        {
            endpoint_detected = FALSE;
            if ((i < ARC_USB_MAX_ENDPOINTS) && (bit_pos & (1 << i)))
            {
                ep_num = i;
                direction = ARC_USB_RECV;
                endpoint_detected = TRUE;
            }
            else
            {
                if( (i >= ARC_USB_MAX_ENDPOINTS) && 
                    (bit_pos & (1 << (i+16-ARC_USB_MAX_ENDPOINTS))))
                {
                    ep_num = (i - ARC_USB_MAX_ENDPOINTS);
                    direction = ARC_USB_SEND;
                    endpoint_detected = TRUE;
                }            
            }

            if(endpoint_detected)
            {
                temp = (2*ep_num + direction);

                /* Get the first dTD */      
                dTD_ptr = usb_dev_ptr->EP_DTD_HEADS[temp];
            
                ep_queue_head_ptr = (VUSB20_EP_QUEUE_HEAD_STRUCT_PTR)usb_dev_ptr->EP_QUEUE_HEAD_PTR + temp;

                /* Process all the dTDs for respective transfers */
                while (dTD_ptr) 
                {            
                    if (USB_32BIT_LE(dTD_ptr->SIZE_IOC_STS) & VUSBHS_TD_STATUS_ACTIVE) 
                    {
                        /* No more dTDs to process. Next one is owned by VUSB */
                        if(counter == 0)
                        {
                            ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_ISR, "tr_complete - break: ep=%d %s, bit_pos=0x%x\n",
                                    (unsigned)ep_num, direction ? "SEND" : "RECV", (unsigned)bit_pos);

                            ARC_DEBUG_CODE(ARC_DEBUG_FLAG_STATS, (usb_dev_ptr->STATS.usb_empty_complete_count++));
                        }
                        break;
                    } /* Endif */
               
                    /* Get the correct internal transfer descriptor */
                    xd_ptr = (XD_STRUCT_PTR)dTD_ptr->SCRATCH_PTR->XD_FOR_THIS_DTD;
                    if (xd_ptr) 
                    {
                        buff_start_address = xd_ptr->WSTARTADDRESS;
                        actual_transfer_length = xd_ptr->WTOTALLENGTH;
                        temp_xd_ptr = xd_ptr;
                    } /* Endif */
               
                    /* Get the address of the next dTD */
                    temp_dTD_ptr = (VUSB20_EP_TR_STRUCT_PTR)USB_DTD_PHYS_TO_VIRT(usb_dev_ptr, 
                                (uint_32)(USB_32BIT_LE(dTD_ptr->NEXT_TR_ELEM_PTR) & VUSBHS_TD_ADDR_MASK) );
                  
                    /* Read the errors */
                    errors = (USB_32BIT_LE(dTD_ptr->SIZE_IOC_STS) & VUSBHS_TD_ERROR_MASK);                  
                    if (!errors) 
                    {
                        /* No errors */
                        /* Get the length of transfer from the current dTD */   
                        remaining_length += ((USB_32BIT_LE(dTD_ptr->SIZE_IOC_STS) & VUSB_EP_TR_PACKET_SIZE) >> 16);
                        actual_transfer_length -= remaining_length;
                    } 
                    else 
                    {
                        USB_printf("complete_tr error: ep=%d %s: error = 0x%x\n", 
                                    (unsigned)ep_num, direction ? "SEND" : "RECV", (unsigned)errors);
                        if (errors & VUSBHS_TD_STATUS_HALTED) 
                        {
                            /* Clear the errors and Halt condition */
                            ep_queue_head_ptr->SIZE_IOC_INT_STS &= USB_32BIT_LE(~errors);
                        } /* Endif */
                    } /* Endif */
               
                    /* Retire the processed dTD */
                    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_ISR, "tr_complete - cancel: ep=%d %s, bit_pos = 0x%x\n",
                                    (unsigned)ep_num, direction ? "SEND" : "RECV", (unsigned)bit_pos);

                    counter++;
                    _usb_dci_vusb20_cancel_transfer(handle, ep_num, direction);
                    if( (temp_dTD_ptr == NULL) || 
                        (temp_dTD_ptr->SCRATCH_PTR->XD_FOR_THIS_DTD != temp_xd_ptr) ) 
                    {
                        /* Transfer complete. Call the register service function for the endpoint */
                        ARC_DEBUG_CODE(ARC_DEBUG_FLAG_STATS, (usb_dev_ptr->STATS.usb_complete_ep_count[temp]++));

#if defined(USB_UNDERRUN_WA)
                        if( (direction == ARC_USB_SEND) && 
                            (((ep_queue_head_ptr->MAX_PKT_LENGTH >> 16) & 0x7FF) > global_wa_threshold) )
                            usbSendComplete(handle, ep_num, FALSE, direction, 
                                   buff_start_address, actual_transfer_length, errors);
                        else
#endif /* USB_UNDERRUN_WA */
                            _usb_device_call_service(handle, ep_num, FALSE, direction, 
                                   buff_start_address, actual_transfer_length, errors);
                        remaining_length = 0;

                        ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_TRANSFER,
                            "comp_%d: fri=0x%x, ep=%d%s, buf=%p, size=%d, xd=%p, dTD=%p %p %p, COMP=0x%x\n", 
                            usb_dev_ptr->STATS.usb_complete_count & 0xFFFF, 
                            USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_FRINDEX), 
                            (unsigned)ep_num, direction ? "in" : "out", 
                            buff_start_address, actual_transfer_length,
                            temp_xd_ptr, dTD_ptr, temp_dTD_ptr, usb_dev_ptr->EP_DTD_HEADS[temp], (unsigned)bit_pos);

                    } /* Endif */
                    else
                    {
                        ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_TRANSFER, "tr_complete not completed: ep=%d %s\n", 
                                (unsigned)ep_num, direction ? "SEND" : "RECV");
                    }
                    if( (temp_dTD_ptr == NULL) && (usb_dev_ptr->EP_DTD_HEADS[temp] != NULL) )
                    {
/*
                        USB_printf("tr_complete: ep=%d, temp_dTD=%p, dTD_ptr=%p (%p), DTD_HEADS=%p, remain=%d\n",
                                    temp, temp_dTD_ptr, dTD_ptr, 
                                    USB_DTD_PHYS_TO_VIRT(usb_dev_ptr, (uint_32)(USB_32BIT_LE(dTD_ptr->NEXT_TR_ELEM_PTR) & VUSBHS_TD_ADDR_MASK) ),
                                    usb_dev_ptr->EP_DTD_HEADS[temp], remaining_length);
*/
                        dTD_ptr = usb_dev_ptr->EP_DTD_HEADS[temp];
                    }
                    else
                    {
                        dTD_ptr = temp_dTD_ptr;
                    }
                    errors = 0;
                } /* Endwhile */
            } /* Endif */
        } /* Endfor */
        ARC_DEBUG_CODE(ARC_DEBUG_FLAG_STATS, 
               ( {if(usb_dev_ptr->STATS.usb_complete_max_count < counter) 
                            usb_dev_ptr->STATS.usb_complete_max_count = counter;}));
   } /* Endif */
} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_isr
*  Returned Value : None
*  Comments       :
*        Services all the VUSB_HS interrupt sources
*
*END*-----------------------------------------------------------------*/
void _usb_dci_vusb20_isr
   (
      _usb_device_handle handle
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR     usb_dev_ptr;
   VUSB20_REG_STRUCT_PTR        dev_ptr;
   uint_32                      status;

   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;

   ARC_DEBUG_CODE(ARC_DEBUG_FLAG_STATS, (usb_dev_ptr->STATS.usb_isr_count++));

   dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;

   status = USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_STS);
   
   status &= USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_INTR);

   if(status == 0) 
   {
       ARC_DEBUG_CODE(ARC_DEBUG_FLAG_STATS, (usb_dev_ptr->STATS.usb_empty_isr_count++));
       return;
   } /* Endif */
/*
    USB_printf("USB_ISR: FRINDEX=0x%x, status=0x%x, PORTSC=0x%x, EP_SETUP=0x%x, EP_COMPLETE=0x%x\n", 
                    USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_FRINDEX),
                    status, 
                    USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.PORTSCX[0]),
                    USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPT_SETUP_STAT),
                    USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCOMPLETE) );
*/
   /* Clear all the interrupts occured */
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_STS = USB_32BIT_LE(status);
   
   if (status & EHCI_STS_ERR) 
   {
      _usb_dci_vusb20_process_error((pointer)usb_dev_ptr);
      USB_printf("USB process error: status=0x%x\n", status);
      usb_dev_ptr->ERROR_STATE |= (status & 0xFFFF);
   } /* Endif */

   if (status & EHCI_STS_RESET) 
   {
       _usb_dci_vusb20_process_reset((pointer)usb_dev_ptr);
   } /* Endif */
   
   if (status & EHCI_STS_PORT_CHANGE) 
   {
      _usb_dci_vusb20_process_port_change((pointer)usb_dev_ptr);
   } /* Endif */
      
   if (status & EHCI_STS_SOF) 
   {
      _usb_dci_vusb20_process_SOF((pointer)usb_dev_ptr);
   } /* Endif */
   
   if (status & EHCI_STS_INT) 
   {
      _usb_dci_vusb20_process_tr_complete((pointer)usb_dev_ptr);
   } /* Endif */
   
    if (status & EHCI_STS_SUSPEND) 
    {
        _usb_dci_vusb20_process_suspend((pointer)usb_dev_ptr);
    } /* Endif */

} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_process_reset
*  Returned Value : None
*  Comments       :
*        Services reset interrupt
*
*END*-----------------------------------------------------------------*/
void _usb_dci_vusb20_process_reset
   (
      /* [IN] the USB_dev_initialize state structure */
      _usb_device_handle         handle
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR     usb_dev_ptr;
   VUSB20_REG_STRUCT_PTR        dev_ptr;
   uint_32                      temp;
   volatile unsigned long       timeout;

   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;

   ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_ISR, "process_reset\n");
   ARC_DEBUG_CODE(ARC_DEBUG_FLAG_STATS, (usb_dev_ptr->STATS.usb_reset_count++));
   
   dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;

   /* Inform the application so that it can cancel all previously queued transfers */
   _usb_device_call_service(usb_dev_ptr, ARC_USB_SERVICE_BUS_RESET, 0, 0, 0, 0, 0);

#if defined(USB_UNDERRUN_WA)
    _usb_reset_send_queue();
#endif /* USB_UNDERRUN_WA */
   
   /* The address bits are past bit 25-31. Set the address */
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.DEVICE_ADDR &= ~USB_32BIT_LE(0xFE000000);
   
   /* Clear all the setup token semaphores */
   temp = USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPT_SETUP_STAT);
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPT_SETUP_STAT = USB_32BIT_LE(temp);

   /* Clear all the endpoint complete status bits */   
   temp = USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCOMPLETE);
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCOMPLETE = USB_32BIT_LE(temp);
   
    timeout = 0x10000;
    while (USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTPRIME) & 0xFFFFFFFF) 
    {
        timeout--;
        if(timeout <= 0)
        {
            USB_printf("%s: Timeout for ENDPTPRIME = 0x%x\n", __FUNCTION__, 
                (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTPRIME));
            break;
        }

      /* Wait until all ENDPTPRIME bits cleared */
    } /* Endif */
   
   /* Write 1s to the Flush register */
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTFLUSH = USB_32BIT_LE(0xFFFFFFFF);
   
    if( (usb_dev_ptr->ERROR_STATE == 0x0) &&
        (USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.PORTSCX[0]) & 
                                                EHCI_PORTSCX_PORT_RESET) )
    {
        ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_INIT,
                    "USB Bus Reset: fri=0x%x, dev_ptr=%p, STATE=%d, PORTSC=0x%x, CMD=0x%x, ENDPT[0]=0x%x\n", 
                    USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_FRINDEX), 
                    usb_dev_ptr, usb_dev_ptr->USB_STATE,
                    USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.PORTSCX[0]),
                    USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_CMD),
                    USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[0]));

        usb_dev_ptr->BUS_RESETTING = TRUE;
        usb_dev_ptr->USB_STATE = ARC_USB_STATE_POWERED;
    } 
    else 
    { 
        USB_printf("USB Chip reinit: PORTSC=0x%x\n", 
                USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.PORTSCX[0]));

        /* re-initialize */      
        _usb_dci_vusb20_chip_initialize((pointer)usb_dev_ptr);
        ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_INIT, "process_reset, Chip reinit hw\n");
    } /* Endif */
   
    _usb_device_call_service(usb_dev_ptr, ARC_USB_SERVICE_BUS_RESET, 1, 0, 0, 0, 0);

} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_process_suspend
*  Returned Value : None
*  Comments       :
*        Services suspend interrupt
*
*END*-----------------------------------------------------------------*/
void _usb_dci_vusb20_process_suspend
   (
      /* [IN] the USB_dev_initialize state structure */
      _usb_device_handle         handle
   )
{ /* Body */
    USB_DEV_STATE_STRUCT_PTR     usb_dev_ptr;
    VUSB20_REG_STRUCT_PTR        dev_ptr;
   
    usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;

    dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;

    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_ISR, "process_suspend\n");
    ARC_DEBUG_CODE(ARC_DEBUG_FLAG_STATS, (usb_dev_ptr->STATS.usb_suspend_count++));
   
    usb_dev_ptr->USB_DEV_STATE_B4_SUSPEND = usb_dev_ptr->USB_STATE;
   
    usb_dev_ptr->USB_STATE = ARC_USB_STATE_SUSPEND;

    /* Inform the upper layers */
    _usb_device_call_service(usb_dev_ptr, ARC_USB_SERVICE_SLEEP, 0, 0, 0, 0, 0);
   
} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_process_SOF
*  Returned Value : None
*  Comments       :
*        Services SOF interrupt
*
*END*-----------------------------------------------------------------*/
void _usb_dci_vusb20_process_SOF
   (
      /* [IN] the USB_dev_initialize state structure */
      _usb_device_handle         handle
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR                     usb_dev_ptr;
   VUSB20_REG_STRUCT_PTR                        dev_ptr;

   ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_ISR, "process_SOF\n");
   
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
   dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;

   /* Inform the upper layer */   
   _usb_device_call_service(usb_dev_ptr, ARC_USB_SERVICE_SOF, 0, 0, 0, 
      USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_FRINDEX), 0);

} /* EndBody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_process_port_change
*  Returned Value : None
*  Comments       :
*        Services port change detect interrupt
*
*END*-----------------------------------------------------------------*/
void _usb_dci_vusb20_process_port_change
   (
      /* [IN] the USB_dev_initialize state structure */
      _usb_device_handle         handle
   )
{ /* Body */
    USB_DEV_STATE_STRUCT_PTR                     usb_dev_ptr;
    VUSB20_REG_STRUCT_PTR                        dev_ptr;

    usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;

    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_ISR, "process_port_change\n");
    ARC_DEBUG_CODE(ARC_DEBUG_FLAG_STATS, (usb_dev_ptr->STATS.usb_port_change_count++));

    dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;
/*
    USB_printf("port_change: PORTSC=0x%x, DTD_ENTRIES=%d, XD_ENTRIES=%d\n", 
                USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.PORTSCX[0]),
                usb_dev_ptr->DTD_ENTRIES, usb_dev_ptr->XD_ENTRIES);
*/
    if (usb_dev_ptr->BUS_RESETTING) 
    {
        /* Bus reset operation complete */
        usb_dev_ptr->BUS_RESETTING = FALSE;
    } /* Endif */
   
    if (!(USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.PORTSCX[0]) & 
                                                EHCI_PORTSCX_PORT_RESET)) 
    {
        /* Get the speed */
        if (USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.PORTSCX[0]) & 
                                            EHCI_PORTSCX_PORT_HIGH_SPEED) 
        {
            usb_dev_ptr->SPEED = ARC_USB_SPEED_HIGH;
        } 
        else 
        {
            usb_dev_ptr->SPEED = ARC_USB_SPEED_FULL;
        } /* Endif */
/*
        USB_printf("USB %s speed device detected\n", 
                (usb_dev_ptr->SPEED == ARC_USB_SPEED_HIGH) ? "High" : "Full");
*/
        /* Inform the upper layers of the speed of operation */      
        _usb_device_call_service(usb_dev_ptr, ARC_USB_SERVICE_SPEED_DETECTION, 0, 0, 
                                0, usb_dev_ptr->SPEED, 0);
    } /* Endif */
      
    if (USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.PORTSCX[0]) & 
                                                EHCI_PORTSCX_PORT_SUSPEND) 
    {   
        usb_dev_ptr->USB_DEV_STATE_B4_SUSPEND = usb_dev_ptr->USB_STATE;
        usb_dev_ptr->USB_STATE = ARC_USB_STATE_SUSPEND;

        /* Inform the upper layers */
        USB_printf("USB suspend\n");
        _usb_device_call_service(usb_dev_ptr, ARC_USB_SERVICE_SUSPEND, 0, 0, 0, 0, 0);
    } /* Endif */
   
    if (!(USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.PORTSCX[0]) & EHCI_PORTSCX_PORT_SUSPEND) 
                    && (usb_dev_ptr->USB_STATE == ARC_USB_STATE_SUSPEND)) 
    {
        USB_printf("USB resume\n");
        usb_dev_ptr->USB_STATE = usb_dev_ptr->USB_DEV_STATE_B4_SUSPEND;
        /* Inform the upper layers */
        _usb_device_call_service(usb_dev_ptr, ARC_USB_SERVICE_RESUME, 0, 0, 0, 0, 0);

        ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_SUSPEND, "process_port_change, SUCCESSFUL, resumed\n");
        return;
    } /* Endif */
   
    usb_dev_ptr->USB_STATE = ARC_USB_STATE_DEFAULT;
      
} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_process_error
*  Returned Value : None
*  Comments       :
*        Services error interrupt
*
*END*-----------------------------------------------------------------*/
void _usb_dci_vusb20_process_error
   (
      /* [IN] the USB_dev_initialize state structure */
      _usb_device_handle         handle
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR          usb_dev_ptr;

   ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_ERROR, "process_error\n");
   
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
   
   /* Increment the error count */
   usb_dev_ptr->ERRORS++;
   
} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_set_speed_full
*  Returned Value : None
*  Comments       :
*        Force the controller port in full speed mode.
*
*END*-----------------------------------------------------------------*/
void _usb_dci_vusb20_set_speed_full
   (
      /* [IN] the USB_dev_initialize state structure */
      _usb_device_handle         handle,
     
      /* The port number on the device */
      uint_8                     port_number
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR                     usb_dev_ptr;
   VUSB20_REG_STRUCT_PTR                        dev_ptr;
   uint_32                                      port_control;
   
   ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_ANY, "FORCE set_speed_full\n");
   
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
   dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;

   port_control = USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.PORTSCX[port_number]);   
   port_control |= EHCI_PORTSCX_FORCE_FULL_SPEED_CONNECT;
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.PORTSCX[port_number] = USB_32BIT_LE(port_control);
  
} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_suspend_phy
*  Returned Value : None
*  Comments       :
*        Suspends the PHY in low power mode
*
*END*-----------------------------------------------------------------*/
void _usb_dci_vusb20_suspend_phy
   (
      /* [IN] the USB_dev_initialize state structure */
      _usb_device_handle         handle,
     
      /* The port number on the device */
      uint_8                     port_number
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR                     usb_dev_ptr;
   VUSB20_REG_STRUCT_PTR                        dev_ptr;
   uint_32                                      port_control;
      
   ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_SUSPEND, "set_suspend_phy\n");
   
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
   dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;

   port_control = USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.PORTSCX[port_number]);   
   port_control |= EHCI_PORTSCX_PHY_CLOCK_DISABLE;
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.PORTSCX[port_number] = USB_32BIT_LE(port_control);
   
} /* EndBody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_set_address
*  Returned Value : None
*  Comments       :
*        Sets the newly assigned device address
*
*END*-----------------------------------------------------------------*/
void _usb_dci_vusb20_set_address
   (
      /* [IN] the USB_dev_initialize state structure */
      _usb_device_handle         handle,
     
      /* Address of the device assigned by the host */
      uint_8                     address
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR          usb_dev_ptr;
   VUSB20_REG_STRUCT_PTR             dev_ptr;

   ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_ADDR, "set_address: address=%d\n",address);
   
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
   dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;
   
#ifdef SET_ADDRESS_HARDWARE_ASSISTANCE   
   /***********************************************************
   Hardware Rev 4.0 onwards have special assistance built in
   for handling the set_address command. As per the USB specs
   a device should be able to receive the response on a new
   address, within 2 msecs after status phase of set_address is
   completed. Since 2 mili second may be a very small time window
   (on high interrupt latency systems) before software could 
   come to the code below and write the device register,
   this routine will be called in advance when status phase of
   set_address is still not finished. The following line in the
   code will set the bit 24 to '1' and hardware will take
   the address and queue it in an internal buffer. From which
   it will use it to decode the next USB token. Please look
   at hardware rev details for the implementation of this
   assistance.
   
   Also note that writing bit 24 to 0x01 will not break
   any old hardware revs because it was an unused bit.
   ***********************************************************/
   /* The address bits are past bit 25-31. Set the address 
   also set the bit 24 to 0x01 to start hardware assitance*/
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.DEVICE_ADDR = 
      USB_32BIT_LE((uint_32)address << VUSBHS_ADDRESS_BIT_SHIFT) | 
      (0x01 << (VUSBHS_ADDRESS_BIT_SHIFT -1)); 
#else
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.DEVICE_ADDR = 
      USB_32BIT_LE((uint_32)address << VUSBHS_ADDRESS_BIT_SHIFT); 
#endif /* SET_ADDRESS_HARDWARE_ASSISTANCE */
   
   usb_dev_ptr->USB_STATE = ARC_USB_STATE_ADDRESS;
      
} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_get_setup_data
*  Returned Value : None
*  Comments       :
*        Reads the Setup data from the 8-byte setup buffer
*
*END*-----------------------------------------------------------------*/
void _usb_dci_vusb20_get_setup_data
   (
      /* [IN] the USB_dev_initialize state structure */
      _usb_device_handle         handle,
            
      /* [IN] the Endpoint number */
      uint_8                     ep_num,
            
      /* [OUT] address of the buffer to read the setup data into */
      uint_8_ptr                  buffer_ptr
   )
{ /* Body */
    USB_DEV_STATE_STRUCT_PTR                    usb_dev_ptr;
    volatile VUSB20_REG_STRUCT_PTR              dev_ptr;
    volatile VUSB20_EP_QUEUE_HEAD_STRUCT_PTR    ep_queue_head_ptr;
    volatile boolean                            read_safe = FALSE;                      
    volatile unsigned long                      timeout;

   
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
   dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;

   /* Get the endpoint queue head */
   ep_queue_head_ptr = (VUSB20_EP_QUEUE_HEAD_STRUCT_PTR)usb_dev_ptr->EP_QUEUE_HEAD_PTR + 
                                                                2*ep_num + ARC_USB_RECV;

   /********************************************************************
   CR 1219. Hardware versions 2.3+ have a implementation of tripwire 
   semaphore mechanism that requires that we read the contents of 
   QH safely by using the semaphore. Read the USBHS document to under
   stand how the code uses the semaphore mechanism. The following are
   the steps in brief
   
   1. USBCMD Write ‘1’ to Setup Tripwire in register.
   2. Duplicate contents of dQH.StatusBuffer into local software byte
      array.
   3  Read Setup TripWire in register. (if set - continue; if
      cleared goto 1.)
   4. Write '0' to clear Setup Tripwire in register.
   5. Process setup packet using local software byte array copy and
      execute status/handshake phases.
   
           
   ********************************************************************/
    timeout = 0x100000;
    while(!read_safe)
    {
        /*********************************************************
        start with setting the semaphores
        *********************************************************/

        dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_CMD |= 
                                                USB_32BIT_LE(EHCI_CMD_SETUP_TRIPWIRE_SET);

        /* Copy the setup packet to private buffer */
        USB_memcopy((uint_8_ptr)ep_queue_head_ptr->SETUP_BUFFER, buffer_ptr, 8);

        /*********************************************************
        If setup tripwire semaphore is cleared by hardware it means
        that we have a danger and we need to restart.
        else we can exit out of loop safely.
        *********************************************************/
        if(USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_CMD) &
                                                   EHCI_CMD_SETUP_TRIPWIRE_SET)
        {
            read_safe = TRUE; /* we can proceed exiting out of loop*/
        }
        if(timeout <= 0)
        {
            USB_printf("%s: Timeout for SETUP_TRIPWIRE = 0x%x\n", __FUNCTION__, 
                        (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_CMD));
            break;
        }
    }
    ARC_DEBUG_CODE(ARC_DEBUG_FLAG_STATS, (usb_dev_ptr->STATS.usb_read_setup_count++));
  
   /*********************************************************
   Clear the semaphore bit now
   *********************************************************/
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_CMD &=
                                    USB_32BIT_LE(EHCI_CMD_SETUP_TRIPWIRE_CLEAR);                         
   
   /* Clear the bit in the ENDPTSETUPSTAT */
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPT_SETUP_STAT = USB_32BIT_LE(1 << ep_num);
   
} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_init_endpoint
*  Returned Value : None
*  Comments       :
*        Initializes the specified endpoint and the endpoint queue head
*
*END*-----------------------------------------------------------------*/
uint_8 _usb_dci_vusb20_init_endpoint
   (
      /* [IN] the USB_dev_initialize state structure */
      _usb_device_handle         handle,
            
      /* [IN] the transaction descriptor address */
      XD_STRUCT_PTR              xd_ptr
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR                     usb_dev_ptr;
   VUSB20_REG_STRUCT_PTR                        dev_ptr;
   VUSB20_EP_QUEUE_HEAD_STRUCT _PTR_            ep_queue_head_ptr;
   uint_32                                      val, bit_pos;
   
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
   dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;

   /* Get the endpoint queue head address */
   ep_queue_head_ptr = (VUSB20_EP_QUEUE_HEAD_STRUCT_PTR)usb_dev_ptr->EP_QUEUE_HEAD_PTR + 
                                                    2*xd_ptr->EP_NUM + xd_ptr->BDIRECTION;
      
   bit_pos = (1 << (16 * xd_ptr->BDIRECTION + xd_ptr->EP_NUM));

   /* Check if the Endpoint is Primed */
   if ((!(USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTPRIME) & bit_pos)) && 
       (!(USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTSTATUS) & bit_pos))) 
   { 
      /* Set the max packet length, interrupt on Setup and Mult fields */
      if (xd_ptr->EP_TYPE == ARC_USB_ISOCHRONOUS_ENDPOINT) 
      {
         /* Mult bit should be set for isochronous endpoints */
         ep_queue_head_ptr->MAX_PKT_LENGTH = USB_32BIT_LE((xd_ptr->WMAXPACKETSIZE << 16) | 
            ((xd_ptr->MAX_PKTS_PER_UFRAME ?  xd_ptr->MAX_PKTS_PER_UFRAME : 1) << 
            VUSB_EP_QUEUE_HEAD_MULT_POS));
      } 
      else 
      {
         if (xd_ptr->EP_TYPE != ARC_USB_CONTROL_ENDPOINT) 
         {
             /* BULK or INTERRUPT */
            ep_queue_head_ptr->MAX_PKT_LENGTH = USB_32BIT_LE((xd_ptr->WMAXPACKETSIZE << 16) | 
               (xd_ptr->DONT_ZERO_TERMINATE ? VUSB_EP_QUEUE_HEAD_ZERO_LEN_TER_SEL : 0));
         } 
         else 
         {
             /* CONTROL */
            ep_queue_head_ptr->MAX_PKT_LENGTH = USB_32BIT_LE((xd_ptr->WMAXPACKETSIZE << 16) | 
                                                    VUSB_EP_QUEUE_HEAD_IOS);
         } /* Endif */
      } /* Endif */
      
      /* Enable the endpoint for Rx or Tx and set the endpoint type */
      val = dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[xd_ptr->EP_NUM];
      if(xd_ptr->BDIRECTION  == ARC_USB_SEND)
      {
          val &= ~(USB_32BIT_LE(EHCI_EPCTRL_TX_ALL_MASK));
          val |= USB_32BIT_LE((EHCI_EPCTRL_TX_ENABLE | EHCI_EPCTRL_TX_DATA_TOGGLE_RST) | 
                          (xd_ptr->EP_TYPE << EHCI_EPCTRL_TX_EP_TYPE_SHIFT));
      }
      else
      {
          val &= ~(USB_32BIT_LE(EHCI_EPCTRL_RX_ALL_MASK));
          val |= USB_32BIT_LE((EHCI_EPCTRL_RX_ENABLE | EHCI_EPCTRL_RX_DATA_TOGGLE_RST) | 
                          (xd_ptr->EP_TYPE << EHCI_EPCTRL_RX_EP_TYPE_SHIFT));
      }
      dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[xd_ptr->EP_NUM] = val;
      
      /* Implement Guideline (GL# USB-7) The unused endpoint type must  */
      /* be programmed to bulk.                                         */
      if( (dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[xd_ptr->EP_NUM] & 
            USB_32BIT_LE(EHCI_EPCTRL_RX_ENABLE)) == 0)
      {
          dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[xd_ptr->EP_NUM] |= 
              USB_32BIT_LE(ARC_USB_BULK_ENDPOINT << EHCI_EPCTRL_RX_EP_TYPE_SHIFT);
      }

      if( (dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[xd_ptr->EP_NUM] & 
            USB_32BIT_LE(EHCI_EPCTRL_TX_ENABLE)) == 0)
      {
          dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[xd_ptr->EP_NUM] |= 
              USB_32BIT_LE(ARC_USB_BULK_ENDPOINT << EHCI_EPCTRL_TX_EP_TYPE_SHIFT);
      }

      ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_INIT,
                    "init ep #%d %s: type=0x%x, EPCTRLX=0x%x, SETUP=0x%x, PRIME=0x%x, STATUS=0x%x, COMPL=0x%x\n", 
                        xd_ptr->EP_NUM, xd_ptr->BDIRECTION ? "SEND" : "RECV", xd_ptr->EP_TYPE, 
                        (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[xd_ptr->EP_NUM]),
                        (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPT_SETUP_STAT),
                        (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTPRIME),
                        (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTSTATUS),
                        (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCOMPLETE) );
   } 
   else 
   { 
      USB_printf("ep=%d %s: Init ERROR: ENDPTPRIME=0x%x, ENDPTSTATUS=0x%x, bit_pos=0x%x\n",
                (unsigned)xd_ptr->EP_NUM, xd_ptr->BDIRECTION ? "SEND" : "RECV",
                (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTPRIME),
                (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTSTATUS), 
                (unsigned)bit_pos);
        return USBERR_EP_INIT_FAILED;
   } /* Endif */
      
   return USB_OK;
   
} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_get_transfer_status
*  Returned Value : USB_OK or error code
*  Comments       :
*        Gets the status of a transfer
*
*END*-----------------------------------------------------------------*/
uint_8 _usb_dci_vusb20_get_transfer_status
   (
      /* [IN] the USB_dev_initialize state structure */
      _usb_device_handle         handle,
     
      /* [IN] the Endpoint number */
      uint_8                     ep_num,
            
      /* [IN] direction */
      uint_8                     direction
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR                     usb_dev_ptr;
   VUSB20_EP_TR_STRUCT_PTR                      dTD_ptr;
   XD_STRUCT_PTR                                xd_ptr;
   uint_8                                       status;
   
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
  
   /* Unlink the dTD */
   dTD_ptr = usb_dev_ptr->EP_DTD_HEADS[2*ep_num + direction];

   if (dTD_ptr) 
   {
      /* Get the transfer descriptor for the dTD */
      xd_ptr = (XD_STRUCT_PTR)dTD_ptr->SCRATCH_PTR->XD_FOR_THIS_DTD;
      status = xd_ptr->BSTATUS;
   } 
   else 
   {
      status = ARC_USB_STATUS_IDLE;
   } /* Endif */
   
   return (status);

} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_get_transfer_details
*  Returned Value : pointer to structure that has details for transfer
*        Gets the status of a transfer
*
*END*-----------------------------------------------------------------*/
XD_STRUCT_PTR  _usb_dci_vusb20_get_transfer_details
   (
      /* [IN] the USB_dev_initialize state structure */
      _usb_device_handle         handle,
     
      /* [IN] the Endpoint number */
      uint_8                     ep_num,
            
      /* [IN] direction */
      uint_8                     direction
   )
{ /* Body */
   USB_DEV_STATE_STRUCT_PTR                     usb_dev_ptr;
   VUSB20_REG_STRUCT_PTR                        dev_ptr;
   VUSB20_EP_TR_STRUCT_PTR                      dTD_ptr, temp_dTD_ptr;
   XD_STRUCT_PTR                                xd_ptr;
   uint_32                                      temp, remaining_bytes;
   VUSB20_EP_QUEUE_HEAD_STRUCT_PTR              ep_queue_head_ptr;

   
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
   dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;
   temp = (2*ep_num + direction);

   /* get a pointer to QH for this endpoint */   
   ep_queue_head_ptr = (VUSB20_EP_QUEUE_HEAD_STRUCT_PTR)usb_dev_ptr->EP_QUEUE_HEAD_PTR + temp;

   ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_TRACE, "get_transfer_details\n");
   
   /* Unlink the dTD */
   dTD_ptr = usb_dev_ptr->EP_DTD_HEADS[2*ep_num + direction];

   if (dTD_ptr) 
   {      
      /* Get the transfer descriptor for the dTD */
      xd_ptr = (XD_STRUCT_PTR)dTD_ptr->SCRATCH_PTR->XD_FOR_THIS_DTD;
      if(!xd_ptr) return NULL;
      
      /* Initialize the transfer length field */
      xd_ptr->WSOFAR =0;
      remaining_bytes =0;
      
      /*if length of this transfer is greater than 20K
      we have multiple DTDs to count */
      if(xd_ptr->WTOTALLENGTH > VUSB_EP_MAX_LENGTH_TRANSFER)
      {
         /* it is a valid DTD. We should parse all DTDs for this XD
         and find the total bytes used so far */
         temp_dTD_ptr = dTD_ptr;
      
         /*loop through the list of DTDS until an active DTD is found
         or list has finished */
         while(!(USB_32BIT_LE(dTD_ptr->NEXT_TR_ELEM_PTR) & VUSBHS_TD_NEXT_TERMINATE))         
         {
            
            /**********************************************************
            If this DTD has been overlayed, we take the actual length 
            from QH.
            **********************************************************/

            if ((uint_32)(USB_32BIT_LE(ep_queue_head_ptr->CURR_DTD_PTR) & VUSBHS_TD_ADDR_MASK) ==
                                     USB_DTD_VIRT_TO_PHYS(usb_dev_ptr, temp_dTD_ptr) )
            {
                remaining_bytes += 
                  ((USB_32BIT_LE(ep_queue_head_ptr->SIZE_IOC_INT_STS) & VUSB_EP_TR_PACKET_SIZE) >> 16);
            }
            else
            {
               /* take the length from DTD itself */
                remaining_bytes += 
                  ((USB_32BIT_LE(temp_dTD_ptr->SIZE_IOC_STS) & VUSB_EP_TR_PACKET_SIZE) >> 16);
            }
   
            dTD_ptr = temp_dTD_ptr;
             
            /* Get the address of the next dTD */
            temp_dTD_ptr = (VUSB20_EP_TR_STRUCT_PTR)USB_DTD_PHYS_TO_VIRT(usb_dev_ptr, 
                                (uint_32)(USB_32BIT_LE(temp_dTD_ptr->NEXT_TR_ELEM_PTR) & VUSBHS_TD_ADDR_MASK) );
         }
         xd_ptr->WSOFAR = xd_ptr->WTOTALLENGTH - remaining_bytes;
      }
      else
      {
         /*look at actual length from QH*/
         xd_ptr->WSOFAR = xd_ptr->WTOTALLENGTH - 
            ((USB_32BIT_LE(ep_queue_head_ptr->SIZE_IOC_INT_STS) & VUSB_EP_TR_PACKET_SIZE) >> 16);         
      }      
   } 
   else 
   {
      xd_ptr = NULL;
   } /* Endif */
   
   return (xd_ptr);

} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_deinit_endpoint
*  Returned Value : None
*  Comments       :
*        Disables the specified endpoint and the endpoint queue head
*
*END*-----------------------------------------------------------------*/
uint_8 _usb_dci_vusb20_deinit_endpoint
   (
      /* [IN] the USB_dev_initialize state structure */
      _usb_device_handle         handle,
            
      /* [IN] the Endpoint number */
      uint_8                     ep_num,
            
      /* [IN] direction */
      uint_8                     direction
   )
{ /* Body */
    USB_DEV_STATE_STRUCT_PTR        usb_dev_ptr;
    VUSB20_REG_STRUCT_PTR           dev_ptr;
    VUSB20_EP_QUEUE_HEAD_STRUCT*    ep_queue_head_ptr;
    uint_32                         bit_pos;
    uint_8                          status = USB_OK;
   
    usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
    dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;
   
    /* Get the endpoint queue head address */
    ep_queue_head_ptr = (VUSB20_EP_QUEUE_HEAD_STRUCT_PTR)usb_dev_ptr->EP_QUEUE_HEAD_PTR +
                                                                    (2*ep_num + direction);
      
    bit_pos = (1 << (16 * direction + ep_num));

    ARC_DEBUG_TRACE(ARC_DEBUG_FLAG_INIT,
               "deinit ep #%d-%s: bit_pos=0x%x, EPCTRLX=0x%x, SETUP=0x%x, PRIME=0x%x, STATUS=0x%x, COMPL=0x%x\n", 
                   ep_num, direction ? "SEND" : "RECV", bit_pos,
                   (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[ep_num]),
                   (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPT_SETUP_STAT),
                   (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTPRIME),
                   (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTSTATUS),
                   (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCOMPLETE) );
      
    /* Check if the Endpoint is Primed */
    if( ((USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTPRIME) & bit_pos)) || 
        ((USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTSTATUS) & bit_pos)) ) 
    { 
        USB_printf("ep=%d %s: Deinit ERROR: ENDPTPRIME=0x%x, ENDPTSTATUS=0x%x, bit_pos=0x%x\n",
                (unsigned)ep_num, direction ? "SEND" : "RECV",
                (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTPRIME),
                (unsigned)USB_32BIT_LE(dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTSTATUS), 
                (unsigned)bit_pos);
        status = USBERR_EP_DEINIT_FAILED;
    }

    /* Reset the max packet length and the interrupt on Setup */
    ep_queue_head_ptr->MAX_PKT_LENGTH = 0;
      
    /* Disable the endpoint for Rx or Tx and reset the endpoint type */
    dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.ENDPTCTRLX[ep_num] &= 
         USB_32BIT_LE( ~((direction ? EHCI_EPCTRL_TX_ENABLE : EHCI_EPCTRL_RX_ENABLE) | 
                        (direction ? EHCI_EPCTRL_TX_TYPE : EHCI_EPCTRL_RX_TYPE)));
      
   return status;
}


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_shutdown
*  Returned Value : None
*  Comments       :
*        Shuts down the VUSB_HS Device
*
*END*-----------------------------------------------------------------*/
void _usb_dci_vusb20_shutdown
   (
      /* [IN] the USB_dev_initialize state structure */
      _usb_device_handle         handle
   )
{ /* Body */
    USB_DEV_STATE_STRUCT_PTR                     usb_dev_ptr;
    VUSB20_REG_STRUCT_PTR                        dev_ptr;
   
    usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
    dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;
   
    /* Disable interrupts */
    dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_INTR &= 
      ~(USB_32BIT_LE(EHCI_INTR_INT_EN | EHCI_INTR_ERR_INT_EN |
      EHCI_INTR_PORT_CHANGE_DETECT_EN | EHCI_INTR_RESET_EN));
      
    USB_uncached_memfree(usb_dev_ptr->EP_QUEUE_HEAD_BASE, 
                        usb_dev_ptr->EP_QUEUE_HEAD_SIZE, 
                        usb_dev_ptr->EP_QUEUE_HEAD_PHYS);

    USB_uncached_memfree(usb_dev_ptr->DTD_BASE_PTR, 
                        usb_dev_ptr->DTD_SIZE, 
                        usb_dev_ptr->DTD_BASE_PHYS);

    USB_memfree(usb_dev_ptr->SCRATCH_STRUCT_BASE);

    USB_printf("USB shutdown: usb_dev_ptr=%p\n", usb_dev_ptr);

   /* Reset the Run the bit in the command register to stop VUSB */
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_CMD &= ~USB_32BIT_LE(EHCI_CMD_RUN_STOP);
   
   /* Reset the controller to get default values */
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_CMD = USB_32BIT_LE(EHCI_CMD_CTRL_RESET);
              
} /* EndBody */
/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_stop
*  Returned Value : None
*  Comments       :
*        Stop USB device controller
*
*END*-----------------------------------------------------------------*/
void _usb_dci_vusb20_stop(_usb_device_handle handle)
{
   USB_DEV_STATE_STRUCT_PTR     usb_dev_ptr;
   VUSB20_REG_STRUCT_PTR        dev_ptr;
   
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
   dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;

    /* Disable interrupts */
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_INTR &= 
            ~(USB_32BIT_LE(EHCI_INTR_INT_EN | EHCI_INTR_ERR_INT_EN |
                           EHCI_INTR_PORT_CHANGE_DETECT_EN | EHCI_INTR_RESET_EN));

   /* Reset the Run the bit in the command register to stop VUSB */
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_CMD &= ~USB_32BIT_LE(EHCI_CMD_RUN_STOP);
}


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dci_vusb20_start
*  Returned Value : None
*  Comments       :
*        Start USB device controller
*
*END*-----------------------------------------------------------------*/
void _usb_dci_vusb20_start(_usb_device_handle handle)
{
   USB_DEV_STATE_STRUCT_PTR     usb_dev_ptr;
   VUSB20_REG_STRUCT_PTR        dev_ptr;
   
   usb_dev_ptr = (USB_DEV_STATE_STRUCT_PTR)handle;
   dev_ptr = (VUSB20_REG_STRUCT_PTR)usb_dev_ptr->DEV_PTR;

      /* Enable interrupts */
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_INTR = USB_32BIT_LE(
                           EHCI_INTR_INT_EN
                         | EHCI_INTR_ERR_INT_EN 
                         | EHCI_INTR_PORT_CHANGE_DETECT_EN 
                         | EHCI_INTR_RESET_EN 
                         | EHCI_INTR_DEVICE_SUSPEND
                       /*  
                         | EHCI_INTR_SOF_UFRAME_EN 
                        */
                         );
   
   usb_dev_ptr->USB_STATE = ARC_USB_STATE_UNKNOWN;
   
   /* Set the Run bit in the command register */
   dev_ptr->REGISTERS.OPERATIONAL_DEVICE_REGISTERS.USB_CMD = USB_32BIT_LE(EHCI_CMD_RUN_STOP);
}

/* EOF */

