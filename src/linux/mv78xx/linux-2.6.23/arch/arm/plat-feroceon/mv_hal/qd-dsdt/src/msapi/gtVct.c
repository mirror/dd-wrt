#include <Copyright.h>
/*******************************************************************************
* gtVct.c
*
* DESCRIPTION:
*       API for Marvell Virtual Cable Tester.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <msApi.h>
#include <gtVct.h>
#include <gtDrvConfig.h>
#include <gtDrvSwRegs.h>
#include <gtHwCntl.h>
#include <gtSem.h>



/*******************************************************************************
* analizePhy100MVCTResult
*
* DESCRIPTION:
*       This routine analize the virtual cable test result for 10/100M Phy
*
* INPUTS:
*       regValue - test result
*
* OUTPUTS:
*       cableStatus - analized test result.
*       cableLen    - cable length or the distance where problem occurs.
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
static
GT_STATUS analizePhy100MVCTResult
(
    IN  GT_QD_DEV *dev,
    IN  GT_U16 regValue, 
    OUT GT_TEST_STATUS *cableStatus,
    OUT GT_CABLE_LEN *cableLen
)
{
	int len;

	switch((regValue & 0x6000) >> 13)
	{
		case 0:
			/* test passed. No problem found. */
			/* check if there is impedance mismatch */
			if ((regValue & 0xFF) == 0xFF)
			{
				*cableStatus = GT_NORMAL_CABLE;
				cableLen->normCableLen = GT_UNKNOWN_LEN;
			}
			else
			{
				*cableStatus = GT_IMPEDANCE_MISMATCH;
				len = (int)FORMULA_PHY100M(regValue & 0xFF);
				if(len <= 0)
					cableLen->errCableLen = 0;
				else
					cableLen->errCableLen = (GT_U8)len;
			}
				
			break;
		case 1:
			/* test passed. Cable is short. */
			*cableStatus = GT_SHORT_CABLE;
			len = (int)FORMULA_PHY100M(regValue & 0xFF);
			if(len <= 0)
				cableLen->errCableLen = 0;
			else
				cableLen->errCableLen = (GT_U8)len;
			break;
		case 2:
			/* test passed. Cable is open. */
			*cableStatus = GT_OPEN_CABLE;
			len = (int)FORMULA_PHY100M(regValue & 0xFF);
			if(len <= 0)
				cableLen->errCableLen = 0;
			else
				cableLen->errCableLen = (GT_U8)len;
			break;
		case 3:
		default:
			/* test failed. No result is valid. */
			*cableStatus = GT_TEST_FAIL;
			break;
	}

	return GT_OK;
}


/*******************************************************************************
* getCableStatus_Phy100M
*
* DESCRIPTION:
*       This routine perform the virtual cable test for the 10/100Mbps phy,
*       and returns the the status per Rx/Tx pair.
*
* INPUTS:
*       port - logical port number.
*
* OUTPUTS:
*       cableStatus - the port copper cable status.
*       cableLen    - the port copper cable length.
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
static 
GT_STATUS getCableStatus_Phy100M
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8            hwPort,
    OUT GT_CABLE_STATUS *cableStatus
)
{
	GT_STATUS status;
	GT_U16 reg26, reg27;

    DBG_INFO(("getCableStatus_100Phy Called.\n"));

	/* 
	 * 	phy should be in 100 Full Duplex.
	 */
	if((status= hwWritePhyReg(dev,hwPort,0,QD_PHY_RESET | QD_PHY_SPEED | QD_PHY_DUPLEX)) != GT_OK)
	{
		return status;
	}

	/* 
	 * start Virtual Cable Tester
	 */
	if((status= hwWritePhyReg(dev,hwPort,26,0x8000)) != GT_OK)
	{
		return status;
	}

	do
	{
		if((status= hwReadPhyReg(dev,hwPort,26,&reg26)) != GT_OK)
		{
			return status;
		}
		
	} while(reg26 & 0x8000);

	/*
	 * read the test result for RX Pair
	 */
	if((status= hwReadPhyReg(dev,hwPort,26,&reg26)) != GT_OK)
	{
		return status;
	}
		
	/*
	 * read the test result for TX Pair
	 */
	if((status= hwReadPhyReg(dev,hwPort,27,&reg27)) != GT_OK)
	{
		return status;
	}
		
	cableStatus->phyType = PHY_100M;

	/*
	 * analyze the test result for RX Pair
	 */
	analizePhy100MVCTResult(dev, reg26, &cableStatus->cableStatus[MDI_RX_PAIR], 
							&cableStatus->cableLen[MDI_RX_PAIR]);

	/*
	 * analyze the test result for TX Pair
	 */
	analizePhy100MVCTResult(dev, reg27, &cableStatus->cableStatus[MDI_TX_PAIR], 
							&cableStatus->cableLen[MDI_TX_PAIR]);

	return status;
}

static
GT_STATUS  enable1stWorkAround_Phy100M
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8     hwPort
)
{
    GT_U16      value;
    GT_STATUS   status;

    /* enable 1st work-around */
    if ((status = hwWritePhyReg(dev, hwPort, 29, 3)) != GT_OK)
       return status;

    value = 0x6440;
    if ((status = hwWritePhyReg(dev, hwPort, 30, value)) != GT_OK)
       return status;

    return GT_OK;
}

static
GT_STATUS  disable1stWorkAround_Phy100M
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8     hwPort
)
{
    GT_STATUS status;

    /* disable 1st work-around */
    if ((status = hwWritePhyReg(dev, hwPort, 29, 3)) != GT_OK)
       return status;

    if ((status = hwWritePhyReg(dev, hwPort, 30, 0)) != GT_OK)
       return status;

    return GT_OK;
}

static
GT_STATUS workAround_Phy100M
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8            hwPort,
    OUT GT_CABLE_STATUS *cableStatus
)
{
	GT_STATUS status = GT_OK;

	/* 
	 * If Cable Status is OPEN and the length is less than 15m,
	 * then apply Work Around.
	 */

	if((cableStatus->cableStatus[MDI_RX_PAIR] == GT_OPEN_CABLE) ||
		(cableStatus->cableStatus[MDI_TX_PAIR] == GT_OPEN_CABLE))
	{
		/* must be disabled first and then enable again */
        disable1stWorkAround_Phy100M(dev,hwPort);

        enable1stWorkAround_Phy100M(dev,hwPort);

		if((status= hwWritePhyReg(dev,hwPort,29,0x000A)) != GT_OK)
		{
			return status;
		}
		if((status= hwWritePhyReg(dev,hwPort,30,0x0002)) != GT_OK)
		{
			return status;
		}

		if((status = getCableStatus_Phy100M(dev,hwPort,cableStatus)) != GT_OK)
		{
			return status;
		}
		
		if((status= hwWritePhyReg(dev,hwPort,29,0x000A)) != GT_OK)
		{
			return status;
		}
		if((status= hwWritePhyReg(dev,hwPort,30,0x0000)) != GT_OK)
		{
			return status;
		}
	}

	return status;
}


static
GT_STATUS  enable1stWorkAround_Phy1000M
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8     hwPort
)
{
    GT_STATUS   status;

    /* enable 1st work-around */
    if ((status = hwWritePhyReg(dev, hwPort, 29, 0x0018)) != GT_OK)
       return status;

    if ((status = hwWritePhyReg(dev, hwPort, 30, 0x00c2)) != GT_OK)
       return status;

    if ((status = hwWritePhyReg(dev, hwPort, 30, 0x00ca)) != GT_OK)
       return status;

    if ((status = hwWritePhyReg(dev, hwPort, 30, 0x00c2)) != GT_OK)
       return status;

    return GT_OK;
}

static
GT_STATUS  disable1stWorkAround_Phy1000M
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8     hwPort
)
{
    GT_STATUS status;

    /* disable 1st work-around */
    if ((status = hwWritePhyReg(dev, hwPort, 29, 0x0018)) != GT_OK)
       return status;

    if ((status = hwWritePhyReg(dev, hwPort, 30, 0x0042)) != GT_OK)
       return status;

    return GT_OK;
}

/*******************************************************************************
* analizePhy1000MVCTResult
*
* DESCRIPTION:
*       This routine analize the virtual cable test result for a Gigabit Phy
*
* INPUTS:
*       reg17 - original value of register 17
*       regValue - test result
*
* OUTPUTS:
*       cableStatus - analized test result.
*       cableLen    - cable length or the distance where problem occurs.
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
static
GT_STATUS analizePhy1000MVCTResult
(
    IN  GT_QD_DEV *dev,
    IN  GT_U16 reg17,
    IN  GT_U16 regValue, 
    OUT GT_TEST_STATUS *cableStatus,
    OUT GT_CABLE_LEN *cableLen
)
{
	GT_U16 u16Data;
	int len;

	switch((regValue & 0x6000) >> 13)
	{
		case 0:

			/* Check Impedance Mismatch */
			if ((regValue & 0xFF) < 0xFF)
			{
				/*  if the reflected amplitude is low it is good cable too.
					for this registers values it is a good cable:
					0xE23, 0xE24, 0xE25, 0xE26, 0xE27 */
				if ((regValue < 0xE23) || (regValue > 0xE27))
				{
					*cableStatus = GT_IMPEDANCE_MISMATCH;
					len = (int)FORMULA_PHY1000M(regValue & 0xFF);
					if(len <= 0)
						cableLen->errCableLen = 0;
					else
						cableLen->errCableLen = (GT_U8)len;
					break;
				}
			}

			/* test passed. No problem found. */
			*cableStatus = GT_NORMAL_CABLE;

			u16Data = reg17;

			/* To get Cable Length, Link should be on and Speed should be 100M or 1000M */
			if(!(u16Data & 0x0400))
			{
				cableLen->normCableLen = GT_UNKNOWN_LEN;
				break;
			}

			if((u16Data & 0xC000) != 0x8000)
			{
				cableLen->normCableLen = GT_UNKNOWN_LEN;
				break;
			}

			/*
			 * read the test result for the selected MDI Pair
			 */

			u16Data = ((u16Data >> 7) & 0x7);

			switch(u16Data)
			{
				case 0:
					cableLen->normCableLen = GT_LESS_THAN_50M;
					break;
				case 1:
					cableLen->normCableLen = GT_50M_80M;
					break;
				case 2:
					cableLen->normCableLen = GT_80M_110M;
					break;
				case 3:
					cableLen->normCableLen = GT_110M_140M;
					break;
				case 4:
					cableLen->normCableLen = GT_MORE_THAN_140;
					break;
				default:
					cableLen->normCableLen = GT_UNKNOWN_LEN;
					break;
			}
			break;
		case 1:
			/* test passed. Cable is short. */
			*cableStatus = GT_SHORT_CABLE;
			len = (int)FORMULA_PHY1000M(regValue & 0xFF);
			if(len <= 0)
				cableLen->errCableLen = 0;
			else
				cableLen->errCableLen = (GT_U8)len;
			break;
		case 2:
			/* test passed. Cable is open. */
			*cableStatus = GT_OPEN_CABLE;
			len = (int)FORMULA_PHY1000M(regValue & 0xFF);
			if(len <= 0)
				cableLen->errCableLen = 0;
			else
				cableLen->errCableLen = (GT_U8)len;
			break;
		case 3:
		default:
			/* test failed. No result is valid. */
			*cableStatus = GT_TEST_FAIL;
			break;
	}

	return GT_OK;
}


/*******************************************************************************
* getCableStatus_Phy1000M
*
* DESCRIPTION:
*       This routine perform the virtual cable test for the 10/100Mbps phy,
*       and returns the the status per Rx/Tx pair.
*
* INPUTS:
*       port - logical port number.
*		reg17 - original value of reg17.
*
* OUTPUTS:
*       cableStatus - the port copper cable status.
*       cableLen    - the port copper cable length.
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
static 
GT_STATUS getCableStatus_Phy1000M
(	
    IN  GT_QD_DEV *dev,
    IN  GT_U8			hwPort,
    IN  GT_U16 			reg17,
    OUT GT_CABLE_STATUS *cableStatus
)
{
	GT_STATUS status;
	GT_U16 reg28;
	int i;

    DBG_INFO(("getCableStatus_Phy1000M Called.\n"));

	/* 
	 * start Virtual Cable Tester
	 */
	if((status= hwWritePagedPhyReg(dev,hwPort,0,28,0,0x8000)) != GT_OK)
	{
		return status;
	}

	do
	{
		if((status= hwReadPhyReg(dev,hwPort,28,&reg28)) != GT_OK)
		{
			return status;
		}
		
	} while(reg28 & 0x8000);

	cableStatus->phyType = PHY_1000M;

    DBG_INFO(("Reg28 after test : %0#x.\n", reg28));

	for (i=0; i<GT_MDI_PAIR_NUM; i++)
	{
		/*
		 * read the test result for the selected MDI Pair
		 */
		if((status= hwReadPagedPhyReg(dev,hwPort,i,28,0,&reg28)) != GT_OK)
		{
			return status;
		}
		
		/*
		 * analyze the test result for RX Pair
		 */
		if((status = analizePhy1000MVCTResult(dev, reg17, reg28, 
								&cableStatus->cableStatus[i], 
								&cableStatus->cableLen[i])) != GT_OK)
		{
			return status;
		}
	}

	return GT_OK;
}

static
GT_STATUS workAround_Phy1000M
(
  GT_QD_DEV *dev,
  GT_U8 hwPort
)
{
	GT_STATUS status;

    DBG_INFO(("workAround for Gigabit Phy Called.\n"));

	if((status = hwWritePhyReg(dev,hwPort,29,0x1e)) != GT_OK)
	{
		return status;
	}
		
	if((status = hwWritePhyReg(dev,hwPort,30,0xcc00)) != GT_OK)
	{
		return status;
	}

	if((status = hwWritePhyReg(dev,hwPort,30,0xc800)) != GT_OK)
	{
		return status;
	}
	if((status = hwWritePhyReg(dev,hwPort,30,0xc400)) != GT_OK)
	{
		return status;
	}
	if((status = hwWritePhyReg(dev,hwPort,30,0xc000)) != GT_OK)
	{
		return status;
	}
	if((status = hwWritePhyReg(dev,hwPort,30,0xc100)) != GT_OK)
	{
		return status;
	}

    DBG_INFO(("workAround for Gigabit Phy completed.\n"));
	return GT_OK;
}


/*******************************************************************************
* getCableStatus_Phy1000MPage
*
* DESCRIPTION:
*       This routine perform the virtual cable test for the 10/100Mbps phy with
*       multiple page mode and returns the the status per MDIP/N.
*
* INPUTS:
*       port - logical port number.
*
* OUTPUTS:
*       cableStatus - the port copper cable status.
*       cableLen    - the port copper cable length.
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
static 
GT_STATUS getCableStatus_Phy1000MPage
(	
    IN  GT_QD_DEV *dev,
    IN  GT_U8			hwPort,
	IN  GT_PHY_INFO		*phyInfo,
    OUT GT_CABLE_STATUS *cableStatus
)
{
	GT_STATUS status;
	GT_U16 u16Data;
	GT_U16 reg17 = 0;
	int i;

    DBG_INFO(("getCableStatus_Phy1000M Called.\n"));

	/*
	 * If Fiber is used, simply return with test fail.
	 */
	if(phyInfo->flag & GT_PHY_FIBER)
	{
		if((status= hwReadPagedPhyReg(dev,(GT_U32)hwPort,1,17,phyInfo->anyPage,&u16Data)) != GT_OK)
		{
			return status;
		}

		if(u16Data & 0x400)
		{
			for (i=0; i<GT_MDI_PAIR_NUM; i++)
			{
				cableStatus->cableStatus[i] = GT_TEST_FAIL;
			}
			return GT_OK;
		}
	}

	/*
	 * If Copper is used and Link is on, get DSP Distance and put it in the
	 * old reg17 format.(bit9:7 with DSP Distance)
	 */
	if((status= hwReadPagedPhyReg(dev,(GT_U32)hwPort,0,17,phyInfo->anyPage,&u16Data)) != GT_OK)
	{
		return status;
	}

	if(u16Data & 0x400)
	{
		reg17 = (u16Data & 0xC000) | 0x400;

		if((status= hwReadPagedPhyReg(dev,(GT_U32)hwPort,5,26,phyInfo->anyPage,&u16Data)) != GT_OK)
		{
			return status;
		}
		reg17 |= ((u16Data & 0x7) << 7);
	}

	/* 
	 * start Virtual Cable Tester
	 */
	if((status= hwWritePagedPhyReg(dev,(GT_U32)hwPort,5,16,phyInfo->anyPage,0x8000)) != GT_OK)
	{
		return status;
	}

	do
	{
		if((status= hwReadPagedPhyReg(dev,(GT_U32)hwPort,5,16,phyInfo->anyPage,&u16Data)) != GT_OK)
		{
			return status;
		}
		
	} while(u16Data & 0x8000);

	cableStatus->phyType = PHY_1000M;

    DBG_INFO(("Page 5 of Reg16 after test : %0#x.\n", u16Data));

	for (i=0; i<GT_MDI_PAIR_NUM; i++)
	{
		/*
		 * read the test result for the selected MDI Pair
		 */
		if((status= hwReadPagedPhyReg(dev,(GT_U32)hwPort,5,16+i,phyInfo->anyPage,&u16Data)) != GT_OK)
		{
			return status;
		}
		
		/*
		 * analyze the test result for RX Pair
		 */
		if((status = analizePhy1000MVCTResult(dev, reg17, u16Data, 
								&cableStatus->cableStatus[i], 
								&cableStatus->cableLen[i])) != GT_OK)
		{
			return status;
		}
	}

	return GT_OK;
}



/*******************************************************************************
* gvctGetCableStatus
*
* DESCRIPTION:
*       This routine perform the virtual cable test for the requested port,
*       and returns the the status per MDI pair.
*
* INPUTS:
*       port - logical port number.
*
* OUTPUTS:
*       cableStatus - the port copper cable status.
*       cableLen    - the port copper cable length.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*       Command - vctGetCableDiag
*
*******************************************************************************/
GT_STATUS gvctGetCableDiag
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT        port,
    OUT GT_CABLE_STATUS *cableStatus
)
{
	GT_STATUS status;
	GT_U8 hwPort;
	GT_U16 orgReg0, orgReg17;
	GT_BOOL ppuEn;
	GT_PHY_INFO	phyInfo;
	GT_BOOL			autoOn;
	GT_U16			pageReg;

    DBG_INFO(("gvctGetCableDiag Called.\n"));
	hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	/* check if the port supports VCT */
	if(driverFindPhyInformation(dev,hwPort,&phyInfo) != GT_OK)
	{
	    DBG_INFO(("Unknown PHY device.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	if (!(phyInfo.flag & GT_PHY_VCT_CAPABLE))
	{
		DBG_INFO(("Not Supported\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	/* Need to disable PPUEn for safe. */
	if(gsysGetPPUEn(dev,&ppuEn) != GT_OK)
	{
		ppuEn = GT_FALSE;
	}

	if(ppuEn != GT_FALSE)
	{
		if((status= gsysSetPPUEn(dev,GT_FALSE)) != GT_OK)
		{
	    	DBG_INFO(("Not able to disable PPUEn.\n"));
			gtSemGive(dev,dev->phyRegsSem);
			return status;
		}
		gtDelay(250);
	}
		
	if(driverPagedAccessStart(dev,hwPort,phyInfo.pageType,&autoOn,&pageReg) != GT_OK)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	/*
	 * save original register 17 value, which will be used later depending on
	 * test result.
	 */
	if((status= hwReadPagedPhyReg(dev,hwPort,0,17,phyInfo.anyPage,&orgReg17)) != GT_OK)
	{
	    DBG_INFO(("Not able to reset the Phy.\n"));
		goto cableDiagCleanup;
	}

	/*
	 * save Config Register data
	 */
	if((status= hwReadPagedPhyReg(dev,hwPort,0,0,phyInfo.anyPage,&orgReg0)) != GT_OK)
	{
	    DBG_INFO(("Not able to reset the Phy.\n"));
		goto cableDiagCleanup;
	}

	switch(phyInfo.vctType)
	{
		case GT_PHY_VCT_TYPE1:
			enable1stWorkAround_Phy100M(dev,hwPort);
			status = getCableStatus_Phy100M(dev,hwPort,cableStatus);
            /* every fast ethernet phy requires this work-around */
			workAround_Phy100M(dev,hwPort,cableStatus);
			disable1stWorkAround_Phy100M(dev,hwPort);
			break;
		case GT_PHY_VCT_TYPE2:
			enable1stWorkAround_Phy1000M(dev,hwPort);
			status = getCableStatus_Phy1000M(dev,hwPort,orgReg17,cableStatus);
			disable1stWorkAround_Phy1000M(dev,hwPort);
			break;
		case GT_PHY_VCT_TYPE3:
			enable1stWorkAround_Phy1000M(dev,hwPort);
			workAround_Phy1000M(dev,hwPort);
			status = getCableStatus_Phy1000M(dev,hwPort,orgReg17,cableStatus);
			disable1stWorkAround_Phy1000M(dev,hwPort);
			break;
		case GT_PHY_VCT_TYPE4:
			status = getCableStatus_Phy1000MPage(dev,hwPort,&phyInfo,cableStatus);
			break;
		default:
			status = GT_FAIL;
			break;
	}
	
	if (!(phyInfo.flag & GT_PHY_GIGABIT))
	{
		if((status = hwPhyReset(dev,hwPort,orgReg0)) != GT_OK)
		{
			gtSemGive(dev,dev->phyRegsSem);
			return status;
		}
	}
	else
	{
		/*
		 * restore Config Register Data
		 */
		if((status= hwWritePagedPhyReg(dev,hwPort,0,0,phyInfo.anyPage,orgReg0)) != GT_OK)
		{
			gtSemGive(dev,dev->phyRegsSem);
			return status;
		}

		/* soft reset */
		if((status = hwPhyReset(dev,hwPort,0xFF)) != GT_OK)
		{
			gtSemGive(dev,dev->phyRegsSem);
			return status;
		}
	}

cableDiagCleanup:

	if(driverPagedAccessStop(dev,hwPort,phyInfo.pageType,autoOn,pageReg) != GT_OK)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	if(ppuEn != GT_FALSE)
	{
		if(gsysSetPPUEn(dev,ppuEn) != GT_OK)
		{
	    	DBG_INFO(("Not able to enable PPUEn.\n"));
			status = GT_FAIL;
		}
	}

	gtSemGive(dev,dev->phyRegsSem);
	return status;	
}


/*******************************************************************************
* getExStatus1000M
*
* DESCRIPTION:
*       This routine retrieves Pair Skew, Pair Swap, and Pair Polarity
*		for 1000M phy
*
* INPUTS:
*       dev - device context.
*       port - logical port number.
*
* OUTPUTS:
*       extendedStatus - extended cable status.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_STATUS getExStatus1000M
(
    IN  GT_QD_DEV 		*dev,
    IN  GT_U8           hwPort,
    OUT GT_1000BT_EXTENDED_STATUS *extendedStatus
)
{
	GT_STATUS status;
	GT_U16 u16Data, i;

	/*
	 * get data from 28_5 register
	 */
	if((status= hwReadPagedPhyReg(dev,hwPort,5,28,0,&u16Data)) != GT_OK)
	{
	    DBG_INFO(("Not able to read a Phy register.\n"));
		return status;
	}

	/* if bit 6 is not set, it's not valid. */
	if (!(u16Data & 0x0040))
	{
	    DBG_INFO(("Valid Bit is not set (%0#x).\n", u16Data));
		extendedStatus->isValid = GT_FALSE;
		return GT_OK;
	}

	extendedStatus->isValid = GT_TRUE;
	
	/* get Pair Polarity */
	for(i=0; i<GT_MDI_PAIR_NUM; i++)
	{
		switch((u16Data >> i) & 0x1)
		{
			case 0:
				extendedStatus->pairPolarity[i] = GT_POSITIVE;
				break;
			default:
				extendedStatus->pairPolarity[i] = GT_NEGATIVE;
				break;
		}
	}

	/* get Pair Swap */
	for(i=0; i<GT_CHANNEL_PAIR_NUM; i++)
	{
		switch((u16Data >> (i+4)) & 0x1)
		{
			case 0:
				extendedStatus->pairSwap[i] = GT_STRAIGHT_CABLE;
				break;
			default:
				extendedStatus->pairSwap[i] = GT_CROSSOVER_CABLE;
				break;
		}
	}

	/*
	 * get data from 28_4 register
	 */
	if((status= hwReadPagedPhyReg(dev,hwPort,4,28,0,&u16Data)) != GT_OK)
	{
	    DBG_INFO(("Not able to read a Phy register.\n"));
		return status;
	}

	/* get Pair Skew */
	for(i=0; i<GT_MDI_PAIR_NUM; i++)
	{
		extendedStatus->pairSkew[i] = ((u16Data >> i*4) & 0xF) * 8;
	}

	return GT_OK;
}


/*******************************************************************************
* getExStatus1000MPage
*
* DESCRIPTION:
*       This routine retrieves Pair Skew, Pair Swap, and Pair Polarity
*		for 1000M phy with multiple page mode
*
* INPUTS:
*       dev - device context.
*       port - logical port number.
*
* OUTPUTS:
*       extendedStatus - extended cable status.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_STATUS getExStatus1000MPage
(
    IN  GT_QD_DEV 		*dev,
    IN  GT_U8	        hwPort,
    OUT GT_1000BT_EXTENDED_STATUS *extendedStatus
)
{
	GT_STATUS status;
	GT_U16 u16Data, i;

	/*
	 * get data from 21_5 register for pair swap
	 */
	if((status= hwReadPagedPhyReg(dev,(GT_U32)hwPort,5,21,0,&u16Data)) != GT_OK)
	{
	    DBG_INFO(("Not able to read a paged Phy register.\n"));
		return status;
	}

	/* if bit 6 is not set, it's not valid. */
	if (!(u16Data & 0x0040))
	{
	    DBG_INFO(("Valid Bit is not set (%0#x).\n", u16Data));
		extendedStatus->isValid = GT_FALSE;
		return GT_OK;
	}

	extendedStatus->isValid = GT_TRUE;
	
	/* get Pair Polarity */
	for(i=0; i<GT_MDI_PAIR_NUM; i++)
	{
		switch((u16Data >> i) & 0x1)
		{
			case 0:
				extendedStatus->pairPolarity[i] = GT_POSITIVE;
				break;
			default:
				extendedStatus->pairPolarity[i] = GT_NEGATIVE;
				break;
		}
	}

	/* get Pair Swap */
	for(i=0; i<GT_CHANNEL_PAIR_NUM; i++)
	{
		switch((u16Data >> (i+4)) & 0x1)
		{
			case 0:
				extendedStatus->pairSwap[i] = GT_STRAIGHT_CABLE;
				break;
			default:
				extendedStatus->pairSwap[i] = GT_CROSSOVER_CABLE;
				break;
		}
	}

	/*
	 * get data from 20_5 register for pair skew
	 */
	if((status= hwReadPagedPhyReg(dev,(GT_U32)hwPort,5,20,0,&u16Data)) != GT_OK)
	{
	    DBG_INFO(("Not able to read a paged Phy register.\n"));
		return status;
	}

	/* get Pair Skew */
	for(i=0; i<GT_MDI_PAIR_NUM; i++)
	{
		extendedStatus->pairSkew[i] = ((u16Data >> i*4) & 0xF) * 8;
	}

	return GT_OK;
}


/*******************************************************************************
* gvctGet1000BTExtendedStatus
*
* DESCRIPTION:
*       This routine retrieves Pair Skew, Pair Swap, and Pair Polarity
*
* INPUTS:
*       dev - device context.
*       port - logical port number.
*
* OUTPUTS:
*       extendedStatus - extended cable status.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gvctGet1000BTExtendedStatus
(
    IN  GT_QD_DEV 		*dev,
    IN  GT_LPORT        port,
    OUT GT_1000BT_EXTENDED_STATUS *extendedStatus
)
{
	GT_STATUS status;
	GT_U8 hwPort;
	GT_BOOL ppuEn;
	GT_PHY_INFO	phyInfo;
	GT_BOOL			autoOn;
	GT_U16			pageReg;

    DBG_INFO(("gvctGetCableDiag Called.\n"));
	hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	/* check if the port supports VCT */
	if(driverFindPhyInformation(dev,hwPort,&phyInfo) != GT_OK)
	{
	    DBG_INFO(("Unknown PHY device.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	if (!(phyInfo.flag & GT_PHY_EX_CABLE_STATUS))
	{
		DBG_INFO(("Not Supported\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	/* Need to disable PPUEn for safe. */
	if(gsysGetPPUEn(dev,&ppuEn) != GT_OK)
	{
		ppuEn = GT_FALSE;
	}

	if(ppuEn != GT_FALSE)
	{
		if((status= gsysSetPPUEn(dev,GT_FALSE)) != GT_OK)
		{
	    	DBG_INFO(("Not able to disable PPUEn.\n"));
			gtSemGive(dev,dev->phyRegsSem);
			return status;
		}
		gtDelay(250);
	}

	if(driverPagedAccessStart(dev,hwPort,phyInfo.pageType,&autoOn,&pageReg) != GT_OK)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	switch(phyInfo.vctType)
	{
		case GT_PHY_VCT_TYPE2:
			status = getExStatus1000M(dev,hwPort,extendedStatus);
			break;
		case GT_PHY_VCT_TYPE4:
			status = getExStatus1000MPage(dev,hwPort,extendedStatus);
			break;
		default:
	   		DBG_INFO(("Device is not supporting Extended Cable Status.\n"));
			status = GT_NOT_SUPPORTED;
	}

	if(driverPagedAccessStop(dev,hwPort,phyInfo.pageType,autoOn,pageReg) != GT_OK)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	if(ppuEn != GT_FALSE)
	{
		if(gsysSetPPUEn(dev,ppuEn) != GT_OK)
		{
	    	DBG_INFO(("Not able to enable PPUEn.\n"));
			status = GT_FAIL;
		}
	}

	gtSemGive(dev,dev->phyRegsSem);
	return status;
}


