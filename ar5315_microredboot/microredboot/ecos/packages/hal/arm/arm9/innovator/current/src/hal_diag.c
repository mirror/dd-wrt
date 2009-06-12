//=============================================================================
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
// Author(s):   Patrick Doyle <wpd@delcomsys.com>
// Contributors:Patrick Doyle <wpd@delcomsys.com>
// Date:        2002-12-17
// Purpose:     HAL diagnostic output
//      This file contains the type definitions, constants, and function
//      prototoypes that implement very simple access to the UART on the
//      OMAP part.
// Description: Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>           // basic machine info
#include <cyg/hal/hal_intr.h>           // interrupt macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_if.h>             // interface API
#include <cyg/hal/hal_misc.h>           // Helper functions
#include <cyg/hal/innovator.h>          // platform definitions

//-----------------------------------------------------------------------------

#define CYG_DEVICE_SERIAL_BAUD_DIV (CYGNUM_HAL_ARM_INNOVATOR_PERIPHERAL_CLOCK/CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD/16)
#define CYG_DEVICE_SERIAL_BAUD_LSB (CYG_DEVICE_SERIAL_BAUD_DIV&0xff)
#define CYG_DEVICE_SERIAL_BAUD_MSB ((CYG_DEVICE_SERIAL_BAUD_DIV>>8)&0xff)

//-----------------------------------------------------------------------------
typedef struct {
    cyg_int32 msec_timeout;
#ifdef LATER
    cyg_uint32 base;
    int isr_vector;
#endif
} channel_data_t;

/************************************************************************
 * Useful definitions -- note that 'USE_MODEM_UART' has not been extensively
 * tested nor debugged (read -- it probably doesn't work).
 ************************************************************************/
/*#define USE_MODEM_UART*/
#ifdef USE_MODEM_UART
#define BASE_ADDR 0xfffce800
#define STRIDE    1
#else
#define BASE_ADDR 0xfffb0000
#define STRIDE    4
#endif

#define RHR   0x00
#define THR   0x00
#define DLL   0x00
#define DLH   0x01
#define IER   0x01
#define FCR   0x02
#define EFR   0x02
#define IIR   0x02
#define LCR   0x03
#define MCR   0x04
#define XON1  0x04
#define LSR   0x05
#define XON2  0x05
#define MSR   0x06
#define TCR   0x06
#define XOFF1 0x06
#define SPR   0x07
#define TLR   0x07
#define XOFF2 0x07
#define MDR1  0x08
#define UASR  0x0e
#define SCR   0x10
#define SSR   0x11
#define OSC_12M 0x13


/************************************************************************
 * Data local to this file
 ************************************************************************/
#define write_serial(offset, value) \
  *(volatile char *)(BASE_ADDR + STRIDE * (offset)) = value

#define read_serial(offset) \
  (*(volatile char *)(BASE_ADDR + STRIDE * (offset)))

/************************************************************************
 * Forward References
 ************************************************************************/

/************************************************************************
 * Function:
 *      init_uart()
 *
 * Purpose:
 *      This procedure may be called in order to initialize the UART
 *      prior to use.
 *
 * Operation:
 *      Set up the UART for 115,200 baud, 8 bits, 1 stop bit, no parity.
 *
 *      The UART is defined by the 'BASE_ADDR' macro.
 *
 * Notes/Issues:
 *      This pays some lip service to being able to use UART3, but since
 *      it seems to me that the use of UART3 requires that the DSP do
 *      some setup (at least, until I learn more), it probably won't
 *      work for UART3.
 ************************************************************************/
static void
quick_init_uart(void)
{
  /* UART Software Reset */ 
  write_serial(LCR, 0xBF); /* Access to EFR & UART break is removed */
  write_serial(EFR, BIT_04); /* Set EFR[4] = 0x1 */
  write_serial(LCR, 0x00); /* Access to IER & MCR is allowed */

  write_serial(IER, 0x00); /* Disable all interrupts */
  write_serial(MCR, 0x00); /* DTR, RTS, XON, loopbback inactive */

  write_serial(MDR1,0x07); /* UART is in reset */

  /* UART FIFO Configuration */
  write_serial(MCR, read_serial(MCR) | BIT_06); /* Set MCR[6] = 1 */
  write_serial(TCR, 0x0F); /* RTS off when Rx FIFO at 60 bytes, on at 0 */
  write_serial(TLR, 0x88); /* set TX & RX trigger levels each to 32 */
  write_serial(FCR, 0x07); /* Enable & reset FIFOs, triggers at 8 */
  write_serial(LCR, 0xBF); /* Access EFR */
  write_serial(EFR, 0xC0); /* Enable auto RTS & CTS */
  write_serial(LCR, 0x00); /* Access to IER & MCR is allowed */
  write_serial(MCR, read_serial(MCR) & ~BIT_06); /* Clear MCR[6] */

  /* Baud Rate and Stop Configuration */
  write_serial(LCR, 0x03); /* 8,N,1 */
#ifdef USE_MODEM_UART
  write_serial(LCR, 0x83); /* gain access to DLH and DLL */
  write_serial(DLH, 0x00); /* Divisor value = Operating Freq/(16 x Baud Rate) */
  write_serial(DLL, 0x0D); /* DPLL2 configured for Operating Freq = 24 MHz */
			   /* Baud Rate = 115,200 bps */
#else
  write_serial(OSC_12M, 1);/* Set divisor value to 6.5 */
  write_serial(LCR, 0x83); /* gain access to DLH and DLL */
  write_serial(DLH, 0x00); /* Divisor value =
                            *     Operating Freq/(16 x 6.5 x Baud Rate) */
  write_serial(DLL, 0x01); /* DPLL2 configured for Operating Freq = 12 MHz */
			   /* Baud Rate = 115,200 bps */
#endif
  write_serial(LCR, 0x03); /* restore LCR */

  write_serial(MDR1,0x00); /* enable UART */
}

/************************************************************************
 * Function:
 *      putchar(c)
 *
 * Purpose:
 *      This procedure may be called in order to output a character on
 *      the serial port.  It blocks until the serial port TX FIFO is
 *      empty.
 *
 * Operation:
 *      Write character to the Transmit Holding Register (THR).  Can
 *      optionally map the Linefeed character to a Carriage Return
 *      character and/or output a Carriage Return character whenever
 *      a Linefeed character is seen, depending on #ifdefs.
 *
 * Notes/Issues:
 *      This could be optimized to block only when the serial port TX
 *      FIFO is full.
 ************************************************************************/
static void
quick_putchar(char c)
{
/* #define MAP_LF_TO_CR */
#ifdef MAP_LF_TO_CR
  if (c == '\n') {
    c = '\r';
  }
#endif
  while ((read_serial(LSR) & 0x20) == 0) ;
  write_serial(THR, c);

/*  #define DO_CRLF */
#ifdef DO_CRLF
  if (c == '\n') {
    quick_putchar('\r');
  }
#endif
}

/************************************************************************
 * Function:
 *      getchar()
 *
 * Purpose:
 *      This function may be called in order to read a character from
 *      the serial port.
 *
 * Operation:
 *      Poll the Line Status register until it indicates a character has
 *      been received.  Return the character to the caller.
 *
 * Notes/Issues:
 *
 ************************************************************************/
static int
quick_getchar(void)
{
  while ((read_serial(LSR) & 0x01) == 0) ;
  return(read_serial(RHR));
}

/************************************************************************
 * Function:
 *      getchar_nonblock()
 *
 * Purpose:
 *      This function may be called in order to read a character from
 *      the serial port.
 *
 * Operation:
 *      Poll the Line Status register until it indicates a character has
 *      been received.  Return the character to the caller.
 *
 * Notes/Issues:
 *
 ************************************************************************/
static int
quick_getchar_nonblock(char *c)
{
  if ((read_serial(LSR) & 0x01) == 0) {
    return(0);
  } else {
    *c = read_serial(RHR);
    return(1);
  }
}

//-----------------------------------------------------------------------------

static void
cyg_hal_plf_serial_init_channel(void* __ch_data)
{
#ifdef LATER
    cyg_uint32 base = ((channel_data_t*)__ch_data)->base;

    // 8-1-no parity.
    HAL_WRITE_UINT32(base+_UART_MC, _UART_MC_8BIT | _UART_MC_1STOP | _UART_MC_PARITY_NONE);

    HAL_WRITE_UINT32(base+_UART_DIV_LO, CYG_DEVICE_SERIAL_BAUD_LSB);
    HAL_WRITE_UINT32(base+_UART_DIV_HI, CYG_DEVICE_SERIAL_BAUD_MSB);
    HAL_WRITE_UINT32(base+_UART_FCR, (_UART_FCR_TC | _UART_FCR_RC |
                                      _UART_FCR_TX_THR_15 | _UART_FCR_RX_THR_1));  // clear & enableFIFO

    // enable RX interrupts - otherwise ISR cannot be polled. Actual
    // interrupt control of serial happens via INT_MASK
    HAL_WRITE_UINT32(base+_UART_IES, _UART_INTS_RE);
#endif
}

void
cyg_hal_plf_serial_putc(void *__ch_data, char c)
{
#ifdef LATER
    cyg_uint32 base = ((channel_data_t*)__ch_data)->base;
    cyg_uint32 tsr;
    CYGARC_HAL_SAVE_GP();

    do {
        HAL_READ_UINT32(base+_UART_TSR, tsr);
        // Wait for TXI flag to be set - or for the register to be
        // zero (works around a HW bug it seems).
    } while (tsr && (tsr & _UART_TSR_TXI) == 0);

    HAL_WRITE_UINT32(base+_UART_TD, (cyg_uint32)(unsigned char)c);

    CYGARC_HAL_RESTORE_GP();
#else
    quick_putchar(c);
#endif
}

static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
#ifdef LATER
    cyg_uint32 base = ((channel_data_t*)__ch_data)->base;
    cyg_uint32 rsr, isr, data;

    HAL_READ_UINT32(base+_UART_ISR, isr);
    if (0 == (isr & _UART_INTS_RI)) {
        HAL_READ_UINT32(base+_UART_RSR, rsr);
        if (0 == rsr) 
            return false;
    }

    HAL_READ_UINT32(base+_UART_RD, data);
    *ch = (cyg_uint8)(data & 0xff);

    // Read RSR to clear interrupt, and RDS to clear errors
    HAL_READ_UINT32(base+_UART_RSR, data);
    HAL_READ_UINT32(base+_UART_RDS, data);

    return true;
#else
    return(quick_getchar_nonblock(ch));
#endif
}

cyg_uint8
cyg_hal_plf_serial_getc(void* __ch_data)
{
#ifdef LATER
    cyg_uint8 ch;
    CYGARC_HAL_SAVE_GP();

    while(!cyg_hal_plf_serial_getc_nonblock(__ch_data, &ch));

    CYGARC_HAL_RESTORE_GP();
    return ch;
#else
    return(quick_getchar());
#endif
}

static channel_data_t innovator_ser_channels[1] = {
    { 1000 }
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
    int ret = 0;
    CYGARC_HAL_SAVE_GP();

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        irq_state = 1;

        // Need to keep it enabled to allow polling using ISR
        //HAL_WRITE_UINT32(chan->base+_UART_IES, _UART_INTS_RE);

#ifdef LATER
        HAL_INTERRUPT_UNMASK(chan->isr_vector);
#endif
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;

        // Need to keep it enabled to allow polling using ISR
        // HAL_WRITE_UINT32(chan->base+_UART_IEC, _UART_INTS_RE);

#ifdef LATER
        HAL_INTERRUPT_MASK(chan->isr_vector);
#endif
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
#ifdef LATER
        ret = chan->isr_vector;
#else
        ret = 0;
#endif
        break;
    case __COMMCTL_SET_TIMEOUT:
    {
        va_list ap;

        va_start(ap, __func);

        ret = chan->msec_timeout;
        chan->msec_timeout = va_arg(ap, cyg_uint32);

        va_end(ap);
    }        
    default:
        break;
    }
    CYGARC_HAL_RESTORE_GP();
    return ret;
}

static int
cyg_hal_plf_serial_isr(void *__ch_data, int* __ctrlc, 
                       CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
#ifdef LATER
    int res = 0;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint32 isr, ch, rsr;
    char c;
    CYGARC_HAL_SAVE_GP();

    cyg_drv_interrupt_acknowledge(chan->isr_vector);

    *__ctrlc = 0;
    HAL_READ_UINT32(chan->base+_UART_ISR, isr);
    HAL_READ_UINT32(chan->base+_UART_RSR, rsr);

    // Again, check both RI and the RX FIFO count.
    if ( ((isr & _UART_INTS_RI) != 0 ) || (rsr) ) {

        HAL_READ_UINT32(chan->base+_UART_RD, ch);

        c = (char)ch;
        if( cyg_hal_is_break( &c , 1 ) )
            *__ctrlc = 1;

        res = CYG_ISR_HANDLED;
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
#else
    return 0;
#endif
}

static void
cyg_hal_plf_serial_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

#ifdef LATER
    // Disable interrupts.
    HAL_INTERRUPT_MASK(innovator_ser_channels[0].isr_vector);

    // Init channels
    cyg_hal_plf_serial_init_channel(&innovator_ser_channels[0]);
#else
    quick_init_uart();
#endif

    // Setup procs in the vector table

    // Set channel 0
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &innovator_ser_channels[0]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);

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

//-----------------------------------------------------------------------------
// LEDs
void
hal_diag_led(int n)
{
}

//-----------------------------------------------------------------------------
// End of hal_diag.c
