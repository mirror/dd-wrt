========================================================================
		RMON Counters
========================================================================

This sample shows how to read/clear RMON counter in the device.
Please notes that there are three groups of RMON counters in Marvell SOHO Switchs.
Each group has different set of counters. Therefore it is necessary to find out
which group the switch device belongs to.

Group for GT_STATS_COUNTERS : 88E6021, 88E6063, and 88E6083
Group for GT_STATS_COUNTERS2 : 88E6183
Group for GT_STATS_COUNTERS3 : 88E6093, 88E6095, 88E6185, and 88E6065

rmon.c
	sampleClearRMONCounter
		shows how to reset RMON counter for the given port

	sampleGetRMONCounter
		shows how to read RMON counter for each port
        this routine is for the devices that use GT_STATS_COUNTERS.

	sampleGetRMONCounter2
		shows how to read RMON counter for each port
        this routine is for the devices that use GT_STATS_COUNTERS2.

	sampleGetRMONCounter3
		shows how to read RMON counter for each port
        this routine is for the devices that use GT_STATS_COUNTERS3.




