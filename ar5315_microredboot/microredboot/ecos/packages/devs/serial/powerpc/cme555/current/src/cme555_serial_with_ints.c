//==========================================================================
//
//      cme555_serial_with_ints.c
//
//      PowerPC 5xx CME555 Serial I/O Interface Module (interrupt driven)
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
// Author(s):   Bob Koninckx
// Contributors:
// Date:        2002-04-25
// Purpose:     CME555 Serial I/O module (interrupt driven version)
// Description: 
//
//   
//####DESCRIPTIONEND####
//==========================================================================
//----------------------------------
// Includes and forward declarations
//----------------------------------
#include <pkgconf/io_serial.h>
#include <pkgconf/io.h>

#include <cyg/io/io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_arbiter.h>
#include <cyg/io/devtab.h>
#include <cyg/infra/diag.h>
#include <cyg/io/serial.h>

// Only build this driver for the MPC555 based CME555 board
#ifdef CYGPKG_IO_SERIAL_POWERPC_CME555

#include "cme555_serial.h"

//-----------------
// Type definitions
//-----------------
typedef struct mpc555_serial_info {
  CYG_ADDRWORD   base;                  // The base address of the serial port
  CYG_WORD       tx_interrupt_num;      // trivial
  CYG_WORD       rx_interrupt_num;      // trivial
  cyg_priority_t tx_interrupt_priority; // trivial
  cyg_priority_t rx_interrupt_priority; // trivial
  bool           tx_interrupt_enable;   // tells if the transmit interrupt may be re-enabled
  cyg_interrupt  tx_interrupt;          // the tx interrupt object
  cyg_handle_t   tx_interrupt_handle;   // the tx interrupt handle
  cyg_interrupt  rx_interrupt;          // the rx interrupt object
  cyg_handle_t   rx_interrupt_handle;   // the rx interrupt handle
} mpc555_serial_info;

//--------------------
// Function prototypes
//--------------------
static bool mpc555_serial_init(struct cyg_devtab_entry * tab);
static bool mpc555_serial_putc(serial_channel * chan, unsigned char c);
static Cyg_ErrNo mpc555_serial_lookup(struct cyg_devtab_entry ** tab, 
                                      struct cyg_devtab_entry * sub_tab,
                                      const char * name);
static unsigned char mpc555_serial_getc(serial_channel *chan);
static Cyg_ErrNo mpc555_serial_set_config(serial_channel *chan, cyg_uint32 key,
                                          const void *xbuf, cyg_uint32 *len);
static void mpc555_serial_start_xmit(serial_channel *chan);
static void mpc555_serial_stop_xmit(serial_channel *chan);

// The interrupt servers
static cyg_uint32 mpc555_serial_tx_ISR(cyg_vector_t vector, cyg_addrword_t data);
static cyg_uint32 mpc555_serial_rx_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       mpc555_serial_tx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);
static void       mpc555_serial_rx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);

//-------------------------------------------
// Register the device driver with the kernel
//-------------------------------------------
static SERIAL_FUNS(mpc555_serial_funs, 
                   mpc555_serial_putc, 
                   mpc555_serial_getc,
                   mpc555_serial_set_config,
                   mpc555_serial_start_xmit,
                   mpc555_serial_stop_xmit);

//-------------------
// Device driver data
//-------------------
#ifdef CYGPKG_IO_SERIAL_POWERPC_CME555_SERIAL_A
static mpc555_serial_info mpc555_serial_info0 = {MPC555_SERIAL_BASE_A,
                                                 CYGNUM_HAL_INTERRUPT_IMB3_SCI0_TX,
                                                 CYGNUM_HAL_INTERRUPT_IMB3_SCI0_RX,
                                                 CYGNUM_HAL_INTERRUPT_IMB3_SCI0_TX_PRIORITY,
                                                 CYGNUM_HAL_INTERRUPT_IMB3_SCI0_RX_PRIORITY,
                                                 false};
#if CYGNUM_IO_SERIAL_POWERPC_CME555_SERIAL_A_BUFSIZE > 0
static unsigned char mpc555_serial_out_buf0[CYGNUM_IO_SERIAL_POWERPC_CME555_SERIAL_A_BUFSIZE]; 
static unsigned char mpc555_serial_in_buf0[CYGNUM_IO_SERIAL_POWERPC_CME555_SERIAL_A_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(mpc555_serial_channel0,
                                       mpc555_serial_funs,
                                       mpc555_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_POWERPC_CME555_SERIAL_A_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &mpc555_serial_out_buf0[0],
                                       sizeof(mpc555_serial_out_buf0),
                                       &mpc555_serial_in_buf0[0],
                                       sizeof(mpc555_serial_in_buf0));
#else 
static SERIAL_CHANNEL(mpc555_serial_channel0,
                      mpc555_serial_funs,
                      mpc555_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_POWERPC_CME555_SERIAL_A_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT);
#endif
DEVTAB_ENTRY(mpc555_serial_io0,
             CYGDAT_IO_SERIAL_POWERPC_CME555_SERIAL_A_NAME,
             0, // does not depend on a lower level device driver
             &cyg_io_serial_devio,
             mpc555_serial_init,
             mpc555_serial_lookup,
             &mpc555_serial_channel0);
#endif // ifdef CYGPKG_IO_SERIAL_POWERPC_CME555_SERIAL_A

#ifdef CYGPKG_IO_SERIAL_POWERPC_CME555_SERIAL_B
static mpc555_serial_info mpc555_serial_info1 = {MPC555_SERIAL_BASE_B,
                                                 CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TX,
                                                 CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RX,
                                                 CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TX_PRIORITY,
                                                 CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RX_PRIORITY,
                                                 false};
#if CYGNUM_IO_SERIAL_POWERPC_CME555_SERIAL_B_BUFSIZE > 0
static unsigned char mpc555_serial_out_buf1[CYGNUM_IO_SERIAL_POWERPC_CME555_SERIAL_B_BUFSIZE]; 
static unsigned char mpc555_serial_in_buf1[CYGNUM_IO_SERIAL_POWERPC_CME555_SERIAL_B_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(mpc555_serial_channel1,
                                       mpc555_serial_funs,
                                       mpc555_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_POWERPC_CME555_SERIAL_B_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &mpc555_serial_out_buf1[0],
                                       sizeof(mpc555_serial_out_buf1),
                                       &mpc555_serial_in_buf1[0],
                                       sizeof(mpc555_serial_in_buf1));
#else
static SERIAL_CHANNEL(mpc555_serial_channel1,
                      mpc555_serial_funs,
                      mpc555_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_POWERPC_CME555_SERIAL_B_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT);
#endif
DEVTAB_ENTRY(mpc555_serial_io1,
             CYGDAT_IO_SERIAL_POWERPC_CME555_SERIAL_B_NAME,
             0, // does not depend on a lower level device driver
             &cyg_io_serial_devio,
             mpc555_serial_init,
             mpc555_serial_lookup,
             &mpc555_serial_channel1);
#endif // ifdef CYGPKG_IO_SERIAL_POWERPC_CME555_SERIAL_B

//-----------------------------
// Device driver implementation
//-----------------------------

// The arbitration isr. 
// I think this is the best place to implement it. The device driver is the only place
// in the code where the knowledge is present about how the hardware is used
//
// Always check receive interrupts. Some rom monitor might be waiting for CTRL-C
static cyg_uint32 hal_arbitration_isr_qsci(CYG_ADDRWORD a_vector, CYG_ADDRWORD a_data)
{
  cyg_uint16 status;
  cyg_uint16 control;

  HAL_READ_UINT16(CYGARC_REG_IMM_SC1SR, status);
  HAL_READ_UINT16(CYGARC_REG_IMM_SCC1R1, control);
  if((status & CYGARC_REG_IMM_SCxSR_RDRF) && (control & CYGARC_REG_IMM_SCCxR1_RIE))
    return hal_call_isr(CYGNUM_HAL_INTERRUPT_IMB3_SCI0_RX);
#ifdef CYGPKG_IO_SERIAL_POWERPC_CME555_SERIAL_A // Do not waist time on unused hardware
  if((status & CYGARC_REG_IMM_SCxSR_TDRE) && (control & CYGARC_REG_IMM_SCCxR1_TIE))
    return hal_call_isr(CYGNUM_HAL_INTERRUPT_IMB3_SCI0_TX);
// Don't waist time on unused interrupts
//  if((status & CYGARC_REG_IMM_SCxSR_TC) && (control & CYGARC_REG_IMM_SCCxR1_TCIE))
//    return hal_call_isr(CYGNUM_HAL_INTERRUPT_IMB3_SCI0_TXC);
// Don't waist time on unused interrupts
//  if((status & CYGARC_REG_IMM_SCxSR_IDLE) && (control & CYGARC_REG_IMM_SCCxR1_ILIE))
//    return hal_call_isr(CYGNUM_HAL_INTERRUPT_IMB3_SCI0_IDLE);
#endif

  HAL_READ_UINT16(CYGARC_REG_IMM_SC2SR, status);
  HAL_READ_UINT16(CYGARC_REG_IMM_SCC2R1, control);
  if((status & CYGARC_REG_IMM_SCxSR_RDRF) && (control & CYGARC_REG_IMM_SCCxR1_RIE))
    return hal_call_isr(CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RX);
#ifdef CYGPKG_IO_SERIAL_POWERPC_CME555_SERIAL_B // Do not waist time on unused hardware
  if((status & CYGARC_REG_IMM_SCxSR_TDRE) && (control & CYGARC_REG_IMM_SCCxR1_TIE))
    return hal_call_isr(CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TX);
// Don't waist time on unused interrupts
//  if((status & CYGARC_REG_IMM_SCxSR_TC) && (control & CYGARC_REG_IMM_SCCxR1_TCIE))
//    return hal_call_isr(CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TXC);
// Don't waist time on unused interrupts
//  if((status & CYGARC_REG_IMM_SCxSR_IDLE) && (control & CYGARC_REG_IMM_SCCxR1_ILIE))
//    return hal_call_isr(CYGNUM_HAL_INTERRUPT_IMB3_SCI1_IDLE);

#if 0
  // The driver doesn't use the queue operation of the hardware (It would need different code for serial 1 and 2
  // since oly one port supports queue mode). So the following is not needed.
  // Leave it there. It is easyer for later implementations to remove the comments than finding
  // out how the hardware works again.
  HAL_READ_UINT16(CYGARC_REG_IMM_QSCI1SR, status);
  HAL_READ_UINT16(CYGARC_REG_IMM_QSCI1CR, control);
  if((status & CYGARC_REG_IMM_QSCI1SR_QTHF) && (control & CYGARC_REG_IMM_QSCI1CR_QTHFI))
    return hal_call_isr(CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RXQTHF);
  if((status & CYGARC_REG_IMM_QSCI1SR_QBHF) && (control & CYGARC_REG_IMM_QSCI1CR_QBHFI))
    return hal_call_isr(CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RXQBHF);
  if((status & CYGARC_REG_IMM_QSCI1SR_QTHE) && (control & CYGARC_REG_IMM_QSCI1CR_QTHEI))
    return hal_call_isr(CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TXQTHE);
  if((status & CYGARC_REG_IMM_QSCI1SR_QBHE) && (control & CYGARC_REG_IMM_QSCI1CR_QBHEI))
    return hal_call_isr(CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TXQBHE);

  cyg_uint16 status;
  cyg_uint16 control;

  HAL_READ_UINT16(CYGARC_REG_IMM_SPSR, status);
  HAL_READ_UINT16(CYGARC_REG_IMM_SPCR2, control);
  if((status & CYGARC_REG_IMM_SPSR_SPIF) && (control & CYGARC_REG_IMM_SPCR2_SPIFIE))
    return hal_call_isr(CYGNUM_HAL_INTERRUPT_IMB3_SPI_FI);

  HAL_READ_UINT16(CYGARC_REG_IMM_SPCR3, control);
  if((status & CYGARC_REG_IMM_SPSR_MODF) && (control & CYGARC_REG_IMM_SPCR3_HMIE))
    return hal_call_isr(CYGNUM_HAL_INTERRUPT_IMB3_SPI_MODF);

  if((status & CYGARC_REG_IMM_SPSR_HALTA) && (control & CYGARC_REG_IMM_SPCR3_HMIE))
    return hal_call_isr(CYGNUM_HAL_INTERRUPT_IMB3_SPI_HALTA);
#endif
  
#endif

  return 0;
}

//--------------------------------------------------------------------------------
// Internal function to actually configure the hardware to desired baud rate, etc.
//--------------------------------------------------------------------------------
static bool mpc555_serial_config_port(serial_channel * chan, cyg_serial_info_t * new_config, bool init)
{
  mpc555_serial_info * mpc555_chan = (mpc555_serial_info *)(chan->dev_priv);

  cyg_addrword_t port = mpc555_chan->base;
  cyg_uint16 baud_rate = select_baud[new_config->baud];
  unsigned char frame_length = 1; // The start bit

  cyg_uint16 old_isrstate;
  cyg_uint16 sccxr;

  if(!baud_rate)
    return false;    // Invalid baud rate selected

  if((new_config->word_length != CYGNUM_SERIAL_WORD_LENGTH_7) &&
     (new_config->word_length != CYGNUM_SERIAL_WORD_LENGTH_8))
    return false;    // Invalid word length selected

  if((new_config->parity != CYGNUM_SERIAL_PARITY_NONE) &&
     (new_config->parity != CYGNUM_SERIAL_PARITY_EVEN) &&
     (new_config->parity != CYGNUM_SERIAL_PARITY_ODD))
    return false;    // Invalid parity selected

  if((new_config->stop != CYGNUM_SERIAL_STOP_1) &&
     (new_config->stop != CYGNUM_SERIAL_STOP_2))
    return false;    // Invalid stop bits selected

  frame_length += select_word_length[new_config->word_length - CYGNUM_SERIAL_WORD_LENGTH_5]; 
  frame_length += select_stop_bits[new_config->stop];
  frame_length += select_parity[new_config->parity];

  if((frame_length != 10) && (frame_length != 11))
    return false;    // Invalid frame format selected

  // Disable port interrupts while changing hardware
  HAL_READ_UINT16(port + MPC555_SERIAL_SCCxR1, sccxr);
  old_isrstate = sccxr;
  old_isrstate &= ~((cyg_uint16)MPC555_SERIAL_SCCxR1_LOOPS);
  old_isrstate &= ~((cyg_uint16)MPC555_SERIAL_SCCxR1_WOMS);
  old_isrstate &= ~((cyg_uint16)MPC555_SERIAL_SCCxR1_ILT);
  old_isrstate &= ~((cyg_uint16)MPC555_SERIAL_SCCxR1_PT);
  old_isrstate &= ~((cyg_uint16)MPC555_SERIAL_SCCxR1_PE);
  old_isrstate &= ~((cyg_uint16)MPC555_SERIAL_SCCxR1_M);
  old_isrstate &= ~((cyg_uint16)MPC555_SERIAL_SCCxR1_WAKE);
  old_isrstate &= ~((cyg_uint16)MPC555_SERIAL_SCCxR1_TE);
  old_isrstate &= ~((cyg_uint16)MPC555_SERIAL_SCCxR1_RE);
  old_isrstate &= ~((cyg_uint16)MPC555_SERIAL_SCCxR1_RWU);
  old_isrstate &= ~((cyg_uint16)MPC555_SERIAL_SCCxR1_SBK);
  sccxr &= ~((cyg_uint16)MPC555_SERIAL_SCCxR1_TIE);
  sccxr &= ~((cyg_uint16)MPC555_SERIAL_SCCxR1_TCIE);
  sccxr &= ~((cyg_uint16)MPC555_SERIAL_SCCxR1_RIE);
  sccxr &= ~((cyg_uint16)MPC555_SERIAL_SCCxR1_ILIE);
  HAL_WRITE_UINT16(port + MPC555_SERIAL_SCCxR1, sccxr);

  // Set databits, stopbits and parity.
  HAL_READ_UINT16(port + MPC555_SERIAL_SCCxR1, sccxr);

  if(frame_length == 11)
    sccxr |= (cyg_uint16)MPC555_SERIAL_SCCxR1_M;
  else
    sccxr &= ~((cyg_uint16)MPC555_SERIAL_SCCxR1_M);

  switch(new_config->parity)
  {
    case CYGNUM_SERIAL_PARITY_NONE:
      sccxr &= ~((cyg_uint16)MPC555_SERIAL_SCCxR1_PE);
      break;
    case CYGNUM_SERIAL_PARITY_EVEN:
      sccxr |= (cyg_uint16)MPC555_SERIAL_SCCxR1_PE;
      sccxr &= ~((cyg_uint16)MPC555_SERIAL_SCCxR1_PT);
      break;
    case CYGNUM_SERIAL_PARITY_ODD:
      sccxr |= (cyg_uint16)MPC555_SERIAL_SCCxR1_PE;
      sccxr |= (cyg_uint16)MPC555_SERIAL_SCCxR1_PT;
      break;
    default:
      break;
  }
  HAL_WRITE_UINT16(port + MPC555_SERIAL_SCCxR1, sccxr);

  // Set baud rate.
  baud_rate &= ~((cyg_uint16)MPC555_SERIAL_SCCxR0_OTHR);
  baud_rate &= ~((cyg_uint16)MPC555_SERIAL_SCCxR0_LINKBD);
  HAL_READ_UINT16(port + MPC555_SERIAL_SCCxR0, sccxr);
  sccxr &= ~(MPC555_SERIAL_SCCxR0_SCxBR);
  sccxr |= baud_rate;
  HAL_WRITE_UINT16(port + MPC555_SERIAL_SCCxR0, sccxr);

  // Enable the device
  HAL_READ_UINT16(port + MPC555_SERIAL_SCCxR1, sccxr);
  sccxr |= MPC555_SERIAL_SCCxR1_TE;
  sccxr |= MPC555_SERIAL_SCCxR1_RE;
  HAL_WRITE_UINT16(port + MPC555_SERIAL_SCCxR1, sccxr);

  if(init) 
  { // enable the receiver interrupt
    HAL_READ_UINT16(port + MPC555_SERIAL_SCCxR1, sccxr);
    sccxr |= MPC555_SERIAL_SCCxR1_RIE;
    HAL_WRITE_UINT16(port + MPC555_SERIAL_SCCxR1, sccxr);
  } 
  else // Restore the old interrupt state
  {
    HAL_READ_UINT16(port + MPC555_SERIAL_SCCxR1, sccxr);
    sccxr |= old_isrstate;
    HAL_WRITE_UINT16(port + MPC555_SERIAL_SCCxR1, sccxr);
  }

  if(new_config != &chan->config) 
    chan->config = *new_config;

  return true;
}

//--------------------------------------------------------------
// Function to initialize the device.  Called at bootstrap time.
//--------------------------------------------------------------
static hal_mpc5xx_arbitration_data arbiter;

static bool mpc555_serial_init(struct cyg_devtab_entry * tab)
{
   serial_channel * chan = (serial_channel *)tab->priv;
   mpc555_serial_info * mpc555_chan = (mpc555_serial_info *)chan->dev_priv;

   if(!mpc555_serial_config_port(chan, &chan->config, true))
     return false;

   (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
   if(chan->out_cbuf.len != 0)
   { 
     arbiter.priority = CYGNUM_HAL_ISR_SOURCE_PRIORITY_QSCI;
     arbiter.data     = 0;
     arbiter.arbiter  = hal_arbitration_isr_qsci;
     
     // Install the arbitration isr, Make sure that is is not installed twice
     hal_mpc5xx_remove_arbitration_isr(CYGNUM_HAL_ISR_SOURCE_PRIORITY_QSCI);
     hal_mpc5xx_install_arbitration_isr(&arbiter); 

     // Create the Tx interrupt, do not enable it yet
     cyg_drv_interrupt_create(mpc555_chan->tx_interrupt_num,
                              mpc555_chan->tx_interrupt_priority,
                              (cyg_addrword_t)chan,   //  Data item passed to interrupt handler
                              mpc555_serial_tx_ISR,
                              mpc555_serial_tx_DSR,
                              &mpc555_chan->tx_interrupt_handle,
                              &mpc555_chan->tx_interrupt);
     cyg_drv_interrupt_attach(mpc555_chan->tx_interrupt_handle);

     // Create the Rx interrupt, this can be safely unmasked now
     cyg_drv_interrupt_create(mpc555_chan->rx_interrupt_num,
                              mpc555_chan->rx_interrupt_priority,
                              (cyg_addrword_t)chan,
                              mpc555_serial_rx_ISR,
                              mpc555_serial_rx_DSR,
                              &mpc555_chan->rx_interrupt_handle,
                              &mpc555_chan->rx_interrupt);
     cyg_drv_interrupt_attach(mpc555_chan->rx_interrupt_handle);
     cyg_drv_interrupt_unmask(mpc555_chan->rx_interrupt_num);
    }

    return true;
}

//----------------------------------------------------------------------
// This routine is called when the device is "looked" up (i.e. attached)
//----------------------------------------------------------------------
static Cyg_ErrNo mpc555_serial_lookup(struct cyg_devtab_entry ** tab, 
                                      struct cyg_devtab_entry * sub_tab,
                                      const char * name)
{
  serial_channel * chan = (serial_channel *)(*tab)->priv;
  (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices

  return ENOERR;
}

//----------------------------------------------
// Send a character to the device output buffer.
// Return 'true' if character is sent to device
//----------------------------------------------
static bool mpc555_serial_putc(serial_channel * chan, unsigned char c)
{
  mpc555_serial_info * mpc555_chan = (mpc555_serial_info *)chan->dev_priv;
  cyg_addrword_t port = mpc555_chan->base;

  cyg_uint16 scsr;
  cyg_uint16 scdr;

  HAL_READ_UINT16(port + MPC555_SERIAL_SCxSR, scsr);
  if(scsr & MPC555_SERIAL_SCxSR_TDRE)
  { // Ok, we have space, write the character and return success
    scdr = (cyg_uint16)c;
    HAL_WRITE_UINT16(port + MPC555_SERIAL_SCxDR, scdr);
    return true;
  }  
  else
    // We cannot write to the transmitter, return failure
    return false;
}

//---------------------------------------------------------------------
// Fetch a character from the device input buffer, waiting if necessary
//---------------------------------------------------------------------
static unsigned char mpc555_serial_getc(serial_channel * chan)
{
  unsigned char c;
  mpc555_serial_info * mpc555_chan = (mpc555_serial_info *)chan->dev_priv;
  cyg_addrword_t port = mpc555_chan->base;

  cyg_uint16 scsr;
  cyg_uint16 scdr;

  do {
    HAL_READ_UINT16(port + MPC555_SERIAL_SCxSR, scsr);
  } while(!(scsr & MPC555_SERIAL_SCxSR_RDRF));

  // Ok, data is received, read it out and return
  HAL_READ_UINT16(port + MPC555_SERIAL_SCxDR, scdr);
  c = (unsigned char)scdr;

  return c;
}

//---------------------------------------------------
// Set up the device characteristics; baud rate, etc.
//---------------------------------------------------
static bool mpc555_serial_set_config(serial_channel * chan, cyg_uint32 key,
                                     const void *xbuf, cyg_uint32 * len)
{
  switch(key) {
  case CYG_IO_SET_CONFIG_SERIAL_INFO:
    {
      cyg_serial_info_t *config = (cyg_serial_info_t *)xbuf;
      if(*len < sizeof(cyg_serial_info_t)) {
	return -EINVAL;
      }
      *len = sizeof(cyg_serial_info_t);
      if(true != mpc555_serial_config_port(chan, config, false))
	return -EINVAL;
    }
    break;
  default:
    return -EINVAL;
  }
  return ENOERR;
}

//-------------------------------------
// Enable the transmitter on the device
//-------------------------------------
static void mpc555_serial_start_xmit(serial_channel * chan)
{
  mpc555_serial_info * mpc555_chan = (mpc555_serial_info *)chan->dev_priv;

  mpc555_chan->tx_interrupt_enable = true;
  cyg_drv_interrupt_unmask(mpc555_chan->tx_interrupt_num);

  // No need to call xmt_char, this will generate an interrupt immediately.
}

//--------------------------------------
// Disable the transmitter on the device
//--------------------------------------
static void mpc555_serial_stop_xmit(serial_channel * chan)
{
  mpc555_serial_info * mpc555_chan = (mpc555_serial_info *)chan->dev_priv;

  cyg_drv_dsr_lock();
  mpc555_chan->tx_interrupt_enable = false;
  cyg_drv_interrupt_mask(mpc555_chan->tx_interrupt_num);
  cyg_drv_dsr_unlock();
}

//-----------------------------------------
// The low level transmit interrupt handler
//-----------------------------------------
static cyg_uint32 mpc555_serial_tx_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
  serial_channel * chan = (serial_channel *)data;
  mpc555_serial_info * mpc555_chan = (mpc555_serial_info *)chan->dev_priv;

  cyg_drv_interrupt_mask(mpc555_chan->tx_interrupt_num);
  cyg_drv_interrupt_acknowledge(mpc555_chan->tx_interrupt_num);

  return CYG_ISR_CALL_DSR; // cause the DSR to run
}

//----------------------------------------
// The low level receive interrupt handler
//----------------------------------------
static cyg_uint32 mpc555_serial_rx_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
  serial_channel * chan = (serial_channel *)data;
  mpc555_serial_info * mpc555_chan = (mpc555_serial_info *)chan->dev_priv;

  cyg_drv_interrupt_mask(mpc555_chan->rx_interrupt_num);
  cyg_drv_interrupt_acknowledge(mpc555_chan->rx_interrupt_num);

  return CYG_ISR_CALL_DSR; // cause the DSR to run
}

//------------------------------------------
// The high level transmit interrupt handler
//------------------------------------------
static void mpc555_serial_tx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
  serial_channel * chan = (serial_channel *)data;
  mpc555_serial_info * mpc555_chan = (mpc555_serial_info *)chan->dev_priv;

  (chan->callbacks->xmt_char)(chan);
  if(mpc555_chan->tx_interrupt_enable)
    cyg_drv_interrupt_unmask(mpc555_chan->tx_interrupt_num);
}

//-----------------------------------------
// The high level receive interrupt handler
//-----------------------------------------
#define MPC555_SERIAL_SCxSR_ERRORS (MPC555_SERIAL_SCxSR_OR | \
                                    MPC555_SERIAL_SCxSR_NF | \
                                    MPC555_SERIAL_SCxSR_FE | \
                                    MPC555_SERIAL_SCxSR_PF)

static void mpc555_serial_rx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
  serial_channel * chan = (serial_channel *)data;
  mpc555_serial_info * mpc555_chan = (mpc555_serial_info *)chan->dev_priv;
  cyg_addrword_t port = mpc555_chan->base;
  cyg_uint16 scdr;
  cyg_uint16 scsr;

  // Allways read out the received character, in order to clear receiver flags
  HAL_READ_UINT16(port + MPC555_SERIAL_SCxDR, scdr);

  HAL_READ_UINT16(port + MPC555_SERIAL_SCxSR, scsr);
  if(scsr & (cyg_uint16)MPC555_SERIAL_SCxSR_ERRORS)
  {
    scsr &= ~((cyg_uint16)MPC555_SERIAL_SCxSR_ERRORS);
    HAL_WRITE_UINT16(port + MPC555_SERIAL_SCxSR, scsr);
  }
  else
  {
    (chan->callbacks->rcv_char)(chan, (cyg_uint8)scdr);
  }

  cyg_drv_interrupt_unmask(mpc555_chan->rx_interrupt_num);
}

#endif // CYGPKG_IO_SERIAL_POWERPC_CME555

// EOF cmd555_serial_with_ints.c
