#include <Copyright.h>
/********************************************************************************
* trailer.c
*
* DESCRIPTION:
*		This sample shows how to enable/disable CPU port's ingress and egress 
*		Trailer mode.
*
* DEPENDENCIES:   NONE.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#include "msSample.h"

GT_STATUS sampleCPUTrailerEnable(GT_QD_DEV *dev, GT_BOOL en)
{
	GT_STATUS status;
	GT_INGRESS_MODE	inMode;

	if (en)	/* Enable Trailer Mode */
	{
		inMode = GT_TRAILER_INGRESS;
	}
	else
	{
		inMode = GT_UNMODIFY_INGRESS;
	}

	/*
	 *	Enable CPU's Ingress Trailer 
	*/
	if((status = gprtSetIngressMode(dev,dev->cpuPortNum, inMode)) != GT_OK)
	{
		MSG_PRINT(("gprtSetIngressMode return Failed\n"));
		return status;
	}

	/*
	 *	Enable CPU's Egress Trailer 
	*/
	if((status = gprtSetTrailerMode(dev,dev->cpuPortNum, en)) != GT_OK)
	{
		MSG_PRINT(("gprtSetTrailerMode return Failed\n"));
		return status;
	}

	return GT_OK;
}
