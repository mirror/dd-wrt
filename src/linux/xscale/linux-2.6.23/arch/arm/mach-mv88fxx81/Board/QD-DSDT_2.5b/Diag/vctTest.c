#include <Copyright.h>
/********************************************************************************
* testApi.c
*
* DESCRIPTION:
*       API test functions
*
* DEPENDENCIES:   Platform.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/
#include "msSample.h"

void displayVCTResult
(
	GT_TEST_STATUS *cableStatus, 
	GT_CABLE_LEN *cableLen
)
{
	switch(*cableStatus)
	{
		case GT_TEST_FAIL:
			MSG_PRINT(("Cable Test Failed\n"));
			break;
		case GT_NORMAL_CABLE:
			MSG_PRINT(("Cable Test Passed. No problem found.\n"));
			switch(cableLen->normCableLen)
			{
				case GT_LESS_THAN_50M:
					MSG_PRINT(("Cable Length is less than 50M.\n"));
					break;
				case GT_50M_80M:
					MSG_PRINT(("Cable Length is between 50M and 80M.\n"));
					break;
				case GT_80M_110M:
					MSG_PRINT(("Cable Length is between 80M and 110M.\n"));
					break;
				case GT_110M_140M:
					MSG_PRINT(("Cable Length is between 110M and 140M.\n"));
					break;
				case GT_MORE_THAN_140:
					MSG_PRINT(("Cable Length is over 140M.\n"));
					break;
				default:
					MSG_PRINT(("Cable Length is unknown.\n"));
					break;
			}
			break;
		case GT_IMPEDANCE_MISMATCH:
			MSG_PRINT(("Cable Test Passed with Impedance Mismatch.\n"));
			MSG_PRINT(("Approximatly %i meters from the tested port.\n",cableLen->errCableLen));
			break;
		case GT_OPEN_CABLE:
			MSG_PRINT(("Cable Test Passed. Cable is open.\n"));
			MSG_PRINT(("Approximatly %i meters from the tested port.\n",cableLen->errCableLen));
			break;
		case GT_SHORT_CABLE:
			MSG_PRINT(("Cable Test Passed. Cable is short.\n"));
			MSG_PRINT(("Approximatly %i meters from the tested port.\n",cableLen->errCableLen));
			break;
		default:
			MSG_PRINT(("Unknown Test Result.\n"));
			break;
	}
}


GT_STATUS vctTest(GT_QD_DEV *dev,GT_LPORT port)
{
	GT_STATUS status;
    GT_CABLE_STATUS cableStatus;
	int i;

	if (dev == 0)
	{
		MSG_PRINT(("QD driver is not initialized\n"));
		return GT_FAIL;
#if 0
		dev = &qdDev;
		memset(dev, 0, sizeof(GT_QD_DEV));
		dev->fgtReadMii = gtBspReadMii;
		dev->fgtWriteMii = gtBspWriteMii;
#endif
	}

	/*
	 *	Start and get Cable Test Result
	*/
	if((status = gvctGetCableDiag(dev,port, &cableStatus)) != GT_OK)
	{
		MSG_PRINT(("gvctGetCableDiag return Failed\n"));
		return status;
	}

	MSG_PRINT(("Cable Test Result for Port %i\n",port));

	if(cableStatus.phyType == PHY_100M)
	{
		MSG_PRINT(("RX PAIR :\n"));
		displayVCTResult(&cableStatus.cableStatus[MDI_RX_PAIR],
									&cableStatus.cableLen[MDI_RX_PAIR]);
		MSG_PRINT(("TX PAIR :\n"));
		displayVCTResult(&cableStatus.cableStatus[MDI_TX_PAIR],
									&cableStatus.cableLen[MDI_TX_PAIR]);
	}
	else /* phyType must be PHY_1000M */
	{
		for(i=0; i<GT_MDI_PAIR_NUM; i++)
		{
			MSG_PRINT(("MDI PAIR %i:\n",i));
			displayVCTResult(&cableStatus.cableStatus[i],
									&cableStatus.cableLen[i]);
		}
	}

	return GT_OK;
}


GT_STATUS getExtendedStatus(GT_QD_DEV *dev,GT_LPORT port)
{
	GT_STATUS status;
	GT_1000BT_EXTENDED_STATUS extendedStatus;
	int i;

	if (dev == 0)
	{
		MSG_PRINT(("QD driver is not initialized\n"));
		return GT_FAIL;
#if 0
		dev = &qdDev;
		memset(dev, 0, sizeof(GT_QD_DEV));
		dev->fgtReadMii = gtBspReadMii;
		dev->fgtWriteMii = gtBspWriteMii;
#endif
	}

	/*
	 * 	Start getting Extended Information.
	 */
	if((status = gvctGet1000BTExtendedStatus(dev,port, &extendedStatus)) != GT_OK)
	{
		MSG_PRINT(("gvctGetCableDiag return Failed\n"));
		return status;
	}

	if (!extendedStatus.isValid)
	{
		MSG_PRINT(("Not able to get Extended Status.\n"));
		return status;

	}

	/* Pair Polarity */
	MSG_PRINT(("Pair Polarity:\n"));
	for(i=0; i<GT_MDI_PAIR_NUM; i++)
	{
		MSG_PRINT(("MDI PAIR %i: %s\n",i,
				(extendedStatus.pairPolarity[i] == GT_POSITIVE)?"Positive":"Negative"));
	}
	
	/* Pair Swap */
	MSG_PRINT(("Pair Swap:\n"));
	for(i=0; i<GT_CHANNEL_PAIR_NUM; i++)
	{
		MSG_PRINT(("CHANNEL PAIR %i: %s\n",i,
				(extendedStatus.pairSwap[i] == GT_STRAIGHT_CABLE)?"Straight":"Crossover"));
	}
	
	/* Pair Polarity */
	MSG_PRINT(("Pair Skew:\n"));
	for(i=0; i<GT_MDI_PAIR_NUM; i++)
	{
		MSG_PRINT(("MDI PAIR %i: %ins\n",i,extendedStatus.pairSkew[i]));
	}
	
	return GT_OK;
}


