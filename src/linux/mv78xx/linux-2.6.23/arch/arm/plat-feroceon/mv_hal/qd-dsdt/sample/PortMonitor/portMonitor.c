#include <Copyright.h>
/********************************************************************************
* portMonitor.c
*
* DESCRIPTION:
*		This sample shows how to monitor a certain port. Port monitoring is 
*		supported by the ClipperShip device with Egress only monitoring or 
*		Egress and Ingress monitoring.
*
* DEPENDENCIES:
*		Only ClipperShip Family supports this feature.
*
* FILE REVISION NUMBER:
*
* COMMENTS:
*******************************************************************************/

#include "msSample.h"

/*
 *	Enable EgressMonitoring for the monitoredPort.
 *	With this setup, monitoringPort will receive every packet 
 *	which egressed from monitoredPort.
*/
GT_STATUS sampleEgressMonitor(GT_QD_DEV *dev,GT_LPORT monitoredPort, GT_LPORT monitoringPort)
{
	GT_STATUS status;
	GT_U16 pav;

	/*
	 *	Enable EgressMonitoring for the monitoredPort.
	*/
	pav = (1<<monitoringPort) || (1<<monitoredPort);

	if((status = gpavSetPAV(dev,monitoredPort, pav)) != GT_OK)
	{
		MSG_PRINT(("gpavSetPAV return Failed\n"));
		return status;
	}

	return GT_OK;
}

/*
 *	Enable Egress Monitoring and Ingress Monitoring for the monitoredPort.
 *	With this setup, monitoringPort will receive every packet 
 *	which is both from monitoredPort and to monitoredPort.
*/
GT_STATUS samplePortMonitor(GT_QD_DEV *dev,GT_LPORT monitoredPort, GT_LPORT monitoringPort)
{
	GT_STATUS status;
	GT_U16 pav;

	/*
	 *	Enable Egress Monitoring for the monitoredPort.
	*/
	pav = (1<<monitoringPort) || (1<<monitoredPort);

	if((status = gpavSetPAV(dev,monitoredPort, pav)) != GT_OK)
	{
		MSG_PRINT(("gpavSetPAV return Failed\n"));
		return status;
	}

	/*
	 *	Enable Ingress Monitoring for the monitoredPort.
	*/
	if((status = gpavSetIngressMonitor(dev,monitoredPort, GT_TRUE)) != GT_OK)
	{
		MSG_PRINT(("gpavSetIngressMonitor return Failed\n"));
		return status;
	}

	return GT_OK;
}


/*
 *	Disable Egress Monitoring and Ingress Monitoring for the monitoredPort.
*/
GT_STATUS sampleDisablePortMonitor(GT_QD_DEV *dev,GT_LPORT monitoredPort)
{
	GT_STATUS status;
	GT_U16 pav;

	/*
	 *	Disable Egress Monitoring for the monitoredPort.
	*/
	pav = (1<<monitoredPort);

	if((status = gpavSetPAV(dev,monitoredPort, pav)) != GT_OK)
	{
		MSG_PRINT(("gpavSetPAV return Failed\n"));
		return status;
	}

	/*
	 *	Disable Ingress Monitoring for the monitoredPort.
	*/
	if((status = gpavSetIngressMonitor(dev,monitoredPort, GT_FALSE)) != GT_OK)
	{
		MSG_PRINT(("gpavSetIngressMonitor return Failed\n"));
		return status;
	}

	return GT_OK;
}

