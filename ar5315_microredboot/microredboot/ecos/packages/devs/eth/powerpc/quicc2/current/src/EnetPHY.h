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
/*------------------------------------------------------------------    
*
* FILE: EnetPHY.c
*
* DESCRIPTION:   LXT970a driver header file
*
*
* Modified for MPC8260 VADS board
*-------------------------------------------------------------------*/

#ifndef _EnetPHY_H 
#define _EnetPHY_H

#include "types.h"

// Board control and status registers
typedef struct bcsr {
  UINT32  bcsr0; 
  UINT32  bcsr1;
  UINT32  bcsr2;
  UINT32  bcsr3;
} t_BCSR;

// Fast ethernet enable/reset pins on bcsr
#define FETHIEN_ 0x08000000
#define FETHRST_ 0x04000000


/**************************/
/* The API for PHY Device */
/**************************/

void   EnableResetPHY(volatile t_BCSR *pBCSR);
UINT16 InitEthernetPHY(VUINT32* pdir, VUINT32* pdat, UINT16 link);
UINT16 EthernetPHYInterruptHandler(void);
void   EnablePHYinterrupt(UINT8 enable);
UINT16 LinkTestPHY(void);


typedef enum MDIORW {READ, WRITE} MDIORW;


#define LINKERROR   0xFFFF
#define NOTLINKED   0x0000
#define TEN_HD      0x0020
#define TEN_FD      0x0040
#define HUNDRED_HD  0x0080
#define HUNDRED_FD  0x0100

#define MD_TEST_FRAME 0xDEAD

//8260 VADS Pin Connections
#define MDIO_PIN_MASK 0x00400000        //PC9  for 8260 VADS
#define MDC_PIN_MASK  0x00200000        //PC10 for 8260 VADS

//#define MDIO_PIN_MASK 0x00000200        //PC9  for 8260 VADS
//#define MDC_PIN_MASK  0x00000400        //PC10 for 8260 VADS

//IEEE 802.3 PHY Register Definitions
#define CONTROL_REG		 0
#define STATUS_REG		 1
#define PHY_ID_REG_A		 2
#define PHY_ID_REG_B		 3
#define AUTONEG_AD_REG		 4
#define AUTONEG_LINKPARTNER_REG  5
#define AUTONEG_EXP_REG		 6

//LXT970a Specific Register Definitions
#define MIRROR_REG		16
#define INT_EN_REG		17
#define INT_STAT_REG		18
#define CONFIG_REG	        19
#define CHIP_STAT_REG	        20

//Clock Timing Control
#define MDC_HOLD_TIME           50

#endif
