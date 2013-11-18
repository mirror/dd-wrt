/*
 * This file is Copyright (c) 2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 *
 * Driver for the iTalk binary protocol used by FasTrax
 *
 * Week counters are not limited to 10 bits. It's unknown what
 * the firmware is doing to disambiguate them, if anything; it might just
 * be adding a fixed offset based on a hidden epoch value, in which case
 * unhappy things will occur on the next rollover.
 */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#ifndef S_SPLINT_S
#include <unistd.h>
#endif /* S_SPLINT_S */

#include "gpsd.h"
#if defined(ITRAX_ENABLE) && defined(BINARY_ENABLE)

#include "bits.h"
#include "driver_italk.h"

static gps_mask_t italk_parse(struct gps_device_t *, unsigned char *, size_t);
static gps_mask_t decode_itk_navfix(struct gps_device_t *, unsigned char *,
				    size_t);
static gps_mask_t decode_itk_prnstatus(struct gps_device_t *, unsigned char *,
				       size_t);
static gps_mask_t decode_itk_utcionomodel(struct gps_device_t *,
					  unsigned char *, size_t);
static gps_mask_t decode_itk_subframe(struct gps_device_t *, unsigned char *,
				      size_t);

static gps_mask_t decode_itk_navfix(struct gps_device_t *session,
				    unsigned char *buf, size_t len)
{
    unsigned short flags, pflags;
    gps_mask_t mask = 0;
    double epx, epy, epz, evx, evy, evz, eph;

    if (len != 296) {
	gpsd_report(LOG_PROG, "ITALK: bad NAV_FIX (len %zu, should be 296)\n",
		    len);
	return -1;
    }

    flags = (unsigned short) getleu16(buf, 7 + 4);
    //cflags = (unsigned short) getleu16(buf, 7 + 6);
    pflags = (unsigned short) getleu16(buf, 7 + 8);

    session->gpsdata.status = STATUS_NO_FIX;
    session->newdata.mode = MODE_NO_FIX;
    mask = ONLINE_SET | MODE_SET | STATUS_SET | CLEAR_IS;

    /* just bail out if this fix is not marked valid */
    if (0 != (pflags & FIX_FLAG_MASK_INVALID)
	|| 0 == (flags & FIXINFO_FLAG_VALID))
	return mask;

    session->newdata.time = gpsd_gpstime_resolve(session,
	(unsigned short) getles16(buf, 7 + 82),
	(unsigned int)getleu32(buf, 7 + 84) / 1000.0);
    mask |= TIME_SET | PPSTIME_IS;

    epx = (double)(getles32(buf, 7 + 96) / 100.0);
    epy = (double)(getles32(buf, 7 + 100) / 100.0);
    epz = (double)(getles32(buf, 7 + 104) / 100.0);
    evx = (double)(getles32(buf, 7 + 186) / 1000.0);
    evy = (double)(getles32(buf, 7 + 190) / 1000.0);
    evz = (double)(getles32(buf, 7 + 194) / 1000.0);
    ecef_to_wgs84fix(&session->newdata, &session->gpsdata.separation,
		     epx, epy, epz, evx, evy, evz);
    mask |= LATLON_SET | ALTITUDE_SET | SPEED_SET | TRACK_SET | CLIMB_SET;
    eph = (double)(getles32(buf, 7 + 252) / 100.0);
    /* eph is a circular error, sqrt(epx**2 + epy**2) */
    session->newdata.epx = session->newdata.epy = eph / sqrt(2);
    session->newdata.eps = (double)(getles32(buf, 7 + 254) / 100.0);

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
    session->gpsdata.satellites_used =
	(int)MAX(getleu16(buf, 7 + 12), getleu16(buf, 7 + 14));
    mask |= USED_IS;

    if (flags & FIX_CONV_DOP_VALID) {
	session->gpsdata.dop.hdop = (double)(getleu16(buf, 7 + 56) / 100.0);
	session->gpsdata.dop.gdop = (double)(getleu16(buf, 7 + 58) / 100.0);
	session->gpsdata.dop.pdop = (double)(getleu16(buf, 7 + 60) / 100.0);
	session->gpsdata.dop.vdop = (double)(getleu16(buf, 7 + 62) / 100.0);
	session->gpsdata.dop.tdop = (double)(getleu16(buf, 7 + 64) / 100.0);
	mask |= DOP_SET;
    }

    if ((pflags & FIX_FLAG_MASK_INVALID) == 0
	&& (flags & FIXINFO_FLAG_VALID) != 0) {
	if (pflags & FIX_FLAG_3DFIX)
	    session->newdata.mode = MODE_3D;
	else
	    session->newdata.mode = MODE_2D;

	if (pflags & FIX_FLAG_DGPS_CORRECTION)
	    session->gpsdata.status = STATUS_DGPS_FIX;
	else
	    session->gpsdata.status = STATUS_FIX;
    }

    gpsd_report(LOG_DATA,
		"NAV_FIX: time=%.2f, lat=%.2f lon=%.2f alt=%.f speed=%.2f track=%.2f climb=%.2f mode=%d status=%d gdop=%.2f pdop=%.2f hdop=%.2f vdop=%.2f tdop=%.2f\n",
		session->newdata.time, session->newdata.latitude,
		session->newdata.longitude, session->newdata.altitude,
		session->newdata.speed, session->newdata.track,
		session->newdata.climb, session->newdata.mode,
		session->gpsdata.status, session->gpsdata.dop.gdop,
		session->gpsdata.dop.pdop, session->gpsdata.dop.hdop,
		session->gpsdata.dop.vdop, session->gpsdata.dop.tdop);
    return mask;
}

static gps_mask_t decode_itk_prnstatus(struct gps_device_t *session,
				       unsigned char *buf, size_t len)
{
    gps_mask_t mask;

    if (len < 62) {
	gpsd_report(LOG_PROG, "ITALK: runt PRN_STATUS (len=%zu)\n", len);
	mask = 0;
    } else {
	unsigned int i, nsv, nchan, st;

	session->gpsdata.skyview_time = gpsd_gpstime_resolve(session,
	    (unsigned short)getleu16(buf, 7 + 4),
	    (unsigned int)getleu32(buf, 7 + 6) / 1000.0),
	gpsd_zero_satellites(&session->gpsdata);
	nsv = 0;
	nchan = (unsigned int)getleu16(buf, 7 + 50);
	if (nchan > MAX_NR_VISIBLE_PRNS)
	    nchan = MAX_NR_VISIBLE_PRNS;
	for (i = st = 0; i < nchan; i++) {
	    unsigned int off = 7 + 52 + 10 * i;
	    unsigned short flags;

	    flags = (unsigned short) getleu16(buf, off);
	    session->gpsdata.ss[i] = (float)(getleu16(buf, off + 2) & 0xff);
	    session->gpsdata.PRN[i] = (int)getleu16(buf, off + 4) & 0xff;
	    session->gpsdata.elevation[i] = (int)getles16(buf, off + 6) & 0xff;
	    session->gpsdata.azimuth[i] = (int)getles16(buf, off + 8) & 0xff;
	    if (session->gpsdata.PRN[i]) {
		st++;
		if (flags & PRN_FLAG_USE_IN_NAV)
		    session->gpsdata.used[nsv++] = session->gpsdata.PRN[i];
	    }
	}
	session->gpsdata.satellites_visible = (int)st;
	session->gpsdata.satellites_used = (int)nsv;
	mask = USED_IS | SATELLITE_SET;;

	gpsd_report(LOG_DATA,
		    "PRN_STATUS: time=%.2f visible=%d used=%d mask={USED|SATELLITE}\n",
		    session->newdata.time,
		    session->gpsdata.satellites_visible,
		    session->gpsdata.satellites_used);
    }

    return mask;
}

static gps_mask_t decode_itk_utcionomodel(struct gps_device_t *session,
					  unsigned char *buf, size_t len)
{
    int leap;
    unsigned short flags;

    if (len != 64) {
	gpsd_report(LOG_PROG,
		    "ITALK: bad UTC_IONO_MODEL (len %zu, should be 64)\n",
		    len);
	return 0;
    }

    flags = (unsigned short) getleu16(buf, 7);
    if (0 == (flags & UTC_IONO_MODEL_UTCVALID))
	return 0;

    leap = (int)getleu16(buf, 7 + 24);
    if (session->context->leap_seconds < leap)
	session->context->leap_seconds = leap;

    session->newdata.time = gpsd_gpstime_resolve(session,
	(unsigned short) getleu16(buf, 7 + 36),
	(unsigned int)getleu32(buf, 7 + 38)  / 1000.0);
    gpsd_report(LOG_DATA,
		"UTC_IONO_MODEL: time=%.2f mask={TIME}\n",
		session->newdata.time);
    return TIME_SET | PPSTIME_IS;
}

static gps_mask_t decode_itk_subframe(struct gps_device_t *session,
				      unsigned char *buf, size_t len)
{
    unsigned short flags, prn, sf;
    unsigned int i;
    uint32_t words[10];

    if (len != 64) {
	gpsd_report(LOG_PROG,
		    "ITALK: bad SUBFRAME (len %zu, should be 64)\n", len);
	return 0;
    }

    flags = (unsigned short) getleu16(buf, 7 + 4);
    prn = (unsigned short) getleu16(buf, 7 + 6);
    sf = (unsigned short) getleu16(buf, 7 + 8);
    gpsd_report(LOG_PROG, "iTalk 50B SUBFRAME prn %u sf %u - decode %s %s\n",
		prn, sf,
		flags & SUBFRAME_WORD_FLAG_MASK ? "error" : "ok",
		flags & SUBFRAME_GPS_PREAMBLE_INVERTED ? "(inverted)" : "");
    if (flags & SUBFRAME_WORD_FLAG_MASK)
	return 0;	// don't try decode an erroneous packet

    /*
     * Timo says "SUBRAME message contains decoded navigation message subframe
     * words with parity checking done but parity bits still present."
     */
    for (i = 0; i < 10; i++)
	words[i] = (uint32_t)(getleu32(buf, 7 + 14 + 4 * i) >> 6) & 0xffffff;

    return gpsd_interpret_subframe(session, prn, words);
}

static gps_mask_t decode_itk_pseudo(struct gps_device_t *session,
				      unsigned char *buf, size_t len)
{
    unsigned short flags, n, i;

    n = (unsigned short) getleu16(buf, 7 + 4);
    if ((n < 1) || (n > MAXCHANNELS)){
	gpsd_report(LOG_INF, "ITALK: bad PSEUDO channel count\n");
	return 0;
    }

    if (len != (size_t)((n+1)*36)) {
	gpsd_report(LOG_PROG,
		    "ITALK: bad PSEUDO len %zu\n", len);
    }

    gpsd_report(LOG_PROG, "iTalk PSEUDO [%u]\n", n);
    flags = (unsigned short)getleu16(buf, 7 + 6);
    if ((flags & 0x3) != 0x3)
	return 0; // bail if measurement time not valid.

    session->newdata.time = gpsd_gpstime_resolve(session,
						 (unsigned short int)getleu16((char *)buf, 7 + 8),
	(unsigned int)getleu32(buf, 7 + 38) / 1000.0);

    /*@-type@*/
    for (i = 0; i < n; i++){
	session->gpsdata.PRN[i] = getleu16(buf, 7 + 26 + (i*36)) & 0xff;
	session->gpsdata.ss[i] = getleu16(buf, 7 + 26 + (i*36 + 2)) & 0x3f;
	session->gpsdata.raw.satstat[i] = getleu32(buf, 7 + 26 + (i*36 + 4));
	session->gpsdata.raw.pseudorange[i] = getled64((char *)buf, 7 + 26 + (i*36 + 8));
	session->gpsdata.raw.doppler[i] = getled64((char *)buf, 7 + 26 + (i*36 + 16));
	session->gpsdata.raw.carrierphase[i] = getleu16(buf, 7 + 26 + (i*36 + 28));

	session->gpsdata.raw.mtime[i] = session->newdata.time;
	session->gpsdata.raw.codephase[i] = NAN;
	session->gpsdata.raw.deltarange[i] = NAN;
    }
    /*@+type@*/
    return RAW_IS;
}

/*@ +charint @*/
static gps_mask_t italk_parse(struct gps_device_t *session,
			      unsigned char *buf, size_t len)
{
    unsigned int type;
    gps_mask_t mask = 0;

    if (len == 0)
	return 0;

    type = (uint) getub(buf, 4);
    /* we may need to dump the raw packet */
    gpsd_report(LOG_RAW, "raw italk packet type 0x%02x\n", type);

    session->cycle_end_reliable = true;

    switch (type) {
    case ITALK_NAV_FIX:
	gpsd_report(LOG_IO, "iTalk NAV_FIX len %zu\n", len);
	mask = decode_itk_navfix(session, buf, len) | (CLEAR_IS | REPORT_IS);
	break;
    case ITALK_PRN_STATUS:
	gpsd_report(LOG_IO, "iTalk PRN_STATUS len %zu\n", len);
	mask = decode_itk_prnstatus(session, buf, len);
	break;
    case ITALK_UTC_IONO_MODEL:
	gpsd_report(LOG_IO, "iTalk UTC_IONO_MODEL len %zu\n", len);
	mask = decode_itk_utcionomodel(session, buf, len);
	break;

    case ITALK_ACQ_DATA:
	gpsd_report(LOG_IO, "iTalk ACQ_DATA len %zu\n", len);
	break;
    case ITALK_TRACK:
	gpsd_report(LOG_IO, "iTalk TRACK len %zu\n", len);
	break;
    case ITALK_PSEUDO:
	gpsd_report(LOG_IO, "iTalk PSEUDO len %zu\n", len);
	mask = decode_itk_pseudo(session, buf, len);
	break;
    case ITALK_RAW_ALMANAC:
	gpsd_report(LOG_IO, "iTalk RAW_ALMANAC len %zu\n", len);
	break;
    case ITALK_RAW_EPHEMERIS:
	gpsd_report(LOG_IO, "iTalk RAW_EPHEMERIS len %zu\n", len);
	break;
    case ITALK_SUBFRAME:
	mask = decode_itk_subframe(session, buf, len);
	break;
    case ITALK_BIT_STREAM:
	gpsd_report(LOG_IO, "iTalk BIT_STREAM len %zu\n", len);
	break;

    case ITALK_AGC:
    case ITALK_SV_HEALTH:
    case ITALK_PRN_PRED:
    case ITALK_FREQ_PRED:
    case ITALK_DBGTRACE:
    case ITALK_START:
    case ITALK_STOP:
    case ITALK_SLEEP:
    case ITALK_STATUS:
    case ITALK_ITALK_CONF:
    case ITALK_SYSINFO:
    case ITALK_ITALK_TASK_ROUTE:
    case ITALK_PARAM_CTRL:
    case ITALK_PARAMS_CHANGED:
    case ITALK_START_COMPLETED:
    case ITALK_STOP_COMPLETED:
    case ITALK_LOG_CMD:
    case ITALK_SYSTEM_START:
    case ITALK_STOP_SEARCH:
    case ITALK_SEARCH:
    case ITALK_PRED_SEARCH:
    case ITALK_SEARCH_DONE:
    case ITALK_TRACK_DROP:
    case ITALK_TRACK_STATUS:
    case ITALK_HANDOVER_DATA:
    case ITALK_CORE_SYNC:
    case ITALK_WAAS_RAWDATA:
    case ITALK_ASSISTANCE:
    case ITALK_PULL_FIX:
    case ITALK_MEMCTRL:
    case ITALK_STOP_TASK:
	gpsd_report(LOG_IO,
		    "iTalk not processing packet: id 0x%02x length %zu\n",
		    type, len);
	break;
    default:
	gpsd_report(LOG_IO, "iTalk unknown packet: id 0x%02x length %zu\n",
		    type, len);
    }
    (void)snprintf(session->gpsdata.tag, sizeof(session->gpsdata.tag),
		       "ITK-%02x", type);

    return mask | ONLINE_SET;
}

/*@ -charint @*/

static gps_mask_t italk_parse_input(struct gps_device_t *session)
{
    gps_mask_t st;

    if (session->packet.type == ITALK_PACKET) {
	st = italk_parse(session, session->packet.outbuffer,
			 session->packet.outbuflen);
	session->gpsdata.dev.driver_mode = MODE_BINARY;	/* binary */
	return st;
#ifdef NMEA_ENABLE
    } else if (session->packet.type == NMEA_PACKET) {
	st = nmea_parse((char *)session->packet.outbuffer, session);
	session->gpsdata.dev.driver_mode = MODE_NMEA;	/* NMEA */
	return st;
#endif /* NMEA_ENABLE */
    } else
	return 0;
}

#ifdef __future__
// cppcheck-suppress unusedFunction
static void italk_ping(struct gps_device_t *session)
/* send a "ping". it may help us detect an itrax more quickly */
{
    char *ping = "<?>";
    (void)gpsd_write(session, ping, 3);
}
#endif /* __future__ */

/* *INDENT-OFF* */
const struct gps_type_t italk_binary =
{
    .type_name      = "iTalk binary",	/* full name of type */
    .packet_type    = ITALK_PACKET,	/* associated lexer packet type */
    .flags	    = DRIVER_NOFLAGS,	/* no rollover or other flags */
    .trigger	    = NULL,		/* recognize the type */
    .channels       = 12,		/* consumer-grade GPS */
    .probe_detect   = NULL,		/* how to detect at startup time */
    .get_packet     = generic_get,	/* use generic packet grabber */
    .parse_packet   = italk_parse_input,/* parse message packets */
    .rtcm_writer    = gpsd_write,	/* send RTCM data straight */
    .event_hook     = NULL,		/* lifetime event handler */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher = NULL,		/* no speed switcher */
    .mode_switcher  = NULL,		/* no mode switcher */
    .rate_switcher  = NULL,		/* no sample-rate switcher */
    .min_cycle      = 1,		/* not relevant, no rate switch */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send   = NULL,		/* no control string sender */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = NULL,		/* no method for NTP fudge factor */
#endif /* NTPSHM_ ENABLE */
};
/* *INDENT-ON* */
#endif /* defined(ITRAX_ENABLE) && defined(BINARY_ENABLE) */
