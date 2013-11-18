/* $Id: drivers.c 4081 2006-12-05 13:44:39Z esr $ */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>
#include <stdarg.h>

#include "gpsd_config.h"
#include "gpsd.h"

extern struct gps_type_t zodiac_binary;

ssize_t generic_get(struct gps_device_t *session)
{
    return packet_get(session->gpsdata.gps_fd, &session->packet);
}

#if defined(NMEA_ENABLE) || defined(SIRF_ENABLE) || defined(EVERMORE_ENABLE)  || defined(ITALK_ENABLE) 
ssize_t pass_rtcm(struct gps_device_t *session, char *buf, size_t rtcmbytes)
/* most GPSes take their RTCM corrections straight up */
{
    return write(session->gpsdata.gps_fd, buf, rtcmbytes);
}
#endif

#ifdef NMEA_ENABLE
/**************************************************************************
 *
 * Generic driver -- straight NMEA 0183
 *
 **************************************************************************/

gps_mask_t nmea_parse_input(struct gps_device_t *session)
{
    if (session->packet.type == COMMENT_PACKET) {
	return 0;
    } else if (session->packet.type == SIRF_PACKET) {
	gpsd_report(LOG_WARN, "SiRF packet seen when NMEA expected.\n");
#ifdef SIRF_ENABLE
	(void)gpsd_switch_driver(session, "SiRF binary");
	return sirf_parse(session, session->packet.outbuffer, session->packet.outbuflen);
#else
	return 0;
#endif /* SIRF_ENABLE */
    } else if (session->packet.type == EVERMORE_PACKET) {
	gpsd_report(LOG_WARN, "EverMore packet seen when NMEA expected.\n");
#ifdef EVERMORE_ENABLE
	(void)gpsd_switch_driver(session, "EverMore binary");
	return evermore_parse(session, session->packet.outbuffer, session->packet.outbuflen);
#else
	return 0;
#endif /* EVERMORE_ENABLE */
    } else if (session->packet.type == GARMIN_PACKET) {
	gpsd_report(LOG_WARN, "Garmin packet seen when NMEA expected.\n");
#ifdef GARMIN_ENABLE
	/* we might never see a trigger, have this as a backstop */
	(void)gpsd_switch_driver(session, "Garmin Serial binary");
	return garmin_ser_parse(session);
#else
	return 0;
#endif /* GARMIN_ENABLE */
    } else if (session->packet.type == NMEA_PACKET) {
	gps_mask_t st = 0;
	gpsd_report(LOG_IO, "<= GPS: %s", session->packet.outbuffer);
	if ((st=nmea_parse((char *)session->packet.outbuffer, session))==0) {
#ifdef NON_NMEA_ENABLE
	    struct gps_type_t **dp;

	    /* maybe this is a trigger string for a driver we know about? */
	    for (dp = gpsd_drivers; *dp; dp++) {
		char	*trigger = (*dp)->trigger;

		if (trigger!=NULL && strncmp((char *)session->packet.outbuffer, trigger, strlen(trigger))==0 && isatty(session->gpsdata.gps_fd)!=0) {
		    gpsd_report(LOG_PROG, "found %s.\n", trigger);
		    (void)gpsd_switch_driver(session, (*dp)->typename);
		    return 1;
		}
	    }
#endif /* NON_NMEA_ENABLE */
	    gpsd_report(LOG_WARN, "unknown sentence: \"%s\"\n", session->packet.outbuffer);
	}
#ifdef NMEADISC
	if (session->gpsdata.ldisc == 0) {
	    uid_t old;
	    int ldisc = NMEADISC;

#ifdef TIOCSTSTAMP
	    struct tstamps tstamps;
#ifdef PPS_ON_CTS
	    tstamps.ts_set |= TIOCM_CTS;
#else /*!PPS_ON_CTS */
	    tstamps.ts_set |= TIOCM_CAR;
#endif /* PPS_ON_CTS */
	    tstamps.ts_clr = 0;

	    old = geteuid();
	    if (seteuid(0) == -1)
		gpsd_report(LOG_WARN, "can't seteuid(0) - %s", strerror(errno));
	    else
		gpsd_report(LOG_WARN, "seteuid(0) to enable timestamping");
	    if (ioctl(session->gpsdata.gps_fd, TIOCSTSTAMP, &tstamps) < 0)
		gpsd_report(LOG_WARN, "can't set kernel timestamping: %s\n",
		    strerror(errno));
	    else
		gpsd_report(LOG_WARN, "activated kernel timestamping\n");
#endif /* TIOCSTSTAMP */
	    if (ioctl(session->gpsdata.gps_fd, TIOCSETD, &ldisc) == -1)
		gpsd_report(LOG_WARN, "can't set nmea discipline: %s\n",
		    strerror(errno));
	    else
		gpsd_report(LOG_WARN, "activated nmea discipline\n");
	/* this is a flag that shows if we've tried the setup */
	session->gpsdata.ldisc = NMEADISC;

	if (old){
	    gpsd_report(LOG_WARN, "giving up euid 0");
	    (void)seteuid(old);
	}
	gpsd_report(LOG_WARN, "running with effective user ID %d\n", geteuid());
	}
#endif /*NMEADISC */
#ifdef NTPSHM_ENABLE
	/* this magic number is derived from observation */
	if (session->context->enable_ntpshm &&
	    (st & TIME_SET) != 0 &&
	    (session->gpsdata.fix.time!=session->last_fixtime)) {
	    /* this magic number is derived from observation */
	    /* GPS-18/USB -> 0.100 */
	    /* GPS-18/LVC at 19200 -> 0.125 */
	    /* GPS-18/LVC at 4800 -> 0.525*/
	    /* Rob Jensen reports 0.675 */
	    (void)ntpshm_put(session, session->gpsdata.fix.time + 0.400);
	    session->last_fixtime = session->gpsdata.fix.time;
	}
#endif /* NTPSHM_ENABLE */
	return st;
    } else
	return 0;
}

static void nmea_probe_subtype(struct gps_device_t *session, unsigned int seq)
{
    /*
     * The reason for splitting these probes up by packet sequence
     * number, interleaving them with the first few packet receives,
     * is because many generic-NMEA devices get confused if you send
     * too much at them in one go.  
     *
     * A fast response to an early probe will change drivers so the
     * later ones won't be sent at all.  Thus, for best overall
     * performance, order these to probe for the most popular types
     * soonest.
     *
     * Note: don't make the trigger strings identical to the probe,
     * because some NMEA devices (notably SiRFs) will just echo 
     * unknown strings right back at you. A useful dodge is to append
     * a comma to the trigger, because that won't be in the response 
     * unless there is actual following data.
     */
    switch (seq) {
#ifdef SIRF_ENABLE
    case 0:
	/* 
	 * We used to try to probe for SiRF by issuing "$PSRF105,1"
	 * and expecting "$Ack Input105.".  But it turns out this
	 * only works for SiRF-IIs; SiRF-I and SiRF-III don't respond.
	 * Thus the only reliable probe is to try to flip the SiRF into
	 * binary mode, cluing in the library to revert it on close.
	 */
	(void)nmea_send(session->gpsdata.gps_fd, 
			"$PSRF100,0,%d,%d,%d,0", 
			session->gpsdata.baudrate,
			9-session->gpsdata.stopbits,
			session->gpsdata.stopbits);
	session->back_to_nmea = true;
	break;
#endif /* SIRF_ENABLE */
#ifdef NMEA_ENABLE
    case 1:
	/* probe for Garmin serial GPS -- expect $PGRMC followed by data*/
	// (void)nmea_send(session->gpsdata.gps_fd, "$PGRMCE");
	(void)nmea_send(session->gpsdata.gps_fd, "$PTNLSNM,0139,01");
	break;
    case 2:
	/* probe for the FV-18 -- expect $PFEC,GPint followed by data */
	(void)nmea_send(session->gpsdata.gps_fd, "$PFEC,GPint");
	break;
#endif /* NMEA_ENABLE */
#ifdef EVERMORE_ENABLE
    case 3:
	/* Enable checksum and GGA(1s), GLL(0s), GSA(1s), GSV(1s), RMC(1s), VTG(0s), PEMT101(1s) */
	/* EverMore will reply with: \x10\x02\x04\x38\x8E\xC6\x10\x03 */
	(void)gpsd_write(session, 
	    "\x10\x02\x12\x8E\x7F\x01\x01\x00\x01\x01\x01\x00\x01\x00\x00\x00\x00\x00\x00\x13\x10\x03", 22);
	break;
#endif /* EVERMORE_ENABLE */
#ifdef ITRAX_ENABLE
    case 4:
	/* probe for iTrax, looking for "$PFST,OK" */
	(void)nmea_send(session->gpsdata.gps_fd, "$PFST");
	break;
#endif /* ITRAX_ENABLE */
    default:
	break;
    }
}

static struct gps_type_t nmea = {
    .typename       = "Generic NMEA",	/* full name of type */
    .trigger        = NULL,		/* it's the default */
    .channels       = 12,		/* consumer-grade GPS */
    .probe_wakeup   = NULL,		/* no wakeup to be done before hunt */
    .probe_detect   = NULL,		/* no probe */
    .probe_subtype  = nmea_probe_subtype,	/* probe for special types */
#ifdef ALLOW_RECONFIGURE
    .configurator   = NULL,		/* enable what we need */
#endif /* ALLOW_RECONFIGURE */
    .get_packet     = generic_get,		/* use generic packet getter */
    .parse_packet   = nmea_parse_input,	/* how to interpret a packet */
    .rtcm_writer    = pass_rtcm,	/* write RTCM data straight */
    .speed_switcher = NULL,		/* no speed switcher */
    .mode_switcher  = NULL,		/* no mode switcher */
    .rate_switcher  = NULL,		/* no sample-rate switcher */
    .cycle_chars    = -1,		/* not relevant, no rate switch */
#ifdef ALLOW_RECONFIGURE
    .revert         = NULL,		/* no setting-reversion method */
#endif /* ALLOW_RECONFIGURE */
    .wrapup         = NULL,		/* no wrapup */
    .cycle          = 1,		/* updates every second */
};

#ifdef GARMIN_ENABLE
/**************************************************************************
 *
 * Garmin NMEA
 *
 **************************************************************************/

#ifdef ALLOW_RECONFIGURE
static void garmin_nmea_configurator(struct gps_device_t *session, unsigned int seq)
{
#if defined(NMEA_ENABLE)
    /*
     * Receivers like the Garmin GPS-10 don't handle having a lot of 
     */
    switch (seq) {
    case 0:
	/* reset some config, AutoFix, WGS84, PPS */
	(void)nmea_send(session->gpsdata.gps_fd, "$PGRMC,A,,100,,,,,,A,,1,2,4,30");
	break;
    case 1:
	/* once a sec, no averaging, NMEA 2.3, WAAS */
	(void)nmea_send(session->gpsdata.gps_fd, "$PGRMC1,1,1,1,,,,2,W,N");
	break;
    case 2:
	/* get some more config info */
	(void)nmea_send(session->gpsdata.gps_fd, "$PGRMC1E");
	break;
    case 3:
	/* turn off all output except GGA */
	(void)nmea_send(session->gpsdata.gps_fd, "$PGRMO,,2");
	(void)nmea_send(session->gpsdata.gps_fd, "$PGRMO,GPGGA,1");
	break;
    case 4:
	/* enable GPGGA, GPGSA, GPGSV, GPRMC on Garmin serial GPS */
	(void)nmea_send(session->gpsdata.gps_fd, "$PGRMO,GPGSA,1");
	break;
    case 5:
	(void)nmea_send(session->gpsdata.gps_fd, "$PGRMO,GPGSV,1");
	break;
    case 6:
	(void)nmea_send(session->gpsdata.gps_fd, "$PGRMO,GPRMC,1");
	break;
    case 7:
	(void)nmea_send(session->gpsdata.gps_fd, "$PGRMO,PGRME,1");
	break;
    }
}
#endif /* NMEA_ENABLE */
#endif /* ALLOW_RECONFIGURE */

static struct gps_type_t garmin = {
    .typename       = "Garmin Serial",	/* full name of type */
    .trigger        = "$PGRMC,",	/* Garmin private */
    .channels       = 12,		/* not used by this driver */
    .probe_wakeup   = NULL,		/* no wakeup to be done before hunt */
    .probe_detect   = NULL,		/* no probe */
    .probe_subtype   = NULL,		/* no further querying */
#ifdef ALLOW_RECONFIGURE
    .configurator   = garmin_nmea_configurator,/* enable what we need */
#endif /*ALLOW_RECONFIGURE */
    .get_packet     = generic_get,	/* use generic packet getter */
    .parse_packet   = nmea_parse_input,	/* how to interpret a packet */
    .rtcm_writer    = NULL,		/* some do, some don't, skip for now */
    .speed_switcher = NULL,		/* no speed switcher */
    .mode_switcher  = NULL,		/* no mode switcher */
    .rate_switcher  = NULL,		/* no sample-rate switcher */
    .cycle_chars    = -1,		/* not relevant, no rate switch */
#ifdef ALLOW_RECONFIGURE
    .revert         = NULL,		/* no setting-reversion method */
#endif /*ALLOW_RECONFIGURE */
    .wrapup         = NULL,		/* no wrapup */
    .cycle          = 1,		/* updates every second */
};
#endif /* GARMIN_ENABLE */

#ifdef FV18_ENABLE
/**************************************************************************
 *
 * FV18 -- uses 2 stop bits, needs to be told to send GSAs
 *
 **************************************************************************/

#ifdef ALLOW_RECONFIGURE
static void fv18_configure(struct gps_device_t *session, unsigned int seq)
{
    /*
     * Tell an FV18 to send GSAs so we'll know if 3D is accurate.
     * Suppress GLL and VTG.  Enable ZDA so dates will be accurate for replay.
     */
    if (seq == 0)
	(void)nmea_send(session->gpsdata.gps_fd,
		    "$PFEC,GPint,GSA01,DTM00,ZDA01,RMC01,GLL00,VTG00,GSV05");
}
#endif /* ALLOW_RECONFIGURE */

static struct gps_type_t fv18 = {
    .typename       = "San Jose Navigation FV18",	/* full name of type */
    .trigger        = "$PFEC,GPint,",	/* FV18s should echo the probe */
    .channels       = 12,		/* not used by this driver */
    .probe_wakeup   = NULL,		/* no wakeup to be done before hunt */
    .probe_detect   = NULL,		/* mo probe */
    .probe_subtype  = NULL,		/* to be sent unconditionally */
#ifdef ALLOW_RECONFIGURE
    .configurator   = fv18_configure,	/* change its sentence set */
#endif /* ALLOW_RECONFIGURE */
    .get_packet     = generic_get,	/* how to get a packet */
    .parse_packet   = nmea_parse_input,	/* how to interpret a packet */
    .rtcm_writer    = pass_rtcm,	/* write RTCM data straight */
    .speed_switcher = NULL,		/* no speed switcher */
    .mode_switcher  = NULL,		/* no mode switcher */
    .rate_switcher  = NULL,		/* no sample-rate switcher */
    .cycle_chars    = -1,		/* not relevant, no rate switch */
#ifdef ALLOW_RECONFIGURE
    .revert         = NULL,		/* no setting-reversion method */
#endif /* ALLOW_RECONFIGURE */
    .wrapup         = NULL,		/* no wrapup */
    .cycle          = 1,		/* updates every second */
};
#endif /* FV18_ENABLE */

#ifdef TRIPMATE_ENABLE
/**************************************************************************
 *
 * TripMate -- extended NMEA, gets faster fix when primed with lat/long/time
 *
 **************************************************************************/

/*
 * Some technical FAQs on the TripMate:
 * http://vancouver-webpages.com/pub/peter/tripmate.faq
 * http://www.asahi-net.or.jp/~KN6Y-GTU/tripmate/trmfaqe.html
 * The TripMate was discontinued sometime before November 1998
 * and was replaced by the Zodiac EarthMate.
 */

static void tripmate_probe_subtype(struct gps_device_t *session, unsigned int seq)
{
    /* TripMate requires this response to the ASTRAL it sends at boot time */
    if (seq == 0)
	(void)nmea_send(session->gpsdata.gps_fd, "$IIGPQ,ASTRAL");
}

#ifdef ALLOW_RECONFIGURE
static void tripmate_configurator(struct gps_device_t *session, unsigned int seq)
{
    /* stop it sending PRWIZCH */
    if (seq == 0)
	(void)nmea_send(session->gpsdata.gps_fd, "$PRWIILOG,ZCH,V,,");
}
#endif /* ALLOW_RECONFIGURE */

static struct gps_type_t tripmate = {
    .typename      = "Delorme TripMate",	/* full name of type */
    .trigger       ="ASTRAL",			/* tells us to switch */
    .channels      = 12,			/* consumer-grade GPS */
    .probe_wakeup  = NULL,			/* no wakeup before hunt */
    .probe_detect  = NULL,			/* no probe */
    .probe_subtype = tripmate_probe_subtype,	/* send unconditionally */
#ifdef ALLOW_RECONFIGURE
    .configurator  = tripmate_configurator,	/* send unconditionally */
#endif /* ALLOW_RECONFIGURE */
    .get_packet    = generic_get,		/* how to get a packet */
    .parse_packet  = nmea_parse_input,		/* how to interpret a packet */
    .rtcm_writer   = pass_rtcm,			/* send RTCM data straight */
    .speed_switcher= NULL,			/* no speed switcher */
    .mode_switcher = NULL,			/* no mode switcher */
    .rate_switcher = NULL,			/* no sample-rate switcher */
    .cycle_chars   = -1,			/* no rate switch */
#ifdef ALLOW_RECONFIGURE
    .revert         = NULL,		/* no setting-reversion method */
#endif /* ALLOW_RECONFIGURE */
    .wrapup         = NULL,			/* no wrapup */
    .cycle          = 1,			/* updates every second */
};
#endif /* TRIPMATE_ENABLE */

#ifdef EARTHMATE_ENABLE
/**************************************************************************
 *
 * Zodiac EarthMate textual mode
 *
 * Note: This is the pre-2003 version using Zodiac binary protocol.
 * It has been replaced with a design that uses a SiRF chipset.
 *
 **************************************************************************/

static struct gps_type_t earthmate;

/*
 * There is a good HOWTO at <http://www.hamhud.net/ka9mva/earthmate.htm>.
 */

static void earthmate_close(struct gps_device_t *session)
{
    /*@i@*/session->device_type = &earthmate;
}

static void earthmate_probe_subtype(struct gps_device_t *session, unsigned int seq)
{
    if (seq == 0) {
	(void)write(session->gpsdata.gps_fd, "EARTHA\r\n", 8);
	(void)usleep(10000);
	/*@i@*/session->device_type = &zodiac_binary;
	zodiac_binary.wrapup = earthmate_close;
	if (zodiac_binary.probe_subtype) zodiac_binary.probe_subtype(session, seq);
    }
}

/*@ -redef @*/
static struct gps_type_t earthmate = {
    .typename      = "Delorme EarthMate (pre-2003, Zodiac chipset)",
    .trigger       = "EARTHA",			/* Earthmate trigger string */
    .channels      = 12,			/* not used by NMEA parser */
    .probe_wakeup  = NULL,		/* no wakeup to be done before hunt */
    .probe_detect  = NULL,			/* no probe */
    .probe_subtype = earthmate_probe_subtype,	/* switch us to Zodiac mode */
#ifdef ALLOW_RECONFIGURE
    .configurator  = NULL,			/* no configuration here */
#endif /* ALLOW_RECONFIGURE */
    .get_packet    = generic_get,		/* how to get a packet */
    .parse_packet  = nmea_parse_input,		/* how to interpret a packet */
    .rtcm_writer   = NULL,			/* don't send RTCM data */
    .speed_switcher= NULL,			/* no speed switcher */
    .mode_switcher = NULL,			/* no mode switcher */
    .rate_switcher = NULL,			/* no sample-rate switcher */
    .cycle_chars   = -1,			/* no rate switch */
#ifdef ALLOW_RECONFIGURE
    .revert         = NULL,		/* no setting-reversion method */
#endif /* ALLOW_RECONFIGURE */
    .wrapup         = NULL,			/* no wrapup code */
    .cycle          = 1,			/* updates every second */
};
/*@ -redef @*/
#endif /* EARTHMATE_ENABLE */


#ifdef ITRAX_ENABLE
/**************************************************************************
 *
 * The NMEA mode of the iTrax chipset, as used in the FastTrax and others.
 *
 * As described by v1.31 of the NMEA Protocol Specification for the
 * iTrax02 Evaluation Kit, 2003-06-12.
 * v1.18 of the  manual, 2002-19-6, describes effectively
 * the same protocol, but without ZDA.
 *
 **************************************************************************/

/*
 * Enable GGA=0x2000, RMC=0x8000, GSA=0x0002, GSV=0x0001, ZDA=0x0004.
 * Disable GLL=0x1000, VTG=0x4000, FOM=0x0020, PPS=0x0010.
 * This is 82+75+67+(3*60)+34 = 438 characters 
 * 
 * 1200   => at most 1 fix per 4 seconds
 * 2400   => at most 1 fix per 2 seconds
 * 4800   => at most 1 fix per 1 seconds
 * 9600   => at most 2 fixes per second
 * 19200  => at most 4 fixes per second
 * 57600  => at most 13 fixes per second
 * 115200 => at most 26 fixes per second
 *
 * We'd use FOM, but they don't specify a confidence interval.
 */
#define ITRAX_MODESTRING	"$PFST,NMEA,A007,%d\r\n"

static int literal_send(int fd, const char *fmt, ... )
/* ship a raw command to the GPS */
{
    int status;
    char buf[BUFSIZ];
    va_list ap;

    va_start(ap, fmt) ;
    (void)vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    status = (int)write(fd, buf, strlen(buf));
    if (status == (int)strlen(buf)) {
	gpsd_report(LOG_IO, "=> GPS: %s\n", buf);
	return status;
    } else {
	gpsd_report(LOG_WARN, "=> GPS: %s FAILED\n", buf);
	return -1;
    }
}

static void itrax_probe_subtype(struct gps_device_t *session, unsigned int seq)
/* start it reporting */
{
    if (seq == 0) {
	/* initialize GPS clock with current system time */ 
	struct tm when;
	double integral, fractional;
	time_t intfixtime;
	char buf[31], frac[6];
	fractional = modf(timestamp(), &integral);
	intfixtime = (time_t)integral;
	(void)gmtime_r(&intfixtime, &when);
	/* FIXME: so what if my local clock is wrong? */
	(void)strftime(buf, sizeof(buf), "$PFST,INITAID,%H%M%S.XX,%d%m%y\r\n", &when);
	(void)snprintf(frac, sizeof(frac), "%.2f", fractional);
	buf[21] = frac[2]; buf[22] = frac[3];
	(void)literal_send(session->gpsdata.gps_fd, buf);
	/* maybe this should be considered a reconfiguration? */
	(void)literal_send(session->gpsdata.gps_fd, "$PFST,START\r\n");
    }
}

#ifdef ALLOW_RECONFIGURE
static void itrax_configurator(struct gps_device_t *session, int seq)
/* set synchronous mode */
{
    if (seq == 0) {
	(void)literal_send(session->gpsdata.gps_fd, "$PFST,SYNCMODE,1\r\n");
	(void)literal_send(session->gpsdata.gps_fd, 
		    ITRAX_MODESTRING, session->gpsdata.baudrate);
    }
}
#endif /* ALLOW_RECONFIGURE */

static bool itrax_speed(struct gps_device_t *session, speed_t speed)
/* change the baud rate */
{
#ifdef ALLOW_RECONFIGURE
    return literal_send(session->gpsdata.gps_fd, ITRAX_MODESTRING, speed) >= 0;
#else
    return false;
#endif /* ALLOW_RECONFIGURE */
}

static bool itrax_rate(struct gps_device_t *session, double rate)
/* change the sample rate of the GPS */
{
#ifdef ALLOW_RECONFIGURE
    return literal_send(session->gpsdata.gps_fd, "$PSFT,FIXRATE,%d\r\n", rate) >= 0;
#else
    return false;
#endif /* ALLOW_RECONFIGURE */
}

static void itrax_wrap(struct gps_device_t *session)
/* stop navigation, this cuts the power drain */
{
#ifdef ALLOW_RECONFIGURE
    (void)literal_send(session->gpsdata.gps_fd, "$PFST,SYNCMODE,0\r\n");
#endif /* ALLOW_RECONFIGURE */
    (void)literal_send(session->gpsdata.gps_fd, "$PFST,STOP\r\n");
}

/*@ -redef @*/
static struct gps_type_t itrax = {
    .typename      = "iTrax",		/* full name of type */
    .trigger       = "$PFST,OK",	/* tells us to switch to Itrax */
    .channels      = 12,		/* consumer-grade GPS */
    .probe_wakeup  = NULL,		/* no wakeup to be done before hunt */
    .probe_detect  = NULL,		/* no probe */
    .probe_subtype = itrax_probe_subtype,	/* initialize */
#ifdef ALLOW_RECONFIGURE
    .configurator  = itrax_configurator,/* set synchronous mode */
#endif /* ALLOW_RECONFIGURE */
    .get_packet    = generic_get,	/* how to get a packet */
    .parse_packet  = nmea_parse_input,	/* how to interpret a packet */
    .rtcm_writer   = NULL,		/* iTrax doesn't support DGPS/WAAS/EGNOS */
    .speed_switcher= itrax_speed,	/* no speed switcher */
    .mode_switcher = NULL,		/* no mode switcher */
    .rate_switcher = itrax_rate,	/* there's a sample-rate switcher */
    .cycle_chars   = 438,		/* not relevant, no rate switch */
#ifdef ALLOW_RECONFIGURE
    .revert         = NULL,		/* no setting-reversion method */
#endif /* ALLOW_RECONFIGURE */
    .wrapup         = itrax_wrap,	/* sleep the receiver */
    .cycle          = 1,		/* updates every second */
};
/*@ -redef @*/
#endif /* ITRAX_ENABLE */
#endif /* NMEA_ENABLE */

#ifdef TNT_ENABLE
/**************************************************************************
 * True North Technologies - Revolution 2X Digital compass
 *
 * More info: http://www.tntc.com/
 *
 * This is a digital compass which uses magnetometers to measure the
 * strength of the earth's magnetic field. Based on these measurements
 * it provides a compass heading using NMEA formatted output strings.
 * This is useful to supplement the heading provided by another GPS
 * unit. A GPS heading is unreliable at slow speed or no speed.
 *
 **************************************************************************/

enum {
#include "packet_states.h"
};

static void tnt_add_checksum(char *sentence)
{
    unsigned char sum = '\0';
    char c, *p = sentence;

    if (*p == '@') {
	p++;
    } else {
        gpsd_report(LOG_ERROR, "Bad TNT sentence: '%s'\n", sentence);
    }
    while ( ((c = *p) != '*') && (c != '\0')) {
	sum ^= c;
	p++;
    }
    *p++ = '*';
    /*@i@*/snprintf(p, 4, "%02X\r\n", sum);
}

static int tnt_send(int fd, const char *fmt, ... )
{
    int status;
    char buf[BUFSIZ];
    va_list ap;

    va_start(ap, fmt) ;
    (void)vsnprintf(buf, sizeof(buf)-5, fmt, ap);
    va_end(ap);
    strlcat(buf, "*", BUFSIZ);
    tnt_add_checksum(buf);
    status = (int)write(fd, buf, strlen(buf));
    tcdrain(fd);
    if (status == (int)strlen(buf)) {
	gpsd_report(LOG_IO, "=> GPS: %s\n", buf);
	return status;
    } else {
	gpsd_report(LOG_WARN, "=> GPS: %s FAILED\n", buf);
	return -1;
    }
}

#define TNT_SNIFF_RETRIES       100
/*
 * The True North compass won't start talking
 * unless you ask it to. So to identify it we
 * need to query for it's ID string.
 */
static int tnt_packet_sniff(struct gps_device_t *session)
{
    unsigned int n, count = 0;

    gpsd_report(LOG_RAW, "tnt_packet_sniff begins\n");
    for (n = 0; n < TNT_SNIFF_RETRIES; n++) 
    {
      count = 0;
      (void)tnt_send(session->gpsdata.gps_fd, "@X?");
      if (ioctl(session->gpsdata.gps_fd, FIONREAD, &count) < 0)
          return BAD_PACKET;
      if (count == 0) {
          //int delay = 10000000000.0 / session->gpsdata.baudrate;
          //gpsd_report(LOG_RAW, "usleep(%d)\n", delay);
          //usleep(delay);
          gpsd_report(LOG_RAW, "sleep(1)\n");
          (void)sleep(1);
      } else if (generic_get(session) >= 0) {
        if((session->packet.type == NMEA_PACKET)&&(session->packet.state == NMEA_RECOGNIZED))
        {
          gpsd_report(LOG_RAW, "tnt_packet_sniff returns %d\n",session->packet.type);
          return session->packet.type;
        }
      }
    }

    gpsd_report(LOG_RAW, "tnt_packet_sniff found no packet\n");
    return BAD_PACKET;
}

static void tnt_probe_subtype(struct gps_device_t *session, unsigned int seq UNUSED)
{
  // Send codes to start the flow of data
  //tnt_send(session->gpsdata.gps_fd, "@BA?"); // Query current rate
  //tnt_send(session->gpsdata.gps_fd, "@BA=8"); // Start HTM packet at 1Hz
  /*
   * Sending this twice seems to make it more reliable!!
   * I think it gets the input on the unit synced up.
   */
  (void)tnt_send(session->gpsdata.gps_fd, "@BA=15"); // Start HTM packet at 1200 per minute
  (void)tnt_send(session->gpsdata.gps_fd, "@BA=15"); // Start HTM packet at 1200 per minute
}

static bool tnt_probe(struct gps_device_t *session)
{
  unsigned int *ip;
#ifdef FIXED_PORT_SPEED
    /* just the one fixed port speed... */
    static unsigned int rates[] = {FIXED_PORT_SPEED};
#else /* FIXED_PORT_SPEED not defined */
  /* The supported baud rates */
  static unsigned int rates[] = {38400, 19200, 2400, 4800, 9600 };
#endif /* FIXED_PORT_SPEED defined */

  gpsd_report(LOG_PROG, "Probing TrueNorth Compass\n");

  /*
   * Only block until we get at least one character, whatever the
   * third arg of read(2) says.
   */
  /*@ ignore @*/
  memset(session->ttyset.c_cc,0,sizeof(session->ttyset.c_cc));
  session->ttyset.c_cc[VMIN] = 1;
  /*@ end @*/

  session->ttyset.c_cflag &= ~(PARENB | PARODD | CRTSCTS);
  session->ttyset.c_cflag |= CREAD | CLOCAL;
  session->ttyset.c_iflag = session->ttyset.c_oflag = session->ttyset.c_lflag = (tcflag_t) 0;

  session->baudindex = 0;
  for (ip = rates; ip < rates + sizeof(rates)/sizeof(rates[0]); ip++)
      if (ip == rates || *ip != rates[0])
      {
          gpsd_report(LOG_PROG, "hunting at speed %d\n", *ip);
          gpsd_set_speed(session, *ip, 'N',1);
          if (tnt_packet_sniff(session) != BAD_PACKET)
              return true;
      }
  return false;
}

struct gps_type_t trueNorth = {
    .typename       = "True North",	/* full name of type */
    .trigger        = " TNT1500",
    .channels       = 0,		/* not an actual GPS at all */
    .probe_wakeup   = NULL,		/* this will become a real method */
    .probe_detect   = tnt_probe,	/* probe by sending ID query */
    .probe_subtype  = tnt_probe_subtype,/* probe for True North Digital Compass */
#ifdef ALLOW_RECONFIGURE
    .configurator   = NULL,		/* no setting changes */
#endif /* ALLOW_RECONFIGURE */
    .get_packet     = generic_get,	/* how to get a packet */
    .parse_packet   = nmea_parse_input,	/* how to interpret a packet */
    .rtcm_writer    = NULL,	        /* Don't send */
    .speed_switcher = NULL,		/* no speed switcher */
    .mode_switcher  = NULL,		/* no mode switcher */
    .rate_switcher  = NULL,		/* no wrapup */
    .cycle_chars    = -1,		/* not relevant, no rate switch */
#ifdef ALLOW_RECONFIGURE
    .revert         = NULL,		/* no setting-reversion method */
#endif /* ALLOW_RECONFIGURE */
    .wrapup         = NULL,		/* no wrapup */
    .cycle          = 20,		/* updates per second */
};
#endif
#ifdef RTCM104_ENABLE
/**************************************************************************
 *
 * RTCM-104, used for broadcasting DGPS corrections and by DGPS radios
 *
 **************************************************************************/

static gps_mask_t rtcm104_analyze(struct gps_device_t *session)
{
    rtcm_unpack(&session->gpsdata.rtcm, (char *)session->packet.isgps.buf);
    gpsd_report(LOG_RAW, "RTCM packet type 0x%02x length %d words: %s\n", 
		session->gpsdata.rtcm.type,
		session->gpsdata.rtcm.length+2,
		gpsd_hexdump(session->packet.isgps.buf, (session->gpsdata.rtcm.length+2)*sizeof(isgps30bits_t)));
    return RTCM_SET;
}

static struct gps_type_t rtcm104 = {
    .typename      = "RTCM104",		/* full name of type */
    .trigger       = NULL,		/* no recognition string */
    .channels      = 0,			/* not used */
    .probe_wakeup  = NULL,		/* no wakeup to be done before hunt */
    .probe_detect  = NULL,		/* no probe */
    .probe_subtype = NULL,		/* no subtypes */
#ifdef ALLOW_RECONFIGURE
    .configurator  = NULL,		/* no configurator */
#endif /* ALLOW_RECONFIGURE */
    .get_packet    = generic_get,	/* how to get a packet */
    .parse_packet  = rtcm104_analyze,	/*  */
    .rtcm_writer   = NULL,		/* don't send RTCM data,  */
    .speed_switcher= NULL,		/* no speed switcher */
    .mode_switcher = NULL,		/* no mode switcher */
    .rate_switcher = NULL,		/* no sample-rate switcher */
    .cycle_chars   = -1,		/* not relevant, no rate switch */
#ifdef ALLOW_RECONFIGURE
    .revert         = NULL,		/* no setting-reversion method */
#endif /* ALLOW_RECONFIGURE */
    .wrapup         = NULL,		/* no wrapup code */
    .cycle          = 1,		/* updates every second */
};
#endif /* RTCM104_ENABLE */

extern struct gps_type_t garmin_usb_binary, garmin_ser_binary;
extern struct gps_type_t sirf_binary, tsip_binary;
extern struct gps_type_t evermore_binary, italk_binary;

/*@ -nullassign @*/
/* the point of this rigamarole is to not have to export a table size */
static struct gps_type_t *gpsd_driver_array[] = {
#ifdef NMEA_ENABLE
    &nmea, 
#ifdef FV18_ENABLE
    &fv18,
#endif /* FV18_ENABLE */
#ifdef GARMIN_ENABLE
    &garmin,
#endif /* GARMIN_ENABLE */
#ifdef TRIPMATE_ENABLE
    &tripmate,
#endif /* TRIPMATE_ENABLE */
#ifdef EARTHMATE_ENABLE
    &earthmate, 
#endif /* EARTHMATE_ENABLE */
#ifdef ITRAX_ENABLE
    &itrax, 
#endif /* ITRAX_ENABLE */
#endif /* NMEA_ENABLE */
#ifdef ZODIAC_ENABLE
    &zodiac_binary,
#endif /* ZODIAC_ENABLE */
#ifdef GARMIN_ENABLE
    &garmin_usb_binary,
    &garmin_ser_binary,
#endif /* GARMIN_ENABLE */
#ifdef SIRF_ENABLE
    &sirf_binary, 
#endif /* SIRF_ENABLE */
#ifdef TSIP_ENABLE
    &tsip_binary, 
#endif /* TSIP_ENABLE */
#ifdef TNT_ENABLE
    &trueNorth,
#endif /* TSIP_ENABLE */
#ifdef EVERMORE_ENABLE
    &evermore_binary, 
#endif /* EVERMORE_ENABLE */
#ifdef ITALK_ENABLE
    &italk_binary, 
#endif /* ITALK_ENABLE */
#ifdef RTCM104_ENABLE
    &rtcm104, 
#endif /* RTCM104_ENABLE */
    NULL,
};
/*@ +nullassign @*/
struct gps_type_t **gpsd_drivers = &gpsd_driver_array[0];
