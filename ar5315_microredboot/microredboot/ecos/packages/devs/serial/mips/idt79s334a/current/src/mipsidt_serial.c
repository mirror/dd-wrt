//==========================================================================
//
//      mipsidt_serial.c
//
//      Serial device driver for MIPS IDT79s334a reference platform on-chip serial devices
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
// Author(s):    tmichals based on driver by dmoseley, based on POWERPC driver by jskov
// Contributors: gthomas, jskov, dmoseley, tmichals
// Date:         2003-02-13
// Purpose:      MIPS IDT79s334a reference platform serial device driver
// Description:  IDT MIPS serial device driver
//
// To Do:
//   Put in magic to effectively use the FIFOs. Transmitter FIFO fill is a
//   problem, and setting receiver FIFO interrupts to happen only after
//   n chars may conflict with hal diag.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/io_serial.h>
#include <pkgconf/io.h>

#include <cyg/io/io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/io/devtab.h>
#include <cyg/infra/diag.h>
#include <cyg/io/serial.h>

#ifdef CYGPKG_IO_SERIAL_MIPS_IDT79S334A

#include "mipsidt_serial.h"

typedef struct mipsidt_serial_info {
    CYG_ADDRWORD   base;
    CYG_WORD       int_num;
    cyg_interrupt  serial_interrupt;
    cyg_handle_t   serial_interrupt_handle;
    cyg_uint8      iir;
} mipsidt_serial_info;

static bool mipsidt_serial_init(struct cyg_devtab_entry *tab);
static bool mipsidt_serial_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo mipsidt_serial_lookup(struct cyg_devtab_entry **tab, 
                                   struct cyg_devtab_entry *sub_tab,
                                   const char *name);
static unsigned char mipsidt_serial_getc(serial_channel *chan);
static Cyg_ErrNo mipsidt_serial_set_config(serial_channel *chan, cyg_uint32 key,
                                         const void *xbuf, cyg_uint32 *len);
static void mipsidt_serial_start_xmit(serial_channel *chan);
static void mipsidt_serial_stop_xmit(serial_channel *chan);

static cyg_uint32 mipsidt_serial_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       mipsidt_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);

static SERIAL_FUNS(mipsidt_serial_funs, 
                   mipsidt_serial_putc, 
                   mipsidt_serial_getc,
                   mipsidt_serial_set_config,
                   mipsidt_serial_start_xmit,
                   mipsidt_serial_stop_xmit
    );


#ifdef CYGPKG_IO_SERIAL_MIPS_IDT79S334A_SERIAL_A
static mipsidt_serial_info mipsidt_serial_info0 ={IDTMIPS_SER_16550_BASE_A,  
						  CYGNUM_HAL_INTERRUPT_SIO_0};

#if CYGNUM_IO_SERIAL_MIPS_IDT79S334A_SERIAL_A_BUFSIZE > 0
static unsigned char mipsidt_serial_out_buf0[CYGNUM_IO_SERIAL_MIPS_IDT79S334A_SERIAL_A_BUFSIZE];
static unsigned char mipsidt_serial_in_buf0[CYGNUM_IO_SERIAL_MIPS_IDT79S334A_SERIAL_A_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(mipsidt_serial_channel0,
                                       mipsidt_serial_funs, 
                                       mipsidt_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MIPS_IDT79S334A_SERIAL_A_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &mipsidt_serial_out_buf0[0], 
                                       sizeof(mipsidt_serial_out_buf0),
                                       &mipsidt_serial_in_buf0[0], 
                                       sizeof(mipsidt_serial_in_buf0)
    );
#else
static SERIAL_CHANNEL(mipsidt_serial_channel0,
                      mipsidt_serial_funs, 
                      mipsidt_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MIPS_IDT79S334A_SERIAL_A_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(mipsidt_serial_io0, 
             CYGDAT_IO_SERIAL_MIPS_IDT79S334A_SERIAL_A_NAME,
             0,                 // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             mipsidt_serial_init, 
             mipsidt_serial_lookup,     // Serial driver may need initializing
             &mipsidt_serial_channel0
    );

#endif

#ifdef CYGPKG_IO_SERIAL_MIPS_IDT79S334A_SERIAL_B
static mipsidt_serial_info mipsidt_serial_info1 ={IDTMIPS_SER_16550_BASE_B,
						  CYGNUM_HAL_INTERRUPT_SIO_1};

#if CYGNUM_IO_SERIAL_MIPS_IDT79S334A_SERIAL_B_BUFSIZE > 0
static unsigned char mipsidt_serial_out_buf1[CYGNUM_IO_SERIAL_MIPS_IDT79S334A_SERIAL_B_BUFSIZE];
static unsigned char mipsidt_serial_in_buf1[CYGNUM_IO_SERIAL_MIPS_IDT79S334A_SERIAL_B_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(mipsidt_serial_channel1,
                                       mipsidt_serial_funs,
                                       mipsidt_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MIPS_IDT79S334A_SERIAL_B_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &mipsidt_serial_out_buf1[0],
                                       sizeof(mipsidt_serial_out_buf1),
                                       &mipsidt_serial_in_buf1[0],
                                       sizeof(mipsidt_serial_in_buf1)
    );
#else
static SERIAL_CHANNEL(mipsidt_serial_channel1,
                      mipsidt_serial_funs,
                      mipsidt_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MIPS_IDT79S334A_SERIAL_B_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(mipsidt_serial_io1,
             CYGDAT_IO_SERIAL_MIPS_IDT79S334A_SERIAL_B_NAME,
             0,                 // Does not depend on a lower level interface
             &cyg_io_serial_devio,
             mipsidt_serial_init,
             mipsidt_serial_lookup,     // Serial driver may need initializing
             &mipsidt_serial_channel1
    );

#endif

// Internal function to actually configure the hardware to desired baud rate, etc.
static bool
mipsidt_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
    mipsidt_serial_info *mipsidt_chan = (mipsidt_serial_info *)chan->dev_priv;
    cyg_addrword_t port = mipsidt_chan->base;
    cyg_uint32 baud_divisor = select_baud[new_config->baud]; ;
    cyg_uint8 _lcr, _ier;

    if (baud_divisor == 0)
        return false;    // Invalid baud rate selected

	baud_divisor = (CYGHWR_HAL_MIPS_CPU_FREQ_ACTUAL * 10) / (16 * select_baud[new_config->baud]);

	baud_divisor +=5;
	baud_divisor =  ((cyg_int32)baud_divisor) / 10;


    // Disable port interrupts while changing hardware
    HAL_READ_UINT8(port+SER_16550_IER, _ier);
    HAL_WRITE_UINT8(port+SER_16550_IER, 0);

    // Set databits, stopbits and parity.
    _lcr = select_word_length[(new_config->word_length -
                               CYGNUM_SERIAL_WORD_LENGTH_5)] | 
        select_stop_bits[new_config->stop] |
        select_parity[new_config->parity];
    HAL_WRITE_UINT8(port+SER_16550_LCR, _lcr);

    // Set baud rate.
    _lcr |= LCR_DL;
    HAL_WRITE_UINT8(port+SER_16550_LCR, _lcr);
    HAL_WRITE_UINT8(port+SER_16550_DLM, baud_divisor >> 8);
    HAL_WRITE_UINT8(port+SER_16550_DLL, baud_divisor & 0xff);
    _lcr &= ~LCR_DL;
    HAL_WRITE_UINT8(port+SER_16550_LCR, _lcr);

    if (init) {
        // Enable and clear FIFO
        HAL_WRITE_UINT8(port+SER_16550_FCR,
                        (FCR_ENABLE | FCR_CLEAR_RCVR | FCR_CLEAR_XMIT));

        if (chan->out_cbuf.len != 0) {
            HAL_WRITE_UINT8(port+SER_16550_IER, SIO_IER_ERDAI);
        } else {
            HAL_WRITE_UINT8(port+SER_16550_IER, 0);
        }
    } else {
        HAL_WRITE_UINT8(port+SER_16550_IER, _ier);
    }
    if (new_config != &chan->config) {
        chan->config = *new_config;
    }
    return true;
}

// Function to initialize the device.  Called at bootstrap time.
static bool 
mipsidt_serial_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    mipsidt_serial_info *mipsidt_chan = (mipsidt_serial_info *)chan->dev_priv;
#ifdef CYGDBG_IO_INIT
    diag_printf("IDT SERIAL init - dev: %x.%d\n", mipsidt_chan->base, mipsidt_chan->int_num);
#endif
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    if (chan->out_cbuf.len != 0) {
        cyg_drv_interrupt_create(mipsidt_chan->int_num,
                                 0,         // can change IRQ0 priority
                                 (cyg_addrword_t)chan,   //  Data item passed to interrupt handler
                                 mipsidt_serial_ISR,
                                 mipsidt_serial_DSR,
                                 &mipsidt_chan->serial_interrupt_handle,
                                 &mipsidt_chan->serial_interrupt);
        cyg_drv_interrupt_attach(mipsidt_chan->serial_interrupt_handle);
        cyg_drv_interrupt_unmask(mipsidt_chan->int_num);
    }
    mipsidt_serial_config_port(chan, &chan->config, true);
    return true;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
mipsidt_serial_lookup(struct cyg_devtab_entry **tab, 
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
mipsidt_serial_putc(serial_channel *chan, unsigned char c)
{
    mipsidt_serial_info *mipsidt_chan = (mipsidt_serial_info *)chan->dev_priv;
    cyg_addrword_t port = mipsidt_chan->base;
    cyg_uint8 _lsr;

    HAL_READ_UINT8(port+SER_16550_LSR, _lsr);
    if (((_lsr & (SIO_LSR_THRE | SIO_LSR_TEMT)) == 0x60)) {
        // Transmit buffer is empty
        HAL_WRITE_UINT8(port+SER_16550_THR, c);
        return true;
    } else {
        // No space
        return false;
    }
}

// Fetch a character from the device input buffer, waiting if necessary
static unsigned char 
mipsidt_serial_getc(serial_channel *chan)
{
    unsigned char c;
    mipsidt_serial_info *mipsidt_chan = (mipsidt_serial_info *)chan->dev_priv;
    cyg_addrword_t port = mipsidt_chan->base;
    cyg_uint8 _lsr;

    do {
        HAL_READ_UINT8(port+SER_16550_LSR, _lsr);
    } while ((_lsr & SIO_LSR_DR) == 0);
    HAL_READ_UINT8(port+SER_16550_RBR, c);
    return c;
}

// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
mipsidt_serial_set_config(serial_channel *chan, cyg_uint32 key,
                        const void *xbuf, cyg_uint32 *len)
{
    switch (key) {
    case CYG_IO_SET_CONFIG_SERIAL_INFO:
      {
        cyg_serial_info_t *config = (cyg_serial_info_t *)xbuf;
        if ( *len < sizeof(cyg_serial_info_t) ) {
            return -EINVAL;
        }
        *len = sizeof(cyg_serial_info_t);
        if ( true != mipsidt_serial_config_port(chan, config, false) )
            return -EINVAL;
      }
      break;
    default:
        return -EINVAL;
    }
    return ENOERR;
}

// Enable the transmitter on the device
static void
mipsidt_serial_start_xmit(serial_channel *chan)
{
    mipsidt_serial_info *mipsidt_chan = (mipsidt_serial_info *)chan->dev_priv;
    cyg_addrword_t port = mipsidt_chan->base;
    cyg_uint8 _ier;

    HAL_READ_UINT8(port+SER_16550_IER, _ier);
    _ier |= IER_XMT;                    // Enable xmit interrupt
    HAL_WRITE_UINT8(port+SER_16550_IER, _ier);

    // We should not need to call this here.  THRE Interrupts are enabled, and the DSR
    // below calls this function.  However, sometimes we get called with Master Interrupts
    // disabled, and thus the DSR never runs.  This is unfortunate because it means we
    // will be doing multiple processing steps for the same thing.
    (chan->callbacks->xmt_char)(chan);
}

// Disable the transmitter on the device
static void 
mipsidt_serial_stop_xmit(serial_channel *chan)
{
    mipsidt_serial_info *mipsidt_chan = (mipsidt_serial_info *)chan->dev_priv;
    cyg_addrword_t port = mipsidt_chan->base;
    cyg_uint8 _ier;

    HAL_READ_UINT8(port+SER_16550_IER, _ier);
    _ier &= ~IER_XMT;                   // Disable xmit interrupt
    HAL_WRITE_UINT8(port+SER_16550_IER, _ier);
}

// Serial I/O - low level interrupt handler (ISR)
static cyg_uint32 
mipsidt_serial_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    mipsidt_serial_info *mipsidt_chan = (mipsidt_serial_info *)chan->dev_priv;

    cyg_drv_interrupt_mask(mipsidt_chan->int_num);
    cyg_drv_interrupt_acknowledge(mipsidt_chan->int_num);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level interrupt handler (DSR)
static void       
mipsidt_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    mipsidt_serial_info *mipsidt_chan = (mipsidt_serial_info *)chan->dev_priv;
    cyg_addrword_t port = mipsidt_chan->base;
    cyg_uint8 _iir;

    HAL_READ_UINT8(port+SER_16550_IIR, _iir);
    _iir &= SIO_IIR_ID_MASK;
    if ( ISR_Tx_Empty == _iir ) {
        (chan->callbacks->xmt_char)(chan);
    } else if (( ISR_Rx_Avail == _iir ) || ( ISR_Rx_Char_Timeout == _iir )) {
        cyg_uint8 _c;
        HAL_READ_UINT8(port+SER_16550_RBR, _c);
        (chan->callbacks->rcv_char)(chan, _c);
    }

    cyg_drv_interrupt_unmask(mipsidt_chan->int_num);
}

#endif

//-------------------------------------------------------------------------
// EOF mipsidt_serial.c
