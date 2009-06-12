//==========================================================================
//
//      io/serial/sh/scif/sh_scif_serial.c
//
//      SH Serial IRDA/SCIF I/O Interface Module (interrupt driven)
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
// Author(s):   jskov
// Contributors:gthomas, jskov
// Date:        2000-04-04
// Purpose:     SH Serial IRDA/SCIF I/O module (interrupt driven version)
// Description: 
//
//####DESCRIPTIONEND####
//==========================================================================

#include <pkgconf/io_serial.h>
#include <pkgconf/io.h>

// FIXME: This is necessary since the SCI driver may be overriding
// CYGDAT_IO_SERIAL_DEVICE_HEADER. Need a better way to include two
// different drivers.
#include <pkgconf/io_serial_sh_scif.h>

#include <cyg/io/io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/io/devtab.h>
#include <cyg/infra/diag.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/io/serial.h>

#include <cyg/hal/sh_regs.h>
#include <cyg/hal/hal_cache.h>

// Only compile driver if an inline file with driver details was selected.
#ifdef CYGDAT_IO_SERIAL_SH_SCIF_INL

#if defined(CYGPKG_HAL_SH_SH2)
// The SCIF controller register layout on the SH2
// The controller base is defined in the board specification file.
# define SCIF_SCSMR      0x00      // serial mode register
# define SCIF_SCBRR      0x02      // bit rate register     
# define SCIF_SCSCR      0x04      // serial control register
# define SCIF_SCFTDR     0x06      // transmit data register
# define SCIF_SC1SSR     0x08      // serial status register 1
# define SCIF_SCSSR      SCIF_SC1SSR
# define SCIF_SC2SSR     0x0a      // serial status register 2
# define SCIF_SCFRDR     0x0c      // receive data register   
# define SCIF_SCFCR      0x0e      // FIFO control            
# define SCIF_SCFDR      0x10      // FIFO data count register
# define SCIF_SCFER      0x12      // FIFO error register
# define SCIF_SCIMR      0x14      // IrDA mode register
#elif defined(CYGPKG_HAL_SH_SH3)
// The SCIF controller register layout on the SH3
// The controller base is defined in the board specification file.
# define SCIF_SCSMR      0x00      // serial mode register
# define SCIF_SCBRR      0x02      // bit rate register
# define SCIF_SCSCR      0x04      // serial control register
# define SCIF_SCFTDR     0x06      // transmit data register
# define SCIF_SCSSR      0x08      // serial status register
# define SCIF_SCFRDR     0x0a      // receive data register
# define SCIF_SCFCR      0x0c      // FIFO control
# define SCIF_SCFDR      0x0e      // FIFO data count register
#else
# error "Unsupported variant"
#endif

static short select_word_length[] = {
    -1,
    -1,
    CYGARC_REG_SCIF_SCSMR_CHR,               // 7 bits
    0                                   // 8 bits
};

static short select_stop_bits[] = {
    -1,
    0,                                  // 1 stop bit
    -1,
    CYGARC_REG_SCIF_SCSMR_STOP               // 2 stop bits
};

static short select_parity[] = {
    0,                                  // No parity
    CYGARC_REG_SCIF_SCSMR_PE,                // Even parity
    CYGARC_REG_SCIF_SCSMR_PE|CYGARC_REG_SCIF_SCSMR_OE, // Odd parity
    -1,
    -1
};

static unsigned short select_baud[] = {
    0,    // Unused
    CYGARC_SCBRR_CKSx(50)<<8 | CYGARC_SCBRR_N(50),
    CYGARC_SCBRR_CKSx(75)<<8 | CYGARC_SCBRR_N(75),
    CYGARC_SCBRR_CKSx(110)<<8 | CYGARC_SCBRR_N(110),
    CYGARC_SCBRR_CKSx(134)<<8 | CYGARC_SCBRR_N(134),
    CYGARC_SCBRR_CKSx(150)<<8 | CYGARC_SCBRR_N(150),
    CYGARC_SCBRR_CKSx(200)<<8 | CYGARC_SCBRR_N(200),
    CYGARC_SCBRR_CKSx(300)<<8 | CYGARC_SCBRR_N(300),
    CYGARC_SCBRR_CKSx(600)<<8 | CYGARC_SCBRR_N(600),
    CYGARC_SCBRR_CKSx(1200)<<8 | CYGARC_SCBRR_N(1200),
    CYGARC_SCBRR_CKSx(1800)<<8 | CYGARC_SCBRR_N(1800),
    CYGARC_SCBRR_CKSx(2400)<<8 | CYGARC_SCBRR_N(2400),
    CYGARC_SCBRR_CKSx(3600)<<8 | CYGARC_SCBRR_N(3600),
    CYGARC_SCBRR_CKSx(4800)<<8 | CYGARC_SCBRR_N(4800),
    CYGARC_SCBRR_CKSx(7200)<<8 | CYGARC_SCBRR_N(7200),
    CYGARC_SCBRR_CKSx(9600)<<8 | CYGARC_SCBRR_N(9600),
    CYGARC_SCBRR_CKSx(14400)<<8 | CYGARC_SCBRR_N(14400),
    CYGARC_SCBRR_CKSx(19200)<<8 | CYGARC_SCBRR_N(19200),
    CYGARC_SCBRR_CKSx(38400)<<8 | CYGARC_SCBRR_N(38400),
    CYGARC_SCBRR_CKSx(57600)<<8 | CYGARC_SCBRR_N(57600),
    CYGARC_SCBRR_CKSx(115200)<<8 | CYGARC_SCBRR_N(115200),
    CYGARC_SCBRR_CKSx(230400)<<8 | CYGARC_SCBRR_N(230400)
};

typedef struct sh_scif_info {
    CYG_WORD          er_int_num,       // Error interrupt number
#ifdef CYGINT_IO_SERIAL_SH_SCIF_BR_INTERRUPT
                      br_int_num,       // Break interrupt number
#endif
                      rx_int_num,       // Receive interrupt number
                      tx_int_num;       // Transmit interrupt number

    CYG_ADDRWORD      ctrl_base;        // Base address of SCI controller

    cyg_interrupt     serial_er_interrupt,
#ifdef CYGINT_IO_SERIAL_SH_SCIF_BR_INTERRUPT
                      serial_br_interrupt,
#endif
                      serial_rx_interrupt,
                      serial_tx_interrupt;
    cyg_handle_t      serial_er_interrupt_handle, 
#ifdef CYGINT_IO_SERIAL_SH_SCIF_BR_INTERRUPT
                      serial_br_interrupt_handle, 
#endif
                      serial_rx_interrupt_handle, 
                      serial_tx_interrupt_handle;

    volatile bool     tx_enabled;       // expect tx _serial_ interrupts
#ifdef CYGINT_IO_SERIAL_SH_SCIF_IRDA
    bool              irda_mode;
#endif
#ifdef CYGINT_IO_SERIAL_SH_SCIF_ASYNC_RXTX
    bool              async_rxtx_mode;
#endif

#ifdef CYGINT_IO_SERIAL_SH_SCIF_DMA
    cyg_bool          dma_enable;       // Set if DMA mode
    cyg_uint32        dma_xmt_cr_flags; // CR flags for DMA mode
    CYG_WORD          dma_xmt_int_num;  // DMA xmt completion interrupt
    CYG_ADDRWORD      dma_xmt_base;     // Base address of DMA channel
    int               dma_xmt_len;      // length transferred by DMA
    cyg_interrupt     dma_xmt_interrupt;
    cyg_handle_t      dma_xmt_interrupt_handle;
    volatile cyg_bool dma_xmt_running;  // expect tx _dma_ interrupts
#endif
} sh_scif_info;

static bool sh_scif_init(struct cyg_devtab_entry *tab);
static bool sh_scif_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo sh_scif_lookup(struct cyg_devtab_entry **tab, 
                                   struct cyg_devtab_entry *sub_tab,
                                   const char *name);
static unsigned char sh_scif_getc(serial_channel *chan);
static Cyg_ErrNo sh_scif_set_config(serial_channel *chan, cyg_uint32 key,
                                     const void *xbuf, cyg_uint32 *len);
static void sh_scif_start_xmit(serial_channel *chan);
static void sh_scif_stop_xmit(serial_channel *chan);

static cyg_uint32 sh_scif_tx_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       sh_scif_tx_DSR(cyg_vector_t vector, cyg_ucount32 count, 
                                   cyg_addrword_t data);
static cyg_uint32 sh_scif_rx_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       sh_scif_rx_DSR(cyg_vector_t vector, cyg_ucount32 count, 
                                   cyg_addrword_t data);
static cyg_uint32 sh_scif_er_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       sh_scif_er_DSR(cyg_vector_t vector, cyg_ucount32 count, 
                                   cyg_addrword_t data);

#ifdef CYGINT_IO_SERIAL_SH_SCIF_DMA
static cyg_uint32 sh_dma_xmt_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       sh_dma_xmt_DSR(cyg_vector_t vector, cyg_ucount32 count, 
                                 cyg_addrword_t data);
#endif

static SERIAL_FUNS(sh_scif_funs, 
                   sh_scif_putc, 
                   sh_scif_getc,
                   sh_scif_set_config,
                   sh_scif_start_xmit,
                   sh_scif_stop_xmit
    );

// Get the board specification
#include CYGDAT_IO_SERIAL_SH_SCIF_INL

// Allow platform to define handling of additional config keys
#ifndef CYGPRI_DEVS_SH_SCIF_SET_CONFIG_PLF
# define CYGPRI_DEVS_SH_SCIF_SET_CONFIG_PLF
#endif

// Internal function to actually configure the hardware to desired baud rate,
// etc.
static bool
sh_scif_config_port(serial_channel *chan, cyg_serial_info_t *new_config, 
                     bool init)
{
    cyg_uint16 baud_divisor = select_baud[new_config->baud];
    sh_scif_info *sh_chan = (sh_scif_info *)chan->dev_priv;
    cyg_uint8 _scr, _smr;
    cyg_uint16 _sr;
    CYG_ADDRWORD base = sh_chan->ctrl_base;

    // Check configuration request
    if ((-1 == select_word_length[(new_config->word_length -
                                  CYGNUM_SERIAL_WORD_LENGTH_5)])
        || -1 == select_stop_bits[new_config->stop]
        || -1 == select_parity[new_config->parity]
        || baud_divisor == 0)
        return false;

    // Disable SCI interrupts while changing hardware
    HAL_READ_UINT8(base+SCIF_SCSCR, _scr);
    HAL_WRITE_UINT8(base+SCIF_SCSCR, 0);

    // Reset FIFO.
    HAL_WRITE_UINT8(base+SCIF_SCFCR, 
                    CYGARC_REG_SCIF_SCFCR_TFRST|CYGARC_REG_SCIF_SCFCR_RFRST);

#ifdef CYGINT_IO_SERIAL_SH_SCIF_ASYNC_RXTX
    sh_chan->async_rxtx_mode = false;
#endif
#ifdef CYGINT_IO_SERIAL_SH_SCIF_IRDA
    if (sh_chan->irda_mode) {
        // In IrDA mode, the configuration is hardwired and the mode
        // bits should not be set
#ifdef CYGARC_REG_SCIF_SCSMR_IRMOD
        _smr = CYGARC_REG_SCIF_SCSMR_IRMOD;
#elif defined(SCIF_SCIMR)
        _smr = 0;
        HAL_WRITE_UINT8(base+SCIF_SCIMR, CYGARC_REG_SCIF_SCIMR_IRMOD);
#endif
    } else
#endif
    {
        // Set databits, stopbits and parity.
        _smr = select_word_length[(new_config->word_length -
                                   CYGNUM_SERIAL_WORD_LENGTH_5)] | 
            select_stop_bits[new_config->stop] |
            select_parity[new_config->parity];
#ifdef CYGINT_IO_SERIAL_SH_SCIF_IRDA
#ifdef SCIF_SCIMR
        // Disable IrDA mode
        HAL_WRITE_UINT8(base+SCIF_SCIMR, 0);
#endif
#endif
    }
    HAL_WRITE_UINT8(base+SCIF_SCSMR, _smr);

    // Set baud rate.
    _smr &= ~CYGARC_REG_SCIF_SCSMR_CKSx_MASK;
    _smr |= baud_divisor >> 8;
    HAL_WRITE_UINT8(base+SCIF_SCSMR, _smr);
    HAL_WRITE_UINT8(base+SCIF_SCBRR, baud_divisor & 0xff);

    // FIXME: Should delay 1/<baud> second here.

    // Clear the status register (read first).
    HAL_READ_UINT16(base+SCIF_SCSSR, _sr);
    HAL_WRITE_UINT16(base+SCIF_SCSSR, 0);

    // Bring FIFO out of reset and set FIFO trigger marks
    //
    // Note that the RX FIFO size must be smaller when flow control is
    // enabled. This due to observations made by running the flow2
    // serial test. The automatic RTS de-assertion happens
    // (apparently) when the FIFO fills past the trigger count -
    // causing the sender to stop transmission. But there's a lag
    // before transmission is stopped, and if the FIFO fills in that
    // time, data will be lost. Thus, seeing as HW flow control is
    // presumed used for prevention of data loss, set the trigger
    // level so the sender has time to stop transmission before the
    // FIFO fills up.
    //
    // The trigger setting of 8 allows test flow2 to complete without
    // problems. It tests duplex data transmission at 115200
    // baud. Depending on the lag time between the de-assertion of RTS
    // and actual transmission stop, it may be necessary to reduce the
    // trigger level further.
#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_HW
    HAL_WRITE_UINT8(base+SCIF_SCFCR, 
                    CYGARC_REG_SCIF_SCFCR_RTRG_8|CYGARC_REG_SCIF_SCFCR_TTRG_8);
#else
    HAL_WRITE_UINT8(base+SCIF_SCFCR, 
                    CYGARC_REG_SCIF_SCFCR_RTRG_14|CYGARC_REG_SCIF_SCFCR_TTRG_8);
#endif

    if (init) {
        // Always enable received and (for normal mode) transmitter
        _scr = CYGARC_REG_SCIF_SCSCR_TE | CYGARC_REG_SCIF_SCSCR_RE;
#ifdef CYGINT_IO_SERIAL_SH_SCIF_ASYNC_RXTX
        if (sh_chan->async_rxtx_mode)
            _scr = CYGARC_REG_SCIF_SCSCR_RE;
#endif
#ifdef CYGINT_IO_SERIAL_SH_SCIF_IRDA
        if (sh_chan->irda_mode)
            _scr = CYGARC_REG_SCIF_SCSCR_RE;
#endif

        if (chan->in_cbuf.len != 0)
            _scr |= CYGARC_REG_SCIF_SCSCR_RIE; // enable rx interrupts
    }
     
    HAL_WRITE_UINT8(base+SCIF_SCSCR, _scr);

    if (new_config != &chan->config) {
        chan->config = *new_config;
    }
    return true;
}

// Function to initialize the device.  Called at bootstrap time.
static bool 
sh_scif_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    sh_scif_info *sh_chan = (sh_scif_info *)chan->dev_priv;
#ifdef CYGDBG_IO_INIT
    diag_printf("SH SERIAL init - dev: %x.%d\n", 
                sh_chan->ctrl_base, sh_chan->rx_int_num);
#endif
    // Really only required for interrupt driven devices
    (chan->callbacks->serial_init)(chan);

    if (chan->out_cbuf.len != 0) {
        cyg_drv_interrupt_create(sh_chan->tx_int_num,
                                 3,
                                 (cyg_addrword_t)chan, // Data item passed to interrupt handler
                                 sh_scif_tx_ISR,
                                 sh_scif_tx_DSR,
                                 &sh_chan->serial_tx_interrupt_handle,
                                 &sh_chan->serial_tx_interrupt);
        cyg_drv_interrupt_attach(sh_chan->serial_tx_interrupt_handle);
        cyg_drv_interrupt_unmask(sh_chan->tx_int_num);
        sh_chan->tx_enabled = false;
    }
    if (chan->in_cbuf.len != 0) {
        // Receive interrupt
        cyg_drv_interrupt_create(sh_chan->rx_int_num,
                                 3,
                                 (cyg_addrword_t)chan, // Data item passed to interrupt handler
                                 sh_scif_rx_ISR,
                                 sh_scif_rx_DSR,
                                 &sh_chan->serial_rx_interrupt_handle,
                                 &sh_chan->serial_rx_interrupt);
        cyg_drv_interrupt_attach(sh_chan->serial_rx_interrupt_handle);
        // Receive error interrupt
        cyg_drv_interrupt_create(sh_chan->er_int_num,
                                 3,
                                 (cyg_addrword_t)chan, // Data item passed to interrupt handler
                                 sh_scif_er_ISR,
                                 sh_scif_er_DSR,
                                 &sh_chan->serial_er_interrupt_handle,
                                 &sh_chan->serial_er_interrupt);
        cyg_drv_interrupt_attach(sh_chan->serial_er_interrupt_handle);
#ifdef CYGINT_IO_SERIAL_SH_SCIF_BR_INTERRUPT
        // Break error interrupt
        cyg_drv_interrupt_create(sh_chan->br_int_num,
                                 3,
                                 (cyg_addrword_t)chan, // Data item passed to interrupt handler
                                 sh_scif_er_ISR,
                                 sh_scif_er_DSR,
                                 &sh_chan->serial_br_interrupt_handle,
                                 &sh_chan->serial_br_interrupt);
        cyg_drv_interrupt_attach(sh_chan->serial_br_interrupt_handle);
#endif
        // This unmasks all interrupt sources.
        cyg_drv_interrupt_unmask(sh_chan->rx_int_num);
    }

#ifdef CYGINT_IO_SERIAL_SH_SCIF_DMA
    // Assign DMA channel and interrupt if requested
    if (sh_chan->dma_enable) {
        // FIXME: Need a cleaner way to assign DMA channels
        static int dma_channel = 0;
#if defined(CYGPKG_HAL_SH_SH2)
        sh_chan->dma_xmt_int_num = dma_channel+CYGNUM_HAL_INTERRUPT_DMAC0_TE;
#elif defined(CYGPKG_HAL_SH_SH3)
        sh_chan->dma_xmt_int_num = dma_channel+CYGNUM_HAL_INTERRUPT_DMAC_DEI0;
#else
# error "No interrupt defined for variant"
#endif
        sh_chan->dma_xmt_base = (dma_channel*0x10)+CYGARC_REG_SAR0;
        dma_channel++;

        // Enable the DMA engines.
        HAL_WRITE_UINT16(CYGARC_REG_DMAOR, CYGARC_REG_DMAOR_DME);

        cyg_drv_interrupt_create(sh_chan->dma_xmt_int_num,
                                 3,
                                 (cyg_addrword_t)chan, // Data item passed to interrupt handler
                                 sh_dma_xmt_ISR,
                                 sh_dma_xmt_DSR,
                                 &sh_chan->dma_xmt_interrupt_handle,
                                 &sh_chan->dma_xmt_interrupt);
        cyg_drv_interrupt_attach(sh_chan->dma_xmt_interrupt_handle);
        cyg_drv_interrupt_unmask(sh_chan->dma_xmt_int_num);
    }
    sh_chan->dma_xmt_running = false;
#endif // CYGINT_IO_SERIAL_SH_SCIF_DMA

    sh_scif_config_port(chan, &chan->config, true);
    return true;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
sh_scif_lookup(struct cyg_devtab_entry **tab,
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
sh_scif_putc(serial_channel *chan, unsigned char c)
{
    cyg_uint16 _fdr, _sr;
    sh_scif_info *sh_chan = (sh_scif_info *)chan->dev_priv;

    HAL_READ_UINT16(sh_chan->ctrl_base+SCIF_SCFDR, _fdr);
    if (((_fdr & CYGARC_REG_SCIF_SCFDR_TCOUNT_MASK) >> CYGARC_REG_SCIF_SCFDR_TCOUNT_shift) < 15) {
// Transmit FIFO has room for another char
        HAL_WRITE_UINT8(sh_chan->ctrl_base+SCIF_SCFTDR, c);
        // Clear FIFO-empty/transmit end flags (read back sr first)
        HAL_READ_UINT16(sh_chan->ctrl_base+SCIF_SCSSR, _sr);
        HAL_WRITE_UINT16(sh_chan->ctrl_base+SCIF_SCSSR,
                        CYGARC_REG_SCIF_SCSSR_CLEARMASK & ~CYGARC_REG_SCIF_SCSSR_TDFE);
        return true;
    } else {
// No space
        return false;
    }
}

// Fetch a character from the device input buffer, waiting if necessary
// Note: Input is running wo FIFO enabled, so the counter is not checked here.
static unsigned char 
sh_scif_getc(serial_channel *chan)
{
    sh_scif_info *sh_chan = (sh_scif_info *)chan->dev_priv;
    unsigned char c;
    cyg_uint16 _sr;

    do {
        HAL_READ_UINT16(sh_chan->ctrl_base+SCIF_SCSSR, _sr);
    } while ((_sr & CYGARC_REG_SCIF_SCSSR_RDF) == 0);

    HAL_READ_UINT8(sh_chan->ctrl_base+SCIF_SCFRDR, c);

    // Clear buffer full flag (read back first)
    HAL_READ_UINT16(sh_chan->ctrl_base+SCIF_SCSSR, _sr);
    HAL_WRITE_UINT16(sh_chan->ctrl_base+SCIF_SCSSR, 
                     CYGARC_REG_SCIF_SCSSR_CLEARMASK & ~CYGARC_REG_SCIF_SCSSR_RDF);

    return c;
}

// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
sh_scif_set_config(serial_channel *chan, cyg_uint32 key,
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
        if ( true != sh_scif_config_port(chan, config, false) )
            return -EINVAL;
      }
      break;
#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_HW
    case CYG_IO_SET_CONFIG_SERIAL_HW_RX_FLOW_THROTTLE:
      {
          sh_scif_info *ser_chan = (sh_scif_info *)chan->dev_priv;
          cyg_addrword_t base = ser_chan->ctrl_base;
          cyg_uint32 *f = (cyg_uint32 *)xbuf;
          if ( *len < *f )
              return -EINVAL;

          if ( chan->config.flags & CYGNUM_SERIAL_FLOW_RTSCTS_RX ) {
              // Control RX RTC/CTS flow control by disabling/enabling
              // RX interrupt.  When disabled, FIFO will fill up and
              // clear RTS.
              cyg_uint8 _scscr;
              HAL_READ_UINT8(base+SCIF_SCSCR, _scscr);
              if (*f) // we should throttle
                  _scscr &= ~CYGARC_REG_SCIF_SCSCR_RIE;
              else // we should no longer throttle
                  _scscr |= CYGARC_REG_SCIF_SCSCR_RIE;
              HAL_WRITE_UINT8(base+SCIF_SCSCR, _scscr);
          }
#ifdef CYGHWR_SH_SCIF_FLOW_DSRDTR
          if ( chan->config.flags & CYGNUM_SERIAL_FLOW_DSRDTR_RX ) {
              // Control RX DSR/DTR flow control via platform specific macro
              CYGHWR_SH_SCIF_FLOW_DSRDTR_RX(chan, *f);
          }
#endif
      }
      break;
    case CYG_IO_SET_CONFIG_SERIAL_HW_FLOW_CONFIG:
      {
          // Handle CTS/RTS flag
          if ( chan->config.flags & 
               (CYGNUM_SERIAL_FLOW_RTSCTS_RX | CYGNUM_SERIAL_FLOW_RTSCTS_TX )){
              cyg_uint8 _scfcr;
              sh_scif_info *ser_chan = (sh_scif_info *)chan->dev_priv;
              cyg_addrword_t base = ser_chan->ctrl_base;
              cyg_uint8 *f = (cyg_uint8 *)xbuf;

              HAL_READ_UINT8(base+SCIF_SCFCR, _scfcr);
              if (*f) // enable RTS/CTS flow control
                  _scfcr |= CYGARC_REG_SCIF_SCFCR_MCE;
              else // disable RTS/CTS flow control
                  _scfcr &= ~CYGARC_REG_SCIF_SCFCR_MCE;
              HAL_WRITE_UINT8(base+SCIF_SCFCR, _scfcr);
          }
#ifndef CYGHWR_SH_SCIF_FLOW_DSRDTR
          // Clear DSR/DTR flag as it's not supported.
          if (chan->config.flags &
              (CYGNUM_SERIAL_FLOW_DSRDTR_RX|CYGNUM_SERIAL_FLOW_DSRDTR_TX)) {
              chan->config.flags &= ~(CYGNUM_SERIAL_FLOW_DSRDTR_RX|
                                      CYGNUM_SERIAL_FLOW_DSRDTR_TX);
              return -EINVAL;
          }
#else
          return CYGHWR_SH_SCIF_FLOW_DSRDTR_CONFIG(chan);
#endif
      }
      break;
#endif

    CYGPRI_DEVS_SH_SCIF_SET_CONFIG_PLF

    default:
        return -EINVAL;
    }
    return ENOERR;
}


#ifdef CYGINT_IO_SERIAL_SH_SCIF_DMA

// Must be called with serial interrupts disabled
static xmt_req_reply_t
sh_scif_start_dma_xmt(serial_channel *chan)
{
    int chars_avail;
    unsigned char* chars;
    xmt_req_reply_t res;

    // We can transfer the full buffer - ask how much to transfer
    res = (chan->callbacks->data_xmt_req)(chan, chan->out_cbuf.len, 
                                          &chars_avail, &chars);
    if (CYG_XMT_OK == res) {
        sh_scif_info *sh_chan = (sh_scif_info *)chan->dev_priv;
        cyg_uint32 dma_base = sh_chan->dma_xmt_base;
        cyg_uint32 scr;

        // Save the length so it can be used in the DMA DSR
        sh_chan->dma_xmt_len = chars_avail;

        // Flush cache for the area
        HAL_DCACHE_FLUSH((cyg_haladdress)chars, chars_avail);

        // Program DMA
        HAL_WRITE_UINT32(dma_base+CYGARC_REG_CHCR, 0); // disable and clear
        HAL_WRITE_UINT32(dma_base+CYGARC_REG_SAR, (cyg_uint32)chars);
        HAL_WRITE_UINT32(dma_base+CYGARC_REG_DAR, 
                         (sh_chan->ctrl_base+SCIF_SCFTDR) & 0x0fffffff);
        HAL_WRITE_UINT32(dma_base+CYGARC_REG_DMATCR, chars_avail);
        // Source increments, dest static, byte transfer, enable
        // interrupt on completion.
        HAL_WRITE_UINT32(dma_base+CYGARC_REG_CHCR,
                         sh_chan->dma_xmt_cr_flags | CYGARC_REG_CHCR_SM0 \
                         | CYGARC_REG_CHCR_IE | CYGARC_REG_CHCR_DE);

        // Enable serial interrupts
        HAL_READ_UINT8(sh_chan->ctrl_base+SCIF_SCSCR, scr);
        scr |= CYGARC_REG_SCIF_SCSCR_TIE;
        HAL_WRITE_UINT8(sh_chan->ctrl_base+SCIF_SCSCR, scr);
    }

    return res;
}

// must be called with serial interrupts masked
static void
sh_scif_stop_dma_xmt(serial_channel *chan)
{
    sh_scif_info *sh_chan = (sh_scif_info *)chan->dev_priv;
    cyg_uint32 dma_base = sh_chan->dma_xmt_base;
    cyg_uint32 cr;

    // Disable DMA engine and interrupt enable flag.  Should be safe
    // to do since it's triggered by the serial interrupt which has
    // already been disabled.
    HAL_READ_UINT32(dma_base+CYGARC_REG_CHCR, cr);
    cr &= ~(CYGARC_REG_CHCR_IE | CYGARC_REG_CHCR_DE);
    HAL_WRITE_UINT32(dma_base+CYGARC_REG_CHCR, cr);

    // Did transfer complete?
    HAL_READ_UINT32(dma_base+CYGARC_REG_CHCR, cr);
    if (0 == (cr & CYGARC_REG_CHCR_TE)) {
        // Transfer incomplete. Report actually transferred amount of data
        // back to the serial driver.
        int chars_left;
        HAL_READ_UINT32(dma_base+CYGARC_REG_DMATCR, chars_left);
        CYG_ASSERT(chars_left > 0, "DMA incomplete, but no data left");
        CYG_ASSERT(chars_left <= sh_chan->dma_xmt_len,
                   "More data remaining than was attempted transferred");

        (chan->callbacks->data_xmt_done)(chan, 
                                         sh_chan->dma_xmt_len - chars_left);
    }

#ifdef CYGDBG_USE_ASSERTS
    {
        cyg_uint32 dmaor;
        HAL_READ_UINT32(CYGARC_REG_DMAOR, dmaor);
        CYG_ASSERT(0== (dmaor & (CYGARC_REG_DMAOR_AE | CYGARC_REG_DMAOR_NMIF)),
                   "DMA error");
    }
#endif
    
    // The DMA engine is free again.
    sh_chan->dma_xmt_running = false;
}

// Serial xmt DMA completion interrupt handler (ISR)
static cyg_uint32 
sh_dma_xmt_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    sh_scif_info *sh_chan = (sh_scif_info *)chan->dev_priv;
    cyg_uint32 _cr;

    // mask serial interrupt
    HAL_READ_UINT8(sh_chan->ctrl_base+SCIF_SCSCR, _cr);
    _cr &= ~CYGARC_REG_SCIF_SCSCR_TIE;      // Disable xmit interrupt
    HAL_WRITE_UINT8(sh_chan->ctrl_base+SCIF_SCSCR, _cr);

    // mask DMA interrupt and disable engine
    HAL_READ_UINT32(sh_chan->dma_xmt_base+CYGARC_REG_CHCR, _cr);
    _cr &= ~(CYGARC_REG_CHCR_IE | CYGARC_REG_CHCR_DE);
    HAL_WRITE_UINT32(sh_chan->dma_xmt_base+CYGARC_REG_CHCR, _cr);

    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial xmt DMA completion interrupt handler (DSR)
static void       
sh_dma_xmt_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    sh_scif_info *sh_chan = (sh_scif_info *)chan->dev_priv;

    (chan->callbacks->data_xmt_done)(chan, sh_chan->dma_xmt_len);

    // Try to load the engine again.
    sh_chan->dma_xmt_running = 
        (CYG_XMT_OK == sh_scif_start_dma_xmt(chan)) ? true : false;
}
#endif // CYGINT_IO_SERIAL_SH_SCIF_DMA


// Enable the transmitter on the device
static void
sh_scif_start_xmit(serial_channel *chan)
{
    cyg_uint8 _scr;
    sh_scif_info *sh_chan = (sh_scif_info *)chan->dev_priv;
    xmt_req_reply_t _block_status = CYG_XMT_DISABLED;

    if (sh_chan->tx_enabled)
        return;

#ifdef CYGINT_IO_SERIAL_SH_SCIF_DMA
    // Check if the engine is already running. If so, return. Note
    // that there will never be a race on this flag - the caller of
    // this function is respecting a per-channel lock.
    if (sh_chan->dma_xmt_running)
        return;
    // If the channel uses DMA, try to start a DMA job for this -
    // but handle the case where the job doesn't start by falling
    // back to the FIFO/interrupt based code.
    if (sh_chan->dma_enable) {
        _block_status = sh_scif_start_dma_xmt(chan);
        CYG_ASSERT(_block_status != CYG_XMT_EMPTY, 
                   "start_xmit called with empty buffers!");
        sh_chan->dma_xmt_running = 
            (CYG_XMT_OK == _block_status) ? true : false;
    }
#endif // CYGINT_IO_SERIAL_SH_SCIF_DMA

    if (CYG_XMT_DISABLED == _block_status) {
        // Mask interrupts while changing the CR since a rx
        // interrupt or another thread doing the same in the
        // middle of this would result in a bad CR state.
        cyg_drv_isr_lock();
        {
            HAL_READ_UINT8(sh_chan->ctrl_base+SCIF_SCSCR, _scr);
            _scr |= CYGARC_REG_SCIF_SCSCR_TIE;       // Enable xmit interrupt
#ifdef CYGINT_IO_SERIAL_SH_SCIF_IRDA
            if (sh_chan->irda_mode) {
                // Enable transmitter - this automatically disables
                // the receiver in the hardware.  Doing it explicitly
                // (like for async RX/TX below) causes more spurious
                // characters to be read when re-enabling the
                // receiver.
                _scr |= CYGARC_REG_SCIF_SCSCR_TE;
            }
#endif
#ifdef CYGINT_IO_SERIAL_SH_SCIF_ASYNC_RXTX
            if (sh_chan->async_rxtx_mode) {
                // Enable transmitter
                _scr |= CYGARC_REG_SCIF_SCSCR_TE;
                // Disable receiver
                _scr &= ~CYGARC_REG_SCIF_SCSCR_RE;
            }
#endif
            HAL_WRITE_UINT8(sh_chan->ctrl_base+SCIF_SCSCR, _scr);
            sh_chan->tx_enabled = true;
        }
        cyg_drv_isr_unlock();
    }
}

// Disable the transmitter on the device
static void 
sh_scif_stop_xmit(serial_channel *chan)
{
    cyg_uint8 _scr;
    sh_scif_info *sh_chan = (sh_scif_info *)chan->dev_priv;

    // In IrDA and async mode the transmitter needs to be disabled, so
    // wait for transmission to complete within reason: disable it
    // after 0.1s
    if (0
#ifdef CYGINT_IO_SERIAL_SH_SCIF_IRDA
        || sh_chan->irda_mode
#endif
#if defined(CYGINT_IO_SERIAL_SH_SCIF_ASYNC_RXTX)
        || sh_chan->async_rxtx_mode
#endif
        ) {
        cyg_uint16 sr;
        int i = 1000;
        do {
            HAL_READ_UINT16(sh_chan->ctrl_base+SCIF_SCSSR, sr);
            if (sr & CYGARC_REG_SCIF_SCSSR_TEND) break;
            HAL_DELAY_US(100);
        } while (i-- > 0);
    }

    // Mask interrupts while changing the CR since a rx interrupt or
    // another thread doing the same in the middle of this would
    // result in a bad CR state.
    cyg_drv_isr_lock();
    {
            HAL_READ_UINT8(sh_chan->ctrl_base+SCIF_SCSCR, _scr);
            _scr &= ~CYGARC_REG_SCIF_SCSCR_TIE;      // Disable xmit interrupt
#ifdef CYGINT_IO_SERIAL_SH_SCIF_IRDA
            if (sh_chan->irda_mode) {
#ifdef CYGHWR_IO_SERIAL_SH_SCIF_IRDA_TXRX_COMPENSATION
                // In IrDA mode there will be generated spurious RX
                // events when the TX unit is switched on. Eat that
                // character.
                cyg_uint8 _junk;
                cyg_uint16 _sr;
                HAL_READ_UINT8(sh_chan->ctrl_base+SCIF_SCFRDR, _junk);

                // Clear buffer full flag (read back first)
                HAL_READ_UINT16(sh_chan->ctrl_base+SCIF_SCSSR, _sr);
                HAL_WRITE_UINT16(sh_chan->ctrl_base+SCIF_SCSSR, 
                                 CYGARC_REG_SCIF_SCSSR_CLEARMASK & ~(CYGARC_REG_SCIF_SCSSR_RDF|CYGARC_REG_SCIF_SCSSR_DR));
#endif
                // Disable transmitter
                _scr &= ~CYGARC_REG_SCIF_SCSCR_TE;
            }
#endif
#ifdef CYGINT_IO_SERIAL_SH_SCIF_ASYNC_RXTX
            if (sh_chan->async_rxtx_mode) {
                // Enable receiver again
                _scr |= CYGARC_REG_SCIF_SCSCR_RE;
                // Disable transmitter
                _scr &= ~CYGARC_REG_SCIF_SCSCR_TE;
            }
#endif
            HAL_WRITE_UINT8(sh_chan->ctrl_base+SCIF_SCSCR, _scr);
    }
    cyg_drv_isr_unlock();

#ifdef CYGINT_IO_SERIAL_SH_SCIF_DMA
    // If the channel uses DMA, stop the DMA engine.
    if (sh_chan->dma_xmt_running)
        sh_scif_stop_dma_xmt(chan);
    else // dangling else!
#endif // CYGINT_IO_SERIAL_SH_SCIF_DMA
        sh_chan->tx_enabled = false;
}

// Serial I/O - low level tx interrupt handler (ISR)
static cyg_uint32 
sh_scif_tx_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    sh_scif_info *sh_chan = (sh_scif_info *)chan->dev_priv;
    cyg_uint8 _scr;

    HAL_READ_UINT8(sh_chan->ctrl_base+SCIF_SCSCR, _scr);
    _scr &= ~CYGARC_REG_SCIF_SCSCR_TIE;      // mask out tx interrupts
    HAL_WRITE_UINT8(sh_chan->ctrl_base+SCIF_SCSCR, _scr);

    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level tx interrupt handler (DSR)
static void       
sh_scif_tx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    sh_scif_info *sh_chan = (sh_scif_info *)chan->dev_priv;
    xmt_req_reply_t _block_status = CYG_XMT_DISABLED;
    cyg_uint16 _fdr, _sr;
    int _space, _chars_avail;
    unsigned char* _chars;
    CYG_ADDRWORD _base = sh_chan->ctrl_base;

    // Always check if we're supposed to be enabled; the driver runs
    // with DSRs disabled, and a DSR may have been posted (but not
    // executed) before the interrupt was masked.
    if (!sh_chan->tx_enabled)
        return;

#ifdef CYGHWR_SH_SCIF_FLOW_DSRDTR
    CYGHWR_SH_SCIF_FLOW_DSRDTR_TX(chan);
#endif

    // How many chars can we stuff into the FIFO?
    HAL_READ_UINT16(_base+SCIF_SCFDR, _fdr);
    _space = 16 - ((_fdr & CYGARC_REG_SCIF_SCFDR_TCOUNT_MASK) >> CYGARC_REG_SCIF_SCFDR_TCOUNT_shift);

    // Try to do the transfer most efficiently
    _block_status = (chan->callbacks->data_xmt_req)(chan, _space,
                                                    &_chars_avail, &_chars);
    if (CYG_XMT_OK == _block_status) {
        // Transfer the data in block(s).
        do {
            int i = _chars_avail;
            while (i--) {
                HAL_WRITE_UINT8(sh_chan->ctrl_base+SCIF_SCFTDR, *_chars++);
                _space--;
            }
            (chan->callbacks->data_xmt_done)(chan, _chars_avail);
        } while (_space > 0 && 
                 (CYG_XMT_OK == (chan->callbacks->data_xmt_req)(chan, _space,
                                                                &_chars_avail,
                                                                &_chars)));
    } else if (CYG_XMT_DISABLED == _block_status) {
        // Transfer char-by-char, but stop if the transmitter
        // gets disabled.
        while (_space-- && sh_chan->tx_enabled)
            (chan->callbacks->xmt_char)(chan);
    }

    // Clear FIFO-empty/transmit end flags (read back sr first)
    HAL_READ_UINT16(sh_chan->ctrl_base+SCIF_SCSSR, _sr);
    HAL_WRITE_UINT16(sh_chan->ctrl_base+SCIF_SCSSR,
                     CYGARC_REG_SCIF_SCSSR_CLEARMASK & ~CYGARC_REG_SCIF_SCSSR_TDFE);

    if (sh_chan->tx_enabled) {
        cyg_uint8 _scr;
        HAL_READ_UINT8(sh_chan->ctrl_base+SCIF_SCSCR, _scr);
        _scr |= CYGARC_REG_SCIF_SCSCR_TIE;       // unmask tx interrupts
        HAL_WRITE_UINT8(sh_chan->ctrl_base+SCIF_SCSCR, _scr);
    }
}

// Serial I/O - low level RX interrupt handler (ISR)
static cyg_uint32 
sh_scif_rx_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    sh_scif_info *sh_chan = (sh_scif_info *)chan->dev_priv;
    cyg_uint8 _scr;

    HAL_READ_UINT8(sh_chan->ctrl_base+SCIF_SCSCR, _scr);
    _scr &= ~CYGARC_REG_SCIF_SCSCR_RIE;      // mask rx interrupts
    HAL_WRITE_UINT8(sh_chan->ctrl_base+SCIF_SCSCR, _scr);
    return CYG_ISR_CALL_DSR;            // Cause DSR to be run
}

// Serial I/O - high level rx interrupt handler (DSR)
static void       
sh_scif_rx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    sh_scif_info *sh_chan = (sh_scif_info *)chan->dev_priv;
    cyg_uint8 _scr;
    cyg_uint16 _fdr, _sr;
    int _avail, _space_avail;
    unsigned char* _space;
    rcv_req_reply_t _block_status;

    HAL_READ_UINT16(sh_chan->ctrl_base+SCIF_SCFDR, _fdr);
    HAL_READ_UINT16(sh_chan->ctrl_base+SCIF_SCSSR, _sr);

    _avail = _fdr & CYGARC_REG_SCIF_SCFDR_RCOUNT_MASK;
    if (_avail > 0) {
        _block_status = (chan->callbacks->data_rcv_req)(chan, _avail, 
                                                        &_space_avail, &_space);
        if (CYG_RCV_OK == _block_status) {
            // Transfer the data in block(s).
            do {
                int i = _space_avail;
                while(i--) {
                    cyg_uint8 _c;
                    HAL_READ_UINT8(sh_chan->ctrl_base+SCIF_SCFRDR, _c);
                    *_space++ = _c;
                    _avail--;
                }
                (chan->callbacks->data_rcv_done)(chan, _space_avail);
            } while (_avail > 0 &&
                     (CYG_RCV_OK == (chan->callbacks->data_rcv_req)(chan, _avail, 
                                                                    &_space_avail,
                                                                    &_space)));
        } else {
            // Transfer the data char-by-char both for CYG_RCV_FULL
            // and CYG_RCV_DISABLED, leaving all policy decisions with
            // the IO driver.
            while(_avail--) {
                cyg_uint8 _c;
                HAL_READ_UINT8(sh_chan->ctrl_base+SCIF_SCFRDR, _c);
                (chan->callbacks->rcv_char)(chan, _c);
            }
        }
    } else {
        CYG_ASSERT(_avail > 0, "No data to be read in RX DSR");
    }

    // Clear buffer full flag (read back first)
    HAL_READ_UINT16(sh_chan->ctrl_base+SCIF_SCSSR, _sr);
    HAL_WRITE_UINT16(sh_chan->ctrl_base+SCIF_SCSSR, 
                     CYGARC_REG_SCIF_SCSSR_CLEARMASK & ~(CYGARC_REG_SCIF_SCSSR_RDF|CYGARC_REG_SCIF_SCSSR_DR));

    HAL_READ_UINT8(sh_chan->ctrl_base+SCIF_SCSCR, _scr);
    _scr |= CYGARC_REG_SCIF_SCSCR_RIE;       // unmask rx interrupts
    HAL_WRITE_UINT8(sh_chan->ctrl_base+SCIF_SCSCR, _scr);
}

// Serial I/O - low level error interrupt handler (ISR)
static cyg_uint32 
sh_scif_er_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    sh_scif_info *sh_chan = (sh_scif_info *)chan->dev_priv;
    cyg_uint8 _scr;

    HAL_READ_UINT8(sh_chan->ctrl_base+SCIF_SCSCR, _scr);
    _scr &= ~CYGARC_REG_SCIF_SCSCR_RIE;      // mask rx interrupts
    HAL_WRITE_UINT8(sh_chan->ctrl_base+SCIF_SCSCR, _scr);
    return CYG_ISR_CALL_DSR;            // Cause DSR to be run
}

// Serial I/O - high level error interrupt handler (DSR)
static void       
sh_scif_er_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    sh_scif_info *sh_chan = (sh_scif_info *)chan->dev_priv;
    cyg_uint16 _ssr, _ssr_mask;
#ifdef SCIF_SC2SSR
    cyg_uint8 _ssr2;
#endif
    cyg_uint8 _scr;
#ifdef CYGOPT_IO_SERIAL_SUPPORT_LINE_STATUS
    cyg_serial_line_status_t stat;
#endif

    HAL_READ_UINT16(sh_chan->ctrl_base+SCIF_SCSSR, _ssr);
    _ssr_mask = CYGARC_REG_SCIF_SCSSR_CLEARMASK;
    // Clear the ER bit
    _ssr_mask &= ~CYGARC_REG_SCIF_SCSSR_ER;


#ifdef SCIF_SC2SSR
    HAL_READ_UINT8(sh_chan->ctrl_base+SCIF_SC2SSR, _ssr2);
    if (_ssr2 & CYGARC_REG_SCIF_SC2SSR_ORER) {
        _ssr2 &= ~CYGARC_REG_SCIF_SC2SSR_ORER;
        HAL_WRITE_UINT8(sh_chan->ctrl_base+SCIF_SC2SSR, _ssr2);
        stat.which = CYGNUM_SERIAL_STATUS_OVERRUNERR;
        (chan->callbacks->indicate_status)(chan, &stat );
    }
#endif
    if (_ssr & CYGARC_REG_SCIF_SCSSR_FER) {
        // _ssr_mask &= ~CYGARC_REG_SCIF_SCSSR_FER; // FER is read-only
#ifdef CYGOPT_IO_SERIAL_SUPPORT_LINE_STATUS
        stat.which = CYGNUM_SERIAL_STATUS_FRAMEERR;
        (chan->callbacks->indicate_status)(chan, &stat );
#endif
    }
    if (_ssr & CYGARC_REG_SCIF_SCSSR_PER) {
        // _ssr_mask &= ~CYGARC_REG_SCIF_SCSSR_PER; // PER is read-only
#ifdef CYGOPT_IO_SERIAL_SUPPORT_LINE_STATUS
        stat.which = CYGNUM_SERIAL_STATUS_PARITYERR;
        (chan->callbacks->indicate_status)(chan, &stat );
#endif
    }
    if (_ssr & CYGARC_REG_SCIF_SCSSR_BRK) {
        _ssr_mask &= ~CYGARC_REG_SCIF_SCSSR_BRK;
#ifdef CYGOPT_IO_SERIAL_SUPPORT_LINE_STATUS
        stat.which = CYGNUM_SERIAL_STATUS_BREAK;
        (chan->callbacks->indicate_status)(chan, &stat );
#endif
    }
    HAL_WRITE_UINT16(sh_chan->ctrl_base+SCIF_SCSSR, _ssr_mask);

    HAL_READ_UINT8(sh_chan->ctrl_base+SCIF_SCSCR, _scr);
    _scr |= CYGARC_REG_SCIF_SCSCR_RIE;       // unmask rx interrupts
    HAL_WRITE_UINT8(sh_chan->ctrl_base+SCIF_SCSCR, _scr);
}

#endif // ifdef CYGDAT_IO_SERIAL_SH_SCIF_INL
