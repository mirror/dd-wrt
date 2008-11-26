#include <Copyright.h>

/********************************************************************************
* gtPhyInt.h
*
* DESCRIPTION:
* API definitions for PHY interrupt handling
*
* DEPENDENCIES:
* None.
*
* FILE REVISION NUMBER:
* $Revision: 10 $
*******************************************************************************/

#include <msApi.h>
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>
#include <gtDrvConfig.h>

/*******************************************************************************
* gprtPhyIntEnable
*
* DESCRIPTION:
* Enable/Disable one PHY Interrupt
* This register determines whether the INT# pin is asserted when an interrupt
* event occurs. When an interrupt occurs, the corresponding bit is set and
* remains set until register 19 is read via the SMI. When interrupt enable
* bits are not set in register 18, interrupt status bits in register 19 are
* still set when the corresponding interrupt events occur. However, the INT#
* is not asserted.
*
* INPUTS:
* port    - logical port number
* intType - the type of interrupt to enable/disable. any combination of
*			GT_SPEED_CHANGED,
*			GT_DUPLEX_CHANGED,
*			GT_PAGE_RECEIVED,
*			GT_AUTO_NEG_COMPLETED,
*			GT_LINK_STATUS_CHANGED,
*			GT_SYMBOL_ERROR,
*			GT_FALSE_CARRIER,
*			GT_FIFO_FLOW,
*			GT_CROSSOVER_CHANGED,	( Copper only )
*			GT_DOWNSHIFT_DETECT,	( for 1000M Copper only )
*			GT_ENERGY_DETECT,		( for 1000M Copper only )
*			GT_POLARITY_CHANGED, and ( Copper only )
*			GT_JABBER				(Copper only )
*
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* 88E3081 data sheet register 18
*
*******************************************************************************/


GT_STATUS gprtPhyIntEnable
(
IN GT_QD_DEV    *dev,
IN GT_LPORT	port,
IN GT_U16	intType
)
{
    GT_STATUS       retVal;
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtPhyIntEnable Called.\n"));

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	/* check if the port is configurable */
	if(!IS_CONFIGURABLE_PHY(dev,hwPort))
	{
		return GT_NOT_SUPPORTED;
	}

	if((IS_IN_DEV_GROUP(dev,DEV_SERDES_CORE)) && (hwPort > 3))
	{
		GT_GET_SERDES_PORT(dev,&hwPort);
		if(hwPort >= dev->maxPhyNum)
		{
			return GT_NOT_SUPPORTED;
		}
	}

	retVal = hwWritePhyReg(dev,hwPort, QD_PHY_INT_ENABLE_REG, intType);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    return retVal;

}

/*******************************************************************************
* gprtGetPhyIntStatus
*
* DESCRIPTION:
* Check to see if a specific type of interrupt occured
*
* INPUTS:
* port - logical port number
* intType - the type of interrupt which causes an interrupt.
*			any combination of
*			GT_SPEED_CHANGED,
*			GT_DUPLEX_CHANGED,
*			GT_PAGE_RECEIVED,
*			GT_AUTO_NEG_COMPLETED,
*			GT_LINK_STATUS_CHANGED,
*			GT_SYMBOL_ERROR,
*			GT_FALSE_CARRIER,
*			GT_FIFO_FLOW,
*			GT_CROSSOVER_CHANGED,	( Copper only )
*			GT_DOWNSHIFT_DETECT,	( for 1000M Copper only )
*			GT_ENERGY_DETECT,		( for 1000M Copper only )
*			GT_POLARITY_CHANGED, and ( Copper only )
*			GT_JABBER				(Copper only )
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* 88E3081 data sheet register 19
*
*******************************************************************************/

GT_STATUS gprtGetPhyIntStatus
(
IN   GT_QD_DEV  *dev,
IN   GT_LPORT   port,
OUT  GT_U16*    intType
)
{
    GT_STATUS       retVal;
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetPhyIntStatus Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the port is configurable */
	if(!IS_CONFIGURABLE_PHY(dev,hwPort))
	{
		return GT_NOT_SUPPORTED;
	}

	if((IS_IN_DEV_GROUP(dev,DEV_SERDES_CORE)) && (hwPort > 3))
	{
		GT_GET_SERDES_PORT(dev,&hwPort);
		if(hwPort >= dev->maxPhyNum)
		{
			return GT_NOT_SUPPORTED;
		}
	}

	retVal = hwReadPhyReg(dev,hwPort, QD_PHY_INT_STATUS_REG, intType);
    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    return retVal;
}

/*******************************************************************************
* gprtGetPhyIntPortSummary
*
* DESCRIPTION:
* Lists the ports that have active interrupts. It provides a quick way to
* isolate the interrupt so that the MAC or switch does not have to poll the
* interrupt status register (19) for all ports. Reading this register does not
* de-assert the INT# pin
*
* INPUTS:
* none
*
* OUTPUTS:
* GT_U8 *intPortMask - bit Mask with the bits set for the corresponding
* phys with active interrupt. E.g., the bit number 0 and 2 are set when
* port number 0 and 2 have active interrupt
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* 88E3081 data sheet register 20
*
*******************************************************************************/

GT_STATUS gprtGetPhyIntPortSummary
(
IN  GT_QD_DEV  *dev,
OUT GT_U16     *intPortMask
)
{
    GT_STATUS       retVal;
    GT_U8           hwPort;         /* the physical port number     */
	GT_U16			portVec;

    DBG_INFO(("gprtGetPhyIntPortSummary Called.\n"));

    /* translate LPORT 0 to hardware port */
    hwPort = GT_LPORT_2_PORT(0);

    *intPortMask=0;

	if (IS_IN_DEV_GROUP(dev,DEV_INTERNAL_GPHY))
	{
	    /* get the interrupt port summary from global register */
	    retVal = hwGetGlobal2RegField(dev,QD_REG_PHYINT_SOURCE,0,dev->maxPorts,&portVec);
		GT_GIG_PHY_INT_MASK(dev,portVec);
		*intPortMask = GT_PORTVEC_2_LPORTVEC(portVec);
	}
	else
	{
	    /* get the interrupt port summary from phy */
        /* Changed by Shadi.    */
        /* For 6161 devices the interrupt summary is at address 23 and not 20.*/
        if ((dev->deviceId == GT_88E6161) || (dev->deviceId == GT_88E6165))
            retVal = hwReadPhyReg(dev,hwPort, 23, &portVec);
        else
		retVal = hwReadPhyReg(dev,hwPort, QD_PHY_INT_PORT_SUMMARY_REG, &portVec);
		*intPortMask = GT_PORTVEC_2_LPORTVEC(portVec);
	}

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

	return retVal;

}

