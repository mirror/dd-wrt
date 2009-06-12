#ifndef CYGONCE_V850_COMMON_H
#define CYGONCE_V850_COMMON_H

/*=============================================================================
//
//      v850_common.h
//
//      NEC/V850 common definitions
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
// Author(s):    gthomas, jlarmour
// Contributors: gthomas, jlarmour
// Date:         2000-03-10
// Purpose:      NEC/V850 CPU family hardware description
// Description:
// Usage:        #include <cyg/hal/v850_common.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

// Note: these defintions match the documentation, thus no attempt is made
// to sanitise (mangle) the names.  Also, care should be taken to keep this
// clean for use in assembly code (no "C" constructs).

#include <pkgconf/hal.h>

// These definitions are for the NEC V850/SA1 (70301x)

#if CYGINT_HAL_V850_VARIANT_SA1

#define V850_REGS         0xFFFFF000

#define V850_REG_P0       0xFFFFF000
#define V850_REG_P1       0xFFFFF002
#define V850_REG_P2       0xFFFFF004
#define V850_REG_P3       0xFFFFF006
#define V850_REG_P4       0xFFFFF008
#define V850_REG_P5       0xFFFFF00A
#define V850_REG_P6       0xFFFFF00C
#define V850_REG_P7       0xFFFFF00E
#define V850_REG_P8       0xFFFFF010
#define V850_REG_P9       0xFFFFF012
#define V850_REG_P10      0xFFFFF014
#define V850_REG_P11      0xFFFFF016
#define V850_REG_P12      0xFFFFF018

#define V850_REG_PM0      0xFFFFF020
#define V850_REG_PM1      0xFFFFF022
#define V850_REG_PM2      0xFFFFF024
#define V850_REG_PM3      0xFFFFF026
#define V850_REG_PM4      0xFFFFF028
#define V850_REG_PM5      0xFFFFF02A
#define V850_REG_PM6      0xFFFFF02C
#define V850_REG_PM9      0xFFFFF032
#define V850_REG_PM10     0xFFFFF034
#define V850_REG_PM11     0xFFFFF036
#define V850_REG_PM12     0xFFFFF038

#define V850_REG_MM       0xFFFFF04C

#define V850_REG_PMC12    0xFFFFF058

#define V850_REG_DWC      0xFFFFF060
#define V850_REG_BCC      0xFFFFF062
#define V850_REG_SYC      0xFFFFF064
#define V850_REG_MAM      0xFFFFF068

#define V850_REG_PSC      0xFFFFF070
#define V850_REG_PCC      0xFFFFF074
#define V850_REG_SYS      0xFFFFF078

#define V850_REG_PU0      0xFFFFF080
#define V850_REG_PU1      0xFFFFF082
#define V850_REG_PU2      0xFFFFF084
#define V850_REG_PU3      0xFFFFF086
#define V850_REG_PU10     0xFFFFF094
#define V850_REG_PU11     0xFFFFF096

#define V850_REG_PF1      0xFFFFF0A2
#define V850_REG_PF2      0xFFFFF0A4
#define V850_REG_PF10     0xFFFFF0B4

#define V850_REG_EGP0     0xFFFFF0C0
#define V850_REG_EGN0     0xFFFFF0C2

#define V850_REG_WDTIC    0xFFFFF100
#define V850_REG_PIC0     0xFFFFF102
#define V850_REG_PIC1     0xFFFFF104
#define V850_REG_PIC2     0xFFFFF106
#define V850_REG_PIC3     0xFFFFF108
#define V850_REG_PIC4     0xFFFFF10A
#define V850_REG_PIC5     0xFFFFF10C
#define V850_REG_PIC6     0xFFFFF10E
#define V850_REG_WTIIC    0xFFFFF110
#define V850_REG_WTNIIC   0xFFFFF110
#define V850_REG_TMIC00   0xFFFFF112
#define V850_REG_TMIC01   0xFFFFF114
#define V850_REG_TMIC10   0xFFFFF116
#define V850_REG_TMIC11   0xFFFFF118
#define V850_REG_TMIC2    0xFFFFF11A
#define V850_REG_TMIC3    0xFFFFF11C
#define V850_REG_TMIC4    0xFFFFF11E
#define V850_REG_TMIC5    0xFFFFF120
#define V850_REG_CSIC0    0xFFFFF122
#define V850_REG_SERIC0   0xFFFFF124
#define V850_REG_CSIC1    0xFFFFF126
#define V850_REG_SRIC0    0xFFFFF126
#define V850_REG_STIC0    0xFFFFF128
#define V850_REG_CSIC2    0xFFFFF12A
#define V850_REG_SRIC2    0xFFFFF12A
#define V850_REG_SERIC1   0xFFFFF12C
#define V850_REG_SRIC1    0xFFFFF12E
#define V850_REG_STIC1    0xFFFFF130
#define V850_REG_ADIC     0xFFFFF132
#define V850_REG_DMAIC0   0xFFFFF134
#define V850_REG_DMAIC1   0xFFFFF136
#define V850_REG_DMAIC2   0xFFFFF138
#define V850_REG_WTIC     0xFFFFF13A
#define V850_REG_WTNIC    0xFFFFF13A

#define V850_REG_ISPR     0xFFFFF166
#define V850_REG_PRCMD    0xFFFFF170

#define V850_REG_DIOA0    0xFFFFF180
#define V850_REG_DRA0     0xFFFFF182
#define V850_REG_DBC0     0xFFFFF184
#define V850_REG_DCHC0    0xFFFFF186

#define V850_REG_DIOA1    0xFFFFF190
#define V850_REG_DRA1     0xFFFFF192
#define V850_REG_DBC1     0xFFFFF194
#define V850_REG_DCHC1    0xFFFFF196

#define V850_REG_DIOA2    0xFFFFF1A0
#define V850_REG_DRA2     0xFFFFF1A2
#define V850_REG_DBC2     0xFFFFF1A4
#define V850_REG_DCHC2    0xFFFFF1A6

#define V850_REG_TM0      0xFFFFF200
#define V850_REG_CR00     0xFFFFF202
#define V850_REG_CR01     0xFFFFF204
#define V850_REG_PRM0     0xFFFFF206
#define V850_REG_PRM00    0xFFFFF206
#define V850_REG_TMC0     0xFFFFF208
#define V850_REG_CRC0     0xFFFFF20A
#define V850_REG_TOC0     0xFFFFF20C
#define V850_REG_PRM01    0xFFFFF20E

#define V850_REG_TM1      0xFFFFF210
#define V850_REG_CR10     0xFFFFF212
#define V850_REG_CR11     0xFFFFF214
#define V850_REG_PRM1     0xFFFFF216
#define V850_REG_PRM10    0xFFFFF216
#define V850_REG_TMC1     0xFFFFF218
#define V850_REG_CRC1     0xFFFFF21A
#define V850_REG_TOC1     0xFFFFF21C
#define V850_REG_PRM11    0xFFFFF21E

#define V850_REG_TM2      0xFFFFF240
#define V850_REG_CR20     0xFFFFF242
#define V850_REG_TCL2     0xFFFFF244
#define V850_REG_TMC2     0xFFFFF246
#define V850_REG_TM23     0xFFFFF24A
#define V850_REG_CR23     0xFFFFF24C
#define V850_REG_TCL21    0xFFFFF24E

#define V850_REG_TM3      0xFFFFF250
#define V850_REG_CR30     0xFFFFF252
#define V850_REG_TCL3     0xFFFFF254
#define V850_REG_TMC3     0xFFFFF256
#define V850_REG_TCL31    0xFFFFF25E

#define V850_REG_TM4      0xFFFFF260
#define V850_REG_CR40     0xFFFFF262
#define V850_REG_TCL4     0xFFFFF264
#define V850_REG_TMC4     0xFFFFF266
#define V850_REG_TM45     0xFFFFF26A
#define V850_REG_CR45     0xFFFFF26C
#define V850_REG_TCL41    0xFFFFF26E

#define V850_REG_TM5      0xFFFFF270
#define V850_REG_CR50     0xFFFFF272
#define V850_REG_TCL5     0xFFFFF274
#define V850_REG_TMC5     0xFFFFF276
#define V850_REG_TCL51    0xFFFFF27E

#define V850_REG_SIO0     0xFFFFF2A0
#define V850_REG_CSIM0    0xFFFFF2A2
#define V850_REG_CSIS0    0xFFFFF2A4

#define V850_REG_SIO1     0xFFFFF2B0
#define V850_REG_CSIM1    0xFFFFF2B2
#define V850_REG_CSIS1    0xFFFFF2B4

#define V850_REG_SIO2     0xFFFFF2C0
#define V850_REG_CSIM2    0xFFFFF2C2
#define V850_REG_CSIS2    0xFFFFF2C4

#define V850_REG_ASIM0    0xFFFFF300
#define V850_REG_ASIS0    0xFFFFF302
#define V850_REG_BRGC0    0xFFFFF304
#define V850_REG_TXS0     0xFFFFF306
#define V850_REG_RXB0     0xFFFFF308
#define V850_REG_BRGMC0   0xFFFFF30E
#define V850_REG_BRGMC00  0xFFFFF30E

#define V850_REG_ASIM1    0xFFFFF310
#define V850_REG_ASIS1    0xFFFFF312
#define V850_REG_BRGC1    0xFFFFF314
#define V850_REG_TXS1     0xFFFFF316
#define V850_REG_RXB1     0xFFFFF318
#define V850_REG_BRGMC1   0xFFFFF31E
#define V850_REG_BRGMC10  0xFFFFF31E
#define V850_REG_BRGMC01  0xFFFFF320

#define V850_REG_IICC0    0xFFFFF340
#define V850_REG_IICS0    0xFFFFF342
#define V850_REG_IICCL0   0xFFFFF344
#define V850_REG_SVA0     0xFFFFF346
#define V850_REG_IIC0     0xFFFFF348
#define V850_REG_IICX0    0xFFFFF34A

#define V850_REG_WTM      0xFFFFF360
#define V850_REG_OSTS     0xFFFFF380
#define V850_REG_WDCS     0xFFFFF382
#define V850_REG_WDTM     0xFFFFF384

#define V850_REG_RTBL     0xFFFFF3A0
#define V850_REG_RTBH     0xFFFFF3A2
#define V850_REG_RTPM     0xFFFFF3A4
#define V850_REG_RTPC     0xFFFFF3A6

#define V850_REG_ADM      0xFFFFF3C0
#define V850_REG_ADS      0xFFFFF3C2
#define V850_REG_ADCR     0xFFFFF3C4
#define V850_REG_ADCRH    0xFFFFF3C6

/*---------------------------------------------------------------------------*/

// These definitions are for the NEC V850/SB1 (70303x)

#elif CYGINT_HAL_V850_VARIANT_SB1

#define V850_REGS         0xFFFFF000

#define V850_REG_P0       0xFFFFF000
#define V850_REG_P1       0xFFFFF002
#define V850_REG_P2       0xFFFFF004
#define V850_REG_P3       0xFFFFF006
#define V850_REG_P4       0xFFFFF008
#define V850_REG_P5       0xFFFFF00A
#define V850_REG_P6       0xFFFFF00C
#define V850_REG_P7       0xFFFFF00E
#define V850_REG_P8       0xFFFFF010
#define V850_REG_P9       0xFFFFF012
#define V850_REG_P10      0xFFFFF014
#define V850_REG_P11      0xFFFFF016

#define V850_REG_PM0      0xFFFFF020
#define V850_REG_PM1      0xFFFFF022
#define V850_REG_PM2      0xFFFFF024
#define V850_REG_PM3      0xFFFFF026
#define V850_REG_PM4      0xFFFFF028
#define V850_REG_PM5      0xFFFFF02A
#define V850_REG_PM6      0xFFFFF02C
#define V850_REG_PM9      0xFFFFF032
#define V850_REG_PM10     0xFFFFF034
#define V850_REG_PM11     0xFFFFF036

#define V850_REG_PAC      0xFFFFF040
#define V850_REG_MM       0xFFFFF04C

#define V850_REG_DWC      0xFFFFF060
#define V850_REG_BCC      0xFFFFF062
#define V850_REG_SYC      0xFFFFF064
#define V850_REG_MAM      0xFFFFF068

#define V850_REG_PSC      0xFFFFF070
#define V850_REG_PCC      0xFFFFF074
#define V850_REG_SYS      0xFFFFF078

#define V850_REG_PU0      0xFFFFF080
#define V850_REG_PU1      0xFFFFF082
#define V850_REG_PU2      0xFFFFF084
#define V850_REG_PU3      0xFFFFF086
#define V850_REG_PU10     0xFFFFF094
#define V850_REG_PU11     0xFFFFF096

#define V850_REG_PF1      0xFFFFF0A2
#define V850_REG_PF2      0xFFFFF0A4
#define V850_REG_PF3      0xFFFFF0A6
#define V850_REG_PF10     0xFFFFF0B4

#define V850_REG_EGP0     0xFFFFF0C0
#define V850_REG_EGN0     0xFFFFF0C2

#define V850_REG_WDTIC    0xFFFFF100
#define V850_REG_PIC0     0xFFFFF102
#define V850_REG_PIC1     0xFFFFF104
#define V850_REG_PIC2     0xFFFFF106
#define V850_REG_PIC3     0xFFFFF108
#define V850_REG_PIC4     0xFFFFF10A
#define V850_REG_PIC5     0xFFFFF10C
#define V850_REG_PIC6     0xFFFFF10E
#define V850_REG_WTNIIC   0xFFFFF118
#define V850_REG_TMIC00   0xFFFFF11A
#define V850_REG_TMIC01   0xFFFFF11C
#define V850_REG_TMIC10   0xFFFFF11E
#define V850_REG_TMIC11   0xFFFFF120
#define V850_REG_TMIC2    0xFFFFF122
#define V850_REG_TMIC3    0xFFFFF124
#define V850_REG_TMIC4    0xFFFFF126
#define V850_REG_TMIC5    0xFFFFF128
#define V850_REG_TMIC6    0xFFFFF12A
#define V850_REG_TMIC7    0xFFFFF12C
#define V850_REG_CSIC0    0xFFFFF12E
#define V850_REG_SERIC0   0xFFFFF130
#define V850_REG_CSIC1    0xFFFFF132
#define V850_REG_SRIC0    0xFFFFF132
#define V850_REG_STIC0    0xFFFFF134
#define V850_REG_CSIC2    0xFFFFF136
#define V850_REG_SRIC2    0xFFFFF136
#define V850_REG_IICIC1   0xFFFFF138
#define V850_REG_SERIC1   0xFFFFF13A
#define V850_REG_CSIC3    0xFFFFF13C
#define V850_REG_SRIC3    0xFFFFF13C
#define V850_REG_STIC1    0xFFFFF13E
#define V850_REG_CSIC4    0xFFFFF140
#define V850_REG_SRIC4    0xFFFFF140
#define V850_REG_IEBIC1   0xFFFFF142
#define V850_REG_IEBIC2   0xFFFFF144
#define V850_REG_ADIC     0xFFFFF146
#define V850_REG_DMAIC0   0xFFFFF148
#define V850_REG_DMAIC1   0xFFFFF14A
#define V850_REG_DMAIC2   0xFFFFF14C
#define V850_REG_DMAIC3   0xFFFFF14E
#define V850_REG_DMAIC4   0xFFFFF150
#define V850_REG_DMAIC5   0xFFFFF152
#define V850_REG_WTNIC    0xFFFFF154
#define V850_REG_KRIC     0xFFFFF156

#define V850_REG_ISPR     0xFFFFF166
#define V850_REG_PRCMD    0xFFFFF170

#define V850_REG_DIOA0    0xFFFFF180
#define V850_REG_DRA0     0xFFFFF182
#define V850_REG_DBC0     0xFFFFF184
#define V850_REG_DCHC0    0xFFFFF186

#define V850_REG_DIOA1    0xFFFFF190
#define V850_REG_DRA1     0xFFFFF192
#define V850_REG_DBC1     0xFFFFF194
#define V850_REG_DCHC1    0xFFFFF196

#define V850_REG_DIOA2    0xFFFFF1A0
#define V850_REG_DRA2     0xFFFFF1A2
#define V850_REG_DBC2     0xFFFFF1A4
#define V850_REG_DCHC2    0xFFFFF1A6

#define V850_REG_DIOA3    0xFFFFF1B0
#define V850_REG_DRA3     0xFFFFF1B2
#define V850_REG_DBC3     0xFFFFF1B4
#define V850_REG_DCHC3    0xFFFFF1B6

#define V850_REG_DIOA4    0xFFFFF1C0
#define V850_REG_DRA4     0xFFFFF1C2
#define V850_REG_DBC4     0xFFFFF1C4
#define V850_REG_DCHC4    0xFFFFF1C6

#define V850_REG_DIOA5    0xFFFFF1D0
#define V850_REG_DRA5     0xFFFFF1D2
#define V850_REG_DBC5     0xFFFFF1D4
#define V850_REG_DCHC5    0xFFFFF1D6

#define V850_REG_TM0      0xFFFFF200
#define V850_REG_CR00     0xFFFFF202
#define V850_REG_CR01     0xFFFFF204
#define V850_REG_PRM00    0xFFFFF206
#define V850_REG_TMC0     0xFFFFF208
#define V850_REG_CRC0     0xFFFFF20A
#define V850_REG_TOC0     0xFFFFF20C
#define V850_REG_PRM01    0xFFFFF20E

#define V850_REG_TM1      0xFFFFF210
#define V850_REG_CR10     0xFFFFF212
#define V850_REG_CR11     0xFFFFF214
#define V850_REG_PRM10    0xFFFFF216
#define V850_REG_TMC1     0xFFFFF218
#define V850_REG_CRC1     0xFFFFF21A
#define V850_REG_TOC1     0xFFFFF21C
#define V850_REG_PRM11    0xFFFFF21E

#define V850_REG_TM2      0xFFFFF240
#define V850_REG_CR20     0xFFFFF242
#define V850_REG_TCL20    0xFFFFF244
#define V850_REG_TMC2     0xFFFFF246
#define V850_REG_TM23     0xFFFFF24A
#define V850_REG_CR23     0xFFFFF24C
#define V850_REG_TCL21    0xFFFFF24E

#define V850_REG_TM3      0xFFFFF250
#define V850_REG_CR30     0xFFFFF252
#define V850_REG_TCL30    0xFFFFF254
#define V850_REG_TMC3     0xFFFFF256
#define V850_REG_TCL31    0xFFFFF25E

#define V850_REG_TM4      0xFFFFF260
#define V850_REG_CR40     0xFFFFF262
#define V850_REG_TCL40    0xFFFFF264
#define V850_REG_TMC4     0xFFFFF266
#define V850_REG_TM45     0xFFFFF26A
#define V850_REG_CR45     0xFFFFF26C
#define V850_REG_TCL41    0xFFFFF26E

#define V850_REG_TM5      0xFFFFF270
#define V850_REG_CR50     0xFFFFF272
#define V850_REG_TCL50    0xFFFFF274
#define V850_REG_TMC5     0xFFFFF276
#define V850_REG_TCL51    0xFFFFF27E

#define V850_REG_TM6      0xFFFFF280
#define V850_REG_CR60     0xFFFFF282
#define V850_REG_TCL60    0xFFFFF284
#define V850_REG_TMC6     0xFFFFF286
#define V850_REG_TM67     0xFFFFF28A
#define V850_REG_CR67     0xFFFFF28C
#define V850_REG_TCL61    0xFFFFF28E

#define V850_REG_TM7      0xFFFFF290
#define V850_REG_CR70     0xFFFFF292
#define V850_REG_TCL70    0xFFFFF294
#define V850_REG_TMC7     0xFFFFF296
#define V850_REG_TCL71    0xFFFFF29E

#define V850_REG_SIO0     0xFFFFF2A0
#define V850_REG_CSIM0    0xFFFFF2A2
#define V850_REG_CSIS0    0xFFFFF2A4

#define V850_REG_SIO1     0xFFFFF2B0
#define V850_REG_CSIM1    0xFFFFF2B2
#define V850_REG_CSIS1    0xFFFFF2B4

#define V850_REG_SIO2     0xFFFFF2C0
#define V850_REG_CSIM2    0xFFFFF2C2
#define V850_REG_CSIS2    0xFFFFF2C4

#define V850_REG_SIO3     0xFFFFF2D0
#define V850_REG_CSIM3    0xFFFFF2D2
#define V850_REG_CSIS3    0xFFFFF2D4

#define V850_REG_SIO4     0xFFFFF2E0
#define V850_REG_CSIM4    0xFFFFF2E2
#define V850_REG_CSIB4    0xFFFFF2E4
#define V850_REG_BRGCN4   0xFFFFF2E6
#define V850_REG_BRGCK4   0xFFFFF2E8

#define V850_REG_ASIM0    0xFFFFF300
#define V850_REG_ASIS0    0xFFFFF302
#define V850_REG_BRGC0    0xFFFFF304
#define V850_REG_TXS0     0xFFFFF306
#define V850_REG_RXB0     0xFFFFF308
#define V850_REG_BRGMC00  0xFFFFF30E

#define V850_REG_ASIM1    0xFFFFF310
#define V850_REG_ASIS1    0xFFFFF312
#define V850_REG_BRGC1    0xFFFFF314
#define V850_REG_TXS1     0xFFFFF316
#define V850_REG_RXB1     0xFFFFF318
#define V850_REG_BRGMC10  0xFFFFF31E
#define V850_REG_BRGMC01  0xFFFFF320
#define V850_REG_BRGMC11  0xFFFFF322

#define V850_REG_IICC0    0xFFFFF340
#define V850_REG_IICS0    0xFFFFF342
#define V850_REG_IICCL0   0xFFFFF344
#define V850_REG_SVA0     0xFFFFF346
#define V850_REG_IIC0     0xFFFFF348
#define V850_REG_IICX0    0xFFFFF34A
#define V850_REG_IICCE0   0xFFFFF34C

#define V850_REG_IICC1    0xFFFFF350
#define V850_REG_IICS1    0xFFFFF352
#define V850_REG_IICCL1   0xFFFFF354
#define V850_REG_SVA1     0xFFFFF356
#define V850_REG_IIC1     0xFFFFF358
#define V850_REG_IICX1    0xFFFFF35A
#define V850_REG_IICCE1   0xFFFFF35C

#define V850_REG_WTNM     0xFFFFF360
#define V850_REG_WTNCS    0xFFFFF364

#define V850_REG_CORCN    0xFFFFF36C
#define V850_REG_CORRQ    0xFFFFF36E
#define V850_REG_CORAD0   0xFFFFF370
#define V850_REG_CORAD1   0xFFFFF374
#define V850_REG_CORAD2   0xFFFFF378
#define V850_REG_CORAD3   0xFFFFF37C

#define V850_REG_OSTS     0xFFFFF380
#define V850_REG_WDCS     0xFFFFF382
#define V850_REG_WDTM     0xFFFFF384
#define V850_REG_DMAS     0xFFFFF38E

#define V850_REG_RTBL     0xFFFFF3A0
#define V850_REG_RTBH     0xFFFFF3A2
#define V850_REG_RTPM     0xFFFFF3A4
#define V850_REG_RTPC     0xFFFFF3A6

#define V850_REG_ADM1     0xFFFFF3C0
#define V850_REG_ADS      0xFFFFF3C2
#define V850_REG_ADCR     0xFFFFF3C4
#define V850_REG_ADCRH    0xFFFFF3C6
#define V850_REG_ADM2     0xFFFFF3C8

#define V850_REG_KRM      0xFFFFF3D0

#define V850_REG_NCC      0xFFFFF3D4

#else // elif CYGINT_HAL_V850_VARIANT_SB1
# error No v850 variant defined
#endif


/*---------------------------------------------------------------------------*/
/* end of v850_common.h                                                         */
#endif /* CYGONCE_V850_COMMON_H */
