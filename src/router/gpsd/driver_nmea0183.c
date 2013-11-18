/*
 * This file is Copyright (c) 2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "gpsd.h"

#ifdef NMEA_ENABLE
/**************************************************************************
 *
 * Parser helpers begin here
 *
 **************************************************************************/

static void do_lat_lon(char *field[], struct gps_fix_t *out)
/* process a pair of latitude/longitude fields starting at field index BEGIN */
{
    double d, m;
    char str[20], *p;

    if (*(p = field[0]) != '\0') {
	double lat;
	(void)strlcpy(str, p, sizeof(str));
	lat = atof(str);
	m = 100.0 * modf(lat / 100.0, &d);
	lat = d + m / 60.0;
	p = field[1];
	if (*p == 'S')
	    lat = -lat;
	out->latitude = lat;
    }
    if (*(p = field[2]) != '\0') {
	double lon;
	(void)strlcpy(str, p, sizeof(str));
	lon = atof(str);
	m = 100.0 * modf(lon / 100.0, &d);
	lon = d + m / 60.0;

	p = field[3];
	if (*p == 'W')
	    lon = -lon;
	out->longitude = lon;
    }
}

/**************************************************************************
 *
 * Scary timestamp fudging begins here
 *
 * Four sentences, GGA and GLL and RMC and ZDA, contain timestamps.
 * GGA/GLL/RMC timestamps look like hhmmss.ss, with the trailing .ss
 * part optional.  RMC has a date field, in the format ddmmyy.  ZDA
 * has separate fields for day/month/year, with a 4-digit year.  This
 * means that for RMC we must supply a century and for GGA and GLL we
 * must supply a century, year, and day.  We get the missing data from
 * a previous RMC or ZDA; century in RMC is supplied from the daemon's
 * context (initialized at startup time) if there has been no previous
 * ZDA.
 *
 **************************************************************************/

#define DD(s)	((int)((s)[0]-'0')*10+(int)((s)[1]-'0'))

static void merge_ddmmyy(char *ddmmyy, struct gps_device_t *session)
/* sentence supplied ddmmyy, but no century part */
{
    int yy = DD(ddmmyy + 4);
    int mon = DD(ddmmyy + 2);
    int mday = DD(ddmmyy);
    int year;

    /* check for century wrap */
    if (session->driver.nmea.date.tm_year % 100 == 99 && yy == 0) {
	session->context->century += 100;
	gpsd_report(LOG_WARN, "century rollover detected.\n");
    }
    year = (session->context->century + yy);

    if ( (1 > mon ) || (12 < mon ) ) {
	gpsd_report(LOG_WARN, "merge_ddmmyy(%s), malformed month\n",  ddmmyy);
    } else if ( (1 > mday ) || (31 < mday ) ) {
	gpsd_report(LOG_WARN, "merge_ddmmyy(%s), malformed day\n",  ddmmyy);
    } else {
	gpsd_report(LOG_DATA, "merge_ddmmyy(%s) sets year %d\n",
		    ddmmyy, year);
	session->driver.nmea.date.tm_year = year - 1900;
	session->driver.nmea.date.tm_mon = mon - 1;
	session->driver.nmea.date.tm_mday = mday;
    }
}

static void merge_hhmmss(char *hhmmss, struct gps_device_t *session)
/* update from a UTC time */
{
    int old_hour = session->driver.nmea.date.tm_hour;

    session->driver.nmea.date.tm_hour = DD(hhmmss);
    if (session->driver.nmea.date.tm_hour < old_hour)	/* midnight wrap */
	session->driver.nmea.date.tm_mday++;
    session->driver.nmea.date.tm_min = DD(hhmmss + 2);
    session->driver.nmea.date.tm_sec = DD(hhmmss + 4);
    session->driver.nmea.subseconds =
	safe_atof(hhmmss + 4) - session->driver.nmea.date.tm_sec;
}

static void register_fractional_time(const char *tag, const char *fld,
				     struct gps_device_t *session)
{
    if (fld[0] != '\0') {
	session->driver.nmea.last_frac_time =
	    session->driver.nmea.this_frac_time;
	session->driver.nmea.this_frac_time = safe_atof(fld);
	session->driver.nmea.latch_frac_time = true;
	gpsd_report(LOG_DATA, "%s: registers fractional time %.2f\n",
		    tag, session->driver.nmea.this_frac_time);
    }
}

/**************************************************************************
 *
 * Compare GPS timestamps for equality.  Depends on the fact that the
 * timestamp granularity of GPS is 1/100th of a second.  Use this to avoid
 * naive float comparisons.
 *
 **************************************************************************/

#define GPS_TIME_EQUAL(a, b) (fabs((a) - (b)) < 0.01)

/**************************************************************************
 *
 * NMEA sentence handling begins here
 *
 **************************************************************************/

static gps_mask_t processGPRMC(int count, char *field[],
			       struct gps_device_t *session)
/* Recommend Minimum Course Specific GPS/TRANSIT Data */
{
    /*
     * RMC,225446.33,A,4916.45,N,12311.12,W,000.5,054.7,191194,020.3,E,A*68
     * 1     225446.33    Time of fix 22:54:46 UTC
     * 2     A          Status of Fix: A = Autonomous, valid;
     * D = Differential, valid; V = invalid
     * 3,4   4916.45,N    Latitude 49 deg. 16.45 min North
     * 5,6   12311.12,W   Longitude 123 deg. 11.12 min West
     * 7     000.5      Speed over ground, Knots
     * 8     054.7      Course Made Good, True north
     * 9     181194       Date of fix  18 November 1994
     * 10,11 020.3,E      Magnetic variation 20.3 deg East
     * 12    A      FAA mode indicator (NMEA 2.3 and later)
     * A=autonomous, D=differential, E=Estimated,
     * N=not valid, S=Simulator, M=Manual input mode
     * *68        mandatory nmea_checksum
     *
     * * SiRF chipsets don't return either Mode Indicator or magnetic variation.
     */
    gps_mask_t mask = 0;

    if (strcmp(field[2], "V") == 0) {
	/* copes with Magellan EC-10X, see below */
	if (session->gpsdata.status != STATUS_NO_FIX) {
	    session->gpsdata.status = STATUS_NO_FIX;
	    mask |= STATUS_SET;
	}
	if (session->newdata.mode >= MODE_2D) {
	    session->newdata.mode = MODE_NO_FIX;
	    mask |= MODE_SET;
	}
	/* set something nz, so it won't look like an unknown sentence */
	mask |= ONLINE_SET;
    } else if (strcmp(field[2], "A") == 0) {
	/*
	 * The MTK3301, Royaltek RGM-3800, and possibly other
	 * devices deliver bogus time values when the navigation
	 * warning bit is set.
	 */
	if (count > 9 && field[1][0] != '\0' && field[9][0] != '\0') {
	    merge_hhmmss(field[1], session);
	    merge_ddmmyy(field[9], session);
	    mask |= TIME_SET;
	    register_fractional_time(field[0], field[1], session);
	}
	do_lat_lon(&field[3], &session->newdata);
	mask |= LATLON_SET;
	session->newdata.speed = safe_atof(field[7]) * KNOTS_TO_MPS;
	session->newdata.track = safe_atof(field[8]);
	mask |= (TRACK_SET | SPEED_SET);
	/*
	 * This copes with GPSes like the Magellan EC-10X that *only* emit
	 * GPRMC. In this case we set mode and status here so the client
	 * code that relies on them won't mistakenly believe it has never
	 * received a fix.
	 */
	if (session->gpsdata.status == STATUS_NO_FIX) {
	    session->gpsdata.status = STATUS_FIX;	/* could be DGPS_FIX, we can't tell */
	    mask |= STATUS_SET;
	}
	if (session->newdata.mode < MODE_2D) {
	    session->newdata.mode = MODE_2D;
	    mask |= MODE_SET;
	}
    }

    gpsd_report(LOG_DATA,
		"RMC: ddmmyy=%s hhmmss=%s lat=%.2f lon=%.2f "
		"speed=%.2f track=%.2f mode=%d status=%d\n",
		field[9], field[1],
		session->newdata.latitude,
		session->newdata.longitude,
		session->newdata.speed,
		session->newdata.track,
		session->newdata.mode,
		session->gpsdata.status);
    return mask;
}

static gps_mask_t processGPGLL(int count, char *field[],
			       struct gps_device_t *session)
/* Geographic position - Latitude, Longitude */
{
    /* Introduced in NMEA 3.0.
     *
     * $GPGLL,4916.45,N,12311.12,W,225444,A,A*5C
     *
     * 1,2: 4916.46,N    Latitude 49 deg. 16.45 min. North
     * 3,4: 12311.12,W   Longitude 123 deg. 11.12 min. West
     * 5:   225444       Fix taken at 22:54:44 UTC
     * 6:   A            Data valid
     * 7:   A            Autonomous mode
     * 8:   *5C          Mandatory NMEA checksum
     *
     * 1,2 Latitude, N (North) or S (South)
     * 3,4 Longitude, E (East) or W (West)
     * 5 UTC of position
     * 6 A=Active, V=Void
     * 7 Mode Indicator
     * A = Autonomous mode
     * D = Differential Mode
     * E = Estimated (dead-reckoning) mode
     * M = Manual Input Mode
     * S = Simulated Mode
     * N = Data Not Valid
     *
     * I found a note at <http://www.secoh.ru/windows/gps/nmfqexep.txt>
     * indicating that the Garmin 65 does not return time and status.
     * SiRF chipsets don't return the Mode Indicator.
     * This code copes gracefully with both quirks.
     *
     * Unless you care about the FAA indicator, this sentence supplies nothing
     * that GPRMC doesn't already.  But at least one Garmin GPS -- the 48
     * actually ships updates in GLL that aren't redundant.
     */
    char *status = field[7];
    gps_mask_t mask = 0;

    if (field[5][0] != '\0') {
	merge_hhmmss(field[5], session);
	register_fractional_time(field[0], field[5], session);
	if (session->driver.nmea.date.tm_year == 0)
	    gpsd_report(LOG_WARN,
			"can't use GLL time until after ZDA or RMC has supplied a year.\n");
	else {
	    mask = TIME_SET;
	}
    }
    if (strcmp(field[6], "A") == 0 && (count < 8 || *status != 'N')) {
	int newstatus;

	do_lat_lon(&field[1], &session->newdata);
	mask |= LATLON_SET;
	if (count >= 8 && *status == 'D')
	    newstatus = STATUS_DGPS_FIX;	/* differential */
	else
	    newstatus = STATUS_FIX;
	/*
	 * This is a bit dodgy.  Technically we shouldn't set the mode
	 * bit until we see GSA.  But it may be later in the cycle,
	 * some devices like the FV-18 don't send it by default, and
	 * elsewhere in the code we want to be able to test for the
	 * presence of a valid fix with mode > MODE_NO_FIX.
	 */
	if (session->newdata.mode < MODE_2D) {
	    session->newdata.mode = MODE_2D;
	    mask |= MODE_SET;
	}
	session->gpsdata.status = newstatus;
	mask |= STATUS_SET;
    }

    gpsd_report(LOG_DATA,
		"GLL: hhmmss=%s lat=%.2f lon=%.2f mode=%d status=%d\n",
		field[5],
		session->newdata.latitude,
		session->newdata.longitude,
		session->newdata.mode,
		session->gpsdata.status);
    return mask;
}

static gps_mask_t processGPGGA(int c UNUSED, char *field[],
			       struct gps_device_t *session)
/* Global Positioning System Fix Data */
{
    /*
     * GGA,123519,4807.038,N,01131.324,E,1,08,0.9,545.4,M,46.9,M, , *42
     * 1     123519       Fix taken at 12:35:19 UTC
     * 2,3   4807.038,N   Latitude 48 deg 07.038' N
     * 4,5   01131.324,E  Longitude 11 deg 31.324' E
     * 6         1            Fix quality: 0 = invalid, 1 = GPS, 2 = DGPS,
     * 3=PPS (Precise Position Service),
     * 4=RTK (Real Time Kinematic) with fixed integers,
     * 5=Float RTK, 6=Estimated, 7=Manual, 8=Simulator
     * 7     08       Number of satellites being tracked
     * 8     0.9              Horizontal dilution of position
     * 9,10  545.4,M      Altitude, Metres above mean sea level
     * 11,12 46.9,M       Height of geoid (mean sea level) above WGS84
     * ellipsoid, in Meters
     * (empty field) time in seconds since last DGPS update
     * (empty field) DGPS station ID number (0000-1023)
     */
    gps_mask_t mask;

    session->gpsdata.status = atoi(field[6]);
    mask = STATUS_SET;
    /*
     * There are some receivers (the Trimble Placer 450 is an example) that
     * don't ship a GSA with mode 1 when they lose satellite lock. Instead
     * they just keep reporting GGA and GSA on subsequent cycles with the
     * timestamp not advancing and a bogus mode.  On the assumption that GGA
     * is only issued once per cycle we can detect this here (it would be
     * nicer to do it on GSA but GSA has no timestamp).
     */
    session->driver.nmea.latch_mode = strncmp(field[1],
					      session->driver.nmea.last_gga_timestamp,
					      sizeof(session->driver.nmea.last_gga_timestamp))==0;
    if (session->driver.nmea.latch_mode) {
	session->gpsdata.status = STATUS_NO_FIX;
	session->newdata.mode = MODE_NO_FIX;
    } else
	(void)strlcpy(session->driver.nmea.last_gga_timestamp,
		       field[1],
		       sizeof(session->driver.nmea.last_gga_timestamp));
    /* if we have a fix and the mode latch is off, go... */
    if (session->gpsdata.status > STATUS_NO_FIX) {
	char *altitude;

	merge_hhmmss(field[1], session);
	register_fractional_time(field[0], field[1], session);
	if (session->driver.nmea.date.tm_year == 0)
	    gpsd_report(LOG_WARN,
			"can't use GGA time until after ZDA or RMC has supplied a year.\n");
	else {
	    mask |= TIME_SET;
	}
	do_lat_lon(&field[2], &session->newdata);
	mask |= LATLON_SET;
	session->gpsdata.satellites_used = atoi(field[7]);
	altitude = field[9];
	/*
	 * SiRF chipsets up to version 2.2 report a null altitude field.
	 * See <http://www.sirf.com/Downloads/Technical/apnt0033.pdf>.
	 * If we see this, force mode to 2D at most.
	 */
	if (altitude[0] == '\0') {
	    if (session->newdata.mode > MODE_2D) {
		session->newdata.mode = MODE_2D;
                mask |= MODE_SET;
	    }
	} else {
	    session->newdata.altitude = safe_atof(altitude);
	    mask |= ALTITUDE_SET;
	    /*
	     * This is a bit dodgy.  Technically we shouldn't set the mode
	     * bit until we see GSA.  But it may be later in the cycle,
	     * some devices like the FV-18 don't send it by default, and
	     * elsewhere in the code we want to be able to test for the
	     * presence of a valid fix with mode > MODE_NO_FIX.
	     */
	    if (session->newdata.mode < MODE_3D) {
		session->newdata.mode = MODE_3D;
		mask |= MODE_SET;
	    }
	}
	if (strlen(field[11]) > 0) {
	    session->gpsdata.separation = safe_atof(field[11]);
	} else {
	    session->gpsdata.separation =
		wgs84_separation(session->newdata.latitude,
				 session->newdata.longitude);
	}
    }
    gpsd_report(LOG_DATA,
		"GGA: hhmmss=%s lat=%.2f lon=%.2f alt=%.2f mode=%d status=%d\n",
		field[1],
		session->newdata.latitude,
		session->newdata.longitude,
		session->newdata.altitude,
		session->newdata.mode,
		session->gpsdata.status);
    return mask;
}


static gps_mask_t processGPGST(int count, char *field[], struct gps_device_t *session)
/* GST - GPS Pseudorange Noise Statistics */
{
    /*
     * GST,hhmmss.ss,x,x,x,x,x,x,x,*hh
     * 1 TC time of associated GGA fix
     * 2 Total RMS standard deviation of ranges inputs to the navigation solution
     * 3 Standard deviation (meters) of semi-major axis of error ellipse
     * 4 Standard deviation (meters) of semi-minor axis of error ellipse
     * 5 Orientation of semi-major axis of error ellipse (true north degrees)
     * 6 Standard deviation (meters) of latitude error
     * 7 Standard deviation (meters) of longitude error
     * 8 Standard deviation (meters) of altitude error
     * 9 Checksum
*/
    if (count < 8) {
      return 0;
    }

#define PARSE_FIELD(n) (*field[n]!='\0' ? safe_atof(field[n]) : NAN)
    session->gpsdata.gst.utctime             = PARSE_FIELD(1);
    session->gpsdata.gst.rms_deviation       = PARSE_FIELD(2);
    session->gpsdata.gst.smajor_deviation    = PARSE_FIELD(3);
    session->gpsdata.gst.sminor_deviation    = PARSE_FIELD(4);
    session->gpsdata.gst.smajor_orientation  = PARSE_FIELD(5);
    session->gpsdata.gst.lat_err_deviation   = PARSE_FIELD(6);
    session->gpsdata.gst.lon_err_deviation   = PARSE_FIELD(7);
    session->gpsdata.gst.alt_err_deviation   = PARSE_FIELD(8);
#undef PARSE_FIELD
    register_fractional_time(field[0], field[1], session);

    gpsd_report(LOG_DATA,
		"GST: utc = %.2f, rms = %.2f, maj = %.2f, min = %.2f, ori = %.2f, lat = %.2f, lon = %.2f, alt = %.2f\n",
                session->gpsdata.gst.utctime,
                session->gpsdata.gst.rms_deviation,
                session->gpsdata.gst.smajor_deviation,
                session->gpsdata.gst.sminor_deviation,
                session->gpsdata.gst.smajor_orientation,
                session->gpsdata.gst.lat_err_deviation,
                session->gpsdata.gst.lon_err_deviation,
                session->gpsdata.gst.alt_err_deviation);

    return GST_SET | ONLINE_SET;
}


static gps_mask_t processGPGSA(int count, char *field[],
			       struct gps_device_t *session)
/* GPS DOP and Active Satellites */
{
    /*
     * eg1. $GPGSA,A,3,,,,,,16,18,,22,24,,,3.6,2.1,2.2*3C
     * eg2. $GPGSA,A,3,19,28,14,18,27,22,31,39,,,,,1.7,1.0,1.3*35
     * 1    = Mode:
     * M=Manual, forced to operate in 2D or 3D
     * A=Automatic, 3D/2D
     * 2    = Mode: 1=Fix not available, 2=2D, 3=3D
     * 3-14 = PRNs of satellites used in position fix (null for unused fields)
     * 15   = PDOP
     * 16   = HDOP
     * 17   = VDOP
     */
    gps_mask_t mask;

    /*
     * One chipset called the i.Trek M3 issues GPGSA lines that look like
     * this: "$GPGSA,A,1,,,,*32" when it has no fix.  This is broken
     * in at least two ways: it's got the wrong number of fields, and
     * it claims to be a valid sentence (A flag) when it isn't.
     * Alarmingly, it's possible this error may be generic to SiRFstarIII.
     */
    if (count < 17) {
	gpsd_report(LOG_DATA, "GPGSA: malformed, setting ONLINE_SET only.\n");
	mask = ONLINE_SET;
    } else if (session->driver.nmea.latch_mode) {
	/* last GGA had a non-advancing timestamp; don't trust this GSA */
	mask = ONLINE_SET;
    } else {
	int i;
	session->newdata.mode = atoi(field[2]);
	/*
	 * The first arm of this conditional ignores dead-reckoning
	 * fixes from an Antaris chipset. which returns E in field 2
	 * for a dead-reckoning estimate.  Fix by Andreas Stricker.
	 */
	if (session->newdata.mode == 0 && field[2][0] == 'E')
	    mask = 0;
	else
	    mask = MODE_SET;
	gpsd_report(LOG_PROG, "GPGSA sets mode %d\n", session->newdata.mode);
	session->gpsdata.dop.pdop = safe_atof(field[15]);
	session->gpsdata.dop.hdop = safe_atof(field[16]);
	session->gpsdata.dop.vdop = safe_atof(field[17]);
	session->gpsdata.satellites_used = 0;
	memset(session->gpsdata.used, 0, sizeof(session->gpsdata.used));
	/* the magic 6 here counts the tag, two mode fields, and the DOP fields */
	for (i = 0; i < count - 6; i++) {
	    int prn = atoi(field[i + 3]);
	    if (prn > 0)
		session->gpsdata.used[session->gpsdata.satellites_used++] =
		    prn;
	}
	mask |= DOP_SET | USED_IS;
	gpsd_report(LOG_DATA,
		    "GPGSA: mode=%d used=%d pdop=%.2f hdop=%.2f vdop=%.2f\n",
		    session->newdata.mode,
		    session->gpsdata.satellites_used,
		    session->gpsdata.dop.pdop,
		    session->gpsdata.dop.hdop,
		    session->gpsdata.dop.vdop);
    }
    return mask;
}

static gps_mask_t processGPGSV(int count, char *field[],
			       struct gps_device_t *session)
/* GPS Satellites in View */
{
    /*
     * GSV,2,1,08,01,40,083,46,02,17,308,41,12,07,344,39,14,22,228,45*75
     * 2           Number of sentences for full data
     * 1           Sentence 1 of 2
     * 08          Total number of satellites in view
     * 01          Satellite PRN number
     * 40          Elevation, degrees
     * 083         Azimuth, degrees
     * 46          Signal-to-noise ratio in decibels
     * <repeat for up to 4 satellites per sentence>
     * There my be up to three GSV sentences in a data packet
     */
    int n, fldnum;
    if (count <= 3) {
	gpsd_report(LOG_WARN, "malformed GPGSV - fieldcount %d <= 3\n",
		    count);
	gpsd_zero_satellites(&session->gpsdata);
	session->gpsdata.satellites_visible = 0;
	return ONLINE_SET;
    }
    if (count % 4 != 0) {
	gpsd_report(LOG_WARN, "malformed GPGSV - fieldcount %d %% 4 != 0\n",
		    count);
	gpsd_zero_satellites(&session->gpsdata);
	session->gpsdata.satellites_visible = 0;
	return ONLINE_SET;
    }

    session->driver.nmea.await = atoi(field[1]);
    if ((session->driver.nmea.part = atoi(field[2])) < 1) {
	gpsd_report(LOG_WARN, "malformed GPGSV - bad part\n");
	gpsd_zero_satellites(&session->gpsdata);
	return ONLINE_SET;
    } else if (session->driver.nmea.part == 1)
	gpsd_zero_satellites(&session->gpsdata);

    for (fldnum = 4; fldnum < count;) {
	if (session->gpsdata.satellites_visible >= MAXCHANNELS) {
	    gpsd_report(LOG_ERROR, "internal error - too many satellites [%d]!\n",
                    session->gpsdata.satellites_visible);
	    gpsd_zero_satellites(&session->gpsdata);
	    break;
	}
	session->gpsdata.PRN[session->gpsdata.satellites_visible] =
	    atoi(field[fldnum++]);
	// NMEA-ID (33..64) to SBAS PRN.
	if (session->gpsdata.PRN[session->gpsdata.satellites_visible] >= 33
		&& session->gpsdata.PRN[session->gpsdata.satellites_visible] <= 64)
	    session->gpsdata.PRN[session->gpsdata.satellites_visible] += 87;
	session->gpsdata.elevation[session->gpsdata.satellites_visible] =
	    atoi(field[fldnum++]);
	session->gpsdata.azimuth[session->gpsdata.satellites_visible] =
	    atoi(field[fldnum++]);
	session->gpsdata.ss[session->gpsdata.satellites_visible] =
	    (float)atoi(field[fldnum++]);
	/*
	 * Incrementing this unconditionally falls afoul of chipsets like
	 * the Motorola Oncore GT+ that emit empty fields at the end of the
	 * last sentence in a GPGSV set if the number of satellites is not
	 * a multiple of 4.
	 */
	if (session->gpsdata.PRN[session->gpsdata.satellites_visible] != 0)
	    session->gpsdata.satellites_visible++;
    }
    if (session->driver.nmea.part == session->driver.nmea.await
	&& atoi(field[3]) != session->gpsdata.satellites_visible)
	gpsd_report(LOG_WARN,
		    "GPGSV field 3 value of %d != actual count %d\n",
		    atoi(field[3]), session->gpsdata.satellites_visible);

    /* not valid data until we've seen a complete set of parts */
    if (session->driver.nmea.part < session->driver.nmea.await) {
	gpsd_report(LOG_PROG, "Partial satellite data (%d of %d).\n",
		    session->driver.nmea.part, session->driver.nmea.await);
	return ONLINE_SET;
    }
    /*
     * This sanity check catches an odd behavior of SiRFstarII receivers.
     * When they can't see any satellites at all (like, inside a
     * building) they sometimes cough up a hairball in the form of a
     * GSV packet with all the azimuth entries 0 (but nonzero
     * elevations).  This behavior was observed under SiRF firmware
     * revision 231.000.000_A2.
     */
    for (n = 0; n < session->gpsdata.satellites_visible; n++)
	if (session->gpsdata.azimuth[n] != 0)
	    goto sane;
    gpsd_report(LOG_WARN, "Satellite data no good (%d of %d).\n",
		session->driver.nmea.part, session->driver.nmea.await);
    gpsd_zero_satellites(&session->gpsdata);
    return ONLINE_SET;
  sane:
    session->gpsdata.skyview_time = NAN;
    gpsd_report(LOG_DATA, "GSV: Satellite data OK (%d of %d).\n",
		session->driver.nmea.part, session->driver.nmea.await);
    return SATELLITE_SET;
}

static gps_mask_t processPGRME(int c UNUSED, char *field[],
			       struct gps_device_t *session)
/* Garmin Estimated Position Error */
{
    /*
     * $PGRME,15.0,M,45.0,M,25.0,M*22
     * 1    = horizontal error estimate
     * 2    = units
     * 3    = vertical error estimate
     * 4    = units
     * 5    = spherical error estimate
     * 6    = units
     * *
     * * Garmin won't say, but the general belief is that these are 50% CEP.
     * * We follow the advice at <http://gpsinformation.net/main/errors.htm>.
     * * If this assumption changes here, it should also change in garmin.c
     * * where we scale error estimates from Garmin binary packets, and
     * * in libgpsd_core.c where we generate $PGRME.
     */
    gps_mask_t mask;
    if ((strcmp(field[2], "M") != 0) ||
	(strcmp(field[4], "M") != 0) || (strcmp(field[6], "M") != 0)) {
	session->newdata.epx =
	    session->newdata.epy =
	    session->newdata.epv = session->gpsdata.epe = 100;
	mask = 0;
    } else {
	session->newdata.epx = session->newdata.epy =
	    safe_atof(field[1]) * (1 / sqrt(2)) * (GPSD_CONFIDENCE / CEP50_SIGMA);
	session->newdata.epv =
	    safe_atof(field[3]) * (GPSD_CONFIDENCE / CEP50_SIGMA);
	session->gpsdata.epe =
	    safe_atof(field[5]) * (GPSD_CONFIDENCE / CEP50_SIGMA);
	mask = HERR_SET | VERR_SET | PERR_IS;
    }

    gpsd_report(LOG_DATA, "PGRME: epx=%.2f epy=%.2f epv=%.2f\n",
		session->newdata.epx,
		session->newdata.epy,
		session->newdata.epv);
    return mask;
}

static gps_mask_t processGPGBS(int c UNUSED, char *field[],
			       struct gps_device_t *session)
/* NMEA 3.0 Estimated Position Error */
{
    /*
     * $GPGBS,082941.00,2.4,1.5,3.9,25,,-43.7,27.5*65
     * 1) UTC time of the fix associated with this sentence (hhmmss.ss)
     * 2) Expected error in latitude (meters)
     * 3) Expected error in longitude (meters)
     * 4) Expected error in altitude (meters)
     * 5) PRN of most likely failed satellite
     * 6) Probability of missed detection for most likely failed satellite
     * 7) Estimate of bias in meters on most likely failed satellite
     * 8) Standard deviation of bias estimate
     * 9) Checksum
     */

    /* register fractional time for end-of-cycle detection */
    register_fractional_time(field[0], field[1], session);

    /* check that we're associated with the current fix */
    if (session->driver.nmea.date.tm_hour == DD(field[1])
	&& session->driver.nmea.date.tm_min == DD(field[1] + 2)
	&& session->driver.nmea.date.tm_sec == DD(field[1] + 4)) {
	session->newdata.epy = safe_atof(field[2]);
	session->newdata.epx = safe_atof(field[3]);
	session->newdata.epv = safe_atof(field[4]);
	gpsd_report(LOG_DATA, "GBS: epx=%.2f epy=%.2f epv=%.2f\n",
		    session->newdata.epx,
		    session->newdata.epy,
		    session->newdata.epv);
	return HERR_SET | VERR_SET;
    } else {
	gpsd_report(LOG_PROG,
		    "second in $GPGBS error estimates doesn't match.\n");
	return 0;
    }
}

static gps_mask_t processGPZDA(int c UNUSED, char *field[],
			       struct gps_device_t *session)
/* Time & Date */
{
    /*
     * $GPZDA,160012.71,11,03,2004,-1,00*7D
     * 1) UTC time (hours, minutes, seconds, may have fractional subsecond)
     * 2) Day, 01 to 31
     * 3) Month, 01 to 12
     * 4) Year (4 digits)
     * 5) Local zone description, 00 to +- 13 hours
     * 6) Local zone minutes description, apply same sign as local hours
     * 7) Checksum
     *
     * Note: some devices, like the uBlox ANTARIS 4h, are known to ship ZDAs
     * with some fields blank under poorly-understood circumstances (probably
     * when they don't have satellite lock yet).
     */
    gps_mask_t mask = 0;

    if (field[1][0] == '\0' || field[2][0] == '\0' || field[3][0] == '\0'
	|| field[4][0] == '\0') {
	gpsd_report(LOG_WARN, "ZDA fields are empty\n");
    } else {
    	int year, mon, mday, century;

	merge_hhmmss(field[1], session);
	/*
	 * We don't register fractional time here because want to leave
	 * ZDA out of end-of-cycle detection. Some devices sensibly emit it only
	 * when they have a fix, so watching for it can make them look
	 * like they have a variable fix reporting cycle.
	 */
	year = atoi(field[4]);
	mon = atoi(field[3]);
	mday = atoi(field[2]);
	century = year - year % 100;
	if ( (1900 > year ) || (2200 < year ) ) {
	    gpsd_report(LOG_WARN, "malformed ZDA year: %s\n",  field[4]);
	} else if ( (1 > mon ) || (12 < mon ) ) {
	    gpsd_report(LOG_WARN, "malformed ZDA month: %s\n",  field[3]);
	} else if ( (1 > mday ) || (31 < mday ) ) {
	    gpsd_report(LOG_WARN, "malformed ZDA day: %s\n",  field[2]);
	} else {
	    if (century > session->context->century) {
		/*
		 * This mismatch is almost certainly not due to a GPS week
		 * rollover, because that would throw the ZDA report backward
		 * into the last rollover period instead of forward.  Almost
		 * certainly it means that a century mark has passed while
		 * gpsd was running, and we should trust the new ZDA year.
		 */
		gpsd_report(LOG_WARN, "century rollover detected.\n");
		session->context->century = century;
	    } else if (session->context->start_time >= GPS_EPOCH && century < session->context->century) {
		/*
		 * This looks like a GPS week-counter rollover.
		 */
		gpsd_report(LOG_WARN, "ZDA year %d less than clock year, "
			    "probable GPS week rollover lossage\n", year);
	    }
	    session->driver.nmea.date.tm_year = year - 1900;
	    session->driver.nmea.date.tm_mon = mon - 1;
	    session->driver.nmea.date.tm_mday = mday;
	    mask = TIME_SET;
	}
    };
    return mask;
}

static gps_mask_t processHDT(int c UNUSED, char *field[],
				struct gps_device_t *session)
{
    /*
     * $HEHDT,341.8,T*21
     *
     * HDT,x.x*hh<cr><lf>
     *
     * The only data field is true heading in degrees.
     * The following field is required to be 'T' indicating a true heading.
     * It is followed by a mandatory nmea_checksum.
     */
    gps_mask_t mask;
    mask = ONLINE_SET;

    session->gpsdata.attitude.heading = safe_atof(field[1]);
    session->gpsdata.attitude.mag_st = '\0';
    session->gpsdata.attitude.pitch = NAN;
    session->gpsdata.attitude.pitch_st = '\0';
    session->gpsdata.attitude.roll = NAN;
    session->gpsdata.attitude.roll_st = '\0';
    session->gpsdata.attitude.yaw = NAN;
    session->gpsdata.attitude.yaw_st = '\0';
    session->gpsdata.attitude.dip = NAN;
    session->gpsdata.attitude.mag_len = NAN;
    session->gpsdata.attitude.mag_x = NAN;
    session->gpsdata.attitude.mag_y = NAN;
    session->gpsdata.attitude.mag_z = NAN;
    session->gpsdata.attitude.acc_len = NAN;
    session->gpsdata.attitude.acc_x = NAN;
    session->gpsdata.attitude.acc_y = NAN;
    session->gpsdata.attitude.acc_z = NAN;
    session->gpsdata.attitude.gyro_x = NAN;
    session->gpsdata.attitude.gyro_y = NAN;
    session->gpsdata.attitude.temp = NAN;
    session->gpsdata.attitude.depth = NAN;
    mask |= (ATTITUDE_SET);

    gpsd_report(LOG_RAW, "time %.3f, heading %lf.\n",
		session->newdata.time,
		session->gpsdata.attitude.heading);
    return mask;
}

static gps_mask_t processDBT(int c UNUSED, char *field[],
				struct gps_device_t *session)
{
    /*
     * $SDDBT,7.7,f,2.3,M,1.3,F*05
     * 1) Depth below sounder in feet
     * 2) Fixed value 'f' indicating feet
     * 3) Depth below sounder in meters
     * 4) Fixed value 'M' indicating meters
     * 5) Depth below sounder in fathoms
     * 6) Fixed value 'F' indicating fathoms
     * 7) Checksum.
     *
     * In real-world sensors, sometimes not all three conversions are reported.
     */
    gps_mask_t mask;
    mask = ONLINE_SET;

    if (field[3][0] != '\0') {
	session->newdata.altitude = -safe_atof(field[3]);
	mask |= (ALTITUDE_SET);
    } else if (field[1][0] != '\0') {
	session->newdata.altitude = -safe_atof(field[1]) / METERS_TO_FEET;
	mask |= (ALTITUDE_SET);
    } else if (field[5][0] != '\0') {
	session->newdata.altitude = -safe_atof(field[5]) / METERS_TO_FATHOMS;
	mask |= (ALTITUDE_SET);
    }

    if ((mask & ALTITUDE_SET) != 0) {
	if (session->newdata.mode < MODE_3D) {
	    session->newdata.mode = MODE_3D;
	    mask |= MODE_SET;
	}
    }

    /*
     * Hack: We report depth below keep as negative altitude because there's
     * no better place to put it.  Should work in practice as nobody is
     * likely to be operating a depth sounder at varying altitudes.
     */
    gpsd_report(LOG_RAW, "mode %d, depth %lf.\n",
		session->newdata.mode,
		session->newdata.altitude);
    return mask;
}

#ifdef TNT_ENABLE
static gps_mask_t processTNTHTM(int c UNUSED, char *field[],
				struct gps_device_t *session)
{
    /*
     * Proprietary sentence for True North Technologies Magnetic Compass.
     * This may also apply to some Honeywell units since they may have been
     * designed by True North.

     $PTNTHTM,14223,N,169,N,-43,N,13641,2454*15

     HTM,x.x,a,x.x,a,x.x,a,x.x,x.x*hh<cr><lf>
     Fields in order:
     1. True heading (compass measurement + deviation + variation)
     2. magnetometer status character:
     C = magnetometer calibration alarm
     L = low alarm
     M = low warning
     N = normal
     O = high warning
     P = high alarm
     V = magnetometer voltage level alarm
     3. pitch angle
     4. pitch status character - see field 2 above
     5. roll angle
     6. roll status character - see field 2 above
     7. dip angle
     8. relative magnitude horizontal component of earth's magnetic field
     *hh          mandatory nmea_checksum

     By default, angles are reported as 26-bit integers: weirdly, the
     technical manual says either 0 to 65535 or -32768 to 32767 can
     occur as a range.
     */
    gps_mask_t mask;
    mask = ONLINE_SET;

    session->gpsdata.attitude.heading = safe_atof(field[1]);
    session->gpsdata.attitude.mag_st = *field[2];
    session->gpsdata.attitude.pitch = safe_atof(field[3]);
    session->gpsdata.attitude.pitch_st = *field[4];
    session->gpsdata.attitude.roll = safe_atof(field[5]);
    session->gpsdata.attitude.roll_st = *field[6];
    session->gpsdata.attitude.yaw = NAN;
    session->gpsdata.attitude.yaw_st = '\0';
    session->gpsdata.attitude.dip = safe_atof(field[7]);
    session->gpsdata.attitude.mag_len = NAN;
    session->gpsdata.attitude.mag_x = safe_atof(field[8]);
    session->gpsdata.attitude.mag_y = NAN;
    session->gpsdata.attitude.mag_z = NAN;
    session->gpsdata.attitude.acc_len = NAN;
    session->gpsdata.attitude.acc_x = NAN;
    session->gpsdata.attitude.acc_y = NAN;
    session->gpsdata.attitude.acc_z = NAN;
    session->gpsdata.attitude.gyro_x = NAN;
    session->gpsdata.attitude.gyro_y = NAN;
    session->gpsdata.attitude.temp = NAN;
    session->gpsdata.attitude.depth = NAN;
    mask |= (ATTITUDE_SET);

    gpsd_report(LOG_RAW, "time %.3f, heading %lf (%c).\n",
		session->newdata.time,
		session->gpsdata.attitude.heading,
		session->gpsdata.attitude.mag_st);
    return mask;
}
#endif /* TNT_ENABLE */

#ifdef OCEANSERVER_ENABLE
static gps_mask_t processOHPR(int c UNUSED, char *field[],
			      struct gps_device_t *session)
{
    /*
     * Proprietary sentence for OceanServer Magnetic Compass.

     OHPR,x.x,x.x,x.x,x.x,x.x,x.x,x.x,x.x,x.x,x.x,x.x,x.x,x.x,x.x,x.x,x.x,x.x,x.x*hh<cr><lf>
     Fields in order:
     1. Azimuth
     2. Pitch Angle
     3. Roll Angle
     4. Sensor temp, degrees centigrade
     5. Depth (feet)
     6. Magnetic Vector Length
     7-9. 3 axis Magnetic Field readings x,y,z
     10. Acceleration Vector Length
     11-13. 3 axis Acceleration Readings x,y,z
     14. Reserved
     15-16. 2 axis Gyro Output, X,y
     17. Reserved
     18. Reserved
     *hh          mandatory nmea_checksum
     */
    gps_mask_t mask;
    mask = ONLINE_SET;

    session->gpsdata.attitude.heading = safe_atof(field[1]);
    session->gpsdata.attitude.mag_st = '\0';
    session->gpsdata.attitude.pitch = safe_atof(field[2]);
    session->gpsdata.attitude.pitch_st = '\0';
    session->gpsdata.attitude.roll = safe_atof(field[3]);
    session->gpsdata.attitude.roll_st = '\0';
    session->gpsdata.attitude.yaw = NAN;
    session->gpsdata.attitude.yaw_st = '\0';
    session->gpsdata.attitude.dip = NAN;
    session->gpsdata.attitude.temp = safe_atof(field[4]);
    session->gpsdata.attitude.depth = safe_atof(field[5]) / METERS_TO_FEET;
    session->gpsdata.attitude.mag_len = safe_atof(field[6]);
    session->gpsdata.attitude.mag_x = safe_atof(field[7]);
    session->gpsdata.attitude.mag_y = safe_atof(field[8]);
    session->gpsdata.attitude.mag_z = safe_atof(field[9]);
    session->gpsdata.attitude.acc_len = safe_atof(field[10]);
    session->gpsdata.attitude.acc_x = safe_atof(field[11]);
    session->gpsdata.attitude.acc_y = safe_atof(field[12]);
    session->gpsdata.attitude.acc_z = safe_atof(field[13]);
    session->gpsdata.attitude.gyro_x = safe_atof(field[15]);
    session->gpsdata.attitude.gyro_y = safe_atof(field[16]);
    mask |= (ATTITUDE_SET);

    gpsd_report(LOG_RAW, "Heading %lf.\n", session->gpsdata.attitude.heading);
    return mask;
}
#endif /* OCEANSERVER_ENABLE */

#ifdef ASHTECH_ENABLE
static gps_mask_t processPASHR(int c UNUSED, char *field[],
			       struct gps_device_t *session)
{
    gps_mask_t mask;
    mask = 0;

    if (0 == strcmp("RID", field[1])) {	/* Receiver ID */
	(void)snprintf(session->subtype, sizeof(session->subtype) - 1,
		       "%s ver %s", field[2], field[3]);
	gpsd_report(LOG_DATA, "PASHR,RID: subtype=%s mask={}\n",
		    session->subtype);
	return mask;
    } else if (0 == strcmp("POS", field[1])) {	/* 3D Position */
	mask |= MODE_SET | STATUS_SET | CLEAR_IS;
	if (0 == strlen(field[2])) {
	    /* empty first field means no 3D fix is available */
	    session->gpsdata.status = STATUS_NO_FIX;
	    session->newdata.mode = MODE_NO_FIX;
	} else {
	    /* if we make it this far, we at least have a 3D fix */
	    session->newdata.mode = MODE_3D;
	    if (1 == atoi(field[2]))
		session->gpsdata.status = STATUS_DGPS_FIX;
	    else
		session->gpsdata.status = STATUS_FIX;

	    session->gpsdata.satellites_used = atoi(field[3]);
	    merge_hhmmss(field[4], session);
	    register_fractional_time(field[0], field[4], session);
	    do_lat_lon(&field[5], &session->newdata);
	    session->newdata.altitude = safe_atof(field[9]);
	    session->newdata.track = safe_atof(field[11]);
	    session->newdata.speed = safe_atof(field[12]) / MPS_TO_KPH;
	    session->newdata.climb = safe_atof(field[13]);
	    session->gpsdata.dop.pdop = safe_atof(field[14]);
	    session->gpsdata.dop.hdop = safe_atof(field[15]);
	    session->gpsdata.dop.vdop = safe_atof(field[16]);
	    session->gpsdata.dop.tdop = safe_atof(field[17]);
	    mask |= (TIME_SET | LATLON_SET | ALTITUDE_SET);
	    mask |= (SPEED_SET | TRACK_SET | CLIMB_SET);
	    mask |= DOP_SET;
	    gpsd_report(LOG_DATA,
			"PASHR,POS: hhmmss=%s lat=%.2f lon=%.2f alt=%.f speed=%.2f track=%.2f climb=%.2f mode=%d status=%d pdop=%.2f hdop=%.2f vdop=%.2f tdop=%.2f\n",
			field[4], session->newdata.latitude,
			session->newdata.longitude, session->newdata.altitude,
			session->newdata.speed, session->newdata.track,
			session->newdata.climb, session->newdata.mode,
			session->gpsdata.status, session->gpsdata.dop.pdop,
			session->gpsdata.dop.hdop, session->gpsdata.dop.vdop,
			session->gpsdata.dop.tdop);
	}
    } else if (0 == strcmp("SAT", field[1])) {	/* Satellite Status */
	int i, n, p, u;
	n = session->gpsdata.satellites_visible = atoi(field[2]);
	u = 0;
	for (i = 0; i < n; i++) {
	    session->gpsdata.PRN[i] = p = atoi(field[3 + i * 5 + 0]);
	    session->gpsdata.azimuth[i] = atoi(field[3 + i * 5 + 1]);
	    session->gpsdata.elevation[i] = atoi(field[3 + i * 5 + 2]);
	    session->gpsdata.ss[i] = safe_atof(field[3 + i * 5 + 3]);
	    if (field[3 + i * 5 + 4][0] == 'U')
		session->gpsdata.used[u++] = p;
	}
	session->gpsdata.satellites_used = u;
	gpsd_report(LOG_DATA, "PASHR,SAT: used=%d\n",
		    session->gpsdata.satellites_used);
	session->gpsdata.skyview_time = NAN;
	mask |= SATELLITE_SET | USED_IS;
    }
    return mask;
}
#endif /* ASHTECH_ENABLE */

/**************************************************************************
 *
 * Entry points begin here
 *
 **************************************************************************/

/*@ -mayaliasunique @*/
gps_mask_t nmea_parse(char *sentence, struct gps_device_t * session)
/* parse an NMEA sentence, unpack it into a session structure */
{
    typedef gps_mask_t(*nmea_decoder) (int count, char *f[],
				       struct gps_device_t * session);
    static struct
    {
	char *name;
	int nf;			/* minimum number of fields required to parse */
	bool cycle_continue;	/* cycle continuer? */
	nmea_decoder decoder;
    } nmea_phrase[] = {
	/*@ -nullassign @*/
	{"PGRMC", 0, false, NULL},	/* ignore Garmin Sensor Config */
	{"PGRME", 7, false, processPGRME},
	{"PGRMI", 0, false, NULL},	/* ignore Garmin Sensor Init */
	{"PGRMO", 0, false, NULL},	/* ignore Garmin Sentence Enable */
	    /*
	     * Basic sentences must come after the PG* ones, otherwise
	     * Garmins can get stuck in a loop that looks like this:
	     *
	     * 1. A Garmin GPS in NMEA mode is detected.
	     *
	     * 2. PGRMC is sent to reconfigure to Garmin binary mode.
	     *    If successful, the GPS echoes the phrase.
	     *
	     * 3. nmea_parse() sees the echo as RMC because the talker
	     *    ID is ignored, and fails to recognize the echo as
	     *    PGRMC and ignore it.
	     *
	     * 4. The mode is changed back to NMEA, resulting in an
	     *    infinite loop.
	     */
	{"RMC", 8,  false, processGPRMC},
	{"GGA", 13, false, processGPGGA},
	{"GST", 8,  false, processGPGST},
	{"GLL", 7,  false, processGPGLL},
	{"GSA", 17, false, processGPGSA},
	{"GSV", 0,  false, processGPGSV},
	{"VTG", 0,  false, NULL},	/* ignore Velocity Track made Good */
	{"ZDA", 4,  false, processGPZDA},
	{"GBS", 7,  false, processGPGBS},
        {"HDT", 1,  false, processHDT},
	{"DBT", 7,  true,  processDBT},
#ifdef TNT_ENABLE
	{"PTNTHTM", 9, false, processTNTHTM},
#endif /* TNT_ENABLE */
#ifdef ASHTECH_ENABLE
	{"PASHR", 3, false, processPASHR},	/* general handler for Ashtech */
#endif /* ASHTECH_ENABLE */
#ifdef OCEANSERVER_ENABLE
	{"OHPR", 18, false, processOHPR},
#endif /* OCEANSERVER_ENABLE */
	    /*@ +nullassign @*/
    };

    int count;
    gps_mask_t retval = 0;
    unsigned int i, thistag;
    char *p, *s, *e;
    volatile char *t;

    /*
     * We've had reports that on the Garmin GPS-10 the device sometimes
     * (1:1000 or so) sends garbage packets that have a valid checksum
     * but are like 2 successive NMEA packets merged together in one
     * with some fields lost.  Usually these are much longer than the
     * legal limit for NMEA, so we can cope by just tossing out overlong
     * packets.  This may be a generic bug of all Garmin chipsets.
     */
    if (strlen(sentence) > NMEA_MAX) {
	gpsd_report(LOG_WARN, "Overlong packet of %zd chars rejected.\n",
		    strlen(sentence));
	return ONLINE_SET;
    }

    /*@ -usedef @*//* splint 3.1.1 seems to have a bug here */
    /* make an editable copy of the sentence */
    (void)strlcpy((char *)session->driver.nmea.fieldcopy, sentence, NMEA_MAX);
    /* discard the checksum part */
    for (p = (char *)session->driver.nmea.fieldcopy;
	 (*p != '*') && (*p >= ' ');)
	++p;
    if (*p == '*')
	*p++ = ',';		/* otherwise we drop the last field */
    *p = '\0';
    e = p;

    /* split sentence copy on commas, filling the field array */
    count = 0;
    t = p;			/* end of sentence */
    p = (char *)session->driver.nmea.fieldcopy + 1;	/* beginning of tag, 'G' not '$' */
    /* while there is a search string and we haven't run off the buffer... */
    while ((p != NULL) && (p <= t)) {
	session->driver.nmea.field[count] = p;	/* we have a field. record it */
	/*@ -compdef @*/
	if ((p = strchr(p, ',')) != NULL) {	/* search for the next delimiter */
	    *p = '\0';		/* replace it with a NUL */
	    count++;		/* bump the counters and continue */
	    p++;
	}
	/*@ +compdef @*/
    }

    /* point remaining fields at empty string, just in case */
    for (i = (unsigned int)count;
	 i <
	 (unsigned)(sizeof(session->driver.nmea.field) /
		    sizeof(session->driver.nmea.field[0])); i++)
	session->driver.nmea.field[i] = e;

    /* sentences handlers will tell us whren they have fractional time */
    session->driver.nmea.latch_frac_time = false;

    /* dispatch on field zero, the sentence tag */
    for (thistag = i = 0;
	 i < (unsigned)(sizeof(nmea_phrase) / sizeof(nmea_phrase[0])); ++i) {
	s = session->driver.nmea.field[0];
	if (strlen(nmea_phrase[i].name) == 3)
	    s += 2;		/* skip talker ID */
	if (strcmp(nmea_phrase[i].name, s) == 0) {
	    if (nmea_phrase[i].decoder != NULL
		&& (count >= nmea_phrase[i].nf)) {
		retval =
		    (nmea_phrase[i].decoder) (count,
					      session->driver.nmea.field,
					      session);
		(void)strlcpy(session->gpsdata.tag,
			      nmea_phrase[i].name,
			      MAXTAGLEN);
		if (nmea_phrase[i].cycle_continue)
		    session->driver.nmea.cycle_continue = true;
		/*
		 * Must force this to be nz, as we're going to rely on a zero
		 * value to mean "no previous tag" later.
		 */
		thistag = i + 1;
	    } else
		retval = ONLINE_SET;	/* unknown sentence */
	    break;
	}
    }

    /* timestamp recording for fixes happens here */
    if ((retval & TIME_SET) != 0) {
	session->newdata.time = gpsd_utc_resolve(session);
	/*
	 * WARNING: This assumes time is always field 0, and that field 0
	 * is a timestamp whenever TIME_SET is set.
	 */
	gpsd_report(LOG_DATA,
		    "%s time is %2f = %d-%02d-%02dT%02d:%02d:%02.2fZ\n",
		    session->driver.nmea.field[0], session->newdata.time,
		    1900 + session->driver.nmea.date.tm_year,
		    session->driver.nmea.date.tm_mon + 1,
		    session->driver.nmea.date.tm_mday,
		    session->driver.nmea.date.tm_hour,
		    session->driver.nmea.date.tm_min,
		    session->driver.nmea.date.tm_sec + session->driver.nmea.subseconds);
	/*
	 * Conservative heuristic for whether we should try using the
	 * device for precision time service. Wait for it to stabilize
	 * after initialization. (We know this is required on some
	 * Garmins in binary mode; safest to do it in case we're
	 * talking to a Garmin in text mode.)
	 */
	if (session->fixcnt > 3)
	    retval |= PPSTIME_IS;
    }

    /*
     * The end-of-cycle detector.  This code depends on just one
     * assumption: if a sentence with a timestamp occurs just before
     * start of cycle, then it is always good to trigger a report on
     * that sentence in the future.  For devices with a fixed cycle
     * this should work perfectly, locking in detection after one
     * cycle.  Most split-cycle devices (Garmin 48, for example) will
     * work fine.  Problems will only arise if a a sentence that
     * occurs just befiore timestamp increments also occurs in
     * mid-cycle, as in the Garmin eXplorist 210; those might jitter.
     */
    if (session->driver.nmea.latch_frac_time) {
	gpsd_report(LOG_PROG,
		    "%s sentence timestamped %.2f.\n",
		    session->driver.nmea.field[0],
		    session->driver.nmea.this_frac_time);
	if (!GPS_TIME_EQUAL
	    (session->driver.nmea.this_frac_time,
	     session->driver.nmea.last_frac_time)) {
	    uint lasttag = session->driver.nmea.lasttag;
	    retval |= CLEAR_IS;
	    gpsd_report(LOG_PROG,
			"%s starts a reporting cycle.\n",
			session->driver.nmea.field[0]);
	    /*
	     * Have we seen a previously timestamped NMEA tag?
	     * If so, designate as end-of-cycle marker.
	     * But not if there are continuation sentences;
	     * those get sorted after the last timestamped sentence
	     */
	    if (lasttag > 0
		&& (session->driver.nmea.cycle_enders & (1 << lasttag)) == 0
		&& !session->driver.nmea.cycle_continue) {
		session->driver.nmea.cycle_enders |= (1 << lasttag);
		gpsd_report(LOG_PROG,
			    "tagged %s as a cycle ender.\n",
			    nmea_phrase[lasttag - 1].name);
	    }
	}
    } else {
	/* extend the cycle to an un-timestamped sentence? */
	if ((session->driver.nmea.lasttag & session->driver.nmea.cycle_enders) != 0)
	    gpsd_report(LOG_PROG,
			"%s is just after a cycle ender.\n",
			session->driver.nmea.field[0]);
	if (session->driver.nmea.cycle_continue) {
	    gpsd_report(LOG_PROG,
			"%s extends the reporting cycle.\n",
			session->driver.nmea.field[0]);
	    session->driver.nmea.cycle_enders &=~ (1 << session->driver.nmea.lasttag);
	    session->driver.nmea.cycle_enders |= (1 << thistag);
	}
    }
    /* here's where we check for end-of-cycle */
    if ((session->driver.nmea.latch_frac_time || session->driver.nmea.cycle_continue)
	&& (session->driver.nmea.cycle_enders & (1 << thistag))!=0) {
	gpsd_report(LOG_PROG,
		    "%s ends a reporting cycle.\n",
		    session->driver.nmea.field[0]);
	retval |= REPORT_IS;
    }
    if (session->driver.nmea.latch_frac_time)
	session->driver.nmea.lasttag = thistag;

    /* we might have a reliable end-of-cycle */
    if (session->driver.nmea.cycle_enders != 0)
	session->cycle_end_reliable = true;

    return retval;
}

/*@ +mayaliasunique @*/
#endif /* NMEA_ENABLE */

void nmea_add_checksum(char *sentence)
/* add NMEA checksum to a possibly  *-terminated sentence */
{
    unsigned char sum = '\0';
    char c, *p = sentence;

    if (*p == '$' || *p == '!') {
	p++;
    } else {
	gpsd_report(LOG_ERROR, "Bad NMEA sentence: '%s'\n", sentence);
    }
    while (((c = *p) != '*') && (c != '\0')) {
	sum ^= c;
	p++;
    }
    *p++ = '*';
    (void)snprintf(p, 5, "%02X\r\n", (unsigned)sum);
}

ssize_t nmea_write(struct gps_device_t *session, char *buf, size_t len UNUSED)
/* ship a command to the GPS, adding * and correct checksum */
{
    (void)strlcpy(session->msgbuf, buf, sizeof(session->msgbuf));
    if (session->msgbuf[0] == '$') {
	(void)strlcat(session->msgbuf, "*", sizeof(session->msgbuf));
	nmea_add_checksum(session->msgbuf);
    } else
	(void)strlcat(session->msgbuf, "\r\n", sizeof(session->msgbuf));
    session->msgbuflen = strlen(session->msgbuf);
    return gpsd_write(session, session->msgbuf, session->msgbuflen);
}

ssize_t nmea_send(struct gps_device_t * session, const char *fmt, ...)
{
    char buf[BUFSIZ];
    va_list ap;

    va_start(ap, fmt);
    (void)vsnprintf(buf, sizeof(buf) - 5, fmt, ap);
    va_end(ap);
    return nmea_write(session, buf, strlen(buf));
}
