/*
 * This is the gpsd driver for SiRF GPSes operating in binary mode.
 * It also handles uBlox, a SiRF derivative.
 *
 * The advantages: Reports climb/sink rate (raw-mode clients won't see this).
 * Also, we can flag DGPS satellites used in the skyview when SBAS is in use.
 * The disadvantages: Doesn't return PDOP or VDOP, just HDOP.
 *
 * Chris Kuethe, our SiRF expert, tells us:
 *
 * "I don't see any indication in any of my material that PDOP, GDOP
 * or VDOP are output. There are quantities called Estimated
 * {Horizontal Position, Vertical Position, Time, Horizonal Velocity}
 * Error, but those are apparently only valid when SiRFDRive is
 * active."
 *
 * "(SiRFdrive is their Dead Reckoning augmented firmware. It
 * allows you to feed odometer ticks, gyro and possibly
 * accelerometer inputs to the chip to allow it to continue
 * to navigate in the absence of satellite information, and
 * to improve fixes when you do have satellites.)"
 *
 * "[When we need RINEX data, we can get it from] SiRF Message #5.
 *  If it's no longer implemented on your receiver, messages
 * 7, 28, 29 and 30 will give you the same information."
 *
 * This file is Copyright (c) 2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <ctype.h>
#ifndef S_SPLINT_S
#include <unistd.h>
#endif /* S_SPLINT_S */

#include "gpsd.h"
#include "bits.h"
#if defined(SIRF_ENABLE) && defined(BINARY_ENABLE)

#define HI(n)		((n) >> 8)
#define LO(n)		((n) & 0xff)

/*@ +charint @*/
/* Poll Software Version MID 132 */
static unsigned char versionprobe[] = {
    0xa0, 0xa2, 0x00, 0x02,
    0x84,		/* MID 132 */
    0x00,		/* unused */
    0x00, 0x00, 0xb0, 0xb3
};

/* Poll Navigation Parameters MID 152
 * query for MID 19 */
static unsigned char navparams[] = {
    0xa0, 0xa2, 0x00, 0x02,
    0x98,		/* MID 152 */
    0x00,
    0x00, 0x00, 0xb0, 0xb3
};

#ifdef RECONFIGURE_ENABLE
/* DGPS Source MID 133 */
static unsigned char dgpscontrol[] = {
    0xa0, 0xa2, 0x00, 0x07,
    0x85,		/* MID 133 */
    0x01,		/* use SBAS */
    0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0xb0, 0xb3
};

/* Set SBAS Parameters MID 170 */
static unsigned char sbasparams[] = {
    0xa0, 0xa2, 0x00, 0x06,
    0xaa,		/* MID 170 */
    0x00,		/* SBAS PRN */
    0x01,		/* SBAS Mode */
    0x00,		/* Auto PRN */
    0x00, 0x00,
    0x00, 0x00, 0xb0, 0xb3
};

/* Set Message Rate MID 166 */
static unsigned char requestecef[] = {
    0xa0, 0xa2, 0x00, 0x08,
    0xa6,		/* MID 166 */
    0x00,		/* enable 1 */
    0x02,		/* MID 2 */
    0x01,		/* once per Sec */
    0x00, 0x00,		/* unused */
    0x00, 0x00,		/* unused */
    0x00, 0x00, 0xb0, 0xb3
};

/* Set Message Rate MID 166 */
static unsigned char requesttracker[] = {
    0xa0, 0xa2, 0x00, 0x08,
    0xa6,		/* MID 166 */
    0x00,		/* enable 1 */
    0x04,		/* MID 4 */
    0x03,		/* every 3 sec */
    0x00, 0x00,		/* unused */
    0x00, 0x00,		/* unused */
    0x00, 0x00, 0xb0, 0xb3
};

/* disable MID XX */
static unsigned char unsetmidXX[] = {
    0xa0, 0xa2, 0x00, 0x08,
    0xa6,		/* MID 166 */
    0x00,		/* enable 1 */
    0x00,		/* MID 0xXX */
    0x00,		/* never */
    0x00, 0x00,		/* unused */
    0x00, 0x00,		/* unused */
    0x00, 0x00, 0xb0, 0xb3
};

/* message to enable:
 *   MID 7 Clock Status
 *   MID 8 50Bps subframe data
 *   MID 17 Differential  Corrections
 *   MID 28 Nav Lib Measurement Data
 *   MID 29 Nav Lib DGPS Data
 *   MID 30 Nav Lib SV State Data
 *   MID 31 Nav Lib Initialization data
 * at 1Hz rate */
static unsigned char enablesubframe[] = {
    0xa0, 0xa2, 0x00, 0x19,
    0x80,			/* MID 128 initialize Data Source */
    0x00, 0x00, 0x00, 0x00,	/* EXEF X */
    0x00, 0x00, 0x00, 0x00,	/* ECEF Y */
    0x00, 0x00, 0x00, 0x00,	/* ECEF Z */
    0x00, 0x00, 0x00, 0x00,	/* clock drift */
    0x00, 0x00, 0x00, 0x00,	/* time of week */
    0x00, 0x00,			/* week number */
    0x0C,			/* Chans 1-12 */
    /* change the next 0x10 to 0x08
     * for factory reset */
    0x10,
    0x00, 0x00, 0xb0, 0xb3
};

/* disable subframe data */
static unsigned char disablesubframe[] = {
    0xa0, 0xa2, 0x00, 0x19,
    0x80,			/* MID 128 initialize Data Source */
    0x00, 0x00, 0x00, 0x00,	/* EXEF X */
    0x00, 0x00, 0x00, 0x00,	/* ECEF Y */
    0x00, 0x00, 0x00, 0x00,	/* ECEF Z */
    0x00, 0x00, 0x00, 0x00,	/* clock drift */
    0x00, 0x00, 0x00, 0x00,	/* time of week */
    0x00, 0x00,			/* week number */
    0x0C,			/* Chans 1-12 */

    0x00,
    0x00, 0x00, 0xb0, 0xb3
};

/* mode control MID */
static unsigned char modecontrol[] = {
    0xa0, 0xa2, 0x00, 0x0e,
    0x88,			/* MID 136 Mode Control */
    0x00, 0x00,			/* pad bytes */
    0x00,			/* degraded mode off */
    0x00, 0x00,			/* pad bytes */
    0x00, 0x00,			/* altitude */
    0x00,			/* altitude hold auto */
    0x00,			/* use last computed alt */
    0x00,			/* reserved */
    0x00,			/* disable degraded mode */
    0x00,			/* disable dead reckoning */
    0x01,			/* enable track smoothing */
    0x00, 0x00, 0xb0, 0xb3
};

/* enable 1 PPS Time MID 52 *
 * using Set Message Rate MID 166 */
static unsigned char enablemid52[] = {
    0xa0, 0xa2, 0x00, 0x08,
    0xa6,			/* MID 166 */
    0x00,			/* enable/disable one message */
    0x34,			/* MID 52 */
    0x01,			/* sent once per second */
    0x00, 0x00, 0x00, 0x00,	/* unused, set to zero */
    0x00, 0xdb, 0xb0, 0xb3
};
#endif /* RECONFIGURE_ENABLE */
/*@ -charint @*/


static gps_mask_t sirf_msg_debug(unsigned char *, size_t);
static gps_mask_t sirf_msg_errors(unsigned char *, size_t);

static gps_mask_t sirf_msg_navdata(struct gps_device_t *, unsigned char *,
				   size_t);
static gps_mask_t sirf_msg_navsol(struct gps_device_t *, unsigned char *,
				  size_t);
static gps_mask_t sirf_msg_nlmd(struct gps_device_t *, unsigned char *,
				size_t);
static gps_mask_t sirf_msg_ppstime(struct gps_device_t *, unsigned char *,
				   size_t);
static gps_mask_t sirf_msg_svinfo(struct gps_device_t *, unsigned char *,
				  size_t);
static gps_mask_t sirf_msg_swversion(struct gps_device_t *, unsigned char *,
				     size_t);
static gps_mask_t sirf_msg_sysparam(struct gps_device_t *, unsigned char *,
				    size_t);
static gps_mask_t sirf_msg_dgpsstatus(struct gps_device_t *, unsigned char *,
				      size_t);
static gps_mask_t sirf_msg_ublox(struct gps_device_t *, unsigned char *,
				 size_t);


static bool sirf_write(struct gps_device_t *session, unsigned char *msg)
{
    unsigned int crc;
    size_t i, len;
    bool ok;

    len = (size_t) ((msg[2] << 8) | msg[3]);

    /* calculate CRC */
    crc = 0;
    for (i = 0; i < len; i++)
	crc += (int)msg[4 + i];
    crc &= 0x7fff;

    /* enter CRC after payload */
    msg[len + 4] = (unsigned char)((crc & 0xff00) >> 8);
    msg[len + 5] = (unsigned char)(crc & 0x00ff);

    gpsd_report(LOG_IO, "SiRF: Writing control type %02x:\n", msg[4]);
    ok = (gpsd_write(session, (const char *)msg, len+8) == (ssize_t) (len+8));
    return (ok);
}

#ifdef CONTROLSEND_ENABLE
static ssize_t sirf_control_send(struct gps_device_t *session, char *msg,
				 size_t len)
{
    /*@ +charint +matchanyintegral -initallelements -mayaliasunique @*/
    session->msgbuf[0] = (char)0xa0;
    session->msgbuf[1] = (char)0xa2;
    session->msgbuf[2] = (len >> 8) & 0xff;
    session->msgbuf[3] = len & 0xff;
    memcpy(session->msgbuf + 4, msg, len);
    session->msgbuf[len + 6] = (char)0xb0;
    session->msgbuf[len + 7] = (char)0xb3;
    session->msgbuflen = len + 8;

    /* *INDENT-OFF* */
    return sirf_write(session,
	      (unsigned char *)session->msgbuf) ? (int)session->msgbuflen : -1;
    /* *INDENT-ON* */
    /*@ -charint -matchanyintegral +initallelements +mayaliasunique @*/
}
#endif /* CONTROLSEND_ENABLE */

#ifdef RECONFIGURE_ENABLE
static bool sirfbin_speed(struct gps_device_t *session, speed_t speed, char parity, int stopbits)
/* change speed in binary mode */
{
    /*@ +charint @*/
    static unsigned char msg[] = {
	0xa0, 0xa2, 0x00, 0x09,
	0x86,			/* byte 4:
				 * Set Binary Serial Port
				 * MID 134 */
	0x00, 0x00, 0x12, 0xc0,	/* bytes 5-8: 4800 bps */
	0x08,			/* byte  9: 8 data bits */
	0x01,			/* byte 10: 1 stop bit */
	0x00,			/* byte 11: no parity */
	0x00,			/* byte 12: reserved pad */
	0x00, 0x00, 0xb0, 0xb3
    };
    /*@ -charint @*/
    gpsd_report(LOG_PROG, "SiRF: sirf_speed(%d,%c,%d)\n",
		speed, parity, stopbits);
    if (9600 >= speed) {
	gpsd_report(LOG_WARN, "SiRF may lag at 9600bps or less.\n");
    }

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
    msg[5] = (unsigned char)((speed >> 24) & 0xff);
    msg[6] = (unsigned char)((speed >> 16) & 0xff);
    msg[7] = (unsigned char)((speed >> 8) & 0xff);
    msg[8] = (unsigned char)(speed & 0xff);
    msg[10] = (unsigned char)stopbits;
    msg[11] = (unsigned char)parity;
    return (sirf_write(session, msg));
}

static bool sirf_to_nmea(struct gps_device_t *session, speed_t speed)
/* switch from binary to NMEA at specified baud */
{
    /*@ +charint @*/
    static unsigned char msg[] = { 0xa0, 0xa2, 0x00, 0x18,
	0x81, 0x02,
	0x01, 0x01,		/* GGA */
	0x00, 0x00,		/* suppress GLL */
	0x01, 0x01,		/* GSA */
	0x05, 0x01,		/* GSV */
	0x01, 0x01,		/* RMC */
	0x00, 0x00,		/* suppress VTG */
	0x00, 0x01,		/* suppress MSS */
	0x00, 0x01,		/* suppress EPE */
	0x00, 0x01,		/* suppress EPE */
	0x00, 0x01,		/* suppress ZDA */
	0x00, 0x00,		/* unused */
	0x12, 0xc0,		/* 4800 bps */
	0xb0, 0xb3
    };
    /*@ -charint @*/

    if (speed >= 0xffff) {
	gpsd_report(LOG_ERROR, "SiRF: can't switch from SiRF to NMEA because current speed %u is big.",
		     speed);
	return false;
    }

    msg[26] = (unsigned char)HI(speed);
    msg[27] = (unsigned char)LO(speed);
    return (sirf_write(session, msg));
}

static void sirfbin_mode(struct gps_device_t *session, int mode)
{
    if (mode == MODE_NMEA) {
	(void)sirf_to_nmea(session, session->gpsdata.dev.baudrate);
    } else if (mode == MODE_BINARY) {
	char parity = '0';
	switch (session->gpsdata.dev.parity) {
	default:
	case 'N':
	    parity = '0';
	    break;
	case 'O':
	    parity = '1';
	    break;
	case 'E':
	    parity = '2';
	    break;

	}
	// gpsd only supports 8[NO]1 or 7[EO]2
	// thus the strange use of stopbits
	(void)nmea_send(session,
			"$PSRF100,0,%d,%d,%d,%c",
			session->gpsdata.dev.baudrate,
			9 - session->gpsdata.dev.stopbits,
			session->gpsdata.dev.stopbits, parity);
	(void)usleep(333);	/* guessed settling time */
	session->gpsdata.dev.driver_mode = MODE_BINARY;
    }
    session->back_to_nmea = false;
}
#endif /* RECONFIGURE_ENABLE */

static ssize_t sirf_get(struct gps_device_t *session)
{
    ssize_t len = generic_get(session);

    if (session->packet.type == SIRF_PACKET) {
	session->gpsdata.dev.driver_mode = MODE_BINARY;
    } else if (session->packet.type == NMEA_PACKET) {
	session->gpsdata.dev.driver_mode = MODE_NMEA;
	(void)gpsd_switch_driver(session, "Generic NMEA");
    } else {
	/* should never happen */
	gpsd_report(LOG_PROG, "SiRF: Unexpected packet type %d\n",
		    session->packet.type);
	(void)gpsd_switch_driver(session, "Generic NMEA");
    }

    return len;
}

static gps_mask_t sirf_msg_debug(unsigned char *buf, size_t len)
{
    char msgbuf[MAX_PACKET_LENGTH * 3 + 2];
    int i;

    bzero(msgbuf, (int)sizeof(msgbuf));

    /*@ +charint @*/
    if (0xe1 == buf[0]) {	/* Development statistics messages */
	for (i = 2; i < (int)len; i++)
	    (void)snprintf(msgbuf + strlen(msgbuf),
			   sizeof(msgbuf) - strlen(msgbuf),
			   "%c", buf[i] ^ 0xff);
	gpsd_report(LOG_PROG, "SiRF: DEV 0xe1: %s\n", msgbuf);
    } else if (0xff == (unsigned char)buf[0]) {	/* Debug messages */
	for (i = 1; i < (int)len; i++)
	    if (isprint(buf[i]))
		(void)snprintf(msgbuf + strlen(msgbuf),
			       sizeof(msgbuf) - strlen(msgbuf), "%c", buf[i]);
	    else
		(void)snprintf(msgbuf + strlen(msgbuf),
			       sizeof(msgbuf) - strlen(msgbuf),
			       "\\x%02x", (unsigned int)buf[i]);
	gpsd_report(LOG_PROG, "SiRF: DBG 0xff: %s\n", msgbuf);
    }
    /*@ -charint @*/
    return 0;
}

static gps_mask_t sirf_msg_errors(unsigned char *buf, size_t len UNUSED)
{
    switch (getbeu16(buf, 1)) {
    case 2:
	gpsd_report(LOG_PROG,
		    "SiRF: EID 0x0a type 2: Subframe %u error on PRN %u\n",
		    getbeu32(buf, 9), getbeu32(buf, 5));
	break;

    case 4107:
	gpsd_report(LOG_PROG,
		    "SiRF: EID 0x0a type 4107: neither KF nor LSQ fix.\n");
	break;

    default:
	gpsd_report(LOG_PROG, "SiRF: EID 0x0a: Error MID %d\n",
		    getbeu16(buf, 1));
	break;
    }
    return 0;
}

/* Navigation Library Measurement Data MID 28 */
static gps_mask_t sirf_msg_nlmd(struct gps_device_t *session UNUSED,
				unsigned char *buf UNUSED, size_t len)
{

    double gps_tow = 0.0;

    if (len != 56)
	return 0;

    /* oh barf, SiRF claims to be IEEE754 but supports two
     * different double orders, neither IEEE754 */
    /* FIXME - decode the time, since this is the first MID with a
     * good time stamp this will be good for ntpshm time */
    gpsd_report(LOG_PROG, "SiRF: MID 0x1c, NLMD, gps_tow: %f\n",
		(double)gps_tow);

    return 0;
}

static gps_mask_t sirf_msg_swversion(struct gps_device_t *session,
				     unsigned char *buf, size_t len)
{
    double fv;
    unsigned char *cp;

    if (len < 20)
	return 0;

    (void)strlcpy(session->subtype, (char *)buf + 1,
		  sizeof(session->subtype));
    for (cp = buf+1; *cp!=(unsigned char)'\0' && isdigit(*cp)==0; cp++)
	continue;
    fv = safe_atof((const char *)cp);
    if (fv < 231) {
	session->driver.sirf.driverstate |= SIRF_LT_231;
#ifdef RECONFIGURE_ENABLE
	if (fv > 200)
	    sirfbin_mode(session, 0);
#endif /* RECONFIGURE_ENABLE */
    } else if (fv < 232) {
	session->driver.sirf.driverstate |= SIRF_EQ_231;
    } else {
	session->driver.sirf.driverstate |= SIRF_GE_232;
    }
    if (strstr((char *)(buf + 1), "ES"))
	gpsd_report(LOG_INF, "SiRF: Firmware has XTrac capability\n");
    gpsd_report(LOG_PROG, "SiRF: fv: %0.2f, Driver state flags are: %0x\n",
		fv, session->driver.sirf.driverstate);
#ifdef NTPSHM_ENABLE
    session->driver.sirf.time_seen = 0;
#endif /* NTPSHM_ENABLE */
    gpsd_report(LOG_DATA, "SiRF: FV MID 0x06: subtype='%s' mask={DEVICEID}\n",
		session->subtype);
    return DEVICEID_SET;
}

static gps_mask_t sirf_msg_navdata(struct gps_device_t *session,
				   unsigned char *buf, size_t len)
{
    unsigned int i, chan, svid;
    uint32_t words[10];

    if (len != 43)
	return 0;

    chan = (unsigned int)getub(buf, 1);
    svid = (unsigned int)getub(buf, 2);

    for (i = 0; i < 10; i++) {
	words[i] = (uint32_t)getbeu32(buf, 4 * i + 3);
    }

    gpsd_report(LOG_IO, "SiRF: NavData chan %u svid %u\n",chan,svid);

#ifdef RECONFIGURE_ENABLE
    if (!session->context->readonly && session->gpsdata.dev.baudrate < 38400) {
        /* some USB are also too slow, no way to tell which ones */
	gpsd_report(LOG_WARN,
		"WARNING: SiRF: link too slow, disabling subframes.\n");
	(void)sirf_write(session, disablesubframe);
    }
#endif /* RECONFIGURE_ENABLE */

    return gpsd_interpret_subframe_raw(session, svid, words);
}

#define SIRF_CHANNELS	12	/* max channels allowed in SiRF format */

static gps_mask_t sirf_msg_svinfo(struct gps_device_t *session,
				  unsigned char *buf, size_t len)
{
    int st, i, j, cn;

    if (len != 188)
	return 0;

    session->gpsdata.skyview_time = gpsd_gpstime_resolve(session,
	(unsigned short)getbes16(buf, 1),
	(unsigned int)getbeu32(buf, 3) * 1e-2);

    gpsd_zero_satellites(&session->gpsdata);
    for (i = st = 0; i < SIRF_CHANNELS; i++) {
	int off = 8 + 15 * i;
	bool good;
	session->gpsdata.PRN[st] = (int)getub(buf, off);
	session->gpsdata.azimuth[st] =
	    (int)(((unsigned)getub(buf, off + 1) * 3) / 2.0);
	session->gpsdata.elevation[st] =
	    (int)((unsigned)getub(buf, off + 2) / 2.0);
	cn = 0;
	for (j = 0; j < 10; j++)
	    cn += (int)getub(buf, off + 5 + j);

	session->gpsdata.ss[st] = (float)(cn / 10.0);
	good = session->gpsdata.PRN[st] != 0 &&
	    session->gpsdata.azimuth[st] != 0 &&
	    session->gpsdata.elevation[st] != 0;
#ifdef __UNUSED__
	gpsd_report(LOG_PROG,
		    "SiRF: PRN=%2d El=%3.2f Az=%3.2f ss=%3d stat=%04x %c\n",
		    getub(buf, off),
		    getub(buf, off + 2) / 2.0,
		    (getub(buf, off + 1) * 3) / 2.0,
		    cn / 10, getbeu16(buf, off + 3), good ? '*' : ' ');
#endif /* UNUSED */
	if (good != 0)
	    st += 1;
    }
    session->gpsdata.satellites_visible = st;
    /* mark SBAS sats in use if SBAS was in use as of the last MID 27 */
    for (i = 0; i < st; i++) {
	int prn = session->gpsdata.PRN[i];
	if (SBAS_PRN(prn) \
		&& session->gpsdata.status == STATUS_DGPS_FIX \
		&& session->driver.sirf.dgps_source == SIRF_DGPS_SOURCE_SBAS)
	    session->gpsdata.used[session->gpsdata.satellites_used++] = prn;
    }
#ifdef NTPSHM_ENABLE
    if (st < 3) {
	gpsd_report(LOG_PROG,
		    "SiRF: NTPD not enough satellites seen: %d\n", st);
    } else {
	/* SiRF says if 3 sats in view the time is good */
	gpsd_report(LOG_PROG,
		    "SiRF: NTPD valid time MID 0x04, seen=0x%02x, time:%.2lf, leap:%d\n",
		    session->driver.sirf.time_seen,
		    session->gpsdata.skyview_time,
		    session->context->leap_seconds);
    }
#endif /* NTPSHM_ENABLE */
    gpsd_report(LOG_DATA, "SiRF: MTD 0x04: visible=%d mask={SATELLITE}\n",
		session->gpsdata.satellites_visible);
    return SATELLITE_SET;
}

#ifdef NTPSHM_ENABLE
static double sirf_ntp_offset(struct gps_device_t *session)
/* return NTP time-offset fudge factor for this device */
{
    double retval = NAN;

    /* we need to have seen UTC time with a valid leap-year offset */
    if ((session->driver.sirf.time_seen & TIME_SEEN_UTC_2) != 0) {
	retval = NAN;
    }

    /* the PPS time message */
    else if (strcmp(session->gpsdata.tag, "MID52") == 0) {
	retval = 0.3;
    }

    /* uBlox EMND message */
    else if (strcmp(session->gpsdata.tag, "MID98") == 0) {
	retval = 0.570;
    }
#ifdef __UNUSED__
    /* geodetic-data message */
    else if (strcmp(session->gpsdata.tag, "MID41") == 0) {
	retval = 0.570;
    }
#endif /* __UNUSED__ */

    /* the Navigation Solution message */
    else if (strcmp(session->gpsdata.tag, "MID2") == 0) {
	if (session->sourcetype == source_usb) {
	    retval = 0.640;	/* USB, expect +/- 50mS jitter */
	} else {
	    switch (session->gpsdata.dev.baudrate) {
	    default:
		retval = 0.704;	/* WAG */
		break;
	    case 4800:
		retval = 0.704;	/* fudge valid at 4800bps */
		break;
	    case 9600:
		retval = 0.688;
		break;
	    case 19200:
		retval = 0.484;
		break;
	    case 38400:
		retval = 0.845;	/*  0.388; ?? */
		break;
	    }
	}
    }

    return retval;
}
#endif /* NTPSHM_ENABLE */

static gps_mask_t sirf_msg_navsol(struct gps_device_t *session,
				  unsigned char *buf, size_t len)
{
    int i;
    unsigned short navtype;
    gps_mask_t mask = 0;

    if (len != 41)
	return 0;

    session->gpsdata.satellites_used = (int)getub(buf, 28);
    memset(session->gpsdata.used, 0, sizeof(session->gpsdata.used));
    for (i = 0; i < SIRF_CHANNELS; i++)
	session->gpsdata.used[i] = (int)getub(buf, 29 + i);
    /* position/velocity is bytes 1-18 */
    ecef_to_wgs84fix(&session->newdata, &session->gpsdata.separation,
		     (double)getbes32(buf, 1) * 1.0,
		     (double)getbes32(buf, 5) * 1.0,
		     (double)getbes32(buf, 9) * 1.0,
		     (double)getbes16(buf, 13) / 8.0,
		     (double)getbes16(buf, 15) / 8.0,
		     (double)getbes16(buf, 17) / 8.0);
    /* fix status is byte 19 */
    navtype = (unsigned short)getub(buf, 19);
    session->gpsdata.status = STATUS_NO_FIX;
    session->newdata.mode = MODE_NO_FIX;
    if ((navtype & 0x80) != 0)
	session->gpsdata.status = STATUS_DGPS_FIX;
    else if ((navtype & 0x07) > 0 && (navtype & 0x07) < 7)
	session->gpsdata.status = STATUS_FIX;
    if ((navtype & 0x07) == 4 || (navtype & 0x07) == 6)
	session->newdata.mode = MODE_3D;
    else if (session->gpsdata.status != 0)
	session->newdata.mode = MODE_2D;
    if (session->newdata.mode == MODE_3D)
	mask |= ALTITUDE_SET | CLIMB_SET;
    gpsd_report(LOG_PROG,
		"SiRF: MND 0x02: Navtype = 0x%0x, Status = %d, mode = %d\n",
		navtype, session->gpsdata.status, session->newdata.mode);
    /* byte 20 is HDOP, see below */
    /* byte 21 is "mode 2", not clear how to interpret that */
    session->newdata.time = gpsd_gpstime_resolve(session,
	(unsigned short)getbes16(buf, 22), (double)getbeu32(buf, 24) * 1e-2);
#ifdef NTPSHM_ENABLE
    if (session->newdata.mode <= MODE_NO_FIX) {
	gpsd_report(LOG_PROG, "SiRF: NTPD no fix, mode: %d\n",
		    session->newdata.mode);
    } else {
	gpsd_report(LOG_PROG,
		    "SiRF: NTPD valid time MID 0x02, seen=0x%02x, time;%.2lf, leap:%d\n",
		    session->driver.sirf.time_seen,
		    session->newdata.time, session->context->leap_seconds);
    }
#endif /* NTPSHM_ENABLE */
    /* fix quality data */
    session->gpsdata.dop.hdop = (double)getub(buf, 20) / 5.0;
    mask |=
	TIME_SET | LATLON_SET | ALTITUDE_SET | TRACK_SET |
	SPEED_SET | STATUS_SET | MODE_SET | DOP_SET | USED_IS;
    if ( 3 <= session->gpsdata.satellites_visible ) {
	mask |= PPSTIME_IS;
    }
    gpsd_report(LOG_DATA,
		"SiRF: MND 0x02: time=%.2f lat=%.2f lon=%.2f alt=%.2f track=%.2f speed=%.2f mode=%d status=%d hdop=%.2f used=%d\n",
		session->newdata.time, session->newdata.latitude,
		session->newdata.longitude, session->newdata.altitude,
		session->newdata.track, session->newdata.speed,
		session->newdata.mode, session->gpsdata.status,
		session->gpsdata.dop.hdop, session->gpsdata.satellites_used);
    return mask;
}

#ifdef __UNUSED__
/***************************************************************************
 We've stopped interpreting GND (0x29) for the following reasons:

1) Versions of SiRF firmware still in wide circulation (and likely to be
   so for a while) don't report a valid time field, leading to annoying
   twice-per-second jitter in client displays.

2) What we wanted out of this that MND didn't give us was horizontal and
   vertical error estimates. But we have to do our own error estimation by
   computing DOPs from the skyview covariance matrix anyway, because we
   want separate epx and epy errors a la NMEA 3.0.

3) The fix-merge logic in gpsd.c is (unavoidably) NMEA-centric and
   thinks multiple sentences in one cycle should be treated as
   incremental updates.  This leads to various silly results when (as
   in GND) a subsequent sentence is (a) intended to be a complete fix
   in itself, and (b) frequently broken.

4) Ignoring this dodgy sentence allows us to go to a nice clean single
   fix update per cycle.

Code left in place in case we need to reverse this decision.

***************************************************************************/
static gps_mask_t sirf_msg_geodetic(struct gps_device_t *session,
				    unsigned char *buf, size_t len)
{
    unsigned short navtype;
    gps_mask_t mask = 0;
    double eph;

    if (len != 91)
	return 0;

    session->gpsdata.sentence_length = 91;
    (void)strlcpy(session->gpsdata.tag, "GND", MAXTAGLEN + 1);

    navtype = (unsigned short)getbeu16(buf, 3);
    session->gpsdata.status = STATUS_NO_FIX;
    session->newdata.mode = MODE_NO_FIX;
    if (navtype & 0x80)
	session->gpsdata.status = STATUS_DGPS_FIX;
    else if ((navtype & 0x07) > 0 && (navtype & 0x07) < 7)
	session->gpsdata.status = STATUS_FIX;
    session->newdata.mode = MODE_NO_FIX;
    if ((navtype & 0x07) == 4 || (navtype & 0x07) == 6)
	session->newdata.mode = MODE_3D;
    else if (session->gpsdata.status)
	session->newdata.mode = MODE_2D;
    gpsd_report(LOG_PROG,
		"SiRF: GND 0x29: Navtype = 0x%0x, Status = %d, mode = %d\n",
		navtype, session->gpsdata.status, session->newdata.mode);
    mask |= STATUS_SET | MODE_SET;

    session->newdata.latitude = getbes32(buf, 23) * 1e-7;
    session->newdata.longitude = getbes32(buf, 27) * 1e-7;
    if (session->newdata.latitude != 0 && session->newdata.latitude != 0)
	mask |= LATLON_SET;

    if ((eph = getbes32(buf, 50) * 1e-2) > 0) {
	session->newdata.epx = session->newdata.epy = eph / sqrt(2);
	mask |= HERR_SET;
    }
    if ((session->newdata.epv = getbes32(buf, 54) * 1e-2) > 0)
	mask |= VERR_SET;
    if ((session->newdata.eps = getbes16(buf, 62) * 1e-2) > 0)
	mask |= SPEEDERR_SET;

    /* HDOP should be available at byte 89, but in 231 it's zero. */
    //session->gpsdata.dop.hdop = (unsigned int)getub(buf, 89) * 0.2;

    if ((session->newdata.mode > MODE_NO_FIX)
	&& (session->driver.sirf.driverstate & SIRF_GE_232)) {
	struct tm unpacked_date;
	double subseconds;
	/*
	 * Early versions of the SiRF protocol manual don't document
	 * this sentence at all.  Some that do incorrectly
	 * describe UTC Day, Hour, and Minute as 2-byte quantities,
	 * not 1-byte. Chris Kuethe, our SiRF expert, tells us:
	 *
	 * "The Geodetic Navigation packet (0x29) was not fully
	 * implemented in firmware prior to version 2.3.2. So for
	 * anyone running 231.000.000 or earlier (including ES,
	 * SiRFDRive, XTrac trains) you won't get UTC time. I don't
	 * know what's broken in firmwares before 2.3.1..."
	 *
	 * To work around the incomplete implementation of this
	 * packet in 231, we used to assume that only the altitude field
	 * from this packet is valid.  But even this doesn't necessarily
	 * seem to be the case.  Instead, we do our own computation
	 * of geoid separation now.
	 *
	 * UTC is left all zeros in 231 and older firmware versions,
	 * and misdocumented in version 1.4 of the Protocol Reference.
	 *            Documented:        Real:
	 * UTC year       2               2
	 * UTC month      1               1
	 * UTC day        2               1
	 * UTC hour       2               1
	 * UTC minute     2               1
	 * UTC second     2               2
	 *                11              8
	 *
	 * Documentation of this field was corrected in the 1.6 version
	 * of the protocol manual.
	 */
	unpacked_date.tm_year = (int)getbeu16(buf, 11) - 1900;
	unpacked_date.tm_mon = (int)getub(buf, 13) - 1;
	unpacked_date.tm_mday = (int)getub(buf, 14);
	unpacked_date.tm_hour = (int)getub(buf, 15);
	unpacked_date.tm_min = (int)getub(buf, 16);
	unpacked_date.tm_sec = 0;
	subseconds = getbeu16(buf, 17) * 1e-3;
	/*@ -compdef -unrecog */
	session->newdata.time = (timestamp_t)timegm(&unpacked_date) + subseconds;
	/*@ +compdef +unrecog */
	gpsd_report(LOG_PROG, "SiRF: GND 0x29 UTC: %lf\n",
		    session->newdata.time);
#ifdef NTPSHM_ENABLE
	if (session->newdata.mode <= MODE_NO_FIX) {
	    gpsd_report(LOG_PROG, "SiRF: NTPD no fix, mode: $d\n",
			session->newdata.mode);
	} else if (0 == unpacked_date.tm_year) {
	    gpsd_report(LOG_PROG, "SiRF: NTPD no year\n",
			session->newdata.mode);
	} else {
	    gpsd_report(LOG_PROG,
			"SiRF: NTPD valid time MID 0x29, seen=0x%02x\n",
			session->driver.sirf.time_seen);
	}
	if ( 3 <= session->gpsdata.satellites_visible ) {
	    mask |= PPSTIME_IS;
	}

#endif /* NTPSHM_ENABLE */
	/* skip 4 bytes of satellite map */
	session->newdata.altitude = getbes32(buf, 35) * 1e-2;
	/* skip 1 byte of map datum */
	session->newdata.speed = getbeu16(buf, 40) * 1e-2;
	session->newdata.track = getbeu16(buf, 42) * 1e-2;
	/* skip 2 bytes of magnetic variation */
	session->newdata.climb = getbes16(buf, 46) * 1e-2;
	mask |= TIME_SET | SPEED_SET | TRACK_SET;
	if (session->newdata.mode == MODE_3D)
	    mask |= ALTITUDE_SET | CLIMB_SET;
    }
    gpsd_report(LOG_DATA,
		"SiRF: GND 0x29: time=%.2f lat=%.2f lon=%.2f alt=%.2f track=%.2f speed=%.2f mode=%d status=%d\n",
		session->newdata.time,
		session->newdata.latitude,
		session->newdata.longitude,
		session->newdata.altitude,
		session->newdata.track,
		session->newdata.speed,
		session->newdata.mode,
		session->gpsdata.status);
    return mask;
}
#endif /* __UNUSED__ */

static gps_mask_t sirf_msg_sysparam(struct gps_device_t *session,
				    unsigned char *buf, size_t len)
{

    if (len != 65)
	return 0;

    /* save these to restore them in the revert method */
    session->driver.sirf.nav_parameters_seen = true;
    session->driver.sirf.altitude_hold_mode = (unsigned char)getub(buf, 5);
    session->driver.sirf.altitude_hold_source = (unsigned char)getub(buf, 6);
    session->driver.sirf.altitude_source_input = getbes16(buf, 7);
    session->driver.sirf.degraded_mode = (unsigned char)getub(buf, 9);
    session->driver.sirf.degraded_timeout = (unsigned char)getub(buf, 10);
    session->driver.sirf.dr_timeout = (unsigned char)getub(buf, 11);
    session->driver.sirf.track_smooth_mode = (unsigned char)getub(buf, 12);
#ifdef RECONFIGURE_ENABLE
    if (!session->context->readonly) {
	gpsd_report(LOG_PROG, "SiRF: Setting Navigation Parameters\n");
	(void)sirf_write(session, modecontrol);
    }
#endif /* RECONFIGURE_ENABLE */
    return 0;
}

static gps_mask_t sirf_msg_dgpsstatus(struct gps_device_t *session,
				 unsigned char *buf, size_t len UNUSED)
/* only documentented from prorocol version 1.7 (2005) onwards */
{
    session->driver.sirf.dgps_source = (unsigned int)getub(buf, 1);
    return 0;
}

static gps_mask_t sirf_msg_ublox(struct gps_device_t *session,
				 unsigned char *buf, size_t len UNUSED)
{
    gps_mask_t mask;
    unsigned short navtype;

    if (len != 39)
	return 0;

    /* this packet is only sent by uBlox firmware from version 1.32 */
    mask = LATLON_SET | ALTITUDE_SET | SPEED_SET | TRACK_SET | CLIMB_SET |
	STATUS_SET | MODE_SET | DOP_SET;
    session->newdata.latitude = (double)getbes32(buf, 1) * RAD_2_DEG * 1e-8;
    session->newdata.longitude = (double)getbes32(buf, 5) * RAD_2_DEG * 1e-8;
    session->gpsdata.separation =
	wgs84_separation(session->newdata.latitude,
			 session->newdata.longitude);
    session->newdata.altitude =
	(double)getbes32(buf, 9) * 1e-3 - session->gpsdata.separation;
    session->newdata.speed = (double)getbes32(buf, 13) * 1e-3;
    session->newdata.climb = (double)getbes32(buf, 17) * 1e-3;
    session->newdata.track = (double)getbes32(buf, 21) * RAD_2_DEG * 1e-8;

    navtype = (unsigned short)getub(buf, 25);
    session->gpsdata.status = STATUS_NO_FIX;
    session->newdata.mode = MODE_NO_FIX;
    if (navtype & 0x80)
	session->gpsdata.status = STATUS_DGPS_FIX;
    else if ((navtype & 0x07) > 0 && (navtype & 0x07) < 7)
	session->gpsdata.status = STATUS_FIX;
    if ((navtype & 0x07) == 4 || (navtype & 0x07) == 6)
	session->newdata.mode = MODE_3D;
    else if (session->gpsdata.status)
	session->newdata.mode = MODE_2D;
    gpsd_report(LOG_PROG,
		"SiRF: EMND 0x62: Navtype = 0x%0x, Status = %d, mode = %d\n",
		navtype, session->gpsdata.status, session->newdata.mode);

    if (navtype & 0x40) {	/* UTC corrected timestamp? */
	struct tm unpacked_date;
	double subseconds;
	mask |= TIME_SET;
	if ( 3 <= session->gpsdata.satellites_visible ) {
	    mask |= PPSTIME_IS;
	}
	unpacked_date.tm_year = (int)getbeu16(buf, 26) - 1900;
	unpacked_date.tm_mon = (int)getub(buf, 28) - 1;
	unpacked_date.tm_mday = (int)getub(buf, 29);
	unpacked_date.tm_hour = (int)getub(buf, 30);
	unpacked_date.tm_min = (int)getub(buf, 31);
	unpacked_date.tm_sec = 0;
	subseconds = ((unsigned short)getbeu16(buf, 32)) * 1e-3;
	/*@ -compdef */
	session->newdata.time = (timestamp_t)mkgmtime(&unpacked_date) + subseconds;
	/*@ +compdef */
#ifdef NTPSHM_ENABLE
	if (0 == (session->driver.sirf.time_seen & TIME_SEEN_UTC_2)) {
	    gpsd_report(LOG_RAW, "SiRF: NTPD just SEEN_UTC_2\n");
	}
	gpsd_report(LOG_PROG,
		    "SiRF: NTPD valid time MID 0x62, seen=0x%02x\n",
		    session->driver.sirf.time_seen);
	session->driver.sirf.time_seen |= TIME_SEEN_UTC_2;
#endif /* NTPSHM_ENABLE */
	session->context->valid |= LEAP_SECOND_VALID;
    }

    session->gpsdata.dop.gdop = (int)getub(buf, 34) / 5.0;
    session->gpsdata.dop.pdop = (int)getub(buf, 35) / 5.0;
    session->gpsdata.dop.hdop = (int)getub(buf, 36) / 5.0;
    session->gpsdata.dop.vdop = (int)getub(buf, 37) / 5.0;
    session->gpsdata.dop.tdop = (int)getub(buf, 38) / 5.0;
    session->driver.sirf.driverstate |= UBLOX;
    gpsd_report(LOG_DATA,
		"SiRF: EMD 0x62: time=%.2f lat=%.2f lon=%.2f alt=%.f speed=%.2f track=%.2f climb=%.2f mode=%d status=%d gdop=%.2f pdop=%.2f hdop=%.2f vdop=%.2f tdop=%.2f\n",
		session->newdata.time, session->newdata.latitude,
		session->newdata.longitude, session->newdata.altitude,
		session->newdata.speed, session->newdata.track,
		session->newdata.climb, session->newdata.mode,
		session->gpsdata.status, session->gpsdata.dop.gdop,
		session->gpsdata.dop.pdop, session->gpsdata.dop.hdop,
		session->gpsdata.dop.vdop, session->gpsdata.dop.tdop);
    return mask;
}

static gps_mask_t sirf_msg_ppstime(struct gps_device_t *session,
				   unsigned char *buf, size_t len)
{
    gps_mask_t mask = 0;

    if (len != 19)
	return 0;

    gpsd_report(LOG_PROG, "SiRF: PPS 0x34: Status = 0x%02x\n",
		getub(buf, 14));
    if (((int)getub(buf, 14) & 0x07) == 0x07) {	/* valid UTC time? */
	struct tm unpacked_date;
	unpacked_date.tm_hour = (int)getub(buf, 1);
	unpacked_date.tm_min = (int)getub(buf, 2);
	unpacked_date.tm_sec = (int)getub(buf, 3);
	unpacked_date.tm_mday = (int)getub(buf, 4);
	unpacked_date.tm_mon = (int)getub(buf, 5) - 1;
	unpacked_date.tm_year = (int)getbeu16(buf, 6) - 1900;
	/*@ -compdef */
	session->newdata.time = (timestamp_t)mkgmtime(&unpacked_date);
	/*@ +compdef */
	session->context->leap_seconds = (int)getbeu16(buf, 8);
	session->context->valid |= LEAP_SECOND_VALID;
#ifdef NTPSHM_ENABLE
	if (0 == (session->driver.sirf.time_seen & TIME_SEEN_UTC_2)) {
	    gpsd_report(LOG_RAW, "SiRF: NTPD just SEEN_UTC_2\n");
	}
	gpsd_report(LOG_PROG,
		    "SiRF: NTPD valid time MID 0x34, seen=0x%02x, leap=%d\n",
		    session->driver.sirf.time_seen,
		    session->context->leap_seconds);
	session->driver.sirf.time_seen |= TIME_SEEN_UTC_2;
#endif /* NTPSHM_ENABLE */
	mask |= TIME_SET;
	if ( 3 <= session->gpsdata.satellites_visible ) {
	    mask |= PPSTIME_IS;
	}
    }
    return mask;
}

gps_mask_t sirf_parse(struct gps_device_t * session, unsigned char *buf,
		      size_t len)
{

    if (len == 0)
	return 0;

    buf += 4;
    len -= 8;
    gpsd_report(LOG_RAW, "SiRF: Raw packet type 0x%02x\n", buf[0]);
    (void)snprintf(session->gpsdata.tag, sizeof(session->gpsdata.tag),
		   "MID%d", (int)buf[0]);

    /* could change if the set of messages we enable does */
    session->cycle_end_reliable = true;

    switch (buf[0]) {
    case 0x02:			/* Measure Navigation Data Out MID 2 */
	if ((session->driver.sirf.driverstate & UBLOX) == 0)
	    return sirf_msg_navsol(session, buf,
				   len) | (CLEAR_IS | REPORT_IS);
	else {
	    gpsd_report(LOG_PROG,
			"SiRF: MND 0x02 skipped, uBlox flag is on.\n");
	    return 0;
	}
    case 0x04:			/* Measured tracker data out MID 4 */
	return sirf_msg_svinfo(session, buf, len);

    case 0x05:			/* Raw Tracker Data Out MID 5 */
	gpsd_report(LOG_PROG, "SiRF: unused Raw Tracker Data 0x05\n");
	return 0;

    case 0x06:			/* Software Version String MID 6 */
	return sirf_msg_swversion(session, buf, len);

    case 0x07:			/* Clock Status Data MID 7 */
	gpsd_report(LOG_PROG, "SiRF: unused CLK 0x07\n");
	return 0;

    case 0x08:			/* subframe data MID 8 */
	/* extract leap-second from this */
	/*
	 * Chris Kuethe says:
	 * "Message 8 is generated as the data is received. It is not
	 * buffered on the chip. So when you enable message 8, you'll
	 * get one subframe every 6 seconds.  Of the data received, the
	 * almanac and ephemeris are buffered and stored, so you can
	 * query them at will. Alas, the time parameters are not
	 * stored, which is really lame, as the UTC-GPS correction
	 * changes 1 second every few years. Maybe."
	 */
	return sirf_msg_navdata(session, buf, len);

    case 0x09:			/* CPU Throughput MID 9 */
	gpsd_report(LOG_PROG,
		    "SiRF: THR 0x09: SegStatMax=%.3f, SegStatLat=%3.f, AveTrkTime=%.3f, Last MS=%u\n",
		    (float)getbeu16(buf, 1) / 186, (float)getbeu16(buf,
								 3) / 186,
		    (float)getbeu16(buf, 5) / 186, getbeu16(buf, 7));
	return 0;

    case 0x0a:			/* Error ID Data MID 10 */
	return sirf_msg_errors(buf, len);

    case 0x0b:			/* Command Acknowledgement MID 11 */
	gpsd_report(LOG_PROG, "SiRF: ACK 0x0b: %02x\n", getub(buf, 1));
	return 0;

    case 0x0c:			/* Command NAcknowledgement MID 12 */
	gpsd_report(LOG_PROG, "SiRF: NAK 0x0c: %02x\n", getub(buf, 1));
	return 0;

    case 0x0d:			/* Visible List MID 13 */
	gpsd_report(LOG_PROG, "SiRF: unused VIS 0x0d\n");
	return 0;

    case 0x0e:			/* Almanac Data MID 14 */
	gpsd_report(LOG_PROG, "SiRF: unused ALM  0x0e\n");
	return 0;

    case 0x0f:			/* Ephemeris Data MID 15 */
	gpsd_report(LOG_PROG, "SiRF: unused EPH  0x0f\n");
	return 0;

    case 0x11:			/* Differential Corrections MID 17 */
	gpsd_report(LOG_PROG, "SiRF: unused DIFF 0x11\n");
	return 0;

    case 0x12:			/* OK To Send MID 18 */
	gpsd_report(LOG_PROG, "SiRF: OTS 0x12: send indicator = %d\n",
		    getub(buf, 1));
	return 0;

    case 0x13:			/* Navigation Parameters MID 19 */
	return sirf_msg_sysparam(session, buf, len);

    case 0x1b:			/* DGPS status MID 27 */
	return sirf_msg_dgpsstatus(session, buf, len);

    case 0x1c:			/* Navigation Library Measurement Data MID 28 */
	gpsd_report(LOG_PROG, "SiRF: NLMD 0x1c\n");
	return sirf_msg_nlmd(session, buf, len);

    case 0x1d:			/* Navigation Library DGPS Data MID 29 */
	gpsd_report(LOG_PROG, "SiRF: unused NLDG 0x1d\n");
	return 0;

    case 0x1e:			/* Navigation Library SV State Data MID 30 */
	gpsd_report(LOG_PROG, "SiRF: unused NLSV 0x1e\n");
	return 0;

    case 0x1f:			/* Navigation Library Initialization Data MID 31 */
	gpsd_report(LOG_PROG, "SiRF: unused NLID 0x1f\n");
	return 0;

    case 0x29:			/* Geodetic Navigation Data MID 41 */
	gpsd_report(LOG_PROG, "SiRF: unused GND 0x29\n");
	return 0;

    case 0x32:			/* SBAS corrections MID 50 */
	gpsd_report(LOG_PROG, "SiRF: unused SBAS 0x32\n");
	return 0;

    case 0x34:			/* PPS Time MID 52 */
	/*
	 * Carl Carter from SiRF writes: "We do not output on the
	 * second (unless you are using MID 52).  We make
	 * measurements in the receiver in time with an internal
	 * counter that is not slaved to GPS time, so the measurements
	 * are made at a time that wanders around the second.  Then,
	 * after the measurements are made (all normalized to the same
	 * point in time) we dispatch the navigation software to make
	 * a solution, and that solution comes out some 200 to 300 ms
	 * after the measurement time.  So you may get a message at
	 * 700 ms after the second that uses measurements time tagged
	 * 450 ms after the second.  And if some other task jumps up
	 * and delays things, that message may not come out until 900
	 * ms after the second.  Things can get out of sync to the
	 * point that if you try to resolve the GPS time of our 1 PPS
	 * pulses using the navigation messages, you will find it
	 * impossible to be consistent.  That is why I added
	 * MID 52 to our system -- it is tied to the creation of the 1
	 * PPS and always comes out right around the top of the
	 * second."
	 */
	return sirf_msg_ppstime(session, buf, len);

    case 0x62:			/* uBlox Extended Measured Navigation Data MID 98 */
	gpsd_report(LOG_PROG, "SiRF: uBlox EMND 0x62\n");
	return sirf_msg_ublox(session, buf, len) | (CLEAR_IS | REPORT_IS);

    case 0x80:			/* Initialize Data Source MID 128 */
	gpsd_report(LOG_PROG, "SiRF: unused INIT 0x80\n");
	return 0;

    case 0xe1:			/* Development statistics messages MID 225 */
	/* FALLTHROUGH */
    case 0xff:			/* Debug messages MID 255 */
	(void)sirf_msg_debug(buf, len);
	return 0;

    default:
	gpsd_report(LOG_WARN, "SiRF: Unknown packet id %d length %zd\n",
		    buf[0], len);
	return 0;
    }
}

static gps_mask_t sirfbin_parse_input(struct gps_device_t *session)
{
    gps_mask_t st;

    if (session->packet.type == SIRF_PACKET) {
	st = sirf_parse(session, session->packet.outbuffer,
			session->packet.outbuflen);
	session->gpsdata.dev.driver_mode = MODE_BINARY;
	return st;
#ifdef NMEA_ENABLE
    } else if (session->packet.type == NMEA_PACKET) {
	st = nmea_parse((char *)session->packet.outbuffer, session);
	session->gpsdata.dev.driver_mode = MODE_NMEA;
	return st;
#endif /* NMEA_ENABLE */
    } else
	return 0;
}

static void sirfbin_event_hook(struct gps_device_t *session, event_t event)
{
    if (session->context->readonly)
	return;

    if (event == event_identified || event == event_reactivate) {
	if (session->packet.type == NMEA_PACKET) {
	    gpsd_report(LOG_PROG, "SiRF: Switching chip mode to binary.\n");
	    (void)nmea_send(session,
			    "$PSRF100,0,%d,8,1,0",
			    session->gpsdata.dev.baudrate);
	    (void)usleep(3330); /* guessed settling time */
	}

	gpsd_report(LOG_PROG, "SiRF: Probing for firmware version...\n");
	(void)sirf_write(session, versionprobe);
	(void)usleep(3330); /* guessed settling time */

	gpsd_report(LOG_PROG, "SiRF: Requesting navigation parameters...\n");
	(void)sirf_write(session, navparams);
	(void)usleep(3330); /* guessed settling time */

#ifdef RECONFIGURE_ENABLE
	gpsd_report(LOG_PROG, "SiRF: Requesting periodic ecef reports...\n");
	(void)sirf_write(session, requestecef);
	(void)usleep(3330); /* guessed settling time */

	gpsd_report(LOG_PROG, "SiRF: Requesting periodic tracker reports...\n");
	(void)sirf_write(session, requesttracker);
	(void)usleep(3330); /* guessed settling time */

	gpsd_report(LOG_PROG, "SiRF: Setting DGPS control to use SBAS...\n");
	(void)sirf_write(session, dgpscontrol);
	(void)usleep(3330); /* guessed settling time */

	gpsd_report(LOG_PROG, "SiRF: Setting SBAS to auto/integrity mode...\n");
	(void)sirf_write(session, sbasparams);
	(void)usleep(3330); /* guessed settling time */

	gpsd_report(LOG_PROG, "SiRF: Enabling PPS message...\n");
	(void)sirf_write(session, enablemid52);
	(void)usleep(3330); /* guessed settling time */

	if (session->gpsdata.dev.baudrate >= 38400) {
	    /* some USB devices are also too slow, no way to tell which ones */
	    gpsd_report(LOG_PROG, "SiRF: Enabling subframe transmission...\n");
	    (void)sirf_write(session, enablesubframe);
	    (void)usleep(3330); /* guessed settling time */
	}

	/* disable some MIDs. we do not decode it, so don't send it */
	gpsd_report(LOG_PROG, "SiRF: unset MID 7...\n");
	putbyte(unsetmidXX, 6, 0x11);
	(void)sirf_write(session, unsetmidXX);
	(void)usleep(3330); /* guessed settling time */

	gpsd_report(LOG_PROG, "SiRF: unset MID 28...\n");
	putbyte(unsetmidXX, 6, 0x1c);
	(void)sirf_write(session, unsetmidXX);
	(void)usleep(3330); /* guessed settling time */

	gpsd_report(LOG_PROG, "SiRF: unset MID 29...\n");
	putbyte(unsetmidXX, 6, 0x1d);
	(void)sirf_write(session, unsetmidXX);
	(void)usleep(3330); /* guessed settling time */

	gpsd_report(LOG_PROG, "SiRF: unset MID 30...\n");
	putbyte(unsetmidXX, 6, 0x1e);
	(void)sirf_write(session, unsetmidXX);
	(void)usleep(3330); /* guessed settling time */

	gpsd_report(LOG_PROG, "SiRF: unset MID 31...\n");
	putbyte(unsetmidXX, 6, 0x1f);
	(void)sirf_write(session, unsetmidXX);
	(void)usleep(3330); /* guessed settling time */
#endif /* RECONFIGURE_ENABLE */
    }

    if (event == event_deactivate) {

	/*@ +charint @*/
	static unsigned char moderevert[] = { 0xa0, 0xa2, 0x00, 0x0e,
	    0x88,
	    0x00, 0x00,		/* pad bytes */
	    0x00,		/* degraded mode */
	    0x00, 0x00,		/* pad bytes */
	    0x00, 0x00,		/* altitude source */
	    0x00,		/* altitude hold mode */
	    0x00,		/* use last computed alt */
	    0x00,		/* reserved */
	    0x00,		/* degraded mode timeout */
	    0x00,		/* dead reckoning timeout */
	    0x00,		/* track smoothing */
	    0x00, 0x00, 0xb0, 0xb3
	};
	/*@ -charint -shiftimplementation @*/
	putbyte(moderevert, 7, session->driver.sirf.degraded_mode);
	putbe16(moderevert, 10, session->driver.sirf.altitude_source_input);
	putbyte(moderevert, 12, session->driver.sirf.altitude_hold_mode);
	putbyte(moderevert, 13, session->driver.sirf.altitude_hold_source);
	putbyte(moderevert, 15, session->driver.sirf.degraded_timeout);
	putbyte(moderevert, 16, session->driver.sirf.dr_timeout);
	putbyte(moderevert, 17, session->driver.sirf.track_smooth_mode);
	/*@ +shiftimplementation @*/
	gpsd_report(LOG_PROG, "SiRF: Reverting navigation parameters...\n");
	(void)sirf_write(session, moderevert);
    }
}

/* this is everything we export */
/* *INDENT-OFF* */
const struct gps_type_t sirf_binary =
{
    .type_name      = "SiRF binary",	/* full name of type */
    .packet_type    = SIRF_PACKET,	/* associated lexer packet type */
    .flags	    = DRIVER_NOFLAGS,	/* no flags set */
    .trigger	    = NULL,		/* no trigger */
    .channels       = SIRF_CHANNELS,	/* consumer-grade GPS */
    .probe_detect   = NULL,		/* no probe */
    .get_packet     = sirf_get,		/* be prepared for SiRF or NMEA */
    .parse_packet   = sirfbin_parse_input,/* parse message packets */
    .rtcm_writer    = gpsd_write,	/* send RTCM data straight */
    .event_hook     = sirfbin_event_hook,/* lifetime event handler */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher = sirfbin_speed,	/* we can change baud rate */
    .mode_switcher  = sirfbin_mode,	/* there's a mode switcher */
    .rate_switcher  = NULL,		/* no sample-rate switcher */
    .min_cycle      = 1,		/* not relevant, no rate switch */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send   = sirf_control_send,/* how to send a control string */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = sirf_ntp_offset,
#endif /* NTP_SHM_ENABLE */
};
/* *INDENT-ON* */
#endif /* defined(SIRF_ENABLE) && defined(BINARY_ENABLE) */
