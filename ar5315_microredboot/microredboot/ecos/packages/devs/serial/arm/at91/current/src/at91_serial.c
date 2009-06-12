//==========================================================================
//
//      devs/serial/arm/at91/at91_serial.c
//
//      Atmel AT91/EB40 Serial I/O Interface Module (interrupt driven)
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
// Author(s):     gthomas
// Contributors:  gthomas, tkoeller
// Date:          2001-07-24
// Purpose:       Atmel AT91/EB40 Serial I/O module (interrupt driven version)
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/infra.h>
#include <pkgconf/system.h>
#include <pkgconf/io_serial.h>
#include <pkgconf/io.h>
#include <pkgconf/kernel.h>

#include <cyg/io/io.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/io/devtab.h>
#include <cyg/io/serial.h>
#include <cyg/infra/diag.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>

externC void * memcpy( void *, const void *, size_t );

#ifdef CYGPKG_IO_SERIAL_ARM_AT91

#include "at91_serial.h"

#define RCVBUF_EXTRA 16
#define RCV_TIMEOUT 10

#define SIFLG_NONE          0x00
#define SIFLG_TX_READY      0x01
#define SIFLG_XMIT_BUSY     0x02
#define SIFLG_XMIT_CONTINUE 0x04

typedef struct at91_serial_info {
    CYG_ADDRWORD   base;
    CYG_WORD       int_num;
    CYG_WORD       stat;
    int            transmit_size;
    cyg_interrupt  serial_interrupt;
    cyg_handle_t   serial_interrupt_handle;
    cyg_uint8      *rcv_buffer[2];
    cyg_uint16     rcv_chunk_size;
    cyg_uint8      curbuf;
    cyg_uint8      flags;
} at91_serial_info;

static bool at91_serial_init(struct cyg_devtab_entry *tab);
static bool at91_serial_putc_interrupt(serial_channel *chan, unsigned char c);
#if (defined(CYGPKG_IO_SERIAL_ARM_AT91_SERIAL0) && CYGNUM_IO_SERIAL_ARM_AT91_SERIAL0_BUFSIZE == 0) \
 || (defined(CYGPKG_IO_SERIAL_ARM_AT91_SERIAL1) && CYGNUM_IO_SERIAL_ARM_AT91_SERIAL1_BUFSIZE == 0)
static bool at91_serial_putc_polled(serial_channel *chan, unsigned char c);
#endif
static Cyg_ErrNo at91_serial_lookup(struct cyg_devtab_entry **tab, 
                                    struct cyg_devtab_entry *sub_tab,
                                    const char *name);
static unsigned char at91_serial_getc_interrupt(serial_channel *chan);
#if (defined(CYGPKG_IO_SERIAL_ARM_AT91_SERIAL0) && CYGNUM_IO_SERIAL_ARM_AT91_SERIAL0_BUFSIZE == 0) \
 || (defined(CYGPKG_IO_SERIAL_ARM_AT91_SERIAL1) && CYGNUM_IO_SERIAL_ARM_AT91_SERIAL1_BUFSIZE == 0)
static unsigned char at91_serial_getc_polled(serial_channel *chan);
#endif
static Cyg_ErrNo at91_serial_set_config(serial_channel *chan, cyg_uint32 key,
                                        const void *xbuf, cyg_uint32 *len);
static void at91_serial_start_xmit(serial_channel *chan);
static void at91_serial_stop_xmit(serial_channel *chan);

static cyg_uint32 at91_serial_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       at91_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);

#if (defined(CYGPKG_IO_SERIAL_ARM_AT91_SERIAL0) && CYGNUM_IO_SERIAL_ARM_AT91_SERIAL0_BUFSIZE > 0) \
 || (defined(CYGPKG_IO_SERIAL_ARM_AT91_SERIAL1) && CYGNUM_IO_SERIAL_ARM_AT91_SERIAL1_BUFSIZE > 0)
static SERIAL_FUNS(at91_serial_funs_interrupt, 
                   at91_serial_putc_interrupt, 
                   at91_serial_getc_interrupt,
                   at91_serial_set_config,
                   at91_serial_start_xmit,
                   at91_serial_stop_xmit
    );
#endif

#if (defined(CYGPKG_IO_SERIAL_ARM_AT91_SERIAL0) && CYGNUM_IO_SERIAL_ARM_AT91_SERIAL0_BUFSIZE == 0) \
 || (defined(CYGPKG_IO_SERIAL_ARM_AT91_SERIAL1) && CYGNUM_IO_SERIAL_ARM_AT91_SERIAL1_BUFSIZE == 0)
static SERIAL_FUNS(at91_serial_funs_polled, 
                   at91_serial_putc_polled, 
                   at91_serial_getc_polled,
                   at91_serial_set_config,
                   at91_serial_start_xmit,
                   at91_serial_stop_xmit
    );
#endif

#ifdef CYGPKG_IO_SERIAL_ARM_AT91_SERIAL0
static cyg_uint8 at91_serial_rcv_buffer_0
    [2][CYGNUM_IO_SERIAL_ARM_AT91_SERIAL0_RCV_CHUNK_SIZE + RCVBUF_EXTRA];
static at91_serial_info at91_serial_info0 = {
    base            : (CYG_ADDRWORD) AT91_USART0,
    int_num         : CYGNUM_HAL_INTERRUPT_USART0,
    rcv_chunk_size  : CYGNUM_IO_SERIAL_ARM_AT91_SERIAL0_RCV_CHUNK_SIZE,
    rcv_buffer      : {at91_serial_rcv_buffer_0[0], at91_serial_rcv_buffer_0[1]}
};

#if CYGNUM_IO_SERIAL_ARM_AT91_SERIAL0_BUFSIZE > 0
static unsigned char at91_serial_out_buf0[CYGNUM_IO_SERIAL_ARM_AT91_SERIAL0_BUFSIZE];
static unsigned char at91_serial_in_buf0[CYGNUM_IO_SERIAL_ARM_AT91_SERIAL0_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(at91_serial_channel0,
                                       at91_serial_funs_interrupt, 
                                       at91_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_AT91_SERIAL0_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &at91_serial_out_buf0[0], sizeof(at91_serial_out_buf0),
                                       &at91_serial_in_buf0[0], sizeof(at91_serial_in_buf0)
    );
#else
static SERIAL_CHANNEL(at91_serial_channel0,
                      at91_serial_funs_polled, 
                      at91_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_AT91_SERIAL0_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(at91_serial_io0, 
             CYGDAT_IO_SERIAL_ARM_AT91_SERIAL0_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             at91_serial_init, 
             at91_serial_lookup,     // Serial driver may need initializing
             &at91_serial_channel0
    );
#endif //  CYGPKG_IO_SERIAL_ARM_AT91_SERIAL1

#ifdef CYGPKG_IO_SERIAL_ARM_AT91_SERIAL1
static cyg_uint8 at91_serial_rcv_buffer_1
    [2][CYGNUM_IO_SERIAL_ARM_AT91_SERIAL1_RCV_CHUNK_SIZE + RCVBUF_EXTRA];
static at91_serial_info at91_serial_info1 = {
    base            : (CYG_ADDRWORD) AT91_USART1,
    int_num         : CYGNUM_HAL_INTERRUPT_USART1,
    rcv_chunk_size  : CYGNUM_IO_SERIAL_ARM_AT91_SERIAL1_RCV_CHUNK_SIZE,
    rcv_buffer      : {at91_serial_rcv_buffer_1[0], at91_serial_rcv_buffer_1[1]}
};

#if CYGNUM_IO_SERIAL_ARM_AT91_SERIAL1_BUFSIZE > 0
static unsigned char at91_serial_out_buf1[CYGNUM_IO_SERIAL_ARM_AT91_SERIAL1_BUFSIZE];
static unsigned char at91_serial_in_buf1[CYGNUM_IO_SERIAL_ARM_AT91_SERIAL1_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(at91_serial_channel1,
                                       at91_serial_funs_interrupt, 
                                       at91_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_AT91_SERIAL1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &at91_serial_out_buf1[0], sizeof(at91_serial_out_buf1),
                                       &at91_serial_in_buf1[0], sizeof(at91_serial_in_buf1)
    );
#else
static SERIAL_CHANNEL(at91_serial_channel1,
                      at91_serial_funs_polled, 
                      at91_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_AT91_SERIAL1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(at91_serial_io1, 
             CYGDAT_IO_SERIAL_ARM_AT91_SERIAL1_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             at91_serial_init, 
             at91_serial_lookup,     // Serial driver may need initializing
             &at91_serial_channel1
    );
#endif //  CYGPKG_IO_SERIAL_ARM_AT91_SERIAL1

// Internal function to actually configure the hardware to desired baud rate, etc.
static bool
at91_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
    at91_serial_info * const at91_chan = (at91_serial_info *)chan->dev_priv;
    const CYG_ADDRWORD base = at91_chan->base;
    cyg_uint32 parity = select_parity[new_config->parity];
    cyg_uint32 word_length = select_word_length[new_config->word_length-CYGNUM_SERIAL_WORD_LENGTH_5];
    cyg_uint32 stop_bits = select_stop_bits[new_config->stop];

    if ((word_length == 0xFF) ||
        (parity == 0xFF) ||
        (stop_bits == 0xFF)) {
        return false;  // Unsupported configuration
    }

    // Reset device
    HAL_WRITE_UINT32(base + AT91_US_CR, AT91_US_CR_RxRESET | AT91_US_CR_TxRESET);

    // Configuration
    HAL_WRITE_UINT32(base + AT91_US_MR, parity | word_length | stop_bits);

    // Baud rate
    HAL_WRITE_UINT32(base + AT91_US_BRG, AT91_US_BAUD(select_baud[new_config->baud]));

    // Disable all interrupts
    HAL_WRITE_UINT32(base + AT91_US_IDR, 0xFFFFFFFF);

    // Start receiver
    at91_chan->curbuf = 0;
    HAL_WRITE_UINT32(base + AT91_US_RPR, (CYG_ADDRESS) at91_chan->rcv_buffer[0]);
    HAL_WRITE_UINT32(base + AT91_US_RTO, RCV_TIMEOUT);
    HAL_WRITE_UINT32(base + AT91_US_IER, AT91_US_IER_ENDRX | AT91_US_IER_TIMEOUT);
    HAL_WRITE_UINT32(base + AT91_US_RCR, at91_chan->rcv_chunk_size);

    // Enable RX and TX
    HAL_WRITE_UINT32(
      base + AT91_US_CR,
      AT91_US_CR_RxENAB | AT91_US_CR_TxENAB | AT91_US_CR_RSTATUS | AT91_US_CR_STTTO
    );

    if (new_config != &chan->config) {
        chan->config = *new_config;
    }

    return true;
}

// Function to initialize the device.  Called at bootstrap time.
static bool 
at91_serial_init(struct cyg_devtab_entry *tab)
{
    serial_channel * const chan = (serial_channel *) tab->priv;
    at91_serial_info * const at91_chan = (at91_serial_info *) chan->dev_priv;
    int res;

#ifdef CYGDBG_IO_INIT
    diag_printf("AT91 SERIAL init - dev: %x.%d\n", at91_chan->base, at91_chan->int_num);
#endif
    at91_chan->curbuf = 0;
    at91_chan->flags = SIFLG_NONE;
    at91_chan->stat = 0;
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    if (chan->out_cbuf.len != 0) {
        cyg_drv_interrupt_create(at91_chan->int_num,
                                 4,                      // Priority
                                 (cyg_addrword_t)chan,   // Data item passed to interrupt handler
                                 at91_serial_ISR,
                                 at91_serial_DSR,
                                 &at91_chan->serial_interrupt_handle,
                                 &at91_chan->serial_interrupt);
        cyg_drv_interrupt_attach(at91_chan->serial_interrupt_handle);
        cyg_drv_interrupt_unmask(at91_chan->int_num);
    }
    res = at91_serial_config_port(chan, &chan->config, true);
    return res;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
at91_serial_lookup(struct cyg_devtab_entry **tab, 
                  struct cyg_devtab_entry *sub_tab,
                  const char *name)
{
    serial_channel * const chan = (serial_channel *) (*tab)->priv;

    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    return ENOERR;
}

// Send a character to the device output buffer.
// Return 'true' if character is sent to device
static bool
at91_serial_putc_interrupt(serial_channel *chan, unsigned char c)
{
    at91_serial_info * const at91_chan = (at91_serial_info *) chan->dev_priv;
    const bool res = (at91_chan->flags & SIFLG_XMIT_BUSY) == 0;
    
    if (res) {
        const CYG_ADDRWORD base = at91_chan->base;
        HAL_WRITE_UINT32(base + AT91_US_THR, c);
        at91_chan->flags |= SIFLG_XMIT_BUSY;
    }
    return res;
}

#if (defined(CYGPKG_IO_SERIAL_ARM_AT91_SERIAL0) && CYGNUM_IO_SERIAL_ARM_AT91_SERIAL0_BUFSIZE == 0) \
 || (defined(CYGPKG_IO_SERIAL_ARM_AT91_SERIAL1) && CYGNUM_IO_SERIAL_ARM_AT91_SERIAL1_BUFSIZE == 0)
static bool
at91_serial_putc_polled(serial_channel *chan, unsigned char c)
{
    at91_serial_info * const at91_chan = (at91_serial_info *) chan->dev_priv;
    const CYG_ADDRWORD base = at91_chan->base;
    CYG_WORD32 w;
    
    while (HAL_READ_UINT32(base + AT91_US_CSR, w), (w & AT91_US_IER_TxRDY) == 0)
        CYG_EMPTY_STATEMENT;
    HAL_WRITE_UINT32(base + AT91_US_THR, c);
    return true;
}
#endif

// Fetch a character from the device input buffer, waiting if necessary
static unsigned char 
at91_serial_getc_interrupt(serial_channel *chan)
{
    at91_serial_info * const at91_chan = (at91_serial_info *) chan->dev_priv;
    const CYG_ADDRWORD base = at91_chan->base;
    CYG_WORD32 c;

    // Read data
    HAL_READ_UINT32(base + AT91_US_RHR, c);
    return (unsigned char) c;
}

#if (defined(CYGPKG_IO_SERIAL_ARM_AT91_SERIAL0) && CYGNUM_IO_SERIAL_ARM_AT91_SERIAL0_BUFSIZE == 0) \
 || (defined(CYGPKG_IO_SERIAL_ARM_AT91_SERIAL1) && CYGNUM_IO_SERIAL_ARM_AT91_SERIAL1_BUFSIZE == 0)
static unsigned char 
at91_serial_getc_polled(serial_channel *chan)
{
    at91_serial_info * const at91_chan = (at91_serial_info *) chan->dev_priv;
    const CYG_ADDRWORD base = at91_chan->base;
    CYG_WORD32 c;

    while (HAL_READ_UINT32(base + AT91_US_CSR, c), (c & AT91_US_IER_RxRDY) == 0)
        CYG_EMPTY_STATEMENT;
    // Read data
    HAL_READ_UINT32(base + AT91_US_RHR, c);
    return (unsigned char) c;
}
#endif
// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
at91_serial_set_config(serial_channel *chan, cyg_uint32 key,
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
        if ( true != at91_serial_config_port(chan, config, false) )
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
at91_serial_start_xmit(serial_channel *chan)
{
    at91_serial_info * const at91_chan = (at91_serial_info *) chan->dev_priv;
    const CYG_ADDRWORD base = at91_chan->base;
    unsigned char * chars;
    xmt_req_reply_t res;
    
    cyg_drv_dsr_lock();
    if ((at91_chan->flags & SIFLG_XMIT_CONTINUE) == 0) {
        res = (chan->callbacks->data_xmt_req)(chan, 0xffff, &at91_chan->transmit_size, &chars);
        switch (res)
        {
            case CYG_XMT_OK:
                HAL_WRITE_UINT32(base + AT91_US_TPR, (CYG_WORD32) chars);
                HAL_WRITE_UINT32(base + AT91_US_TCR, at91_chan->transmit_size);
                at91_chan->flags |= SIFLG_XMIT_CONTINUE;
                HAL_WRITE_UINT32(base + AT91_US_IER, AT91_US_IER_ENDTX);
                break;
            case CYG_XMT_DISABLED:
                (chan->callbacks->xmt_char)(chan);  // Kick transmitter
                at91_chan->flags |= SIFLG_XMIT_CONTINUE;
                HAL_WRITE_UINT32(base + AT91_US_IER, AT91_US_IER_TxRDY);
                break;
            default:
                // No data or unknown error - can't do anything about it
                break;
        }
    }
    cyg_drv_dsr_unlock();
}

// Disable the transmitter on the device
static void 
at91_serial_stop_xmit(serial_channel *chan)
{
    at91_serial_info * const at91_chan = (at91_serial_info *) chan->dev_priv;
    const CYG_ADDRWORD base = at91_chan->base;
    HAL_WRITE_UINT32(base + AT91_US_IDR, AT91_US_IER_TxRDY | AT91_US_IER_ENDTX);
    at91_chan->flags &= ~SIFLG_XMIT_CONTINUE;
}

// Serial I/O - low level interrupt handler (ISR)
static cyg_uint32 
at91_serial_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel * const chan = (serial_channel *) data;
    at91_serial_info * const at91_chan = (at91_serial_info *) chan->dev_priv;
    const CYG_ADDRWORD base = at91_chan->base;
    CYG_WORD32 stat, mask;

    HAL_READ_UINT32(base + AT91_US_CSR, stat);
    HAL_READ_UINT32(base + AT91_US_IMR, mask);
    stat &= mask;

    if (stat & (AT91_US_IER_ENDRX | AT91_US_IER_TIMEOUT)) {
        cyg_uint32 x;
        HAL_WRITE_UINT32(base + AT91_US_IDR, AT91_US_IER_ENDRX | AT91_US_IER_TIMEOUT);
        HAL_WRITE_UINT32(base + AT91_US_RCR, 0);
        HAL_WRITE_UINT32(base + AT91_US_RTO, 0);
        HAL_READ_UINT32(base + AT91_US_RPR, x);
        HAL_WRITE_UINT32(
            base + AT91_US_RCR,
            (CYG_ADDRESS) at91_chan->rcv_buffer[at91_chan->curbuf]
                + at91_chan->rcv_chunk_size + RCVBUF_EXTRA - x
        );
    }

    if (stat & (AT91_US_IER_TxRDY | AT91_US_IER_ENDTX))
      HAL_WRITE_UINT32(base + AT91_US_IDR, AT91_US_IER_TxRDY | AT91_US_IER_ENDTX);

    at91_chan->stat |= stat;
    cyg_drv_interrupt_acknowledge(vector);
    return CYG_ISR_CALL_DSR;
}

// Serial I/O - high level interrupt handler (DSR)
static void       
at91_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel * const chan = (serial_channel *) data;
    at91_serial_info * const at91_chan = (at91_serial_info *) chan->dev_priv;
    const CYG_ADDRWORD base = at91_chan->base;
    CYG_WORD32 stat;

    cyg_drv_interrupt_mask(vector);    
    stat = at91_chan->stat;
    at91_chan->stat = 0;
    cyg_drv_interrupt_unmask(vector);    

    if (stat & (AT91_US_IER_ENDRX | AT91_US_IER_TIMEOUT)) {
        const cyg_uint8 cb = at91_chan->curbuf, nb = cb ^ 0x01;
        const cyg_uint8 * p = at91_chan->rcv_buffer[cb], * end;

        at91_chan->curbuf = nb;
        HAL_WRITE_UINT32(base + AT91_US_RCR, 0);
        HAL_READ_UINT32(base + AT91_US_RPR, (CYG_ADDRESS) end);
        HAL_WRITE_UINT32(base + AT91_US_RTO, RCV_TIMEOUT);
	HAL_WRITE_UINT32(base + AT91_US_CR, AT91_US_CR_RSTATUS | AT91_US_CR_STTTO);
        HAL_WRITE_UINT32(base + AT91_US_RPR, (CYG_ADDRESS) at91_chan->rcv_buffer[nb]);
        HAL_WRITE_UINT32(base + AT91_US_RCR, at91_chan->rcv_chunk_size);
        HAL_WRITE_UINT32(
            base + AT91_US_IER,
            AT91_US_IER_ENDRX | AT91_US_IER_TIMEOUT
        );

        while (p < end) {
            rcv_req_reply_t res;
            int space_avail;
            unsigned char *space;

            res = (chan->callbacks->data_rcv_req)(
              chan,
              end - at91_chan->rcv_buffer[cb],
              &space_avail,
              &space
            );

            switch (res)
            {
                case CYG_RCV_OK:
                    memcpy(space, p, space_avail);
                    (chan->callbacks->data_rcv_done)(chan, space_avail);
                    p += space_avail;
                    break;
                case CYG_RCV_DISABLED:
                    (chan->callbacks->rcv_char)(chan, *p++);
                    break;
                default:
                    // Buffer full or unknown error, can't do anything about it
                    // Discard data
                    CYG_FAIL("Serial receiver buffer overflow");
                    p = end;
                    break;
            }
        }
    }

    if (stat & AT91_US_IER_TxRDY) {
        at91_chan->flags &= ~SIFLG_XMIT_BUSY;
        (chan->callbacks->xmt_char)(chan);
        if (at91_chan->flags & SIFLG_XMIT_CONTINUE)
            HAL_WRITE_UINT32(base + AT91_US_IER, AT91_US_IER_TxRDY);
    }
    
    if (stat & AT91_US_IER_ENDTX) {
        (chan->callbacks->data_xmt_done)(chan, at91_chan->transmit_size);
        if (at91_chan->flags & SIFLG_XMIT_CONTINUE) {
            unsigned char * chars;
            xmt_req_reply_t res;

            res = (chan->callbacks->data_xmt_req)(chan, 0xffff, &at91_chan->transmit_size, &chars);

            switch (res)
            {
                case CYG_XMT_OK:
                    HAL_WRITE_UINT32(base + AT91_US_TPR, (CYG_WORD32) chars);
                    HAL_WRITE_UINT32(base + AT91_US_TCR, at91_chan->transmit_size);
                    at91_chan->flags |= SIFLG_XMIT_CONTINUE;
                    HAL_WRITE_UINT32(base + AT91_US_IER, AT91_US_IER_ENDTX);
                    break;
                default:
                    // No data or unknown error - can't do anything about it
                    // CYG_XMT_DISABLED should not happen here!
                    break;
            }
        }
    }
}
#endif
