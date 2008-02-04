/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File under the following licensing terms. 
Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	    this list of conditions and the following disclaimer. 

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution. 

    *   Neither the name of Marvell nor the names of its contributors may be 
        used to endorse or promote products derived from this software without 
        specific prior written permission. 
    
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#include "mvOs.h"
#include "mvCtrlEnvLib.h"
#include "mvEthPhy.h"

#define PHY_UPDATE_TIMEOUT	10000


static void switchVlanInit(MV_U32 ethPortNum,
						   MV_U32 switchCpuPort,
					   MV_U32 switchMaxPortsNum,
					   MV_U32 switchPortsOffset,
					   MV_U32 switchEnabledPortsMask);

/*******************************************************************************
* mvEthPhyRegRead - Read from ethernet phy register.
*
* DESCRIPTION:
*       This function reads ethernet phy register.
*
* INPUT:
*       phyAddr - Phy address.
*       regOffs - Phy register offset.
*
* OUTPUT:
*       None.
*
* RETURN:
*       16bit phy register value, or 0xffff on error
*
*******************************************************************************/
MV_STATUS mvEthPhyRegRead(MV_U32 phyAddr, MV_U32 regOffs, MV_U16 *data)
{
	MV_U32 			smiReg;
	volatile MV_U32 timeout;

	/* check parameters */
	if ((phyAddr << ETH_PHY_SMI_DEV_ADDR_OFFS) & ~ETH_PHY_SMI_DEV_ADDR_MASK)
	{
		mvOsPrintf("mvEthPhyRegRead: Err. Illigal PHY device address %d\n",
                   phyAddr);
		return MV_FAIL;
	}
	if ((regOffs <<  ETH_PHY_SMI_REG_ADDR_OFFS) & ~ETH_PHY_SMI_REG_ADDR_MASK)
	{
		mvOsPrintf("mvEthPhyRegRead: Err. Illigal PHY register offset %d\n", 
                   regOffs);
		return MV_FAIL;
	}

	timeout = ETH_PHY_TIMEOUT;
	/* wait till the SMI is not busy*/
	do
	{
		/* read smi register */
		smiReg = MV_REG_READ(ETH_PHY_SMI_REG);
		if (timeout-- == 0) 
		{
			mvOsPrintf("mvEthPhyRegRead: SMI busy timeout\n");
            		return MV_FAIL;
		}
	}while (smiReg & ETH_PHY_SMI_BUSY_MASK);

	/* fill the phy address and regiser offset and read opcode */
	smiReg = (phyAddr <<  ETH_PHY_SMI_DEV_ADDR_OFFS) | (regOffs << ETH_PHY_SMI_REG_ADDR_OFFS )|
			   ETH_PHY_SMI_OPCODE_READ;

	/* write the smi register */
	MV_REG_WRITE(ETH_PHY_SMI_REG, smiReg);

	timeout=ETH_PHY_TIMEOUT;

	/*wait till readed value is ready */
	do
	{
		/* read smi register */
		smiReg=MV_REG_READ(ETH_PHY_SMI_REG);

		if (timeout-- == 0) {
			mvOsPrintf("mvEthPhyRegRead: SMI read-valid timeout\n");
            		return MV_FAIL;
		}
	}while (!(smiReg & ETH_PHY_SMI_READ_VALID_MASK));

    	/* Wait for the data to update in the SMI register */
	for(timeout = 0 ; timeout < PHY_UPDATE_TIMEOUT ; timeout++);
	
	*data = (MV_U16)( MV_REG_READ(ETH_PHY_SMI_REG) & ETH_PHY_SMI_DATA_MASK);
	return MV_OK;
}

/*******************************************************************************
* mvEthPhyRegWrite - Write to ethernet phy register.
*
* DESCRIPTION:
*       This function write to ethernet phy register.
*
* INPUT:
*       phyAddr - Phy address.
*       regOffs - Phy register offset.
*       data    - 16bit data.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_OK if write succeed, MV_BAD_PARAM on bad parameters , MV_ERROR on error .
*		MV_TIMEOUT on timeout 
*
*******************************************************************************/
MV_STATUS mvEthPhyRegWrite(MV_U32 phyAddr, MV_U32 regOffs, MV_U16 data)
{
	MV_U32 			smiReg;
	volatile MV_U32 timeout;

	/* check parameters */
	if ((phyAddr <<  ETH_PHY_SMI_DEV_ADDR_OFFS) & ~ETH_PHY_SMI_DEV_ADDR_MASK)
	{
		mvOsPrintf("mvEthPhyRegWrite: Err. Illigal phy address \n");
		return MV_BAD_PARAM;
	}
	if ((regOffs <<  ETH_PHY_SMI_REG_ADDR_OFFS) & ~ETH_PHY_SMI_REG_ADDR_MASK)
	{
		mvOsPrintf("mvEthPhyRegWrite: Err. Illigal register offset \n");
		return MV_BAD_PARAM;
	}
	
	timeout=ETH_PHY_TIMEOUT;

	/* wait till the SMI is not busy*/
	do
	{
		/* read smi register */
		smiReg=MV_REG_READ(ETH_PHY_SMI_REG);
		if (timeout-- == 0) {
			mvOsPrintf("mvEthPhyRegWrite: SMI busy timeout\n");
            return MV_TIMEOUT;
		}
	}while (smiReg & ETH_PHY_SMI_BUSY_MASK);

	/* fill the phy address and regiser offset and write opcode and data*/
	smiReg = (data << ETH_PHY_SMI_DATA_OFFS);
	smiReg |= (phyAddr <<  ETH_PHY_SMI_DEV_ADDR_OFFS) | (regOffs << ETH_PHY_SMI_REG_ADDR_OFFS );
    smiReg &= ~ETH_PHY_SMI_OPCODE_READ;

	/* write the smi register */
	MV_REG_WRITE(ETH_PHY_SMI_REG, smiReg);

	return MV_OK;


}

/*******************************************************************************
* mvEthPhyReset - Reset ethernet Phy.
*
* DESCRIPTION:
*       This function resets a given ethernet Phy.
*
* INPUT:
*       phyAddr - Phy address.
*       timeout - in millisec        
*
* OUTPUT:
*       None.
*
* RETURN:   MV_OK       - Success
*           MV_TIMEOUT  - Timeout 
*       
*******************************************************************************/
MV_STATUS mvEthPhyReset(MV_U32 phyAddr, int timeout)
{
  	MV_U16  phyRegData;

    	/* Reset the PHY */
    	if(mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &phyRegData) != MV_OK)
		return MV_FAIL;		 
    	/* Set bit 15 to reset the PHY */
	phyRegData |= ETH_PHY_CTRL_RESET_MASK; 
	mvEthPhyRegWrite(phyAddr, ETH_PHY_CTRL_REG, phyRegData);

	/* Wait untill Auotonegotiation completed */
    	while(timeout > 0)
	{
        	mvOsSleep(100);
        	timeout -= 100;

		if( mvEthPhyRegRead(phyAddr, ETH_PHY_STATUS_REG, &phyRegData) != MV_OK)
			return MV_FAIL;		
	    	if(phyRegData & ETH_PHY_STATUS_AN_DONE_MASK)
            		return MV_OK;
   	}
	return MV_TIMEOUT;
}


/*******************************************************************************
* mvEthPhyRestartAN - Restart ethernet Phy Auto-Negotiation.
*
* DESCRIPTION:
*       This function resets a given ethernet Phy.
*
* INPUT:
*       phyAddr - Phy address.
*       timeout - in millisec        
*
* OUTPUT:
*       None.
*
* RETURN:   MV_OK       - Success
*           MV_TIMEOUT  - Timeout 
*
*******************************************************************************/
MV_STATUS mvEthPhyRestartAN(MV_U32 phyAddr, int timeout)
{
  	MV_U16  phyRegData;

    	/* Reset the PHY */
     	if(mvEthPhyRegRead (phyAddr, ETH_PHY_CTRL_REG, &phyRegData) != MV_OK)
		return MV_FAIL;
    	/* Set bit 12 to Enable autonegotiation of the PHY */
	phyRegData |= ETH_PHY_CTRL_AN_ENABLE_MASK; 
	/* Set bit 9 to Restart autonegotiation of the PHY */
	phyRegData |= ETH_PHY_CTRL_AN_RESTART_MASK; 
	mvEthPhyRegWrite(phyAddr, ETH_PHY_CTRL_REG, phyRegData);

	/* Wait untill Auotonegotiation completed */
    	while(timeout > 0)
	{
        	mvOsSleep(100);
        	timeout -= 100;

		if( mvEthPhyRegRead(phyAddr, ETH_PHY_STATUS_REG, &phyRegData) != MV_OK)
			return MV_FAIL;
	    	if(phyRegData & ETH_PHY_STATUS_AN_DONE_MASK)
            		return MV_OK;
    	}
    	return MV_TIMEOUT;
}


/*******************************************************************************
* mvEthPhyCheckLink - 
*
* DESCRIPTION:
*	check link in phy port
*       
* INPUT:
*       phyAddr - Phy address.
*
* OUTPUT:
*       None.
*
* RETURN:   MV_TRUE if link is up, MV_FALSE if down
*
*******************************************************************************/
MV_BOOL mvEthPhyCheckLink( MV_U32 phyAddr )
{
	MV_U16 val_st, val_ctrl, val_spec_st;

	/* read status reg */
	if( mvEthPhyRegRead( phyAddr, ETH_PHY_STATUS_REG, &val_st) != MV_OK )
		return MV_FALSE;

	/* read control reg */
	if( mvEthPhyRegRead( phyAddr, ETH_PHY_CTRL_REG, &val_ctrl) != MV_OK )
		return MV_FALSE;

	/* read special status reg */
	if( mvEthPhyRegRead( phyAddr, ETH_PHY_SPEC_STATUS_REG, &val_spec_st) != MV_OK )
		return MV_FALSE;

	if(val_ctrl & ETH_PHY_CTRL_AN_ENABLE_MASK)
	{
		if(val_st & ETH_PHY_STATUS_AN_DONE_MASK)
			return MV_TRUE;
		else
			return MV_FALSE;
	}
	else
	{
		if(val_spec_st & ETH_PHY_SPEC_STATUS_LINK_MASK)
			return MV_TRUE;
	}
	return MV_FALSE;
}

/*******************************************************************************
* mvEthPhyPrintStatus - 
*
* DESCRIPTION:
*	print port Speed, Duplex, Auto-negotiation, Link.
*       
* INPUT:
*       phyAddr - Phy address.
*
* OUTPUT:
*       None.
*
* RETURN:   16bit phy register value, or 0xffff on error
*
*******************************************************************************/
MV_STATUS	mvEthPhyPrintStatus( MV_U32 phyAddr )
{
	MV_U16 val;

	/* read control reg */ 
	if( mvEthPhyRegRead( phyAddr, ETH_PHY_CTRL_REG, &val) != MV_OK )
		return MV_ERROR;

	if( val & ETH_PHY_CTRL_AN_ENABLE_MASK )
		mvOsOutput( "Auto negotiation: Enabled\n" );
	else
		mvOsOutput( "Auto negotiation: Disabled\n" );


	/* read specific status reg */ 
	if( mvEthPhyRegRead( phyAddr, ETH_PHY_SPEC_STATUS_REG, &val) != MV_OK )
		return MV_ERROR;

	switch (val & ETH_PHY_SPEC_STATUS_SPEED_MASK)
	{
		case ETH_PHY_SPEC_STATUS_SPEED_1000MBPS:
			mvOsOutput( "Speed: 1000 Mbps\n" );
			break;
		case ETH_PHY_SPEC_STATUS_SPEED_100MBPS:
			mvOsOutput( "Speed: 100 Mbps\n" );
			break;
		case ETH_PHY_SPEC_STATUS_SPEED_10MBPS:
			mvOsOutput( "Speed: 10 Mbps\n" );
		default:
			mvOsOutput( "Speed: Uknown\n" );
			break;

	}

	if( val & ETH_PHY_SPEC_STATUS_DUPLEX_MASK )
		mvOsOutput( "Duplex: Full\n" );
	else
		mvOsOutput( "Duplex: Half\n" );
 

	if( val & ETH_PHY_SPEC_STATUS_LINK_MASK )
		mvOsOutput("Link: up\n");
	else
		mvOsOutput("Link: down\n");

	return MV_OK;
}

/*******************************************************************************
* mvEthE1111PhyBasicInit - 
*
* DESCRIPTION:
*	Do a basic Init to the Phy , including reset
*       
* INPUT:
*       ethPortNum - Ethernet port number
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
MV_VOID		mvEthE1111PhyBasicInit(MV_U32 ethPortNum)
{
	MV_U16 reg;


	/* Phy recv and tx delay */
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),20,&reg);
	reg |= BIT1 | BIT7;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),20,reg);

	/* Leds link and activity*/
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),24,0x4111);

	/* reset the phy */
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),0,&reg);
	reg |= BIT15;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),0,reg);

	if ( RD_88F5181_POS_NAS == mvBoardIdGet())
	{
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),0x18,0x4151);
	}

}


MV_VOID		mvEthE1116PhyBasicInit(MV_U32 ethPortNum)
{

	MV_U16 reg;

	/* Access Page 3*/
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,0x3);



	/* Leds link and activity*/
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),16,&reg);
	reg &= ~0xf;
	reg	|= 0x1;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),16,reg);

}

/*******************************************************************************
* mvEthE1011PhyBasicInit - 
*
* DESCRIPTION:
*	Do a basic Init to the Phy , including reset
*       
* INPUT:
*       ethPortNum - Ethernet port number
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
MV_VOID		mvEthE1011PhyBasicInit(MV_U32 ethPortNum)
{
	MV_U16 reg;

	/* Phy recv and tx delay */
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),20,&reg);
	reg &= ~(BIT1 | BIT7);
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),20,reg);

	/* Leds link and activity*/
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),24,0x4111);

	/* reset the phy */
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),0, &reg);
	reg |= BIT15;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),0,reg);

}

/*******************************************************************************
* mvEthE6065_61PhyBasicInit - 
*
* DESCRIPTION:
*	Do a basic Init to the Phy , including reset
*       
* INPUT:
*       ethPortNum - Ethernet port number
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
MV_VOID		mvEthE6065_61PhyBasicInit(MV_U32 ethPortNum)
{
	switchVlanInit(ethPortNum,
			   MV_E6065_CPU_PORT,
			   MV_E6065_MAX_PORTS_NUM,
			   MV_E6065_PORTS_OFFSET,
			   MV_E6065_ENABLED_PORTS);
}

/*******************************************************************************
* mvEthE6063PhyBasicInit - 
*
* DESCRIPTION:
*	Do a basic Init to the Phy , including reset
*       
* INPUT:
*       ethPortNum - Ethernet port number
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
MV_VOID		mvEthE6063PhyBasicInit(MV_U32 ethPortNum)
{
	switchVlanInit(ethPortNum,
			   MV_E6063_CPU_PORT,
			   MV_E6063_MAX_PORTS_NUM,
			   MV_E6063_PORTS_OFFSET,
			   MV_E6063_ENABLED_PORTS);
}

/*******************************************************************************
* mvEthE6131PhyBasicInit - 
*
* DESCRIPTION:
*	Do a basic Init to the Phy , including reset
*       
* INPUT:
*       ethPortNum - Ethernet port number
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
MV_VOID		mvEthE6131PhyBasicInit(MV_U32 ethPortNum)
{

	MV_U16 reg;

	/*Enable Phy power up*/
	mvEthPhyRegWrite (0,0,0x9140);
	mvEthPhyRegWrite (1,0,0x9140);
	mvEthPhyRegWrite (2,0,0x9140);


	/*Enable PPU*/
	mvEthPhyRegWrite (0x1b,4,0x4080);


	/*Enable Phy detection*/
	mvEthPhyRegRead (0x13,0,&reg);
	reg &= ~(1<<12);
	mvEthPhyRegWrite (0x13,0,reg);

	mvOsDelay(100);
	mvEthPhyRegWrite (0x13,1,0x33);
    
	switchVlanInit(ethPortNum,
				   MV_E6131_CPU_PORT,
			   MV_E6131_MAX_PORTS_NUM,
			   MV_E6131_PORTS_OFFSET,
			   MV_E6131_ENABLED_PORTS);

}


static void switchVlanInit(MV_U32 ethPortNum,
						   MV_U32 switchCpuPort,
					   MV_U32 switchMaxPortsNum,
					   MV_U32 switchPortsOffset,
					   MV_U32 switchEnabledPortsMask)
{
    MV_U32 prt;
	MV_U16 reg;

	/* be sure all ports are disabled */
    for(prt=0; prt < switchMaxPortsNum; prt++)
	{
		mvEthPhyRegRead (mvBoardPhyAddrGet(ethPortNum)+ MV_SWITCH_PORT_OFFSET(prt),
							  MV_SWITCH_PORT_CONTROL_REG,&reg);
		reg &= ~0x3;
		mvEthPhyRegWrite (mvBoardPhyAddrGet(ethPortNum)+ MV_SWITCH_PORT_OFFSET(prt),
							  MV_SWITCH_PORT_CONTROL_REG,reg);

	}

	/* Set CPU port VID to 0x1 */
	mvEthPhyRegRead (mvBoardPhyAddrGet(ethPortNum)+ MV_SWITCH_PORT_OFFSET(switchCpuPort),
						  MV_SWITCH_PORT_VID_REG,&reg);
	reg &= ~0xfff;
	reg |= 0x1;
	mvEthPhyRegWrite (mvBoardPhyAddrGet(ethPortNum)+ MV_SWITCH_PORT_OFFSET(switchCpuPort),
						  MV_SWITCH_PORT_VID_REG,reg);


	/* Setting  Port default priority for all ports to zero */
    for(prt=0; prt < switchMaxPortsNum; prt++)
	{

		mvEthPhyRegRead (mvBoardPhyAddrGet(ethPortNum)+ MV_SWITCH_PORT_OFFSET(prt),
							  MV_SWITCH_PORT_VID_REG,&reg);
		reg &= ~0xc000;
		mvEthPhyRegWrite (mvBoardPhyAddrGet(ethPortNum)+ MV_SWITCH_PORT_OFFSET(prt),
							  MV_SWITCH_PORT_VID_REG,reg);
	}

	/* Setting VID and VID map for all ports except CPU port */
    for(prt=0; prt < switchMaxPortsNum; prt++)
	{

		/* only for enabled ports */
        if ((1 << prt)& switchEnabledPortsMask)
		{
			/* skip CPU port */
			if (prt== switchCpuPort) continue;

			/* 
			*  set Ports VLAN Mapping.
			*	port prt <--> MV_SWITCH_CPU_PORT VLAN #prt+1.
			*/
	
			mvEthPhyRegRead (mvBoardPhyAddrGet(ethPortNum)+ MV_SWITCH_PORT_OFFSET(prt),
								  MV_SWITCH_PORT_VID_REG,&reg);
			reg &= ~0x0fff;
			reg |= (prt+1);
			mvEthPhyRegWrite (mvBoardPhyAddrGet(ethPortNum)+ MV_SWITCH_PORT_OFFSET(prt),
								  MV_SWITCH_PORT_VID_REG,reg);
	
	
			/* Set Vlan map table for all ports to send only to MV_SWITCH_CPU_PORT */
			mvEthPhyRegRead (mvBoardPhyAddrGet(ethPortNum)+ MV_SWITCH_PORT_OFFSET(prt),
								  MV_SWITCH_PORT_VMAP_REG,&reg);
			reg &= ~((1 << switchMaxPortsNum) - 1);
			reg |= (1 << switchCpuPort);
			mvEthPhyRegWrite (mvBoardPhyAddrGet(ethPortNum)+ MV_SWITCH_PORT_OFFSET(prt),
								  MV_SWITCH_PORT_VMAP_REG,reg);
		}

	}


	/* Set Vlan map table for MV_SWITCH_CPU_PORT to see all ports*/
	mvEthPhyRegRead (mvBoardPhyAddrGet(ethPortNum)+ MV_SWITCH_PORT_OFFSET(switchCpuPort),
						  MV_SWITCH_PORT_VMAP_REG,&reg);
	reg &= ~((1 << switchMaxPortsNum) - 1);
	reg |= switchEnabledPortsMask & ~(1 << switchCpuPort);
	mvEthPhyRegWrite (mvBoardPhyAddrGet(ethPortNum)+ MV_SWITCH_PORT_OFFSET(switchCpuPort),
						  MV_SWITCH_PORT_VMAP_REG,reg);


    /*enable only appropriate ports to forwarding mode - and disable the others*/
    for(prt=0; prt < switchMaxPortsNum; prt++)
	{

        if ((1 << prt)& switchEnabledPortsMask)
		{
			mvEthPhyRegRead (mvBoardPhyAddrGet(ethPortNum)+ MV_SWITCH_PORT_OFFSET(prt),
								  MV_SWITCH_PORT_CONTROL_REG,&reg);
			reg |= 0x3;
			mvEthPhyRegWrite (mvBoardPhyAddrGet(ethPortNum)+ MV_SWITCH_PORT_OFFSET(prt),
								  MV_SWITCH_PORT_CONTROL_REG,reg);

		}
		else
		{
			/* Disable port 6 */
			mvEthPhyRegRead (mvBoardPhyAddrGet(ethPortNum)+ MV_SWITCH_PORT_OFFSET(prt),
								  MV_SWITCH_PORT_CONTROL_REG,&reg);
			reg &= ~0x3;
			mvEthPhyRegWrite (mvBoardPhyAddrGet(ethPortNum)+ MV_SWITCH_PORT_OFFSET(prt),
								  MV_SWITCH_PORT_CONTROL_REG,reg);

		}
	}


	return;

}






