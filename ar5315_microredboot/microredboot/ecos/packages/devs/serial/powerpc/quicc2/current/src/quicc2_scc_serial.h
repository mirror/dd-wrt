#ifndef CYGONCE_POWERPC_QUICC2_SCC_SERIAL_H
#define CYGONCE_POWERPC_QUICC2_SCC_SERIAL_H

// ====================================================================
//
//      quicc2_scc_serial.h
//
//      Device I/O - Description of PowerPC QUICC2/SCC serial hardware
//
// ====================================================================
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
// ====================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):           mtek
// Contributors:        gthomas
// Date:                2002-2-27
// Purpose:     Internal interfaces for serial I/O drivers
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================

// Description of serial ports using QUICC2/SCC

// macro for aligning buffers to cache lines
#define ALIGN_TO_CACHELINES(b) ((cyg_uint8 *)(((CYG_ADDRESS)(b) + (HAL_DCACHE_LINE_SIZE-1)) & ~(HAL_DCACHE_LINE_SIZE-1)))

#define UART_BIT_RATE(n) \
    ((((int)(CYGHWR_HAL_POWERPC_BOARD_SPEED*1000000))/(n * 64))-1)

// SCC PSMR masks ....
#define QUICC2_SCC_PSMR_ASYNC     0x8000
#define QUICC2_SCC_PSMR_SB(n)     ((n-1)<<14)  // Stop bits (1=1sb, 2=2sb)
#define QUICC2_SCC_PSMR_CLEN(n)   ((n-5)<<12)  // Character Length (5-8)
#define QUICC2_SCC_PSMR_PE(n)     (n<<4)       // Parity enable(0=disabled, 1=enabled)
#define QUICC2_SCC_PSMR_RPM(n)    (n<<2)       // Rx Parity mode (0=odd,  1=low, 2=even, 3=high)
#define QUICC2_SCC_PSMR_TPM(n)    (n)          // Tx Parity mode (0=odd,  1=low, 2=even, 3=high)

// SCC DSR masks
#define QUICC2_SCC_DSR_FULL 0x7e7e
#define QUICC2_SCC_DSR_HALF 0x467e

// SCC GSMR masks ...
#define QUICC2_SCC_GSMR_H_INIT  0x00000060 
#define QUICC2_SCC_GSMR_L_INIT  0x00028004 
#define QUICC2_SCC_GSMR_L_ENT   0x00000010
#define QUICC2_SCC_GSMR_L_ENR   0x00000020

// SCC Events (interrupts)
#define QUICC2_SCCE_BRK  0x0040
#define QUICC2_SCCE_BSY  0x0004
#define QUICC2_SCCE_TX   0x0002
#define QUICC2_SCCE_RX   0x0001

// CP commands for SCC1 and SCC2
#define QUICC2_CPCR_SCC1        0x00800000
#define QUICC2_CPCR_SCC2        0x04A00000
#define QUICC2_CPCR_READY       0x00010000 
#define QUICC2_CPCR_INIT_TX_RX  0x0
#define QUICC2_CPCR_INIT_RX     0x1
#define QUICC2_CPCR_INIT_TX     0x2
#define QUICC2_CPCR_STOP_TX     0x4
#define QUICC2_CPCR_RESTART_TX  0x6
#define QUICC2_CPCR_RESET       0x80000000

// SCC Buffer descriptor control bits
#define QUICC2_BD_CTL_Ready 0x8000  // Buffer contains data (tx) or is empty (rx)
#define QUICC2_BD_CTL_Wrap  0x2000  // Last buffer in list
#define QUICC2_BD_CTL_Int   0x1000  // Generate interrupt when empty (tx) or full (rx)

// PORT configuration masks for SCC1 and SCC2
#define QUICC2_SCC1_PORTC_PPAR  (0x00020000)
#define QUICC2_SCC1_PORTD_PPAR  (0x00000003)
#define QUICC2_SCC1_PORTD_PDIR  (0x00000002)

#define QUICC2_SCC2_PORTC_PPAR  (0x00080000)
#define QUICC2_SCC2_PORTD_PPAR  (0x00000018)
#define QUICC2_SCC2_PORTD_PDIR  (0x00000010)

// SCC clock Route register constants
#define QUICC2_CMX_SCC1_CLR      0x00ffffff
#define QUICC2_CMX_SCC1_BRG1     0x00000000
#define QUICC2_CMX_SCC1_BRG2     0x09000000
#define QUICC2_CMX_SCC1_BRG3     0x12000000
#define QUICC2_CMX_SCC1_BRG4     0x1b000000

#define QUICC2_CMX_SCC2_CLR      0xff00ffff
#define QUICC2_CMX_SCC2_BRG1     0x00000000
#define QUICC2_CMX_SCC2_BRG2     0x00090000
#define QUICC2_CMX_SCC2_BRG3     0x00120000
#define QUICC2_CMX_SCC2_BRG4     0x001b0000

static unsigned int select_word_length[] = {
  QUICC2_SCC_PSMR_CLEN(5),  // 5 bits / word (char)
  QUICC2_SCC_PSMR_CLEN(6),
  QUICC2_SCC_PSMR_CLEN(7),
  QUICC2_SCC_PSMR_CLEN(8)
};

static unsigned int select_stop_bits[] = {
  QUICC2_SCC_PSMR_SB(1),   // 0.5 stop bit ??
  QUICC2_SCC_PSMR_SB(1),   // 1   stop bit
  QUICC2_SCC_PSMR_SB(2),   // 1.5 stop bit
  QUICC2_SCC_PSMR_SB(2)    // 2   stop bits
};


static unsigned int select_parity[] = {
    QUICC2_SCC_PSMR_PE(0),                                               // No parity
    QUICC2_SCC_PSMR_PE(1)|QUICC2_SCC_PSMR_TPM(2)|QUICC2_SCC_PSMR_RPM(2), // Even parity
    QUICC2_SCC_PSMR_PE(1)|QUICC2_SCC_PSMR_TPM(0)|QUICC2_SCC_PSMR_RPM(0), // Odd parity
    QUICC2_SCC_PSMR_PE(1)|QUICC2_SCC_PSMR_TPM(3)|QUICC2_SCC_PSMR_RPM(3), // High (mark) parity
    QUICC2_SCC_PSMR_PE(1)|QUICC2_SCC_PSMR_TPM(1)|QUICC2_SCC_PSMR_RPM(1), // Low (space) parity
};


// Baud rate values, will be used by the macro ...
#define QUICC2_BRG_EN  0x00010000
static unsigned long select_baud[] = {
  0,             // unused
  50,
  75,
  110,
  134,
  150,
  200,
  300,
  600,
  1200,
  1800,
  2400,
  3600,
  4800,
  7200,
  9600,
  14400,
  19200,
  38400,
  57600,
  115200,
  230400
};

// Board control and status registers
#define QUICC2_BCSR_EN_SCC1 0x02000000
#define QUICC2_BCSR_EN_SCC2 0x01000000

typedef struct bcsr {
  volatile unsigned long  bcsr0; 
  volatile unsigned long  bcsr1;
  volatile unsigned long  bcsr2;
  volatile unsigned long  bcsr3;
} t_BCSR;


typedef struct scc_bd{
  cyg_int16  ctrl;
  cyg_int16  length;
  cyg_int8  *buffer;
}  scc_bd;

typedef struct quicc2_scc_serial_info {
  unsigned long                   scc_cpcr;        // Selects scc for cpcr
  volatile struct scc_regs_8260  *scc_regs;         // Ptr to scc registers
  volatile t_Scc_Pram            *scc_pram;        // Ptr to scc pram
  volatile int                   *brg;             // Ptr to baud rate generator  
  struct scc_bd                  *txbd, *rxbd;     // Next Tx, Rx descriptor to use
  struct scc_bd                  *tbase, *rbase;   // First Tx, Rx descriptor
  int                             txsize, rxsize;  // Length of individual buffers
  unsigned int                    int_vector;
  cyg_interrupt                   serial_interrupt;
  cyg_handle_t                    serial_interrupt_handle;
} quicc2_scc_serial_info;

#endif // CYGONCE_POWERPC_QUICC_SMC_SERIAL_H
