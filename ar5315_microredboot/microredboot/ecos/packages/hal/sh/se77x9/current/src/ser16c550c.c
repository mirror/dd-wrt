//=============================================================================
//
//      ser16c550c.c
//
//      Simple driver for the 16c550c serial controllers on the HS7729PCI board
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
// Author(s):   dmoseley
// Contributors:dmoseley, jskov
// Date:        2001-03-20
// Description: Simple driver for the 16c550c serial controller
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

//-----------------------------------------------------------------------------
// Define the serial registers. The SE77x9 board are equipped with a 16550C
// serial chips and different addresses and clocked at different rates.
// Details are in CDL.
// The registers are accessed as 16 bit, but only the upper 8 bits contain data.
#define SE77X9_SER_CLOCK           CYGNUM_HAL_SH_SE77X9_16550_CLOCK
#define SE77X9_SER_16550_BASE_A    CYGNUM_HAL_SH_SE77X9_16550_BASE
#define SER_16550_RBR 0x00   // receiver buffer register, read, dlab = 0
#define SER_16550_THR 0x00   // transmitter holding register, write, dlab = 0
#define SER_16550_DLL 0x00   // divisor latch (LS), read/write, dlab = 1
#define SER_16550_IER 0x02   // interrupt enable register, read/write, dlab = 0
#define SER_16550_DLM 0x02   // divisor latch (MS), read/write, dlab = 1
#define SER_16550_IIR 0x04   // interrupt identification reg, read, dlab = 0
#define SER_16550_FCR 0x04   // fifo control register, write, dlab = 0
#define SER_16550_AFR 0x04   // alternate function reg, read/write, dlab = 1
#define SER_16550_LCR 0x06   // line control register, read/write
#define SER_16550_MCR 0x08   // modem control register, read/write
#define SER_16550_LSR 0x0a   // line status register, read
#define SER_16550_MSR 0x0c   // modem status register, read
#define SER_16550_SCR 0x0e   // scratch pad register

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
#define MCR_AFE 0x20

// Interrupt status register
#define ISR_None             0x01
#define ISR_Rx_Line_Status   0x06
#define ISR_Rx_Avail         0x04
#define ISR_Rx_Char_Timeout  0x0C
#define ISR_Tx_Empty         0x02
#define IRS_Modem_Status     0x00

// FIFO control register
#define FCR_ENABLE     0x01
#define FCR_CLEAR_RCVR 0x02
#define FCR_CLEAR_XMIT 0x04

#define CYG_DEV_SERIAL_BAUD_DIVISOR (SE77X9_SER_CLOCK/16/CYGNUM_HAL_VIRTUAL_VECTOR_CHANNELS_DEFAULT_BAUD)

//-----------------------------------------------------------------------------

#define UART_READ_UINT8(_a_, _d_)               \
    CYG_MACRO_START                             \
    cyg_uint16 t;                               \
    HAL_READ_UINT16((_a_), t);                  \
    (_d_) = (t >> 8) & 0xff;                    \
    CYG_MACRO_END

#define UART_WRITE_UINT8(_a_, _d_)              \
    CYG_MACRO_START                             \
    HAL_WRITE_UINT16((_a_), (_d_)<<8);          \
    CYG_MACRO_END


//-----------------------------------------------------------------------------
typedef struct {
    cyg_uint8* base;
    cyg_int32 msec_timeout;
    int isr_vector;
} channel_data_t;

static channel_data_t channels[1] = {
    { (cyg_uint8*)SE77X9_SER_16550_BASE_A, 1000, CYGNUM_HAL_INTERRUPT_COM1 },
};

//-----------------------------------------------------------------------------
// Set the baud rate

static void
cyg_hal_plf_serial_set_baud(cyg_uint8* port, cyg_uint16 baud_divisor)
{
    cyg_uint8 _lcr;

    UART_READ_UINT8(port+SER_16550_LCR, _lcr);
    _lcr |= LCR_DL;
    UART_WRITE_UINT8(port+SER_16550_LCR, _lcr);

    UART_WRITE_UINT8(port+SER_16550_DLM, baud_divisor >> 8);
    UART_WRITE_UINT8(port+SER_16550_DLL, baud_divisor & 0xff);

    _lcr &= ~LCR_DL;
    UART_WRITE_UINT8(port+SER_16550_LCR, _lcr);
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
    UART_WRITE_UINT8(port+SER_16550_IER, 0);

    // Set databits, stopbits and parity.
    _lcr = LCR_WL8 | LCR_SB1 | LCR_PN;
    UART_WRITE_UINT8(port+SER_16550_LCR, _lcr);

    // Set baud rate.
    cyg_hal_plf_serial_set_baud(port, CYG_DEV_SERIAL_BAUD_DIVISOR);

    // Enable and clear FIFO
    UART_WRITE_UINT8(port+SER_16550_FCR, (FCR_ENABLE | FCR_CLEAR_RCVR | FCR_CLEAR_XMIT));

    // enable RTS to keep host side happy. Also allow interrupts
    UART_WRITE_UINT8( port+SER_16550_MCR, MCR_DTR | MCR_RTS | MCR_INT);
    
    // Don't allow interrupts.
    UART_WRITE_UINT8(port+SER_16550_IER, 0);
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
        UART_READ_UINT8(port+SER_16550_LSR, _lsr);
    } while ((_lsr & SIO_LSR_THRE) == 0);

    // Now, the transmit buffer is empty
    UART_WRITE_UINT8(port+SER_16550_THR, __ch);

    // Hang around until the character has been safely sent.
    do {
        UART_READ_UINT8(port+SER_16550_LSR, _lsr);
    } while ((_lsr & SIO_LSR_THRE) == 0);

    CYGARC_HAL_RESTORE_GP();
}

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

    UART_READ_UINT8(port+SER_16550_LSR, _lsr);
    if ((_lsr & SIO_LSR_DR) == 0)
        return false;

    UART_READ_UINT8(port+SER_16550_RBR, *ch);

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

        UART_READ_UINT8(chan->base + SER_16550_IER, ier);
        ier |= SIO_IER_ERDAI;
        UART_WRITE_UINT8(chan->base + SER_16550_IER, ier);

        HAL_INTERRUPT_SET_LEVEL(chan->isr_vector, 1);
        HAL_INTERRUPT_UNMASK(chan->isr_vector);
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;

        UART_READ_UINT8(chan->base + SER_16550_IER, ier);
        ier &= ~SIO_IER_ERDAI;
        UART_WRITE_UINT8(chan->base + SER_16550_IER, ier);

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

        baud_divisor = (SE77X9_SER_CLOCK / 16 / baud_rate);

        // Disable port interrupts while changing hardware
        UART_READ_UINT8(port+SER_16550_IER, ier);
        UART_WRITE_UINT8(port+SER_16550_IER, 0);

        // Set baud rate.
        cyg_hal_plf_serial_set_baud(port, baud_divisor);

        // Reenable interrupts if necessary
        UART_WRITE_UINT8(port+SER_16550_IER, ier);
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

    UART_READ_UINT8(chan->base + SER_16550_IIR, _iir);
    _iir &= SIO_IIR_ID_MASK;

    *__ctrlc = 0;
    if ((_iir == ISR_Rx_Avail) || (_iir == ISR_Rx_Char_Timeout)) {

        UART_READ_UINT8(chan->base + SER_16550_RBR, c);
    
        if( cyg_hal_is_break( &c , 1 ) )
            *__ctrlc = 1;

        res = CYG_ISR_HANDLED;
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

void
cyg_hal_plf_serial_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    // Disable interrupts.
    HAL_INTERRUPT_MASK(channels[0].isr_vector);

    // Init channels
    cyg_hal_plf_serial_init_channel((void*)&channels[0]);
    
    // Setup procs in the vector table

    // Set channel 0
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &channels[0]);
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

//-----------------------------------------------------------------------------
// end of ser16c550c.c
