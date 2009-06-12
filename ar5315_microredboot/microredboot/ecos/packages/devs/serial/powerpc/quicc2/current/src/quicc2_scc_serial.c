//==========================================================================
//
//      io/serial/powerpc/quicc2_scc_serial.c
//
//      PowerPC QUICC2 (SCC) Serial I/O Interface Module (interrupt driven)
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
// Author(s):    mtek
// Contributors: gthomas
// Date:         1999-06-20
// Purpose:      QUICC2 SCC Serial I/O module (interrupt driven version)
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
#include <cyg/hal/var_intr.h>
#include <cyg/io/devtab.h>
#include <cyg/io/serial.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/mpc8260.h>
#include CYGBLD_HAL_PLATFORM_H

#include "quicc2_scc_serial.h"
#define QUICC2_VADS_IMM_BASE    0x04700000
#define QUICC2_VADS_BCSR_BASE   0x04500000

#ifdef CYGPKG_IO_SERIAL_POWERPC_QUICC2_SCC

static bool 
quicc2_scc_serial_init(struct cyg_devtab_entry *tab);
static bool 
quicc2_scc_serial_putc(serial_channel *chan, 
                       unsigned char c);
static Cyg_ErrNo 
quicc2_scc_serial_lookup(struct cyg_devtab_entry **tab, 
                         struct cyg_devtab_entry *sub_tab,
                         const char *name);
static unsigned char 
quicc2_scc_serial_getc(serial_channel *chan);
static Cyg_ErrNo 
quicc2_scc_serial_set_config(serial_channel *chan,
                             cyg_uint32 key, const void *xbuf,
                             cyg_uint32 *len);
static void 
quicc2_scc_serial_start_xmit(serial_channel *chan);
static void 
quicc2_scc_serial_stop_xmit(serial_channel *chan);

static cyg_uint32 
quicc2_scc_serial_ISR(cyg_vector_t vector, 
                      cyg_addrword_t data);
static void       
quicc2_scc_serial_DSR(cyg_vector_t vector, 
                      cyg_ucount32 count, 
                      cyg_addrword_t data);

static SERIAL_FUNS(quicc2_scc_serial_funs, 
                   quicc2_scc_serial_putc, 
                   quicc2_scc_serial_getc,
                   quicc2_scc_serial_set_config,
                   quicc2_scc_serial_start_xmit,
                   quicc2_scc_serial_stop_xmit
    );

#ifdef CYGPKG_IO_SERIAL_POWERPC_QUICC2_SCC_SCC1
static quicc2_scc_serial_info quicc2_scc_serial_info1;

#if CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC1_BUFSIZE > 0
static unsigned char quicc2_scc_serial_out_buf1[CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC1_BUFSIZE];
static unsigned char quicc2_scc_serial_in_buf1[CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC1_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(quicc2_scc_serial_channel1,
                                       quicc2_scc_serial_funs, 
                                       quicc2_scc_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &quicc2_scc_serial_out_buf1[0], sizeof(quicc2_scc_serial_out_buf1),
                                       &quicc2_scc_serial_in_buf1[0], sizeof(quicc2_scc_serial_in_buf1)
    );
#else
static SERIAL_CHANNEL(quicc2_scc_serial_channel1,
                      quicc2_scc_serial_funs, 
                      quicc2_scc_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

static unsigned char quicc2_scc1_txbuf[CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC1_TxNUM]
                                      [CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC1_TxSIZE + HAL_DCACHE_LINE_SIZE-1];
static unsigned char quicc2_scc1_rxbuf[CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC1_RxNUM]
                                      [CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC1_RxSIZE + HAL_DCACHE_LINE_SIZE-1];

DEVTAB_ENTRY(quicc2_scc_serial_io1, 
             CYGDAT_IO_SERIAL_POWERPC_QUICC2_SCC_SCC1_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             quicc2_scc_serial_init, 
             quicc2_scc_serial_lookup,     // Serial driver may need initializing
             &quicc2_scc_serial_channel1
    );
#endif //  CYGPKG_IO_SERIAL_POWERPC_QUICC2_SCC_SCC1

#ifdef CYGPKG_IO_SERIAL_POWERPC_QUICC2_SCC_SCC2
static quicc2_scc_serial_info quicc2_scc_serial_info2;

#if CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC2_BUFSIZE > 0
static unsigned char quicc2_scc_serial_out_buf2[CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC2_BUFSIZE];
static unsigned char quicc2_scc_serial_in_buf2[CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC2_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(quicc2_scc_serial_channel2,
                                       quicc2_scc_serial_funs, 
                                       quicc2_scc_serial_info2,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC2_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &quicc2_scc_serial_out_buf2[0], sizeof(quicc2_scc_serial_out_buf2),
                                       &quicc2_scc_serial_in_buf2[0], sizeof(quicc2_scc_serial_in_buf2)
    );
#else
static SERIAL_CHANNEL(quicc2_scc_serial_channel2,
                      quicc2_scc_serial_funs, 
                      quicc2_scc_serial_info2,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC2_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif
static unsigned char quicc2_scc2_txbuf[CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC2_TxNUM]
                                      [CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC2_TxSIZE + HAL_DCACHE_LINE_SIZE-1];
static unsigned char quicc2_scc2_rxbuf[CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC2_RxNUM]
                                      [CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC2_RxSIZE + HAL_DCACHE_LINE_SIZE-1];

DEVTAB_ENTRY(quicc2_scc_serial_io2, 
             CYGDAT_IO_SERIAL_POWERPC_QUICC2_SCC_SCC2_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             quicc2_scc_serial_init, 
             quicc2_scc_serial_lookup,     // Serial driver may need initializing
             &quicc2_scc_serial_channel2
    );
#endif //  CYGPKG_IO_SERIAL_POWERPC_QUICC2_SCC

#ifdef CYGDBG_DIAG_BUF
extern int enable_diag_uart;
#endif // CYGDBG_DIAG_BUF

// Internal function to actually configure the hardware to 
// desired baud rate, stop bits and parity ...
static bool
quicc2_scc_serial_config_port(serial_channel *chan, 
                              cyg_serial_info_t *new_config, 
                              bool init)
{
    quicc2_scc_serial_info *scc_chan = (quicc2_scc_serial_info *)chan->dev_priv;
    volatile t_PQ2IMM *IMM = (volatile t_PQ2IMM *) QUICC2_VADS_IMM_BASE;

    unsigned long b_rate   = select_baud[new_config->baud];

    if (b_rate == 0) return false;

    // Stop the transmitter while changing baud rate
    while (IMM->cpm_cpcr & QUICC2_CPCR_READY);
    IMM->cpm_cpcr = scc_chan->scc_cpcr | QUICC2_CPCR_STOP_TX | QUICC2_CPCR_READY;
    while (IMM->cpm_cpcr & QUICC2_CPCR_READY);

    // Disable Tx, RX and put them in a reset state
    scc_chan->scc_regs->gsmr_l &= ~(QUICC2_SCC_GSMR_L_ENT | QUICC2_SCC_GSMR_L_ENR);

    // Set the baud rate
    *(scc_chan->brg) = (UART_BIT_RATE(b_rate) << 1) | QUICC2_BRG_EN;

    // Set stop bits, word length and parity
    scc_chan->scc_regs->psmr = QUICC2_SCC_PSMR_ASYNC |
      select_stop_bits[new_config->stop] |
      select_word_length[new_config->word_length - CYGNUM_SERIAL_WORD_LENGTH_5] | 
      select_parity[new_config->parity];

    // Support fractional stop bits 
    scc_chan->scc_regs->dsr = (new_config->stop & 1) ? QUICC2_SCC_DSR_FULL : QUICC2_SCC_DSR_HALF;
      
    // Initialize the parameters
    while (IMM->cpm_cpcr & QUICC2_CPCR_READY);
    IMM->cpm_cpcr = scc_chan->scc_cpcr | QUICC2_CPCR_INIT_TX_RX | QUICC2_CPCR_READY;
    while (IMM->cpm_cpcr & QUICC2_CPCR_READY);

    // Enable Tx and Rx
    scc_chan->scc_regs->gsmr_l |= (QUICC2_SCC_GSMR_L_ENT | QUICC2_SCC_GSMR_L_ENR);

    if (new_config != &chan->config) {
        chan->config = *new_config;
    }
    return true;
}

// Function to set up internal tables for device.
static void
quicc2_scc_serial_init_info(quicc2_scc_serial_info *scc_chan,
                            int SCC_index,
                            int BRG_index,
                            int TxBD, int TxNUM, int TxSIZE,
                            cyg_uint8 *TxBUF,
                            int RxBD, int RxNUM, int RxSIZE,
                            cyg_uint8 *RxBUF)
{
  volatile t_PQ2IMM *IMM = (volatile t_PQ2IMM *) QUICC2_VADS_IMM_BASE;
#ifdef CYGPKG_HAL_POWERPC_VADS
  volatile t_BCSR *bcsr  = (volatile t_BCSR *) QUICC2_VADS_BCSR_BASE;
#endif
  t_UartScc_Pram *uart_pram;
  scc_bd  *txbd, *rxbd;
  int i;
  
  // Disable the channel, just in case 
  IMM->scc_regs[SCC_index-1].gsmr_l &= ~(QUICC2_SCC_GSMR_L_ENT | QUICC2_SCC_GSMR_L_ENR);
  
  switch (SCC_index) {
    
  case 1:
    // Put the data into the info structure 
    scc_chan->scc_cpcr = QUICC2_CPCR_SCC1;
    scc_chan->scc_regs = &(IMM->scc_regs[0]);
    scc_chan->scc_pram = &(IMM->pram.serials.scc_pram[0]);
    scc_chan->int_vector = CYGNUM_HAL_INTERRUPT_SCC1;
    
    // Set-up the PORT D pins
    IMM->io_regs[PORT_D].psor &= ~QUICC2_SCC1_PORTD_PPAR;
    IMM->io_regs[PORT_D].psor |=  QUICC2_SCC1_PORTD_PDIR;
    IMM->io_regs[PORT_D].ppar |=  QUICC2_SCC1_PORTD_PPAR;
    IMM->io_regs[PORT_D].pdir &= ~QUICC2_SCC1_PORTD_PPAR;
    IMM->io_regs[PORT_D].pdir |=  QUICC2_SCC1_PORTD_PDIR;
    IMM->io_regs[PORT_D].podr &= ~QUICC2_SCC1_PORTD_PPAR;

    // Set-up the PORT C pins
    IMM->io_regs[PORT_C].psor &= ~QUICC2_SCC1_PORTC_PPAR;
    IMM->io_regs[PORT_C].ppar |=  QUICC2_SCC1_PORTC_PPAR;
    IMM->io_regs[PORT_C].pdir &= ~QUICC2_SCC1_PORTC_PPAR;
    IMM->io_regs[PORT_C].podr &= ~QUICC2_SCC1_PORTC_PPAR;

    // Select the baud rate generator and connect it 
    IMM->cpm_mux_cmxscr &= QUICC2_CMX_SCC1_CLR;

    switch (BRG_index) {
    case 1:
      scc_chan->brg = &(IMM->brgs_brgc1);
      IMM->cpm_mux_cmxscr |= QUICC2_CMX_SCC1_BRG1;
      break;
    case 2:
      scc_chan->brg = &(IMM->brgs_brgc2);
      IMM->cpm_mux_cmxscr |= QUICC2_CMX_SCC1_BRG2;
      break;
    case 3:
      scc_chan->brg = &(IMM->brgs_brgc3);
      IMM->cpm_mux_cmxscr |= QUICC2_CMX_SCC1_BRG3;
      break;
    case 4:
      scc_chan->brg = &(IMM->brgs_brgc4);
      IMM->cpm_mux_cmxscr |= QUICC2_CMX_SCC1_BRG4;
      break;
    }
#ifdef CYGPKG_HAL_POWERPC_VADS     
    // Enable the transciever
    bcsr->bcsr1 &= ~(QUICC2_BCSR_EN_SCC1);
#endif
    break;

  case 2:
    // Put the data into the info structure 
    scc_chan->scc_cpcr = QUICC2_CPCR_SCC2;
    scc_chan->scc_regs = &(IMM->scc_regs[1]);
    scc_chan->scc_pram = &(IMM->pram.serials.scc_pram[1]);
    scc_chan->int_vector = CYGNUM_HAL_INTERRUPT_SCC2;

    // Set-up the PORT D pins
    IMM->io_regs[PORT_D].psor &= ~QUICC2_SCC2_PORTD_PPAR;
    IMM->io_regs[PORT_D].ppar |=  QUICC2_SCC2_PORTD_PPAR;
    IMM->io_regs[PORT_D].pdir &= ~QUICC2_SCC2_PORTD_PPAR;
    IMM->io_regs[PORT_D].pdir |=  QUICC2_SCC2_PORTD_PDIR;
    IMM->io_regs[PORT_D].podr &= ~QUICC2_SCC2_PORTD_PPAR;

    // Set-up the PORT C pins
    IMM->io_regs[PORT_C].psor &= ~QUICC2_SCC2_PORTC_PPAR;
    IMM->io_regs[PORT_C].ppar |=  QUICC2_SCC2_PORTC_PPAR;
    IMM->io_regs[PORT_C].pdir &= ~QUICC2_SCC2_PORTC_PPAR;
    IMM->io_regs[PORT_C].podr &= ~QUICC2_SCC2_PORTC_PPAR;

    // Select the baud rate generator and connect it 
    IMM->cpm_mux_cmxscr &= QUICC2_CMX_SCC2_CLR;

    switch (BRG_index) {
    case 1:
      scc_chan->brg = &(IMM->brgs_brgc1);
      IMM->cpm_mux_cmxscr |= QUICC2_CMX_SCC2_BRG1;
      break;
    case 2:
      scc_chan->brg = &(IMM->brgs_brgc2);
      IMM->cpm_mux_cmxscr |= QUICC2_CMX_SCC2_BRG2;
      break;
    case 3:
      scc_chan->brg = &(IMM->brgs_brgc3);
      IMM->cpm_mux_cmxscr |= QUICC2_CMX_SCC2_BRG3;
      break;
    case 4:
      scc_chan->brg = &(IMM->brgs_brgc4);
      IMM->cpm_mux_cmxscr |= QUICC2_CMX_SCC2_BRG4;
      break;
    } 
#ifdef CYGPKG_HAL_POWERPC_VADS 
    // Enable the transciever
    bcsr->bcsr1 &= ~(QUICC2_BCSR_EN_SCC2);
#endif
    break;

  default:
    diag_printf("Incorrect SCC index in quicc2_scc_serial_init_info \n");
    break;
  }

  // Initialize common SCC PRAM
  scc_chan->tbase = (scc_bd *) (QUICC2_VADS_IMM_BASE + TxBD);
  scc_chan->rbase = (scc_bd *) (QUICC2_VADS_IMM_BASE + RxBD);
  scc_chan->txbd  = (scc_bd *) (QUICC2_VADS_IMM_BASE + TxBD);
  scc_chan->rxbd  = (scc_bd *) (QUICC2_VADS_IMM_BASE + RxBD);
  scc_chan->txsize = TxSIZE;
  scc_chan->rxsize = RxSIZE;

  scc_chan->scc_pram->rbase = RxBD;
  scc_chan->scc_pram->tbase = TxBD;
  scc_chan->scc_pram->rfcr  = 0x10;
  scc_chan->scc_pram->tfcr  = 0x10;
  scc_chan->scc_pram->mrblr = RxSIZE;

  // Initialize UART PRAM
  uart_pram = &(scc_chan->scc_pram->SpecificProtocol.u);
    
  uart_pram->max_idl = 4;
  uart_pram->brkcr   = 1;
  uart_pram->brkln   = 0;
  uart_pram->parec   = 0;
  uart_pram->frmec   = 0;
  uart_pram->nosec   = 0;
  uart_pram->brkec   = 0;
  uart_pram->uaddr1  = 0;
  uart_pram->uaddr2  = 0;
  uart_pram->toseq   = 0;
  uart_pram->cc[0] = 0x8000;
  uart_pram->cc[1] = 0x8000;
  uart_pram->cc[2] = 0x8000;
  uart_pram->cc[3] = 0x8000;
  uart_pram->cc[4] = 0x8000;
  uart_pram->cc[5] = 0x8000;
  uart_pram->cc[6] = 0x8000;
  uart_pram->cc[7] = 0x8000;
  uart_pram->rccm  = 0xC0FF;

  // Initialize registers
  scc_chan->scc_regs->gsmr_l = QUICC2_SCC_GSMR_L_INIT;
  scc_chan->scc_regs->gsmr_h = QUICC2_SCC_GSMR_H_INIT;
  // scc_chan->scc_regs->psmr   = 0x8000;  // Set by config
  scc_chan->scc_regs->todr   = 0;
  //  scc_chan->scc_regs->dsr    = 0x7e7e; // Set by config
  scc_chan->scc_regs->scce   = 0xffff;
  scc_chan->scc_regs->sccm   = (QUICC2_SCCE_BSY | QUICC2_SCCE_TX | QUICC2_SCCE_RX);

  /* setup RX buffer descriptors */
  rxbd = (struct scc_bd *)((char *) QUICC2_VADS_IMM_BASE + RxBD);

  for (i = 0;  i < RxNUM;  i++) {
    rxbd->ctrl   = QUICC2_BD_CTL_Ready | QUICC2_BD_CTL_Int;
    rxbd->length = 0;
    rxbd->buffer = RxBUF;

    RxBUF += RxSIZE;
    rxbd++;
  }

  rxbd--;
  rxbd->ctrl |= QUICC2_BD_CTL_Wrap;  // Last buffer

  /* setup TX buffer descriptors */
  txbd = (struct scc_bd *)((char *) QUICC2_VADS_IMM_BASE + TxBD);

  for (i = 0;  i < TxNUM;  i++) {
    txbd->ctrl   = 0;
    txbd->length = 0;
    txbd->buffer = TxBUF;
    TxBUF += TxSIZE;
    txbd++;
  }

  txbd--;
  txbd->ctrl |= QUICC2_BD_CTL_Wrap;  // Last buffer
  
  // Issue Init RX & TX Parameters Command 
  while (IMM->cpm_cpcr & QUICC2_CPCR_READY);
  IMM->cpm_cpcr = scc_chan->scc_cpcr | QUICC2_CPCR_INIT_TX_RX | QUICC2_CPCR_READY;
  while (IMM->cpm_cpcr & QUICC2_CPCR_READY);

  return;

}

// Function to initialize the device.  Called at bootstrap time.
static bool 
quicc2_scc_serial_init(struct cyg_devtab_entry *tab)
{
  serial_channel *chan = (serial_channel *)tab->priv;
  quicc2_scc_serial_info *scc_chan = (quicc2_scc_serial_info *)chan->dev_priv;
  volatile t_PQ2IMM *IMM = (volatile t_PQ2IMM *) QUICC2_VADS_IMM_BASE;
  int TxBD, RxBD;
  static int first_init = 1;
  int cache_state;
  
  HAL_DCACHE_IS_ENABLED(cache_state);
  HAL_DCACHE_SYNC();
  HAL_DCACHE_DISABLE();

#ifdef CYGDBG_IO_INIT
  diag_printf("QUICC2_SCC SERIAL init - dev: %x\n", 
              scc_chan->channel);
#endif
    if (first_init) {
        // Set up tables since many fields are dynamic [computed at runtime]
        first_init = 0;
#ifdef CYGPKG_IO_SERIAL_POWERPC_QUICC2_SCC_SCC1

        // Totally reset the CP
        while (IMM->cpm_cpcr & QUICC2_CPCR_READY);
        IMM->cpm_cpcr = QUICC2_CPCR_RESET | QUICC2_CPCR_READY;
        while (IMM->cpm_cpcr & QUICC2_CPCR_READY);

        TxBD = 0x2800;  // Note: this should be configurable
        RxBD = TxBD + CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC1_TxNUM*8;
        quicc2_scc_serial_init_info(&quicc2_scc_serial_info1,
                                    1, // indicates SCC1
                                    CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC1_BRG,
                                    TxBD, 
                                    CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC1_TxNUM,
                                    CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC1_TxSIZE,
                                    ALIGN_TO_CACHELINES(&quicc2_scc1_txbuf[0][0]),
                                    RxBD, 
                                    CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC1_RxNUM,
                                    CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC1_RxSIZE,
                                    ALIGN_TO_CACHELINES(&quicc2_scc1_rxbuf[0][0])
                                    );
#else
#ifdef CYGPKG_HAL_POWERPC_MPC8260
        // Ensure that SCC1 side is initialized first
        diag_init(); // (pull in constructor that inits diag channel)
        TxBD = 0x2900;  // Note : this should be inferred from the
                        // chip state
#else
        // there is no diag device wanting to use the QUICC, so prepare it
        // for SCC2 use only.
        while (IMM->cpm_cpcr & QUICC2_CPCR_READY); // Totally reset the CP
        IMM->cpm_cpcr = QUICC2_CPCR_RESET | QUICC2_CPCR_READY;
        while (IMM->cpm_cpcr & QUICC2_CPCR_READY);
        TxBD = 0x2800; // Note: this should be configurable
#endif
#endif
#ifdef CYGPKG_IO_SERIAL_POWERPC_QUICC2_SCC_SCC2

        RxBD = TxBD + CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC2_TxNUM*8;
        quicc2_scc_serial_init_info(&quicc2_scc_serial_info2,
                                    2, // indicates SCC2
                                    CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC2_BRG,
                                    TxBD, 
                                    CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC2_TxNUM,
                                    CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC2_TxSIZE,
                                    ALIGN_TO_CACHELINES(&quicc2_scc2_txbuf[0][0]),
                                    RxBD, 
                                    CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC2_RxNUM,
                                    CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC2_RxSIZE,
                                    ALIGN_TO_CACHELINES(&quicc2_scc2_rxbuf[0][0])
            );
#endif
    }

    // Really only required for interrupt driven devices
    (chan->callbacks->serial_init)(chan);  
    if (chan->out_cbuf.len != 0) {
        cyg_drv_interrupt_create(scc_chan->int_vector,
                                 0, // CYGARC_SIU_PRIORITY_HIGH, - unused 
                                 (cyg_addrword_t)chan,   //  Data item passed to interrupt handler
                                 quicc2_scc_serial_ISR,
                                 quicc2_scc_serial_DSR,
                                 &scc_chan->serial_interrupt_handle,
                                 &scc_chan->serial_interrupt);
        cyg_drv_interrupt_attach(scc_chan->serial_interrupt_handle);
        cyg_drv_interrupt_acknowledge(scc_chan->int_vector);
        cyg_drv_interrupt_unmask(scc_chan->int_vector);
    }
    quicc2_scc_serial_config_port(chan, &chan->config, true);
    if (cache_state)
        HAL_DCACHE_ENABLE();
    return true;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
quicc2_scc_serial_lookup(struct cyg_devtab_entry **tab, 
                  struct cyg_devtab_entry *sub_tab,
                  const char *name)
{
    serial_channel *chan = (serial_channel *)(*tab)->priv;
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    return ENOERR;
}

// Force the current transmit buffer to be sent
static void
quicc2_scc_serial_flush(quicc2_scc_serial_info *scc_chan)
{
  volatile struct scc_bd *txbd = scc_chan->txbd;
  int cache_state;
                                       
  HAL_DCACHE_IS_ENABLED(cache_state);
  if (cache_state) {
    HAL_DCACHE_FLUSH(txbd->buffer, scc_chan->txsize);
  }

  if ((txbd->length > 0) && 
      ((txbd->ctrl & (QUICC2_BD_CTL_Ready|QUICC2_BD_CTL_Int)) == 0)) {
    txbd->ctrl |= QUICC2_BD_CTL_Ready|QUICC2_BD_CTL_Int;  // Signal buffer ready
    if (txbd->ctrl & QUICC2_BD_CTL_Wrap) {
      txbd = scc_chan->tbase;
    } else {
      txbd++;
    }
    scc_chan->txbd = (scc_bd *) txbd;
  }
}

// Send a character to the device output buffer.
// Return 'true' if character is sent to device
static bool
quicc2_scc_serial_putc(serial_channel *chan, unsigned char c)
{
    quicc2_scc_serial_info *scc_chan = (quicc2_scc_serial_info *)chan->dev_priv;
    volatile struct scc_bd *txbd, *txfirst;
    bool res;

    cyg_drv_dsr_lock();  // Avoid race condition testing pointers

    txbd = (scc_bd *)(QUICC2_VADS_IMM_BASE + ((int) scc_chan->scc_pram->tbptr));
    txfirst = txbd;

    // Scan for a non-busy buffer
    while (txbd->ctrl & QUICC2_BD_CTL_Ready) {
      // This buffer is busy, move to next one
      if (txbd->ctrl & QUICC2_BD_CTL_Wrap) {
        txbd = scc_chan->tbase;
      } else {
        txbd++;
      }
      if (txbd == txfirst) break;  // Went all the way around
    }
    
    scc_chan->txbd = (scc_bd *) txbd;
    if ((txbd->ctrl & (QUICC2_BD_CTL_Ready|QUICC2_BD_CTL_Int)) == 0) {
      // Transmit buffer is not full/busy
      txbd->buffer[txbd->length++] = c;
      if (txbd->length == scc_chan->txsize) {
        // This buffer is now full, tell SCC to start processing it
        quicc2_scc_serial_flush(scc_chan);
      }
      res = true;
    } else {
      // No space
      res = false;
    }

    cyg_drv_dsr_unlock();
    return res;
}

// Fetch a character from the device input buffer, waiting if necessary
static unsigned char 
quicc2_scc_serial_getc(serial_channel *chan)
{
  unsigned char c;
  quicc2_scc_serial_info *scc_chan = (quicc2_scc_serial_info *)chan->dev_priv;
  volatile scc_bd *rxbd = scc_chan->rxbd;

  while ((rxbd->ctrl & QUICC2_BD_CTL_Ready) != 0) ; // WAIT ...

  c = rxbd->buffer[0];
  rxbd->length = scc_chan->rxsize;
  rxbd->ctrl |= QUICC2_BD_CTL_Ready;
  if (rxbd->ctrl & QUICC2_BD_CTL_Wrap) {
    rxbd = scc_chan->rbase;
  } else {
    rxbd++;
  }
  scc_chan->rxbd = (scc_bd *) rxbd;
  return c;
}

// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
quicc2_scc_serial_set_config(serial_channel *chan, cyg_uint32 key,
                            const void *xbuf, cyg_uint32 *len)
{
  switch (key) {
  case CYG_IO_SET_CONFIG_SERIAL_INFO:
    {
      // FIXME - The documentation says that you can't change the baud rate
      // again until at least two BRG input clocks have occurred.
      cyg_serial_info_t *config = (cyg_serial_info_t *)xbuf;
      if ( *len < sizeof(cyg_serial_info_t) ) {
        return -EINVAL;
      }
      *len = sizeof(cyg_serial_info_t);
      if ( true != quicc2_scc_serial_config_port(chan, config, false) )
        return -EINVAL;
    }
    break;
  default:
    return -EINVAL;
  }
  return ENOERR;
}

// Enable the transmitter (interrupt) on the device
static void
quicc2_scc_serial_start_xmit(serial_channel *chan)
{
  quicc2_scc_serial_info *scc_chan = (quicc2_scc_serial_info *)chan->dev_priv;

  cyg_drv_dsr_lock();

  if (scc_chan->txbd->length == 0) {
    // See if there is anything to put in this buffer, just to get it going
    (chan->callbacks->xmt_char)(chan);
  }
  if (scc_chan->txbd->length != 0) {
    // Make sure it gets started
    quicc2_scc_serial_flush(scc_chan);
  }

  cyg_drv_dsr_unlock();
}

// Disable the transmitter on the device
static void 
quicc2_scc_serial_stop_xmit(serial_channel *chan)
{
  quicc2_scc_serial_info *scc_chan = (quicc2_scc_serial_info *)chan->dev_priv;
  // If anything is in the last buffer, need to get it started
  if (scc_chan->txbd->length != 0) {
    quicc2_scc_serial_flush(scc_chan);
  }
}

// Serial I/O - low level interrupt handler (ISR)
static cyg_uint32 
quicc2_scc_serial_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
  serial_channel *chan = (serial_channel *)data;
  quicc2_scc_serial_info *scc_chan = (quicc2_scc_serial_info *)chan->dev_priv;
  cyg_drv_interrupt_mask(scc_chan->int_vector);
  return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR);  // Cause DSR to be run
}

// Serial I/O - high level interrupt handler (DSR)
static void       
quicc2_scc_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    quicc2_scc_serial_info *scc_chan = (quicc2_scc_serial_info *)chan->dev_priv;
    volatile struct scc_regs_8260 *regs = scc_chan->scc_regs;
    volatile scc_bd *txbd;
    volatile scc_bd *rxbd = scc_chan->rxbd;
    scc_bd *rxlast;
    int i, cache_state;

#ifdef CYGDBG_DIAG_BUF
    int _time, _stime;
    externC cyg_tick_count_t cyg_current_time(void);
    cyg_drv_isr_lock();
    enable_diag_uart = 0;
    HAL_CLOCK_READ(&_time);
    _stime = (int)cyg_current_time();
    diag_printf("DSR start - CE: %x, time: %x.%x\n", 
               regs->scce, _stime, _time);
    enable_diag_uart = 1;
#endif // CYGDBG_DIAG_BUF

    if (regs->scce & QUICC2_SCCE_TX) { // Tx Event

#ifdef XX_CYGDBG_DIAG_BUF
      enable_diag_uart = 0;
      txbd = scc_chan->tbase;
      for (i = 0;  i < CYGNUM_IO_SERIAL_POWERPC_QUICC2_SCC_SCC1_TxNUM;  i++, txbd++) {
        diag_printf("Tx BD: %x, length: %d, ctl: %x\n", txbd, txbd->length, txbd->ctrl);
      }
      enable_diag_uart = 1;
#endif // CYGDBG_DIAG_BUF

      regs->scce = QUICC2_SCCE_TX;  // Reset Tx Event
      txbd = scc_chan->tbase;  // First buffer
      while (true) {
        if ((txbd->ctrl & (QUICC2_BD_CTL_Ready|QUICC2_BD_CTL_Int)) == QUICC2_BD_CTL_Int) {
#ifdef XX_CYGDBG_DIAG_BUF
          enable_diag_uart = 0;
          HAL_CLOCK_READ(&_time);
          _stime = (int)cyg_current_time();
          diag_printf("TX Done - Tx: %x, length: %d, time: %x.%x\n", 
                      txbd, txbd->length, _stime, _time);
          enable_diag_uart = 1;
#endif // CYGDBG_DIAG_BUF
          txbd->length = 0;
          txbd->ctrl &= ~QUICC2_BD_CTL_Int;  // Reset interrupt bit
        }

        if (txbd->ctrl & QUICC2_BD_CTL_Wrap) {
          txbd = scc_chan->tbase;
          break;
        } else {
          txbd++;
        }
      }
      (chan->callbacks->xmt_char)(chan);
    }

    while (regs->scce & QUICC2_SCCE_RX) { // Rx Event

      regs->scce = QUICC2_SCCE_RX;  // Reset interrupt state;
      rxlast = (scc_bd *) ((char *)QUICC2_VADS_IMM_BASE + scc_chan->scc_pram->rbptr );

#ifdef CYGDBG_DIAG_BUF
      enable_diag_uart = 0;
      HAL_CLOCK_READ(&_time);
      _stime = (int)cyg_current_time();
      diag_printf("Scan RX - rxbd: %x, rbptr: %x, time: %x.%x\n", 
                  rxbd, rxlast, _stime, _time);
#endif // CYGDBG_DIAG_BUF
      while (rxbd != rxlast) {
        if ((rxbd->ctrl & QUICC2_BD_CTL_Ready) == 0) {
#ifdef CYGDBG_DIAG_BUF
          diag_printf("rxbuf: %x, flags: %x, length: %d\n", 
                      rxbd, rxbd->ctrl, rxbd->length);
          diag_dump_buf(rxbd->buffer, rxbd->length);
#endif // CYGDBG_DIAG_BUF

          for (i = 0;  i < rxbd->length;  i++) {
            (chan->callbacks->rcv_char)(chan, rxbd->buffer[i]);
          }
          // Note: the MBX860 does not seem to snoop/invalidate the data cache properly!
          HAL_DCACHE_IS_ENABLED(cache_state);
          if (cache_state) {
            HAL_DCACHE_INVALIDATE(rxbd->buffer, scc_chan->rxsize);  // Make sure no stale data
          }
          
          rxbd->length = 0;
          rxbd->ctrl |= QUICC2_BD_CTL_Ready;
        }
         
        if (rxbd->ctrl & QUICC2_BD_CTL_Wrap) {
          rxbd = scc_chan->rbase;
        } else {
          rxbd++;
        }
      }
#ifdef CYGDBG_DIAG_BUF
      enable_diag_uart = 1;
#endif // CYGDBG_DIAG_BUF
      scc_chan->rxbd = (scc_bd *) rxbd;
    }

    if (regs->scce & QUICC2_SCCE_BSY) {
#ifdef CYGDBG_DIAG_BUF
      enable_diag_uart = 0;
      diag_printf("RX BUSY interrupt\n");
      enable_diag_uart = 1;
#endif // CYGDBG_DIAG_BUF
      regs->scce = QUICC2_SCCE_BSY;  // Reset interrupt state;
    }
#ifdef CYGDBG_DIAG_BUF
    enable_diag_uart = 0;
    HAL_CLOCK_READ(&_time);
    _stime = (int)cyg_current_time();
    diag_printf("DSR done - CE: %x, time: %x.%x\n", 
                regs->scce, _stime, _time);
    enable_diag_uart = 1;
#endif // CYGDBG_DIAG_BUF
    cyg_drv_interrupt_acknowledge(scc_chan->int_vector);
    cyg_drv_interrupt_unmask(scc_chan->int_vector);
#ifdef CYGDBG_DIAG_BUF
    cyg_drv_isr_unlock();
#endif // CYGDBG_DIAG_BUF
}

#endif // CYGPKG_IO_SERIAL_POWERPC_QUICC2_SCC

// ------------------------------------------------------------------------
// EOF powerpc/quicc2_scc_serial.c
