/* This code is generated.  Do not hand-hack it! */

/*
 * Also, beware that it is something of a CPU hog when called on every packet.
 * Try to write guards so it is only called at higher log levels.
 */

#include <stdio.h>
#include <string.h>

#include "gpsd.h"

const char *gps_maskdump(gps_mask_t set)
{
    static char buf[190];
    const struct {
	gps_mask_t      mask;
	const char      *name;
    } *sp, names[] = {
	{ONLINE_SET,	"ONLINE"},
	{TIME_SET,	"TIME"},
	{TIMERR_SET,	"TIMERR"},
	{LATLON_SET,	"LATLON"},
	{ALTITUDE_SET,	"ALTITUDE"},
	{SPEED_SET,	"SPEED"},
	{TRACK_SET,	"TRACK"},
	{CLIMB_SET,	"CLIMB"},
	{STATUS_SET,	"STATUS"},
	{MODE_SET,	"MODE"},
	{DOP_SET,	"DOP"},
	{HERR_SET,	"HERR"},
	{VERR_SET,	"VERR"},
	{ATTITUDE_SET,	"ATTITUDE"},
	{SATELLITE_SET,	"SATELLITE"},
	{SPEEDERR_SET,	"SPEEDERR"},
	{TRACKERR_SET,	"TRACKERR"},
	{CLIMBERR_SET,	"CLIMBERR"},
	{DEVICE_SET,	"DEVICE"},
	{DEVICELIST_SET,	"DEVICELIST"},
	{DEVICEID_SET,	"DEVICEID"},
	{RTCM2_SET,	"RTCM2"},
	{RTCM3_SET,	"RTCM3"},
	{AIS_SET,	"AIS"},
	{PACKET_SET,	"PACKET"},
	{SUBFRAME_SET,	"SUBFRAME"},
	{GST_SET,	"GST"},
	{VERSION_SET,	"VERSION"},
	{POLICY_SET,	"POLICY"},
	{LOGMESSAGE_SET,	"LOGMESSAGE"},
	{ERROR_SET,	"ERROR"},
	{RAW_IS,	"RAW"},
	{USED_IS,	"USED"},
	{DRIVER_IS,	"DRIVER"},
	{CLEAR_IS,	"CLEAR"},
	{REPORT_IS,	"REPORT"},
	{NODATA_IS,	"NODATA"},
	{PPSTIME_IS,	"PPSTIME"},
	{PERR_IS,	"PERR"},
	{PASSTHROUGH_IS,	"PASSTHROUGH"},
    };

    memset(buf, '\0', sizeof(buf));
    buf[0] = '{';
    for (sp = names; sp < names + sizeof(names)/sizeof(names[0]); sp++)
	if ((set & sp->mask)!=0) {
	    (void)strlcat(buf, sp->name, sizeof(buf));
	    (void)strlcat(buf, "|", sizeof(buf));
	}
    if (buf[1] != '\0')
	buf[strlen(buf)-1] = '\0';
    (void)strlcat(buf, "}", sizeof(buf));
    return buf;
}

