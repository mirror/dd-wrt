/* 
 * @file IxUART.c
 * @author Intel Corporation
 * @date 15-OCT-2001
 * 
 * @brief UART Access driver for the Intel IXP4XX.
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

/**
 * @addtogroup UART API
 * @{
 */


#include "IxOsal.h"
#include "IxUART_p.h"
#include "IxUART.h"


#define IX_UART_REG_WRITE(pUART, reg, data) \
(IX_OSAL_WRITE_LONG((VUINT32 *)((UINT32)(pUART)->addr + ((reg) * IX_UART_REG_DELTA)), (data)))

#define IX_UART_REG_READ(pUART, reg, val) \
((val) = IX_OSAL_READ_LONG((VUINT32 *)((UINT32)(pUART)->addr + ((reg) * IX_UART_REG_DELTA))))

#define IX_UART_CHECK_REF	\
{				\
    if(pUART == NULL)		\
    {				\
	return (IX_FAIL);	\
    }				\
}


/* Local prototypes */
PRIVATE IX_STATUS ixUARTBaudSet(ixUARTDev* pUART, UINT32 baud);
PRIVATE IX_STATUS ixUARTOptsSet(ixUARTDev* pUART, UINT32 options);
PRIVATE IX_STATUS ixUARTModeSet(ixUARTDev* pUART, ixUARTMode mode);
PRIVATE void ixUARTErr(ixUARTDev* pUART, char status);


PUBLIC IX_STATUS ixUARTInit(ixUARTDev* pUART)
{
    IX_UART_CHECK_REF;

    /* Initialise counter statistics */
    pUART->stats.rxCount = 0;
    pUART->stats.txCount = 0;
    pUART->stats.overrunErr = 0;
    pUART->stats.parityErr = 0;
    pUART->stats.framingErr = 0;
    pUART->stats.breakErr = 0;

    /* Set the default hardware options */
    pUART->options = IX_UART_DEF_OPTS;
    pUART->mode = POLLED;
    pUART->fifoSize = IX_UART_DEF_XMIT;
    pUART->freq = IX_UART_XTAL;

    /* Enable the UART */
    IX_UART_REG_WRITE(pUART, IX_IER, IX_IER_UUE);

    /* Set the baud rate */
    if((pUART->baudRate < IX_UART_MIN_BAUD) || (pUART->baudRate > IX_UART_MAX_BAUD))
    {
	pUART->baudRate = IX_UART_DEF_BAUD;
    }
	
    if(ixUARTBaudSet(pUART, pUART->baudRate) == IX_FAIL)
    {
	return IX_FAIL;
    }
  
    if(ixUARTOptsSet(pUART, pUART->options) == IX_FAIL)
    {
	return IX_FAIL;
    }

    /* Enable FIFOs */
    IX_UART_REG_WRITE(pUART, IX_FCR, (IX_FCR_RESETRF | IX_FCR_RESETTF | IX_FCR_TRFIFOE));
    
    return IX_SUCCESS;
}


PUBLIC IX_STATUS ixUARTIoctl(ixUARTDev* pUART, int cmd, void* arg)
{
    int status = IX_SUCCESS;

    IX_UART_CHECK_REF;

    switch (cmd)
    {
	case IX_BAUD_SET:
	    if (*(int *)arg < IX_UART_MIN_BAUD || *(int *)arg > IX_UART_MAX_BAUD)
	    {
		status = IX_FAIL;		/* baud rate out of range */
	    }
	    else
	    {
	        status = ixUARTBaudSet(pUART, *(int *)arg);
	    }
	    break;

        case IX_BAUD_GET:
            *(int *)arg = pUART->baudRate;
            break; 

        case IX_MODE_SET:
	    status = ixUARTModeSet(pUART, *(int *)arg);
            break;          

        case IX_MODE_GET:
            *(ixUARTMode *)arg = pUART->mode;
            break;

        case IX_OPTS_SET:
    	    status = ixUARTOptsSet(pUART, *(int *)arg);
    	    break;

        case IX_OPTS_GET:
            *(int *)arg = pUART->options;
            break;

        case IX_STATS_GET:
	    *(ixUARTStats *)arg = pUART->stats;
            break;
	
        default:
            status = IX_FAIL;
    }

    return status;
}

#ifdef _DIAB_TOOL
__asm volatile void _RTODisableWait(UINT32 pRTO)
{ 
% reg pRTO;
! "r1"               /* scratch registers used */ 
    ldr r1, [pRTO];
    mov r1, r1;
}
#endif /* #ifdef _DIAB_TOOL */

    
PUBLIC IX_STATUS ixUARTPollInput(ixUARTDev* pUART, char *inChar)
{
    char volatile lsr, ier;
#ifdef _DIAB_TOOL
    register UINT32 pRTO;
#endif

    IX_UART_CHECK_REF;

    IX_UART_REG_READ(pUART, IX_LSR, lsr);

    ixUARTErr(pUART, lsr);	/* Check LSR for errors */    
    
    if(lsr & IX_LSR_DR)		/* Rx FIFO requests data */
    {
        /* disable RTO */
        IX_UART_REG_READ(pUART, IX_IER, ier);
        IX_UART_REG_WRITE(pUART, IX_IER, (ier & (~IX_IER_RTOIE)));
       
        #if ((CPU!=SIMSPARCSOLARIS) && (CPU!=SIMLINUX)) 
        /* wait for RTO to be disabled - read back IER and stall */
#ifdef _DIAB_TOOL
        pRTO = ((UINT32)(pUART)->addr + (IX_IER * IX_UART_REG_DELTA));
        _RTODisableWait(pRTO);
#else
        __asm__ volatile ("ldr r1, [%0]; mov r1, r1;" 
            : /* no outputs */ 
            : "g" ((UINT32)(pUART)->addr + (IX_IER * IX_UART_REG_DELTA))
            : "r1");
#endif /* #ifdef _DIAB_TOOL */
        #endif
    
	IX_UART_REG_READ(pUART, IX_RBR, *inChar);
	pUART->stats.rxCount++;
        
        /* enable RTO */
        IX_UART_REG_READ(pUART, IX_IER, ier);
        IX_UART_REG_WRITE(pUART, IX_IER, (ier | IX_IER_RTOIE));
        
	return IX_SUCCESS;
    }

    return IX_FAIL;
}


PUBLIC IX_STATUS ixUARTPollOutput(ixUARTDev* pUART, int outChar)
{
    volatile char lsr;
    volatile char msr;

    IX_UART_CHECK_REF;

    IX_UART_REG_READ(pUART, IX_LSR, lsr);
    IX_UART_REG_READ(pUART, IX_MSR, msr);

    ixUARTErr(pUART, lsr);	/* Check LSR for errors */
    
    if(!(lsr & IX_LSR_TDRQ))	/* Tx FIFO Full */
    {
	return IX_FAIL;
    }
    
    if (!(pUART->options & CLOCAL))	 /* hardware flow control */
    {
    	if (msr & IX_MSR_CTS)
	{
	    IX_UART_REG_WRITE(pUART, IX_THR, outChar);
	}
	else
	{
	    return IX_FAIL;
	}
    }
    else				/* software flow (default) */
    {
	IX_UART_REG_WRITE(pUART, IX_THR, outChar);
    }
    pUART->stats.txCount++;
    return IX_SUCCESS;
}


/**
 * @addtogroup UART SupportAPI
 * @{
 */

/**
 * @fn IX_STATUS ixUARTOptsSet(ixUARTDev* pUART, UINT32 options)
 *
 * @param pUART	- pointer to UART structure describing our device.
 * @param options - bitmask of hardware options
 *
 * @brief Set the hardware options for the device.
 *
 * @return IX_SUCCESS - the device options were set correctly.
 * @return IX_FAIL - error setting requested option.
 *
 ***************************************************************************/
PRIVATE IX_STATUS ixUARTOptsSet(ixUARTDev* pUART, UINT32 options)
{
    VUINT32 lcr = 0;
    VUINT32 mcr = 0;
    VUINT32 ier = 0;

    IX_UART_CHECK_REF;

    /* Character size */
    switch (options & CSIZE)	/* bits 3,4 encode character size */
    {
	case CS5:
	{
	    IX_UART_REG_WRITE(pUART, IX_LCR, IX_CHAR_LEN_5);
	    break;
	}
	case CS6:
	{
	    IX_UART_REG_WRITE(pUART, IX_LCR, IX_CHAR_LEN_6);
	    break;
	}
	case CS7:
	{
	    IX_UART_REG_WRITE(pUART, IX_LCR, IX_CHAR_LEN_7);
	    break;
	}
	case CS8:
	default:
	{
	    IX_UART_REG_WRITE(pUART, IX_LCR, IX_CHAR_LEN_8);
	    break;
	}
    }

    /* No. of stop bits */
    IX_UART_REG_READ(pUART, IX_LCR, lcr);
    if (options & STOPB)
    {
	IX_UART_REG_WRITE(pUART, IX_LCR, (lcr | IX_LCR_STB_2)); 
    }
    else
    {
	IX_UART_REG_WRITE(pUART, IX_LCR, (lcr | IX_LCR_STB_1));	/* default */
    }
  
    /* Parity */
    IX_UART_REG_READ(pUART, IX_LCR, lcr);
    switch (options & (PARENB | PARODD))
    {
	case PARENB|PARODD:
	{
	    IX_UART_REG_WRITE(pUART, IX_LCR, (lcr | IX_LCR_PEN)); 
	    break;
	}
	case PARENB:
	{
	    IX_UART_REG_WRITE(pUART, IX_LCR, (lcr | IX_LCR_PEN | IX_LCR_EPS));
	    break;
	}
	default:
	case 0:
	{
	    IX_UART_REG_WRITE(pUART, IX_LCR, (lcr | IX_LCR_PDIS));
	    break;
	}
    }
    
    /* Flow control */
    
    /* Read Interrupt Enable Register - note that the TIE bit 2 (Transmission Interrupt Enable)
       is never set by this driver and defaults to 0 (not enabled) after reset, therefore
       it should never be 1 (enabled) in 'ier'. Enabling the modem status interrupt by writing
       the read 'ier' value OR-ed with MIE (Modem Interrupt Enable) back to the 
       Interrupt Enable Register will not normally enable TIE. */
    IX_UART_REG_READ(pUART, IX_IER, ier);
    
    if (!(options & CLOCAL))	/* hardware (RTS/CTS) */
    {
	IX_UART_REG_READ(pUART, IX_MCR, mcr);
	IX_UART_REG_WRITE(pUART, IX_MCR, (mcr | IX_MCR_RTS));
    	IX_UART_REG_WRITE(pUART, IX_IER, (ier & (~IX_IER_TIE))); 
	IX_UART_REG_WRITE(pUART, IX_IER, (ier |= IX_IER_MIE));  /* enable modem status interrupt */
    }
    else			/* software */
    {
        /* Read MCR status */
        IX_UART_REG_READ(pUART, IX_MCR, mcr);

        /* Reset RTS bit of the MCR */
        IX_UART_REG_WRITE(pUART, IX_MCR, mcr & (~IX_MCR_RTS));
        IX_UART_REG_WRITE(pUART, IX_IER, (ier & ~(IX_IER_MIE))); /* software flow ctrl - default */ 
    }

    pUART->options = options;
    return IX_SUCCESS;
}

/**
 * @fn IX_STATUS ixUARTBaudSet(ixUARTDev* pUART, UINT32 baud)
 *
 * @param pUART - pointer to UART structure describing our device.
 * @param baud - baud rate to set to.
 *
 * @brief Set the baud rate for the device.
 *
 * @pre Baud must be within max/min range as defined in ixUART.h
 *
 * @return IX_SUCCESS - baud rate set successfully
 * @return IX_FAIL - error setting baud rate
 *
 ***************************************************************************/
PRIVATE IX_STATUS ixUARTBaudSet(ixUARTDev* pUART, UINT32 baud)
{
    VUINT32 lcr = 0;
    UINT32 divisor = 0;

    IX_UART_CHECK_REF;

    IX_UART_REG_READ(pUART, IX_LCR, lcr);

    /* Enable access to the divisor latches by setting DLAB in LCR. */
    IX_UART_REG_WRITE(pUART, IX_LCR, (IX_LCR_DLAB | lcr));

    /* Set divisor latches. */
    divisor = pUART->freq/(16*baud);
    IX_UART_REG_WRITE(pUART, IX_DLL, (divisor & 0xFF));
    IX_UART_REG_WRITE(pUART, IX_DLM, ((divisor >> 8) & 0xF));

    /* Restore line control register */
    IX_UART_REG_WRITE(pUART, IX_LCR, lcr);

    pUART->baudRate = baud;
    return IX_SUCCESS;
}

/**
 * @fn IX_STATUS ixUARTModeSet(ixUARTDev* pUART, ixUARTMode mode)
 *
 * @param pUART - pointer to UART structure describing our device.
 * @param mode - mode to switch to.
 *
 * @brief Set the current mode for the device.
 *
 * @return IX_SUCCESS - mode set successfully
 * @return IX_FAIL - invalid mode
 *
 ***************************************************************************/
PRIVATE IX_STATUS ixUARTModeSet(ixUARTDev* pUART, ixUARTMode mode)
{
    int status = IX_SUCCESS;
    VUINT32 mcr = 0;
    VUINT32 msr = 0;

    IX_UART_CHECK_REF;

    switch (mode)
    {
	case INTERRUPT:		/* Not supported for now */
	{    
	    status = IX_FAIL;
	    break;
	}

	case POLLED:
	{
	    IX_UART_REG_WRITE(pUART, IX_IER, IX_IER_UUE);  /* Disable interrupts */
	    IX_UART_REG_READ(pUART, IX_MCR, mcr);
	    IX_UART_REG_WRITE(pUART, IX_MCR, (mcr | ~IX_MCR_LOOP));  /* Ensure loopback disabled */
	    IX_UART_REG_READ(pUART, IX_MSR, msr);  /* Read once to clear delta bits */
	    pUART->mode = POLLED;
	    break;
	}

	case LOOPBACK:
	{
	    IX_UART_REG_WRITE(pUART, IX_IER, IX_IER_UUE);  /* Ints are optional but we disable them anyways */
	    IX_UART_REG_READ(pUART, IX_MCR, mcr);
	    IX_UART_REG_WRITE(pUART, IX_MCR, (mcr | IX_MCR_LOOP));
	    pUART->mode = LOOPBACK;
	    break;
	}

	default:
	    status = IX_FAIL;
    }

    return status;
}


/**
 * @fn void ixUARTErr(ixUARTDev* pUART, char status)
 *
 * @param pUART - pointer to UART structure describing our device.
 * @param status - current status of LSR.
 *
 * @brief UART error handler. Just increments counters for now.
 *
 * @return  N/A 
 ****************************************************************************/
PRIVATE void ixUARTErr(ixUARTDev* pUART, char status)
{

    if(status & IX_LSR_OE)
	pUART->stats.overrunErr++;
	
    if(status & IX_LSR_PE)
	pUART->stats.parityErr++;

    if(status & IX_LSR_FE)
	pUART->stats.framingErr++;

    if(status & IX_LSR_BI)
	pUART->stats.breakErr++;

}

/**
 * @} addtogroup UART SupportAPI
 */
