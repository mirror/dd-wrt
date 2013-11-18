/* $Id: zodiac.c 4030 2006-11-30 07:29:25Z esr $ */
/*
 * Handle the Rockwell binary packet format supported by the old Zodiac chipset
 */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "gpsd_config.h"
#include "gpsd.h"

#define LITTLE_ENDIAN_PROTOCOL
#include "bits.h"

#ifdef ZODIAC_ENABLE

struct header {
    unsigned short sync;
    unsigned short id;
    unsigned short ndata;
    unsigned short flags;
    unsigned short csum;
};

static unsigned short zodiac_checksum(unsigned short *w, int n)
{
    unsigned short csum = 0;

    while (n-- > 0)
	csum += *(w++);
    return -csum;
}

/* zodiac_spew - Takes a message type, an array of data words, and a length
   for the array, and prepends a 5 word header (including checksum).
   The data words are expected to be checksummed */
#if defined (WORDS_BIGENDIAN)
/* data is assumed to contain len/2 unsigned short words
 * we change the endianness to little, when needed.
 */
static int end_write(int fd, void *d, int len)
{
    char buf[BUFSIZ];
    char *p = buf;
    char *data = (char *)d;
    size_t n = (size_t)len;

    while (n>0) {
	*p++ = *(data+1); *p++ = *data;
	data += 2; n -= 2;
    }
    return write(fd, buf, len);
}
#else
#define end_write write
#endif

static void zodiac_spew(struct gps_device_t *session, int type, unsigned short *dat, int dlen)
{
    struct header h;
    int i;
    char buf[BUFSIZ];

    h.sync = 0x81ff;
    h.id = (unsigned short)type;
    h.ndata = (unsigned short)(dlen - 1);
    h.flags = 0;
    h.csum = zodiac_checksum((unsigned short *) &h, 4);

    if (session->gpsdata.gps_fd != -1) {
#ifdef ALLOW_RECONFIGURE
	(void)end_write(session->gpsdata.gps_fd, &h, sizeof(h));
	(void)end_write(session->gpsdata.gps_fd, dat, sizeof(unsigned short) * dlen);
#endif /* ALLOW_RECONFIGURE */
    }

    (void)snprintf(buf, sizeof(buf),
		   "%04x %04x %04x %04x %04x",
		   h.sync,h.id,h.ndata,h.flags,h.csum);
    for (i = 0; i < dlen; i++)
	(void)snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf),
		       " %04x", dat[i]);

    gpsd_report(LOG_RAW, "Sent Zodiac packet: %s\n",buf);
}

static bool zodiac_speed_switch(struct gps_device_t *session, speed_t speed)
{
    unsigned short data[15];

    if (session->driver.zodiac.sn++ > 32767)
	session->driver.zodiac.sn = 0;
      
    memset(data, 0, sizeof(data));
    /* data is the part of the message starting at word 6 */
    data[0] = session->driver.zodiac.sn;	/* sequence number */
    data[1] = 1;			/* port 1 data valid */
    data[2] = 1;			/* port 1 character width (8 bits) */
    data[3] = 0;			/* port 1 stop bits (1 stopbit) */
    data[4] = 0;			/* port 1 parity (none) */
    data[5] = (unsigned short)(round(log((double)speed/300)/M_LN2)+1); /* port 1 speed */
    data[14] = zodiac_checksum(data, 14);

    zodiac_spew(session, 1330, data, 15);
#ifdef ALLOW_RECONFIGURE
    return true; /* it would be nice to error-check this */
#else
    return false;
#endif /* ALLOW_RECONFIGURE */

}

static void send_rtcm(struct gps_device_t *session, 
		      char *rtcmbuf, size_t rtcmbytes)
{
    unsigned short data[34];
    int n = 1 + (int)(rtcmbytes/2 + rtcmbytes%2);

    if (session->driver.zodiac.sn++ > 32767)
	session->driver.zodiac.sn = 0;

    memset(data, 0, sizeof(data));
    data[0] = session->driver.zodiac.sn;		/* sequence number */
    memcpy(&data[1], rtcmbuf, rtcmbytes);
    data[n] = zodiac_checksum(data, n);

    zodiac_spew(session, 1351, data, n+1);
}

static ssize_t zodiac_send_rtcm(struct gps_device_t *session,
			char *rtcmbuf, size_t rtcmbytes)
{
    size_t len;

    while (rtcmbytes > 0) {
	len = (size_t)(rtcmbytes>64?64:rtcmbytes);
	send_rtcm(session, rtcmbuf, len);
	rtcmbytes -= len;
	rtcmbuf += len;
    }
    return 1;
}

static gps_mask_t handle1000(struct gps_device_t *session)
{
    double subseconds;
    struct tm unpacked_date;
    /* ticks                      = getlong(6); */
    /* sequence                   = getword(8); */
    /* measurement_sequence       = getword(9); */
    /*@ -boolops -predboolothers @*/
    session->gpsdata.status       = (getword(10) & 0x1c) ? 0 : 1;
    if (session->gpsdata.status != 0)
	session->gpsdata.fix.mode = (getword(10) & 1) ? MODE_2D : MODE_3D;
    else
	session->gpsdata.fix.mode = MODE_NO_FIX;
    /*@ +boolops -predboolothers @*/

    /* solution_type                 = getword(11); */
    session->gpsdata.satellites_used = (int)getword(12);
    /* polar_navigation              = getword(13); */
    /* gps_week                      = getword(14); */
    /* gps_seconds                   = getlong(15); */
    /* gps_nanoseconds               = getlong(17); */
    unpacked_date.tm_mday = (int)getword(19);
    unpacked_date.tm_mon = (int)getword(20) - 1;
    unpacked_date.tm_year = (int)getword(21) - 1900;
    unpacked_date.tm_hour = (int)getword(22);
    unpacked_date.tm_min = (int)getword(23);
    unpacked_date.tm_sec = (int)getword(24);
    subseconds = (int)getlong(25) / 1e9;
    /*@ -compdef */
    session->gpsdata.fix.time = session->gpsdata.sentence_time =
	(double)mkgmtime(&unpacked_date) + subseconds;
    /*@ +compdef */
#ifdef NTPSHM_ENABLE
    if (session->context->enable_ntpshm && session->gpsdata.fix.mode > MODE_NO_FIX)
	(void)ntpshm_put(session, session->gpsdata.fix.time + 1.1);
#endif
    /*@ -type @*/
    session->gpsdata.fix.latitude  = ((long)getlong(27)) * RAD_2_DEG * 1e-8;
    session->gpsdata.fix.longitude = ((long)getlong(29)) * RAD_2_DEG * 1e-8;
    /*
     * The Rockwell Jupiter TU30-D140 reports altitude as uncorrected height
     * above WGS84 geoid.  The Zodiac binary protocol manual does not 
     * specify whether word 31 is geodetic or WGS 84. 
     */
    session->gpsdata.fix.altitude  = ((long)getlong(31)) * 1e-2;
    /*@ +type @*/
    session->gpsdata.separation    = ((short)getword(33)) * 1e-2;
    session->gpsdata.fix.altitude -= session->gpsdata.separation;
    session->gpsdata.fix.speed     = (int)getlong(34) * 1e-2;
    session->gpsdata.fix.track     = (int)getword(36) * RAD_2_DEG * 1e-3;
    session->mag_var               = ((short)getword(37)) * RAD_2_DEG * 1e-4;
    session->gpsdata.fix.climb     = ((short)getword(38)) * 1e-2;
    /* map_datum                   = getword(39); */
    /* manual says these are 1-sigma */
    session->gpsdata.fix.eph       = (int)getlong(40) * 1e-2 * GPSD_CONFIDENCE;
    session->gpsdata.fix.epv       = (int)getlong(42) * 1e-2 * GPSD_CONFIDENCE;
    session->gpsdata.fix.ept       = (int)getlong(44) * 1e-2 * GPSD_CONFIDENCE;
    session->gpsdata.fix.eps       = (int)getword(46) * 1e-2 * GPSD_CONFIDENCE;
    /* clock_bias                  = (int)getlong(47) * 1e-2; */
    /* clock_bias_sd               = (int)getlong(49) * 1e-2; */
    /* clock_drift                 = (int)getlong(51) * 1e-2; */
    /* clock_drift_sd              = (int)getlong(53) * 1e-2; */

#if 0
    gpsd_report(LOG_INF, "date: %lf\n", session->gpsdata.fix.time);
    gpsd_report(LOG_INF, "  solution invalid:\n");
    gpsd_report(LOG_INF, "    altitude: %d\n", (getword(10) & 1) ? 1 : 0);
    gpsd_report(LOG_INF, "    no diff gps: %d\n", (getword(10) & 2) ? 1 : 0);
    gpsd_report(LOG_INF, "    not enough satellites: %d\n", (getword(10) & 4) ? 1 : 0);
    gpsd_report(LOG_INF, "    exceed max EHPE: %d\n", (getword(10) & 8) ? 1 : 0);
    gpsd_report(LOG_INF, "    exceed max EVPE: %d\n", (getword(10) & 16) ? 1 : 0);
    gpsd_report(LOG_INF, "  solution type:\n");
    gpsd_report(LOG_INF, "    propagated: %d\n", (getword(11) & 1) ? 1 : 0);
    gpsd_report(LOG_INF, "    altitude: %d\n", (getword(11) & 2) ? 1 : 0);
    gpsd_report(LOG_INF, "    differential: %d\n", (getword(11) & 4) ? 1 : 0);
    gpsd_report(LOG_INF, "Number of measurements in solution: %d\n", getword(12));
    gpsd_report(LOG_INF, "Lat: %f\n", getlong(27) * RAD_2_DEG * 1e-8);
    gpsd_report(LOG_INF, "Lon: %f\n", getlong(29) * RAD_2_DEG * 1e-8);
    gpsd_report(LOG_INF, "Alt: %f\n", (double) getlong(31) * 1e-2);
    gpsd_report(LOG_INF, "Speed: %f\n", (double) getlong(34) * 1e-2 * MPS_TO_KNOTS);
    gpsd_report(LOG_INF, "Map datum: %d\n", getword(39));
    gpsd_report(LOG_INF, "Magnetic variation: %f\n", getword(37) * RAD_2_DEG * 1e-4);
    gpsd_report(LOG_INF, "Course: %f\n", getword(36) * RAD_2_DEG * 1e-4);
    gpsd_report(LOG_INF, "Separation: %f\n", getword(33) * 1e-2);
#endif

    session->gpsdata.sentence_length = 55;
    return TIME_SET|LATLON_SET|ALTITUDE_SET|CLIMB_SET|SPEED_SET|TRACK_SET|STATUS_SET|MODE_SET|CYCLE_START_SET; /* |HERR_SET|VERR_SET|SPEEDERR_SET */
}

static gps_mask_t handle1002(struct gps_device_t *session)
{
    int i, j, status, prn;

    session->gpsdata.satellites_used = 0;
    memset(session->gpsdata.used,0,sizeof(session->gpsdata.used));
    /* ticks                      = getlong(6); */
    /* sequence                   = getword(8); */
    /* measurement_sequence       = getword(9); */
    /* gps_week                   = getword(10); */
    /* gps_seconds                = getlong(11); */
    /* gps_nanoseconds            = getlong(13); */
    for (i = 0; i < ZODIAC_CHANNELS; i++) {
	/*@ -type @*/ 
	session->driver.zodiac.Zv[i] = status = (int)getword(15 + (3 * i));
	session->driver.zodiac.Zs[i] = prn = (int)getword(16 + (3 * i));
	/*@ +type @*/ 
#if 0
	gpsd_report(LOG_INF, "Sat%02d:\n", i);
	gpsd_report(LOG_INF, " used:%d\n", (status & 1) ? 1 : 0);
	gpsd_report(LOG_INF, " eph:%d\n", (status & 2) ? 1 : 0);
	gpsd_report(LOG_INF, " val:%d\n", (status & 4) ? 1 : 0);
	gpsd_report(LOG_INF, " dgps:%d\n", (status & 8) ? 1 : 0);
	gpsd_report(LOG_INF, " PRN:%d\n", prn);
	gpsd_report(LOG_INF, " C/No:%d\n", getword(17 + (3 * i)));
#endif
	if (status & 1)
	    session->gpsdata.used[session->gpsdata.satellites_used++] = prn;
	for (j = 0; j < ZODIAC_CHANNELS; j++) {
	    if (session->gpsdata.PRN[j] != prn)
		continue;
	    session->gpsdata.ss[j] = (int)getword(17 + (3 * i));
	    break;
	}
    }
    return SATELLITE_SET | USED_SET;
}

static gps_mask_t handle1003(struct gps_device_t *session)
{
    int i;

    /* ticks              = getlong(6); */
    /* sequence           = getword(8); */
    session->gpsdata.gdop = (unsigned int)getword(9) * 1e-2;
    session->gpsdata.pdop = (unsigned int)getword(10) * 1e-2;
    session->gpsdata.hdop = (unsigned int)getword(11) * 1e-2;
    session->gpsdata.vdop = (unsigned int)getword(12) * 1e-2;
    session->gpsdata.tdop = (unsigned int)getword(13) * 1e-2;
    session->gpsdata.satellites = (int)getword(14);

    for (i = 0; i < ZODIAC_CHANNELS; i++) {
	if (i < session->gpsdata.satellites) {
	    session->gpsdata.PRN[i] = (int)getword(15 + (3 * i));
	    session->gpsdata.azimuth[i] = (int)(((short)getword(16 + (3 * i))) * RAD_2_DEG * 1e-4);
	    if (session->gpsdata.azimuth[i] < 0)
		session->gpsdata.azimuth[i] += 360;
	    session->gpsdata.elevation[i] = (int)(((short)getword(17 + (3 * i))) * RAD_2_DEG * 1e-4);
#if 0
	    gpsd_report(LOG_INF, "Sat%02d:  PRN:%d az:%d el:%d\n", 
			i, getword(15+(3 * i)),getword(16+(3 * i)),getword(17+(3 * i)));
#endif
	} else {
	    session->gpsdata.PRN[i] = 0;
	    session->gpsdata.azimuth[i] = 0;
	    session->gpsdata.elevation[i] = 0;
	}
    }
    return SATELLITE_SET | HDOP_SET | VDOP_SET | PDOP_SET;
}

static void handle1005(struct gps_device_t *session UNUSED)
{
    /* ticks              = getlong(6); */
    /* sequence           = getword(8); */
    int numcorrections = (int)getword(12);
#if 0
    int i;

    gpsd_report(LOG_INF, "Packet: %d\n", session->driver.zodiac.sn);
    gpsd_report(LOG_INF, "Station bad: %d\n", (getword(9) & 1) ? 1 : 0);
    gpsd_report(LOG_INF, "User disabled: %d\n", (getword(9) & 2) ? 1 : 0);
    gpsd_report(LOG_INF, "Station ID: %d\n", getword(10));
    gpsd_report(LOG_INF, "Age of last correction in seconds: %d\n", getword(11));
    gpsd_report(LOG_INF, "Number of corrections: %d\n", getword(12));
    for (i = 0; i < numcorrections; i++) {
	gpsd_report(LOG_INF, "Sat%02d:\n", getword(13+i) & 0x3f);
	gpsd_report(LOG_INF, "ephemeris:%d\n", (getword(13+i) & 64) ? 1 : 0);
	gpsd_report(LOG_INF, "rtcm corrections:%d\n", (getword(13+i) & 128) ? 1 : 0);
	gpsd_report(LOG_INF, "rtcm udre:%d\n", (getword(13+i) & 256) ? 1 : 0);
	gpsd_report(LOG_INF, "sat health:%d\n", (getword(13+i) & 512) ? 1 : 0);
	gpsd_report(LOG_INF, "rtcm sat health:%d\n", (getword(13+i) & 1024) ? 1 : 0);
	gpsd_report(LOG_INF, "corrections state:%d\n", (getword(13+i) & 2048) ? 1 : 0);
	gpsd_report(LOG_INF, "iode mismatch:%d\n", (getword(13+i) & 4096) ? 1 : 0);
    }
#endif
    if (session->gpsdata.fix.mode == MODE_NO_FIX)
	session->gpsdata.status = STATUS_NO_FIX;
    else if (numcorrections == 0)
	session->gpsdata.status = STATUS_FIX;
    else
	session->gpsdata.status = STATUS_DGPS_FIX;
}

static gps_mask_t handle1011(struct gps_device_t *session)
{
    /*
     * This is UNTESTED -- but harmless if buggy.  Added to support
     * client querying of the ID with firmware version in 2006.
     * The Zodiac is supposed to send one of these messages on startup.
     */
    getstring(session->subtype, 19, 28);	/* software version field */
    gpsd_report(LOG_INF, "Software version: %s\n", session->subtype);
    return DEVICEID_SET;
}


static void handle1108(struct gps_device_t *session)
{
    /* ticks              = getlong(6); */
    /* sequence           = getword(8); */
    /* utc_week_seconds   = getlong(14); */
    /* leap_nanoseconds   = getlong(17); */
    if ((int)(getword(19) & 3) == 3)
	session->context->leap_seconds = (int)getword(16);
#if 0
    gpsd_report(LOG_INF, "Leap seconds: %d.%09d\n", getword(16), getlong(17));
    gpsd_report(LOG_INF, "UTC validity: %d\n", getword(19) & 3);
#endif
}

static gps_mask_t zodiac_analyze(struct gps_device_t *session)
{
    char buf[BUFSIZ];
    int i;
    unsigned int id = (unsigned int)((session->packet.outbuffer[3]<<8) | session->packet.outbuffer[2]);

    if (session->packet.type != ZODIAC_PACKET) {
	struct gps_type_t **dp;
	gpsd_report(LOG_PROG, "zodiac_analyze packet type %d\n",session->packet.type);
 	// Wrong packet type ? 
	// Maybe find a trigger just in case it's an Earthmate
	gpsd_report(LOG_RAW+4, "Is this a trigger: %s ?\n", (char*)session->packet.outbuffer);

	for (dp = gpsd_drivers; *dp; dp++) {
	    char	*trigger = (*dp)->trigger;

	    if (trigger!=NULL && strncmp((char *)session->packet.outbuffer, trigger, strlen(trigger))==0 && isatty(session->gpsdata.gps_fd)!=0) {
		gpsd_report(LOG_PROG, "found %s.\n", trigger);
		    
		(void)gpsd_switch_driver(session, (*dp)->typename);
		return 0;
	    }
   	}
        return 0;
    }

    buf[0] = '\0';
    for (i = 0; i < (int)session->packet.outbuflen; i++)
	(void)snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf),
		       "%02x", (unsigned int)session->packet.outbuffer[i]);
    (void)strlcat(buf, "\n", BUFSIZ);
    gpsd_report(LOG_RAW, "Raw Zodiac packet type %d length %d: %s\n",id,session->packet.outbuflen,buf);

    if (session->packet.outbuflen < 10)
	return 0;

    (void)snprintf(session->gpsdata.tag,sizeof(session->gpsdata.tag),"%u",id);

    switch (id) {
    case 1000:
	return handle1000(session);
    case 1002:
	return handle1002(session);
    case 1003:
	return handle1003(session);
    case 1005:
	handle1005(session);
	return 0;	
    case 1011:
	return handle1011(session);
    case 1108:
	handle1108(session);
	return 0;
    default:
	return 0;
    }
}

/* caller needs to specify a wrapup function */

/* this is everything we export */
struct gps_type_t zodiac_binary =
{
    .typename       = "Zodiac binary",	/* full name of type */
    .trigger        = NULL,		/* no trigger */
    .channels       = 12,		/* consumer-grade GPS */
    .probe_wakeup   = NULL,		/* no probe on baud rate change */
    .probe_detect   = NULL,		/* no probe */
    .probe_subtype  = NULL,		/* no initialization */
#ifdef ALLOW_RECONFIGURE
    .configurator   = NULL,		/* no configuration */
#endif /* ALLOW_RECONFIGURE */
    .get_packet     = generic_get,	/* use the generic packet getter */
    .parse_packet   = zodiac_analyze,	/* parse message packets */
    .rtcm_writer    = zodiac_send_rtcm,	/* send DGPS correction */
    .speed_switcher = zodiac_speed_switch,/* we can change baud rate */
    .mode_switcher  = NULL,		/* no mode switcher */
    .rate_switcher  = NULL,		/* no sample-rate switcher */
    .cycle_chars    = -1,		/* not relevant, no rate switch */
#ifdef ALLOW_RECONFIGURE
    .revert         = NULL,		/* no reversion hook */
#endif /* ALLOW_RECONFIGURE */
    .wrapup         = NULL,		/* caller might supply a close hook */
    .cycle          = 1,		/* updates every second */
};

#endif /* ZODIAC_ENABLE */
