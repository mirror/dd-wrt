/*
 * Handle the Garmin simple text format supported by some Garmins.
 * Tested with the 'Garmin eTrex Legend' device working in 'Text Out' mode.
 *
 * Protocol info from:
 *	 http://www8.garmin.com/support/text_out.html
 *       http://www.garmin.com/support/commProtocol.html
 *
 * Code by: Petr Slansky <slansky@usa.net>
 * all rights abandoned, a thank would be nice if you use this code.
 *
 * -D 3 = packet trace
 * -D 4 = packet details
 * -D 5 = more packet details
 * -D 6 = very excessive details
 *
 * limitations:
 *  very simple protocol, only very basic information
 * TODO
 * do not have from garmin:
 *      pdop
 *      vdop
 *	magnetic variation
 *      satellite information
 *
 * This file is Copyright (c) 2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 *
 */

/***************************************************
Garmin Simple Text Output Format:

The simple text (ASCII) output contains time, position, and velocity data in
the fixed width fields (not delimited) defined in the following table:

    FIELD DESCRIPTION:      WIDTH:  NOTES:
    ----------------------- ------- ------------------------
    Sentence start          1       Always '@'
    ----------------------- ------- ------------------------
   /Year                    2       Last two digits of UTC year
  | ----------------------- ------- ------------------------
  | Month                   2       UTC month, "01".."12"
T | ----------------------- ------- ------------------------
i | Day                     2       UTC day of month, "01".."31"
m | ----------------------- ------- ------------------------
e | Hour                    2       UTC hour, "00".."23"
  | ----------------------- ------- ------------------------
  | Minute                  2       UTC minute, "00".."59"
  | ----------------------- ------- ------------------------
   \Second                  2       UTC second, "00".."59"
    ----------------------- ------- ------------------------
   /Latitude hemisphere     1       'N' or 'S'
  | ----------------------- ------- ------------------------
  | Latitude position       7       WGS84 ddmmmmm, with an implied
  |                                 decimal after the 4th digit
  | ----------------------- ------- ------------------------
  | Longitude hemishpere    1       'E' or 'W'
  | ----------------------- ------- ------------------------
  | Longitude position      8       WGS84 dddmmmmm with an implied
P |                                 decimal after the 5th digit
o | ----------------------- ------- ------------------------
s | Position status         1       'd' if current 2D differential GPS position
i |                                 'D' if current 3D differential GPS position
t |                                 'g' if current 2D GPS position
i |                                 'G' if current 3D GPS position
o |                                 'S' if simulated position
n |                                 '_' if invalid position
  | ----------------------- ------- ------------------------
  | Horizontal posn error   3       EPH in meters
  | ----------------------- ------- ------------------------
  | Altitude sign           1       '+' or '-'
  | ----------------------- ------- ------------------------
  | Altitude                5       Height above or below mean
   \                                sea level in meters
    ----------------------- ------- ------------------------
   /East/West velocity      1       'E' or 'W'
  |     direction
  | ----------------------- ------- ------------------------
  | East/West velocity      4       Meters per second in tenths,
  |     magnitude                   ("1234" = 123.4 m/s)
V | ----------------------- ------- ------------------------
e | North/South velocity    1       'N' or 'S'
l |     direction
o | ----------------------- ------- ------------------------
c | North/South velocity    4       Meters per second in tenths,
i |     magnitude                   ("1234" = 123.4 m/s)
t | ----------------------- ------- ------------------------
y | Vertical velocity       1       'U' or 'D' (up/down)
  |     direction
  | ----------------------- ------- ------------------------
  | Vertical velocity       4       Meters per second in hundredths,
   \    magnitude                   ("1234" = 12.34 m/s)
    ----------------------- ------- ------------------------
    Sentence end            2       Carriage return, '0x0D', and
                                    line feed, '0x0A'
    ----------------------- ------- ------------------------

If a numeric value does not fill its entire field width, the field is padded
with leading '0's (eg. an altitude of 50 meters above MSL will be output as
"+00050").

Any or all of the data in the text sentence (except for the sentence start
and sentence end fields) may be replaced with underscores to indicate
invalid data.

***************************************************/


#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <strings.h>

#include "gpsd.h"

#ifdef GARMINTXT_ENABLE

/* Simple text message is fixed length, 55 chars text data + 2 characters EOL */
/* buffer for text processing */
#define TXT_BUFFER_SIZE 13

/**************************************************************************
 * decode text string to double number, translate prefix to sign
 * return 0: OK
 *       -1: data error
 *       -2: data not valid
 *
 * examples:

 *  gar_decode(cbuf, 9, "EW", 100000.0, &result);
 *  E01412345 -> +14.12345

 *  gar_decode(cbuf, 9, "EW", 100000.0, &result);
 *  W01412345 -> -14.12345

 *  gar_decode(cbuf, 3, "", 10.0, &result);
 *  123 -> +12.3

**************************************************************************/
static int gar_decode(const char *data, const size_t length, const char *prefix, const double dividor,	/*@out@*/
		      double *result)
{
    char buf[10];
    float sign = 1.0;
    int preflen = (int)strlen(prefix);
    int offset = 1;		/* assume one character prefix (E,W,S,N,U,D, etc) */
    long int intresult;

    /* splint is buggy here, thinks buf can be a null pointer */
    /*@ -mustdefine -nullderef -nullpass @*/
    if (length >= sizeof(buf)) {
	gpsd_report(LOG_ERROR, "internal buffer too small\n");
	return -1;
    }

    bzero(buf, (int)sizeof(buf));
    (void)strlcpy(buf, data, length);
    gpsd_report(LOG_RAW + 2, "Decoded string: %s\n", buf);

    if (strchr(buf, '_') != NULL) {
	/* value is not valid, ignore it */
	return -2;
    }

    /* parse prefix */
    do {
	if (preflen == 0) {
	    offset = 0;		/* only number, no prefix */
	    break;
	}
	/* second character in prefix is flag for negative number */
	if (preflen >= 2) {
	    if (buf[0] == prefix[1]) {
		sign = -1.0;
		break;
	    }
	}
	/* first character in prefix is flag for positive number */
	if (preflen >= 1) {
	    if (buf[0] == prefix[0]) {
		sign = 1.0;
		break;
	    }
	}
	gpsd_report(LOG_WARN, "Unexpected char \"%c\" in data \"%s\"\n",
		    buf[0], buf);
	return -1;
    } while (0);

    if (strspn(buf + offset, "0123456789") != length - offset) {
	gpsd_report(LOG_WARN, "Invalid value %s\n", buf);
	return -1;
    }
    /*@ +mustdefine +nullderef +nullpass @*/

    intresult = atol(buf + offset);
    if (intresult == 0L)
	sign = 0.0;		/*  don't create negative zero */

    *result = (double)intresult / dividor * sign;

    return 0;			/* SUCCESS */
}

/**************************************************************************
 * decode integer from string, check if the result is in expected range
 * return 0: OK
 *       -1: data error
 *       -2: data not valid
**************************************************************************/
static int gar_int_decode(const char *data, const size_t length,
			  const unsigned int min, const unsigned int max,
			  /*@out@*/ unsigned int *result)
{
    char buf[6];
    unsigned int res;

    /*@ -mustdefine @*/
    if (length >= sizeof(buf)) {
	gpsd_report(LOG_ERROR, "internal buffer too small\n");
	return -1;
    }

    bzero(buf, (int)sizeof(buf));
    (void)strlcpy(buf, data, length);
    gpsd_report(LOG_RAW + 2, "Decoded string: %s\n", buf);

    if (strchr(buf, '_') != NULL) {
	/* value is not valid, ignore it */
	return -2;
    }

    /*@ -nullpass @*//* splint bug */
    if (strspn(buf, "0123456789") != length) {
	gpsd_report(LOG_WARN, "Invalid value %s\n", buf);
	return -1;
    }

    res = (unsigned)atoi(buf);
    if ((res >= min) && (res <= max)) {
	*result = res;
	return 0;		/* SUCCESS */
    } else {
	gpsd_report(LOG_WARN, "Value %u out of range <%u, %u>\n", res, min,
		    max);
	return -1;
    }
    /*@ +mustdefine +nullpass @*/
}


/**************************************************************************
 *
 * Entry points begin here
 *
 **************************************************************************/

gps_mask_t garmintxt_parse(struct gps_device_t * session)
{
/* parse GARMIN Simple Text sentence, unpack it into a session structure */

    gps_mask_t mask = 0;

    gpsd_report(LOG_PROG, "Garmin Simple Text packet, len %zd: %s\n",
		session->packet.outbuflen, (char*)session->packet.outbuffer);

    if (session->packet.outbuflen < 54) {
	/* trailing CR and LF can be ignored; ('@' + 54x 'DATA' + '\r\n') has length 57 */
	gpsd_report(LOG_WARN, "Message is too short, rejected.\n");
	return ONLINE_SET;
    }

    session->packet.type = GARMINTXT_PACKET;
    /* TAG message as GTXT, Garmin Simple Text Message */
    (void)strlcpy(session->gpsdata.tag, "GTXT", MAXTAGLEN);

    /* only one message, set cycle start */
    session->cycle_end_reliable = true;
    do {
	unsigned int result;
	char *buf = (char *)session->packet.outbuffer + 1;
	gpsd_report(LOG_PROG, "Timestamp: %.12s\n", buf);

	/* year */
	if (0 != gar_int_decode(buf + 0, 2, 0, 99, &result))
	    break;
	session->driver.garmintxt.date.tm_year =
	    (session->context->century + (int)result) - 1900;
	/* month */
	if (0 != gar_int_decode(buf + 2, 2, 1, 12, &result))
	    break;
	session->driver.garmintxt.date.tm_mon = (int)result - 1;
	/* day */
	if (0 != gar_int_decode(buf + 4, 2, 1, 31, &result))
	    break;
	session->driver.garmintxt.date.tm_mday = (int)result;
	/* hour */
	if (0 != gar_int_decode(buf + 6, 2, 0, 23, &result))
	    break;
	session->driver.garmintxt.date.tm_hour = (int)result;	/* mday update?? */
	/* minute */
	if (0 != gar_int_decode(buf + 8, 2, 0, 59, &result))
	    break;
	session->driver.garmintxt.date.tm_min = (int)result;
	/* second */
	/* second value can be even 60, occasional leap second */
	if (0 != gar_int_decode(buf + 10, 2, 0, 60, &result))
	    break;
	session->driver.garmintxt.date.tm_sec = (int)result;
	session->driver.garmintxt.subseconds = 0;
	session->newdata.time =
	    (timestamp_t)mkgmtime(&session->driver.garmintxt.date) +
	    session->driver.garmintxt.subseconds;
	mask |= TIME_SET;
    } while (0);

    /* assume that possition is unknown; if the position is known we will fix status information later */
    session->newdata.mode = MODE_NO_FIX;
    session->gpsdata.status = STATUS_NO_FIX;
    mask |= MODE_SET | STATUS_SET | CLEAR_IS | REPORT_IS;

    /* process position */

    do {
	double lat, lon;
	unsigned int degfrag;
	char status;

	/* Latitude, [NS]ddmmmmm */
	/* decode degrees of Latitude */
	if (0 !=
	    gar_decode((char *)session->packet.outbuffer + 13, 3, "NS", 1.0,
		       &lat))
	    break;
	/* decode minutes of Latitude */
	if (0 !=
	    gar_int_decode((char *)session->packet.outbuffer + 16, 5, 0,
			   99999, &degfrag))
	    break;
	lat += degfrag * 100.0 / 60.0 / 100000.0;
	session->newdata.latitude = lat;

	/* Longitude, [EW]dddmmmmm */
	/* decode degrees of Longitude */
	if (0 !=
	    gar_decode((char *)session->packet.outbuffer + 21, 4, "EW", 1.0,
		       &lon))
	    break;
	/* decode minutes of Longitude */
	if (0 !=
	    gar_int_decode((char *)session->packet.outbuffer + 25, 5, 0,
			   99999, &degfrag))
	    break;
	lon += degfrag * 100.0 / 60.0 / 100000.0;
	session->newdata.longitude = lon;

	/* fix mode, GPS status, [gGdDS_] */
	status = (char)session->packet.outbuffer[30];

	switch (status) {
	case 'G':
	case 'S':		/* 'S' is DEMO mode, assume 3D position */
	    session->newdata.mode = MODE_3D;
	    session->gpsdata.status = STATUS_FIX;
	    break;
	case 'D':
	    session->newdata.mode = MODE_3D;
	    session->gpsdata.status = STATUS_DGPS_FIX;
	    break;
	case 'g':
	    session->newdata.mode = MODE_2D;
	    session->gpsdata.status = STATUS_FIX;
	    break;
	case 'd':
	    session->newdata.mode = MODE_2D;
	    session->gpsdata.status = STATUS_DGPS_FIX;
	    break;
	default:
	    session->newdata.mode = MODE_NO_FIX;
	    session->gpsdata.status = STATUS_NO_FIX;
	}
	mask |= MODE_SET | STATUS_SET | LATLON_SET;
    } while (0);

    /* EPH */
    do {
	double eph;
	if (0 !=
	    gar_decode((char *)session->packet.outbuffer + 31, 3, "", 1.0,
		       &eph))
	    break;
	/* eph is a circular error, sqrt(epx**2 + epy**2) */
	session->newdata.epx = session->newdata.epy =
	    eph * (1 / sqrt(2)) * (GPSD_CONFIDENCE / CEP50_SIGMA);
	mask |= HERR_SET;
    } while (0);

    /* Altitude */
    do {
	double alt;
	if (0 !=
	    gar_decode((char *)session->packet.outbuffer + 34, 6, "+-", 1.0,
		       &alt))
	    break;
	session->newdata.altitude = alt;
	mask |= ALTITUDE_SET;
    } while (0);

    /* Velocity */
    do {
	double ewvel, nsvel, speed, track;
	if (0 !=
	    gar_decode((char *)session->packet.outbuffer + 40, 5, "EW", 10.0,
		       &ewvel))
	    break;
	if (0 !=
	    gar_decode((char *)session->packet.outbuffer + 45, 5, "NS", 10.0,
		       &nsvel))
	    break;
	speed = sqrt(ewvel * ewvel + nsvel * nsvel);	/* is this correct formula? Result is in mps */
	session->newdata.speed = speed;
	track = atan2(ewvel, nsvel) * RAD_2_DEG;	/* is this correct formula? Result is in degrees */
	if (track < 0.0)
	    track += 360.0;
	session->newdata.track = track;
	mask |= SPEED_SET | TRACK_SET;
    } while (0);


    /* Climb (vertical velocity) */
    do {
	double climb;
	if (0 !=
	    gar_decode((char *)session->packet.outbuffer + 50, 5, "UD", 100.0,
		       &climb))
	    break;
	session->newdata.climb = climb;	/* climb in mps */
	mask |= CLIMB_SET;
    } while (0);

    gpsd_report(LOG_DATA,
		"GTXT: time=%.2f, lat=%.2f lon=%.2f alt=%.2f speed=%.2f track=%.2f climb=%.2f exp=%.2f epy=%.2f mode=%d status=%d\n",
		session->newdata.time, session->newdata.latitude,
		session->newdata.longitude, session->newdata.altitude,
		session->newdata.speed, session->newdata.track,
		session->newdata.climb, session->newdata.epx,
		session->newdata.epy, session->newdata.mode,
		session->gpsdata.status);
    return mask;
}

#endif /* GARMINTXT_ENABLE */
