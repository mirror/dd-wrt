/*
 * A prototype driver.  Doesn't run, doesn't even compile.
 *
 * For new driver authors: replace "_PROTO_" and "_proto_" with the name of
 * your new driver. That will give you a skeleton with all the required
 * functions defined.
 *
 * Once that is done, you will likely have to define a large number of
 * flags and masks. From there, you will be able to start extracting
 * useful quantities. There are roughed-in decoders for the navigation
 * solution, satellite status and gps-utc offset. These are the 3 key
 * messages that gpsd needs. Some protocols transmit error estimates
 * separately from the navigation solution; if developing a driver for
 * such a protocol you will need to add a decoder function for that
 * message. Be extra careful when using sizeof(<type>) to extract part
 * of packets (ie. don't do it). This idiom creates portability problems
 * between 32 and 64 bit systems.
 *
 * For anyone hacking this driver skeleton: "_PROTO_" and "_proto_" are now
 * reserved tokens. We suggest that they only ever be used as prefixes,
 * but if they are used infix, they must be used in a way that allows a
 * driver author to find-and-replace to create a unique namespace for
 * driver functions.
 *
 * If using vi, ":%s/_PROTO_/MYDRIVER/g" and ":%s/_proto_/mydriver/g"
 * should produce a source file that comes very close to being useful.
 * You will also need to add hooks for your new driver to:
 * SConstruct
 * drivers.c
 * gpsd.h-tail
 * libgpsd_core.c
 * packet.c
 * packet_states.h
 *
 * This file is Copyright (c) 2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#if defined(_PROTO__ENABLE) && defined(BINARY_ENABLE)

#include "bits.h"

static	gps_mask_t _proto__parse_input(struct gps_device_t *);
static	gps_mask_t _proto__dispatch(struct gps_device_t *, unsigned char *, size_t );
static	gps_mask_t _proto__msg_navsol(struct gps_device_t *, unsigned char *, size_t );
static	gps_mask_t _proto__msg_utctime(struct gps_device_t *, unsigned char *, size_t );
static	gps_mask_t _proto__msg_svinfo(struct gps_device_t *, unsigned char *, size_t );
static	gps_mask_t _proto__msg_raw(struct gps_device_t *, unsigned char *, size_t );

/*
 * These methods may be called elsewhere in gpsd
 */
static	ssize_t _proto__control_send(struct gps_device_t *, char *, size_t);
static	bool _proto__probe_detect(struct gps_device_t *);
static	void _proto__event_hook(struct gps_device_t *, event_t);
static	bool _proto__set_speed(struct gps_device_t *, speed_t, char, int);
static	void _proto__set_mode(struct gps_device_t *, int);

/*
 * Decode the navigation solution message
 */
static gps_mask_t
_proto__msg_navsol(struct gps_device_t *session, unsigned char *buf, size_t data_len)
{
    gps_mask_t mask;
    int flags;
    double Px, Py, Pz, Vx, Vy, Vz;

    if (data_len != _PROTO__NAVSOL_MSG_LEN)
	return 0;

    gpsd_report(LOG_IO, "_proto_ NAVSOL - navigation data\n");
    /* if this protocol has a way to test message validity, use it */
    flags = GET_FLAGS();
    if ((flags & _PROTO__SOLUTION_VALID) == 0)
	return 0;

    mask = ONLINE_SET;

    /* extract ECEF navigation solution here */
    /* or extract the local tangential plane (ENU) solution */
    [Px, Py, Pz, Vx, Vy, Vz] = GET_ECEF_FIX();
    ecef_to_wgs84fix(&session->newdata,  &session->gpsdata.separation,
		     Px, Py, Pz, Vx, Vy, Vz);
    mask |= LATLON_SET | ALTITUDE_SET | SPEED_SET | TRACK_SET | CLIMB_SET  ;

    session->newdata.epx = GET_LONGITUDE_ERROR();
    session->newdata.epy = GET_LATITUDE_ERROR();
    session->newdata.eps = GET_SPEED_ERROR();
    session->gpsdata.satellites_used = GET_SATELLITES_USED();
    /*
     * Do *not* clear DOPs in a navigation solution message;
     * instead, opportunistically pick up whatever it gives
     * us and replace whatever values we computed from the
     * visibility matrix for he last skyview. The reason to trust
     * the chip returns over what we compute is that some
     * chips have internal deweighting albums to throw out sats
     * that increase DOP.
     */
    session->gpsdata.dop.hdop = GET_HDOP();
    session->gpsdata.dop.vdop = GET_VDOP();
    /* other DOP if available */
    mask |= DOP_SET;

    session->newdata.mode = GET_FIX_MODE();
    session->gpsdata.status = GET_FIX_STATUS();

    /*
     * Mix in CLEAR_IS to clue the daemon in about when to clear fix
     * information.  Mix in REPORT_IS when the sentence is reliably
     * the last in a reporting cycle.
     */
    mask |= MODE_SET | STATUS_SET | REPORT_IS;

    /*
     * At the end of each packet-cracking function, report at LOG_DATA level
     * the fields it potentially set and the transfer mask. Doing this
     * makes it relatively easy to track down data-management problems.
     */
    gpsd_report(LOG_DATA, "NAVSOL: time=%.2f, lat=%.2f lon=%.2f alt=%.2f mode=%d status=%d\n",
		session->newdata.time,
		session->newdata.latitude,
		session->newdata.longitude,
		session->newdata.altitude,
		session->newdata.mode,
		session->gpsdata.status);

    return mask;
}

/**
 * GPS Leap Seconds
 */
static gps_mask_t
_proto__msg_utctime(struct gps_device_t *session, unsigned char *buf, size_t data_len)
{
    double t;

    if (data_len != UTCTIME_MSG_LEN)
	return 0;

    gpsd_report(LOG_IO, "_proto_ UTCTIME - navigation data\n");
    /* if this protocol has a way to test message validity, use it */
    flags = GET_FLAGS();
    if ((flags & _PROTO__TIME_VALID) == 0)
	return 0;

    tow = GET_MS_TIMEOFWEEK();
    gps_week = GET_WEEKNUMBER();
    session->context->leap_seconds = GET_GPS_LEAPSECONDS();
    session->newdata.time = gpsd_gpstime_resolve(session, gps_week, tow / 1000.0);

    return TIME_SET | PPSTIME_IS | ONLINE_SET;
}

/**
 * GPS Satellite Info
 */
static gps_mask_t
_proto__msg_svinfo(struct gps_device_t *session, unsigned char *buf, size_t data_len)
{
    unsigned char i, st, nchan, nsv;
    unsigned int tow;

    if (data_len != SVINFO_MSG_LEN )
	return 0;

    gpsd_report(LOG_IO, "_proto_ SVINFO - navigation data\n");
    /* if this protocol has a way to test message validity, use it */
    flags = GET_FLAGS();
    if ((flags & _PROTO__SVINFO_VALID) == 0)
	return 0;

    /*
     * some protocols have a variable length message listing only visible
     * satellites, even if there are less than the number of channels. others
     * have a fixed length message and send empty records for idle channels
     * that are not tracking or searching. whatever the case, nchan should
     * be set to the number of satellites which might be visible.
     */
    nchan = GET_NUMBER_OF_CHANNELS();
    if ((nchan < 1) || (nchan > MAXCHANNELS)) {
	gpsd_report(LOG_INF, "too many channels reported\n");
	return 0;
    }
    gpsd_zero_satellites(&session->gpsdata);
    nsv = 0; /* number of actually used satellites */
    for (i = st = 0; i < nchan; i++) {
	/* get info for one channel/satellite */
	int off = GET_CHANNEL_STATUS(i);

	session->gpsdata.PRN[i]		= PRN_THIS_CHANNEL_IS_TRACKING(i);
	session->gpsdata.ss[i]		= (float)SIGNAL_STRENGTH_FOR_CHANNEL(i);
	session->gpsdata.elevation[i]	= SV_ELEVATION_FOR_CHANNEL(i);
	session->gpsdata.azimuth[i]	= SV_AZIMUTH_FOR_CHANNEL(i);

	if (CHANNEL_USED_IN_SOLUTION(i))
	    session->gpsdata.used[nsv++] = session->gpsdata.PRN[i];

	if(session->gpsdata.PRN[i])
		st++;
    }
    /* if the satellite-info setence gives you UTC time, use it */
    session->gpsdata.skyview_time = NaN;
    session->gpsdata.satellites_used = nsv;
    session->gpsdata.satellites_visible = st;
    gpsd_report(LOG_DATA,
		"SVINFO: visible=%d used=%d mask={SATELLITE|USED}\n",
		session->gpsdata.satellites_visible,
		session->gpsdata.satellites_used);
    return SATELLITE_SET | USED_IS;
}

/**
 * Raw measurements
 */
static gps_mask_t
_proto__msg_raw(struct gps_device_t *session, unsigned char *buf, size_t data_len)
{
    unsigned char i, st, nchan, nsv;
    unsigned int tow;

    if (data_len != RAW_MSG_LEN )
	return 0;

    gpsd_report(LOG_IO, "_proto_ RAW - raw measurements\n");
    /* if this protocol has a way to test message validity, use it */
    flags = GET_FLAGS();
    if ((flags & _PROTO__SVINFO_VALID) == 0)
	return 0;

    /*
     * not all chipsets emit the same information. some of these observables
     * can be easily converted into others. these are suggestions for the
     * quantities you may wish to try extract. chipset documentation may say
     * something like "this message contains information required to generate
     * a RINEX file." assign NAN for unavailable data.
     */
    nchan = GET_NUMBER_OF_CHANNELS();
    if ((nchan < 1) || (nchan > MAXCHANNELS)) {
	gpsd_report(LOG_INF, "too many channels reported\n");
	return 0;
    }

    for (i = 0; i < n; i++){
	session->gpsdata.PRN[i] = GET_PRN();
	session->gpsdata.ss[i] = GET_SIGNAL()
	session->gpsdata.raw.satstat[i] = GET_FLAGS();
	session->gpsdata.raw.pseudorange[i] = GET_PSEUDORANGE();
	session->gpsdata.raw.doppler[i] = GET_DOPPLER();
	session->gpsdata.raw.carrierphase[i] = GET_CARRIER_PHASE();
	session->gpsdata.raw.mtime[i] = GET_MEASUREMENT_TIME();
	session->gpsdata.raw.codephase[i] = GET_CODE_PHASE();
	session->gpsdata.raw.deltarange[i] = GET_DELTA_RANGE();
    }
    return RAW_IS;
}

/**
 * Parse the data from the device
 */
/*@ +charint @*/
gps_mask_t _proto__dispatch(struct gps_device_t *session, unsigned char *buf, size_t len)
{
    size_t i;
    int type, used, visible, retmask = 0;

    if (len == 0)
	return 0;

    /*
     * Set this if the driver reliably signals end of cycle.
     * The core library zeroes it just before it calls each driver's
     * packet analyzer.
     */
    session->cycle_end_reliable = true;
    if (msgid == MY_START_OF_CYCLE)
	retmask |= CLEAR_IS;
    else if (msgid == MY_END_OF_CYCLE)
	retmask |= REPORT_IS;

    type = GET_MESSAGE_TYPE();

    /* we may need to dump the raw packet */
    gpsd_report(LOG_RAW, "raw _proto_ packet type 0x%02x\n", type);

   /*
    * The tag field is only 8 bytes; be careful you do not overflow.
    * Using an abbreviation (eg. "italk" -> "itk") may be useful.
    */
    (void)snprintf(session->gpsdata.tag, sizeof(session->gpsdata.tag),
	"_PROTO_%02x", type);

    switch (type)
    {
	/* Deliver message to specific decoder based on message type */

    default:
	gpsd_report(LOG_WARN, "unknown packet id %d length %d\n", type, len);
	return 0;
    }
}
/*@ -charint @*/

/**********************************************************
 *
 * Externally called routines below here
 *
 **********************************************************/

static bool _proto__probe_detect(struct gps_device_t *session)
{
   /*
    * This method is used to elicit a positively identifying
    * response from a candidate device. Some drivers may use
    * this to test for the presence of a certain kernel module.
    */
   int test, satisfied;

   /* Your testing code here */
   test=satisfied=0;
   if (test==satisfied)
      return true;
   return false;
}

#ifdef CONTROLSEND_ENABLE
/**
 * Write data to the device, doing any required padding or checksumming
 */
/*@ +charint -usedef -compdef @*/
static ssize_t _proto__control_send(struct gps_device_t *session,
			   char *msg, size_t msglen)
{
   bool ok;

   /* CONSTRUCT THE MESSAGE */

   /*
    * This copy to a public assembly buffer
    * enables gpsmon to snoop the control message
    * after it has been sent.
    */
   session->msgbuflen = msglen;
   (void)memcpy(session->msgbuf, msg, msglen);

   /* we may need to dump the message */
    return gpsd_write(session, session->msgbuf, session->msgbuflen);
   gpsd_report(LOG_IO, "writing _proto_ control type %02x\n");
   return gpsd_write(session, session->msgbuf, session->msgbuflen);
}
/*@ -charint +usedef +compdef @*/
#endif /* CONTROLSEND_ENABLE */

#ifdef RECONFIGURE_ENABLE
static void _proto__event_hook(struct gps_device_t *session, event_t event)
{
    if (session->context->readonly)
	return;

    if (event == event_wakeup) {
       /*
	* Code to make the device ready to communicate.  Only needed if the
	* device is in some kind of sleeping state, and only shipped to
	* RS232C, so that gpsd won't send strings to unidentified USB devices
	* that might not be GPSes at all.
	*/
    }
    if (event == event_identified) {
	/*
	 * Fires when the first full packet is recognized from a
	 * previously unidentified device.  The session packet counter
	 * is zeroed.  If your device has a default cycle time other
	 * than 1 second, set session->device->gpsdata.cycle here. If
	 * possible, get the software version and store it in
	 * session->subtype.
	 */
    }
    if (event == event_configure) {
	/*
	 * Change sentence mix and set reporting modes as needed.
	 * Called immediately after event_identified fires, then just
	 * after every packet received thereafter, but you probably
	 * only want to take actions on the first few packets after
	 * the session packet counter has been zeroed,
	 *
	 * Remember that session->packet.counter is available when you
	 * write this hook; you can use this fact to interleave configuration
	 * sends with the first few packet reads, which is useful for
	 * devices with small receive buffers.
	 */
    } else if (event == event_driver_switch) {
	/*
	 * Fires when the driver on a device is changed *after* it
	 * has been identified.
	 */
    } else if (event == event_deactivate) {
	/*
	 * Fires when the device is deactivated.  Usr this to revert
	 * whatever was done at event_identify and event_configure
	 * time.
	 */
    } else if (event == event_reactivate) {
       /*
	* Fires when a device is reactivated after having been closed.
	* Use this hook for re-establishing device settings that
	* it doesn't hold through closes.
	*/
    }
}

/*
 * This is the entry point to the driver. When the packet sniffer recognizes
 * a packet for this driver, it calls this method which passes the packet to
 * the binary processor or the nmea processor, depending on the session type.
 */
static gps_mask_t _proto__parse_input(struct gps_device_t *session)
{
    gps_mask_t st;

    if (session->packet.type == _PROTO__PACKET) {
	st = _proto__dispatch(session, session->packet.outbuffer, session->packet.outbuflen);
	session->gpsdata.driver_mode = MODE_BINARY;
	return st;
#ifdef NMEA_ENABLE
    } else if (session->packet.type == NMEA_PACKET) {
	st = nmea_parse((char *)session->packet.outbuffer, session);
	session->gpsdata.driver_mode = MODE_NMEA;
	return st;
#endif /* NMEA_ENABLE */
    } else
	return 0;
}

static bool _proto__set_speed(struct gps_device_t *session,
			      speed_t speed, char parity, int stopbits)
{
    /*
     * Set port operating mode, speed, parity, stopbits etc. here.
     * Note: parity is passed as 'N'/'E'/'O', but you should program
     * defensively and allow 0/1/2 as well.
     */
}

/*
 * Switch between NMEA and binary mode, if supported
 */
static void _proto__set_mode(struct gps_device_t *session, int mode)
{
    if (mode == MODE_NMEA) {
	// _proto__to_nmea(session->gpsdata.gps_fd,session->gpsdata.baudrate); /* send the mode switch control string */
	session->gpsdata.driver_mode = MODE_NMEA;
	/*
	 * Anticipatory switching works only when the packet getter is the
	 * generic one and it recognizes packets of the type this driver
	 * is expecting.  This should be the normal case.
	 */
	(void)gpsd_switch_driver(session, "Generic NMEA");
    } else {
	session->back_to_nmea = false;
	session->gpsdata.driver_mode = MODE_BINARY;
    }
}
#endif /* RECONFIGURE_ENABLE */

#ifdef NTPSHM_ENABLE
static double _proto_ntp_offset(struct gps_device_t *session)
{
    /*
     * If NTP notification is enabled, the GPS will occasionally NTP
     * its notion of the time. This will lag behind actual time by
     * some amount which has to be determined by observation vs. (say
     * WWVB radio broadcasts) and, furthermore, may differ by baud
     * rate. This method is for computing the NTP fudge factor.  If
     * it's absent, an offset of 0.0 will be assumed, effectively
     * falling back on what's in ntp.conf. When it returns NAN,
     * nothing will be sent to NTP.
     */
    return MAGIC_CONSTANT;
}
#endif /* NTPSHM_ENABLE */

static void _proto__wrapup(struct gps_device_t *session)
{
}

/* The methods in this code take parameters and have */
/* return values that conform to the requirements AT */
/* THE TIME THE CODE WAS WRITTEN.                    */
/*                                                   */
/* These values may well have changed by the time    */
/* you read this and methods could have been added   */
/* or deleted. Unused methods can be set to NULL.    */
/*                                                   */
/* The latest version can be found by inspecting   */
/* the contents of struct gps_type_t in gpsd.h.      */
/*                                                   */
/* This always contains the correct definitions that */
/* any driver must use to compile.                   */

/* This is everything we export */
/* *INDENT-OFF* */
const struct gps_type_t _proto__binary = {
    /* Full name of type */
    .type_name        = "_proto_ binary",
    /* Associated lexer packet type */
    .packet_type      = _PROTO__PACKET,
    /* Driver tyoe flags */
    .flags	      = DRIVER_NOFLAGS,
    /* Response string that identifies device (not active) */
    .trigger          = NULL,
    /* Number of satellite channels supported by the device */
    .channels         = 12,
    /* Startup-time device detector */
    .probe_detect     = _proto__probe_detect,
    /* Packet getter (using default routine) */
    .get_packet       = generic_get,
    /* Parse message packets */
    .parse_packet     = _proto__parse_input,
    /* RTCM handler (using default routine) */
    .rtcm_writer      = pass_rtcm,
    /* fire on various lifetime events */
    .event_hook       = _proto__event_hook,
#ifdef RECONFIGURE_ENABLE
    /* Speed (baudrate) switch */
    .speed_switcher   = _proto__set_speed,
    /* Switch to NMEA mode */
    .mode_switcher    = _proto__set_mode,
    /* Message delivery rate switcher (not active) */
    .rate_switcher    = NULL,
    /* Minimum cycle time of the device */
    .min_cycle        = 1,
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    /* Control string sender - should provide checksum and headers/trailer */
    .control_send   = _proto__control_send,
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = _proto_ntp_offset,
#endif /* NTPSHM_ENABLE */
/* *INDENT-ON* */
};
#endif /* defined(_PROTO__ENABLE) && defined(BINARY_ENABLE) */

