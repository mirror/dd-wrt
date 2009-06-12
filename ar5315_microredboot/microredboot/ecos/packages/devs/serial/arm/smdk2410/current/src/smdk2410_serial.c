//==========================================================================
//
//      io/serial/arm/smdk2410_serial.c
//
//      Samsung SMDK2410 Serial I/O Interface Module (interrupt driven)
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
// Author(s):    michael anburaj <michaelanburaj@hotmail.com>
// Contributors: michael anburaj <michaelanburaj@hotmail.com>
// Date:         2003-08-01
// Purpose:      SMDK2410 Serial I/O module (interrupt driven version)
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/io_serial.h>
#include <pkgconf/io.h>
#include <cyg/io/io.h>
#include <cyg/hal/hal_intr.h>           // interrupt
#include <cyg/hal/hal_io.h>             // register base
#include <cyg/io/devtab.h>
#include <cyg/io/serial.h>
#include <cyg/infra/diag.h>

#ifdef CYGPKG_IO_SERIAL_ARM_SMDK2410

#include "smdk2410_serial.h"

typedef struct smdk2410_serial_info {
    CYG_ADDRWORD   base;
    CYG_WORD       int_num;
    cyg_uint32     bit_sub_rxd;
    cyg_interrupt  serial_interrupt;
    cyg_handle_t   serial_interrupt_handle;
} smdk2410_serial_info;

static bool smdk2410_serial_init(struct cyg_devtab_entry *tab);
static bool smdk2410_serial_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo smdk2410_serial_lookup(struct cyg_devtab_entry **tab, 
                                        struct cyg_devtab_entry *sub_tab,
                                        const char *name);
static unsigned char smdk2410_serial_getc(serial_channel *chan);
static Cyg_ErrNo smdk2410_serial_set_config(serial_channel *chan, 
                                            cyg_uint32 key,
                                            const void *xbuf, cyg_uint32 *len);
static void smdk2410_serial_start_xmit(serial_channel *chan);
static void smdk2410_serial_stop_xmit(serial_channel *chan);

static cyg_uint32 smdk2410_serial_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       smdk2410_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);

static SERIAL_FUNS(smdk2410_serial_funs, 
                   smdk2410_serial_putc, 
                   smdk2410_serial_getc,
                   smdk2410_serial_set_config,
                   smdk2410_serial_start_xmit,
                   smdk2410_serial_stop_xmit
    );

#ifdef CYGPKG_IO_SERIAL_ARM_SMDK2410_SERIAL0
static smdk2410_serial_info smdk2410_serial_info0 = {(cyg_uint32)ULCON0,
                                                     CYGNUM_HAL_INTERRUPT_UART0,
                                                     BIT_SUB_RXD0};
#if CYGNUM_IO_SERIAL_ARM_SMDK2410_SERIAL0_BUFSIZE > 0
static unsigned char smdk2410_serial_out_buf0[CYGNUM_IO_SERIAL_ARM_SMDK2410_SERIAL0_BUFSIZE];
static unsigned char smdk2410_serial_in_buf0[CYGNUM_IO_SERIAL_ARM_SMDK2410_SERIAL0_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(smdk2410_serial_channel0,
                                       smdk2410_serial_funs, 
                                       smdk2410_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_SMDK2410_SERIAL0_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &smdk2410_serial_out_buf0[0], sizeof(smdk2410_serial_out_buf0),
                                       &smdk2410_serial_in_buf0[0], sizeof(smdk2410_serial_in_buf0)
    );
#else
static SERIAL_CHANNEL(smdk2410_serial_channel0,
                      smdk2410_serial_funs, 
                      smdk2410_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_SMDK2410_SERIAL0_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(smdk2410_serial_io0, 
             CYGDAT_IO_SERIAL_ARM_SMDK2410_SERIAL0_NAME,
             0,                          // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             smdk2410_serial_init, 
             smdk2410_serial_lookup,     // Serial driver may need initializing
             &smdk2410_serial_channel0
    );
#endif //  CYGPKG_IO_SERIAL_ARM_SMDK2410_SERIAL0

#ifdef CYGPKG_IO_SERIAL_ARM_SMDK2410_SERIAL1
static smdk2410_serial_info smdk2410_serial_info1 = {(cyg_uint32)ULCON1,
                                                     CYGNUM_HAL_INTERRUPT_UART1,
                                                     BIT_SUB_RXD1};
#if CYGNUM_IO_SERIAL_ARM_SMDK2410_SERIAL1_BUFSIZE > 0
static unsigned char smdk2410_serial_out_buf1[CYGNUM_IO_SERIAL_ARM_SMDK2410_SERIAL1_BUFSIZE];
static unsigned char smdk2410_serial_in_buf1[CYGNUM_IO_SERIAL_ARM_SMDK2410_SERIAL1_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(smdk2410_serial_channel1,
                                       smdk2410_serial_funs, 
                                       smdk2410_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_SMDK2410_SERIAL1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &smdk2410_serial_out_buf1[0], sizeof(smdk2410_serial_out_buf1),
                                       &smdk2410_serial_in_buf1[0], sizeof(smdk2410_serial_in_buf1)
    );
#else
static SERIAL_CHANNEL(smdk2410_serial_channel1,
                      smdk2410_serial_funs, 
                      smdk2410_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_SMDK2410_SERIAL1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(smdk2410_serial_io1, 
             CYGDAT_IO_SERIAL_ARM_SMDK2410_SERIAL1_NAME,
             0,                          // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             smdk2410_serial_init, 
             smdk2410_serial_lookup,     // Serial driver may need initializing
             &smdk2410_serial_channel1
    );
#endif //  CYGPKG_IO_SERIAL_ARM_SMDK2410_SERIAL1

// Internal function to actually configure the hardware to desired baud rate, etc.
static bool
smdk2410_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
    smdk2410_serial_info *smdk2410_chan = (smdk2410_serial_info *)chan->dev_priv;
    CYG_ADDRWORD base = smdk2410_chan->base;
    unsigned short baud_divisor = select_baud[new_config->baud];
    cyg_uint32 _lcr;

    if (baud_divisor == 0) return false;

    if (init) {
        //UART FIFO control register
        HAL_WRITE_UINT32(base+OFS_UFCON, (3<<6) | (3<<4) | (1<<2) | (1<<1) | (1<<0));

        //UART modem control register
        HAL_WRITE_UINT32(base+OFS_UMCON, 0);
    }

    _lcr = select_word_length[new_config->word_length - CYGNUM_SERIAL_WORD_LENGTH_5] | 
        select_stop_bits[new_config->stop] |
        select_parity[new_config->parity];
    HAL_WRITE_UINT32(base+OFS_ULCON, _lcr);

    //UART control register, Enable Rx Timeout Int
    HAL_WRITE_UINT32(base+OFS_UCON, 0x085);

    if (new_config != &chan->config) {
        chan->config = *new_config;
    }
    return true;
}

// Function to initialize the device.  Called at bootstrap time.
static bool 
smdk2410_serial_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    smdk2410_serial_info *smdk2410_chan = (smdk2410_serial_info *)chan->dev_priv;
    cyg_uint32 _intsubm;
#ifdef CYGDBG_IO_INIT
    diag_printf("SMDK2410 SERIAL init - dev: 0x%08x.%d\n", 
                smdk2410_chan->base, smdk2410_chan->int_num);
#endif
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    if (chan->out_cbuf.len != 0) {
        cyg_drv_interrupt_create(smdk2410_chan->int_num,
                                 1,                    // Priority - unused
                                 (cyg_addrword_t)chan, //  Data item passed to interrupt handler
                                 smdk2410_serial_ISR,
                                 smdk2410_serial_DSR,
                                 &smdk2410_chan->serial_interrupt_handle,
                                 &smdk2410_chan->serial_interrupt);
        cyg_drv_interrupt_attach(smdk2410_chan->serial_interrupt_handle);
        cyg_drv_interrupt_unmask(smdk2410_chan->int_num);

        HAL_READ_UINT32(INTSUBMSK, _intsubm);
        _intsubm &= ~(smdk2410_chan->bit_sub_rxd<<0);         // BIT_SUB_RXD
        HAL_WRITE_UINT32(INTSUBMSK, _intsubm);
    }
    smdk2410_serial_config_port(chan, &chan->config, true);
    return true;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
smdk2410_serial_lookup(struct cyg_devtab_entry **tab, 
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
smdk2410_serial_putc(serial_channel *chan, unsigned char c)
{
    smdk2410_serial_info *smdk2410_chan = (smdk2410_serial_info *)chan->dev_priv;
    CYG_ADDRWORD base = smdk2410_chan->base;
    cyg_uint32 _status;

    HAL_READ_UINT32(base+OFS_UFSTAT, _status);
    if (_status & 0x200) {
        // No space
        return false;
    } else {
        // Transmit buffer is not full
        HAL_WRITE_UINT8(base+OFS_UTXH, (cyg_uint32)c);
        return true;
    }
}

// Fetch a character from the device input buffer, waiting if necessary
static unsigned char 
smdk2410_serial_getc(serial_channel *chan)
{
    smdk2410_serial_info *smdk2410_chan = (smdk2410_serial_info *)chan->dev_priv;
    CYG_ADDRWORD base = smdk2410_chan->base;
    cyg_uint32 _status;
    cyg_uint8 _c;

    do {
        HAL_READ_UINT32(base+OFS_UFSTAT, _status);
    } while ((_status & 0xf) == 0);

    HAL_READ_UINT8(base+OFS_URXH, _c);
    return (unsigned char)_c;
}

// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
smdk2410_serial_set_config(serial_channel *chan, cyg_uint32 key,
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
        if ( true != smdk2410_serial_config_port(chan, config, false) )
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
smdk2410_serial_start_xmit(serial_channel *chan)
{
    smdk2410_serial_info *smdk2410_chan = (smdk2410_serial_info *)chan->dev_priv;
    cyg_uint32 _intsubm;

    HAL_READ_UINT32(INTSUBMSK, _intsubm);
    _intsubm &= ~(smdk2410_chan->bit_sub_rxd<<1);         // BIT_SUB_TXD
    HAL_WRITE_UINT32(INTSUBMSK, _intsubm);
}

// Disable the transmitter on the device
static void 
smdk2410_serial_stop_xmit(serial_channel *chan)
{
    smdk2410_serial_info *smdk2410_chan = (smdk2410_serial_info *)chan->dev_priv;
    cyg_uint32 _intsubm;

    HAL_READ_UINT32(INTSUBMSK, _intsubm);
    _intsubm |= (smdk2410_chan->bit_sub_rxd<<1);          // BIT_SUB_TXD
    HAL_WRITE_UINT32(INTSUBMSK, _intsubm);
}

// Serial I/O - low level interrupt handler (ISR)
static cyg_uint32 
smdk2410_serial_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    smdk2410_serial_info *smdk2410_chan = (smdk2410_serial_info *)chan->dev_priv;
    cyg_drv_interrupt_mask(smdk2410_chan->int_num);
    cyg_drv_interrupt_acknowledge(smdk2410_chan->int_num);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level interrupt handler (DSR)
static void       
smdk2410_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    smdk2410_serial_info *smdk2410_chan = (smdk2410_serial_info *)chan->dev_priv;
    CYG_ADDRWORD base = smdk2410_chan->base;
    cyg_uint32 _intsubpnd, _status, _c;
    cyg_uint32 _rxd_bit = (smdk2410_chan->bit_sub_rxd<<0), _txd_bit=(smdk2410_chan->bit_sub_rxd<<1);

    HAL_READ_UINT32(SUBSRCPND, _intsubpnd);

    // Empty Rx FIFO
    if (_intsubpnd & _rxd_bit) {
        HAL_READ_UINT32(base+OFS_UFSTAT, _status);
        while((_status & 0x0f) != 0) {
            HAL_READ_UINT8(base+OFS_URXH, _c);
            (chan->callbacks->rcv_char)(chan, (unsigned char)_c);
            HAL_READ_UINT32(base+OFS_UFSTAT, _status);
        }
        HAL_WRITE_UINT32(SUBSRCPND, _rxd_bit);
    }

    // Fill into Tx FIFO. xmt_char will mask the interrupt when it
    // runs out of chars, so doing this in a loop is OK.
    if (_intsubpnd & _txd_bit) {
        (chan->callbacks->xmt_char)(chan);
        HAL_WRITE_UINT32(SUBSRCPND, _txd_bit);
    }

    cyg_drv_interrupt_unmask(smdk2410_chan->int_num);
}

#endif // CYGPKG_IO_SERIAL_ARM_SMDK2410

// EOF smdk2410_serial.c
