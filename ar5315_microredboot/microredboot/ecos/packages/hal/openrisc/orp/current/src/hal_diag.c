//=============================================================================
//
//      hal_diag.c
//
//      Simple polling driver for the 16c550c serial controller(s) in the ORP,
//      to be used for diagnostic I/O and gdb remote debugging.
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
// Author(s):   sfurman
// Contributors:dmoseley
// Date:        2003-02-28
// Description: Simple polling driver for the 16c550c serial controller(s) in the ORP,
//              to be used for diagnostic I/O and gdb remote debugging.
//      
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/hal/hal_arch.h>           // SAVE/RESTORE GP macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // interface API
#include <cyg/hal/hal_intr.h>           // HAL_ENABLE/MASK/UNMASK_INTERRUPTS
#include <cyg/hal/hal_misc.h>           // Helper functions
#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED
#include <cyg/infra/cyg_ass.h>          // assertion macros

//-----------------------------------------------------------------------------
// Base addresses for each 16550 UART in the system
#define SERIAL_16550_CONSOLE_BASE_ADDR    0x90000000
#define SERIAL_16550_DEBUGGER_BASE_ADDR   0x90000008

//-----------------------------------------------------------------------------
// Define the 16550C serial registers.
#define SER_16550_RBR 0x00   // receiver buffer register, read, dlab = 0
#define SER_16550_THR 0x00   // transmitter holding register, write, dlab = 0
#define SER_16550_DLL 0x00   // divisor latch (LS), read/write, dlab = 1
#define SER_16550_IER 0x01   // interrupt enable register, read/write, dlab = 0
#define SER_16550_DLM 0x01   // divisor latch (MS), read/write, dlab = 1
#define SER_16550_IIR 0x02   // interrupt identification reg, read, dlab = 0
#define SER_16550_FCR 0x02   // fifo control register, write, dlab = 0
#define SER_16550_AFR 0x02   // alternate function reg, read/write, dlab = 1
#define SER_16550_LCR 0x03   // line control register, read/write
#define SER_16550_MCR 0x04   // modem control register, read/write
#define SER_16550_LSR 0x05   // line status register, read
#define SER_16550_MSR 0x06   // modem status register, read
#define SER_16550_SCR 0x07   // scratch pad register

// The interrupt enable register bits.
#define SIO_IER_ERDAI   0x01            // enable received data available irq
#define SIO_IER_ETHREI  0x02            // enable THR empty interrupt
#define SIO_IER_ELSI    0x04            // enable receiver line status irq
#define SIO_IER_EMSI    0x08            // enable modem status interrupt

// The interrupt identification register bits.
#define SIO_IIR_IP      0x01            // 0 if interrupt pending
#define SIO_IIR_ID_MASK 0x0e            // mask for interrupt ID bits

// The line status register bits.
#define SIO_LSR_DR      0x01            // data ready
#define SIO_LSR_OE      0x02            // overrun error
#define SIO_LSR_PE      0x04            // parity error
#define SIO_LSR_FE      0x08            // framing error
#define SIO_LSR_BI      0x10            // break interrupt
#define SIO_LSR_THRE    0x20            // transmitter holding register empty
#define SIO_LSR_TEMT    0x40            // transmitter register empty
#define SIO_LSR_ERR     0x80            // any error condition

// The modem status register bits.
#define SIO_MSR_DCTS  0x01              // delta clear to send
#define SIO_MSR_DDSR  0x02              // delta data set ready
#define SIO_MSR_TERI  0x04              // trailing edge ring indicator
#define SIO_MSR_DDCD  0x08              // delta data carrier detect
#define SIO_MSR_CTS   0x10              // clear to send
#define SIO_MSR_DSR   0x20              // data set ready
#define SIO_MSR_RI    0x40              // ring indicator
#define SIO_MSR_DCD   0x80              // data carrier detect

// The line control register bits.
#define SIO_LCR_WLS0   0x01             // word length select bit 0
#define SIO_LCR_WLS1   0x02             // word length select bit 1
#define SIO_LCR_STB    0x04             // number of stop bits
#define SIO_LCR_PEN    0x08             // parity enable
#define SIO_LCR_EPS    0x10             // even parity select
#define SIO_LCR_SP     0x20             // stick parity
#define SIO_LCR_SB     0x40             // set break
#define SIO_LCR_DLAB   0x80             // divisor latch access bit

// The FIFO control register
#define SIO_FCR_FCR0   0x01             // enable xmit and rcvr fifos
#define SIO_FCR_FCR1   0x02             // clear RCVR FIFO
#define SIO_FCR_FCR2   0x04             // clear XMIT FIFO

/////////////////////////////////////////
// Interrupt Enable Register
#define IER_RCV 0x01
#define IER_XMT 0x02
#define IER_LS  0x04
#define IER_MS  0x08

// Line Control Register
#define LCR_WL5 0x00    // Word length
#define LCR_WL6 0x01
#define LCR_WL7 0x02
#define LCR_WL8 0x03
#define LCR_SB1 0x00    // Number of stop bits
#define LCR_SB1_5 0x04  // 1.5 -> only valid with 5 bit words
#define LCR_SB2 0x04
#define LCR_PN  0x00    // Parity mode - none
#define LCR_PE  0x0C    // Parity mode - even
#define LCR_PO  0x08    // Parity mode - odd
#define LCR_PM  0x28    // Forced "mark" parity
#define LCR_PS  0x38    // Forced "space" parity
#define LCR_DL  0x80    // Enable baud rate latch

// Line Status Register
#define LSR_RSR 0x01
#define LSR_THE 0x20

// Modem Control Register
#define MCR_DTR 0x01
#define MCR_RTS 0x02
#define MCR_INT 0x08   // Enable interrupts

// Interrupt status register
#define ISR_None             0x01
#define ISR_Rx_Line_Status   0x06
#define ISR_Rx_Avail         0x04
#define ISR_Rx_Char_Timeout  0x0C
#define ISR_Tx_Empty         0x02
#define ISR_Modem_Status     0x00

// FIFO control register
#define FCR_ENABLE     0x01
#define FCR_CLEAR_RCVR 0x02
#define FCR_CLEAR_XMIT 0x04

// Assume the UART is driven 1/16 CPU frequency
#define UART_CLOCK    ((CYGHWR_HAL_OPENRISC_CPU_FREQ)*1.0e6/16.0)

#define DIVISOR(baud) ((int)((UART_CLOCK)/baud))

#ifdef CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD
#define CYG_DEV_SERIAL_BAUD_DIVISOR   \
    DIVISOR(CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD)
#else
#error Missing/incorrect serial baud rate defined - CDL error?
#endif


//-----------------------------------------------------------------------------
typedef struct {
    cyg_uint8* base;
    cyg_int32 msec_timeout;
    int isr_vector;
} channel_data_t;

static channel_data_t channels[] = {
    { (cyg_uint8*)SERIAL_16550_CONSOLE_BASE_ADDR,
      1000,
      CYGNUM_HAL_INTERRUPT_SERIAL_CONSOLE
    },
    { (cyg_uint8*)SERIAL_16550_DEBUGGER_BASE_ADDR,
      1000,
      CYGNUM_HAL_INTERRUPT_SERIAL_DEBUGGER
    }
};

//-----------------------------------------------------------------------------
// Set the baud rate

static void
cyg_hal_plf_serial_set_baud(cyg_uint8* port, cyg_uint16 baud_divisor)
{
    cyg_uint8 _lcr;

    HAL_READ_UINT8(port+SER_16550_LCR, _lcr);
    _lcr |= LCR_DL;
    HAL_WRITE_UINT8(port+SER_16550_LCR, _lcr);

    HAL_WRITE_UINT8(port+SER_16550_DLM, baud_divisor >> 8);
    HAL_WRITE_UINT8(port+SER_16550_DLL, baud_divisor & 0xff);

    _lcr &= ~LCR_DL;
    HAL_WRITE_UINT8(port+SER_16550_LCR, _lcr);
}

//-----------------------------------------------------------------------------
// The minimal init, get and put functions. All by polling.

void
cyg_hal_plf_serial_init_channel(void* __ch_data)
{
    cyg_uint8* port;
    cyg_uint8 _lcr;

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    port = ((channel_data_t*)__ch_data)->base;

    // Disable port interrupts while changing hardware
    HAL_WRITE_UINT8(port+SER_16550_IER, 0);

    // Set databits, stopbits and parity.
    _lcr = LCR_WL8 | LCR_SB1 | LCR_PN;
    HAL_WRITE_UINT8(port+SER_16550_LCR, _lcr);

    // Set baud rate.
    cyg_hal_plf_serial_set_baud(port, CYG_DEV_SERIAL_BAUD_DIVISOR);

    // Enable and clear FIFO
    HAL_WRITE_UINT8(port+SER_16550_FCR, (FCR_ENABLE | FCR_CLEAR_RCVR | FCR_CLEAR_XMIT));

    // enable RTS to keep host side happy
    HAL_WRITE_UINT8( port+SER_16550_MCR, MCR_RTS );
    
    // Don't allow interrupts.
    HAL_WRITE_UINT8(port+SER_16550_IER, 0);
}

void
cyg_hal_plf_serial_putc(void* __ch_data, cyg_uint8 __ch)
{
    cyg_uint8* port;
    cyg_uint8 _lsr;

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    port = ((channel_data_t*)__ch_data)->base;

    CYGARC_HAL_SAVE_GP();

    do {
        HAL_READ_UINT8(port+SER_16550_LSR, _lsr);
    } while ((_lsr & SIO_LSR_THRE) == 0);

    // Now, the transmit buffer is empty
    HAL_WRITE_UINT8(port+SER_16550_THR, __ch);

    // Hang around until the character has been safely sent.
    do {
        HAL_READ_UINT8(port+SER_16550_LSR, _lsr);
    } while ((_lsr & SIO_LSR_THRE) == 0);

    CYGARC_HAL_RESTORE_GP();
}

static int lsr_global;

static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    cyg_uint8* port;
    cyg_uint8 _lsr;

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    port = ((channel_data_t*)__ch_data)->base;

    HAL_READ_UINT8(port+SER_16550_LSR, _lsr);
    if ((_lsr & SIO_LSR_DR) == 0)
        return false;
    lsr_global = _lsr;
    CYG_ASSERT((_lsr & SIO_LSR_OE) == 0 , "UART receiver overrun error");
    HAL_READ_UINT8(port+SER_16550_RBR, *ch);

    return true;
}

cyg_uint8
cyg_hal_plf_serial_getc(void* __ch_data)
{
    cyg_uint8 ch;
    CYGARC_HAL_SAVE_GP();

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    while(!cyg_hal_plf_serial_getc_nonblock(__ch_data, &ch));

    CYGARC_HAL_RESTORE_GP();
    return ch;
}

static void
cyg_hal_plf_serial_write(void* __ch_data, const cyg_uint8* __buf, 
                         cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    while(__len-- > 0)
        cyg_hal_plf_serial_putc(__ch_data, *__buf++);

    CYGARC_HAL_RESTORE_GP();
}

static void
cyg_hal_plf_serial_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    while(__len-- > 0)
        *__buf++ = cyg_hal_plf_serial_getc(__ch_data);

    CYGARC_HAL_RESTORE_GP();
}


cyg_bool
cyg_hal_plf_serial_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    int delay_count;
    channel_data_t* chan;
    cyg_bool res;
    CYGARC_HAL_SAVE_GP();

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    chan = (channel_data_t*)__ch_data;

    delay_count = chan->msec_timeout; // delay in 1000 us steps

    for(;;) {
        res = cyg_hal_plf_serial_getc_nonblock(__ch_data, ch);
        if (res || 0 == delay_count--)
            break;
        CYGACC_CALL_IF_DELAY_US(1000);
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static int
cyg_hal_plf_serial_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
    static int irq_state = 0;
    channel_data_t* chan;
    cyg_uint8 ier;
    int ret = 0;
    CYGARC_HAL_SAVE_GP();

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    chan = (channel_data_t*)__ch_data;

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        irq_state = 1;

        HAL_READ_UINT8(chan->base + SER_16550_IER, ier);
        ier |= SIO_IER_ERDAI;
        HAL_WRITE_UINT8(chan->base + SER_16550_IER, ier);

        HAL_INTERRUPT_SET_LEVEL(chan->isr_vector, 1);
        HAL_INTERRUPT_UNMASK(chan->isr_vector);
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;

        HAL_READ_UINT8(chan->base + SER_16550_IER, ier);
        ier &= ~SIO_IER_ERDAI;
        HAL_WRITE_UINT8(chan->base + SER_16550_IER, ier);

        HAL_INTERRUPT_MASK(chan->isr_vector);
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = chan->isr_vector;
        break;
    case __COMMCTL_SET_TIMEOUT:
    {
        va_list ap;

        va_start(ap, __func);

        ret = chan->msec_timeout;
        chan->msec_timeout = va_arg(ap, cyg_uint32);

        va_end(ap);
    }        
    break;
    case __COMMCTL_SETBAUD:
    {
        cyg_uint32 baud_rate;
        cyg_uint16 baud_divisor;
        cyg_uint8* port = chan->base;
        va_list ap;

        va_start(ap, __func);
        baud_rate = va_arg(ap, cyg_uint32);
        va_end(ap);

        switch (baud_rate)
        {
        case 110:    baud_divisor = DIVISOR(110);    break;
        case 150:    baud_divisor = DIVISOR(150);    break;
        case 300:    baud_divisor = DIVISOR(300);    break;
        case 600:    baud_divisor = DIVISOR(600);    break;
        case 1200:   baud_divisor = DIVISOR(1200);   break;
        case 2400:   baud_divisor = DIVISOR(2400);   break;
        case 4800:   baud_divisor = DIVISOR(4800);   break;
        case 7200:   baud_divisor = DIVISOR(7200);   break;
        case 9600:   baud_divisor = DIVISOR(9600);   break;
        case 14400:  baud_divisor = DIVISOR(14400);  break;
        case 19200:  baud_divisor = DIVISOR(19200);  break;
        case 38400:  baud_divisor = DIVISOR(38400);  break;
        case 57600:  baud_divisor = DIVISOR(57600);  break;
        case 115200: baud_divisor = DIVISOR(115200); break;
        case 230400: baud_divisor = DIVISOR(230400); break;
        default:     return -1;                      break; // Invalid baud rate selected
        }

        // Disable port interrupts while changing hardware
        HAL_READ_UINT8(port+SER_16550_IER, ier);
        HAL_WRITE_UINT8(port+SER_16550_IER, 0);

        // Set baud rate.
        cyg_hal_plf_serial_set_baud(port, baud_divisor);

        // Reenable interrupts if necessary
        HAL_WRITE_UINT8(port+SER_16550_IER, ier);
    }
    break;

    case __COMMCTL_GETBAUD:
        break;
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
    int res = 0;
    cyg_uint8 _iir, c;
    channel_data_t* chan;
    CYGARC_HAL_SAVE_GP();

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    chan = (channel_data_t*)__ch_data;

    HAL_INTERRUPT_ACKNOWLEDGE(chan->isr_vector);

    HAL_READ_UINT8(chan->base + SER_16550_IIR, _iir);
    _iir &= SIO_IIR_ID_MASK;

    *__ctrlc = 0;
    if ((_iir == ISR_Rx_Avail) || (_iir == ISR_Rx_Char_Timeout)) {

        HAL_READ_UINT8(chan->base + SER_16550_RBR, c);
    
        if( cyg_hal_is_break( &c , 1 ) )
            *__ctrlc = 1;

        res = CYG_ISR_HANDLED;
    }

    /* sfurman - Hmmm.  Under or1ksim, we sometimes receive interrupts
       when no characters are in the FIFO.  I think this is a SW bug
       and not a problem w/ or1ksim, but until the problem is solved,
       we always consume the interrupt */
    res = CYG_ISR_HANDLED;

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static void
cyg_hal_plf_serial_init(void)
{
    int i;
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    //#define NUM_CHANNELS (sizeof(channels)/sizeof(channels[0]))
#define NUM_CHANNELS CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS
    for (i = 0; i < NUM_CHANNELS; i++) {

	// Disable interrupts.
	HAL_INTERRUPT_MASK(channels[i].isr_vector);

	// Init channels
	cyg_hal_plf_serial_init_channel((void*)&channels[i]);
    
	// Setup procs in the vector table

	// Set COMM callbacks for channel
	CYGACC_CALL_IF_SET_CONSOLE_COMM(i);
	comm = CYGACC_CALL_IF_CONSOLE_PROCS();
	CYGACC_COMM_IF_CH_DATA_SET(*comm, &channels[i]);
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

//-----------------------------------------------------------------------------
// end of hal_diag.c
