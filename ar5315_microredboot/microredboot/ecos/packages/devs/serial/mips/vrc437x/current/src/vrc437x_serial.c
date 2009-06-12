//==========================================================================
//
//      io/serial/mips/vrc437x_serial.c
//
//      Mips VRC437X Serial I/O Interface Module (interrupt driven)
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         1999-04-15
// Purpose:      VRC437X Serial I/O module (interrupt driven version)
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

#ifdef CYGPKG_IO_SERIAL_MIPS_VRC437X

#include "vrc437x_serial.h"

#if defined(CYGPKG_HAL_MIPS_LSBFIRST)
#define VRC437X_SCC_BASE 0xC1000000
#elif defined(CYGPKG_HAL_MIPS_MSBFIRST)
#define VRC437X_SCC_BASE 0xC1000003
#else
#error MIPS endianness not defined by configuration
#endif

#define VRC437X_SCC_INT  CYGNUM_HAL_INTERRUPT_DUART
#define SCC_CHANNEL_A             4
#define SCC_CHANNEL_B             0

extern void diag_printf(const char *fmt, ...);

typedef struct vrc437x_serial_info {
    CYG_ADDRWORD   base;
    unsigned char  regs[16];   // Known register state (since hardware is write-only!)
} vrc437x_serial_info;

static bool vrc437x_serial_init(struct cyg_devtab_entry *tab);
static bool vrc437x_serial_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo vrc437x_serial_lookup(struct cyg_devtab_entry **tab, 
                                       struct cyg_devtab_entry *sub_tab,
                                       const char *name);
static unsigned char vrc437x_serial_getc(serial_channel *chan);
static Cyg_ErrNo vrc437x_serial_set_config(serial_channel *chan, cyg_uint32 key,
                                           const void *xbuf, cyg_uint32 *len);
static void vrc437x_serial_start_xmit(serial_channel *chan);
static void vrc437x_serial_stop_xmit(serial_channel *chan);

static cyg_uint32 vrc437x_serial_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       vrc437x_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);

static SERIAL_FUNS(vrc437x_serial_funs, 
                   vrc437x_serial_putc, 
                   vrc437x_serial_getc,
                   vrc437x_serial_set_config,
                   vrc437x_serial_start_xmit,
                   vrc437x_serial_stop_xmit
    );

#ifdef CYGPKG_IO_SERIAL_MIPS_VRC437X_SERIAL0
static vrc437x_serial_info vrc437x_serial_info0 = {VRC437X_SCC_BASE+SCC_CHANNEL_A};
#if CYGNUM_IO_SERIAL_MIPS_VRC437X_SERIAL0_BUFSIZE > 0
static unsigned char vrc437x_serial_out_buf0[CYGNUM_IO_SERIAL_MIPS_VRC437X_SERIAL0_BUFSIZE];
static unsigned char vrc437x_serial_in_buf0[CYGNUM_IO_SERIAL_MIPS_VRC437X_SERIAL0_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(vrc437x_serial_channel0,
                                       vrc437x_serial_funs, 
                                       vrc437x_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MIPS_VRC437X_SERIAL0_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &vrc437x_serial_out_buf0[0], sizeof(vrc437x_serial_out_buf0),
                                       &vrc437x_serial_in_buf0[0], sizeof(vrc437x_serial_in_buf0)
    );
#else
static SERIAL_CHANNEL(vrc437x_serial_channel0,
                      vrc437x_serial_funs, 
                      vrc437x_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MIPS_VRC437X_SERIAL0_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(vrc437x_serial_io0, 
             CYGDAT_IO_SERIAL_MIPS_VRC437X_SERIAL0_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             vrc437x_serial_init, 
             vrc437x_serial_lookup,     // Serial driver may need initializing
             &vrc437x_serial_channel0
    );
#endif //  CYGPKG_IO_SERIAL_MIPS_VRC437X_SERIAL0

#ifdef CYGPKG_IO_SERIAL_MIPS_VRC437X_SERIAL1
static vrc437x_serial_info vrc437x_serial_info1 = {VRC437X_SCC_BASE+SCC_CHANNEL_B};
#if CYGNUM_IO_SERIAL_MIPS_VRC437X_SERIAL1_BUFSIZE > 0
static unsigned char vrc437x_serial_out_buf1[CYGNUM_IO_SERIAL_MIPS_VRC437X_SERIAL1_BUFSIZE];
static unsigned char vrc437x_serial_in_buf1[CYGNUM_IO_SERIAL_MIPS_VRC437X_SERIAL1_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(vrc437x_serial_channel1,
                                       vrc437x_serial_funs, 
                                       vrc437x_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MIPS_VRC437X_SERIAL1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &vrc437x_serial_out_buf1[0], sizeof(vrc437x_serial_out_buf1),
                                       &vrc437x_serial_in_buf1[0], sizeof(vrc437x_serial_in_buf1)
    );
#else
static SERIAL_CHANNEL(vrc437x_serial_channel1,
                      vrc437x_serial_funs, 
                      vrc437x_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MIPS_VRC437X_SERIAL1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(vrc437x_serial_io1, 
             CYGDAT_IO_SERIAL_MIPS_VRC437X_SERIAL1_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             vrc437x_serial_init, 
             vrc437x_serial_lookup,     // Serial driver may need initializing
             &vrc437x_serial_channel1
    );
#endif //  CYGPKG_IO_SERIAL_MIPS_VRC437X_SERIAL1

static cyg_interrupt  vrc437x_serial_interrupt;
static cyg_handle_t   vrc437x_serial_interrupt_handle;

// Table which maps hardware channels (A,B) to software ones
struct serial_channel *vrc437x_chans[] = {
#ifdef CYGPKG_IO_SERIAL_MIPS_VRC437X_SERIAL0    // Hardware channel A
    &vrc437x_serial_channel0,        
#else
    0,
#endif
#ifdef CYGPKG_IO_SERIAL_MIPS_VRC437X_SERIAL1    // Hardware channel B
    &vrc437x_serial_channel1,
#else
    0,
#endif
};

// Support functions which access the serial device.  Note that this chip requires
// a substantial delay after each access. 

#define SCC_DELAY 100
inline static void
scc_delay(void)
{
    int i;
    for (i = 0;  i < SCC_DELAY;  i++) ;
}

inline static void
scc_write_reg(volatile unsigned char *reg, unsigned char val)
{
    scc_delay();
    *reg = val;
}

inline static unsigned char
scc_read_reg(volatile unsigned char *reg)
{
    unsigned char val;
    scc_delay();
    val = *reg;
    return (val);
}

inline static unsigned char
scc_read_ctl(volatile struct serial_port *port, int reg)
{
    if (reg != 0) {
        scc_write_reg(&port->scc_ctl, reg);
    }       
    return (scc_read_reg(&port->scc_ctl));
}

inline static void
scc_write_ctl(volatile struct serial_port *port, int reg, unsigned char val)
{
    if (reg != 0) {
        scc_write_reg(&port->scc_ctl, reg);
    }       
    scc_write_reg(&port->scc_ctl, val);
}

inline static unsigned char
scc_read_dat(volatile struct serial_port *port)
{
    return (scc_read_reg(&port->scc_dat));
}

inline static void
scc_write_dat(volatile struct serial_port *port, unsigned char val)
{
    scc_write_reg(&port->scc_dat, val);
}

// Internal function to actually configure the hardware to desired baud rate, etc.
static bool
vrc437x_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
    vrc437x_serial_info *vrc437x_chan = (vrc437x_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)vrc437x_chan->base;
    cyg_int32 baud_rate = select_baud[new_config->baud];
    cyg_int32 baud_divisor;
    unsigned char *regs = &vrc437x_chan->regs[0];
    if (baud_rate == 0) return false;
    // Compute state of registers.  The register/control state needs to be kept in
    // the shadow variable 'regs' because the hardware registers can only be written,
    // not read (in general).
    if (init) {
        // Insert appropriate resets?
        if (chan->out_cbuf.len != 0) {
            regs[R1] = WR1_IntAllRx;
            regs[R9] = WR9_MIE | WR9_NoVector;
        } else {
            regs[R1] = 0;
            regs[R9] = 0;
        }
        // Clocks are from the baud rate generator
        regs[R11] = WR11_TRxCBR | WR11_TRxCOI | WR11_TxCBR | WR11_RxCBR;  
        regs[R14] = WR14_BRenable | WR14_BRSRC;
        regs[R10] = 0;    // Unused in this [async] mode
        regs[R15] = 0;
    }
    regs[R3] = WR3_RxEnable | select_word_length_WR3[new_config->word_length - CYGNUM_SERIAL_WORD_LENGTH_5];
    regs[R4] = WR4_X16CLK | select_stop_bits[new_config->stop] | select_parity[new_config->parity];
    regs[R5] = WR5_TxEnable  | select_word_length_WR5[new_config->word_length - CYGNUM_SERIAL_WORD_LENGTH_5];
    baud_divisor = BRTC(baud_rate);
    regs[R12] = baud_divisor & 0xFF;
    regs[R13] = baud_divisor >> 8;
    // Now load the registers
    scc_write_ctl(port, R4, regs[R4]);
    scc_write_ctl(port, R10, regs[R10]);
    scc_write_ctl(port, R3, regs[R3] & ~WR3_RxEnable);
    scc_write_ctl(port, R5, regs[R5] & ~WR5_TxEnable);
    scc_write_ctl(port, R1, regs[R1]);
    scc_write_ctl(port, R9, regs[R9]);
    scc_write_ctl(port, R11, regs[R11]);
    scc_write_ctl(port, R12, regs[R12]);
    scc_write_ctl(port, R13, regs[R13]);
    scc_write_ctl(port, R14, regs[R14]);
    scc_write_ctl(port, R15, regs[R15]);
    scc_write_ctl(port, R3, regs[R3]);
    scc_write_ctl(port, R5, regs[R5]);
    // Update configuration
    if (new_config != &chan->config) {
        chan->config = *new_config;
    }
    return true;
}

// Function to initialize the device.  Called at bootstrap time.
static bool 
vrc437x_serial_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    vrc437x_serial_info *vrc437x_chan = (vrc437x_serial_info *)chan->dev_priv;
    static bool init = false;
#ifdef CYGDBG_IO_INIT
    diag_printf("VRC437X SERIAL init '%s' - dev: %x\n", tab->name, vrc437x_chan->base);
#endif
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    if (!init && chan->out_cbuf.len != 0) {
        init = true;
// Note that the hardware is rather broken.  The interrupt status needs to
// be read using only channel A
        cyg_drv_interrupt_create(VRC437X_SCC_INT,
                                 99,                     
                                 (cyg_addrword_t)VRC437X_SCC_BASE+SCC_CHANNEL_A,
                                 vrc437x_serial_ISR,
                                 vrc437x_serial_DSR,
                                 &vrc437x_serial_interrupt_handle,
                                 &vrc437x_serial_interrupt);
        cyg_drv_interrupt_attach(vrc437x_serial_interrupt_handle);
        cyg_drv_interrupt_unmask(VRC437X_SCC_INT);
    }
    vrc437x_serial_config_port(chan, &chan->config, true);
    return true;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
vrc437x_serial_lookup(struct cyg_devtab_entry **tab, 
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
vrc437x_serial_putc(serial_channel *chan, unsigned char c)
{
    vrc437x_serial_info *vrc437x_chan = (vrc437x_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)vrc437x_chan->base;
    if (scc_read_ctl(port, R0) & RR0_TxEmpty) {
// Transmit buffer is empty
        scc_write_dat(port, c);
        return true;
    } else {
// No space
        return false;
    }
}

// Fetch a character from the device input buffer, waiting if necessary
static unsigned char 
vrc437x_serial_getc(serial_channel *chan)
{
    unsigned char c;
    vrc437x_serial_info *vrc437x_chan = (vrc437x_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)vrc437x_chan->base;
    while ((scc_read_ctl(port, R0) & RR0_RxAvail) == 0) ;   // Wait for char
    c = scc_read_dat(port);
    return c;
}

// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
vrc437x_serial_set_config(serial_channel *chan, cyg_uint32 key,
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
        if ( true != vrc437x_serial_config_port(chan, config, false) )
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
vrc437x_serial_start_xmit(serial_channel *chan)
{
    vrc437x_serial_info *vrc437x_chan = (vrc437x_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)vrc437x_chan->base;
    if ((vrc437x_chan->regs[R1] & WR1_TxIntEnab) == 0) {
        CYG_INTERRUPT_STATE old;
        HAL_DISABLE_INTERRUPTS(old);
        vrc437x_chan->regs[R1] |= WR1_TxIntEnab;  // Enable Tx interrupt
        scc_write_ctl(port, R1, vrc437x_chan->regs[R1]);
        (chan->callbacks->xmt_char)(chan);  // Send first character to start xmitter
        HAL_RESTORE_INTERRUPTS(old);
    }
}

// Disable the transmitter on the device
static void 
vrc437x_serial_stop_xmit(serial_channel *chan)
{
    vrc437x_serial_info *vrc437x_chan = (vrc437x_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)vrc437x_chan->base;
    if ((vrc437x_chan->regs[R1] & WR1_TxIntEnab) != 0) {
        CYG_INTERRUPT_STATE old;
        HAL_DISABLE_INTERRUPTS(old);
        vrc437x_chan->regs[R1] &= ~WR1_TxIntEnab;  // Disable Tx interrupt
        scc_write_ctl(port, R1, vrc437x_chan->regs[R1]);
        HAL_RESTORE_INTERRUPTS(old);
    }
}

// Serial I/O - low level interrupt handler (ISR)
static cyg_uint32 
vrc437x_serial_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(VRC437X_SCC_INT);
    cyg_drv_interrupt_acknowledge(VRC437X_SCC_INT);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

inline static void
vrc437x_int(serial_channel *chan, unsigned char stat)
{
    vrc437x_serial_info *vrc437x_chan = (vrc437x_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)vrc437x_chan->base;
    // Note: 'stat' value is interrupt status register, shifted into "B" position
    if (stat & RR3_BRxIP) {
        // Receive interrupt
        unsigned char c;
        c = scc_read_dat(port);
        (chan->callbacks->rcv_char)(chan, c);
    }
    if (stat & RR3_BTxIP) {
        // Transmit interrupt
        (chan->callbacks->xmt_char)(chan);
    }
    if (stat & RR3_BExt) {
        // Status interrupt (parity error, framing error, etc)
    }
}

// Serial I/O - high level interrupt handler (DSR)
// Note: This device presents a single interrupt for both channels.  Thus the
// interrupt handler has to query the device and decide which channel needs service.
// Additionally, more than one interrupt condition may be present so this needs to
// be done in a loop until all interrupt requests have been handled.
// Also note that the hardware is rather broken.  The interrupt status needs to
// be read using only channel A (pointed to by 'data')
static void       
vrc437x_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan;
    volatile struct serial_port *port = (volatile struct serial_port *)data;
    unsigned char stat;
    while (true) {
        stat = scc_read_ctl(port, R3);
        if (stat & (RR3_AExt | RR3_ATxIP | RR3_ARxIP)) {
            chan = vrc437x_chans[0];  // Hardware channel A
            vrc437x_int(chan, stat>>3);  // Handle interrupt
        } else if (stat & (RR3_BExt | RR3_BTxIP | RR3_BRxIP)) {
            chan = vrc437x_chans[1];  // Hardware channel B
            vrc437x_int(chan, stat);  // Handle interrupt
        } else {
            // No more interrupts, all done
            break;
        }
    }
    cyg_drv_interrupt_unmask(VRC437X_SCC_INT);
}
#endif
