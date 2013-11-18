/*
 * Handle the Trimble TSIP packet format
 * by Rob Janssen, PE1CHL.
 *
 * Week counters are not limited to 10 bits. It's unknown what
 * the firmware is doing to disambiguate them, if anything; it might just
 * be adding a fixed offset based on a hidden epoch value, in which case
 * unhappy things will occur on the next rollover.
 *
 * This file is Copyright (c) 2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 */
#include <sys/time.h>		/* for select() */
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#ifndef S_SPLINT_S
#include <unistd.h>
#endif /* S_SPLINT_S */

#include "gpsd.h"
#include "bits.h"

#define USE_SUPERPACKET	1	/* use Super Packet mode? */

#define SEMI_2_DEG	(180.0 / 2147483647)	/* 2^-31 semicircle to deg */

#ifdef TSIP_ENABLE
#define TSIP_CHANNELS	12

static int tsip_write(struct gps_device_t *session,
		      unsigned int id, /*@null@*/ unsigned char *buf,
		      size_t len)
{
    char *ep, *cp;

    /*@ +charint @*/
    session->msgbuf[0] = '\x10';
    session->msgbuf[1] = (char)id;
    ep = session->msgbuf + 2;
    /*@ -nullderef @*/
    for (cp = (char *)buf; len-- > 0; cp++) {
	if (*cp == '\x10')
	    *ep++ = '\x10';
	*ep++ = *cp;
    }
    /*@ +nullderef @*/
    *ep++ = '\x10';
    *ep++ = '\x03';
    session->msgbuflen = (size_t) (ep - session->msgbuf);
    /*@ -charint @*/
    gpsd_report(LOG_IO, "Sent TSIP packet id 0x%02x\n", id);
    if (gpsd_write(session, session->msgbuf, session->msgbuflen) !=
	(ssize_t) session->msgbuflen)
	return -1;

    return 0;
}

/* tsip_detect()
 *
 * see if it looks like a TSIP device (speaking 9600O81) is listening and
 * return 1 if found, 0 if not
 */
static bool tsip_detect(struct gps_device_t *session)
{
    char buf[BUFSIZ];
    bool ret = false;
    int myfd;
    fd_set fdset;
    struct timeval to;
    speed_t old_baudrate;
    char old_parity;
    unsigned int old_stopbits;

    old_baudrate = session->gpsdata.dev.baudrate;
    old_parity = session->gpsdata.dev.parity;
    old_stopbits = session->gpsdata.dev.stopbits;
    gpsd_set_speed(session, 9600, 'O', 1);

    /* request firmware revision and look for a valid response */
    /*@+ignoresigns@*/
    putbyte(buf, 0, 0x10);
    putbyte(buf, 1, 0x1f);
    putbyte(buf, 2, 0x10);
    putbyte(buf, 3, 0x03);
    /*@+ignoresigns@*/
    myfd = session->gpsdata.gps_fd;
    if (write(myfd, buf, 4) == 4) {
	unsigned int n;
	for (n = 0; n < 3; n++) {
	    FD_ZERO(&fdset);
	    FD_SET(myfd, &fdset);
	    to.tv_sec = 1;
	    to.tv_usec = 0;
	    if (select(myfd + 1, &fdset, NULL, NULL, &to) != 1)
		break;
	    if (generic_get(session) >= 0) {
		if (session->packet.type == TSIP_PACKET) {
		    gpsd_report(LOG_RAW, "tsip_detect found\n");
		    ret = true;
		    break;
		}
	    }
	}
    }

    if (!ret)
	/* return serial port to original settings */
	gpsd_set_speed(session, old_baudrate, old_parity, old_stopbits);

    return ret;
}

static gps_mask_t tsip_analyze(struct gps_device_t *session)
{
    int i, j, len, count;
    gps_mask_t mask = 0;
    unsigned int id;
    uint8_t u1, u2, u3, u4, u5;
    int16_t s1, s2, s3, s4;
    int32_t sl1, sl2, sl3;
    uint32_t ul1, ul2;
    float f1, f2, f3, f4, f5;
    double d1, d2, d3, d4, d5;
    time_t now;
    unsigned char buf[BUFSIZ];
    char buf2[BUFSIZ];

    if (session->packet.type != TSIP_PACKET) {
	gpsd_report(LOG_INF, "tsip_analyze packet type %d\n",
		    session->packet.type);
	return 0;
    }

    /*@ +charint @*/
    if (session->packet.outbuflen < 4 || session->packet.outbuffer[0] != 0x10)
	return 0;

    /* remove DLE stuffing and put data part of message in buf */

    memset(buf, 0, sizeof(buf));
    buf2[len = 0] = '\0';
    for (i = 2; i < (int)session->packet.outbuflen; i++) {
	if (session->packet.outbuffer[i] == 0x10)
	    if (session->packet.outbuffer[++i] == 0x03)
		break;

	(void)snprintf(buf2 + strlen(buf2),
		       sizeof(buf2) - strlen(buf2),
		       "%02x", buf[len++] = session->packet.outbuffer[i]);
    }
    /*@ -charint @*/

    (void)snprintf(session->gpsdata.tag, sizeof(session->gpsdata.tag),
		   "ID%02x", id = (unsigned)session->packet.outbuffer[1]);

    gpsd_report(LOG_IO, "TSIP packet id 0x%02x length %d: %s\n", id, len,
		buf2);
    (void)time(&now);

    session->cycle_end_reliable = true;
    switch (id) {
    case 0x13:			/* Packet Received */
	u1 = getub(buf, 0);
	u2 = getub(buf, 1);
	gpsd_report(LOG_WARN,
		    "Received packet of type %02x cannot be parsed\n", u1);
#if USE_SUPERPACKET
	if ((int)u1 == 0x8e && (int)u2 == 0x23) {	/* no Compact Super Packet */
	    gpsd_report(LOG_WARN, "No Compact Super Packet, use LFwEI\n");

	    /* Request LFwEI Super Packet */
	    putbyte(buf, 0, 0x20);
	    putbyte(buf, 1, 0x01);	/* enabled */
	    (void)tsip_write(session, 0x8e, buf, 2);
	}
#endif /* USE_SUPERPACKET */
	break;
    case 0x41:			/* GPS Time */
	if (len != 10)
	    break;
	session->driver.tsip.last_41 = now;	/* keep timestamp for request */
	f1 = getbef32((char *)buf, 0);	/* gpstime */
	s1 = getbes16(buf, 4);	/* week */
	f2 = getbef32((char *)buf, 6);	/* leap seconds */
	if (f1 >= 0.0 && f2 > 10.0) {
	    session->context->leap_seconds = (int)round(f2);
	    session->context->valid |= LEAP_SECOND_VALID;
	    session->newdata.time =
		gpsd_gpstime_resolve(session, (unsigned short)s1, (double)f1);
	    mask |= TIME_SET | PPSTIME_IS;
	}
	gpsd_report(LOG_INF, "GPS Time %f %d %f\n", f1, s1, f2);
	break;
    case 0x42:			/* Single-Precision Position Fix, XYZ ECEF */
	if (len != 16)
	    break;
	f1 = getbef32((char *)buf, 0);	/* X */
	f2 = getbef32((char *)buf, 4);	/* Y */
	f3 = getbef32((char *)buf, 8);	/* Z */
	f4 = getbef32((char *)buf, 12);	/* time-of-fix */
	gpsd_report(LOG_INF, "GPS Position XYZ %f %f %f %f\n", f1, f2, f3,
		    f4);
	break;
    case 0x43:			/* Velocity Fix, XYZ ECEF */
	if (len != 20)
	    break;
	f1 = getbef32((char *)buf, 0);	/* X velocity */
	f2 = getbef32((char *)buf, 4);	/* Y velocity */
	f3 = getbef32((char *)buf, 8);	/* Z velocity */
	f4 = getbef32((char *)buf, 12);	/* bias rate */
	f5 = getbef32((char *)buf, 16);	/* time-of-fix */
	gpsd_report(LOG_INF, "GPS Velocity XYZ %f %f %f %f %f\n", f1, f2, f3,
		    f4, f5);
	break;
    case 0x45:			/* Software Version Information */
	if (len != 10)
	    break;
	/*@ -formattype @*/
	(void)snprintf(session->subtype, sizeof(session->subtype),
		       "%d.%d %02d%02d%02d %d.%d %02d%02d%02d",
		       getub(buf, 0), getub(buf, 1), getub(buf, 4), getub(buf,
									  2),
		       getub(buf, 3), getub(buf, 5), getub(buf, 6), getub(buf,
									  9),
		       getub(buf, 7), getub(buf, 8));
	/*@ +formattype @*/
	gpsd_report(LOG_INF, "Software version: %s\n", session->subtype);
	mask |= DEVICEID_SET;
	break;
    case 0x46:			/* Health of Receiver */
	if (len != 2)
	    break;
	session->driver.tsip.last_46 = now;
	u1 = getub(buf, 0);	/* Status code */
	u2 = getub(buf, 1);	/* Antenna/Battery */
	if (u1 != (uint8_t) 0) {
	    session->gpsdata.status = STATUS_NO_FIX;
	    mask |= STATUS_SET;
	} else {
	    if (session->gpsdata.status < STATUS_FIX) {
		session->gpsdata.status = STATUS_FIX;
		mask |= STATUS_SET;
	    }
	}
	gpsd_report(LOG_PROG, "Receiver health %02x %02x\n", u1, u2);
	break;
    case 0x47:			/* Signal Levels for all Satellites */
	gpsd_zero_satellites(&session->gpsdata);
	count = (int)getub(buf, 0);	/* satellite count */
	if (len != (5 * count + 1))
	    break;
	buf2[0] = '\0';
	for (i = 0; i < count; i++) {
	    u1 = getub(buf, 5 * i + 1);
	    if ((f1 = getbef32((char *)buf, 5 * i + 2)) < 0)
		f1 = 0.0;
	    for (j = 0; j < TSIP_CHANNELS; j++)
		if (session->gpsdata.PRN[j] == (int)u1) {
		    session->gpsdata.ss[j] = f1;
		    break;
		}
	    (void)snprintf(buf2 + strlen(buf2), sizeof(buf2) - strlen(buf2),
			   " %d=%.1f", (int)u1, f1);
	}
	gpsd_report(LOG_PROG, "Signal Levels (%d):%s\n", count, buf2);
	mask |= SATELLITE_SET;
	break;
    case 0x48:			/* GPS System Message */
	buf[len] = '\0';
	gpsd_report(LOG_PROG, "GPS System Message: %s\n", buf);
	break;
    case 0x49:			/* Almanac Health Page */
	break;
    case 0x4a:			/* Single-Precision Position LLA */
	if (len != 20)
	    break;
	session->newdata.latitude = getbef32((char *)buf, 0) * RAD_2_DEG;
	session->newdata.longitude = getbef32((char *)buf, 4) * RAD_2_DEG;
	session->newdata.altitude = getbef32((char *)buf, 8);
	//f1 = getbef32((char *)buf, 12);	clock bias */
	f2 = getbef32((char *)buf, 16);	/* time-of-fix */
	if ((session->context->valid & GPS_TIME_VALID)!=0) {
	    session->newdata.time =
		gpsd_gpstime_resolve(session,
				  (unsigned short)session->context->gps_week,
				  (double)f2);
	    mask |= TIME_SET | PPSTIME_IS;
	}
	mask |= LATLON_SET | ALTITUDE_SET | CLEAR_IS | REPORT_IS;
	gpsd_report(LOG_DATA, "SPPLLA 0x4a "
		    "time=%.2f lat=%.2f lon=%.2f alt=%.2f\n",
		    session->newdata.time,
		    session->newdata.latitude,
		    session->newdata.longitude,
		    session->newdata.altitude);
	break;
    case 0x4b:			/* Machine/Code ID and Additional Status */
	if (len != 3)
	    break;
	u1 = getub(buf, 0);	/* Machine ID */
	u2 = getub(buf, 1);	/* Status 1 */
	u3 = getub(buf, 2);	/* Status 2 */
	gpsd_report(LOG_INF, "Machine ID %02x %02x %02x\n", u1, u2, u3);
#if USE_SUPERPACKET
	if ((u3 & 0x01) != (uint8_t) 0 && !session->driver.tsip.superpkt) {
	    gpsd_report(LOG_PROG, "Switching to Super Packet mode\n");

	    /* set new I/O Options for Super Packet output */
	    putbyte(buf, 0, 0x2c);	/* Position: SP, MSL */
	    putbyte(buf, 1, 0x00);	/* Velocity: none (via SP) */
	    putbyte(buf, 2, 0x00);	/* Time: GPS */
	    putbyte(buf, 3, 0x08);	/* Aux: dBHz */
	    (void)tsip_write(session, 0x35, buf, 4);
	    session->driver.tsip.superpkt = true;
	}
#endif /* USE_SUPERPACKET */
	break;
    case 0x4c:			/* Operating Parameters Report */
	break;
    case 0x54:			/* One Satellite Bias */
	break;
    case 0x55:			/* IO Options */
	if (len != 4)
	    break;
	u1 = getub(buf, 0);	/* Position */
	u2 = getub(buf, 1);	/* Velocity */
	u3 = getub(buf, 2);	/* Timing */
	u4 = getub(buf, 3);	/* Aux */
	gpsd_report(LOG_INF, "IO Options %02x %02x %02x %02x\n", u1, u2, u3,
		    u4);
#if USE_SUPERPACKET
	if ((u1 & 0x20) != (uint8_t) 0) {	/* Output Super Packets? */
	    /* No LFwEI Super Packet */
	    putbyte(buf, 0, 0x20);
	    putbyte(buf, 1, 0x00);	/* disabled */
	    (void)tsip_write(session, 0x8e, buf, 2);

	    /* Request Compact Super Packet */
	    putbyte(buf, 0, 0x23);
	    putbyte(buf, 1, 0x01);	/* enabled */
	    (void)tsip_write(session, 0x8e, buf, 2);
	    session->driver.tsip.req_compact = now;
	}
#endif /* USE_SUPERPACKET */
	break;
    case 0x56:			/* Velocity Fix, East-North-Up (ENU) */
	if (len != 20)
	    break;
	f1 = getbef32((char *)buf, 0);	/* East velocity */
	f2 = getbef32((char *)buf, 4);	/* North velocity */
	f3 = getbef32((char *)buf, 8);	/* Up velocity */
	f4 = getbef32((char *)buf, 12);	/* clock bias rate */
	f5 = getbef32((char *)buf, 16);	/* time-of-fix */
	session->newdata.climb = f3;
	/*@ -evalorder @*/
	session->newdata.speed = sqrt(pow(f2, 2) + pow(f1, 2));
	/*@ +evalorder @*/
	if ((session->newdata.track = atan2(f1, f2) * RAD_2_DEG) < 0)
	    session->newdata.track += 360.0;
	gpsd_report(LOG_INF, "GPS Velocity ENU %f %f %f %f %f\n", f1, f2, f3,
		    f4, f5);
	mask |= SPEED_SET | TRACK_SET | CLIMB_SET;
	gpsd_report(LOG_DATA, "VFENU 0x56 "
		    "time=%.2f speed=%.2f track=%.2f climb=%.2f\n",
		    session->newdata.time,
		    session->newdata.speed,
		    session->newdata.track,
		    session->newdata.climb);
	break;
    case 0x57:			/* Information About Last Computed Fix */
	if (len != 8)
	    break;
	u1 = getub(buf, 0);	/* Source of information */
	u2 = getub(buf, 1);	/* Mfg. diagnostic */
	f1 = getbef32((char *)buf, 2);	/* gps_time */
	s1 = getbes16(buf, 6);	/* tsip.gps_week */
	/*@ +charint @*/
	if (getub(buf, 0) == 0x01)	/* good current fix? */
	    (void)gpsd_gpstime_resolve(session, (unsigned short)s1, (double)f1);
	/*@ -charint @*/
	gpsd_report(LOG_INF, "Fix info %02x %02x %d %f\n", u1, u2, s1, f1);
	break;
    case 0x58:			/* Satellite System Data/Acknowledge from Receiver */
	break;
    case 0x59:			/* Status of Satellite Disable or Ignore Health */
	break;
    case 0x5a:			/* Raw Measurement Data */
	if (len != 29)
	    break;
	f1 = getbef32((char *)buf, 5);	/* Signal Level */
	f2 = getbef32((char *)buf, 9);	/* Code phase */
	f3 = getbef32((char *)buf, 13);	/* Doppler */
	d1 = getbed64((char *)buf, 17);	/* Time of Measurement */
	gpsd_report(LOG_PROG, "Raw Measurement Data %d %f %f %f %f\n",
		    getub(buf, 0), f1, f2, f3, d1);
	break;
    case 0x5b:			/* Satellite Ephemeris Status */
	break;
    case 0x5c:			/* Satellite Tracking Status */
	if (len != 24)
	    break;
	u1 = getub(buf, 0);	/* PRN */
	u2 = getub(buf, 1);	/* chan */
	u3 = getub(buf, 2);	/* Acquisition flag */
	u4 = getub(buf, 3);	/* Ephemeris flag */
	f1 = getbef32((char *)buf, 4);	/* Signal level */
	f2 = getbef32((char *)buf, 8);	/* time of Last measurement */
	d1 = getbef32((char *)buf, 12) * RAD_2_DEG;	/* Elevation */
	d2 = getbef32((char *)buf, 16) * RAD_2_DEG;	/* Azimuth */
	i = (int)(u2 >> 3);	/* channel number */
	gpsd_report(LOG_INF,
		    "Satellite Tracking Status: Ch %2d PRN %3d Res %d Acq %d Eph %2d SNR %4.1f LMT %.04f El %4.1f Az %5.1f\n",
		    i, u1, u2 & 7, u3, u4, f1, f2, d1, d2);
	if (i < TSIP_CHANNELS) {
	    if (d1 >= 0.0) {
		session->gpsdata.PRN[i] = (int)u1;
		session->gpsdata.ss[i] = f1;
		session->gpsdata.elevation[i] = (int)round(d1);
		session->gpsdata.azimuth[i] = (int)round(d2);
	    } else {
		session->gpsdata.PRN[i] = session->gpsdata.elevation[i]
		    = session->gpsdata.azimuth[i] = 0;
		session->gpsdata.ss[i] = 0.0;
	    }
	    if (++i == session->gpsdata.satellites_visible) {
		session->gpsdata.skyview_time = NAN;
		mask |= SATELLITE_SET;	/* last of the series */
	    }
	    if (i > session->gpsdata.satellites_visible)
		session->gpsdata.satellites_visible = i;
	}
	break;
    case 0x5e:			/* Additional Fix Status Report */
	break;
    case 0x6d:			/* All-In-View Satellite Selection */
	u1 = getub(buf, 0);	/* nsvs/dimension */
	count = (int)((u1 >> 4) & 0x0f);
	if (len != (17 + count))
	    break;
	session->driver.tsip.last_6d = now;	/* keep timestamp for request */
#ifdef __UNUSED__
	/*
	 * This looks right, but it sets a spurious mode value when
	 * the satellite constellation looks good to the chip but no
	 * actual fix has yet been acquired.  We should set the mode
	 * field (which controls gpsd's fix reporting) only from sentences
	 * that convey actual fix information, like 0x20, otherwise we
	 * get results like triggering their error modeler spuriously.
	 */
	switch (u1 & 7) {	/* dimension */
	case 3:
	    //session->gpsdata.status = STATUS_FIX;
	    session->newdata.mode = MODE_2D;
	    break;
	case 4:
	    //session->gpsdata.status = STATUS_FIX;
	    session->newdata.mode = MODE_3D;
	    break;
	default:
	    //session->gpsdata.status = STATUS_NO_FIX;
	    session->newdata.mode = MODE_NO_FIX;
	    break;
	}
	mask |= MODE_SET;
#endif /* __UNUSED__ */
	session->gpsdata.satellites_used = count;
	session->gpsdata.dop.pdop = getbef32((char *)buf, 1);
	session->gpsdata.dop.hdop = getbef32((char *)buf, 5);
	session->gpsdata.dop.vdop = getbef32((char *)buf, 9);
	session->gpsdata.dop.tdop = getbef32((char *)buf, 13);
	/*@ -evalorder @*/
	session->gpsdata.dop.gdop =
	    sqrt(pow(session->gpsdata.dop.pdop, 2) +
		 pow(session->gpsdata.dop.tdop, 2));
	/*@ +evalorder @*/

	memset(session->gpsdata.used, 0, sizeof(session->gpsdata.used));
	buf2[0] = '\0';
	/*@ +charint @*/
	for (i = 0; i < count; i++)
	    (void)snprintf(buf2 + strlen(buf2), sizeof(buf2) - strlen(buf2),
			   " %d", session->gpsdata.used[i] =
			   (int)getub(buf, 17 + i));
	/*@ -charint @*/
	gpsd_report(LOG_DATA, "AIVSS: 0x6d "
		    "status=%d used=%d "
		    "pdop=%.1f hdop=%.1f vdop=%.1f tdop=%.1f gdup=%.1f\n",
		    session->gpsdata.status,
		    session->gpsdata.satellites_used,
		    session->gpsdata.dop.pdop,
		    session->gpsdata.dop.hdop,
		    session->gpsdata.dop.vdop,
		    session->gpsdata.dop.tdop,
		    session->gpsdata.dop.gdop);
	mask |= DOP_SET | STATUS_SET | USED_IS;
	break;
    case 0x6e:			/* Synchronized Measurements */
	break;
#ifdef __UNUSED__
    case 0x6f:			/* Synchronized Measurements Report */
	/*@ +charint @*/
	if (len < 20 || getub(buf, 0) != 1 || getub(buf, 1) != 2)
	    break;
	/*@ -charint @*/
	s1 = getbes16(buf, 2);	/* number of bytes */
	u1 = getub(buf, 20);	/* number of SVs */
	break;
#endif /* __UNUSED__ */
    case 0x70:			/* Filter Report */
	break;
    case 0x7a:			/* NMEA settings */
	break;
    case 0x82:			/* Differential Position Fix Mode */
	if (len != 1)
	    break;
	u1 = getub(buf, 0);	/* fix mode */
	/*@ +charint @*/
	if (session->gpsdata.status == STATUS_FIX && (u1 & 0x01) != 0) {
	    session->gpsdata.status = STATUS_DGPS_FIX;
	    mask |= STATUS_SET;
	}
	/*@ -charint @*/
	gpsd_report(LOG_DATA, "DPFM 0x82 status=%d\n", session->gpsdata.status);
	break;
    case 0x83:			/* Double-Precision XYZ Position Fix and Bias Information */
	if (len != 36)
	    break;
	d1 = getbed64((char *)buf, 0);	/* X */
	d2 = getbed64((char *)buf, 8);	/* Y */
	d3 = getbed64((char *)buf, 16);	/* Z */
	d4 = getbed64((char *)buf, 24);	/* clock bias */
	f1 = getbef32((char *)buf, 32);	/* time-of-fix */
	gpsd_report(LOG_INF, "GPS Position XYZ %f %f %f %f %f\n", d1, d2, d3,
		    d4, f1);
	break;
    case 0x84:			/* Double-Precision LLA Position Fix and Bias Information */
	if (len != 36)
	    break;
	session->newdata.latitude = getbed64((char *)buf, 0) * RAD_2_DEG;
	session->newdata.longitude = getbed64((char *)buf, 8) * RAD_2_DEG;
	session->newdata.altitude = getbed64((char *)buf, 16);
	//d1 = getbed64((char *)buf, 24);	clock bias */
	f1 = getbef32((char *)buf, 32);	/* time-of-fix */
	if ((session->context->valid & GPS_TIME_VALID)!=0) {
	    session->newdata.time =
		gpsd_gpstime_resolve(session,
				  (unsigned short)session->context->gps_week,
				  (double)f1);
	    mask |= TIME_SET | PPSTIME_IS;
	}
	gpsd_report(LOG_INF, "GPS DP LLA %f %f %f %f\n",
		    session->newdata.time,
		    session->newdata.latitude,
		    session->newdata.longitude, session->newdata.altitude);
	mask |= LATLON_SET | ALTITUDE_SET | CLEAR_IS | REPORT_IS;
	gpsd_report(LOG_DATA, "DPPLLA 0x84 "
		    "time=%.2f lat=%.2f lon=%.2f alt=%.2f\n",
		    session->newdata.time,
		    session->newdata.latitude,
		    session->newdata.longitude,
		    session->newdata.altitude);
	break;
    case 0x8f:			/* Super Packet.  Well...  */
	/*@ +charint @*/
	u1 = (uint8_t) getub(buf, 0);
	(void)snprintf(session->gpsdata.tag + strlen(session->gpsdata.tag),
		       sizeof(session->gpsdata.tag) -
		       strlen(session->gpsdata.tag), "%02x", (uint) u1);
	/*@ -charint @*/
	switch (u1) {		/* sub-packet ID */
	case 0x15:		/* Current Datum Values */
	    if (len != 43)
		break;
	    s1 = getbes16(buf, 1);	/* Datum Index */
	    d1 = getbed64((char *)buf, 3);	/* DX */
	    d2 = getbed64((char *)buf, 11);	/* DY */
	    d3 = getbed64((char *)buf, 19);	/* DZ */
	    d4 = getbed64((char *)buf, 27);	/* A-axis */
	    d5 = getbed64((char *)buf, 35);	/* Eccentricity Squared */
	    gpsd_report(LOG_INF, "Current Datum %d %f %f %f %f %f\n", s1, d1,
			d2, d3, d4, d5);
	    break;

	case 0x20:		/* Last Fix with Extra Information (binary fixed point) */
	    /* CSK sez "why does my Lassen iQ output oversize packets?" */
	    if ((len != 56) && (len != 64))
		break;
	    s1 = getbes16(buf, 2);	/* east velocity */
	    s2 = getbes16(buf, 4);	/* north velocity */
	    s3 = getbes16(buf, 6);	/* up velocity */
	    ul1 = getbeu32(buf, 8);	/* time */
	    sl1 = getbes32(buf, 12);	/* latitude */
	    ul2 = getbeu32(buf, 16);	/* longitude */
	    sl2 = getbes32(buf, 20);	/* altitude */
	    u1 = getub(buf, 24);	/* velocity scaling */
	    u2 = getub(buf, 27);	/* fix flags */
	    u3 = getub(buf, 28);	/* num svs */
	    u4 = getub(buf, 29);	/* utc offset */
	    s4 = getbes16(buf, 30);	/* tsip.gps_week */
	    /* PRN/IODE data follows */
	    gpsd_report(LOG_RAW,
			"LFwEI %d %d %d %u %d %u %u %x %x %u %u %d\n", s1, s2,
			s3, ul1, sl1, ul2, sl2, u1, u2, u3, u4, s4);

	    if ((u1 & 0x01) != (uint8_t) 0)	/* check velocity scaling */
		d5 = 0.02;
	    else
		d5 = 0.005;
	    d1 = (double)s1 * d5;	/* east velocity m/s */
	    d2 = (double)s2 * d5;	/* north velocity m/s */
	    session->newdata.climb = (double)s3 * d5;	/* up velocity m/s */
	    /*@ -evalorder @*/
	    session->newdata.speed = sqrt(pow(d2, 2) + pow(d1, 2));
	    /*@ +evalorder @*/
	    if ((session->newdata.track = atan2(d1, d2) * RAD_2_DEG) < 0)
		session->newdata.track += 360.0;
	    session->newdata.latitude = (double)sl1 * SEMI_2_DEG;
	    /*@i1@*/session->newdata.longitude = ul2 * SEMI_2_DEG;
	    if (session->newdata.longitude > 180.0)
		session->newdata.longitude -= 360.0;
	    session->gpsdata.separation =
		wgs84_separation(session->newdata.latitude,
				 session->newdata.longitude);
	    session->newdata.altitude =
		(double)sl2 * 1e-3 - session->gpsdata.separation;;
	    session->gpsdata.status = STATUS_NO_FIX;
	    session->newdata.mode = MODE_NO_FIX;
	    if ((u2 & 0x01) == (uint8_t) 0) {	/* Fix Available */
		session->gpsdata.status = STATUS_FIX;
		if ((u2 & 0x02) != (uint8_t) 0)	/* DGPS Corrected */
		    session->gpsdata.status = STATUS_DGPS_FIX;
		if ((u2 & 0x04) != (uint8_t) 0)	/* Fix Dimension */
		    session->newdata.mode = MODE_2D;
		else
		    session->newdata.mode = MODE_3D;
	    }
	    session->gpsdata.satellites_used = (int)u3;
	    if ((int)u4 > 10) {
		session->context->leap_seconds = (int)u4;
		session->context->valid |= LEAP_SECOND_VALID;
	    }
	    session->newdata.time = gpsd_gpstime_resolve(session,
						      (unsigned short)s4,
						      (double)ul1 *1e-3);
	    mask |=
		TIME_SET | PPSTIME_IS | LATLON_SET | ALTITUDE_SET | SPEED_SET |
		TRACK_SET | CLIMB_SET | STATUS_SET | MODE_SET | CLEAR_IS |
		REPORT_IS;
	    gpsd_report(LOG_DATA,
			"SP-LFEI 0x20: time=%.2f lat=%.2f lon=%.2f alt=%.2f "
			"speed=%.2f track=%.2f climb=%.2f "
			"mode=%d status=%d\n",
			session->newdata.time,
			session->newdata.latitude, session->newdata.longitude,
			session->newdata.altitude, session->newdata.speed,
			session->newdata.track, session->newdata.climb,
			session->newdata.mode, session->gpsdata.status);
	    break;
	case 0x23:		/* Compact Super Packet */
	    session->driver.tsip.req_compact = 0;
	    /* CSK sez "i don't trust this to not be oversized either." */
	    if (len < 29)
		break;
	    ul1 = getbeu32(buf, 1);	/* time */
	    s1 = getbes16(buf, 5);	/* tsip.gps_week */
	    u1 = getub(buf, 7);	/* utc offset */
	    u2 = getub(buf, 8);	/* fix flags */
	    sl1 = getbes32(buf, 9);	/* latitude */
	    ul2 = getbeu32(buf, 13);	/* longitude */
	    sl3 = getbes32(buf, 17);	/* altitude */
	    s2 = getbes16(buf, 21);	/* east velocity */
	    s3 = getbes16(buf, 23);	/* north velocity */
	    s4 = getbes16(buf, 25);	/* up velocity */
	    gpsd_report(LOG_INF, "CSP %u %d %u %u %d %u %d %d %d %d\n", ul1,
			s1, u1, u2, sl1, ul2, sl3, s2, s3, s4);
	    if ((int)u1 > 10) {
		session->context->leap_seconds = (int)u1;
		session->context->valid |= LEAP_SECOND_VALID;
	    }
	    session->newdata.time =
		gpsd_gpstime_resolve(session,
				  (unsigned short)s1, (double)ul1 *1e3);
	    session->gpsdata.status = STATUS_NO_FIX;
	    session->newdata.mode = MODE_NO_FIX;
	    if ((u2 & 0x01) == (uint8_t) 0) {	/* Fix Available */
		session->gpsdata.status = STATUS_FIX;
		if ((u2 & 0x02) != (uint8_t) 0)	/* DGPS Corrected */
		    session->gpsdata.status = STATUS_DGPS_FIX;
		if ((u2 & 0x04) != (uint8_t) 0)	/* Fix Dimension */
		    session->newdata.mode = MODE_2D;
		else
		    session->newdata.mode = MODE_3D;
	    }
	    session->newdata.latitude = (double)sl1 * SEMI_2_DEG;
	    session->newdata.longitude = (double)ul2 * SEMI_2_DEG;
	    if (session->newdata.longitude > 180.0)
		session->newdata.longitude -= 360.0;
	    session->gpsdata.separation =
		wgs84_separation(session->newdata.latitude,
				 session->newdata.longitude);
	    session->newdata.altitude =
		(double)sl3 * 1e-3 - session->gpsdata.separation;;
	    if ((u2 & 0x20) != (uint8_t) 0)	/* check velocity scaling */
		d5 = 0.02;
	    else
		d5 = 0.005;
	    d1 = (double)s2 * d5;	/* east velocity m/s */
	    d2 = (double)s3 * d5;	/* north velocity m/s */
	    session->newdata.climb = (double)s4 * d5;	/* up velocity m/s */
	    /*@ -evalorder @*/
	    session->newdata.speed =
		sqrt(pow(d2, 2) + pow(d1, 2)) * MPS_TO_KNOTS;
	    /*@ +evalorder @*/
	    if ((session->newdata.track = atan2(d1, d2) * RAD_2_DEG) < 0)
		session->newdata.track += 360.0;
	    mask |=
		TIME_SET | PPSTIME_IS | LATLON_SET | ALTITUDE_SET | SPEED_SET |
		TRACK_SET |CLIMB_SET | STATUS_SET | MODE_SET | CLEAR_IS |
		REPORT_IS;
	    gpsd_report(LOG_DATA,
			"SP-CSP 0x23: time=%.2f lat=%.2f lon=%.2f alt=%.2f "
			"speed=%.2f track=%.2f climb=%.2f mode=%d status=%d\n",
			session->newdata.time,
			session->newdata.latitude, session->newdata.longitude,
			session->newdata.altitude, session->newdata.speed,
			session->newdata.track, session->newdata.climb,
			session->newdata.mode, session->gpsdata.status);
	    break;

	case 0xab:		/* Thunderbolt Timing Superpacket */
	    if (len != 17) {
		gpsd_report(4, "pkt 0xab len=%d\n", len);
		break;
	    }
	    session->driver.tsip.last_41 = now;	/* keep timestamp for request */
	    ul1 = getbeu32(buf, 1);	/* gpstime */
	    s1 = (int16_t)getbeu16(buf, 5);	/* week */
	    s2 = getbes16(buf, 7);	/* leap seconds */

	    if ((int)u1 > 10) {
		session->context->leap_seconds = (int)s2;
		session->context->valid |= LEAP_SECOND_VALID;
		session->newdata.time =
		    gpsd_gpstime_resolve(session, (unsigned short)s1, (double)ul1);
		mask |= TIME_SET | PPSTIME_IS | CLEAR_IS;
		gpsd_report(LOG_DATA, "SP-TTS 0xab time=%.2f mask={TIME}\n",
			    session->newdata.time);
	    }

	    gpsd_report(4, "GPS Time %u %d %d\n", ul1, s1, s2);
	    break;


	case 0xac:		/* Thunderbolt Position Superpacket */
	    if (len != 68) {
		gpsd_report(4, "pkt 0xac len=%d\n", len);

		break;
	    }
	    session->newdata.latitude = getbed64((char *)buf, 36) * RAD_2_DEG;
	    session->newdata.longitude = getbed64((char *)buf, 44) * RAD_2_DEG;
	    session->newdata.altitude = getbed64((char *)buf, 52);
	    //f1 = getbef32((char *)buf, 16);    clock bias */

	    u1 = getub(buf, 12);	/* GPS Decoding Status */
	    u2 = getub(buf, 1);	/* Receiver Mode */
	    if (u1 != (uint8_t) 0) {
		session->gpsdata.status = STATUS_NO_FIX;
		mask |= STATUS_SET;
	    } else {
		if (session->gpsdata.status < STATUS_FIX) {
		    session->gpsdata.status = STATUS_FIX;
		    mask |= STATUS_SET;
		}
	    }

	    /* Decode Fix modes */
	    switch (u2 & 7) {
            case 0:     /* Auto */
                switch (u1) {
                       /*
			* According to the Thunderbolt Manual, the
                        * first byte of the supplemental timing packet
                        * simply indicates the configuration of the
                        * device, not the actual lock, so we need to
                        * look at the decode status.
			*/
                       case 0:   /* "Doing Fixes" */
                         session->newdata.mode = MODE_3D;
                         break;
                       case 0x0B: /* "Only 3 usable sats" */
                         session->newdata.mode = MODE_2D;
                         break;
                       case 0x1:   /* "Don't have GPS time" */
                       case 0x3:   /* "PDOP is too high" */
                       case 0x8:   /* "No usable sats" */
                       case 0x9:   /* "Only 1 usable sat" */
                       case 0x0A:  /* "Only 2 usable sats */
                       case 0x0C:  /* "The chosen sat is unusable" */
                       case 0x10:  /* TRAIM rejected the fix */
                       default:
                          session->newdata.mode = MODE_NO_FIX;
                }
		break;
	    case 6:		/* Clock Hold 2D */
	    case 3:		/* 2D Position Fix */
		//session->gpsdata.status = STATUS_FIX;
		session->newdata.mode = MODE_2D;
		break;
	    case 7:		/* Thunderbolt overdetermined clock */
	    case 4:		/* 3D position Fix */
		//session->gpsdata.status = STATUS_FIX;
		session->newdata.mode = MODE_3D;
		break;
	    default:
		//session->gpsdata.status = STATUS_NO_FIX;
		session->newdata.mode = MODE_NO_FIX;
		break;
	    }

	    mask |= LATLON_SET | ALTITUDE_SET | MODE_SET | REPORT_IS;
	    gpsd_report(LOG_DATA, "SP-TPS 0xac "
			"time=%.2f lat=%.2f lon=%.2f alt=%.2f\n",
			session->newdata.time,
			session->newdata.latitude,
			session->newdata.longitude,
			session->newdata.altitude);
	    break;

	default:
	    gpsd_report(LOG_WARN, "Unhandled TSIP superpacket type 0x%02x\n",
			u1);
	}
	break;
    case 0xbb:			/* Navigation Configuration */
	if (len != 40)
	    break;
	u1 = getub(buf, 0);	/* Subcode */
	u2 = getub(buf, 1);	/* Operating Dimension */
	u3 = getub(buf, 2);	/* DGPS Mode */
	u4 = getub(buf, 3);	/* Dynamics Code */
	f1 = getbef32((char *)buf, 5);	/* Elevation Mask */
	f2 = getbef32((char *)buf, 9);	/* AMU Mask */
	f3 = getbef32((char *)buf, 13);	/* DOP Mask */
	f4 = getbef32((char *)buf, 17);	/* DOP Switch */
	u5 = getub(buf, 21);	/* DGPS Age Limit */
	gpsd_report(LOG_INF,
		    "Navigation Configuration %u %u %u %u %f %f %f %f %u\n",
		    u1, u2, u3, u4, f1, f2, f3, f4, u5);
	break;
    default:
	gpsd_report(LOG_WARN, "Unhandled TSIP packet type 0x%02x\n", id);
	break;
    }

    /* see if it is time to send some request packets for reports that */
    /* the receiver won't send at fixed intervals */

    if ((now - session->driver.tsip.last_41) > 5) {
	/* Request Current Time */
	(void)tsip_write(session, 0x21, buf, 0);
	session->driver.tsip.last_41 = now;
    }

    if ((now - session->driver.tsip.last_6d) > 5) {
	/* Request GPS Receiver Position Fix Mode */
	(void)tsip_write(session, 0x24, buf, 0);
	session->driver.tsip.last_6d = now;
    }

    if ((now - session->driver.tsip.last_48) > 60) {
	/* Request GPS System Message */
	(void)tsip_write(session, 0x28, buf, 0);
	session->driver.tsip.last_48 = now;
    }

    if ((now - session->driver.tsip.last_5c) >= 5) {
	/* Request Current Satellite Tracking Status */
	putbyte(buf, 0, 0x00);	/* All satellites */
	(void)tsip_write(session, 0x3c, buf, 1);
	session->driver.tsip.last_5c = now;
    }

    if ((now - session->driver.tsip.last_46) > 5) {
	/* Request Health of Receiver */
	(void)tsip_write(session, 0x26, buf, 0);
	session->driver.tsip.last_46 = now;
    }
#if USE_SUPERPACKET
    if ((session->driver.tsip.req_compact > 0) &&
	((now - session->driver.tsip.req_compact) > 5)) {
	/* Compact Superpacket requested but no response */
	session->driver.tsip.req_compact = 0;
	gpsd_report(LOG_WARN, "No Compact Super Packet, use LFwEI\n");

	/* Request LFwEI Super Packet */
	putbyte(buf, 0, 0x20);
	putbyte(buf, 1, 0x01);	/* enabled */
	(void)tsip_write(session, 0x8e, buf, 2);
    }
#endif /* USE_SUPERPACKET */

    return mask;
}

static gps_mask_t tsip_parse_input(struct gps_device_t *session)
{
    gps_mask_t st;

    if (session->packet.type == TSIP_PACKET) {
	st = tsip_analyze(session);
	session->gpsdata.dev.driver_mode = MODE_BINARY;
	return st;
#ifdef EVERMORE_ENABLE
    } else if (session->packet.type == EVERMORE_PACKET) {
	(void)gpsd_switch_driver(session, "EverMore binary");
	st = evermore_parse(session, session->packet.outbuffer,
			    session->packet.outbuflen);
	session->gpsdata.dev.driver_mode = MODE_BINARY;
	return st;
#endif /* EVERMORE_ENABLE */
#ifdef SIRF_ENABLE
	/*
	 * mrd reported that once every couple of weeks his SiRF was flipping
	 * into Trimble binary mode and not recovering.  Damn Trimble for not
	 * checksumming their packets, it makes false positives hard to reject.
	 * This should enable the SiRF to recover.
	 */
    } else if (session->packet.type == SIRF_PACKET) {
	(void)gpsd_switch_driver(session, "SiRF binary");
	st = sirf_parse(session, session->packet.outbuffer,
			session->packet.outbuflen);
	session->gpsdata.dev.driver_mode = MODE_BINARY;
	return st;
#endif /* SIRF_ENABLE */
    } else
	return 0;
}

#ifdef CONTROLSEND_ENABLE
static ssize_t tsip_control_send(struct gps_device_t *session,
				 char *buf, size_t buflen)
/* not used by the daemon, it's for gpsctl and friends */
{
    return (ssize_t) tsip_write(session,
				(unsigned int)buf[0],
				(unsigned char *)buf + 1, buflen - 1);
}
#endif /* CONTROLSEND_ENABLE */

static void tsip_event_hook(struct gps_device_t *session, event_t event)
{
    if (session->context->readonly)
	return;
    /* FIX-ME: Resending this might not be needed on reactivation */
    if (event == event_identified || event == event_reactivate) {
	unsigned char buf[100];

	/* I/O Options */
	putbyte(buf, 0, 0x1e);	/* Position: DP, MSL, LLA */
	putbyte(buf, 1, 0x02);	/* Velocity: ENU */
	putbyte(buf, 2, 0x00);	/* Time: GPS */
	putbyte(buf, 3, 0x08);	/* Aux: dBHz */
	(void)tsip_write(session, 0x35, buf, 4);
    }
    if (event == event_configure) {
	unsigned char buf[100];

	switch (session->packet.counter) {
	case 0:
	    /*
	     * TSIP is ODD parity 1 stopbit, save original values and
	     * change it Thunderbolts and Copernicus use
	     * 8N1... which isn't exactly a good idea due to the
	     * fragile wire format.  We must divine a clever
	     * heuristic to decide if the parity change is required.
	     */
	    session->driver.tsip.parity = session->gpsdata.dev.parity;
	    session->driver.tsip.stopbits =
		(uint) session->gpsdata.dev.stopbits;
	    // gpsd_set_speed(session, session->gpsdata.dev.baudrate, 'O', 1);
	    break;

	case 1:
	    /*@ -shiftimplementation @*/
	    /* Request Software Versions */
	    (void)tsip_write(session, 0x1f, NULL, 0);
	    /* Request Current Time */
	    (void)tsip_write(session, 0x21, NULL, 0);
	    /* Set Operating Parameters */
	    /* - dynamic code: land */
	    putbyte(buf, 0, 0x01);
	    /* - elevation mask */
	    putbef32((char *)buf, 1, 5.0 * DEG_2_RAD);
	    /* - signal level mask */
	    putbef32((char *)buf, 5, 06.0);
	    /* - PDOP mask */
	    putbef32((char *)buf, 9, 8.0);
	    /* - PDOP switch */
	    putbef32((char *)buf, 13, 6.0);
	    /*@ +shiftimplementation @*/
	    (void)tsip_write(session, 0x2c, buf, 17);
	    /* Set Position Fix Mode (auto 2D/3D) */
	    putbyte(buf, 0, 0x00);
	    (void)tsip_write(session, 0x22, buf, 1);
	    /* Request GPS Systems Message */
	    (void)tsip_write(session, 0x28, NULL, 0);
	    /* Request Current Datum Values */
	    (void)tsip_write(session, 0x37, NULL, 0);
	    putbyte(buf, 0, 0x15);
	    (void)tsip_write(session, 0x8e, buf, 1);
	    /* Request Navigation Configuration */
	    putbyte(buf, 0, 0x03);
	    (void)tsip_write(session, 0xbb, buf, 1);
	    break;
	}
    }
    if (event == event_deactivate) {
	/* restore saved parity and stopbits when leaving TSIP mode */
	gpsd_set_speed(session,
		       session->gpsdata.dev.baudrate,
		       session->driver.tsip.parity,
		       session->driver.tsip.stopbits);
    }
}

#ifdef RECONFIGURE_ENABLE
static bool tsip_speed_switch(struct gps_device_t *session,
			      speed_t speed, char parity, int stopbits)
{
    unsigned char buf[100];

    switch (parity) {
    case 'E':
    case 2:
	parity = (char)2;
	break;
    case 'O':
    case 1:
	parity = (char)1;
	break;
    case 'N':
    case 0:
    default:
	parity = (char)0;
	break;
    }

    putbyte(buf, 0, 0xff);	/* current port */
    putbyte(buf, 1, (round(log((double)speed / 300) / M_LN2)) + 2);	/* input dev.baudrate */
    putbyte(buf, 2, getub(buf, 1));	/* output baudrate */
    putbyte(buf, 3, 3);		/* character width (8 bits) */
    putbyte(buf, 4, parity);	/* parity (normally odd) */
    putbyte(buf, 5, stopbits - 1);	/* stop bits (normally 1 stopbit) */
    putbyte(buf, 6, 0);		/* flow control (none) */
    putbyte(buf, 7, 0x02);	/* input protocol (TSIP) */
    putbyte(buf, 8, 0x02);	/* output protocol (TSIP) */
    putbyte(buf, 9, 0);		/* reserved */
    (void)tsip_write(session, 0xbc, buf, 10);

    return true;		/* it would be nice to error-check this */
}

static void tsip_mode(struct gps_device_t *session, int mode)
{
    if (mode == MODE_NMEA) {
	unsigned char buf[16];

	/* First turn on the NMEA messages we want */
	putbyte(buf, 0, 0x00);	/* subcode 0 */
	putbyte(buf, 1, 0x01);	/* 1-second fix interval */
	putbyte(buf, 2, 0x00);	/* Reserved */
	putbyte(buf, 3, 0x00);	/* Reserved */
	putbyte(buf, 4, 0x01);	/* 0=RMC, 1-7=Reserved */
	putbyte(buf, 5, 0x19);	/* 0=GGA, 1=GGL, 2=VTG, 3=GSV, */
	/* 4=GSA, 5=ZDA, 6-7=Reserved  */

	(void)tsip_write(session, 0x7A, buf, 6);

	/* Now switch to NMEA mode */

	memset(buf, 0, sizeof(buf));

	putbyte(buf, 0, 0xff);	/* current port */
	putbyte(buf, 1, 0x06);	/* 4800 bps input */
	putbyte(buf, 2, 0x06);	/* 4800 bps output */
	putbyte(buf, 3, 0x03);	/* 8 data bits */
	putbyte(buf, 4, 0x00);	/* No parity */
	putbyte(buf, 5, 0x00);	/* 1 stop bit */
	putbyte(buf, 6, 0x00);	/* No flow control */
	putbyte(buf, 7, 0x02);	/* Input protocol TSIP */
	putbyte(buf, 8, 0x04);	/* Output protocol NMEA */
	putbyte(buf, 9, 0x00);	/* Reserved */

	(void)tsip_write(session, 0xBC, buf, 10);

    } else if (mode == MODE_BINARY) {
	/* The speed switcher also puts us back in TSIP, so call it */
	/* with the default 9600 8O1. */
	// FIX-ME: Should preserve the current speed.
	// (void)tsip_speed_switch(session, 9600, 'O', 1);
	;

    } else {
	gpsd_report(LOG_ERROR, "unknown mode %i requested\n", mode);
    }
}
#endif /* RECONFIGURE_ENABLE */

#ifdef NTPSHM_ENABLE
static double tsip_ntp_offset(struct gps_device_t *session UNUSED)
{
    /* FIX-ME: is a constant offset right here? */
    return 0.075;
}
#endif /* NTPSHM_ENABLE */

/* this is everything we export */
/* *INDENT-OFF* */
const struct gps_type_t tsip_binary =
{
    .type_name      = "Trimble TSIP",	/* full name of type */
    .packet_type    = TSIP_PACKET,	/* associated lexer packet type */
    .flags	    = DRIVER_NOFLAGS,	/* no flags set */
    .trigger	    = NULL,		/* no trigger */
    .channels       = TSIP_CHANNELS,	/* consumer-grade GPS */
    .probe_detect   = tsip_detect,	/* probe for 9600O81 device */
    .get_packet     = generic_get,	/* use the generic packet getter */
    .parse_packet   = tsip_parse_input,	/* parse message packets */
    .rtcm_writer    = NULL,		/* doesn't accept DGPS corrections */
    .event_hook     = tsip_event_hook,	/* fire on various lifetime events */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher = tsip_speed_switch,/* change baud rate */
    .mode_switcher  = tsip_mode,	/* there is a mode switcher */
    .rate_switcher  = NULL,		/* no rate switcher */
    .min_cycle      = 1,		/* not relevant, no rate switcher */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send   = tsip_control_send,/* how to send commands */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = tsip_ntp_offset,
#endif /* NTPSHM_ENABLE */
};
/* *INDENT-ON* */

#endif /* TSIP_ENABLE */
