#include <Copyright.h>

/*******************************************************************************
* gtCCPVT.c
*
* DESCRIPTION:
*       API definitions for Cross Chip Port Vlan Data Table
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*******************************************************************************/

#include <msApi.h>
#include <gtSem.h>
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>

/****************************************************************************/
/* Cross Chip Port Vlan operation function declaration.                                    */
/****************************************************************************/
static GT_STATUS pvtOperationPerform
(
    IN   GT_QD_DEV 			*dev,
    IN   GT_PVT_OPERATION	pvtOp,
    INOUT GT_PVT_OP_DATA	*opData
);


/*******************************************************************************
* gpvtInitialize
*
* DESCRIPTION:
*       This routine initializes the PVT Table to all one's (initial state)
*
* INPUTS:
*		None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpvtInitialize
(
	IN  GT_QD_DEV 	*dev
)
{
	GT_STATUS       	retVal;
	GT_PVT_OPERATION	op;

	DBG_INFO(("gpvtInitialize Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_CROSS_CHIP_PORT_VLAN))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	/* Program Tuning register */
	op = PVT_INITIALIZE;
	retVal = pvtOperationPerform(dev,op,NULL);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("Failed (pvtOperationPerform returned GT_FAIL).\n"));
    	return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gpvtWritePVTData
*
* DESCRIPTION:
*       This routine write Cross Chip Port Vlan Data.
*		Cross chip Port VLAN Data used as a bit mask to limit where cross chip
*		frames can egress (in chip Port VLANs are masked using gvlnSetPortVlanPorts
*		API). Cross chip frames are Forward frames that ingress a DSA or Ether 
*		Type DSA port (see gprtSetFrameMode API). Bit 0 is a mask for port 0, 
*		bit 1 for port 1, etc. When a port's mask bit is one, frames are allowed 
*		to egress that port on this device. When a port's mask bit is zero,
*		frames are not allowed to egress that port on this device.
*
*		The Cross Chip Port VLAN Table is accessed by ingressing frames based
*		upon the original source port of the frame using the Forward frame's DSA tag
*		fields Src_Dev, Src_Port/Src_Trunk and Src_Is_Trunk. The 1 entry of the 512
*		that is accessed by the frame is:
*			If 5 Bit Port (in Global 2, offset 0x1D) = 0:
*				If Src_Is_Trunk = 0   Src_Dev[4:0], Src_Port[3:0]119
*				If Src_Is_Trunk = 1   Device Number (global offset 0x1C), Src_Trunk[3:0]
*			If 5 Bit Port (in Global 2, offset 0x1D) = 1:
*				If Src_Is_Trunk = 0   Src_Dev[3:0], Src_Port[4:0]120
*				If Src_Is_Trunk = 1   Device Number[3:0], Src_Trunk[4:0]
*
*		Cross chip port VLANs with Trunks are supported in the table where this
*		device's entries would be stored (defined by this device's Device Number).
*		This portion of the table is available for Trunk entries because this device's
*		port VLAN mappings to ports inside this device are masked by the port's
*		VLAN Table (see gvlnSetPortVlanPorts API).
*
*
* INPUTS:
*		pvtPointer - pointer to the desired entry of PVT (0 ~ 511)
*		pvtData    - Cross Chip Port Vlan Data
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpvtWritePVTData
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_U32		pvtPointer,
	IN  GT_U32		pvtData
)
{
	GT_STATUS       	retVal;
	GT_PVT_OPERATION	op;
	GT_PVT_OP_DATA		opData;

	DBG_INFO(("gpvtWritePVTData Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_CROSS_CHIP_PORT_VLAN))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* check if the given pointer is valid */
	if (pvtPointer > 0x1FF)
    {
        DBG_INFO(("GT_BAD_PARAM\n"));
		return GT_BAD_PARAM;
    }

    /* check if the given pvtData is valid */
	if (pvtData >= (1 << dev->maxPorts))
    {
        DBG_INFO(("GT_BAD_PARAM\n"));
		return GT_BAD_PARAM;
    }

	/* Program Tuning register */
	op = PVT_WRITE;
	opData.pvtAddr = pvtPointer;

	if((opData.pvtData = GT_LPORTVEC_2_PORTVEC(pvtData)) == GT_INVALID_PORT_VEC)
	{
		DBG_INFO(("GT_BAD_PARAM\n"));
		return GT_BAD_PARAM;
	}


	retVal = pvtOperationPerform(dev,op,&opData);
	if(retVal != GT_OK)
	{
		DBG_INFO(("Failed (pvtOperationPerform returned GT_FAIL).\n"));
		return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gpvtReadPVTData
*
* DESCRIPTION:
*       This routine reads Cross Chip Port Vlan Data.
*		Cross chip Port VLAN Data used as a bit mask to limit where cross chip
*		frames can egress (in chip Port VLANs are masked using gvlnSetPortVlanPorts
*		API). Cross chip frames are Forward frames that ingress a DSA or Ether 
*		Type DSA port (see gprtSetFrameMode API). Bit 0 is a mask for port 0, 
*		bit 1 for port 1, etc. When a port's mask bit is one, frames are allowed 
*		to egress that port on this device. When a port's mask bit is zero,
*		frames are not allowed to egress that port on this device.
*
*		The Cross Chip Port VLAN Table is accessed by ingressing frames based
*		upon the original source port of the frame using the Forward frame's DSA tag
*		fields Src_Dev, Src_Port/Src_Trunk and Src_Is_Trunk. The 1 entry of the 512
*		that is accessed by the frame is:
*			If 5 Bit Port (in Global 2, offset 0x1D) = 0:
*				If Src_Is_Trunk = 0   Src_Dev[4:0], Src_Port[3:0]119
*				If Src_Is_Trunk = 1   Device Number (global offset 0x1C), Src_Trunk[3:0]
*			If 5 Bit Port (in Global 2, offset 0x1D) = 1:
*				If Src_Is_Trunk = 0   Src_Dev[3:0], Src_Port[4:0]120
*				If Src_Is_Trunk = 1   Device Number[3:0], Src_Trunk[4:0]
*
*		Cross chip port VLANs with Trunks are supported in the table where this
*		device's entries would be stored (defined by this device's Device Number).
*		This portion of the table is available for Trunk entries because this device's
*		port VLAN mappings to ports inside this device are masked by the port's
*		VLAN Table (see gvlnSetPortVlanPorts API).
*
*
* INPUTS:
*		pvtPointer - pointer to the desired entry of PVT (0 ~ 511)
*
* OUTPUTS:
*		pvtData    - Cross Chip Port Vlan Data
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpvtReadPVTData
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_U32		pvtPointer,
	OUT GT_U32		*pvtData
)
{
	GT_STATUS       	retVal;
	GT_PVT_OPERATION	op;
	GT_PVT_OP_DATA		opData;

	DBG_INFO(("gpvtReadPVTData Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_CROSS_CHIP_PORT_VLAN))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* check if the given pointer is valid */
	if (pvtPointer > 0x1FF)
    {
        DBG_INFO(("GT_BAD_PARAM\n"));
		return GT_BAD_PARAM;
    }

	/* Program Tuning register */
	op = PVT_READ;
	opData.pvtAddr = pvtPointer;
	retVal = pvtOperationPerform(dev,op,&opData);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("Failed (pvtOperationPerform returned GT_FAIL).\n"));
    	return retVal;
	}

	opData.pvtData &= (1 << dev->maxPorts) - 1;
	*pvtData = GT_PORTVEC_2_LPORTVEC(opData.pvtData);

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/****************************************************************************/
/* Internal functions.                                                  */
/****************************************************************************/


/*******************************************************************************
* pvtOperationPerform
*
* DESCRIPTION:
*       This function accesses PVT Table
*
* INPUTS:
*       pvtOp   - The pvt operation
*       pvtData - address and data to be written into PVT
*
* OUTPUTS:
*       pvtData - data read from PVT pointed by address
*
* RETURNS:
*       GT_OK on success,
*       GT_FAIL otherwise.
*
* COMMENTS:
*
*******************************************************************************/
static GT_STATUS pvtOperationPerform
(
    IN    GT_QD_DEV           *dev,
    IN    GT_PVT_OPERATION   pvtOp,
    INOUT GT_PVT_OP_DATA     *opData
)
{
    GT_STATUS       retVal;	/* Functions return value */
    GT_U16          data; 	/* temporary Data storage */

    gtSemTake(dev,dev->tblRegsSem,OS_WAIT_FOREVER);

    /* Wait until the pvt in ready. */
    data = 1;
    while(data == 1)
    {
        retVal = hwGetGlobal2RegField(dev,QD_REG_PVT_ADDR,15,1,&data);
        if(retVal != GT_OK)
        {
            gtSemGive(dev,dev->tblRegsSem);
            return retVal;
        }
    }

    /* Set the PVT Operation register */
	switch (pvtOp)
	{
		case PVT_INITIALIZE:
			data = (1 << 15) | (pvtOp << 12);
			retVal = hwWriteGlobal2Reg(dev,QD_REG_PVT_ADDR,data);
	        if(retVal != GT_OK)
    	    {
        	    gtSemGive(dev,dev->tblRegsSem);
            	return retVal;
	        }
			break;

		case PVT_WRITE:
			data = (GT_U16)opData->pvtData;
			retVal = hwWriteGlobal2Reg(dev,QD_REG_PVT_DATA,data);
	        if(retVal != GT_OK)
    	    {
        	    gtSemGive(dev,dev->tblRegsSem);
            	return retVal;
	        }

			data = (1 << 15) | (pvtOp << 12) | opData->pvtAddr;
			retVal = hwWriteGlobal2Reg(dev,QD_REG_PVT_ADDR,data);
	        if(retVal != GT_OK)
    	    {
        	    gtSemGive(dev,dev->tblRegsSem);
            	return retVal;
	        }
			break;

		case PVT_READ:
			data = (1 << 15) | (pvtOp << 12) | opData->pvtAddr;
			retVal = hwWriteGlobal2Reg(dev,QD_REG_PVT_ADDR,data);
	        if(retVal != GT_OK)
    	    {
        	    gtSemGive(dev,dev->tblRegsSem);
            	return retVal;
	        }

		    data = 1;
		    while(data == 1)
		    {
		        retVal = hwGetGlobal2RegField(dev,QD_REG_PVT_ADDR,15,1,&data);
		        if(retVal != GT_OK)
		        {
		            gtSemGive(dev,dev->tblRegsSem);
		            return retVal;
        		}
		    }

			retVal = hwReadGlobal2Reg(dev,QD_REG_PVT_DATA,&data);
			opData->pvtData = (GT_U32)data;
	        if(retVal != GT_OK)
    	    {
        	    gtSemGive(dev,dev->tblRegsSem);
            	return retVal;
	        }
	
			break;

		default:
			
			gtSemGive(dev,dev->tblRegsSem);
			return GT_FAIL;
	}

    gtSemGive(dev,dev->tblRegsSem);
    return retVal;
}

