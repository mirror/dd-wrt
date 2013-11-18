/*
 * Handle the Rockwell binary packet format supported by the old Zodiac chipset
 *
 * Week counters are not limited to 10 bits. It's unknown what
 * the firmware is doing to disambiguate them, if anything; it might just
 * be adding a fixed offset based on a hidden epoch value, in which case
 * unhappy things will occur on the next rollover.
 *
 * This file is Copyright (c) 2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#ifndef S_SPLINT_S
#include <unistd.h>
#endif /* S_SPLINT_S */

#include "gpsd.h"

/* Zodiac protocol description uses 1-origin indexing by little-endian word */
#define get16z(buf, n)	( (buf[2*(n)-2])	\
		| (buf[2*(n)-1] << 8))
#define get32z(buf, n)	( (buf[2*(n)-2])	\
		| (buf[2*(n)-1] << 8) \
		| (buf[2*(n)+0] << 16) \
		| (buf[2*(n)+1] << 24))
#define getstringz(to, from, s, e)			\
    (void)memcpy(to, from+2*(s)-2, 2*((e)-(s)+1))

#ifdef ZODIAC_ENABLE
struct header
{
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
    size_t n = (size_t) len;

    while (n > 0) {
	*p++ = *(data + 1);
	*p++ = *data;
	data += 2;
	n -= 2;
    }
    return write(fd, buf, len);
}
#else
#define end_write write
#endif /* WORDS_BIGENDIAN */

static ssize_t zodiac_spew(struct gps_device_t *session, unsigned short type,
			   unsigned short *dat, int dlen)
{
    struct header h;
    int i;
    char buf[BUFSIZ];

    h.sync = 0x81ff;
    h.id = (unsigned short)type;
    h.ndata = (unsigned short)(dlen - 1);
    h.flags = 0;
    h.csum = zodiac_checksum((unsigned short *)&h, 4);

    if (!BAD_SOCKET(session->gpsdata.gps_fd)) {
	size_t hlen, datlen;
	hlen = sizeof(h);
	datlen = sizeof(unsigned short) * dlen;
	if (end_write(session->gpsdata.gps_fd, &h, hlen) != (ssize_t) hlen ||
	    end_write(session->gpsdata.gps_fd, dat,
		      datlen) != (ssize_t) datlen) {
	    gpsd_report(LOG_RAW, "Reconfigure write failed\n");
	    return -1;
	}
    }

    (void)snprintf(buf, sizeof(buf),
		   "%04x %04x %04x %04x %04x",
		   h.sync, h.id, h.ndata, h.flags, h.csum);
    for (i = 0; i < dlen; i++)
	(void)snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
		       " %04x", dat[i]);

    gpsd_report(LOG_RAW, "Sent Zodiac packet: %s\n", buf);

    return 0;
}

static void send_rtcm(struct gps_device_t *session,
		      const char *rtcmbuf, size_t rtcmbytes)
{
    unsigned short data[34];
    int n = 1 + (int)(rtcmbytes / 2 + rtcmbytes % 2);

    if (session->driver.zodiac.sn++ > 32767)
	session->driver.zodiac.sn = 0;

    memset(data, 0, sizeof(data));
    data[0] = session->driver.zodiac.sn;	/* sequence number */
    memcpy(&data[1], rtcmbuf, rtcmbytes);
    data[n] = zodiac_checksum(data, n);

    (void)zodiac_spew(session, 1351, data, n + 1);
}

static ssize_t zodiac_send_rtcm(struct gps_device_t *session,
				const char *rtcmbuf, size_t rtcmbytes)
{
    size_t len;

    while (rtcmbytes > 0) {
	len = (size_t) (rtcmbytes > 64 ? 64 : rtcmbytes);
	send_rtcm(session, rtcmbuf, len);
	rtcmbytes -= len;
	rtcmbuf += len;
    }
    return 1;
}

#define getzword(n)	get16z(session->packet.outbuffer, n)
#define getzlong(n)	get32z(session->packet.outbuffer, n)

static gps_mask_t handle1000(struct gps_device_t *session)
/* time-position-velocity report */
{
    gps_mask_t mask;
    double subseconds;
    struct tm unpacked_date;
    /* ticks                      = getzlong(6); */
    /* sequence                   = getzword(8); */
    /* measurement_sequence       = getzword(9); */
    /*@ -boolops -predboolothers @*/
    session->gpsdata.status = (getzword(10) & 0x1c) ? 0 : 1;
    if (session->gpsdata.status != 0)
	session->newdata.mode = (getzword(10) & 1) ? MODE_2D : MODE_3D;
    else
	session->newdata.mode = MODE_NO_FIX;
    /*@ +boolops -predboolothers @*/

    /* solution_type                 = getzword(11); */
    session->gpsdata.satellites_used = (int)getzword(12);
    /* polar_navigation              = getzword(13); */
    session->context->gps_week = (unsigned short)getzword(14);
    /* gps_seconds                   = getzlong(15); */
    /* gps_nanoseconds               = getzlong(17); */
    unpacked_date.tm_mday = (int)getzword(19);
    unpacked_date.tm_mon = (int)getzword(20) - 1;
    unpacked_date.tm_year = (int)getzword(21) - 1900;
    unpacked_date.tm_hour = (int)getzword(22);
    unpacked_date.tm_min = (int)getzword(23);
    unpacked_date.tm_sec = (int)getzword(24);
    subseconds = (int)getzlong(25) / 1e9;
    /*@ -compdef */
    session->newdata.time = (timestamp_t)mkgmtime(&unpacked_date) + subseconds;
    /*@ +compdef */
    /*@ -type @*/
    session->newdata.latitude = ((long)getzlong(27)) * RAD_2_DEG * 1e-8;
    session->newdata.longitude = ((long)getzlong(29)) * RAD_2_DEG * 1e-8;
    /*
     * The Rockwell Jupiter TU30-D140 reports altitude as uncorrected height
     * above WGS84 geoid.  The Zodiac binary protocol manual does not
     * specify whether word 31 is geodetic or WGS 84.
     */
    session->newdata.altitude = ((long)getzlong(31)) * 1e-2;
    /*@ +type @*/
    session->gpsdata.separation = ((short)getzword(33)) * 1e-2;
    session->newdata.altitude -= session->gpsdata.separation;
    session->newdata.speed = (int)getzlong(34) * 1e-2;
    session->newdata.track = (int)getzword(36) * RAD_2_DEG * 1e-3;
    session->mag_var = ((short)getzword(37)) * RAD_2_DEG * 1e-4;
    session->newdata.climb = ((short)getzword(38)) * 1e-2;
    /* map_datum                   = getzword(39); */
    /*
     * The manual says these are 1-sigma.  Device reports only eph, circular
     * error; no harm in assigning it to both x and y components.
     */
    session->newdata.epx = session->newdata.epy =
	(int)getzlong(40) * 1e-2 * (1 / sqrt(2)) * GPSD_CONFIDENCE;
    session->newdata.epv = (int)getzlong(42) * 1e-2 * GPSD_CONFIDENCE;
    session->newdata.ept = (int)getzlong(44) * 1e-2 * GPSD_CONFIDENCE;
    session->newdata.eps = (int)getzword(46) * 1e-2 * GPSD_CONFIDENCE;
    /* clock_bias                  = (int)getzlong(47) * 1e-2; */
    /* clock_bias_sd               = (int)getzlong(49) * 1e-2; */
    /* clock_drift                 = (int)getzlong(51) * 1e-2; */
    /* clock_drift_sd              = (int)getzlong(53) * 1e-2; */

    mask =
	TIME_SET | PPSTIME_IS | LATLON_SET | ALTITUDE_SET | CLIMB_SET | SPEED_SET |
	TRACK_SET | STATUS_SET | MODE_SET;
    gpsd_report(LOG_DATA,
		"1000: time=%.2f lat=%.2f lon=%.2f alt=%.2f track=%.2f speed=%.2f climb=%.2f mode=%d status=%d\n",
		session->newdata.time, session->newdata.latitude,
		session->newdata.longitude, session->newdata.altitude,
		session->newdata.track, session->newdata.speed,
		session->newdata.climb, session->newdata.mode,
		session->gpsdata.status);
    return mask;
}

static gps_mask_t handle1002(struct gps_device_t *session)
/* satellite signal quality report */
{
    int i, j, status, prn;

    /* ticks                      = getzlong(6); */
    /* sequence                   = getzword(8); */
    /* measurement_sequence       = getzword(9); */
    /*@+charint@*/
    int gps_week = getzword(10);
    int gps_seconds = getzlong(11);
    /* gps_nanoseconds            = getzlong(13); */
    /*@-charint@*/
    /* Note: this week counter is not limited to 10 bits. */
    session->context->gps_week = (unsigned short)gps_week;
    session->gpsdata.satellites_used = 0;
    memset(session->gpsdata.used, 0, sizeof(session->gpsdata.used));
    for (i = 0; i < ZODIAC_CHANNELS; i++) {
	/*@ -type @*/
	session->driver.zodiac.Zv[i] = status = (int)getzword(15 + (3 * i));
	session->driver.zodiac.Zs[i] = prn = (int)getzword(16 + (3 * i));
	/*@ +type @*/

	if (status & 1)
	    session->gpsdata.used[session->gpsdata.satellites_used++] = prn;
	for (j = 0; j < ZODIAC_CHANNELS; j++) {
	    if (session->gpsdata.PRN[j] != prn)
		continue;
	    session->gpsdata.ss[j] = (float)getzword(17 + (3 * i));
	    break;
	}
    }
    session->gpsdata.skyview_time = gpsd_gpstime_resolve(session,
						      (unsigned short)gps_week,
						      (double)gps_seconds);
    gpsd_report(LOG_DATA, "1002: visible=%d used=%d mask={SATELLITE|USED}\n",
		session->gpsdata.satellites_visible,
		session->gpsdata.satellites_used);
    return SATELLITE_SET | USED_IS;
}

static gps_mask_t handle1003(struct gps_device_t *session)
/* skyview report */
{
    int i, n;

    /* The Polaris (and probably the DAGR) emit some strange variant of
     * this message which causes gpsd to crash filtering on impossible
     * number of satellites avoids this */
    n = (int)getzword(14);
    if ((n < 0) || (n > 12))
	return 0;

    gpsd_zero_satellites(&session->gpsdata);

    /* ticks              = getzlong(6); */
    /* sequence           = getzword(8); */
    session->gpsdata.dop.gdop = (unsigned int)getzword(9) * 1e-2;
    session->gpsdata.dop.pdop = (unsigned int)getzword(10) * 1e-2;
    session->gpsdata.dop.hdop = (unsigned int)getzword(11) * 1e-2;
    session->gpsdata.dop.vdop = (unsigned int)getzword(12) * 1e-2;
    session->gpsdata.dop.tdop = (unsigned int)getzword(13) * 1e-2;
    session->gpsdata.satellites_visible = n;

    for (i = 0; i < ZODIAC_CHANNELS; i++) {
	if (i < session->gpsdata.satellites_visible) {
	    session->gpsdata.PRN[i] = (int)getzword(15 + (3 * i));
	    session->gpsdata.azimuth[i] =
		(int)(((short)getzword(16 + (3 * i))) * RAD_2_DEG * 1e-4);
	    if (session->gpsdata.azimuth[i] < 0)
		session->gpsdata.azimuth[i] += 360;
	    session->gpsdata.elevation[i] =
		(int)(((short)getzword(17 + (3 * i))) * RAD_2_DEG * 1e-4);
	} else {
	    session->gpsdata.PRN[i] = 0;
	    session->gpsdata.azimuth[i] = 0;
	    session->gpsdata.elevation[i] = 0;
	}
    }
    session->gpsdata.skyview_time = NAN;
    gpsd_report(LOG_DATA, "NAVDOP: visible=%d gdop=%.2f pdop=%.2f "
		"hdop=%.2f vdop=%.2f tdop=%.2f mask={SATELLITE|DOP}\n",
		session->gpsdata.satellites_visible,
		session->gpsdata.dop.gdop,
		session->gpsdata.dop.hdop,
		session->gpsdata.dop.vdop,
		session->gpsdata.dop.pdop, session->gpsdata.dop.tdop);
    return SATELLITE_SET | DOP_SET;
}

static void handle1005(struct gps_device_t *session UNUSED)
/* fix quality report */
{
    /* ticks              = getzlong(6); */
    /* sequence           = getzword(8); */
    int numcorrections = (int)getzword(12);

    if (session->newdata.mode == MODE_NO_FIX)
	session->gpsdata.status = STATUS_NO_FIX;
    else if (numcorrections == 0)
	session->gpsdata.status = STATUS_FIX;
    else
	session->gpsdata.status = STATUS_DGPS_FIX;
}

static gps_mask_t handle1011(struct gps_device_t *session)
/* version report */
{
    /*
     * This is UNTESTED -- but harmless if buggy.  Added to support
     * client querying of the ID with firmware version in 2006.
     * The Zodiac is supposed to send one of these messages on startup.
     */
    getstringz(session->subtype, session->packet.outbuffer, 19, 28);	/* software version field */
    gpsd_report(LOG_DATA, "1011: subtype=%s mask={DEVICEID}\n",
		session->subtype);
    return DEVICEID_SET;
}


static void handle1108(struct gps_device_t *session)
/* leap-second correction report */
{
    /* ticks              = getzlong(6); */
    /* sequence           = getzword(8); */
    /* utc_week_seconds   = getzlong(14); */
    /* leap_nanoseconds   = getzlong(17); */
    if ((int)(getzword(19) & 3) == 3) {
	session->context->valid |= LEAP_SECOND_VALID;
	session->context->leap_seconds = (int)getzword(16);
    }
}

static gps_mask_t zodiac_analyze(struct gps_device_t *session)
{
    char buf[BUFSIZ];
    int i;
    unsigned int id =
	(unsigned int)((session->packet.outbuffer[3] << 8) |
		       session->packet.outbuffer[2]);

    if (session->packet.type != ZODIAC_PACKET) {
	const struct gps_type_t **dp;
	gpsd_report(LOG_PROG, "zodiac_analyze packet type %d\n",
		    session->packet.type);
	// Wrong packet type ?
	// Maybe find a trigger just in case it's an Earthmate
	gpsd_report(LOG_RAW + 4, "Is this a trigger: %s ?\n",
		    (char *)session->packet.outbuffer);

	for (dp = gpsd_drivers; *dp; dp++) {
	    char *trigger = (*dp)->trigger;

	    if (trigger != NULL
		&& strncmp((char *)session->packet.outbuffer, trigger,
			   strlen(trigger)) == 0
		&& isatty(session->gpsdata.gps_fd) != 0) {
		gpsd_report(LOG_PROG, "found %s.\n", trigger);

		(void)gpsd_switch_driver(session, (*dp)->type_name);
		return 0;
	    }
	}
	return 0;
    }

    buf[0] = '\0';
    for (i = 0; i < (int)session->packet.outbuflen; i++)
	(void)snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
		       "%02x", (unsigned int)session->packet.outbuffer[i]);
    gpsd_report(LOG_RAW, "Raw Zodiac packet type %d length %zd: %s\n",
		id, session->packet.outbuflen, buf);

    if (session->packet.outbuflen < 10)
	return 0;

    (void)snprintf(session->gpsdata.tag, sizeof(session->gpsdata.tag), "%u",
		   id);

    /*
     * Normal cycle for these devices is 1001 1002.
     * We count 1001 as end of cycle because 1002 doesn't
     * carry fix information.
     */
    session->cycle_end_reliable = true;

    switch (id) {
    case 1000:
	return handle1000(session) | (CLEAR_IS | REPORT_IS);
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

#ifdef CONTROLSEND_ENABLE
static ssize_t zodiac_control_send(struct gps_device_t *session,
				   char *msg, size_t len)
{
    unsigned short *shortwords = (unsigned short *)msg;

    /* and if len isn't even, it's your own fault */
    return zodiac_spew(session, shortwords[0], shortwords + 1,
		       (int)(len / 2 - 1));
}
#endif /* CONTROLSEND_ENABLE */

#ifdef RECONFIGURE_ENABLE
static bool zodiac_speed_switch(struct gps_device_t *session,
				speed_t speed, char parity, int stopbits)
{
    unsigned short data[15];

    if (session->driver.zodiac.sn++ > 32767)
	session->driver.zodiac.sn = 0;

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

    memset(data, 0, sizeof(data));
    /* data is the part of the message starting at word 6 */
    data[0] = session->driver.zodiac.sn;	/* sequence number */
    data[1] = 1;		/* port 1 data valid */
    data[2] = (unsigned short)parity;	/* port 1 character width (8 bits) */
    data[3] = (unsigned short)(stopbits - 1);	/* port 1 stop bits (1 stopbit) */
    data[4] = 0;		/* port 1 parity (none) */
    data[5] = (unsigned short)(round(log((double)speed / 300) / M_LN2) + 1);	/* port 1 speed */
    data[14] = zodiac_checksum(data, 14);

    (void)zodiac_spew(session, 1330, data, 15);
    return true;		/* it would be nice to error-check this */
}
#endif /* RECONFIGURE_ENABLE */

#ifdef NTPSHM_ENABLE
static double zodiac_ntp_offset(struct gps_device_t *session UNUSED)
{
    /* Removing/changing the magic number below is likely to disturb
     * the handling of the 1pps signal from the gps device. The regression
     * tests and simple gps applications do not detect this. A live test
     * with the 1pps signal active is required. */
    return 1.1;
}
#endif /* NTPSHM_ENABLE */

/* this is everything we export */
/* *INDENT-OFF* */
const struct gps_type_t zodiac_binary =
{
    .type_name      = "Zodiac binary",	/* full name of type */
    .packet_type    = ZODIAC_PACKET,	/* associated lexer packet type */
    .flags	    = DRIVER_NOFLAGS,	/* no flags set */
    .trigger	    = NULL,		/* no trigger */
    .channels       = 12,		/* consumer-grade GPS */
    .probe_detect   = NULL,		/* no probe */
    .get_packet     = generic_get,	/* use the generic packet getter */
    .parse_packet   = zodiac_analyze,	/* parse message packets */
    .rtcm_writer    = zodiac_send_rtcm,	/* send DGPS correction */
    .event_hook     = NULL,		/* no configuration */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher = zodiac_speed_switch,/* we can change baud rate */
    .mode_switcher  = NULL,		/* no mode switcher */
    .rate_switcher  = NULL,		/* no sample-rate switcher */
    .min_cycle      = 1,		/* not relevant, no rate switch */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send   = zodiac_control_send,	/* for gpsctl and friends */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = zodiac_ntp_offset,	/* compute NTO fudge factor */
#endif /* NTPSHM_ENABLE */
};
/* *INDENT-ON* */

#endif /* ZODIAC_ENABLE */
