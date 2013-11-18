/*
 * This file is Copyright (c) 2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#ifndef S_SPLINT_S
#include <unistd.h>
#endif /* S_SPLINT_S */

#include "gpsd.h"
#include "bits.h"		/* for getbeu16(), to extract big-endian words */

extern const struct gps_type_t zodiac_binary;
extern const struct gps_type_t ubx_binary;
extern const struct gps_type_t sirf_binary;
extern const struct gps_type_t nmea2000;

ssize_t generic_get(struct gps_device_t *session)
{
    return packet_get(session->gpsdata.gps_fd, &session->packet);
}

gps_mask_t generic_parse_input(struct gps_device_t *session)
{
    const struct gps_type_t **dp;

    if (session->packet.type == COMMENT_PACKET) {
	gpsd_set_century(session);
	return 0;
#ifdef NMEA_ENABLE
    } else if (session->packet.type == NMEA_PACKET) {
	gps_mask_t st = 0;
	char *sentence = (char *)session->packet.outbuffer;

	if (sentence[strlen(sentence)-1] != '\n')
	    gpsd_report(LOG_IO, "<= GPS: %s\n", sentence);
	else
	    gpsd_report(LOG_IO, "<= GPS: %s", sentence);

	if ((st=nmea_parse(sentence, session)) == 0) {
	    gpsd_report(LOG_WARN, "unknown sentence: \"%s\"\n",	sentence);
	}
	for (dp = gpsd_drivers; *dp; dp++) {
	    char *trigger = (*dp)->trigger;

	    if (trigger!=NULL && strncmp(sentence,trigger,strlen(trigger))==0) {
		gpsd_report(LOG_PROG, "found trigger string %s.\n", trigger);
		if (*dp != session->device_type) {
		    (void)gpsd_switch_driver(session, (*dp)->type_name);
		    if (session->device_type != NULL
			&& session->device_type->event_hook != NULL)
			session->device_type->event_hook(session,
							 event_triggermatch);
		    st |= DEVICEID_SET;
		}
	    }
	}
	return st;
#endif /* NMEA_ENABLE */
    } else {
	for (dp = gpsd_drivers; *dp; dp++) {
	    if (session->packet.type == (*dp)->packet_type) {
		(void)gpsd_switch_driver(session, (*dp)->type_name);
		return (*dp)->parse_packet(session);
	    }
	}
	return 0;
    }
}

/**************************************************************************
 *
 * Generic driver -- make no assumptions about the device type
 *
 **************************************************************************/

/* *INDENT-OFF* */
const struct gps_type_t unknown = {
    .type_name      = "Unknown",	/* full name of type */
    .packet_type    = COMMENT_PACKET,	/* associated lexer packet type */
    .flags	    = DRIVER_NOFLAGS,	/* no flags set */
    .trigger	    = NULL,		/* it's the default */
    .channels       = 12,		/* consumer-grade GPS */
    .probe_detect   = NULL,		/* no probe */
    .get_packet     = generic_get,	/* use generic packet getter */
    .parse_packet   = generic_parse_input,	/* how to interpret a packet */
    .rtcm_writer    = NULL,		/* write RTCM data straight */
    .event_hook     = NULL,		/* lifetime event handler */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher = NULL,		/* no speed switcher */
    .mode_switcher  = NULL,		/* no mode switcher */
    .rate_switcher  = NULL,		/* no sample-rate switcher */
    .min_cycle      = 1,		/* not relevant, no rate switch */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send   = NULL,		/* how to send control strings */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = NULL,		/* no method for NTP fudge factor */
#endif /* NTPSHM_ ENABLE */
};
/* *INDENT-ON* */

#ifdef NMEA_ENABLE
/**************************************************************************
 *
 * NMEA 0183
 *
 * This is separate from the 'unknown' driver because we don't want to
 * ship NMEA subtype probe strings to a device until we've seen at
 * least one NMEA packet.  This avoids spamming devices that might
 * actually be USB modems or other things in USB device class FF that
 * just happen to have one of 'our' adaptor chips in pront of them.
 *
 **************************************************************************/

static void nmea_event_hook(struct gps_device_t *session, event_t event)
{
    if (session->context->readonly)
	return;
    /*
     * This is where we try to tickle NMEA devices into revealing their
     * inner natures.
     */
    if (event == event_configure) {
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
	switch (session->packet.counter) {
#ifdef NMEA_ENABLE
	case 0:
	    /* probe for Garmin serial GPS -- expect $PGRMC followed by data */
	    gpsd_report(LOG_PROG, "=> Probing for Garmin NMEA\n");
	    (void)nmea_send(session, "$PGRMCE");
	    break;
#endif /* NMEA_ENABLE */
#ifdef SIRF_ENABLE
	case 1:
	    /*
	     * We used to try to probe for SiRF by issuing "$PSRF105,1"
	     * and expecting "$Ack Input105.".  But it turns out this
	     * only works for SiRF-IIs; SiRF-I and SiRF-III don't respond.
	     * Thus the only reliable probe is to try to flip the SiRF into
	     * binary mode, cluing in the library to revert it on close.
	     *
	     * SiRFs dominate the GPS-mouse market, so we used to put this test
	     * first. Unfortunately this causes problems for gpsctl, as it cannot
	     * select the NMEA driver without switching the device back to
	     * binary mode!  Fix this if we ever find a nondisruptive probe string.
	     */
	    gpsd_report(LOG_PROG, "=> Probing for SiRF\n");
	    (void)nmea_send(session,
			    "$PSRF100,0,%d,%d,%d,0",
			    session->gpsdata.dev.baudrate,
			    9 - session->gpsdata.dev.stopbits,
			    session->gpsdata.dev.stopbits);
	    session->back_to_nmea = true;
	    break;
#endif /* SIRF_ENABLE */
#ifdef NMEA_ENABLE
	case 2:
	    /* probe for the FV-18 -- expect $PFEC,GPint followed by data */
	    gpsd_report(LOG_PROG, "=> Probing for FV-18\n");
	    (void)nmea_send(session, "$PFEC,GPint");
	    break;
	case 3:
	    /* probe for the Trimble Copernicus */
	    gpsd_report(LOG_PROG, "=> Probing for Trimble Copernicus\n");
	    (void)nmea_send(session, "$PTNLSNM,0139,01");
	    break;
#endif /* NMEA_ENABLE */
#ifdef EVERMORE_ENABLE
	case 4:
	    gpsd_report(LOG_PROG, "=> Probing for Evermore\n");
	    /* Enable checksum and GGA(1s), GLL(0s), GSA(1s), GSV(1s), RMC(1s), VTG(0s), PEMT101(1s) */
	    /* EverMore will reply with: \x10\x02\x04\x38\x8E\xC6\x10\x03 */
	    (void)gpsd_write(session,
			     "\x10\x02\x12\x8E\x7F\x01\x01\x00\x01\x01\x01\x00\x01\x00\x00\x00\x00\x00\x00\x13\x10\x03",
			     22);
	    break;
#endif /* EVERMORE_ENABLE */
#ifdef GPSCLOCK_ENABLE
	case 5:
	    /* probe for Furuno Electric GH-79L4-N (GPSClock); expect $PFEC,GPssd */
	    gpsd_report(LOG_PROG, "=> Probing for GPSClock\n");
	    (void)nmea_send(session, "$PFEC,GPsrq");
	    break;
#endif /* GPSCLOCK_ENABLE */
#ifdef ASHTECH_ENABLE
	case 6:
	    /* probe for Ashtech -- expect $PASHR,RID */
	    gpsd_report(LOG_PROG, "=> Probing for Ashtech\n");
	    (void)nmea_send(session, "$PASHQ,RID");
	    break;
#endif /* ASHTECH_ENABLE */
#ifdef UBX_ENABLE
	case 7:
	    /* probe for UBX -- query software version */
	    gpsd_report(LOG_PROG, "=> Probing for UBX\n");
	    (void)ubx_write(session, 0x0au, 0x04, NULL, 0);
	    break;
#endif /* UBX_ENABLE */
#ifdef MTK3301_ENABLE
	case 8:
	    /* probe for MTK-3301 -- expect $PMTK705 */
	    gpsd_report(LOG_PROG, "=> Probing for MediaTek\n");
	    (void)nmea_send(session, "$PMTK605");
	    break;
#endif /* MTK3301_ENABLE */
	default:
	    break;
	}
    }
}

#if defined(RECONFIGURE_ENABLE) && defined(BINARY_ENABLE)
static void nmea_mode_switch(struct gps_device_t *session, int mode)
{
    /*
     * If the daemon has seen this device in a binary mode, we may
     * actually know how to switch back.
     */
    if (mode == MODE_BINARY)
    {
	const struct gps_type_t **dp;

	/*@-shiftnegative@*/
	for (dp = gpsd_drivers; *dp; dp++) {
	    if ((*dp)->packet_type > 0 && (*dp)->packet_type != session->packet.type &&
	    	    (session->observed & PACKET_TYPEMASK((*dp)->packet_type))!=0) {
		(*dp)->mode_switcher(session, mode);
		break;
	    }
	}
	/*@+shiftnegative@*/
    }
}
#endif /* defined(RECONFIGURE_ENABLE) && defined(BINARY_ENABLE) */

/* *INDENT-OFF* */
const struct gps_type_t nmea = {
    .type_name      = "Generic NMEA",	/* full name of type */
    .packet_type    = NMEA_PACKET,	/* associated lexer packet type */
    .flags	    = DRIVER_NOFLAGS,	/* no flags set */
    .trigger	    = NULL,		/* it's the default */
    .channels       = 12,		/* consumer-grade GPS */
    .probe_detect   = NULL,		/* no probe */
    .get_packet     = generic_get,	/* use generic packet getter */
    .parse_packet   = generic_parse_input,	/* how to interpret a packet */
    .rtcm_writer    = gpsd_write,	/* write RTCM data straight */
    .event_hook     = nmea_event_hook,	/* lifetime event handler */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher = NULL,		/* no speed switcher */
#ifdef BINARY_ENABLE
    .mode_switcher  = nmea_mode_switch,	/* maybe switchable if it was a SiRF */
#else
    .mode_switcher  = NULL,		/* no binary mode to revert to */
#endif /* BINARY_ENABLE */
    .rate_switcher  = NULL,		/* no sample-rate switcher */
    .min_cycle      = 1,		/* not relevant, no rate switch */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send   = nmea_write,	/* how to send control strings */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = NULL,		/* no method for NTP fudge factor */
#endif /* NTPSHM_ ENABLE */
};
/* *INDENT-ON* */

#if defined(GARMIN_ENABLE) && defined(NMEA_ENABLE)
/**************************************************************************
 *
 * Garmin NMEA
 *
 **************************************************************************/

#ifdef RECONFIGURE_ENABLE
static void garmin_mode_switch(struct gps_device_t *session, int mode)
/* only does anything in one direction, going to Garmin binary driver */
{
    if (mode == MODE_BINARY) {
	(void)nmea_send(session, "$PGRMC1,1,2,1,,,,2,W,N");
	(void)nmea_send(session, "$PGRMI,,,,,,,R");
	(void)usleep(333);	/* standard Garmin settling time */
	session->gpsdata.dev.driver_mode = MODE_BINARY;
    }
}
#endif /* RECONFIGURE_ENABLE */

static void garmin_nmea_event_hook(struct gps_device_t *session,
				   event_t event)
{
    if (session->context->readonly)
	return;

    if (event == event_driver_switch) {
	/* forces a reconfigure as the following packets come in */
	session->packet.counter = 0;
    }
    if (event == event_configure) {
	/*
	 * And here's that reconfigure.  It's split up like this because
	 * receivers like the Garmin GPS-10 don't handle having having a lot of
	 * probes shoved at them very well.
	 */
	switch (session->packet.counter) {
	case 0:
	    /* reset some config, AutoFix, WGS84, PPS
	     * Set the PPS pulse length to 40ms which leaves the Garmin 18-5hz
	     * with a 160ms low state.
	     * NOTE: new PPS only takes effect after next power cycle
	     */
	    (void)nmea_send(session, "$PGRMC,A,,100,,,,,,A,,1,2,1,30");
	    break;
	case 1:
	    /* once a sec, no averaging, NMEA 2.3, WAAS */
	    (void)nmea_send(session, "$PGRMC1,1,1,1,,,,2,W,N");
	    break;
	case 2:
	    /* get some more config info */
	    (void)nmea_send(session, "$PGRMC1E");
	    break;
	case 3:
	    /* turn off all output except GGA */
	    (void)nmea_send(session, "$PGRMO,,2");
	    (void)nmea_send(session, "$PGRMO,GPGGA,1");
	    break;
	case 4:
	    /* enable GPGGA, GPGSA, GPGSV, GPRMC on Garmin serial GPS */
	    (void)nmea_send(session, "$PGRMO,GPGSA,1");
	    break;
	case 5:
	    (void)nmea_send(session, "$PGRMO,GPGSV,1");
	    break;
	case 6:
	    (void)nmea_send(session, "$PGRMO,GPRMC,1");
	    break;
	case 7:
	    (void)nmea_send(session, "$PGRMO,PGRME,1");
	    break;
	}
    }
}

/* *INDENT-OFF* */
const struct gps_type_t garmin = {
    .type_name      = "Garmin NMEA",	/* full name of type */
    .packet_type    = NMEA_PACKET,	/* associated lexer packet type */
    .flags	    = DRIVER_NOFLAGS,	/* no flags set */
    .trigger	    = "$PGRMC,",	/* Garmin private */
    .channels       = 12,		/* not used by this driver */
    .probe_detect   = NULL,		/* no probe */
    .get_packet     = generic_get,	/* use generic packet getter */
    .parse_packet   = generic_parse_input,	/* how to interpret a packet */
    .rtcm_writer    = NULL,		/* some do, some don't, skip for now */
    .event_hook     = garmin_nmea_event_hook,	/* lifetime event handler */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher = NULL,			/* no speed switcher */
    .mode_switcher  = garmin_mode_switch,	/* mode switcher */
    .rate_switcher  = NULL,		/* no sample-rate switcher */
    .min_cycle      = 1,		/* not relevant, no rate switch */
#endif /*RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send   = nmea_write,	/* how to send control strings */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = NULL,		/* no method for NTP fudge factor */
#endif /* NTPSHM_ ENABLE */
};
/* *INDENT-ON* */
#endif /* GARMIN_ENABLE && NMEA_ENABLE */

#ifdef ASHTECH_ENABLE
/**************************************************************************
 *
 * Ashtech (then Thales, now Magellan Professional) Receivers
 *
 **************************************************************************/

static void ashtech_event_hook(struct gps_device_t *session, event_t event)
{
    if (session->context->readonly)
	return;

    if (event == event_wakeup)
	(void)nmea_send(session, "$PASHQ,RID");
    if (event == event_identified) {
	/* turn WAAS on. can't hurt... */
	(void)nmea_send(session, "$PASHS,WAS,ON");
	/* reset to known output state */
	(void)nmea_send(session, "$PASHS,NME,ALL,A,OFF");
	/* then turn on some useful sentences */
#ifdef __future__
	/* we could parse these, but they're oversize so they get dropped */
	(void)nmea_send(session, "$PASHS,NME,POS,A,ON");
	(void)nmea_send(session, "$PASHS,NME,SAT,A,ON");
#else
	(void)nmea_send(session, "$PASHS,NME,GGA,A,ON");
	(void)nmea_send(session, "$PASHS,NME,GSA,A,ON");
	(void)nmea_send(session, "$PASHS,NME,GSV,A,ON");
	(void)nmea_send(session, "$PASHS,NME,RMC,A,ON");
#endif
	(void)nmea_send(session, "$PASHS,NME,ZDA,A,ON");
    }
}

/* *INDENT-OFF* */
const struct gps_type_t ashtech = {
    .type_name      = "Ashtech",	/* full name of type */
    .packet_type    = NMEA_PACKET,	/* associated lexer packet type */
    .flags	    = DRIVER_NOFLAGS,	/* no flags set */
    .trigger	    = "$PASHR,RID,",	/* Ashtech receivers respond thus */
    .channels       = 24,		/* not used, GG24 has 24 channels */
    .probe_detect   = NULL,		/* no probe */
    .get_packet     = generic_get,	/* how to get a packet */
    .parse_packet   = generic_parse_input,	/* how to interpret a packet */
    .rtcm_writer    = gpsd_write,	/* write RTCM data straight */
    .event_hook     = ashtech_event_hook, /* lifetime event handler */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher = NULL,		/* no speed switcher */
    .mode_switcher  = NULL,		/* no mode switcher */
    .rate_switcher  = NULL,		/* no sample-rate switcher */
    .min_cycle      = 1,		/* not relevant, no rate switch */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send   = nmea_write,	/* how to send control strings */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = NULL,		/* no method for NTP fudge factor */
#endif /* NTPSHM_ ENABLE */
};
/* *INDENT-ON* */
#endif /* ASHTECH_ENABLE */

#ifdef FV18_ENABLE
/**************************************************************************
 *
 * FV18 -- uses 2 stop bits, needs to be told to send GSAs
 *
 **************************************************************************/

static void fv18_event_hook(struct gps_device_t *session, event_t event)
{
    if (session->context->readonly)
	return;

    /*
     * Tell an FV18 to send GSAs so we'll know if 3D is accurate.
     * Suppress GLL and VTG.  Enable ZDA so dates will be accurate for replay.
     * It's possible we might not need to redo this on event_reactivate,
     * but doing so is safe and cheap.
     */
    if (event == event_identified || event == event_reactivate)
	(void)nmea_send(session,
			"$PFEC,GPint,GSA01,DTM00,ZDA01,RMC01,GLL00,VTG00,GSV05");
}

/* *INDENT-OFF* */
const struct gps_type_t fv18 = {
    .type_name      = "San Jose Navigation FV18",	/* full name of type */
    .packet_type    = NMEA_PACKET,	/* associated lexer packet type */
    .flags	    = DRIVER_NOFLAGS,	/* no flags set */
    .trigger	    = "$PFEC,GPint,",	/* FV18s should echo the probe */
    .channels       = 12,		/* not used by this driver */
    .probe_detect   = NULL,		/* no probe */
    .get_packet     = generic_get,	/* how to get a packet */
    .parse_packet   = generic_parse_input,	/* how to interpret a packet */
    .rtcm_writer    = gpsd_write,	/* write RTCM data straight */
    .event_hook     = fv18_event_hook,	/* lifetime event handler */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher = NULL,		/* no speed switcher */
    .mode_switcher  = NULL,		/* no mode switcher */
    .rate_switcher  = NULL,		/* no sample-rate switcher */
    .min_cycle      = 1,		/* not relevant, no rate switch */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send   = nmea_write,	/* how to send control strings */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = NULL,		/* no method for NTP fudge factor */
#endif /* NTPSHM_ ENABLE */
};
/* *INDENT-ON* */
#endif /* FV18_ENABLE */

#ifdef GPSCLOCK_ENABLE
/**************************************************************************
 *
 * Furuno Electric GPSClock (GH-79L4)
 *
 **************************************************************************/

/*
 * Based on http://www.tecsys.de/fileadmin/user_upload/pdf/gh79_1an_intant.pdf
 */

static void gpsclock_event_hook(struct gps_device_t *session, event_t event)
{
    if (session->context->readonly)
	return;
    /*
     * Michael St. Laurent <mikes@hartwellcorp.com> reports that you have to
     * ignore the trailing PPS edge when extracting time from this chip.
     */
    if (event == event_identified || event == event_reactivate) {
	gpsd_report(LOG_INF, "PPS trailing edge will be ignored\n");
	session->driver.nmea.ignore_trailing_edge = true;
    }
}

/* *INDENT-OFF* */
const struct gps_type_t gpsclock = {
    .type_name      = "Furuno Electric GH-79L4",	/* full name of type */
    .packet_type    = NMEA_PACKET,	/* associated lexer packet type */
    .flags	    = DRIVER_NOFLAGS,	/* no flags set */
    .trigger	    = "$PFEC,GPssd",	/* GPSclock should return this */
    .channels       = 12,		/* not used by this driver */
    .probe_detect   = NULL,		/* no probe */
    .get_packet     = generic_get,	/* how to get a packet */
    .parse_packet   = generic_parse_input,	/* how to interpret a packet */
    .rtcm_writer    = gpsd_write,	/* write RTCM data straight */
    .event_hook     = gpsclock_event_hook,	/* lifetime event handler */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher = NULL,		/* no speed switcher */
    .mode_switcher  = NULL,		/* no mode switcher */
    .rate_switcher  = NULL,		/* sample rate is fixed */
    .min_cycle      = 1,		/* sample rate is fixed */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send   = nmea_write,	/* how to send control strings */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = NULL,		/* no method for NTP fudge factor */
#endif /* NTPSHM_ ENABLE */
};
/* *INDENT-ON* */
#endif /* GPSCLOCK_ENABLE */

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

static void tripmate_event_hook(struct gps_device_t *session, event_t event)
{
    if (session->context->readonly)
	return;
    /* TripMate requires this response to the ASTRAL it sends at boot time */
    if (event == event_identified)
	(void)nmea_send(session, "$IIGPQ,ASTRAL");
    /* stop it sending PRWIZCH */
    if (event == event_identified || event == event_reactivate)
	(void)nmea_send(session, "$PRWIILOG,ZCH,V,,");
}

/* *INDENT-OFF* */
static const struct gps_type_t tripmate = {
    .type_name     = "Delorme TripMate",	/* full name of type */
    .packet_type   = NMEA_PACKET,		/* lexer packet type */
    .flags	   = DRIVER_NOFLAGS,		/* no rollover or other flags */
    .trigger       ="ASTRAL",			/* tells us to switch */
    .channels      = 12,			/* consumer-grade GPS */
    .probe_detect  = NULL,			/* no probe */
    .get_packet    = generic_get,		/* how to get a packet */
    .parse_packet  = generic_parse_input,		/* how to interpret a packet */
    .rtcm_writer   = gpsd_write,			/* send RTCM data straight */
    .event_hook    = tripmate_event_hook,	/* lifetime event handler */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher= NULL,			/* no speed switcher */
    .mode_switcher = NULL,			/* no mode switcher */
    .rate_switcher = NULL,			/* no sample-rate switcher */
    .min_cycle     = 1,				/* no rate switch */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send  = nmea_write,	/* how to send control strings */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = NULL,		/* no method for NTP fudge factor */
#endif /* NTPSHM_ ENABLE */
};
/* *INDENT-ON* */
#endif /* TRIPMATE_ENABLE */

#ifdef EARTHMATE_ENABLE
/**************************************************************************
 *
 * Zodiac EarthMate textual mode
 *
 * Note: This is the pre-2003 version using Zodiac binary protocol.
 * There is a good HOWTO at <http://www.hamhud.net/ka9mva/earthmate.htm>.
 * It has been replaced with a design that uses a SiRF chipset.
 *
 **************************************************************************/

static void earthmate_event_hook(struct gps_device_t *session, event_t event)
{
    if (session->context->readonly)
	return;
    if (event == event_triggermatch) {
	(void)gpsd_write(session, "EARTHA\r\n", 8);
	(void)usleep(10000);
	(void)gpsd_switch_driver(session, "Zodiac Binary");
    }
}

/*@ -redef @*/
/* *INDENT-OFF* */
static const struct gps_type_t earthmate = {
    .type_name     = "Delorme EarthMate (pre-2003, Zodiac chipset)",
    .packet_type   = NMEA_PACKET,	/* associated lexer packet type */
    .flags	   = DRIVER_NOFLAGS,		/* no rollover or other flags */
    .trigger       = "EARTHA",			/* Earthmate trigger string */
    .channels      = 12,			/* not used by NMEA parser */
    .probe_detect  = NULL,			/* no probe */
    .get_packet    = generic_get,		/* how to get a packet */
    .parse_packet  = generic_parse_input,		/* how to interpret a packet */
    .rtcm_writer   = NULL,			/* don't send RTCM data */
    .event_hook    = earthmate_event_hook,	/* lifetime event handler */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher= NULL,			/* no speed switcher */
    .mode_switcher = NULL,			/* no mode switcher */
    .rate_switcher = NULL,			/* no sample-rate switcher */
    .min_cycle     = 1,				/* no rate switch */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send  = nmea_write,	/* how to send control strings */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = NULL,		/* no method for NTP fudge factor */
#endif /* NTPSHM_ ENABLE */
};
/*@ -redef @*/
/* *INDENT-ON* */
#endif /* EARTHMATE_ENABLE */

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

static void tnt_add_checksum(char *sentence)
/* add NMEA-style CRC checksum to a command */
{
    unsigned char sum = '\0';
    char c, *p = sentence;

    if (*p == '@') {
	p++;
    } else {
	gpsd_report(LOG_ERROR, "Bad TNT sentence: '%s'\n", sentence);
    }
    while (((c = *p) != '\0')) {
	sum ^= c;
	p++;
    }
    (void)snprintf(p, 6, "*%02X\r\n", (unsigned int)sum);
}


static ssize_t tnt_control_send(struct gps_device_t *session,
				char *msg, size_t len UNUSED)
/* send a control string in TNT native formal */
{
    ssize_t status;

    tnt_add_checksum(msg);
    status = write(session->gpsdata.gps_fd, msg, strlen(msg));
    (void)tcdrain(session->gpsdata.gps_fd);
    return status;
}

static bool tnt_send(struct gps_device_t *session, const char *fmt, ...)
/* printf(3)-like TNT command generator */
{
    char buf[BUFSIZ];
    va_list ap;
    ssize_t sent;

    va_start(ap, fmt);
    (void)vsnprintf(buf, sizeof(buf) - 5, fmt, ap);
    va_end(ap);
    sent = tnt_control_send(session, buf, strlen(buf));
    if (sent == (ssize_t) strlen(buf)) {
	gpsd_report(LOG_IO, "=> GPS: %s\n", buf);
	return true;
    } else {
	gpsd_report(LOG_WARN, "=> GPS: %s FAILED\n", buf);
	return false;
    }
}

#ifdef RECONFIGURE_ENABLE
static bool tnt_speed(struct gps_device_t *session,
		      speed_t speed, char parity UNUSED, int stopbits UNUSED)
{
    /*
     * Baud rate change followed by device reset.
     * See page 40 of Technical Guide 1555-B.  We need:
     * 2400 -> 1, 4800 -> 2, 9600 -> 3, 19200 -> 4, 38400 -> 5
     */
    unsigned int val = speed / 2400u;	/* 2400->1, 4800->2, 9600->4, 19200->8...  */
    unsigned int i = 0;

    /* fast way to compute log2(val) */
    while ((val >> i) > 1)
	++i;
    return tnt_send(session, "@B6=%d", i + 1)
	&& tnt_send(session, "@F28.6=1");
}
#endif /* RECONFIGURE_ENABLE */

static void tnt_event_hook(struct gps_device_t *session, event_t event)
/* TNT lifetime event hook */
{
    if (session->context->readonly)
	return;
    if (event == event_wakeup) {
	(void)tnt_send(session, "@F0.3=1");	/* set run mode */
	(void)tnt_send(session, "@F2.2=1");	/* report in degrees */
    }
}

/* *INDENT-OFF* */
const struct gps_type_t trueNorth = {
    .type_name      = "True North",	/* full name of type */
    .packet_type    = NMEA_PACKET,	/* associated lexer packet type */
    .flags	    = DRIVER_NOFLAGS,	/* no flags set */
    .trigger	    = "$PTNTHTM",	/* their proprietary sentence */
    .channels       = 0,		/* not an actual GPS at all */
    .probe_detect   = NULL,		/* no probe in run mode */
    .get_packet     = generic_get,	/* how to get a packet */
    .parse_packet   = generic_parse_input,	/* how to interpret a packet */
    .rtcm_writer    = NULL,		/* Don't send */
    .event_hook     = tnt_event_hook,	/* lifetime event handler */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher = tnt_speed,	/* no speed switcher */
    .mode_switcher  = NULL,		/* no mode switcher */
    .rate_switcher  = NULL,		/* no wrapup */
    .min_cycle      = 0.5,		/* fixed at 20 samples per second */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send   = tnt_control_send,	/* how to send control strings */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = NULL,
#endif /* NTPSHM_ ENABLE */
};
/* *INDENT-ON* */
#endif

#ifdef OCEANSERVER_ENABLE
/**************************************************************************
 * OceanServer - Digital Compass, OS5000 Series
 *
 * More info: http://www.ocean-server.com/download/OS5000_Compass_Manual.pdf
 *
 * This is a digital compass which uses magnetometers to measure the
 * strength of the earth's magnetic field. Based on these measurements
 * it provides a compass heading using NMEA formatted output strings.
 * This is useful to supplement the heading provided by another GPS
 * unit. A GPS heading is unreliable at slow speed or no speed.
 *
 **************************************************************************/

static int oceanserver_send(int fd, const char *fmt, ...)
{
    int status;
    char buf[BUFSIZ];
    va_list ap;

    va_start(ap, fmt);
    (void)vsnprintf(buf, sizeof(buf) - 5, fmt, ap);
    va_end(ap);
    (void)strlcat(buf, "", BUFSIZ);
    status = (int)write(fd, buf, strlen(buf));
    (void)tcdrain(fd);
    if (status == (int)strlen(buf)) {
	gpsd_report(LOG_IO, "=> GPS: %s\n", buf);
	return status;
    } else {
	gpsd_report(LOG_WARN, "=> GPS: %s FAILED\n", buf);
	return -1;
    }
}

static void oceanserver_event_hook(struct gps_device_t *session,
				   event_t event)
{
    if (session->context->readonly)
	return;
    if (event == event_configure && session->packet.counter == 0) {
	/* report in NMEA format */
	(void)oceanserver_send(session->gpsdata.gps_fd, "2\n");
	/* ship all fields */
	(void)oceanserver_send(session->gpsdata.gps_fd, "X2047");
    }
}

/* *INDENT-OFF* */
static const struct gps_type_t oceanServer = {
    .type_name      = "OceanServer Digital Compass OS5000", /* full name of type */
    .packet_type    = NMEA_PACKET,	/* associated lexer packet type */
    .flags	    = DRIVER_NOFLAGS,	/* no rollover or other flags */
    .trigger	    = "$OHPR,",		/* detect their main sentence */
    .channels       = 0,		/* not an actual GPS at all */
    .probe_detect   = NULL,
    .get_packet     = generic_get,	/* how to get a packet */
    .parse_packet   = generic_parse_input,	/* how to interpret a packet */
    .rtcm_writer    = NULL,		/* Don't send */
    .event_hook     = oceanserver_event_hook,
#ifdef RECONFIGURE_ENABLE
    .speed_switcher = NULL,		/* no speed switcher */
    .mode_switcher  = NULL,		/* no mode switcher */
    .rate_switcher  = NULL,		/* no wrapup */
    .min_cycle      = 1,		/* not relevant, no rate switch */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send   = nmea_write,	/* how to send control strings */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = NULL,
#endif /* NTPSHM_ ENABLE */
};
/* *INDENT-ON* */
#endif

#ifdef FURY_ENABLE
/**************************************************************************
 *
 * Jackson Labs Fury, a high-precision laboratory clock
 *
 * Will also support other Jackon Labs boards, including the Firefly.
 *
 * Note: you must either build with fixed_port_speed=115200 or tweak the
 * speed on the port to 115200 before running.  The device's default mode
 * does not stream output, so our hunt loop will simply time out otherwise.
 *
 **************************************************************************/

static bool fury_rate_switcher(struct gps_device_t *session, double rate)
{
    char buf[78];
    double inverted;

    /* rate is a frequency, but the command takes interval in # of sedconds */
    if (rate == 0.0)
	inverted = 0.0;
    else
	inverted = 1.0/rate;
    if (inverted > 256)
	return false;
    (void)snprintf(buf, sizeof(buf), "GPS:GPGGA %d\r\n", (int)inverted);
    (void)gpsd_write(session, buf, strlen(buf));
    return true;
}

static void fury_event_hook(struct gps_device_t *session, event_t event)
{
    if (event == event_wakeup && gpsd_get_speed(session) == 115200)
	(void)fury_rate_switcher(session, 1.0);
    else if (event == event_deactivate)
	(void)fury_rate_switcher(session, 0.0);
}


/* *INDENT-OFF* */
static const struct gps_type_t fury = {
    .type_name      = "Jackson Labs Fury", /* full name of type */
    .packet_type    = NMEA_PACKET,	/* associated lexer packet type */
    .flags	    = DRIVER_NOFLAGS,	/* no rollover or other flags */
    .trigger	    = NULL,		/* detect their main sentence */
    .channels       = 0,		/* not an actual GPS at all */
    .probe_detect   = NULL,
    .get_packet     = generic_get,	/* how to get a packet */
    .parse_packet   = generic_parse_input,	/* how to interpret a packet */
    .rtcm_writer    = NULL,		/* Don't send */
    .event_hook     = fury_event_hook,
#ifdef RECONFIGURE_ENABLE
    .speed_switcher = NULL,		/* no speed switcher */
    .mode_switcher  = NULL,		/* no mode switcher */
    .rate_switcher  = fury_rate_switcher,
    .min_cycle      = 1,		/* has rate switch */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send   = nmea_write,	/* how to send control strings */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = NULL,
#endif /* NTPSHM_ ENABLE */
};
/* *INDENT-ON* */

#endif /* FURY_ENABLE */

#ifdef RTCM104V2_ENABLE
/**************************************************************************
 *
 * RTCM-104 (v2), used for broadcasting DGPS corrections and by DGPS radios
 *
 **************************************************************************/

static gps_mask_t rtcm104v2_analyze(struct gps_device_t *session)
{
    rtcm2_unpack(&session->gpsdata.rtcm2, (char *)session->packet.isgps.buf);
    /* extra guard prevents expensive hexdump calls */
    if (session->context->debug >= LOG_RAW)
	gpsd_report(LOG_RAW, "RTCM 2.x packet type 0x%02x length %d words from %zd bytes: %s\n",
		    session->gpsdata.rtcm2.type,
		    session->gpsdata.rtcm2.length + 2,
		    session->packet.isgps.buflen,
		    gpsd_hexdump((char *)session->packet.isgps.buf,
				 (session->gpsdata.rtcm2.length +
				  2) * sizeof(isgps30bits_t)));
    session->cycle_end_reliable = true;
    return RTCM2_SET;
}

/* *INDENT-OFF* */
static const struct gps_type_t rtcm104v2 = {
    .type_name     = "RTCM104V2",	/* full name of type */
    .packet_type   = RTCM2_PACKET,	/* associated lexer packet type */
    .flags	   = DRIVER_NOFLAGS,	/* no rollover or other flags */
    .trigger       = NULL,		/* no recognition string */
    .channels      = 0,			/* not used */
    .probe_detect  = NULL,		/* no probe */
    .get_packet    = generic_get,	/* how to get a packet */
    .parse_packet  = rtcm104v2_analyze,	/*  */
    .rtcm_writer   = NULL,		/* don't send RTCM data,  */
    .event_hook    = NULL,		/* no event_hook */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher= NULL,		/* no speed switcher */
    .mode_switcher = NULL,		/* no mode switcher */
    .rate_switcher = NULL,		/* no sample-rate switcher */
    .min_cycle     = 1,			/* not relevant, no rate switch */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send   = nmea_write,	/* how to send control strings */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = NULL,
#endif /* NTPSHM_ ENABLE */
};
/* *INDENT-ON* */
#endif /* RTCM104V2_ENABLE */
#ifdef RTCM104V3_ENABLE
/**************************************************************************
 *
 * RTCM-104 (v3), used for broadcasting DGPS corrections and by DGPS radios
 *
 **************************************************************************/

static gps_mask_t rtcm104v3_analyze(struct gps_device_t *session)
{
    uint16_t type = getbeu16(session->packet.inbuffer, 3) >> 4;

    gpsd_report(LOG_RAW, "RTCM 3.x packet %d\n", type);
    rtcm3_unpack(&session->gpsdata.rtcm3, (char *)session->packet.outbuffer);
    session->cycle_end_reliable = true;
    return RTCM3_SET;
}

/* *INDENT-OFF* */
static const struct gps_type_t rtcm104v3 = {
    .type_name     = "RTCM104V3",	/* full name of type */
    .packet_type   = RTCM3_PACKET,	/* associated lexer packet type */
    .flags	   = DRIVER_NOFLAGS,	/* no rollover or other flags */
    .trigger       = NULL,		/* no recognition string */
    .channels      = 0,			/* not used */
    .probe_detect  = NULL,		/* no probe */
    .get_packet    = generic_get,	/* how to get a packet */
    .parse_packet  = rtcm104v3_analyze,	/*  */
    .rtcm_writer   = NULL,		/* don't send RTCM data,  */
    .event_hook    = NULL,		/* no event hook */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher= NULL,		/* no speed switcher */
    .mode_switcher = NULL,		/* no mode switcher */
    .rate_switcher = NULL,		/* no sample-rate switcher */
    .min_cycle     = 1,			/* not relevant, no rate switch */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send   = nmea_write,	/* how to send control strings */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = NULL,
#endif /* NTPSHM_ ENABLE */
};
/* *INDENT-ON* */
#endif /* RTCM104V3_ENABLE */

#ifdef GARMINTXT_ENABLE
/**************************************************************************
 *
 * Garmin Simple Text protocol
 *
 **************************************************************************/

/* *INDENT-OFF* */
static const struct gps_type_t garmintxt = {
    .type_name     = "Garmin Simple Text",		/* full name of type */
    .packet_type   = GARMINTXT_PACKET,	/* associated lexer packet type */
    .flags	   = DRIVER_NOFLAGS,	/* no rollover or other flags */
    .trigger       = NULL,		/* no recognition string */
    .channels      = 0,			/* not used */
    .probe_detect  = NULL,		/* no probe */
    .get_packet    = generic_get,	/* how to get a packet */
    .parse_packet  = generic_parse_input,	/* how to parse one */
    .rtcm_writer   = NULL,		/* don't send RTCM data,  */
    .event_hook    = NULL,		/* no event hook */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher= NULL,		/* no speed switcher */
    .mode_switcher = NULL,		/* no mode switcher */
    .rate_switcher = NULL,		/* no sample-rate switcher */
    .min_cycle     = 1,			/* not relevant, no rate switch */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send   = nmea_write,	/* how to send control strings */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = NULL,
#endif /* NTPSHM_ ENABLE */
};
/* *INDENT-ON* */
#endif /* GARMINTXT_ENABLE */

#ifdef MTK3301_ENABLE
/**************************************************************************
 *
 * MediaTek MTK-3301
 *
 * OEMs for several GPS vendors, notably including Garmin and FasTrax.
 * Website at <http://www.mediatek.com/>.
 *
 **************************************************************************/

static gps_mask_t processMTK3301(struct gps_device_t *session)
{
    gps_mask_t mask;

    /* try a straight NMEA parse, this will set up fields */
    mask = generic_parse_input(session);

    if (session->packet.type == NMEA_PACKET
	&& strncmp(session->driver.nmea.field[0], "PMTK", 4) == 0)
    {
	int msg, reason;

	msg = atoi(&(session->driver.nmea.field[0])[4]);
	switch (msg) {
	case 705:			/*  */
	    (void)strlcat(session->subtype, session->driver.nmea.field[1], sizeof(session->subtype));
	    (void)strlcat(session->subtype, "-", sizeof(session->subtype));
	    (void)strlcat(session->subtype, session->driver.nmea.field[2], sizeof(session->subtype));
	    return ONLINE_SET;
	case 001:			/* ACK / NACK */
	    reason = atoi(session->driver.nmea.field[2]);
	    if (atoi(session->driver.nmea.field[1]) == -1)
		gpsd_report(LOG_WARN, "MTK NACK: unknown sentence\n");
	    else if (reason < 3) {
		const char *mtk_reasons[] = {
		    "Invalid",
		    "Unsupported",
		    "Valid but Failed",
		    "Valid success"
		};
		gpsd_report(LOG_WARN, "MTK NACK: %s, reason: %s\n", session->driver.nmea.field[1],
			    mtk_reasons[reason]);
	    }
	    else
		gpsd_report(LOG_WARN, "MTK ACK: %s\n", session->driver.nmea.field[1]);
	    break;
	default:
	    return ONLINE_SET;		/* ignore */
	}
    }

    return mask;
}

static void mtk3301_event_hook(struct gps_device_t *session, event_t event)
{
/*
0  NMEA_SEN_GLL,  GPGLL   interval - Geographic Position - Latitude longitude
1  NMEA_SEN_RMC,  GPRMC   interval - Recommended Minimum Specific GNSS Sentence
2  NMEA_SEN_VTG,  GPVTG   interval - Course Over Ground and Ground Speed
3  NMEA_SEN_GGA,  GPGGA   interval - GPS Fix Data
4  NMEA_SEN_GSA,  GPGSA   interval - GNSS DOPS and Active Satellites
5  NMEA_SEN_GSV,  GPGSV   interval - GNSS Satellites in View
6  NMEA_SEN_GRS,  GPGRS   interval - GNSS Range Residuals
7  NMEA_SEN_GST,  GPGST   interval - GNSS Pseudorange Errors Statistics
13 NMEA_SEN_MALM, PMTKALM interval - GPS almanac information
14 NMEA_SEN_MEPH, PMTKEPH interval - GPS ephemeris information
15 NMEA_SEN_MDGP, PMTKDGP interval - GPS differential correction information
16 NMEA_SEN_MDBG, PMTKDBG interval – MTK debug information
17 NMEA_SEN_ZDA,  GPZDA   interval - Time & Date
18 NMEA_SEN_MCHN, PMTKCHN interval – GPS channel status

"$PMTK314,1,1,1,1,1,5,1,1,0,0,0,0,0,0,0,0,0,1,0"

*/
    if (session->context->readonly)
	return;
    /* FIX-ME: Do we need to resend this on reactivation? */
    if (event == event_identified) {
	(void)nmea_send(session, "$PMTK320,0");	/* power save off */
	(void)nmea_send(session, "$PMTK300,1000,0,0,0.0,0.0");	/* Fix interval */
	(void)nmea_send(session,
			"$PMTK314,0,1,0,1,1,5,1,1,0,0,0,0,0,0,0,0,0,1,0");
	(void)nmea_send(session, "$PMTK301,2");	/* DGPS is WAAS */
	(void)nmea_send(session, "$PMTK313,1");	/* SBAS enable */
    }
}

#ifdef RECONFIGURE_ENABLE
static bool mtk3301_rate_switcher(struct gps_device_t *session, double rate)
{
    char buf[78];

    /*@i1@*/ unsigned int milliseconds = 1000 * rate;
    if (rate > 1)
	milliseconds = 1000;
    else if (rate < 0.2)
	milliseconds = 200;

    (void)snprintf(buf, sizeof(buf), "$PMTK300,%u,0,0,0,0", milliseconds);
    (void)nmea_send(session, buf);	/* Fix interval */
    return true;
}
#endif /* RECONFIGURE_ENABLE */

/* *INDENT-OFF* */
const struct gps_type_t mtk3301 = {
    .type_name      = "MTK-3301",	/* full name of type */
    .packet_type    = NMEA_PACKET,	/* associated lexer packet type */
    .flags	    = DRIVER_NOFLAGS,	/* no flags set */
    .trigger	    = "$PMTK705,",	/* firmware release name and version */
    .channels       = 12,		/* not used by this driver */
    .probe_detect   = NULL,		/* no probe */
    .get_packet     = generic_get,	/* how to get a packet */
    .parse_packet   = processMTK3301,	/* how to interpret a packet */
    .rtcm_writer    = gpsd_write,	/* write RTCM data straight */
    .event_hook     = mtk3301_event_hook,	/* lifetime event handler */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher = NULL,		/* no speed switcher */
    .mode_switcher  = NULL,		/* no mode switcher */
    .rate_switcher  = mtk3301_rate_switcher,		/* sample rate switcher */
    .min_cycle      = 0.2,		/* max 5Hz */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send   = nmea_write,	/* how to send control strings */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = NULL,
#endif /* NTPSHM_ ENABLE */
};
/* *INDENT-ON* */
#endif /* MTK3301_ENABLE */

#ifdef AIVDM_ENABLE
/**************************************************************************
 *
 * AIVDM - ASCII armoring of binary AIS packets
 *
 **************************************************************************/

/*@ -fixedformalarray -usedef -branchstate @*/
static bool aivdm_decode(const char *buf, size_t buflen,
		  struct aivdm_context_t ais_contexts[AIVDM_CHANNELS],
		  struct ais_t *ais,
		  int debug)
{
#ifdef __UNUSED_DEBUG__
    char *sixbits[64] = {
	"000000", "000001", "000010", "000011", "000100",
	"000101", "000110", "000111", "001000", "001001",
	"001010", "001011", "001100", "001101", "001110",
	"001111", "010000", "010001", "010010", "010011",
	"010100", "010101", "010110", "010111", "011000",
	"011001", "011010", "011011", "011100", "011101",
	"011110", "011111", "100000", "100001", "100010",
	"100011", "100100", "100101", "100110", "100111",
	"101000", "101001", "101010", "101011", "101100",
	"101101", "101110", "101111", "110000", "110001",
	"110010", "110011", "110100", "110101", "110110",
	"110111", "111000", "111001", "111010", "111011",
	"111100", "111101", "111110", "111111",
    };
#endif /* __UNUSED_DEBUG__ */
    int nfrags, ifrag, nfields = 0;
    unsigned char *field[NMEA_MAX*2];
    unsigned char fieldcopy[NMEA_MAX*2+1];
    unsigned char *data, *cp;
    unsigned char ch, pad;
    struct aivdm_context_t *ais_context;
    int i;

    if (buflen == 0)
	return false;

    /* we may need to dump the raw packet */
    gpsd_report(LOG_PROG, "AIVDM packet length %zd: %s\n", buflen, buf);

    /* first clear the result, making sure we don't return garbage */
    memset(ais, 0, sizeof(*ais));

    /* discard overlong sentences */
    if (strlen(buf) > sizeof(fieldcopy)-1) {
	gpsd_report(LOG_ERROR, "overlong AIVDM packet.\n");
	return false;
    }

    /* extract packet fields */
    (void)strlcpy((char *)fieldcopy, buf, sizeof(fieldcopy));
    field[nfields++] = (unsigned char *)buf;
    for (cp = fieldcopy;
	 cp < fieldcopy + buflen; cp++)
	if (*cp == (unsigned char)',') {
	    *cp = '\0';
	    field[nfields++] = cp + 1;
	}

    /* discard sentences with exiguous commas; catches run-ons */
    if (nfields < 7) {
	gpsd_report(LOG_ERROR, "malformed AIVDM packet.\n");
	return false;
    }

    switch (field[4][0]) {
    /* FIXME: if fields[4] == "12", it doesn't detect the error */
    case '\0':
	/*
	 * Apparently an empty channel is normal for AIVDO sentences,
	 * which makes sense as they don't come in over radio.  This
	 * is going to break if there's ever an AIVDO type 24, though.
	 */
	if (strncmp((const char *)field[0], "!AIVDO", 6) != 0)
	    gpsd_report(LOG_ERROR, "invalid empty AIS channel. Assuming 'A'\n");
	ais_context = &ais_contexts[0];
	break;
    case '1':
	/*@fallthrough@*/
    case 'A':
	ais_context = &ais_contexts[0];
	break;
    case '2':
	/*@fallthrough@*/
    case 'B':
	ais_context = &ais_contexts[1];
	break;
    default:
	gpsd_report(LOG_ERROR, "invalid AIS channel 0x%0X .\n", field[4][0]);
	return false;
    }

    nfrags = atoi((char *)field[1]); /* number of fragments to expect */
    ifrag = atoi((char *)field[2]); /* fragment id */
    data = field[5];
    pad = field[6][0]; /* number of padding bits */
    gpsd_report(LOG_PROG, "nfrags=%d, ifrag=%d, decoded_frags=%d, data=%s\n",
		nfrags, ifrag, ais_context->decoded_frags, data);

    /* assemble the binary data */

    /* check fragment ordering */
    if (ifrag != ais_context->decoded_frags + 1) {
	gpsd_report(LOG_ERROR, "invalid fragment #%d received, expected #%d.\n",
	                       ifrag, ais_context->decoded_frags + 1);
	if (ifrag != 1)
	    return false;
        /* else, ifrag==1: Just discard all that was previously decoded and
         * simply handle that packet */
        ais_context->decoded_frags = 0;
    }
    if (ifrag == 1) {
	(void)memset(ais_context->bits, '\0', sizeof(ais_context->bits));
	ais_context->bitlen = 0;
    }

    /* wacky 6-bit encoding, shades of FIELDATA */
    /*@ +charint @*/
    for (cp = data; cp < data + strlen((char *)data); cp++) {
	ch = *cp;
	ch -= 48;
	if (ch >= 40)
	    ch -= 8;
#ifdef __UNUSED_DEBUG__
	gpsd_report(LOG_RAW, "%c: %s\n", *cp, sixbits[ch]);
#endif /* __UNUSED_DEBUG__ */
	/*@ -shiftnegative @*/
	for (i = 5; i >= 0; i--) {
	    if ((ch >> i) & 0x01) {
		ais_context->bits[ais_context->bitlen / 8] |=
		    (1 << (7 - ais_context->bitlen % 8));
	    }
	    ais_context->bitlen++;
	    if (ais_context->bitlen > sizeof(ais_context->bits)) {
		gpsd_report(LOG_INF, "overlong AIVDM payload truncated.\n");
		return false;
	    }
	}
	/*@ +shiftnegative @*/
    }
    if (isdigit(pad))
	ais_context->bitlen -= (pad - '0');	/* ASCII assumption */
    /*@ -charint @*/

    /* time to pass buffered-up data to where it's actually processed? */
    if (ifrag == nfrags) {
	if (debug >= LOG_INF) {
	    size_t clen = (ais_context->bitlen + 7) / 8;
	    gpsd_report(LOG_INF, "AIVDM payload is %zd bits, %zd chars: %s\n",
			ais_context->bitlen, clen,
			gpsd_hexdump((char *)ais_context->bits, clen));
	}

        /* clear waiting fragments count */
        ais_context->decoded_frags = 0;

	/* decode the assembled binary packet */
	return ais_binary_decode(ais,
				 ais_context->bits,
				 ais_context->bitlen,
				 &ais_context->type24_queue);
    }

    /* we're still waiting on another sentence */
    ais_context->decoded_frags++;
    return false;
}
/*@ +fixedformalarray +usedef +branchstate @*/

static void aivdm_event_hook(struct gps_device_t *session, event_t event)
{
    if (event == event_configure)
	/*@i1@*/session->aivdm->type24_queue.index = 0;
}

static gps_mask_t aivdm_analyze(struct gps_device_t *session)
{
    if (session->packet.type == AIVDM_PACKET) {
	if (aivdm_decode
	    ((char *)session->packet.outbuffer, session->packet.outbuflen,
	     session->aivdm, &session->gpsdata.ais, session->context->debug)) {
	    return ONLINE_SET | AIS_SET;
	} else
	    return ONLINE_SET;
#ifdef NMEA_ENABLE
    } else if (session->packet.type == NMEA_PACKET) {
	return nmea_parse((char *)session->packet.outbuffer, session);
#endif /* NMEA_ENABLE */
    } else
	return 0;
}

/* *INDENT-OFF* */
static const struct gps_type_t aivdm = {
    /* Full name of type */
    .type_name        = "AIVDM",    	/* associated lexer packet type */
    .packet_type      = AIVDM_PACKET,	/* numeric packet type */
    .flags	      = DRIVER_NOFLAGS,	/* no rollover or other flags */
    .trigger          = NULL,		/* identifying response */
    .channels         = 0,		/* not used by this driver */
    .probe_detect     = NULL,		/* no probe */
    .get_packet       = generic_get,	/* how to get a packet */
    .parse_packet     = aivdm_analyze,	/* how to analyze a packet */
    .rtcm_writer      = NULL,		/* don't send RTCM data,  */
    .event_hook       = aivdm_event_hook,/* lifetime event handler */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher   = NULL,		/* no speed switcher */
    .mode_switcher    = NULL,		/* no mode switcher */
    .rate_switcher    = NULL,		/* no rate switcher */
    .min_cycle        = 1,		/* max 1Hz */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send     = NULL,		/* no control sender */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = NULL,		/* no NTP communication */
#endif /* NTPSHM_ ENABLE */
};
/* *INDENT-ON* */
#endif /* AIVDM_ENABLE */

#ifdef PASSTHROUGH_ENABLE
/**************************************************************************
 *
 * JSON passthrough driver
 *
 **************************************************************************/

static void path_rewrite(struct gps_device_t *session, char *prefix)
/* prepend the session path to the value of a specified attribute */
{
    /*
     * Hack the packet to reflect its origin.  This code is supposed
     * to insert the path naming the remote gpsd instance into the
     * baginning of the path attribute, followed by a # to separate it
     * from the device.
     */
    char *prefloc;
    for (prefloc = (char *)session->packet.outbuffer;
	 prefloc < (char *)session->packet.outbuffer+session->packet.outbuflen;
	 prefloc++)
	if (strncmp(prefloc, prefix, strlen(prefix)) == 0) {
	    char copy[sizeof(session->packet.outbuffer)];
	    (void)strlcpy(copy,
			  (char *)session->packet.outbuffer,
			  sizeof(copy));
	    prefloc += strlen(prefix);
	    (void)strlcpy(prefloc,
			  session->gpsdata.dev.path,
			  sizeof(session->gpsdata.dev.path));
	    (void)strlcat((char *)session->packet.outbuffer, "#",
			  sizeof(session->packet.outbuffer));
	    (void)strlcat((char *)session->packet.outbuffer,
			  copy + (prefloc-(char *)session->packet.outbuffer),
			  sizeof(session->packet.outbuffer));
	}
    session->packet.outbuflen = strlen((char *)session->packet.outbuffer);
}

static gps_mask_t json_pass_packet(struct gps_device_t *session UNUSED)
{
    gpsd_report(LOG_IO, "<= GPS: %s\n", (char *)session->packet.outbuffer);

    /*@-nullpass@*/ /* required only because splint is buggy */
    /* devices and paths need to be edited to */
    if (strstr((char *)session->packet.outbuffer, "DEVICE") != NULL)
	path_rewrite(session, "\"path\":\"");
    path_rewrite(session, "\"device\":\"");

    /* mark certain responses without a path or device attribute */
    if (strstr((char *)session->packet.outbuffer, "VERSION") != NULL
	|| strstr((char *)session->packet.outbuffer, "WATCH") != NULL
	|| strstr((char *)session->packet.outbuffer, "DEVICES") != NULL) {
	session->packet.outbuffer[session->packet.outbuflen-1] = '\0';
	(void)strlcat((char *)session->packet.outbuffer, ",\"remote\":\"",
		      sizeof(session->packet.outbuffer));
	(void)strlcat((char *)session->packet.outbuffer,
		      session->gpsdata.dev.path,
		      sizeof(session->packet.outbuffer));
	(void)strlcat((char *)session->packet.outbuffer, "\"}",
		      sizeof(session->packet.outbuffer));
    }

    gpsd_report (LOG_PROG,
		 "JSON, passing through %s\n",
		 (char *)session->packet.outbuffer);
    /*@-nullpass@*/
    return PASSTHROUGH_IS;
}

/* *INDENT-OFF* */
const struct gps_type_t json_passthrough = {
    .type_name      = "JSON slave driver",	/* full name of type */
    .packet_type    = JSON_PACKET,	/* associated lexer packet type */
    .flags	    = DRIVER_NOFLAGS,	/* no flags set */
    .trigger	    = NULL,		/* it's the default */
    .channels       = 0,		/* not used */
    .probe_detect   = NULL,		/* no probe */
    .get_packet     = generic_get,	/* use generic packet getter */
    .parse_packet   = json_pass_packet,	/* how to interpret a packet */
    .rtcm_writer    = NULL,		/* write RTCM data straight */
    .event_hook     = NULL,		/* lifetime event handler */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher = NULL,		/* no speed switcher */
    .mode_switcher  = NULL,		/* no mode switcher */
    .rate_switcher  = NULL,		/* no sample-rate switcher */
    .min_cycle      = 1,		/* not relevant, no rate switch */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send   = NULL,		/* how to send control strings */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = NULL,		/* no method for NTP fudge factor */
#endif /* NTPSHM_ ENABLE */
};
/* *INDENT-ON* */

#endif /* PASSTHROUGH_ENABLE */

extern const struct gps_type_t garmin_usb_binary, garmin_ser_binary;
extern const struct gps_type_t geostar_binary;
extern const struct gps_type_t tsip_binary, oncore_binary;
extern const struct gps_type_t evermore_binary, italk_binary;
extern const struct gps_type_t navcom_binary, superstar2_binary;

/*@ -nullassign @*/
/* the point of this rigamarole is to not have to export a table size */
static const struct gps_type_t *gpsd_driver_array[] = {
    &unknown,
#ifdef NMEA_ENABLE
    &nmea,
#ifdef ASHTECH_ENABLE
    &ashtech,
#endif /* ASHTECHV18_ENABLE */
#ifdef TRIPMATE_ENABLE
    &tripmate,
#endif /* TRIPMATE_ENABLE */
#ifdef EARTHMATE_ENABLE
    &earthmate,
#endif /* EARTHMATE_ENABLE */
#ifdef GPSCLOCK_ENABLE
    &gpsclock,
#endif /* GPSCLOCK_ENABLE */
#ifdef GARMIN_ENABLE
    &garmin,
#endif /* GARMIN_ENABLE */
#ifdef MTK3301_ENABLE
    &mtk3301,
#endif /*  MTK3301_ENABLE */
#ifdef OCEANSERVER_ENABLE
    &oceanServer,
#endif /* OCEANSERVER_ENABLE */
#ifdef FV18_ENABLE
    &fv18,
#endif /* FV18_ENABLE */
#ifdef TNT_ENABLE
    &trueNorth,
#endif /* TNT_ENABLE */
#ifdef FURY_ENABLE
    &fury,
#endif /* FURY_ENABLE */
#ifdef AIVDM_ENABLE
    &aivdm,
#endif /* AIVDM_ENABLE */
#endif /* NMEA_ENABLE */

#ifdef EVERMORE_ENABLE
    &evermore_binary,
#endif /* EVERMORE_ENABLE */
#ifdef GARMIN_ENABLE
    /* be sure to try Garmin Serial Binary before Garmin USB Binary */
    &garmin_ser_binary,
    &garmin_usb_binary,
#endif /* GARMIN_ENABLE */
#ifdef GEOSTAR_ENABLE
    &geostar_binary,
#endif /* GEOSTAR_ENABLE */
#ifdef ITRAX_ENABLE
    &italk_binary,
#endif /* ITRAX_ENABLE */
#ifdef ONCORE_ENABLE
    &oncore_binary,
#endif /* ONCORE_ENABLE */
#ifdef NAVCOM_ENABLE
    &navcom_binary,
#endif /* NAVCOM_ENABLE */
#ifdef SIRF_ENABLE
    &sirf_binary,
#endif /* SIRF_ENABLE */
#ifdef SUPERSTAR2_ENABLE
    &superstar2_binary,
#endif /* SUPERSTAR2_ENABLE */
#ifdef TSIP_ENABLE
    &tsip_binary,
#endif /* TSIP_ENABLE */
#ifdef UBX_ENABLE
    &ubx_binary,
#endif /* UBX_ENABLE */
#ifdef ZODIAC_ENABLE
    &zodiac_binary,
#endif /* ZODIAC_ENABLE */

#ifdef NMEA2000_ENABLE
    &nmea2000,
#endif /* NMEA2000_ENABLE */

#ifdef RTCM104V2_ENABLE
    &rtcm104v2,
#endif /* RTCM104V2_ENABLE */
#ifdef RTCM104V3_ENABLE
    &rtcm104v3,
#endif /* RTCM104V3_ENABLE */
#ifdef GARMINTXT_ENABLE
    &garmintxt,
#endif /* GARMINTXT_ENABLE */

#ifdef PASSTHROUGH_ENABLE
    &json_passthrough,
#endif /* PASSTHROUGH_ENABLE */

    NULL,
};

/*@ +nullassign @*/
const struct gps_type_t **gpsd_drivers = &gpsd_driver_array[0];
