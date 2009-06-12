//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Atheros Communications, Inc.
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
// Alternative licenses for eCos may be arranged by contacting the copyright
// holders.
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Atheros Communications, Inc.
// Contributors: Atheros Engineering
// Date:         2003-10-22
// Purpose:      
// Description:  AR531X ethernet hardware driver
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
/*
 * Manage the ethernet PHY.
 * This code supports ADMTek Phy. The ADK Tek on power up is already configured 
 * to auto negotiate. Hence auto negotiation seetings are not configured again.
 * AUTO MDIX is one feature which is not enabled by default. This is configured
 * in software.
 */

#if defined(linux)
#include <linux/config.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>

#include "ar531xlnx.h"
#endif

#if defined(__ECOS)
#include "ae531xecos.h"
#endif

extern UINT16 ae531x_MiiRead(UINT32 phyBase, UINT32 phyAddr, UINT8 reg);
extern void ae531x_MiiWrite(UINT32 phyBase, UINT32 phyAddr, UINT8 reg, UINT16 data);

#define phyRegRead ae531x_MiiRead
#define phyRegWrite ae531x_MiiWrite

#if /*DEBUG */ 1
#define ADM_DEBUG_ERROR     0x00000001
#define ADM_DEBUG_PHYSETUP  0x00000002
#define ADM_DEBUG_PHYCHANGE 0x00000004

int rtPhyDebug = ADM_DEBUG_ERROR ;

#define ADM_PRINT(FLG, X)                            \
{                                                   \
    if (rtPhyDebug & (FLG)) {                       \
        DEBUG_PRINTF X;                             \
    }                                               \
}
#else
#define ADM_PRINT(FLG, X)
#endif


/* Convert from phy unit# to (phyBase, phyAddr) pair */
static UINT32 admPhyBase;

#define ADM_PHYBASE(phyUnit) (admPhyBase)


#define ADM_CHIP_ID1_EXPECTATION                   0x1020 
#define ADM_CHIP_ID2_EXPECTATION                   0x0007 
#define ADM_PHY_ADDR                               0x5



void setupAdmTekAutoMdix(void) {

#define PHY_ADDR_SW_PORT 0
#define ADM_SW_AUTO_MDIX_EN     0x8000

   UINT32  phyUnit;
   UINT16  reg = 0;
   UINT32  phyBase;
   /* sw_port address for ports 0-5. Found on page 4-1 of ADM6996FC datasheet v1.0 */   
   UINT32 sw_port_addr[6] = {1,3,5,7,8,9};

   phyBase = admPhyBase;
   
   for(phyUnit=0;phyUnit<6;phyUnit++) {
      reg = phyRegRead(phyBase,PHY_ADDR_SW_PORT,sw_port_addr[phyUnit]);
      ADM_PRINT(ADM_DEBUG_PHYSETUP,("reg(0x%x)=0x%x\n",sw_port_addr[phyUnit],reg));
      reg |= ADM_SW_AUTO_MDIX_EN;
      phyRegWrite(phyBase,PHY_ADDR_SW_PORT,sw_port_addr[phyUnit],reg);
   }

}


/******************************************************************************
*
* phySetup - reset and setup the PHY associated with
* the specified MAC unit number.
*
* Resets the associated PHY port.
*
* RETURNS:
*    TRUE  --> associated PHY is alive
*    FALSE --> no LINKs on this ethernet unit
*/

BOOL
phySetup(int ethUnit, UINT32 phyBase)
{

    UINT32  phyAddr;

    UINT16  phyID1;
    UINT16  phyID2;

    admPhyBase = phyBase;
    
    /* Checking whether this is ADMTek Phy. This is used on AP61. */

    phyID1 = phyRegRead(phyBase, 0x5, 0x0);
    phyID2 = phyRegRead(phyBase, 0x5, 0x1);
    ADM_PRINT(ADM_DEBUG_PHYSETUP,("phyID1=0x%x phyId2=0x%x.\n",phyID1,phyID2));
    if(((phyID1 & 0xfff0) == ADM_CHIP_ID1_EXPECTATION) && (phyID2 == ADM_CHIP_ID2_EXPECTATION)){
       ADM_PRINT(ADM_DEBUG_PHYSETUP,("Found ADM6996FC.phyID1=0x%x phyId2=0x%x.\n",phyID1,phyID2));
       setupAdmTekAutoMdix();
    }
    return TRUE;

}



/******************************************************************************
*
* phyIsDuplexFull - Determines whether the phy ports associated with the
* specified device are FULL or HALF duplex.
*
* RETURNS:
*    TRUE  --> FULL
*    FALSE --> HALF
*/
BOOL
phyIsFullDuplex(int ethUnit)
{

   ethUnit = 0; /* Keep compiler happy*/
   return TRUE;

}

/******************************************************************************
*
* rt_phyIsSpeed100 - Determines the speed of phy ports associated with the
* specified device.
*
* RETURNS:
*    TRUE --> 100Mbit
*    FALSE --> 10Mbit
*/
BOOL
phyIsSpeed100(int phyUnit)
{
   phyUnit = 0; /* Keep compiler happy*/
   return TRUE;
}

/*****************************************************************************
*
* phyCheckStatusChange -- checks for significant changes in PHY state.
*
* A "significant change" is:
*     dropped link (e.g. ethernet cable unplugged) OR
*     autonegotiation completed + link (e.g. ethernet cable plugged in)
*
*/
void
phyCheckStatusChange(int ethUnit)
{
   ethUnit = 0; /* Keep compiler happy*/
}
