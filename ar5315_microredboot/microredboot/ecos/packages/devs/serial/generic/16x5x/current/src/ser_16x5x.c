//==========================================================================
//
//      io/serial/generic/16x5x/ser_16x5x.c
//
//      Generic 16x5x serial driver
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
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
// Author(s):    gthomas
// Contributors: gthomas, jlarmour, jskov
// Date:         1999-02-04
// Purpose:      16x5x generic serial driver
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/io_serial.h>
#include <pkgconf/io.h>

#include <cyg/io/io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/io/devtab.h>
#include <cyg/io/serial.h>
#include <cyg/infra/diag.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/hal/hal_io.h>

// Only compile driver if an inline file with driver details was selected.
#ifdef CYGDAT_IO_SERIAL_GENERIC_16X5X_INL

#ifndef CYGPRI_IO_SERIAL_GENERIC_16X5X_STEP
#define CYGPRI_IO_SERIAL_GENERIC_16X5X_STEP 1
#endif

#define SER_REG(_x_) ((_x_)*CYGPRI_IO_SERIAL_GENERIC_16X5X_STEP)

// Receive control Registers
#define REG_rhr SER_REG(0)    // Receive holding register
#define REG_isr SER_REG(2)    // Interrupt status register
#define REG_lsr SER_REG(5)    // Line status register
#define REG_msr SER_REG(6)    // Modem status register
#define REG_scr SER_REG(7)    // Scratch register

// Transmit control Registers
#define REG_thr SER_REG(0)    // Transmit holding register
#define REG_ier SER_REG(1)    // Interrupt enable register
#define REG_fcr SER_REG(2)    // FIFO control register
#define REG_lcr SER_REG(3)    // Line control register
#define REG_mcr SER_REG(4)    // Modem control register
#define REG_ldl SER_REG(0)    // LSB of baud rate
#define REG_mdl SER_REG(1)    // MSB of baud rate

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
#define LCR_PE  0x18    // Parity mode - even
#define LCR_PO  0x08    // Parity mode - odd
#define LCR_PM  0x28    // Forced "mark" parity
#define LCR_PS  0x38    // Forced "space" parity
#define LCR_DL  0x80    // Enable baud rate latch

// Line Status Register
#define LSR_RSR 0x01
#define LSR_OE  0x02
#define LSR_PE  0x04
#define LSR_FE  0x08
#define LSR_BI  0x10
#define LSR_THE 0x20
#define LSR_TEMT 0x40
#define LSR_FIE 0x80

// Modem Control Register
#define MCR_DTR  0x01
#define MCR_RTS  0x02
#define MCR_INT  0x08   // Enable interrupts
#define MCR_LOOP 0x10   // Loopback mode

// Interrupt status Register
#define ISR_MS        0x00
#define ISR_nIP       0x01
#define ISR_Tx        0x02
#define ISR_Rx        0x04
#define ISR_LS        0x06
#define ISR_RxTO      0x0C
#define ISR_64BFIFO   0x20
#define ISR_FIFOworks 0x40
#define ISR_FIFOen    0x80

// Modem Status Register
#define MSR_DCTS 0x01
#define MSR_DDSR 0x02
#define MSR_TERI 0x04
#define MSR_DDCD 0x08
#define MSR_CTS  0x10
#define MSR_DSR  0x20
#define MSR_RI   0x40
#define MSR_CD   0x80

// FIFO Control Register
#define FCR_FE   0x01    // FIFO enable
#define FCR_CRF  0x02    // Clear receive FIFO
#define FCR_CTF  0x04    // Clear transmit FIFO
#define FCR_DMA  0x08    // DMA mode select
#define FCR_F64  0x20    // Enable 64 byte fifo (16750+)
#define FCR_RT14 0xC0    // Set Rx trigger at 14
#define FCR_RT8  0x80    // Set Rx trigger at 8
#define FCR_RT4  0x40    // Set Rx trigger at 4
#define FCR_RT1  0x00    // Set Rx trigger at 1

static unsigned char select_word_length[] = {
    LCR_WL5,    // 5 bits / word (char)
    LCR_WL6,
    LCR_WL7,
    LCR_WL8
};

static unsigned char select_stop_bits[] = {
    0,
    LCR_SB1,    // 1 stop bit
    LCR_SB1_5,  // 1.5 stop bit
    LCR_SB2     // 2 stop bits
};

static unsigned char select_parity[] = {
    LCR_PN,     // No parity
    LCR_PE,     // Even parity
    LCR_PO,     // Odd parity
    LCR_PM,     // Mark parity
    LCR_PS,     // Space parity
};

// selec_baud[] must be define by the client

typedef struct pc_serial_info {
    cyg_addrword_t base;
    int            int_num;
    cyg_interrupt  serial_interrupt;
    cyg_handle_t   serial_interrupt_handle;
#ifdef CYGPKG_IO_SERIAL_GENERIC_16X5X_FIFO
    enum {
        sNone = 0,
        s8250,
        s16450,
        s16550,
        s16550a
    } deviceType;
#endif
} pc_serial_info;

static bool pc_serial_init(struct cyg_devtab_entry *tab);
static bool pc_serial_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo pc_serial_lookup(struct cyg_devtab_entry **tab, 
                                  struct cyg_devtab_entry *sub_tab,
                                  const char *name);
static unsigned char pc_serial_getc(serial_channel *chan);
static Cyg_ErrNo pc_serial_set_config(serial_channel *chan, cyg_uint32 key,
                                      const void *xbuf, cyg_uint32 *len);
static void pc_serial_start_xmit(serial_channel *chan);
static void pc_serial_stop_xmit(serial_channel *chan);

static cyg_uint32 pc_serial_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       pc_serial_DSR(cyg_vector_t vector, cyg_ucount32 count,
                                cyg_addrword_t data);

static SERIAL_FUNS(pc_serial_funs, 
                   pc_serial_putc, 
                   pc_serial_getc,
                   pc_serial_set_config,
                   pc_serial_start_xmit,
                   pc_serial_stop_xmit
    );

#include CYGDAT_IO_SERIAL_GENERIC_16X5X_INL

#ifndef CYG_IO_SERIAL_GENERIC_16X5X_INT_PRIORITY
# define CYG_IO_SERIAL_GENERIC_16X5X_INT_PRIORITY 4
#endif


// Internal function to actually configure the hardware to desired
// baud rate, etc.
static bool
serial_config_port(serial_channel *chan, 
                   cyg_serial_info_t *new_config, bool init)
{
    pc_serial_info *ser_chan = (pc_serial_info *)chan->dev_priv;
    cyg_addrword_t base = ser_chan->base;
    unsigned short baud_divisor = select_baud[new_config->baud];
    unsigned char _lcr, _ier;
    if (baud_divisor == 0) return false;  // Invalid configuration

    // Disable port interrupts while changing hardware
    HAL_READ_UINT8(base+REG_ier, _ier);
    HAL_WRITE_UINT8(base+REG_ier, 0);

    _lcr = select_word_length[new_config->word_length - CYGNUM_SERIAL_WORD_LENGTH_5] | 
        select_stop_bits[new_config->stop] |
        select_parity[new_config->parity];
    HAL_WRITE_UINT8(base+REG_lcr, _lcr | LCR_DL);
    HAL_WRITE_UINT8(base+REG_mdl, baud_divisor >> 8);
    HAL_WRITE_UINT8(base+REG_ldl, baud_divisor & 0xFF);
    HAL_WRITE_UINT8(base+REG_lcr, _lcr);
    if (init) {
#ifdef CYGPKG_IO_SERIAL_GENERIC_16X5X_FIFO
        unsigned char _fcr_thresh;
        cyg_uint8 b;

        /* First, find out what kind of device it is. */
        ser_chan->deviceType = sNone;
        HAL_WRITE_UINT8(base+REG_mcr, MCR_LOOP); // enable loopback mode
        HAL_READ_UINT8(base+REG_msr, b);         
        if (0 == (b & 0xF0)) {   // see if MSR had CD, RI, DSR or CTS set
            HAL_WRITE_UINT8(base+REG_mcr, MCR_LOOP|MCR_DTR|MCR_RTS);
            HAL_READ_UINT8(base+REG_msr, b);
            if (0xF0 != (b & 0xF0))  // check that all of CD,RI,DSR and CTS set
                ser_chan->deviceType = s8250;
        }
        HAL_WRITE_UINT8(base+REG_mcr, 0); // disable loopback mode

        if (ser_chan->deviceType == s8250) {
            // Check for a scratch register; scratch register 
            // indicates 16450 or above.
            HAL_WRITE_UINT8(base+REG_scr, 0x55);
            HAL_READ_UINT8(base+REG_scr, b);
            if (b == 0x55) {
                HAL_WRITE_UINT8(base+REG_scr, 0xAA);
                HAL_READ_UINT8(base+REG_scr, b);
                if (b == 0xAA)
                    ser_chan->deviceType = s16450;
            }
        }

        if (ser_chan->deviceType == s16450) {
            // Check for a FIFO
            HAL_WRITE_UINT8(base+REG_fcr, FCR_FE);
            HAL_READ_UINT8(base+REG_isr, b);
            if (b & ISR_FIFOen)
                ser_chan->deviceType = s16550; // but FIFO doesn't 
                                               // necessarily work
            if (b & ISR_FIFOworks)
                ser_chan->deviceType = s16550a; // 16550a FIFOs work
        }

        if (ser_chan->deviceType == s16550a) {
            switch(CYGPKG_IO_SERIAL_GENERIC_16X5X_FIFO_RX_THRESHOLD) {
            default:
            case 1:
                _fcr_thresh=FCR_RT1; break;
            case 4:
                _fcr_thresh=FCR_RT4; break;
            case 8:
                _fcr_thresh=FCR_RT8; break;
            case 14:
                _fcr_thresh=FCR_RT14; break;
            }
            _fcr_thresh|=FCR_FE|FCR_CRF|FCR_CTF;
            HAL_WRITE_UINT8(base+REG_fcr, _fcr_thresh); // Enable and clear FIFO
        }
        else
            HAL_WRITE_UINT8(base+REG_fcr, 0); // make sure it's disabled
#endif
        if (chan->out_cbuf.len != 0) {
            _ier = IER_RCV;
        } else {
            _ier = 0;
        }
        // Master interrupt enable
        HAL_WRITE_UINT8(base+REG_mcr, MCR_INT|MCR_DTR|MCR_RTS);
    }
#ifdef CYGOPT_IO_SERIAL_SUPPORT_LINE_STATUS
    _ier |= (IER_LS|IER_MS);
#endif
    HAL_WRITE_UINT8(base+REG_ier, _ier);

    if (new_config != &chan->config) {
        chan->config = *new_config;
    }
    return true;
}

// Function to initialize the device.  Called at bootstrap time.
static bool 
pc_serial_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    pc_serial_info *ser_chan = (pc_serial_info *)chan->dev_priv;

#ifdef CYG_IO_SERIAL_GENERIC_16X5X_BAUD_GENERATOR
    // Fill in baud rate table - used for platforms where this cannot
    // be determined statically
    int baud_idx, baud_val;
    if (select_baud[0] == 9999) {
        // Table not yet initialized
        // Assumes that 'select_baud' looks like this:
        //   static int select_baud[] = {
        //       9999,  -- marker
        //       50,    -- first baud rate
        //       110,   -- second baud rate
        // etc.
        for (baud_idx = 1;  baud_idx < sizeof(select_baud)/sizeof(select_baud[0]);  baud_idx++) {
            baud_val = CYG_IO_SERIAL_GENERIC_16X5X_BAUD_GENERATOR(select_baud[baud_idx]);
            select_baud[baud_idx] = baud_val;
        }
        select_baud[0] = 0;
    }
#endif

#ifdef CYGDBG_IO_INIT
    diag_printf("16x5x SERIAL init - dev: %x.%d\n", 
                ser_chan->base, ser_chan->int_num);
#endif
    // Really only required for interrupt driven devices
    (chan->callbacks->serial_init)(chan);

    if (chan->out_cbuf.len != 0) {
        cyg_drv_interrupt_create(ser_chan->int_num,
                                 CYG_IO_SERIAL_GENERIC_16X5X_INT_PRIORITY,
                                 (cyg_addrword_t)chan,
                                 pc_serial_ISR,
                                 pc_serial_DSR,
                                 &ser_chan->serial_interrupt_handle,
                                 &ser_chan->serial_interrupt);
        cyg_drv_interrupt_attach(ser_chan->serial_interrupt_handle);
        cyg_drv_interrupt_unmask(ser_chan->int_num);
    }
    serial_config_port(chan, &chan->config, true);
    return true;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
pc_serial_lookup(struct cyg_devtab_entry **tab, 
                 struct cyg_devtab_entry *sub_tab,
                 const char *name)
{
    serial_channel *chan = (serial_channel *)(*tab)->priv;

    // Really only required for interrupt driven devices
    (chan->callbacks->serial_init)(chan);
    return ENOERR;
}

// Send a character to the device output buffer.
// Return 'true' if character is sent to device
static bool
pc_serial_putc(serial_channel *chan, unsigned char c)
{
    cyg_uint8 _lsr;
    pc_serial_info *ser_chan = (pc_serial_info *)chan->dev_priv;
    cyg_addrword_t base = ser_chan->base;

    HAL_READ_UINT8(base+REG_lsr, _lsr);
    if (_lsr & LSR_THE) {
        // Transmit buffer is empty
        HAL_WRITE_UINT8(base+REG_thr, c);
        return true;
    }
    // No space
    return false;
}

// Fetch a character from the device input buffer, waiting if necessary
static unsigned char 
pc_serial_getc(serial_channel *chan)
{
    unsigned char c;
    cyg_uint8 _lsr;
    pc_serial_info *ser_chan = (pc_serial_info *)chan->dev_priv;
    cyg_addrword_t base = ser_chan->base;

    // Wait for char
    do {
        HAL_READ_UINT8(base+REG_lsr, _lsr);
    } while ((_lsr & LSR_RSR) == 0);

    HAL_READ_UINT8(base+REG_rhr, c);
    return c;
}

// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
pc_serial_set_config(serial_channel *chan, cyg_uint32 key, const void *xbuf,
                     cyg_uint32 *len)
{
    switch (key) {
    case CYG_IO_SET_CONFIG_SERIAL_INFO:
      {
        cyg_serial_info_t *config = (cyg_serial_info_t *)xbuf;
        if ( *len < sizeof(cyg_serial_info_t) ) {
            return -EINVAL;
        }
        *len = sizeof(cyg_serial_info_t);
        if ( true != serial_config_port(chan, config, false) )
            return -EINVAL;
      }
      break;
#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_HW
    case CYG_IO_SET_CONFIG_SERIAL_HW_RX_FLOW_THROTTLE:
      {
          cyg_uint8 _mcr;
          pc_serial_info *ser_chan = (pc_serial_info *)chan->dev_priv;
          cyg_addrword_t base = ser_chan->base;
          cyg_uint32 *f = (cyg_uint32 *)xbuf;
          unsigned char mask=0;
          if ( *len < sizeof(*f) )
              return -EINVAL;
          
          if ( chan->config.flags & CYGNUM_SERIAL_FLOW_RTSCTS_RX )
              mask = MCR_RTS;
          if ( chan->config.flags & CYGNUM_SERIAL_FLOW_DSRDTR_RX )
              mask |= MCR_DTR;
          HAL_READ_UINT8(base+REG_mcr, _mcr);
          if (*f) // we should throttle
              _mcr &= ~mask;
          else // we should no longer throttle
              _mcr |= mask;
          HAL_WRITE_UINT8(base+REG_mcr, _mcr);
      }
      break;
    case CYG_IO_SET_CONFIG_SERIAL_HW_FLOW_CONFIG:
        // Nothing to do because we do support both RTSCTS and DSRDTR flow
        // control.
        // Other targets would clear any unsupported flags here and
        // would then return -ENOSUPP - the higher layer can then query
        // what flags are set and decide what to do. This is optimised for
        // the most common case - i.e. that authors know what their hardware
        // is capable of.
        // We just return ENOERR.
      break;
#endif
    default:
        return -EINVAL;
    }
    return ENOERR;
}

// Enable the transmitter on the device
static void
pc_serial_start_xmit(serial_channel *chan)
{
    pc_serial_info *ser_chan = (pc_serial_info *)chan->dev_priv;
    cyg_addrword_t base = ser_chan->base;
    cyg_uint8 _ier;
    
    HAL_READ_UINT8(base+REG_ier, _ier);
    _ier |= IER_XMT;                    // Enable xmit interrupt
    HAL_WRITE_UINT8(base+REG_ier, _ier);
}

// Disable the transmitter on the device
static void 
pc_serial_stop_xmit(serial_channel *chan)
{
    pc_serial_info *ser_chan = (pc_serial_info *)chan->dev_priv;
    cyg_addrword_t base = ser_chan->base;
    cyg_uint8 _ier;

    HAL_READ_UINT8(base+REG_ier, _ier);
    _ier &= ~IER_XMT;                   // Disable xmit interrupt
    HAL_WRITE_UINT8(base+REG_ier, _ier);
}

// Serial I/O - low level interrupt handler (ISR)
static cyg_uint32 
pc_serial_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    pc_serial_info *ser_chan = (pc_serial_info *)chan->dev_priv;
    cyg_drv_interrupt_mask(ser_chan->int_num);
    cyg_drv_interrupt_acknowledge(ser_chan->int_num);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level interrupt handler (DSR)
static void       
pc_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    pc_serial_info *ser_chan = (pc_serial_info *)chan->dev_priv;
    cyg_addrword_t base = ser_chan->base;
    cyg_uint8 _isr;

    // Check if we have an interrupt pending - note that the interrupt
    // is pending of the low bit of the isr is *0*, not 1.
    HAL_READ_UINT8(base+REG_isr, _isr);
    while ((_isr & ISR_nIP) == 0) {
        switch (_isr&0xE) {
        case ISR_Rx:
        case ISR_RxTO:
        {
            cyg_uint8 _lsr;
            unsigned char c;
            HAL_READ_UINT8(base+REG_lsr, _lsr);
            while(_lsr & LSR_RSR) {
                HAL_READ_UINT8(base+REG_rhr, c);
                (chan->callbacks->rcv_char)(chan, c);
                HAL_READ_UINT8(base+REG_lsr, _lsr);
            }
            break;
        }
        case ISR_Tx:
            (chan->callbacks->xmt_char)(chan);
            break;

#ifdef CYGOPT_IO_SERIAL_SUPPORT_LINE_STATUS
        case ISR_LS:
            {
                cyg_serial_line_status_t stat;
                cyg_uint8 _lsr;
                HAL_READ_UINT8(base+REG_lsr, _lsr);

                // this might look expensive, but it is rarely the case that
                // more than one of these is set
                stat.value = 1;
                if ( _lsr & LSR_OE ) {
                    stat.which = CYGNUM_SERIAL_STATUS_OVERRUNERR;
                    (chan->callbacks->indicate_status)(chan, &stat );
                }
                if ( _lsr & LSR_PE ) {
                    stat.which = CYGNUM_SERIAL_STATUS_PARITYERR;
                    (chan->callbacks->indicate_status)(chan, &stat );
                }
                if ( _lsr & LSR_FE ) {
                    stat.which = CYGNUM_SERIAL_STATUS_FRAMEERR;
                    (chan->callbacks->indicate_status)(chan, &stat );
                }
                if ( _lsr & LSR_BI ) {
                    stat.which = CYGNUM_SERIAL_STATUS_BREAK;
                    (chan->callbacks->indicate_status)(chan, &stat );
                }
            }
            break;

        case ISR_MS:
            {
                cyg_serial_line_status_t stat;
                cyg_uint8 _msr;
                
                HAL_READ_UINT8(base+REG_msr, _msr);
#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_HW
                if ( _msr & MSR_DDSR )
                    if ( chan->config.flags & CYGNUM_SERIAL_FLOW_DSRDTR_TX ) {
                        stat.which = CYGNUM_SERIAL_STATUS_FLOW;
                        stat.value = (0 != (_msr & MSR_DSR));
                        (chan->callbacks->indicate_status)(chan, &stat );
                    }
                if ( _msr & MSR_DCTS )
                    if ( chan->config.flags & CYGNUM_SERIAL_FLOW_RTSCTS_TX ) {
                        stat.which = CYGNUM_SERIAL_STATUS_FLOW;
                        stat.value = (0 != (_msr & MSR_CTS));
                        (chan->callbacks->indicate_status)(chan, &stat );
                    }
#endif
                if ( _msr & MSR_DDCD ) {
                    stat.which = CYGNUM_SERIAL_STATUS_CARRIERDETECT;
                    stat.value = (0 != (_msr & MSR_CD));
                    (chan->callbacks->indicate_status)(chan, &stat );
                }
                if ( _msr & MSR_RI ) {
                    stat.which = CYGNUM_SERIAL_STATUS_RINGINDICATOR;
                    stat.value = 1;
                    (chan->callbacks->indicate_status)(chan, &stat );
                }
                if ( _msr & MSR_TERI ) {
                    stat.which = CYGNUM_SERIAL_STATUS_RINGINDICATOR;
                    stat.value = 0;
                    (chan->callbacks->indicate_status)(chan, &stat );
                }
            }
            break;
#endif
        default:
            // Yes, this assertion may well not be visible. *But*
            // if debugging, we may still successfully hit a breakpoint
            // on cyg_assert_fail, which _is_ useful
            CYG_FAIL("unhandled serial interrupt state");
        }

        HAL_READ_UINT8(base+REG_isr, _isr);
    } // while

    cyg_drv_interrupt_unmask(ser_chan->int_num);
}
#endif

// EOF ser_16x5x.c
