/**
 * file usbprivateconstants.h
 *
 * author Intel Corporation
 * date 30-OCT-2001
 *
 * This file containes the private constants used by the USB driver, mostly bit masks
 *
 * 
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
 */

/*
 * Private constants used by the USB Driver's support API
 *
 */

#ifndef usbprivateconstants_H
#define usbprivateconstants_H

typedef enum /* USBToken */
{
    UNKNOWN_TOKEN = 0,
    SETUP_TOKEN,
    IN_TOKEN,
    OUT_TOKEN
} USBToken;

typedef enum /* EP0State */
{
    IDLE = 0,
    ACTIVE_IN,
    ACTIVE_OUT,
    END_IN_XFER,
    END_OUT_XFER
} EP0State;

typedef enum /* USBControlTransfer */
{
    UNKNOWN_TRANSFER = 0,
    CONTROL_READ,
    CONTROL_WRITE,
    CONTROL_NO_DATA
} USBControlTransfer;

/* Masks for UDC Registers */

/* USB Control Register (UDCCR) */
#define UDC_UDCCR_UDE					( 0x1 << 0 )	/* UDC enabled */
#define UDC_UDCCR_UDA					( 0x1 << 1 )	/* READ-ONLY: udc is active */
#define UDC_UDCCR_RSM					( 0x1 << 2 )	/* Forces the usb out of suspend state */
#define UDC_UDCCR_RESIR					( 0x1 << 3 )	/* UDC received resume signalling from host */
#define UDC_UDCCR_SUSIR					( 0x1 << 4 )	/* UDC receive suspend signalling from host */
#define UDC_UDCCR_SRM					( 0x1 << 5 )	/* Suspend/Resume interrupt disabled */
#define UDC_UDCCR_RSTIR					( 0x1 << 6 )	/* Set when the host issues a UDC reset */
#define UDC_UDCCR_REM					( 0x1 << 7 )	/* Reset interrupt disabled */

/* UDC Endpoint 0 Control/Status Register (UDCCS0) */
#define UDC_UDCCS0_OPR					( 0x1 << 0 )	/* OUT packet to endpoint zero received */
#define UDC_UDCCS0_IPR					( 0x1 << 1 )	/* Packet has been written to endpoint zero FIFO */
#define UDC_UDCCS0_FTF					( 0x1 << 2 )	/* Flush the Tx FIFO */
#define UDC_UDCCS0_DRWF                 ( 0x1 << 3 )    /**<  Device remote wakeup feature */
#define UDC_UDCCS0_SST					( 0x1 << 4 )	/* UDC sent stall handshake */
#define UDC_UDCCS0_FST					( 0x1 << 5 )	/* Force the UDC to issue a stall handshake */
#define UDC_UDCCS0_RNE					( 0x1 << 6 )	/* There is unread data in the Rx FIFO */
#define UDC_UDCCS0_SA					( 0x1 << 7 )	/* Current packet in FIFO is part of UDC setup command */

/* UDC IN Endpoint Control/Status Register (UDCCS_IN) */
#define UDC_UDCCS_TFS_IN				( 0x1 << 0 )	/* Tx FIFO has room for at least one packet */
#define UDC_UDCCS_TPC_IN				( 0x1 << 1 )	/* Packet sent and err/status bits valid */
#define UDC_UDCCS_TUR_IN				( 0x1 << 3 )	/* Tx FIFO experienced underrun */
#define UDC_UDCCS_SST_IN				( 0x1 << 4 )	/* Write 1 to clear.  Stall was sent */
#define UDC_UDCCS_FST_IN				( 0x1 << 5 )	/* Issue stall handshake */
#define UDC_UDCCS_TSP_IN				( 0X1 << 7 )	/* Short packet ready for transmission */

/* UDC OUT Endpoint Control/Status Register (UDCCS_OUT) */
#define UDC_UDCCS_RFS_OUT				( 0x1 << 0 )	/* Rx FIFO has 1 or more packets */
#define UDC_UDCCS_RPC_OUT				( 0x1 << 1 )	/* Rx packet received and err/stats valid */
#define UDC_UDCCS_DME_OUT				( 0x1 << 3 )	/* DMA Enable */
#define UDC_UDCCS_SST_OUT				( 0x1 << 4 )	/* Stall handshake was sent */
#define UDC_UDCCS_FST_OUT				( 0x1 << 5 )	/* Issue stall handshake to OUT tokens */
#define UDC_UDCCS_RNE_OUT				( 0x1 << 6 )	/* Receive FIFO is not empty */
#define UDC_UDCCS_RSP_OUT				( 0x1 << 7 )	/* Short packet ready for reading */

/* Used both for IN and OUT endpoints to flush the FIFOs */
#define UDC_UDCCS_FTF                   ( 0x1 << 2 )    /* Flush Rx/Tx FIFO */

/* UDC Control Endpoint special bits for Isochronous endpoints */
#define UDC_UDCCS_ROF_OUT               ( 0x1 << 2 )    /* Receive overflow */

/* UDC Interrupt Control Register 0 (UICR0) */
#define UDC_UICR0_IM0					( 0x1 << 0 )	/* Endpoint 0 interrupt disabled */
#define UDC_UICR0_IM1					( 0x1 << 1 )	/* Endpoint 1 Tx interrupt disabled */
#define UDC_UICR0_IM2					( 0x1 << 2 )	/* Endpoint 2 Rx interrupt disabled */
#define UDC_UICR0_IM3					( 0x1 << 3 )	/* Endpoint 3 Tx interrupt disabled */
#define UDC_UICR0_IM4					( 0x1 << 4 )	/* Endpoint 4 Rx interrupt disabled */
#define UDC_UICR0_IM5					( 0x1 << 5 )	/* Endpoint 5 Tx interrupt disabled */
#define UDC_UICR0_IM6					( 0x1 << 6 )	/* Endpoint 6 Tx interrupt disabled */
#define UDC_UICR0_IM7					( 0x1 << 7 )	/* Endpoint 7 Rx interrupt disabled */

/* UDC Interrupt Control Register 1 (UICR1) */
#define UDC_UICR1_IM8					( 0x1 << 0 )	/* Endpoint 8 Tx interrupt disabled */
#define UDC_UICR1_IM9					( 0x1 << 1 )	/* Endpoint 9 Rx interrupt disabled */
#define UDC_UICR1_IM10					( 0x1 << 2 )	/* Endpoint 10 Rx interrupt disabled */
#define UDC_UICR1_IM11					( 0x1 << 3 )	/* Endpoint 11 Tx interrupt disabled */
#define UDC_UICR1_IM12					( 0x1 << 4 )	/* Endpoint 12 Rx interrupt disabled */
#define UDC_UICR1_IM13					( 0x1 << 5 )	/* Endpoint 13 Tx interrupt disabled */
#define UDC_UICR1_IM14					( 0x1 << 6 )	/* Endpoint 14 Rx interrupt disabled */
#define UDC_UICR1_IM15					( 0x1 << 7 )	/* Endpoint 15 Rx interrupt disabled */

/* UDC Status/Interrupt Register 0 (UISR0) */
#define UDC_UISR0_IR0					( 0x1 << 0 )	/* Endpoint 0 needs service */
#define UDC_UISR0_IR1					( 0x1 << 0 )	/* Endpoint 1 needs service */
#define UDC_UISR0_IR2					( 0x1 << 0 )	/* Endpoint 2 needs service */
#define UDC_UISR0_IR3					( 0x1 << 0 )	/* Endpoint 3 needs service */
#define UDC_UISR0_IR4					( 0x1 << 0 )	/* Endpoint 4 needs service */
#define UDC_UISR0_IR5					( 0x1 << 0 )	/* Endpoint 5 needs service */
#define UDC_UISR0_IR6					( 0x1 << 0 )	/* Endpoint 6 needs service */
#define UDC_UISR0_IR7					( 0x1 << 0 )	/* Endpoint 7 needs service */

/* UDC Status/Interrupt Register 1 (UISR1) */
#define UDC_UISR1_IR8					( 0x1 << 0 )	/* Endpoint 8 needs service */
#define UDC_UISR1_IR9					( 0x1 << 0 )	/* Endpoint 9 needs service */
#define UDC_UISR1_IR10					( 0x1 << 0 )	/* Endpoint 10 needs service */
#define UDC_UISR1_IR11					( 0x1 << 0 )	/* Endpoint 11 needs service */
#define UDC_UISR1_IR12					( 0x1 << 0 )	/* Endpoint 12 needs service */
#define UDC_UISR1_IR13					( 0x1 << 0 )	/* Endpoint 13 needs service */
#define UDC_UISR1_IR14					( 0x1 << 0 )	/* Endpoint 14 needs service */
#define UDC_UISR1_IR15					( 0x1 << 0 )	/* Endpoint 15 needs service */

/* UDC Frame Number High Register (UFNHR) */
#define UDC_UFNHR_FN_SHIFT			    8
#define UDC_UFNHR_FN_MASK			    ( 0x7 )
                                                        /* Used to read the 3 most significant bits of the
                                                         *  11-bit frame numbe associated with last SOF 
                                                         */
#define UDC_UFNHR_SIM					( 0x1 << 6 )	/* SOF(start of frame) interrupt disabled */
#define UDC_UFNHR_SIR					( 0x1 << 7 )	/* SOF(start of frame) interrupt received */


/* UDC Frame Number Low Register (UFNLR) */
#define UDC_UFNLR_FN_MASK			    ( 0xff )        /* Used to read the 8 least significant bits of the
                                                         *  11-bit frame number associated with last SOF 
                                                         */

/* UDC Byte Count Register (UBCR) */
#define UDC_UBC_BYTECNT_MASK			( 0xff << 0 )	/* Used to read the number of bytes
                                                         *  remaining in the input buffer of
                                                         *  any endpoint 
                                                         */


/*  UDC Endpoint Data Register (UDDR) */

/*  The Endpoint FIFOs are composed of 32 bit words of which only the lower 8 bits are used.
    Use this mask to ensure that FIFO reads and writes are not affected by the unused bits 8-31 */
#define UDC_UDDR_RW_MASK		( 0xff << 0 )	/* Used to read bottom of Endpoint 0 data
                                                 * currently being loaded.  Also used to
                                                 * mask data to be written to top of
                                                 * endpoint 0 data
                                                 */


#define UDC_ENABLE_ALL_INT		( 0x0  )    	/* Used to enable all int for EP0-7 and EP8-15 */
#define UDC_DISABLE_ALL_INT		( 0xff )        /* Used to disable all int for EP0-7 and EP8-15 */
#define UDC_CLEAR_ALL_INT       ( 0xff )        /* Used to clear all intterupts for EP0-7 and EP8-15 */
#define UDC_DMA_ENABLED			( 0x1 )		    /* Used to enable DMA transfer for Endpoint  */
#define UDC_NO_DMA			    ( 0x0 )		    /* Used to disable DMA transfer for Endpoint  */
#define UDC_DME_SET			    ( 0x1 )	
#define UDC_INT_MASKED			( 0x1 )
#define UDC_SETUP	 		    0x81		    /* Used to identify a Setup transaction  */
#define UDC_TYPE_MASK           (0x3 << 5)      /* Used to identify a Request Type */
#define UDC_DIRECTION_MASK      (0x1 << 7)      /* Used to identify the transfer direction */

#define HOST_TO_DEVICE 0
#define DEVICE_TO_HOST 1

#define USB_DEVICE_CONTEXT_CHECK_PATTERN   ( 0xff00aa33)   /* Check pattern used to validate device contexts */

#endif /* usbprivateconstants_H */

