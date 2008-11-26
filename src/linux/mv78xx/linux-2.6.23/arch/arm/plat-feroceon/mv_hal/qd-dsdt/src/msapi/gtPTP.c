#include <Copyright.h>

/*******************************************************************************
* gtPTP.c
*
* DESCRIPTION:
*       API definitions for Precise Time Protocol logic
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*******************************************************************************/

#include <msApi.h>
#include <gtSem.h>
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>


#ifdef CONFIG_AVB_FPGA

#define USE_SINGLE_READ

#define AVB_SMI_ADDR		0xC

#define QD_REG_PTP_INT_OFFSET		0
#define QD_REG_PTP_INTEN_OFFSET		1
#define QD_REG_PTP_FREQ_OFFSET		4
#define QD_REG_PTP_PHASE_OFFSET		6
#define QD_REG_PTP_CLK_CTRL_OFFSET	4
#define QD_REG_PTP_CYCLE_INTERVAL_OFFSET		5
#define QD_REG_PTP_CYCLE_ADJ_OFFSET				6
#define QD_REG_PTP_PLL_CTRL_OFFSET	7
#define QD_REG_PTP_CLK_SRC_OFFSET	0x9
#define QD_REG_PTP_P9_MODE_OFFSET	0xA
#define QD_REG_PTP_RESET_OFFSET		0xB

#define GT_PTP_MERGE_32BIT(_high16,_low16)	(((_high16)<<16)|(_low16))
#define GT_PTP_GET_HIGH16(_data)	((_data) >> 16) & 0xFFFF
#define GT_PTP_GET_LOW16(_data)		(_data) & 0xFFFF

#define AVB_FPGA_READ_REG	gsysReadMiiReg
#define AVB_FPGA_WRITE_REG	gsysWriteMiiReg

#endif

#define GT_PTP_BUILD_TIME(_time1, _time2)	(((_time1) << 16) | (_time2))

/****************************************************************************/
/* PTP operation function declaration.                                    */
/****************************************************************************/
static GT_STATUS ptpOperationPerform
(
    IN   GT_QD_DEV 			*dev,
    IN   GT_PTP_OPERATION	ptpOp,
    INOUT GT_PTP_OP_DATA 	*opData
);


/*******************************************************************************
* gptpSetConfig
*
* DESCRIPTION:
*       This routine writes PTP configuration parameters.
*
* INPUTS:
*		ptpData  - PTP configuration parameters.
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
GT_STATUS gptpSetConfig
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_PTP_CONFIG	*ptpData
)
{
	GT_STATUS       	retVal;
	GT_PTP_OPERATION	op;
	GT_PTP_OP_DATA		opData;

	DBG_INFO(("gptpSetConfig Called.\n"));

    /* check if device supports this feature */
#ifndef CONFIG_AVB_FPGA
	if (!IS_IN_DEV_GROUP(dev,DEV_PTP))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }
#endif

	opData.ptpPort = 0xF;	/* Global register */
	op = PTP_WRITE_DATA;

	/* setting PTPEType, offset 0 */
	opData.ptpAddr = 0;
	opData.ptpData = ptpData->ptpEType;

	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed writing PTPEType.\n"));
		return GT_FAIL;
	}

	/* setting MsgIDTSEn, offset 1 */
	opData.ptpAddr = 1;
	opData.ptpData = ptpData->msgIdTSEn;

	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed writing MsgIDTSEn.\n"));
		return GT_FAIL;
	}

	/* setting TSArrPtr, offset 2 */
	opData.ptpAddr = 2;
	opData.ptpData = ptpData->tsArrPtr;

	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed writing TSArrPtr.\n"));
		return GT_FAIL;
	}

	/* setting PTPArrIntEn, offset 3 */
	opData.ptpAddr = 3;
	opData.ptpData = GT_LPORTVEC_2_PORTVEC(ptpData->ptpArrIntEn);

	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed writing PTPArrIntEn.\n"));
		return GT_FAIL;
	}

	/* setting PTPDepIntEn, offset 4 */
	opData.ptpAddr = 4;
	opData.ptpData = GT_LPORTVEC_2_PORTVEC(ptpData->ptpDepIntEn);

	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed writing PTPDepIntEn.\n"));
		return GT_FAIL;
	}

	/* TransSpec, MsgIDStartBit, DisTSOverwrite bit, offset 5 */
	op = PTP_READ_DATA;
	opData.ptpAddr = 5;

	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed reading DisPTP.\n"));
		return GT_FAIL;
	}

	op = PTP_WRITE_DATA;
	opData.ptpData = ((ptpData->transSpec&0xF) << 12) | ((ptpData->msgIdStartBit&0x7) << 9) | 
					(opData.ptpData & 0x1) | ((ptpData->disTSOverwrite?1:0) << 1);

	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed writing MsgIDStartBit & DisTSOverwrite.\n"));
		return GT_FAIL;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gptpGetConfig
*
* DESCRIPTION:
*       This routine reads PTP configuration parameters.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*		ptpData  - PTP configuration parameters.
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
GT_STATUS gptpGetConfig
(
	IN  GT_QD_DEV 	*dev,
	OUT GT_PTP_CONFIG	*ptpData
)
{
	GT_STATUS       	retVal;
	GT_PTP_OPERATION	op;
	GT_PTP_OP_DATA		opData;

	DBG_INFO(("gptpGetConfig Called.\n"));

    /* check if device supports this feature */
#ifndef CONFIG_AVB_FPGA
	if (!IS_IN_DEV_GROUP(dev,DEV_PTP))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }
#endif

	opData.ptpPort = 0xF;	/* Global register */
	op = PTP_READ_DATA;

	/* getting PTPEType, offset 0 */
	opData.ptpAddr = 0;
	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed reading PTPEType.\n"));
		return GT_FAIL;
	}

	ptpData->ptpEType = opData.ptpData;

	/* getting MsgIDTSEn, offset 1 */
	opData.ptpAddr = 1;
	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed reading MsgIDTSEn.\n"));
		return GT_FAIL;
	}

	ptpData->msgIdTSEn = opData.ptpData;

	/* getting TSArrPtr, offset 2 */
	opData.ptpAddr = 2;
	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed reading TSArrPtr.\n"));
		return GT_FAIL;
	}

	ptpData->tsArrPtr = opData.ptpData;

	/* getting PTPArrIntEn, offset 3 */
	opData.ptpAddr = 3;
	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed reading PTPArrIntEn.\n"));
		return GT_FAIL;
	}
	opData.ptpData &= dev->validPortVec;
	ptpData->ptpArrIntEn = GT_PORTVEC_2_LPORTVEC(opData.ptpData);


	/* getting PTPDepIntEn, offset 4 */
	opData.ptpAddr = 4;
	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed reading PTPDepIntEn.\n"));
		return GT_FAIL;
	}

	opData.ptpData &= dev->validPortVec;
	ptpData->ptpDepIntEn = GT_PORTVEC_2_LPORTVEC(opData.ptpData);

	/* MsgIDStartBit, DisTSOverwrite bit, offset 5 */
	opData.ptpAddr = 5;

	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed reading DisPTP.\n"));
		return GT_FAIL;
	}

	ptpData->transSpec = (opData.ptpData >> 12) & 0xF;
	ptpData->msgIdStartBit = (opData.ptpData >> 9) & 0x7;
	ptpData->disTSOverwrite = ((opData.ptpData >> 1) & 0x1) ? GT_TRUE : GT_FALSE;

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gptpSetPTPEn
*
* DESCRIPTION:
*       This routine enables or disables PTP.
*
* INPUTS:
*		en - GT_TRUE to enable PTP, GT_FALSE to disable PTP
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
GT_STATUS gptpSetPTPEn
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_BOOL		en
)
{
	GT_STATUS       	retVal;
	GT_PTP_OPERATION	op;
	GT_PTP_OP_DATA		opData;

	DBG_INFO(("gptpSetPTPEn Called.\n"));

#ifndef CONFIG_AVB_FPGA
    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_PTP))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }
#endif

	opData.ptpPort = 0xF;	/* Global register */
	op = PTP_READ_DATA;
	opData.ptpAddr = 5;

	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed reading DisPTP.\n"));
		return GT_FAIL;
	}

	op = PTP_WRITE_DATA;
	opData.ptpData &= ~0x1;
	opData.ptpData |= (en ? 0 : 1);

	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed writing MsgIDStartBit & DisTSOverwrite.\n"));
		return GT_FAIL;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gptpGetPTPEn
*
* DESCRIPTION:
*       This routine checks if PTP is enabled.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*		en - GT_TRUE if enabled, GT_FALSE otherwise
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
GT_STATUS gptpGetPTPEn
(
	IN  GT_QD_DEV 	*dev,
	OUT GT_BOOL		*en
)
{
	GT_STATUS       	retVal;
	GT_PTP_OPERATION	op;
	GT_PTP_OP_DATA		opData;

	DBG_INFO(("gptpGetPTPEn Called.\n"));

#ifndef CONFIG_AVB_FPGA
    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_PTP))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }
#endif

	opData.ptpPort = 0xF;	/* Global register */
	op = PTP_READ_DATA;
	opData.ptpAddr = 5;

	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed reading DisPTP.\n"));
		return GT_FAIL;
	}

	*en = (opData.ptpData & 0x1) ? GT_FALSE : GT_TRUE;

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gptpGetPTPInt
*
* DESCRIPTION:
*       This routine gets PTP interrupt status for each port.
*		The PTP Interrupt bit gets set for a given port when an incoming PTP 
*		frame is time stamped and PTPArrIntEn for that port is set to 0x1.
*		Similary PTP Interrupt bit gets set for a given port when an outgoing
*		PTP frame is time stamped and PTPDepIntEn for that port is set to 0x1.
*		This bit gets cleared upon software reading and clearing the corresponding
*		time counter valid bits that are valid for that port.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*		ptpInt 	- interrupt status for each port (bit 0 for port 0, bit 1 for port 1, etc.)
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
GT_STATUS gptpGetPTPInt
(
	IN  GT_QD_DEV 	*dev,
	OUT GT_U32		*ptpInt
)
{
	GT_STATUS       	retVal;
	GT_PTP_OPERATION	op;
	GT_PTP_OP_DATA		opData;

	DBG_INFO(("gptpGetPTPInt Called.\n"));

#ifndef CONFIG_AVB_FPGA
    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_PTP))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }
#endif

	opData.ptpPort = 0xF;	/* Global register */
	op = PTP_READ_DATA;
	opData.ptpAddr = 8;

	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed reading DisPTP.\n"));
		return GT_FAIL;
	}

	opData.ptpData &= (1 << dev->maxPorts) - 1;

	*ptpInt = GT_PORTVEC_2_LPORTVEC(opData.ptpData);

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gptpGetPTPGlobalTime
*
* DESCRIPTION:
*       This routine gets the global timer value that is running off of the free
*		running switch core clock.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*		ptpTime	- PTP global time
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
GT_STATUS gptpGetPTPGlobalTime
(
	IN  GT_QD_DEV 	*dev,
	OUT GT_U32		*ptpTime
)
{
	GT_STATUS       	retVal;
	GT_PTP_OPERATION	op;
	GT_PTP_OP_DATA		opData;

	DBG_INFO(("gptpGetPTPGlobalTime Called.\n"));

#ifndef CONFIG_AVB_FPGA
    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_PTP))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }
#endif

#ifndef USE_SINGLE_READ
	opData.ptpPort = 0xF;	/* Global register */
	op = PTP_READ_MULTIPLE_DATA;
	opData.ptpAddr = 9;
	opData.nData = 2;

	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed reading DisPTP.\n"));
		return GT_FAIL;
	}

	*ptpTime = GT_PTP_BUILD_TIME(opData.ptpMultiData[1],opData.ptpMultiData[0]);
#else
	{
	GT_U32 data[2];

	opData.ptpPort = 0xF;	/* Global register */
	op = PTP_READ_DATA;
	opData.ptpAddr = 9;

	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed reading DisPTP.\n"));
		return GT_FAIL;
	}

	data[0] = opData.ptpData;

	opData.ptpPort = 0xF;	/* Global register */
	op = PTP_READ_DATA;
	opData.ptpAddr++;

	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed reading DisPTP.\n"));
		return GT_FAIL;
	}

	data[1] = opData.ptpData;

	*ptpTime = GT_PTP_BUILD_TIME(data[1],data[0]);

	}
#endif

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gptpGetTimeStamped
*
* DESCRIPTION:
*		This routine retrieves the PTP port status that includes time stamp value 
*		and sequce Id that are captured by PTP logic for a PTP frame that needs 
*		to be time stamped.
*
* INPUTS:
*       port 		- logical port number.
*       timeToRead	- Arr0, Arr1, or Dep time (GT_PTP_TIME enum type)
*
* OUTPUTS:
*		ptpStatus	- PTP port status
*
* RETURNS:
*       GT_OK 		- on success
*       GT_FAIL 	- on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpGetTimeStamped
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_LPORT	port,
	IN	GT_PTP_TIME	timeToRead,
	OUT GT_PTP_TS_STATUS	*ptpStatus
)
{
	GT_STATUS       	retVal;
	GT_U32				hwPort;
	GT_PTP_OPERATION	op;
	GT_PTP_OP_DATA		opData;

	DBG_INFO(("gptpGetTimeStamped Called.\n"));

#ifndef CONFIG_AVB_FPGA
    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_PTP))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }
#endif

    hwPort = (GT_U32)GT_LPORT_2_PORT(port);
	if (hwPort == GT_INVALID_PORT)
	{
        DBG_INFO(("Invalid port number\n"));
		return GT_BAD_PARAM;
	}

	switch (timeToRead)
	{
		case PTP_ARR0_TIME:
			opData.ptpAddr = 0;
			break;
		case PTP_ARR1_TIME:
			opData.ptpAddr = 4;
			break;
		case PTP_DEP_TIME:
			opData.ptpAddr = 8;
			break;
		default:
        	DBG_INFO(("Invalid time to be read\n"));
			return GT_BAD_PARAM;
	}

	opData.ptpPort = hwPort;

#ifndef USE_SINGLE_READ
	op = PTP_READ_MULTIPLE_DATA;
	opData.nData = 4;

	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed reading DisPTP.\n"));
		return GT_FAIL;
	}

	ptpStatus->isValid = (opData.ptpMultiData[0] & 0x1) ? GT_TRUE : GT_FALSE;
	ptpStatus->status = (GT_PTP_INT_STATUS)((opData.ptpMultiData[0] >> 1) & 0x3);
	ptpStatus->timeStamped = GT_PTP_BUILD_TIME(opData.ptpMultiData[2],opData.ptpMultiData[1]);
	ptpStatus->ptpSeqId = opData.ptpMultiData[3];
#else
	{
	GT_U32 data[4], i;

	op = PTP_READ_DATA;

	for (i=0; i<4; i++)
	{
		if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
		{
        	DBG_INFO(("Failed reading DisPTP.\n"));
			return GT_FAIL;
		}
	
		data[i] = opData.ptpData;
		opData.ptpAddr++;
	}

	ptpStatus->isValid = (data[0] & 0x1) ? GT_TRUE : GT_FALSE;
	ptpStatus->status = (GT_PTP_INT_STATUS)((data[0] >> 1) & 0x3);
	ptpStatus->timeStamped = GT_PTP_BUILD_TIME(data[2],data[1]);
	ptpStatus->ptpSeqId = data[3];

	}
#endif

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gptpResetTimeStamp
*
* DESCRIPTION:
*		This routine resets PTP Time valid bit so that PTP logic can time stamp
*		a next PTP frame that needs to be time stamped.
*
* INPUTS:
*       port 		- logical port number.
*       timeToReset	- Arr0, Arr1, or Dep time (GT_PTP_TIME enum type)
*
* OUTPUTS:
*		None.
*
* RETURNS:
*       GT_OK 		- on success
*       GT_FAIL 	- on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpResetTimeStamp
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_LPORT	port,
	IN	GT_PTP_TIME	timeToReset
)
{
	GT_STATUS       	retVal;
	GT_U32				hwPort;
	GT_PTP_OPERATION	op;
	GT_PTP_OP_DATA		opData;

	DBG_INFO(("gptpResetTimeStamp Called.\n"));

#ifndef CONFIG_AVB_FPGA
    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_PTP))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }
#endif

    hwPort = (GT_U32)GT_LPORT_2_PORT(port);
	if (hwPort == GT_INVALID_PORT)
	{
        DBG_INFO(("Invalid port number\n"));
		return GT_BAD_PARAM;
	}

	switch (timeToReset)
	{
		case PTP_ARR0_TIME:
			opData.ptpAddr = 0;
			break;
		case PTP_ARR1_TIME:
			opData.ptpAddr = 4;
			break;
		case PTP_DEP_TIME:
			opData.ptpAddr = 8;
			break;
		default:
        	DBG_INFO(("Invalid time to reset\n"));
			return GT_BAD_PARAM;
	}

	opData.ptpPort = hwPort;
	op = PTP_READ_DATA;

	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed reading Port Status.\n"));
		return GT_FAIL;
	}

	opData.ptpData &= ~0x1;
	op = PTP_WRITE_DATA;

	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed writing Port Status.\n"));
		return GT_FAIL;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gptpGetReg
*
* DESCRIPTION:
*       This routine reads PTP register.
*
* INPUTS:
*       port 		- logical port number.
*       regOffset	- register to read
*
* OUTPUTS:
*		data		- register data
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
GT_STATUS gptpGetReg
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_LPORT	port,
	IN  GT_U32		regOffset,
	OUT GT_U32		*data
)
{
	GT_STATUS       	retVal;
	GT_U32				hwPort;
	GT_PTP_OPERATION	op;
	GT_PTP_OP_DATA		opData;

	DBG_INFO(("gptpGetReg Called.\n"));

#ifndef CONFIG_AVB_FPGA
    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_PTP))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }
#endif

    hwPort = (GT_U32)port;

	if (regOffset > 0xF)
	{
        DBG_INFO(("Invalid reg offset\n"));
		return GT_BAD_PARAM;
	}

	op = PTP_READ_DATA;
	opData.ptpPort = hwPort;
	opData.ptpAddr = regOffset;

	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed reading DisPTP.\n"));
		return GT_FAIL;
	}

	*data = opData.ptpData;

	DBG_INFO(("OK.\n"));
	return GT_OK;

}

/*******************************************************************************
* gptpSetReg
*
* DESCRIPTION:
*       This routine writes data to PTP register.
*
* INPUTS:
*       port 		- logical port number
*       regOffset	- register to be written
*		data		- data to be written
*
* OUTPUTS:
*		None.
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
GT_STATUS gptpSetReg
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_LPORT	port,
	IN  GT_U32		regOffset,
	IN  GT_U32		data
)
{
	GT_STATUS       	retVal;
	GT_U32				hwPort;
	GT_PTP_OPERATION	op;
	GT_PTP_OP_DATA		opData;

	DBG_INFO(("gptpSetReg Called.\n"));

#ifndef CONFIG_AVB_FPGA
    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_PTP))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }
#endif

    hwPort = (GT_U32)port;

	if ((regOffset > 0xF) || (data > 0xFFFF))
	{
        DBG_INFO(("Invalid reg offset/data\n"));
		return GT_BAD_PARAM;
	}

	op = PTP_WRITE_DATA;
	opData.ptpPort = hwPort;
	opData.ptpAddr = regOffset;
	opData.ptpData = data;

	if((retVal = ptpOperationPerform(dev, op, &opData)) != GT_OK)
	{
        DBG_INFO(("Failed reading DisPTP.\n"));
		return GT_FAIL;
	}


	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/****************************************************************************/
/* Internal functions.                                                  */
/****************************************************************************/

/*******************************************************************************
* ptpOperationPerform
*
* DESCRIPTION:
*       This function accesses PTP Command Register and Data Register.
*
* INPUTS:
*       ptpOp      - The stats operation bits to be written into the stats
*                    operation register.
*
* OUTPUTS:
*       ptpData    - points to the data storage that the operation requires.
*
* RETURNS:
*       GT_OK on success,
*       GT_FAIL otherwise.
*
* COMMENTS:
*
*******************************************************************************/
static GT_STATUS ptpOperationPerform
(
    IN    GT_QD_DEV 			*dev,
    IN    GT_PTP_OPERATION		ptpOp,
    INOUT GT_PTP_OP_DATA		*opData
)
{
    GT_STATUS       retVal;	/* Functions return value */
    GT_U16          data; 	/* temporary Data storage */
	GT_U32 			i;
#ifdef CONFIG_AVB_FPGA
	GT_U32 			tmpData;
#endif

    gtSemTake(dev,dev->ptpRegsSem,OS_WAIT_FOREVER);

    /* Wait until the ptp in ready. */
    data = 1;
    while(data == 1)
    {
#ifndef CONFIG_AVB_FPGA
        retVal = hwGetGlobal2RegField(dev,QD_REG_PTP_COMMAND,15,1,&data);
#else
        retVal = AVB_FPGA_READ_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_COMMAND,&tmpData);
		data = (GT_U16)tmpData;
		data = (data >> 15) & 0x1;
#endif
        if(retVal != GT_OK)
        {
            gtSemGive(dev,dev->ptpRegsSem);
            return retVal;
        }
    }

    /* Set the PTP Operation register */
	switch (ptpOp)
	{
		case PTP_WRITE_DATA:
			data = (GT_U16)opData->ptpData;
#ifndef CONFIG_AVB_FPGA
			retVal = hwWriteGlobal2Reg(dev,QD_REG_PTP_DATA,data);
#else
			tmpData = (GT_U32)data;
	        retVal = AVB_FPGA_WRITE_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_DATA,tmpData);
#endif
	        if(retVal != GT_OK)
    	    {
        	    gtSemGive(dev,dev->ptpRegsSem);
            	return retVal;
	        }

			data = (1 << 15) | (PTP_WRITE_DATA << 12) | 
					(opData->ptpPort << 8)	|
					(opData->ptpAddr & 0xFF);
#ifndef CONFIG_AVB_FPGA
			retVal = hwWriteGlobal2Reg(dev,QD_REG_PTP_COMMAND,data);
#else
			tmpData = (GT_U32)data;
	        retVal = AVB_FPGA_WRITE_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_COMMAND,tmpData);
#endif
	        if(retVal != GT_OK)
    	    {
        	    gtSemGive(dev,dev->ptpRegsSem);
            	return retVal;
	        }
			break;

		case PTP_READ_DATA:
			data = (1 << 15) | (PTP_READ_DATA << 12) | 
					(opData->ptpPort << 8)	|
					(opData->ptpAddr & 0xFF);
#ifndef CONFIG_AVB_FPGA
			retVal = hwWriteGlobal2Reg(dev,QD_REG_PTP_COMMAND,data);
#else
			tmpData = (GT_U32)data;
	        retVal = AVB_FPGA_WRITE_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_COMMAND,tmpData);
#endif
	        if(retVal != GT_OK)
    	    {
        	    gtSemGive(dev,dev->ptpRegsSem);
            	return retVal;
	        }

		    data = 1;
		    while(data == 1)
		    {
#ifndef CONFIG_AVB_FPGA
		        retVal = hwGetGlobal2RegField(dev,QD_REG_PTP_COMMAND,15,1,&data);
#else
        		retVal = AVB_FPGA_READ_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_COMMAND,&tmpData);
				data = (GT_U32)tmpData;
				data = (data >> 15) & 0x1;
#endif
		        if(retVal != GT_OK)
		        {
		            gtSemGive(dev,dev->ptpRegsSem);
		            return retVal;
        		}
		    }

#ifndef CONFIG_AVB_FPGA
			retVal = hwReadGlobal2Reg(dev,QD_REG_PTP_DATA,&data);
#else
	        retVal = AVB_FPGA_READ_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_DATA,&tmpData);
			data = (GT_U32)tmpData;
#endif
			opData->ptpData = (GT_U32)data;
		    gtSemGive(dev,dev->ptpRegsSem);
		    return retVal;

		case PTP_READ_MULTIPLE_DATA:
			data = (1 << 15) | (PTP_READ_MULTIPLE_DATA << 12) | 
					(opData->ptpPort << 8)	|
					(opData->ptpAddr & 0xFF);
#ifndef CONFIG_AVB_FPGA
			retVal = hwWriteGlobal2Reg(dev,QD_REG_PTP_COMMAND,data);
#else
			tmpData = (GT_U32)data;
	        retVal = AVB_FPGA_WRITE_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_COMMAND,tmpData);
#endif
	        if(retVal != GT_OK)
    	    {
        	    gtSemGive(dev,dev->ptpRegsSem);
            	return retVal;
	        }

		    data = 1;
		    while(data == 1)
		    {
#ifndef CONFIG_AVB_FPGA
		        retVal = hwGetGlobal2RegField(dev,QD_REG_PTP_COMMAND,15,1,&data);
#else
        		retVal = AVB_FPGA_READ_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_COMMAND,&tmpData);
				data = (GT_U32)tmpData;
				data = (data >> 15) & 0x1;
#endif
		        if(retVal != GT_OK)
		        {
		            gtSemGive(dev,dev->ptpRegsSem);
		            return retVal;
        		}
		    }

			for(i=0; i<opData->nData; i++)
			{
#ifndef CONFIG_AVB_FPGA
				retVal = hwReadGlobal2Reg(dev,QD_REG_PTP_DATA,&data);
#else
		        retVal = AVB_FPGA_READ_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_DATA,&tmpData);
				data = (GT_U32)tmpData;
#endif
				opData->ptpMultiData[i] = (GT_U32)data;
	    	    if(retVal != GT_OK)
    	    	{
        	    	gtSemGive(dev,dev->ptpRegsSem);
	            	return retVal;
		        }
			}

		    gtSemGive(dev,dev->ptpRegsSem);
		    return retVal;

		default:
			
			gtSemGive(dev,dev->ptpRegsSem);
			return GT_FAIL;
	}

    /* Wait until the ptp is ready. */
    data = 1;
    while(data == 1)
    {
#ifndef CONFIG_AVB_FPGA
        retVal = hwGetGlobal2RegField(dev,QD_REG_PTP_COMMAND,15,1,&data);
#else
        retVal = AVB_FPGA_READ_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_COMMAND,&tmpData);
		data = (GT_U16)tmpData;
		data = (data >> 15) & 0x1;
#endif
        if(retVal != GT_OK)
        {
            gtSemGive(dev,dev->ptpRegsSem);
            return retVal;
        }
    }

    gtSemGive(dev,dev->ptpRegsSem);
    return retVal;
}


#ifdef CONFIG_AVB_FPGA


/*******************************************************************************
* gptpGetFPGAIntStatus
*
* DESCRIPTION:
*       This routine gets interrupt status of PTP logic.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*		ptpInt	- PTP Int Status
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
GT_STATUS gptpGetFPGAIntStatus
(
	IN  GT_QD_DEV 	*dev,
	OUT GT_U32		*ptpInt
)
{
	GT_STATUS       	retVal;
	GT_U32				data;

	DBG_INFO(("gptpGetPTPIntStatus Called.\n"));

	retVal = AVB_FPGA_READ_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_INT_OFFSET,&data);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	*ptpInt = (GT_U32)data & 0x1;

	DBG_INFO(("OK.\n"));
	return GT_OK;

}

/*******************************************************************************
* gptpSetFPGAIntStatus
*
* DESCRIPTION:
*       This routine sets interrupt status of PTP logic.
*
* INPUTS:
*	ptpInt	- PTP Int Status
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
GT_STATUS gptpSetFPGAIntStatus
(
	IN  GT_QD_DEV 	*dev,
	OUT GT_U32	ptpInt
)
{
	GT_STATUS       	retVal;
	GT_U32				data;

	DBG_INFO(("gptpSetPTPIntStatus Called.\n"));

	data = ptpInt?1:0;

	retVal = AVB_FPGA_WRITE_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_INT_OFFSET,ptpInt);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;
}

/*******************************************************************************
* gptpSetFPGAIntEn
*
* DESCRIPTION:
*       This routine enables PTP interrupt.
*
* INPUTS:
*		ptpInt	- PTP Int Status (1 to enable, 0 to disable)
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
GT_STATUS gptpSetFPGAIntEn
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_U32		ptpInt
)
{
	GT_STATUS       	retVal;
	GT_U32				data;

	DBG_INFO(("gptpGetPTPIntEn Called.\n"));

	data = (ptpInt == 0)?0:1;

	retVal = AVB_FPGA_WRITE_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_INTEN_OFFSET,data);
	if(retVal != GT_OK)
	{
		return retVal;
	}


	DBG_INFO(("OK.\n"));
	return GT_OK;

}

/*******************************************************************************
* gptpGetClockSource
*
* DESCRIPTION:
*       This routine gets PTP Clock source setup.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*		clkSrc	- PTP clock source (A/D Device or FPGA)
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
GT_STATUS gptpGetClockSource
(
	IN  GT_QD_DEV 	*dev,
	OUT GT_PTP_CLOCK_SRC 	*clkSrc
)
{
	GT_STATUS       	retVal;
	GT_U32				data;

	DBG_INFO(("gptpGetClockSource Called.\n"));

	retVal = AVB_FPGA_READ_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_CLK_SRC_OFFSET,&data);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	*clkSrc = (GT_PTP_CLOCK_SRC)(data & 0x1);

	DBG_INFO(("OK.\n"));
	return GT_OK;

}

/*******************************************************************************
* gptpSetClockSource
*
* DESCRIPTION:
*       This routine sets PTP Clock source setup.
*
* INPUTS:
*		clkSrc	- PTP clock source (A/D Device or FPGA)
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
GT_STATUS gptpSetClockSource
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_PTP_CLOCK_SRC 	clkSrc
)
{
	GT_STATUS       	retVal;
	GT_U32				data;

	DBG_INFO(("gptpSetClockSource Called.\n"));

	data = (GT_U32)clkSrc;

	retVal = AVB_FPGA_WRITE_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_CLK_SRC_OFFSET,data);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}

/*******************************************************************************
* gptpGetP9Mode
*
* DESCRIPTION:
*       This routine gets Port 9 Mode.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*		mode - Port 9 mode (GT_PTP_P9_MODE enum type)
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
GT_STATUS gptpGetP9Mode
(
	IN  GT_QD_DEV 	*dev,
	OUT GT_PTP_P9_MODE 	*mode
)
{
	GT_STATUS       	retVal;
	GT_U32				data;

	DBG_INFO(("gptpGetP9Mode Called.\n"));

	retVal = AVB_FPGA_READ_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_P9_MODE_OFFSET,&data);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	if (data & 0x1)
	{
		switch (data & 0x6)
		{
			case 0:
				*mode = PTP_P9_MODE_GMII;
				break;
			case 2:
				*mode = PTP_P9_MODE_MII;
				break;
			case 4:
				*mode = PTP_P9_MODE_MII_CONNECTOR;
				break;
			default:
				return GT_BAD_PARAM;
		}
	}
	else
	{
		*mode = PTP_P9_MODE_JUMPER;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gptpSetP9Mode
*
* DESCRIPTION:
*       This routine sets Port 9 Mode.
*
* INPUTS:
*		mode - Port 9 mode (GT_PTP_P9_MODE enum type)
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
GT_STATUS gptpSetP9Mode
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_PTP_P9_MODE 	mode
)
{
	GT_STATUS       	retVal;
	GT_U32				data;

	DBG_INFO(("gptpSetP9Mode Called.\n"));

	switch (mode)
	{
		case PTP_P9_MODE_GMII:
			data = 1;
			break;
		case PTP_P9_MODE_MII:
			data = 3;
			break;
		case PTP_P9_MODE_MII_CONNECTOR:
			data = 5;
			break;
		case PTP_P9_MODE_JUMPER:
			data = 0;
			break;
		default:
			return GT_BAD_PARAM;
	}

	retVal = AVB_FPGA_WRITE_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_P9_MODE_OFFSET,data);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}

/*******************************************************************************
* gptpReset
*
* DESCRIPTION:
*       This routine performs software reset for PTP logic.
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
GT_STATUS gptpReset
(
	IN  GT_QD_DEV 	*dev
)
{
	GT_STATUS       	retVal;
	GT_U32				data;

	DBG_INFO(("gptpReset Called.\n"));

	data = 1;

	retVal = AVB_FPGA_WRITE_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_RESET_OFFSET,data);
	if(retVal != GT_OK)
	{
		return retVal;
	}


	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gptpGetCycleAdjustEn
*
* DESCRIPTION:
*       This routine checks if PTP Duty Cycle Adjustment is enabled.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*		adjEn	- GT_TRUE if enabled, GT_FALSE otherwise
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
GT_STATUS gptpGetCycleAdjustEn
(
	IN  GT_QD_DEV 	*dev,
	OUT GT_BOOL		*adjEn
)
{
	GT_STATUS       	retVal;
	GT_U32				data;

	DBG_INFO(("gptpGetCycleAdjustEn Called.\n"));

	retVal = AVB_FPGA_READ_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_CLK_CTRL_OFFSET,&data);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	*adjEn = (data & 0x2)?GT_TRUE:GT_FALSE;

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gptpSetCycleAdjustEn
*
* DESCRIPTION:
*       This routine enables/disables PTP Duty Cycle Adjustment.
*
* INPUTS:
*		adjEn	- GT_TRUE to enable, GT_FALSE to disable
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
GT_STATUS gptpSetCycleAdjustEn
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_BOOL		adjEn
)
{
	GT_STATUS       	retVal;
	GT_U32				data;

	DBG_INFO(("gptpGetCycleAdjustEn Called.\n"));

	retVal = AVB_FPGA_READ_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_CLK_CTRL_OFFSET,&data);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	if (adjEn == GT_FALSE)
		data &= ~0x3;	/* clear both Enable bit and Valid bit */
	else
		data |= 0x2;

	retVal = AVB_FPGA_WRITE_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_CLK_CTRL_OFFSET,data);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gptpGetCycleAdjust
*
* DESCRIPTION:
*       This routine gets clock duty cycle adjustment value.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*		adj	- adjustment value (GT_PTP_CLOCK_ADJUSTMENT structure)
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
GT_STATUS gptpGetCycleAdjust
(
	IN  GT_QD_DEV 	*dev,
	OUT GT_PTP_CLOCK_ADJUSTMENT	*adj
)
{
	GT_STATUS       	retVal;
	GT_U32				data;

	DBG_INFO(("gptpGetCycleAdjust Called.\n"));

	retVal = AVB_FPGA_READ_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_CLK_CTRL_OFFSET,&data);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	adj->adjSign = (data & 0x4)?GT_PTP_SIGN_PLUS:GT_PTP_SIGN_NEGATIVE;
	adj->cycleStep = (data >> 3) & 0x7;

	retVal = AVB_FPGA_READ_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_CYCLE_INTERVAL_OFFSET,&data);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	adj->cycleInterval = data;

	retVal = AVB_FPGA_READ_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_CYCLE_ADJ_OFFSET,&data);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	adj->cycleAdjust = data;

	DBG_INFO(("OK.\n"));
	return GT_OK;

}

/*******************************************************************************
* gptpSetCycleAdjust
*
* DESCRIPTION:
*       This routine sets clock duty cycle adjustment value.
*
* INPUTS:
*		adj	- adjustment value (GT_PTP_CLOCK_ADJUSTMENT structure)
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
GT_STATUS gptpSetCycleAdjust
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_PTP_CLOCK_ADJUSTMENT	*adj
)
{
	GT_STATUS       	retVal;
	GT_U32				data, data1;

	DBG_INFO(("gptpSetCycleAdjust Called.\n"));

	retVal = AVB_FPGA_READ_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_CLK_CTRL_OFFSET,&data);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	data &= ~0x1;	/* clear Valid bit */
	retVal = AVB_FPGA_WRITE_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_CLK_CTRL_OFFSET,data);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	/* Setup the Cycle Interval */
	data1 = adj->cycleInterval & 0xFFFF;

	retVal = AVB_FPGA_WRITE_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_CYCLE_INTERVAL_OFFSET,data1);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	/* Setup the Cycle Adjustment */
	data1 = adj->cycleAdjust & 0xFFFF;

	retVal = AVB_FPGA_WRITE_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_CYCLE_ADJ_OFFSET,data1);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	/* clear Sign bit and Cycle Step bits on QD_REG_PTP_CLK_CTRL_OFFSET value */
	data &= ~0x3C;

	switch (adj->adjSign)
	{
		case GT_PTP_SIGN_PLUS:
			data |= 0x4;
			break;
			
		case GT_PTP_SIGN_NEGATIVE:
			break;

		default:
			return GT_BAD_PARAM;
	}

	data |= ((adj->cycleStep & 0x7) << 3);	/* setup Step bits */
	data |= 0x1;							/* set Valid bit */

	retVal = AVB_FPGA_WRITE_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_CLK_CTRL_OFFSET,data);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gptpGetPLLEn
*
* DESCRIPTION:
*       This routine checks if PLL is enabled.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*		en		- GT_TRUE if enabled, GT_FALSE otherwise
*		freqSel	- PLL Frequency Selection (default 0x3 - 22.368MHz)
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       PLL Frequence selection is based on the Clock Recovery PLL device.
*		IDT MK1575-01 is the default PLL device.
*
*******************************************************************************/
GT_STATUS gptpGetPLLEn
(
	IN  GT_QD_DEV 	*dev,
	OUT GT_BOOL		*en,
	OUT GT_U32		*freqSel
)
{
	GT_STATUS       	retVal;
	GT_U32				data;

	DBG_INFO(("gptpGetPLLEn Called.\n"));

	retVal = AVB_FPGA_READ_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_PLL_CTRL_OFFSET,&data);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	*en = (data & 0x1)?GT_TRUE:GT_FALSE;

	*freqSel = (data >> 1) & 0x7;

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gptpSetPLLEn
*
* DESCRIPTION:
*       This routine enables/disables PLL device.
*
* INPUTS:
*		en		- GT_TRUE to enable, GT_FALSE to disable
*		freqSel	- PLL Frequency Selection (default 0x3 - 22.368MHz)
*				  Meaningful only when enabling PLL device
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
*       PLL Frequence selection is based on the Clock Recovery PLL device.
*		IDT MK1575-01 is the default PLL device.
*
*******************************************************************************/
GT_STATUS gptpSetPLLEn
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_BOOL		en,
	IN  GT_U32		freqSel
)
{
	GT_STATUS       	retVal;
	GT_U32				data;

	DBG_INFO(("gptpSetPPLEn Called.\n"));

	retVal = AVB_FPGA_READ_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_PLL_CTRL_OFFSET,&data);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	if(en == GT_FALSE)
	{
		data |= 0x1;
	}
	else
	{
		data &= ~0x1;
		data |= (freqSel & 0x7) << 1;
	}

	retVal = AVB_FPGA_WRITE_REG(dev,AVB_SMI_ADDR,QD_REG_PTP_PLL_CTRL_OFFSET,data);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}



#endif

