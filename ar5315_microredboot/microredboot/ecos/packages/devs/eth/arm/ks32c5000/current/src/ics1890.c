//==========================================================================
//
//      ics1890.c
//
//      
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
// Contributors: gthomas, jskov
//               Grant Edwards <grante@visi.com>
// Date:         2001-07-31
// Purpose:      
// Description:  
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include "std.h"
#include "ks5000_regs.h"
#include "phy.h"

#define	PHY_CNTL_REG 	0x00
#define	PHY_STATUS_REG 	0x01
#define	PHY_ID_REG1    	0x02
#define	PHY_ID_REG2    	0x03
#define	PHY_ANA_REG    	0x04
#define	PHY_ANLPAR_REG 	0x05
#define	PHY_ANE_REG    	0x06
#define	PHY_ECNTL_REG1 	0x10
#define	PHY_QPDS_REG   	0x11
#define	PHY_10BOP_REG  	0x12
#define	PHY_ECNTL_REG2 	0x13

#ifdef CYGPKG_DEVS_ETH_ARM_KS32C5000_PHYADDR
#define PHYHWADDR  CYGPKG_DEVS_ETH_ARM_KS32C5000_PHYADDR
#else
#define PHYHWADDR  1
#endif

#define Bit(n) (1<<(n))

#define RESET_PHY	Bit(15)
#define ENABLE_LOOPBACK	Bit(14)
#define DR_100MB	Bit(13)
#define ENABLE_AN	Bit(12)
#define PHY_MAC_ISOLATE	Bit(10)
#define RESTART_AN	Bit(9)
#define PHY_FULLDUPLEX	Bit(8)
#define PHY_COL_TEST	Bit(7)

void PhyReset(void)
{
  MiiStationWrite(PHY_CNTL_REG, PHYHWADDR, RESET_PHY ) ;
  MiiStationWrite(PHY_CNTL_REG, PHYHWADDR, ENABLE_AN | RESTART_AN) ;
}

unsigned PhyStatus(void)
{
  unsigned status = MiiStationRead(PHY_QPDS_REG,PHYHWADDR);
  unsigned r = 0;
  if (status & Bit(0))
    r |= PhyStatus_LinkUp;
  if (status & Bit(14))
    r |= PhyStatus_FullDuplex;
  if (status & Bit(15))
    r |=  PhyStatus_100Mb;
  return r;
}
