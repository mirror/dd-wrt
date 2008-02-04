#include <Copyright.h>
/********************************************************************************
* rmon.c
*
* DESCRIPTION:
*		This sample shows how to read/clear RMON counter in the device
*
* DEPENDENCIES:   NONE.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#include "msSample.h"


/*
 * This sample is for every device that support RMON counter.
*/
GT_STATUS sampleClearRMONCounter(GT_QD_DEV *dev,GT_LPORT port)
{
	GT_STATUS status;

	if((status = gstatsFlushPort(dev,port)) != GT_OK)
	{
		MSG_PRINT(("gstatsFlushPort returned fail (%#x).\n",status));
		return status;
	}

	return GT_OK;
}

/*
 * This sample is for 88E6021, 88E6063, and 88E6083.
*/
void sampleDisplayCounter(GT_STATS_COUNTER_SET *statsCounter)
{
	MSG_PRINT(("InUnicasts    %08i    ", statsCounter->InUnicasts));
	MSG_PRINT(("InBroadcasts  %08i   \n", statsCounter->InBroadcasts));
	MSG_PRINT(("InPause       %08i    ", statsCounter->InPause));
	MSG_PRINT(("InMulticasts  %08i   \n", statsCounter->InMulticasts));
	MSG_PRINT(("InFCSErr      %08i    ", statsCounter->InFCSErr));
	MSG_PRINT(("AlignErr      %08i   \n", statsCounter->AlignErr));
	MSG_PRINT(("InGoodOctets  %08i    ", statsCounter->InGoodOctets));
	MSG_PRINT(("InBadOctets   %08i   \n", statsCounter->InBadOctets));
	MSG_PRINT(("Undersize     %08i    ", statsCounter->Undersize));
	MSG_PRINT(("Fragments     %08i   \n", statsCounter->Fragments));
	MSG_PRINT(("In64Octets    %08i    ", statsCounter->In64Octets));
	MSG_PRINT(("In127Octets   %08i   \n", statsCounter->In127Octets));
	MSG_PRINT(("In255Octets   %08i    ", statsCounter->In255Octets));
	MSG_PRINT(("In511Octets   %08i   \n", statsCounter->In511Octets));
	MSG_PRINT(("In1023Octets  %08i    ", statsCounter->In1023Octets));
	MSG_PRINT(("InMaxOctets   %08i   \n", statsCounter->InMaxOctets));
	MSG_PRINT(("Jabber        %08i    ", statsCounter->Jabber));
	MSG_PRINT(("Oversize      %08i   \n", statsCounter->Oversize));
	MSG_PRINT(("InDiscards    %08i    ", statsCounter->InDiscards));
	MSG_PRINT(("Filtered      %08i   \n", statsCounter->Filtered));
	MSG_PRINT(("OutUnicasts   %08i    ", statsCounter->OutUnicasts));
	MSG_PRINT(("OutBroadcasts %08i   \n", statsCounter->OutBroadcasts));
	MSG_PRINT(("OutPause      %08i    ", statsCounter->OutPause));
	MSG_PRINT(("OutMulticasts %08i   \n", statsCounter->OutMulticasts));
	MSG_PRINT(("OutFCSErr     %08i    ", statsCounter->OutFCSErr));
	MSG_PRINT(("OutGoodOctets %08i   \n", statsCounter->OutGoodOctets));
	MSG_PRINT(("Out64Octets   %08i    ", statsCounter->Out64Octets));
	MSG_PRINT(("Out127Octets  %08i   \n", statsCounter->Out127Octets));
	MSG_PRINT(("Out255Octets  %08i    ", statsCounter->Out255Octets));
	MSG_PRINT(("Out511Octets  %08i   \n", statsCounter->Out511Octets));
	MSG_PRINT(("Out1023Octets %08i    ", statsCounter->Out1023Octets));
	MSG_PRINT(("OutMaxOctets  %08i   \n", statsCounter->OutMaxOctets));
	MSG_PRINT(("Collisions    %08i    ", statsCounter->Collisions));
	MSG_PRINT(("Late          %08i   \n", statsCounter->Late));
	MSG_PRINT(("Excessive     %08i    ", statsCounter->Excessive));
	MSG_PRINT(("Multiple      %08i   \n", statsCounter->Multiple));
	MSG_PRINT(("Single        %08i    ", statsCounter->Single));
	MSG_PRINT(("Deferred      %08i   \n", statsCounter->Deferred));
	MSG_PRINT(("OutDiscards   %08i   \n", statsCounter->OutDiscards));
}

/*
 * This sample is for 88E6021, 88E6063, and 88E6083.
*/
GT_STATUS sampleGetRMONCounter(GT_QD_DEV *dev)
{
	GT_STATUS status;
	GT_LPORT port;
	GT_STATS_COUNTER_SET	statsCounterSet;

	for(port=0; port<dev->numOfPorts; port++)
	{
		MSG_PRINT(("Port %i :\n",port));

		if((status = gstatsGetPortAllCounters(dev,port,&statsCounterSet)) != GT_OK)
		{
			MSG_PRINT(("gstatsGetPortAllCounters returned fail (%#x).\n",status));
			return status;
		}

		sampleDisplayCounter(&statsCounterSet);

	}

	return GT_OK;
}


/*
 * This sample is for 88E6183
*/
void sampleDisplayCounter2(GT_STATS_COUNTER_SET2 *statsCounter)
{
	MSG_PRINT(("InGoodOctetsHi  %08i    ", statsCounter->InGoodOctetsHi));
	MSG_PRINT(("InGoodOctetsLo  %08i   \n", statsCounter->InGoodOctetsLo));
	MSG_PRINT(("InBadOctets     %08i    ", statsCounter->InBadOctets));
	MSG_PRINT(("OutDiscards     %08i   \n", statsCounter->OutDiscards));
	MSG_PRINT(("InGoodFrames    %08i    ", statsCounter->InGoodFrames));
	MSG_PRINT(("InBadFrames     %08i   \n", statsCounter->InBadFrames));
	MSG_PRINT(("InBroadcasts    %08i    ", statsCounter->InBroadcasts));
	MSG_PRINT(("InMulticasts    %08i   \n", statsCounter->InMulticasts));
	MSG_PRINT(("64Octets        %08i    ", statsCounter->Octets64));
	MSG_PRINT(("127Octets       %08i   \n", statsCounter->Octets127));
	MSG_PRINT(("255Octets       %08i    ", statsCounter->Octets255));
	MSG_PRINT(("511Octets       %08i   \n", statsCounter->Octets511));
	MSG_PRINT(("1023Octets      %08i    ", statsCounter->Octets1023));
	MSG_PRINT(("MaxOctets       %08i   \n", statsCounter->OctetsMax));
	MSG_PRINT(("OutOctetsHi     %08i    ", statsCounter->OutOctetsHi));
	MSG_PRINT(("OutOctetsLo     %08i   \n", statsCounter->OutOctetsLo));
	MSG_PRINT(("OutFrames       %08i    ", statsCounter->OutFrames));
	MSG_PRINT(("Excessive       %08i   \n", statsCounter->Excessive));
	MSG_PRINT(("OutMulticasts   %08i    ", statsCounter->OutMulticasts));
	MSG_PRINT(("OutBroadcasts   %08i    ", statsCounter->OutBroadcasts));
	MSG_PRINT(("InBadMACCtrl    %08i    ", statsCounter->InBadMACCtrl));
	MSG_PRINT(("OutPause        %08i   \n", statsCounter->OutPause));
	MSG_PRINT(("InPause         %08i    ", statsCounter->InPause));
	MSG_PRINT(("InDiscards      %08i   \n", statsCounter->InDiscards));
	MSG_PRINT(("Undersize       %08i    ", statsCounter->Undersize));
	MSG_PRINT(("Fragments       %08i   \n", statsCounter->Fragments));
	MSG_PRINT(("Oversize        %08i    ", statsCounter->Oversize));
	MSG_PRINT(("Jabber          %08i   \n", statsCounter->Jabber));
	MSG_PRINT(("MACRcvErr       %08i    ", statsCounter->MACRcvErr));
	MSG_PRINT(("InFCSErr        %08i   \n", statsCounter->InFCSErr));
	MSG_PRINT(("Collisions      %08i    ", statsCounter->Collisions));
	MSG_PRINT(("Late            %08i   \n", statsCounter->Late));
}

/*
 * This sample is for 88E6183
*/
GT_STATUS sampleGetRMONCounter2(GT_QD_DEV *dev)
{
	GT_STATUS status;
	GT_LPORT port;
	GT_STATS_COUNTER_SET2 statsCounterSet;

	for(port=0; port<dev->numOfPorts; port++)
	{
		MSG_PRINT(("Port %i :\n",port));

		if((status = gstatsGetPortAllCounters2(dev,port,&statsCounterSet)) != GT_OK)
		{
			MSG_PRINT(("gstatsGetPortAllCounters2 returned fail (%#x).\n",status));
			return status;
		}

		sampleDisplayCounter2(&statsCounterSet);

	}

	return GT_OK;
}



/*
 * This sample is for 88E6093, 88E6095, 88E6185, and 88E6065
*/
void sampleDisplayCounter3(GT_STATS_COUNTER_SET3 *statsCounter)
{
	MSG_PRINT(("InGoodOctetsLo  %08i    ", statsCounter->InGoodOctetsLo));
	MSG_PRINT(("InGoodOctetsHi  %08i   \n", statsCounter->InGoodOctetsHi));
	MSG_PRINT(("InBadOctets     %08i    ", statsCounter->InBadOctets));
	MSG_PRINT(("OutFCSErr       %08i   \n", statsCounter->OutFCSErr));
	MSG_PRINT(("InUnicasts      %08i    ", statsCounter->InUnicasts));
	MSG_PRINT(("Deferred        %08i   \n", statsCounter->Deferred));
	MSG_PRINT(("InBroadcasts    %08i    ", statsCounter->InBroadcasts));
	MSG_PRINT(("InMulticasts    %08i   \n", statsCounter->InMulticasts));
	MSG_PRINT(("64Octets        %08i    ", statsCounter->Octets64));
	MSG_PRINT(("127Octets       %08i   \n", statsCounter->Octets127));
	MSG_PRINT(("255Octets       %08i    ", statsCounter->Octets255));
	MSG_PRINT(("511Octets       %08i   \n", statsCounter->Octets511));
	MSG_PRINT(("1023Octets      %08i    ", statsCounter->Octets1023));
	MSG_PRINT(("MaxOctets       %08i   \n", statsCounter->OctetsMax));
	MSG_PRINT(("OutOctetsLo     %08i    ", statsCounter->OutOctetsLo));
	MSG_PRINT(("OutOctetsHi     %08i   \n", statsCounter->OutOctetsHi));
	MSG_PRINT(("OutUnicasts     %08i    ", statsCounter->OutUnicasts));
	MSG_PRINT(("Excessive       %08i   \n", statsCounter->Excessive));
	MSG_PRINT(("OutMulticasts   %08i    ", statsCounter->OutMulticasts));
	MSG_PRINT(("OutBroadcasts   %08i   \n", statsCounter->OutBroadcasts));
	MSG_PRINT(("Single          %08i    ", statsCounter->Single));
	MSG_PRINT(("OutPause        %08i   \n", statsCounter->OutPause));
	MSG_PRINT(("InPause         %08i    ", statsCounter->InPause));
	MSG_PRINT(("Multiple        %08i   \n", statsCounter->Multiple));
	MSG_PRINT(("Undersize       %08i    ", statsCounter->Undersize));
	MSG_PRINT(("Fragments       %08i   \n", statsCounter->Fragments));
	MSG_PRINT(("Oversize        %08i    ", statsCounter->Oversize));
	MSG_PRINT(("Jabber          %08i   \n", statsCounter->Jabber));
	MSG_PRINT(("InMACRcvErr     %08i    ", statsCounter->InMACRcvErr));
	MSG_PRINT(("InFCSErr        %08i   \n", statsCounter->InFCSErr));
	MSG_PRINT(("Collisions      %08i    ", statsCounter->Collisions));
	MSG_PRINT(("Late            %08i   \n", statsCounter->Late));
}


/*
 * This sample is for 88E6093, 88E6095, 88E6185, and 88E6065
*/
GT_STATUS sampleGetRMONCounter3(GT_QD_DEV *dev)
{
	GT_STATUS status;
	GT_LPORT port;
	GT_STATS_COUNTER_SET3 statsCounterSet;

	for(port=0; port<dev->numOfPorts; port++)
	{
		MSG_PRINT(("Port %i :\n",port));

		if((status = gstatsGetPortAllCounters3(dev,port,&statsCounterSet)) != GT_OK)
		{
			MSG_PRINT(("gstatsGetPortAllCounters3 returned fail (%#x).\n",status));
			return status;
		}

		sampleDisplayCounter3(&statsCounterSet);

	}

	return GT_OK;
}

