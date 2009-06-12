#ifndef CYGONCE_HAL_PLF_IO_H
#define CYGONCE_HAL_PLF_IO_H
//=============================================================================
//
//      plf_io.h
//
//      JTST board specific registers
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   tkoeller
// Contributors: amichelotti
// Date:        2004-06-6
// Purpose:     Atmel JTST board specific registers
// Description: 
// Usage:       #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#define CYGARC_PHYSICAL_ADDRESS(_x_)

// mapping diopsis internal resources
#define AT91_USART0 0xFFFC0000
#define AT91_USART1 0xFFFC4000
#define AT91_SPI0   0xFFFC8000
#define AT91_SPI1   0xFFFCC000

#define AT91_PIO    0xFFFF0000
#define AT91_AIC    0xFFFFF000
#define AT91_TC     0xFFFEC000
#define AT91_EBI    0xFFFE4000
#define AT91_WD     0xFFFF8000
#define AT91_CLKGEN 0xFFF00000 // clock divider
#define AT91_ADDA   0xFFF08000 // ADDA Analog Digital Digital Analog

// PDC2 USART control
#define AT91_US_RPR  0x100 // Receive Pointer Register
#define AT91_US_RCR  0x104 // Receive Counter Register
#define AT91_US_TPR  0x108 // Transmit Pointer Register
#define AT91_US_TCR  0x10C // Transmit Counter Register
#define AT91_US_NRPR 0x110 // Next Receive Pointer Register
#define AT91_US_NRCR 0x114 // Next Receive Counter Register
#define AT91_US_NTPR 0x118 // Next Transmit Pointer Register 
#define AT91_US_NTCR 0x11C // Next Trsnsmit Counter Register

// PDC2 SPI control
#define AT91_SPI_RPR  0x100 // equal meaning for SPI
#define AT91_SPI_RCR  0x104
#define AT91_SPI_TPR  0x108
#define AT91_SPI_TCR  0x10C
#define AT91_SPI_NRPR 0x110
#define AT91_SPI_NRCR 0x114
#define AT91_SPI_NTPR 0x118
#define AT91_SPI_NTCR 0x11C


#define AT91_PDC_PTCR        0x120 // Pdc Transfer control register
#define AT91_PDC_PTCR_TXEN   0x100
#define AT91_PDC_PTCR_RXEN   0x1

#define AT91_PDC_PTCR_TXDIS  0x200
#define AT91_PDC_PTCR_RXDIS  0x2


// CLOCK divider interface

#define AT91_CLKGEN_CPTMAX0    0x0 //counter 0
#define AT91_CLKGEN_CPTMAX1    0x4 //counter 1
#define AT91_CLKGEN_CPTMAX2    0x8 //..
#define AT91_CLKGEN_CPTMAX3    0xC
#define AT91_CLKGEN_CPTMAX4    0x10
#define AT91_CLKGEN_CPTMAX5    0x14
#define AT91_CLKGEN_CPTMAX6    0x18
#define AT91_CLKGEN_CPTMAX7    0x1C
#define AT91_CLKGEN_CPTMAX8    0x20
#define AT91_CLKGEN_CLKENABLE  0x24 // enable clocks out, wronly
#define AT91_CLKGEN_CLKDISABLE 0x28 // disable clocks out, wronly

// ADDA Analog Digital Digital Analog interface

#define AT91_ADDA_CR    0x0  // adda configuration
#define AT91_ADDA_ADCL0 0x20 // ADC input channel0 LEFT
#define AT91_ADDA_ADCR0 0x24 // ADC input channel0 RIGHT
#define AT91_ADDA_ADCL1 0x28 // ADC input channel1 LEFT
#define AT91_ADDA_ADCR1 0x2C // ADC input channel1 RIGHT
#define AT91_ADDA_ADCL2 0x30 // ADC input channel2 LEFT
#define AT91_ADDA_ADCR2 0x34 // ADC input channel2 RIGHT
#define AT91_ADDA_ADCL3 0x38 // ADC input channel3 LEFT
#define AT91_ADDA_ADCR3 0x3C // ADC input channel3 RIGHT

#define AT91_ADDA_DACL0 0x20 // DAC output channel0 LEFT
#define AT91_ADDA_DACR0 0x24 // DAC output channel0 RIGHT
#define AT91_ADDA_DACL1 0x28 // DAC output channel1 LEFT
#define AT91_ADDA_DACR1 0x2C // DAC output channel1 RIGHT
#define AT91_ADDA_DACL2 0x30 // DAC output channel2 LEFT
#define AT91_ADDA_DACR2 0x34 // DAC output channel2 RIGHT
#define AT91_ADDA_DACL3 0x38 // DAC output channel3 LEFT
#define AT91_ADDA_DACR4 0x3C // DAC output channel3 RIGHT

/////  MAGIC DSP
// Magic Data Memory Left BASE 40 bit width (64 bit aligned)
#define AT91_MAARDML    0x00410000
// Magic Data Memory Right BASE  40 bit width (64 bit aligned)
#define AT91_MAARDMR    0x00420000
// Magic Parm Left Base (arm interchange memory) 40 bit width (64 bit aligned)
#define AT91_MAARPARML  0x00490000
// Magic Parm Right Base (arm interchange memory) 40 bit width (64 bit aligned)
#define AT91_MAARPARMR  0x004A0000
// Magic Program Memory
#define AT91_MAARPM     0x00430000

// Magic Global Controller registers 
#define AT91_MAARGSR               0x00450000
#define AT91_MAARGSR_SAR           0x0  // start magic program address
#define AT91_MAARGSR_CONF          0x4  // magic configuration
#define AT91_MAARGSR_STAT          0x8  // magic status rdonly
#define AT91_MAARGSR_EXC           0xC  // magic exception rdonly
#define AT91_MAARGSR_EXC_MSK       0x10 // magic exception mask 
#define AT91_MAARGSR_PC            0x14 // magic program counter
#define AT91_MAARGSR_QCS           0x18 // magic condition stack Q
#define AT91_MAARGSR_ICS           0x1C // magic condition stack I
#define AT91_MAARGSR_PMS           0x20 // magic pma stack
#define AT91_MAARGSR_DMA_TYPE      0x24 // magic dma type
#define AT91_MAARGSR_DMA_LEN       0x28 // magic dma len
#define AT91_MAARGSR_DMA_MOD       0x2C // magic modifier/stride
#define AT91_MAARGSR_DMA_BADD      0x30 // magic dma buffer address 
                                        // (internal address)
#define AT91_MAARGSR_DMA_XADD      0x34 // magic dma external address
#define AT91_MAARGSR_DMA_START     0x38 // magic start dma
#define AT91_MAARGSR_STEP_MODE     0x3C // magic single cycle mode

// Magic MAAR (MAgic ARm interface) Controller registers base
#define AT91_MAARCSE         0x00460000
#define AT91_MAARCSE_CMD     0x0 // command register
#define AT91_MAARCSE_CMD_RUN 0x1 // run
#define AT91_MAARCSE_SR      0x4 // status register
#define AT91_MAARCSE_EXC     0x8 // exception register
#define AT91_MAARCSE_EXC_MSK 0xC // mask exception register

// usarts are connected to clock divider
#define AT91_US_BAUD(baud)  (CYGNUM_HAL_ARM_AT91_CLOCK_SPEED/(16*(baud)))
#define AT91_SPI_BAUD(baud) (CYGNUM_HAL_ARM_AT91_CLOCK_SPEED/(4*(baud)))

#endif // CYGONCE_HAL_PLF_IO_H
