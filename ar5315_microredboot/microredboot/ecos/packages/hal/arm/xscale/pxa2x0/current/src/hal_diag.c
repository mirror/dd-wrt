/*=============================================================================
//
//      hal_diag.c
//
//      HAL diagnostic output code
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   <knud.woehler@microplex.de>
// Date:        2002-09-03
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>           // basic machine info
#include <cyg/hal/hal_intr.h>           // interrupt macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // Calling interface definitions
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/drv_api.h>            // cyg_drv_interrupt_acknowledge
#include <cyg/hal/hal_misc.h>           // Helper functions
#include <cyg/hal/hal_pxa2x0.h>         // Hardware definitions


//-----------------------------------------------------------------------------
typedef struct {
    cyg_uint8* base;
    cyg_int32 msec_timeout;
    int isr_vector;
    int baud_rate;
} channel_data_t;

/*---------------------------------------------------------------------------*/
// PXA2X0 Serial Port (UARTx) for Debug

static void
init_channel(channel_data_t* __ch_data)
{
	cyg_uint8* base = __ch_data->base;
	cyg_uint8 lcr;
    cyg_uint32 brd;

	// 8-1-no parity.
	lcr = PXA2X0_UART_LCR_WLS0 | PXA2X0_UART_LCR_WLS1;
	lcr |= PXA2X0_UART_LCR_DLAB;
	HAL_WRITE_UINT8( base+PXA2X0_UART_LCR, lcr );

	//	Setup divisor
    brd = PXA2X0_UART_BAUD_RATE_DIVISOR( __ch_data->baud_rate );
    HAL_WRITE_UINT8( base+PXA2X0_UART_DLH, (brd >> 8) & 0xff );
    HAL_WRITE_UINT8( base+PXA2X0_UART_DLL, brd & 0xff );


	//	DLAB = 0 to allow access to FIFOs
    lcr &= ~PXA2X0_UART_LCR_DLAB;
    HAL_WRITE_UINT8(base+PXA2X0_UART_LCR, lcr);

	//  Enable & clear FIFOs
	//  set Interrupt Trigger Level to be 1 byte
	HAL_WRITE_UINT8(base+PXA2X0_UART_FCR, 
		(PXA2X0_UART_FCR_FCR0 | PXA2X0_UART_FCR_FCR1 | PXA2X0_UART_FCR_FCR2));  // Enable & clear FIFO

	//	Configure NRZ, disable DMA requests and enable UART
	HAL_WRITE_UINT8(base+PXA2X0_UART_IER, PXA2X0_UART_IER_UUE);
}

static void
cyg_hal_plf_serial_putc(void *__ch_data, char c)
{
	cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
	cyg_uint8 lsr;
	CYGARC_HAL_SAVE_GP();

	do {
		HAL_READ_UINT8(base+PXA2X0_UART_LSR, lsr);
	} while ((lsr & PXA2X0_UART_LSR_THRE) == 0);

	HAL_WRITE_UINT8(base+PXA2X0_UART_THR, c);

	do {
		HAL_READ_UINT8(base+PXA2X0_UART_LSR, lsr);
	} while ((lsr & PXA2X0_UART_LSR_THRE) == 0);

	CYGARC_HAL_RESTORE_GP();
}

static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
	cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
	cyg_uint8 lsr;

	HAL_READ_UINT8(base+PXA2X0_UART_LSR, lsr);
	if ((lsr & PXA2X0_UART_LSR_DR) == 0)
		return false;

	HAL_READ_UINT8(base+PXA2X0_UART_RBR, *ch);

	return true;
}

static cyg_uint8
cyg_hal_plf_serial_getc(void* __ch_data)
{
    cyg_uint8 ch;
    CYGARC_HAL_SAVE_GP();

    while(!cyg_hal_plf_serial_getc_nonblock(__ch_data, &ch));

    CYGARC_HAL_RESTORE_GP();
    return ch;
}

static channel_data_t ser_channels[] = {
#if CYGHWR_HAL_ARM_PXA2X0_FFUART != 0
    { (cyg_uint8*)PXA2X0_FFUART_BASE, 1000, 
      CYGNUM_HAL_INTERRUPT_FFUART, CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD },
#endif
#if CYGHWR_HAL_ARM_PXA2X0_BTUART != 0
    { (cyg_uint8*)PXA2X0_BTUART_BASE, 1000, 
      CYGNUM_HAL_INTERRUPT_BTUART, CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD },
#endif
#if CYGHWR_HAL_ARM_PXA2X0_STUART != 0
    { (cyg_uint8*)PXA2X0_STUART_BASE, 1000, 
      CYGNUM_HAL_INTERRUPT_STUART, CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD },
#endif
};

static void
cyg_hal_plf_serial_write(void* __ch_data, const cyg_uint8* __buf, 
                         cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        cyg_hal_plf_serial_putc(__ch_data, *__buf++);

    CYGARC_HAL_RESTORE_GP();
}

static void
cyg_hal_plf_serial_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        *__buf++ = cyg_hal_plf_serial_getc(__ch_data);

    CYGARC_HAL_RESTORE_GP();
}

cyg_bool
cyg_hal_plf_serial_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    int delay_count;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_bool res;
    CYGARC_HAL_SAVE_GP();

    delay_count = chan->msec_timeout * 10; // delay in .1 ms steps

    for(;;) {
        res = cyg_hal_plf_serial_getc_nonblock(__ch_data, ch);
        if (res || 0 == delay_count--)
            break;
        
        CYGACC_CALL_IF_DELAY_US(100);
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static int
cyg_hal_plf_serial_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
    static int irq_state = 0;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    int ret = -1;
    cyg_uint8 ier;
	va_list ap;

    CYGARC_HAL_SAVE_GP();
    va_start(ap, __func);

    switch (__func) {
    case __COMMCTL_GETBAUD:
        ret = chan->baud_rate;
        break;
    case __COMMCTL_SETBAUD:
        chan->baud_rate = va_arg(ap, cyg_int32);
        // Should we verify this value here?
        init_channel(chan);
        ret = 0;
        break;
    case __COMMCTL_IRQ_ENABLE:
        HAL_INTERRUPT_UNMASK(chan->isr_vector);
        HAL_INTERRUPT_SET_LEVEL(chan->isr_vector, 1);
        HAL_READ_UINT8(chan->base+PXA2X0_UART_IER, ier);
        ier |= PXA2X0_UART_IER_RAVIE;
        HAL_WRITE_UINT8(chan->base+PXA2X0_UART_IER, ier);
        irq_state = 1;
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;
        HAL_INTERRUPT_MASK(chan->isr_vector);
        HAL_READ_UINT8(chan->base+PXA2X0_UART_IER, ier);
        ier &= ~PXA2X0_UART_IER_RAVIE;
        HAL_WRITE_UINT8(chan->base+PXA2X0_UART_IER, ier);
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = chan->isr_vector;
        break;
    case __COMMCTL_SET_TIMEOUT:
        ret = chan->msec_timeout;
        chan->msec_timeout = va_arg(ap, cyg_uint32);
        break;
    default:
        break;
    }
    va_end(ap);
    CYGARC_HAL_RESTORE_GP();
    return ret;
}

static int
cyg_hal_plf_serial_isr(void *__ch_data, int* __ctrlc, 
                       CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint8 iir;
    int res = 0;
    CYGARC_HAL_SAVE_GP();

    HAL_READ_UINT8(chan->base+PXA2X0_UART_IIR, iir);
    iir &= PXA2X0_UART_IIR_ID_MASK;

    *__ctrlc = 0;
    if ( iir == 0x04 ) {
        cyg_uint8 c, lsr;
        HAL_READ_UINT8(chan->base+PXA2X0_UART_LSR, lsr);
        if (lsr & PXA2X0_UART_LSR_DR) {

            HAL_READ_UINT8(chan->base+PXA2X0_UART_RBR, c);

            if( cyg_hal_is_break( &c , 1 ) )
                *__ctrlc = 1;
        }

        // Acknowledge the interrupt
        HAL_INTERRUPT_ACKNOWLEDGE(chan->isr_vector);
        res = CYG_ISR_HANDLED;
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static void
cyg_hal_plf_serial_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);
    int i;

    // Init channels
#define NUMOF(x) (sizeof(x)/sizeof(x[0]))
    for (i = 0;  i < NUMOF(ser_channels);  i++) {
        init_channel(&ser_channels[i]);
        CYGACC_CALL_IF_SET_CONSOLE_COMM(i);
        comm = CYGACC_CALL_IF_CONSOLE_PROCS();
        CYGACC_COMM_IF_CH_DATA_SET(*comm, &ser_channels[i]);
        CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
        CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
        CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
        CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
        CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
        CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
        CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);
    }
     
    // Restore original console
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
}

void
cyg_hal_plf_comms_init(void)
{
    static int initialized = 0;

    if (initialized)
        return;

    initialized = 1;

    cyg_hal_plf_serial_init();
}

/*---------------------------------------------------------------------------*/
/* End of hal_diag.c */
