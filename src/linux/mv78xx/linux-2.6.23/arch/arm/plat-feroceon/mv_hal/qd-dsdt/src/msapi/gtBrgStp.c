#include <Copyright.h>

/********************************************************************************
* gtBrgStp.c
*
* DESCRIPTION:
*       API definitions to handle port spanning tree state.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*******************************************************************************/

#include <msApi.h>
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>

static GT_STATUS enhancedBPDUSet(GT_QD_DEV *dev,GT_BOOL en)
{
    GT_STATUS       retVal = GT_OK; /* Functions return value.      */
	GT_U16			enBits;

	/* If disable, reset the BPDU bit(bit0) from Rsvd2CpuEnables register */
   	if(en == GT_FALSE)
	{
		if((retVal = gsysGetRsvd2CpuEnables(dev,&enBits)) != GT_OK)
		{
	        DBG_INFO(("gsysGetRsvd2CpuEnables failed.\n"));
			return retVal;
		}
		enBits &= ~0x1;

		if((retVal = gsysSetRsvd2CpuEnables(dev,enBits)) != GT_OK)
		{
    	    DBG_INFO(("gsysSetRsvd2CpuEnables failed.\n"));
			return retVal;
		}

		return retVal;
	}

	/*
		If enable,
		1) Set MGMT Pri bits,
		2) Set BPDU bit(bit0) from Rsvd2CpuEnables register,
		3) Enable Rsvd2Cpu
	*/
	if((retVal = gsysSetMGMTPri(dev,7)) != GT_OK)
	{
        DBG_INFO(("gsysSetMGMTPri failed.\n"));
		return retVal;
	}

	if((retVal = gsysGetRsvd2CpuEnables(dev,&enBits)) != GT_OK)
	{
        DBG_INFO(("gsysGetRsvd2CpuEnables failed.\n"));
		return retVal;
	}
	if((retVal = gsysSetRsvd2CpuEnables(dev,enBits|0x1)) != GT_OK)
	{
        DBG_INFO(("gsysSetRsvd2CpuEnables failed.\n"));
		return retVal;
	}

	if((retVal = gsysSetRsvd2Cpu(dev,GT_TRUE)) != GT_OK)
	{
        DBG_INFO(("gsysSetRsvd2Cpu failed.\n"));
		return retVal;
	}

	return retVal;
}


/*******************************************************************************
* gstpSetMode
*
* DESCRIPTION:
*       This routine Enable the Spanning tree.
*
* INPUTS:
*       en - GT_TRUE for enable, GT_FALSE for disable.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       when enabled, this function sets all port to blocking state, and inserts
*       the BPDU MAC into the ATU to be captured to CPU, on disable all port are
*       being modified to be in forwarding state.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstpSetMode
(
    IN GT_QD_DEV *dev,
    IN GT_BOOL  en
)
{
    GT_STATUS       retVal = GT_OK; /* Functions return value.      */
    GT_ATU_ENTRY        atuEntry;   /* The ATU entry data to be set */
    GT_U32          i, dbNum;

    DBG_INFO(("gstpSetMode Called.\n"));
    if(dev->deviceId == GT_88E6051)
    {
        DBG_INFO(("Failed.\n"));
        return GT_FAIL;
    }

    if((en == GT_TRUE) && (dev->stpMode == 1))
    {
        DBG_INFO(("OK.\n"));
        return GT_OK;
    }

	switch(dev->deviceId)
	{
		case GT_88E6051:
		case GT_88E6052:
			dbNum = 1;
			break;
		case GT_FF_HG:
		case GT_FF_EG:
		case GT_88E6021:
		case GT_88E6060:
		case GT_88E6031:
		case GT_88E6061:
		case GT_88E6063:
		case GT_FH_VPN:
		case GT_88E6083:
		case GT_88E6153:
		case GT_88E6181:
		case GT_88E6183:
		case GT_88E6093:
			dbNum = 16;
			break;
		case GT_88E6035:
		case GT_88E6055:
		case GT_88E6065:
			dbNum = 64;
			break;
		case GT_88E6092:
		case GT_88E6095:
		case GT_88E6045:
		case GT_88E6152:
		case GT_88E6155:
		case GT_88E6182:
		case GT_88E6185:
		case GT_88E6121:
		case GT_88E6122:
		case GT_88E6131:
		case GT_88E6108:
		case GT_88E6046:
		case GT_88E6047:
		case GT_88E6096:
		case GT_88E6097:
		case GT_88E6165:
			/*
				No need to add BPDU entry to the fdb table.
				Set or reset bit 0 of Rsvd2Cpu register.
			*/
			dbNum = 0;
			retVal = enhancedBPDUSet(dev,en);
			break;
		default:
			dbNum = 16;
			break;
	}

	for (i=0; i<dbNum; i++)
	{
	    /* Set the Atu entry parameters.    */
    	atuEntry.macAddr.arEther[0] = 0x01;
	    atuEntry.macAddr.arEther[1] = 0x80;
    	atuEntry.macAddr.arEther[2] = 0xC2;
	    atuEntry.macAddr.arEther[3] = 0x00;
    	atuEntry.macAddr.arEther[4] = 0x00;
	    atuEntry.macAddr.arEther[5] = 0x00;
    	atuEntry.portVec = GT_LPORTVEC_2_PORTVEC((1<<dev->cpuPortNum));
		if(IS_IN_DEV_GROUP(dev,DEV_ATU_EXT_PRI))
		{
			if(IS_IN_DEV_GROUP(dev,DEV_FQPRI_IN_TABLE))
			{
				atuEntry.exPrio.useMacFPri = GT_TRUE;
				atuEntry.exPrio.macFPri = 7;
			}
			else
			{
				atuEntry.exPrio.useMacFPri = 0;
				atuEntry.exPrio.macFPri = 0;
			}
			atuEntry.exPrio.macQPri = 3;
		    atuEntry.prio    = 0;
		}
		else
		{
		    atuEntry.prio    = 3;
			atuEntry.exPrio.useMacFPri = 0;
			atuEntry.exPrio.macFPri = 0;
			atuEntry.exPrio.macQPri = 0;
		}
		atuEntry.DBNum = (GT_U8)i;
	    atuEntry.entryState.mcEntryState = GT_MC_PRIO_MGM_STATIC;

    	if(en == GT_TRUE)
	    {
    	    retVal = gfdbAddMacEntry(dev,&atuEntry);
	    }
    	else
		{
			if(dev->stpMode == 0)
				break;
        	retVal = gfdbDelAtuEntry(dev,&atuEntry);
		}

		if (retVal != GT_OK)
			break;
	}

    if(retVal == GT_OK)
	{
	    if(en == GT_TRUE)
    	    dev->stpMode = 1;
	    else
    	    dev->stpMode = 2;
        DBG_INFO(("OK.\n"));
	}
    else
	{
   	    dev->stpMode = 0;
        DBG_INFO(("Failed.\n"));
	}


    return retVal;
}



/*******************************************************************************
* gstpSetPortState
*
* DESCRIPTION:
*       This routine set the port state.
*
* INPUTS:
*       port  - the logical port number.
*       state - the port state to set.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstpSetPortState
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT           port,
    IN GT_PORT_STP_STATE  state
)
{
    GT_U8           phyPort;        /* Physical port                */
    GT_U16          data;           /* Data to write to register.   */
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gstpSetPortState Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);
    data    = state;

    /* Set the port state bits.             */
    retVal= hwSetPortRegField(dev,phyPort, QD_REG_PORT_CONTROL,0,2,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gstpGetPortState
*
* DESCRIPTION:
*       This routine returns the port state.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       state - the current port state.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstpGetPortState
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT           port,
    OUT GT_PORT_STP_STATE  *state
)
{
    GT_U8           phyPort;        /* Physical port                */
    GT_U16          data;           /* Data read from register.     */
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gstpGetPortState Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* Get the port state bits.             */
    retVal = hwGetPortRegField(dev,phyPort, QD_REG_PORT_CONTROL,0,2,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    *state = data & 0x3;
    DBG_INFO(("OK.\n"));
    return GT_OK;
}
