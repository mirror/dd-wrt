//==========================================================================
//
//      io/serial/arm/integrator_serial_with_ints.c
//
//      ARM INTEGRATOR Serial I/O Interface Module (interrupt driven)
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    David A Rusling
// Contributors: Philippe Robin
// Date:         November 7, 2000
// Purpose:      INTEGRATOR Serial I/O module (interrupt driven)
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/io.h>
#include <pkgconf/io_serial.h>
#include <cyg/io/io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/io/devtab.h>
#include <cyg/io/serial.h>
#include <cyg/infra/diag.h>

#ifdef CYGPKG_IO_SERIAL_ARM_INTEGRATOR
#include "integrator_serial.h"

typedef struct integrator_serial_info {
    CYG_ADDRWORD   base;
    CYG_WORD       int_num;
    cyg_interrupt  serial_interrupt;
    cyg_handle_t   serial_interrupt_handle;
} integrator_serial_info;

static bool integrator_serial_init(struct cyg_devtab_entry *tab);
static bool integrator_serial_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo integrator_serial_lookup(struct cyg_devtab_entry **tab, 
                                   struct cyg_devtab_entry *sub_tab,
                                   const char *name);
static unsigned char integrator_serial_getc(serial_channel *chan);
static Cyg_ErrNo integrator_serial_set_config(serial_channel *chan, cyg_uint32 key,
					      const void *xbuf, cyg_uint32 *len);
static void integrator_serial_start_xmit(serial_channel *chan);
static void integrator_serial_stop_xmit(serial_channel *chan);

static cyg_uint32 integrator_serial_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       integrator_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);

static SERIAL_FUNS(integrator_serial_funs, 
                   integrator_serial_putc, 
                   integrator_serial_getc,
                   integrator_serial_set_config,
                   integrator_serial_start_xmit,
                   integrator_serial_stop_xmit
    );

#ifdef CYGPKG_IO_SERIAL_ARM_INTEGRATOR_SERIAL0
#define INTEGRATOR_UART0_BASE           0x16000000	 /*  UART 0 */
static integrator_serial_info integrator_serial_info0 = {INTEGRATOR_UART0_BASE, CYGNUM_HAL_INTERRUPT_UARTINT0};
#if CYGNUM_IO_SERIAL_ARM_INTEGRATOR_SERIAL0_BUFSIZE > 0
static unsigned char integrator_serial_out_buf0[CYGNUM_IO_SERIAL_ARM_INTEGRATOR_SERIAL0_BUFSIZE];
static unsigned char integrator_serial_in_buf0[CYGNUM_IO_SERIAL_ARM_INTEGRATOR_SERIAL0_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(integrator_serial_channel0,
                                       integrator_serial_funs, 
                                       integrator_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_INTEGRATOR_SERIAL0_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &integrator_serial_out_buf0[0], sizeof(integrator_serial_out_buf0),
                                       &integrator_serial_in_buf0[0], sizeof(integrator_serial_in_buf0)
    );
#else
static SERIAL_CHANNEL(integrator_serial_channel0,
                      integrator_serial_funs, 
                      integrator_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_INTEGRATOR_SERIAL0_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(integrator_serial_io0, 
             CYGDAT_IO_SERIAL_ARM_INTEGRATOR_SERIAL0_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             integrator_serial_init, 
             integrator_serial_lookup,     // Serial driver may need initializing
             &integrator_serial_channel0
    );
#endif //  CYGPKG_IO_SERIAL_ARM_INTEGRATOR_SERIAL0

#ifdef CYGPKG_IO_SERIAL_ARM_INTEGRATOR_SERIAL1
#define INTEGRATOR_UART1_BASE           0x17000000	 /*  UART 1 */
static integrator_serial_info integrator_serial_info1 = {INTEGRATOR_UART1_BASE, CYGNUM_HAL_INTERRUPT_UARTINT1};
#if CYGNUM_IO_SERIAL_ARM_INTEGRATOR_SERIAL1_BUFSIZE > 0
static unsigned char integrator_serial_out_buf1[CYGNUM_IO_SERIAL_ARM_INTEGRATOR_SERIAL1_BUFSIZE];
static unsigned char integrator_serial_in_buf1[CYGNUM_IO_SERIAL_ARM_INTEGRATOR_SERIAL1_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(integrator_serial_channel1,
                                       integrator_serial_funs, 
                                       integrator_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_INTEGRATOR_SERIAL1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &integrator_serial_out_buf1[0], sizeof(integrator_serial_out_buf1),
                                       &integrator_serial_in_buf1[0], sizeof(integrator_serial_in_buf1)
    );
#else
static SERIAL_CHANNEL(integrator_serial_channel1,
                      integrator_serial_funs, 
                      integrator_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_INTEGRATOR_SERIAL1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(integrator_serial_io1, 
             CYGDAT_IO_SERIAL_ARM_INTEGRATOR_SERIAL1_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             integrator_serial_init, 
             integrator_serial_lookup,     // Serial driver may need initializing
             &integrator_serial_channel1
    );
#endif //  CYGPKG_IO_SERIAL_ARM_INTEGRATOR_SERIAL1

#define GET_INTERRUPT_STATUS(p)           IO_READ((p) + AMBA_UARTIIR)
#define GET_STATUS(p)		          (IO_READ((p) + AMBA_UARTFR))
#define GET_CHAR(p)		          (IO_READ((p) + AMBA_UARTDR))
#define PUT_CHAR(p, c)		          (IO_WRITE(((p) + AMBA_UARTDR), (c)))
#define IO_READ(p)                        ((*(volatile unsigned int *)(p)) & 0xFF)
#define IO_WRITE(p, c)                    (*(unsigned int *)(p) = (c))
#define RX_DATA(s)		          (((s) & AMBA_UARTFR_RXFE) == 0)
#define TX_READY(s)		          (((s) & AMBA_UARTFR_TXFF) == 0)
#define TX_EMPTY(p)		          ((GET_STATUS(p) & AMBA_UARTFR_TMSK) == 0)

// debugging help
static int chars_rx = 0 ;
static int chars_tx = 0 ;

// Internal function to actually configure the hardware to desired baud rate, etc.
static bool
integrator_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
    integrator_serial_info *integrator_chan = (integrator_serial_info *)chan->dev_priv;
    unsigned int port = (unsigned int)integrator_chan->base;
    unsigned short baud_divisor = select_baud[new_config->baud];
    unsigned char _lcr ;

    // don't do this baud rate...
    if (baud_divisor == 0) return false;  // Invalid configuration

    // first, disable everything 
    IO_WRITE(port + AMBA_UARTCR, 0x0);

    // Set baud rate 
    IO_WRITE(port + AMBA_UARTLCR_M, ((baud_divisor & 0xf00) >> 8));
    IO_WRITE(port + AMBA_UARTLCR_L, (baud_divisor & 0xff));
    
    // ----------v----------v----------v----------v---------- 
    // NOTE: MUST BE WRITTEN LAST (AFTER UARTLCR_M & UARTLCR_L) 
    // ----------^----------^----------^----------^----------
    _lcr = 
      select_word_length[new_config->word_length - CYGNUM_SERIAL_WORD_LENGTH_5] | 
      select_stop_bits[new_config->stop] |
      select_parity[new_config->parity] | AMBA_UARTLCR_H_FEN ;
    IO_WRITE(port + AMBA_UARTLCR_H, _lcr);
    
    // finally, enable the uart 
    IO_WRITE(port + AMBA_UARTCR, (AMBA_UARTCR_RIE | AMBA_UARTCR_RTIE | AMBA_UARTCR_UARTEN));

    // save the configuration
    if (new_config != &chan->config) {
        chan->config = *new_config;
    }

    // success
    return true;
}

// Function to initialize the device.  Called at bootstrap time.
static bool 
integrator_serial_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    integrator_serial_info *integrator_chan = (integrator_serial_info *)chan->dev_priv;
#ifdef CYGDBG_IO_INIT
    diag_printf("INTEGRATOR SERIAL init - dev: %x.%d\n", integrator_chan->base, integrator_chan->int_num);
#endif
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    if (chan->out_cbuf.len != 0) {
        cyg_drv_interrupt_create(integrator_chan->int_num,
                                 99,                     // Priority - what goes here?
                                 (cyg_addrword_t)chan,   //  Data item passed to interrupt handler
                                 integrator_serial_ISR,
                                 integrator_serial_DSR,
                                 &integrator_chan->serial_interrupt_handle,
                                 &integrator_chan->serial_interrupt);
        cyg_drv_interrupt_attach(integrator_chan->serial_interrupt_handle);
        cyg_drv_interrupt_unmask(integrator_chan->int_num);
    }
    integrator_serial_config_port(chan, &chan->config, true);
    return true;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
integrator_serial_lookup(struct cyg_devtab_entry **tab, 
                  struct cyg_devtab_entry *sub_tab,
                  const char *name)
{
    serial_channel *chan = (serial_channel *)(*tab)->priv;
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    return ENOERR;
}

// Send a character to the device output buffer.
// Return 'true' if character is sent to device
static bool
integrator_serial_putc(serial_channel *chan, unsigned char c)
{
    integrator_serial_info *integrator_chan = (integrator_serial_info *)chan->dev_priv;
    unsigned int status = GET_STATUS(integrator_chan->base) ;

    if (TX_READY(status)) {
// Transmit buffer is empty
        PUT_CHAR(integrator_chan->base, c) ;
	chars_tx++ ;
        return true;
    } else {
// No space
        return false;
    }
}

// Fetch a character from the device input buffer, waiting if necessary
static unsigned char 
integrator_serial_getc(serial_channel *chan)
{
    unsigned char c;
    integrator_serial_info *integrator_chan = (integrator_serial_info *)chan->dev_priv;
    unsigned int status ;

    do {
        status = GET_STATUS(integrator_chan->base) ;
    } while (!RX_DATA(status)) ;                   // Wait for char

    chars_rx++ ;

    // get it 
    c = GET_CHAR(integrator_chan->base) ;
    return c;
}

// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
integrator_serial_set_config(serial_channel *chan, cyg_uint32 key, const void *xbuf,
                      cyg_uint32 *len)
{
    integrator_serial_info *integrator_chan = (integrator_serial_info *)chan->dev_priv;

    switch (key) {
    case CYG_IO_SET_CONFIG_SERIAL_INFO:
      {
        cyg_serial_info_t *config = (cyg_serial_info_t *)xbuf;
        if ( *len < sizeof(cyg_serial_info_t) ) {
            return -EINVAL;
        }
        *len = sizeof(cyg_serial_info_t);
        if ( true != integrator_serial_config_port(chan, config, false) )
            return -EINVAL;
      }
      break;
#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_HW
#ifdef FIXME
    case CYG_IO_SET_CONFIG_SERIAL_HW_RX_FLOW_THROTTLE:
      {
          volatile struct serial_port *port = (volatile struct serial_port *)integrator_chan->base;
          cyg_uint32 *f = (cyg_uint32 *)xbuf;
          unsigned char mask=0;
          if ( *len < *f )
              return -EINVAL;
          
          if ( chan->config.flags & CYGNUM_SERIAL_FLOW_RTSCTS_RX )
              mask = MCR_RTS;
          if ( chan->config.flags & CYGNUM_SERIAL_FLOW_DSRDTR_RX )
              mask |= MCR_DTR;
          if (*f) // we should throttle
              port->REG_mcr &= ~mask;
          else // we should no longer throttle
              port->REG_mcr |= mask;
      }
      break;
    case CYG_IO_SET_CONFIG_SERIAL_HW_FLOW_CONFIG:
        // Nothing to do because we do support both RTSCTS and DSRDTR flow
        // control.
        // Other targets would clear any unsupported flags here.
        // We just return ENOERR.
      break;
#else
#error "Flow control for Integrator not integrated!"
#endif
#endif
    default:
        return -EINVAL;
    }
    return ENOERR;
}

// Enable the transmitter on the device
static void
integrator_serial_start_xmit(serial_channel *chan)
{
    integrator_serial_info *integrator_chan = (integrator_serial_info *)chan->dev_priv;

    IO_WRITE(integrator_chan->base + AMBA_UARTCR, 
	     IO_READ(integrator_chan->base + AMBA_UARTCR) | AMBA_UARTCR_TIE);
}

// Disable the transmitter on the device
static void 
integrator_serial_stop_xmit(serial_channel *chan)
{
    integrator_serial_info *integrator_chan = (integrator_serial_info *)chan->dev_priv;

    IO_WRITE(integrator_chan->base + AMBA_UARTCR, 
	     IO_READ(integrator_chan->base + AMBA_UARTCR) & ~AMBA_UARTCR_TIE);
}

// Serial I/O - low level interrupt handler (ISR)
static cyg_uint32 
integrator_serial_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    integrator_serial_info *integrator_chan = (integrator_serial_info *)chan->dev_priv;

    cyg_drv_interrupt_mask(integrator_chan->int_num);
    cyg_drv_interrupt_acknowledge(integrator_chan->int_num);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level interrupt handler (DSR)
static void       
integrator_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    integrator_serial_info *integrator_chan = (integrator_serial_info *)chan->dev_priv;
    volatile unsigned char isr = GET_INTERRUPT_STATUS(integrator_chan->base) ;

    while ((isr & (AMBA_UARTIIR_RTIS | AMBA_UARTIIR_TIS | AMBA_UARTIIR_RIS)) != 0) {
        if (isr & AMBA_UARTIIR_TIS) {
            (chan->callbacks->xmt_char)(chan);
        } else if (isr & AMBA_UARTIIR_RTIS) {
	    chars_rx++ ;
            (chan->callbacks->rcv_char)(chan, GET_CHAR(integrator_chan->base));
        } else if (isr & AMBA_UARTIIR_RIS) {
	    chars_rx++ ;
            (chan->callbacks->rcv_char)(chan, GET_CHAR(integrator_chan->base));
        }
	isr = GET_INTERRUPT_STATUS(integrator_chan->base) ;
    }
    cyg_drv_interrupt_unmask(integrator_chan->int_num);
}
#endif
