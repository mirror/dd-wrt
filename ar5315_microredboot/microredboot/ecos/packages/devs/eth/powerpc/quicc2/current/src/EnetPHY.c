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
/*-------------------------------------------------------------------    
*
* FILE: enetPHY.c
*
* DESCRIPTION:   GPIO Management Pins driver for the LXT970a 
*                
*
* Modified for the mpc8260 VADS board
*--------------------------------------------------------------------*/
#include "types.h"
#include <cyg/hal/hal_intr.h> /* HAL_DELAY_US */
#include "EnetPHY.h"

/* Internal functions */
void	MdioSend(UINT32, UINT16);
UINT16	MdioReceive(UINT16);
UINT16  MdioFrame(MDIORW, UINT16, UINT16, UINT32);

VUINT32 * pPortDir;
VUINT32 * pPortData;

/*-------------------------------------------------------------------
*
* FUNCTION NAME: 
*
* DESCRIPTION:
*
* EXTERNAL EFFECT: Turns on the LXT970 transciever
*
* PARAMETERS:
*
* RETURNS: None
*
* ASSUMPTIONS:
*
*-------------------------------------------------------------------*/
void 
EnableResetPHY(volatile t_BCSR *pBCSR)
{
#ifdef CYGPKG_HAL_POWERPC_TS6
#define ETH_RST_MASK 0x20
  /* The FPGA control register on the TS6 board uses the same memory
   * location as the BCSR register on the VADS board.
   */
  volatile cyg_uint32 *fpga_ctrl = (cyg_uint32 *) pBCSR;
  volatile cyg_uint32 *fpga_vers;
  cyg_uint32 value;
  fpga_vers = fpga_ctrl + 1;
  value = *fpga_vers;
  if(value >= 6){ /* version 06 of the FPGA added PHY reset control */
    value = *fpga_ctrl;
    /* Set the PHY reset bit */
    value |= ETH_RST_MASK;
    *fpga_vers = value;

    /* Give the PHY time to reset */
    HAL_DELAY_US(10000);

    /* Clear the reset bit */
    *fpga_vers = value & ~ETH_RST_MASK;
  }
#else
  // active low FETHIEN on BSCR1, assert reset low
  pBCSR->bcsr1 &= ~(FETHIEN_ | FETHRST_);  
  // de-assert reset
  pBCSR->bcsr1 |= FETHRST_;  
#endif
}


/*-------------------------------------------------------------------
*
* FUNCTION NAME: 
*
* DESCRIPTION: Writes parameters to the control registers of LXT970
*
* EXTERNAL EFFECT:
*
* PARAMETERS:
*
* RETURNS: None
*
* ASSUMPTIONS:
*
*-------------------------------------------------------------------*/
UINT16 
InitEthernetPHY(VUINT32* pdir, VUINT32* pdat, UINT16 link)
{

  VUINT16 FrameValue;
  
  /* 8101 Ethernet Management Pin Assignments */
  pPortDir = pdir;
  pPortData = pdat;
  
  (*pPortDir) |= MDC_PIN_MASK;   /* MD_Clock will always be output only */
  
  /* Test MDC & MDIO Pin Connection to PHY */
  MdioFrame(WRITE, 0, MIRROR_REG, MD_TEST_FRAME); //send test frame
  MdioFrame(WRITE, 0, MIRROR_REG, MD_TEST_FRAME); //send test frame
  FrameValue = MdioFrame(READ, 0, MIRROR_REG, 0); //read test frame

  if (FrameValue != MD_TEST_FRAME) 
    return LINKERROR;	//test data integrity
  
  /* General Configuration */
  MdioFrame(WRITE, 0, CONFIG_REG, 0x0000);  

  if(link == HUNDRED_HD)
      MdioFrame(WRITE, 0, AUTONEG_AD_REG, 0x0081); //100 Mbps Half, 802.3
  else
      MdioFrame(WRITE, 0, AUTONEG_AD_REG, 0x0021); //10  Mbps Half, 802.3

  // 100 Mbps full duplex not supported
  // MdioFrame(WRITE, 0, AUTONEG_AD_REG, 0x0101); //100 Mbps Full, 802.3

  MdioFrame(WRITE, 0, CONTROL_REG, 0x1300);

  return 0;
}

/*-------------------------------------------------------------------
*
* FUNCTION NAME: 
*
* DESCRIPTION:
*
* EXTERNAL EFFECT:
*
* PARAMETERS:
*
* RETURNS: None
*
* ASSUMPTIONS:
*
*-------------------------------------------------------------------*/
UINT16 
EthernetPHYInterruptHandler()
{		 
  // Reading registers 1 and 18 in sequence 
  // clears the transceiver interrupt

  MdioFrame(READ, 0, STATUS_REG, 0);    
  MdioFrame(READ, 0, INT_STAT_REG, 0);  
  
  return LinkTestPHY();
} /* end EthernetPHYInterruptHandler */

/*-------------------------------------------------------------------
*
* FUNCTION NAME: 
*
* DESCRIPTION:
*
* EXTERNAL EFFECT:
*
* PARAMETERS:
*
* RETURNS: None
*
* ASSUMPTIONS:
*
*-------------------------------------------------------------------*/
UINT16 
LinkTestPHY()
{
  UINT32 j;
  UINT16 FrameValue = 0;

  for (j = 0; j < 50; j++) {

    HAL_DELAY_US(100000);
  
    FrameValue = MdioFrame(READ,0,CHIP_STAT_REG,0);  
  
    if ( (FrameValue & 0x0200) != 0 ) 
      break;
  }

  FrameValue &= 0x3800;

  switch (FrameValue) {
    
    case 0x3800: return HUNDRED_FD;
    case 0x2800: return HUNDRED_HD;
    case 0x3000: return TEN_FD;
    case 0x2000: return TEN_HD;
    default:     return NOTLINKED;
  }

}

/*-------------------------------------------------------------------
*
* FUNCTION NAME: 
*
* DESCRIPTION:
*
* EXTERNAL EFFECT:
*
* PARAMETERS:
*
* RETURNS: None
*
* ASSUMPTIONS:
*
*-------------------------------------------------------------------*/
void EnablePHYinterrupt(UINT8 enable)
{
  MdioFrame(WRITE, 0, INT_EN_REG, enable?0x2:0x0);
}

/*----------------------------------------------------------------------
*
* FUNCTION NAME: 
*
* DESCRIPTION: generic READ/WRITE function of LXT970 
*              through the MDC/MDIO interface.
*
* EXTERNAL EFFECT:
*
* PARAMETERS:
*
* RETURNS: None
*
* ASSUMPTIONS:
*
*---------------------------------------------------------------------*/
UINT16 
MdioFrame(MDIORW R_W, UINT16 PhyAddr, UINT16 RegAddr, UINT32 PutData) {

  UINT16 GetData;

  *pPortDir |= MDIO_PIN_MASK;	//set to output mode
  
  MdioSend(0xFFFFFFFF,32);	//PreAmble
  MdioSend(0x1,2);              //Start Frame Delimiter
  if (R_W==READ)
    MdioSend(0x2,2);            //Read OpCode
  else
    MdioSend(0x1,2);            //Write OpCode
  
  MdioSend(PhyAddr,5);         //Send PHY transciever Address
  MdioSend(RegAddr,5);         //Send Register Address

  if (R_W==READ) {
    *pPortDir &= ~MDIO_PIN_MASK;  //set to input mode
    GetData = MdioReceive(17);    //Drive TurnAround and Data
    MdioReceive(2);
  }
  else {
    MdioSend(0x2,2);              //Drive TurnAround
    MdioSend(PutData, 16);        //Send Data
    GetData = 0;
    *pPortDir &= ~MDIO_PIN_MASK;  //set to input mode
  }

  return GetData;

}
/*----------------------------------------------------------------------
*
* FUNCTION NAME: 
*
* DESCRIPTION:  Shift out  bits of data
*
* EXTERNAL EFFECT:
*
* PARAMETERS:
*
* RETURNS: None
*
* ASSUMPTIONS:
*
*----------------------------------------------------------------------*/
void 
MdioSend(UINT32 txF, UINT16 size) {

  UINT32 dmask;
  INT_NATIVE i, j; 

  dmask = 1 << (size-1);            // msbit out first

  for (i = 0; i < size; i++) {      // for "size" bits
    
    if ( txF & dmask )  	    //output data bit high
      *pPortData |=  MDIO_PIN_MASK;
    else                            //output data bit low > 400ns
      *pPortData &= ~MDIO_PIN_MASK;
                                        // >10ns       
    *pPortData |= MDC_PIN_MASK;		// clock rise
    
    txF = (UINT32)(txF << 1);		// >160ns
    
    for (j=0; j<MDC_HOLD_TIME; j++);

    *pPortData &= ~MDC_PIN_MASK; 	// clock fall 

    for (j=0; j<MDC_HOLD_TIME; j++);

  } 
  
  return;
}
 

/*---------------------------------------------------------------------
*
* FUNCTION NAME:
*
* DESCRIPTION:  Shifts in bits of data
*
* EXTERNAL EFFECT:
*
* PARAMETERS:
*
* RETURNS:
*
* ASSUMPTIONS:
*
*---------------------------------------------------------------------*/
UINT16 
MdioReceive(UINT16 size) {

  UINT16 i,j, rxF = 0;

  for (i = 0; i < size; i++) {	 // 16 bits
	
    *pPortData |= MDC_PIN_MASK;		   // clock rise

    if ( *pPortData & MDIO_PIN_MASK )             // if read in a high bit
      rxF = ( (UINT16)(rxF << 1) | 1 );	          // shift in a one
    else                                          // if read in a low bit
      rxF = ( (UINT16)(rxF << 1) & ~(UINT16)1 );  // shift in a zero

    	
    for (j=0; j<MDC_HOLD_TIME; j++);

    *pPortData &= ~MDC_PIN_MASK; 	  // clock fall    	

    for (j=0; j<MDC_HOLD_TIME; j++);

  } 
  
  return rxF;
}

