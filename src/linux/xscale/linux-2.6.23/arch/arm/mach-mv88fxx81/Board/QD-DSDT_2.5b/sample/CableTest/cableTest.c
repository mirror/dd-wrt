#include <Copyright.h>
/********************************************************************************
* cableTest.c
*
* DESCRIPTION:
*		This sample shows how to run Virtual Cable Test and how to use the 
*		test result.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*
* COMMENTS:
*******************************************************************************/

#include "msSample.h"

void sampleDisplayCableTestResult
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
			MSG_PRINT(("Cable Test Passed. Cable has Impedance Mismatch .\n"));
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


GT_STATUS sampleCableTest(GT_QD_DEV *dev,GT_LPORT port)
{
	GT_STATUS status;
    GT_CABLE_STATUS cableStatus;
	int i;
		
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
		sampleDisplayCableTestResult(&cableStatus.cableStatus[MDI_RX_PAIR],
									&cableStatus.cableLen[MDI_RX_PAIR]);
		MSG_PRINT(("TX PAIR :\n"));
		sampleDisplayCableTestResult(&cableStatus.cableStatus[MDI_TX_PAIR],
									&cableStatus.cableLen[MDI_TX_PAIR]);
	}
	else /* phyType must be PHY_1000M */
	{
		for(i=0; i<GT_MDI_PAIR_NUM; i++)
		{
			MSG_PRINT(("MDI PAIR %i:\n",i));
			sampleDisplayCableTestResult(&cableStatus.cableStatus[i],
									&cableStatus.cableLen[i]);
		}
	}

	return GT_OK;
}

