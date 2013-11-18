/*
 * UBX driver.  All capabilities are common to Antaris4 and uBlox6.
 * Reference manuals are at
 * http://www.u-blox.com/en/download/documents-a-resources/u-blox-6-gps-modules-resources.html
 *
 * Week counters are not limited to 10 bits. It's unknown what
 * the firmware is doing to disambiguate them, if anything; it might just
 * be adding a fixed offset based on a hidden epoch value, in which case
 * unhappy things will occur on the next rollover.
 *
 * For the Antaris 4, the default leap-secoond offset (before getting one from
 * the sats, one presumes) is 0sec; for the uBlox6 it's 15sec.
 *
 * This file is Copyright (c) 2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 *
 */

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#ifndef S_SPLINT_S
#include <unistd.h>
#endif /* S_SPLINT_S */

#include "gpsd.h"
#if defined(UBX_ENABLE) && defined(BINARY_ENABLE)
#include "driver_ubx.h"

#include "bits.h"

/*
 * A ubx packet looks like this:
 * leader: 0xb5 0x62
 * message class: 1 byte
 * message type: 1 byte
 * length of payload: 2 bytes
 * payload: variable length
 * checksum: 2 bytes
 *
 * see also the FV25 and UBX documents on reference.html
 */
#define UBX_PREFIX_LEN		6
#define UBX_CLASS_OFFSET	2
#define UBX_TYPE_OFFSET		3

static gps_mask_t ubx_parse(struct gps_device_t *session, unsigned char *buf,
			    size_t len);
static gps_mask_t ubx_msg_nav_sol(struct gps_device_t *session,
				  unsigned char *buf, size_t data_len);
static gps_mask_t ubx_msg_nav_dop(struct gps_device_t *session,
				  unsigned char *buf, size_t data_len);
static gps_mask_t ubx_msg_nav_timegps(struct gps_device_t *session,
				      unsigned char *buf, size_t data_len);
static gps_mask_t ubx_msg_nav_svinfo(struct gps_device_t *session,
				     unsigned char *buf, size_t data_len);
static void ubx_msg_sbas(struct gps_device_t *session, unsigned char *buf);
static void ubx_msg_inf(unsigned char *buf, size_t data_len);

/**
 * Navigation solution message
 */
static gps_mask_t
ubx_msg_nav_sol(struct gps_device_t *session, unsigned char *buf,
		size_t data_len)
{
    unsigned short gw;
    unsigned int tow, flags;
    double epx, epy, epz, evx, evy, evz;
    unsigned char navmode;
    gps_mask_t mask;

    if (data_len != 52)
	return 0;

    flags = (unsigned int)getub(buf, 11);
    mask = 0;
    if ((flags & (UBX_SOL_VALID_WEEK | UBX_SOL_VALID_TIME)) != 0) {
	tow = (unsigned int)getleu32(buf, 0);
	gw = (unsigned short)getles16(buf, 8);
	session->newdata.time = gpsd_gpstime_resolve(session, gw, tow / 1000.0);
	mask |= TIME_SET | PPSTIME_IS;
    }

    epx = (double)(getles32(buf, 12) / 100.0);
    epy = (double)(getles32(buf, 16) / 100.0);
    epz = (double)(getles32(buf, 20) / 100.0);
    evx = (double)(getles32(buf, 28) / 100.0);
    evy = (double)(getles32(buf, 32) / 100.0);
    evz = (double)(getles32(buf, 36) / 100.0);
    ecef_to_wgs84fix(&session->newdata, &session->gpsdata.separation,
		     epx, epy, epz, evx, evy, evz);
    mask |= LATLON_SET | ALTITUDE_SET | SPEED_SET | TRACK_SET | CLIMB_SET;
    session->newdata.epx = session->newdata.epy =
	(double)(getles32(buf, 24) / 100.0) / sqrt(2);
    session->newdata.eps = (double)(getles32(buf, 40) / 100.0);
    /* Better to have a single point of truth about DOPs */
    //session->gpsdata.dop.pdop = (double)(getleu16(buf, 44)/100.0);
    session->gpsdata.satellites_used = (int)getub(buf, 47);

    navmode = (unsigned char)getub(buf, 10);
    switch (navmode) {
    case UBX_MODE_TMONLY:
    case UBX_MODE_3D:
	session->newdata.mode = MODE_3D;
	break;
    case UBX_MODE_2D:
    case UBX_MODE_DR:		/* consider this too as 2D */
    case UBX_MODE_GPSDR:	/* FIX-ME: DR-aided GPS may be valid 3D */
	session->newdata.mode = MODE_2D;
	break;
    default:
	session->newdata.mode = MODE_NO_FIX;
    }

    if ((flags & UBX_SOL_FLAG_DGPS) != 0)
	session->gpsdata.status = STATUS_DGPS_FIX;
    else if (session->newdata.mode != MODE_NO_FIX)
	session->gpsdata.status = STATUS_FIX;

    mask |= MODE_SET | STATUS_SET;
    gpsd_report(LOG_DATA,
		"NAVSOL: time=%.2f lat=%.2f lon=%.2f alt=%.2f track=%.2f speed=%.2f climb=%.2f mode=%d status=%d used=%d\n",
		session->newdata.time,
		session->newdata.latitude,
		session->newdata.longitude,
		session->newdata.altitude,
		session->newdata.track,
		session->newdata.speed,
		session->newdata.climb,
		session->newdata.mode,
		session->gpsdata.status,
		session->gpsdata.satellites_used);
    return mask;
}

/**
 * Dilution of precision message
 */
static gps_mask_t
ubx_msg_nav_dop(struct gps_device_t *session, unsigned char *buf,
		size_t data_len)
{
    if (data_len != 18)
	return 0;

    /*
     * We make a deliberate choice not to clear DOPs from the
     * last skyview here, but rather to treat this as a supplement
     * to our calculations from the visiniolity matrix, trusting
     * the firmware algorithms over ours.
     */
    session->gpsdata.dop.gdop = (double)(getleu16(buf, 4) / 100.0);
    session->gpsdata.dop.pdop = (double)(getleu16(buf, 6) / 100.0);
    session->gpsdata.dop.tdop = (double)(getleu16(buf, 8) / 100.0);
    session->gpsdata.dop.vdop = (double)(getleu16(buf, 10) / 100.0);
    session->gpsdata.dop.hdop = (double)(getleu16(buf, 12) / 100.0);
    gpsd_report(LOG_DATA, "NAVDOP: gdop=%.2f pdop=%.2f "
		"hdop=%.2f vdop=%.2f tdop=%.2f mask={DOP}\n",
		session->gpsdata.dop.gdop,
		session->gpsdata.dop.hdop,
		session->gpsdata.dop.vdop,
		session->gpsdata.dop.pdop, session->gpsdata.dop.tdop);
    return DOP_SET;
}

/**
 * GPS Leap Seconds
 */
static gps_mask_t
ubx_msg_nav_timegps(struct gps_device_t *session, unsigned char *buf,
		    size_t data_len)
{
    unsigned int gw, tow, flags;

    if (data_len != 16)
	return 0;

    tow = (unsigned int)getleu32(buf, 0);
    gw = (unsigned int)getles16(buf, 8);
    flags = (unsigned int)getub(buf, 11);
    if ((flags & 0x7) != 0)
	session->context->leap_seconds = (int)getub(buf, 10);
    session->newdata.time = gpsd_gpstime_resolve(session,
					      (unsigned short int)gw,
					      (double)tow / 1000.0);

    gpsd_report(LOG_DATA, "TIMEGPS: time=%.2f mask={TIME}\n",
		session->newdata.time);
    return TIME_SET | PPSTIME_IS;
}

/**
 * GPS Satellite Info
 */
static gps_mask_t
ubx_msg_nav_svinfo(struct gps_device_t *session, unsigned char *buf,
		   size_t data_len)
{
    unsigned int i, j, nchan, nsv, st;

    if (data_len < 152) {
	gpsd_report(LOG_PROG, "runt svinfo (datalen=%zd)\n", data_len);
	return 0;
    }
    /*@ +charint @*/
    nchan = (unsigned int)getub(buf, 4);
    if (nchan > MAXCHANNELS) {
	gpsd_report(LOG_WARN,
		    "Invalid NAV SVINFO message, >%d reported visible",
		    MAXCHANNELS);
	return 0;
    }
    /*@ -charint @*/
    gpsd_zero_satellites(&session->gpsdata);
    nsv = 0;
    for (i = j = st = 0; i < nchan; i++) {
	unsigned int off = 8 + 12 * i;
	if ((int)getub(buf, off + 4) == 0)
	    continue;		/* LEA-5H seems to have a bug reporting sats it does not see or hear */
	session->gpsdata.PRN[j] = (int)getub(buf, off + 1);
	session->gpsdata.ss[j] = (float)getub(buf, off + 4);
	session->gpsdata.elevation[j] = (int)getsb(buf, off + 5);
	session->gpsdata.azimuth[j] = (int)getles16(buf, off + 6);
	if (session->gpsdata.PRN[j])
	    st++;
	/*@ -predboolothers */
	if (getub(buf, off + 2) & 0x01)
	    session->gpsdata.used[nsv++] = session->gpsdata.PRN[j];
	if (session->gpsdata.PRN[j] == (int)session->driver.ubx.sbas_in_use)
	    session->gpsdata.used[nsv++] = session->gpsdata.PRN[j];
	/*@ +predboolothers */
	j++;
    }

    session->gpsdata.skyview_time = NAN;
    session->gpsdata.satellites_visible = (int)st;
    session->gpsdata.satellites_used = (int)nsv;
    gpsd_report(LOG_DATA,
		"SVINFO: visible=%d used=%d mask={SATELLITE|USED}\n",
		session->gpsdata.satellites_visible,
		session->gpsdata.satellites_used);
    return SATELLITE_SET | USED_IS;
}

/*
 * SBAS Info
 */
static void ubx_msg_sbas(struct gps_device_t *session, unsigned char *buf)
{
#ifdef __UNUSED_DEBUG__
    unsigned int i, nsv;

    gpsd_report(LOG_WARN, "SBAS: %d %d %d %d %d\n",
		(int)getub(buf, 4), (int)getub(buf, 5), (int)getub(buf, 6),
		(int)getub(buf, 7), (int)getub(buf, 8));

    nsv = (int)getub(buf, 8);
    for (i = 0; i < nsv; i++) {
	int off = 12 + 12 * i;
	gpsd_report(LOG_WARN, "SBAS info on SV: %d\n", (int)getub(buf, off));
    }
#endif /* __UNUSED_DEBUG__ */
/* really 'in_use' depends on the sats info, EGNOS is still in test */
/* In WAAS areas one might also check for the type of corrections indicated */
    session->driver.ubx.sbas_in_use = (unsigned char)getub(buf, 4);
}

/*
 * Raw Subframes
 */
static gps_mask_t ubx_msg_sfrb(struct gps_device_t *session, unsigned char *buf)
{
    unsigned int i, chan, svid;
    uint32_t words[10];

    chan = (unsigned int)getub(buf, 0);
    svid = (unsigned int)getub(buf, 1);
    gpsd_report(LOG_PROG, "UBX_RXM_SFRB: %u %u\n", chan, svid);

    /* UBX does all the parity checking, but still bad data gets through */
    for (i = 0; i < 10; i++) {
	words[i] = (uint32_t)getleu32(buf, 4 * i + 2) & 0xffffff;
    }

    return gpsd_interpret_subframe(session, svid, words);
}

static void ubx_msg_inf(unsigned char *buf, size_t data_len)
{
    unsigned short msgid;
    static char txtbuf[MAX_PACKET_LENGTH];

    msgid = (unsigned short)((buf[2] << 8) | buf[3]);
    if (data_len > MAX_PACKET_LENGTH - 1)
	data_len = MAX_PACKET_LENGTH - 1;

    (void)strlcpy(txtbuf, (char *)buf + UBX_PREFIX_LEN, MAX_PACKET_LENGTH);
    txtbuf[data_len] = '\0';
    switch (msgid) {
    case UBX_INF_DEBUG:
	gpsd_report(LOG_PROG, "UBX_INF_DEBUG: %s\n", txtbuf);
	break;
    case UBX_INF_TEST:
	gpsd_report(LOG_PROG, "UBX_INF_TEST: %s\n", txtbuf);
	break;
    case UBX_INF_NOTICE:
	gpsd_report(LOG_INF, "UBX_INF_NOTICE: %s\n", txtbuf);
	break;
    case UBX_INF_WARNING:
	gpsd_report(LOG_WARN, "UBX_INF_WARNING: %s\n", txtbuf);
	break;
    case UBX_INF_ERROR:
	gpsd_report(LOG_WARN, "UBX_INF_ERROR: %s\n", txtbuf);
	break;
    default:
	break;
    }
    return;
}

/*@ +charint @*/
gps_mask_t ubx_parse(struct gps_device_t * session, unsigned char *buf,
		     size_t len)
{
    size_t data_len;
    unsigned short msgid;
    gps_mask_t mask = 0;
    int i;

    /* the packet at least contains a head long enough for an empty message */
    if (len < UBX_PREFIX_LEN)
	return 0;

    session->cycle_end_reliable = true;

    /* extract message id and length */
    msgid = (buf[2] << 8) | buf[3];
    data_len = (size_t) getles16(buf, 4);
    switch (msgid) {
    case UBX_NAV_POSECEF:
	gpsd_report(LOG_IO, "UBX_NAV_POSECEF\n");
	break;
    case UBX_NAV_POSLLH:
	gpsd_report(LOG_IO, "UBX_NAV_POSLLH\n");
	break;
    case UBX_NAV_STATUS:
	gpsd_report(LOG_IO, "UBX_NAV_STATUS\n");
	break;
    case UBX_NAV_DOP:
	gpsd_report(LOG_PROG, "UBX_NAV_DOP\n");
	mask = ubx_msg_nav_dop(session, &buf[UBX_PREFIX_LEN], data_len);
	break;
    case UBX_NAV_SOL:
	gpsd_report(LOG_PROG, "UBX_NAV_SOL\n");
	mask =
	    ubx_msg_nav_sol(session, &buf[UBX_PREFIX_LEN],
			    data_len) | (CLEAR_IS | REPORT_IS);
	break;
    case UBX_NAV_POSUTM:
	gpsd_report(LOG_IO, "UBX_NAV_POSUTM\n");
	break;
    case UBX_NAV_VELECEF:
	gpsd_report(LOG_IO, "UBX_NAV_VELECEF\n");
	break;
    case UBX_NAV_VELNED:
	gpsd_report(LOG_IO, "UBX_NAV_VELNED\n");
	break;
    case UBX_NAV_TIMEGPS:
	gpsd_report(LOG_PROG, "UBX_NAV_TIMEGPS\n");
	mask = ubx_msg_nav_timegps(session, &buf[UBX_PREFIX_LEN], data_len);
	break;
    case UBX_NAV_TIMEUTC:
	gpsd_report(LOG_IO, "UBX_NAV_TIMEUTC\n");
	break;
    case UBX_NAV_CLOCK:
	gpsd_report(LOG_IO, "UBX_NAV_CLOCK\n");
	break;
    case UBX_NAV_SVINFO:
	gpsd_report(LOG_PROG, "UBX_NAV_SVINFO\n");
	mask = ubx_msg_nav_svinfo(session, &buf[UBX_PREFIX_LEN], data_len);
	break;
    case UBX_NAV_DGPS:
	gpsd_report(LOG_IO, "UBX_NAV_DGPS\n");
	break;
    case UBX_NAV_SBAS:
	gpsd_report(LOG_IO, "UBX_NAV_SBAS\n");
	ubx_msg_sbas(session, &buf[6]);
	break;
    case UBX_NAV_EKFSTATUS:
	gpsd_report(LOG_IO, "UBX_NAV_EKFSTATUS\n");
	break;

    case UBX_RXM_RAW:
	gpsd_report(LOG_IO, "UBX_RXM_RAW\n");
	break;
    case UBX_RXM_SFRB:
	mask = ubx_msg_sfrb(session, &buf[UBX_PREFIX_LEN]);
	break;
    case UBX_RXM_SVSI:
	gpsd_report(LOG_PROG, "UBX_RXM_SVSI\n");
	break;
    case UBX_RXM_ALM:
	gpsd_report(LOG_IO, "UBX_RXM_ALM\n");
	break;
    case UBX_RXM_EPH:
	gpsd_report(LOG_IO, "UBX_RXM_EPH\n");
	break;
    case UBX_RXM_POSREQ:
	gpsd_report(LOG_IO, "UBX_RXM_POSREQ\n");
	break;

    case UBX_MON_SCHED:
	gpsd_report(LOG_IO, "UBX_MON_SCHED\n");
	break;
    case UBX_MON_IO:
	gpsd_report(LOG_IO, "UBX_MON_IO\n");
	break;
    case UBX_MON_IPC:
	gpsd_report(LOG_IO, "UBX_MON_IPC\n");
	break;
    case UBX_MON_VER:
	gpsd_report(LOG_IO, "UBX_MON_VER\n");
	break;
    case UBX_MON_EXCEPT:
	gpsd_report(LOG_IO, "UBX_MON_EXCEPT\n");
	break;
    case UBX_MON_MSGPP:
	gpsd_report(LOG_IO, "UBX_MON_MSGPP\n");
	break;
    case UBX_MON_RXBUF:
	gpsd_report(LOG_IO, "UBX_MON_RXBUF\n");
	break;
    case UBX_MON_TXBUF:
	gpsd_report(LOG_IO, "UBX_MON_TXBUF\n");
	break;
    case UBX_MON_HW:
	gpsd_report(LOG_IO, "UBX_MON_HW\n");
	break;
    case UBX_MON_USB:
	gpsd_report(LOG_IO, "UBX_MON_USB\n");
	break;

    case UBX_INF_DEBUG:
	/* FALLTHROUGH */
    case UBX_INF_TEST:
	/* FALLTHROUGH */
    case UBX_INF_NOTICE:
	/* FALLTHROUGH */
    case UBX_INF_WARNING:
	/* FALLTHROUGH */
    case UBX_INF_ERROR:
	ubx_msg_inf(buf, data_len);
	break;

    case UBX_TIM_TP:
	gpsd_report(LOG_IO, "UBX_TIM_TP\n");
	break;
    case UBX_TIM_TM:
	gpsd_report(LOG_IO, "UBX_TIM_TM\n");
	break;
    case UBX_TIM_TM2:
	gpsd_report(LOG_IO, "UBX_TIM_TM2\n");
	break;
    case UBX_TIM_SVIN:
	gpsd_report(LOG_IO, "UBX_TIM_SVIN\n");
	break;

    case UBX_CFG_PRT:
	gpsd_report(LOG_IO, "UBX_CFG_PRT\n");
	for (i = 0; i < (int)sizeof(session->driver.ubx.port_settings); i++)
	    session->driver.ubx.port_settings[i] = buf[UBX_PREFIX_LEN+i];
	/* turn off NMEA output on this port */
	session->driver.ubx.port_settings[14] &= ~0x02;
	(void)ubx_write(session, 0x06, 0x00,
			session->driver.ubx.port_settings,
			sizeof(session->driver.ubx.port_settings));
	session->driver.ubx.have_port_configuration = true;
	break;

    case UBX_ACK_NAK:
	gpsd_report(LOG_IO, "UBX_ACK_NAK, class: %02x, id: %02x\n",
		    buf[UBX_CLASS_OFFSET],
		    buf[UBX_TYPE_OFFSET]);
	break;
    case UBX_ACK_ACK:
	gpsd_report(LOG_IO, "UBX_ACK_ACK, class: %02x, id: %02x\n",
		    buf[UBX_CLASS_OFFSET],
		    buf[UBX_TYPE_OFFSET]);
	break;

    default:
	gpsd_report(LOG_WARN,
		    "UBX: unknown packet id 0x%04hx (length %zd)\n",
		    msgid, len);
    }

    if (mask)
	(void)snprintf(session->gpsdata.tag, sizeof(session->gpsdata.tag),
		       "0x%04hx", msgid);

    return mask | ONLINE_SET;
}

/*@ -charint @*/

static gps_mask_t parse_input(struct gps_device_t *session)
{
    gps_mask_t st = 0;

    if (session->packet.type == UBX_PACKET) {
	st = ubx_parse(session, session->packet.outbuffer,
		       session->packet.outbuflen);
	session->gpsdata.dev.driver_mode = MODE_BINARY;
#ifdef NMEA_ENABLE
    } else if (session->packet.type == NMEA_PACKET) {
	st = nmea_parse((char *)session->packet.outbuffer, session);
	session->gpsdata.dev.driver_mode = MODE_NMEA;
#endif /* NMEA_ENABLE */
    }
    return st;
}

bool ubx_write(struct gps_device_t * session,
	       unsigned int msg_class, unsigned int msg_id,
	       unsigned char *msg, size_t data_len)
{
    unsigned char CK_A, CK_B;
    ssize_t count;
    size_t i;
    bool ok;

    /*@ -type @*/
    session->msgbuf[0] = 0xb5;
    session->msgbuf[1] = 0x62;

    CK_A = CK_B = 0;
    session->msgbuf[2] = msg_class;
    session->msgbuf[3] = msg_id;
    session->msgbuf[4] = data_len & 0xff;
    session->msgbuf[5] = (data_len >> 8) & 0xff;

    assert(msg != NULL || data_len == 0);
    if (msg != NULL)
	(void)memcpy(&session->msgbuf[6], msg, data_len);

    /* calculate CRC */
    for (i = 2; i < 6; i++) {
	CK_A += session->msgbuf[i];
	CK_B += CK_A;
    }
    /*@ -nullderef @*/
    if (msg != NULL)
	for (i = 0; i < data_len; i++) {
	    CK_A += msg[i];
	    CK_B += CK_A;
	}

    session->msgbuf[6 + data_len] = CK_A;
    session->msgbuf[7 + data_len] = CK_B;
    session->msgbuflen = data_len + 8;
    /*@ +type @*/


    gpsd_report(LOG_IO,
		"=> GPS: UBX class: %02x, id: %02x, len: %zd, crc: %02x%02x\n",
		msg_class, msg_id, data_len,
		CK_A, CK_B);
    count = gpsd_write(session, session->msgbuf, session->msgbuflen);
    ok = (count == (ssize_t) session->msgbuflen);
    /*@ +nullderef @*/
    return (ok);
}

#ifdef CONTROLSEND_ENABLE
static ssize_t ubx_control_send(struct gps_device_t *session, char *msg,
				size_t data_len)
/* not used by gpsd, it's for gpsctl and friends */
{
    return ubx_write(session, (unsigned int)msg[0], (unsigned int)msg[1],
		     (unsigned char *)msg + 2,
		     (size_t)(data_len - 2)) ? ((ssize_t) (data_len + 7)) : -1;
}
#endif /* CONTROLSEND_ENABLE */

static void ubx_catch_model(struct gps_device_t *session, unsigned char *buf,
			    size_t len)
{
    /*@ +charint */
    unsigned char *ip = &buf[19];
    unsigned char *op = (unsigned char *)session->subtype;
    size_t end = ((len - 19) < 63) ? (len - 19) : 63;
    size_t i;

    for (i = 0; i < end; i++) {
	if ((*ip == 0x00) || (*ip == '*')) {
	    *op = 0x00;
	    break;
	}
	*(op++) = *(ip++);
    }
    /*@ -charint */
}

static void ubx_event_hook(struct gps_device_t *session, event_t event)
{
    if (session->context->readonly)
	return;
    if (event == event_triggermatch)
	ubx_catch_model(session,
			session->packet.outbuffer, session->packet.outbuflen);
    else if (event == event_identified || event == event_reactivate) {
	unsigned char msg[32];

	gpsd_report(LOG_IO, "UBX configure: %d\n", session->packet.counter);

	(void)ubx_write(session, 0x06u, 0x00, NULL, 0);	/* get this port's settings */

	/*@ -type @*/
	msg[0] = 0x03;		/* SBAS mode enabled, accept testbed mode */
	msg[1] = 0x07;		/* SBAS usage: range, differential corrections and integrity */
	msg[2] = 0x03;		/* use the maximun search range: 3 channels */
	msg[3] = 0x00;		/* PRN numbers to search for all set to 0 => auto scan */
	msg[4] = 0x00;
	msg[5] = 0x00;
	msg[6] = 0x00;
	msg[7] = 0x00;
	(void)ubx_write(session, 0x06u, 0x16, msg, 8);

	msg[0] = 0x01;		/* class */
	msg[1] = 0x04;		/* msg id  = UBX_NAV_DOP */
	msg[2] = 0x01;		/* rate */
	(void)ubx_write(session, 0x06u, 0x01, msg, 3);
	msg[0] = 0x01;		/* class */
	msg[1] = 0x06;		/* msg id  = NAV-SOL */
	msg[2] = 0x01;		/* rate */
	(void)ubx_write(session, 0x06u, 0x01, msg, 3);
	msg[0] = 0x01;		/* class */
	msg[1] = 0x20;		/* msg id  = UBX_NAV_TIMEGPS */
	msg[2] = 0x01;		/* rate */
	(void)ubx_write(session, 0x06u, 0x01, msg, 3);
	msg[0] = 0x01;		/* class */
	msg[1] = 0x30;		/* msg id  = NAV-SVINFO */
	msg[2] = 0x0a;		/* rate */
	(void)ubx_write(session, 0x06u, 0x01, msg, 3);
	msg[0] = 0x01;		/* class */
	msg[1] = 0x32;		/* msg id  = NAV-SBAS */
	msg[2] = 0x0a;		/* rate */
	(void)ubx_write(session, 0x06u, 0x01, msg, 3);
	/*@ +type @*/
    } else if (event == event_deactivate) {
	/*@ -type @*/
	unsigned char msg[4] = {
	    0x00, 0x00,		/* hotstart */
	    0x01,		/* controlled software reset */
	    0x00
	};			/* reserved */
	/*@ +type @*/

	gpsd_report(LOG_IO, "UBX revert\n");

	/* Reverting all in one fast and reliable reset */
	(void)ubx_write(session, 0x06, 0x04, msg, 4);	/* CFG-RST */
    }
}

#ifdef RECONFIGURE_ENABLE
static void ubx_nmea_mode(struct gps_device_t *session, int mode)
{
    if (!session->driver.ubx.have_port_configuration)
	return;

    /*@+charint@*/
    if (session->driver.ubx.port_settings[0] == 0x01)		/* set baudrate on serial port only */
	putle32(session->driver.ubx.port_settings, 8, session->gpsdata.dev.baudrate);

    if (mode == MODE_NMEA) {
	session->driver.ubx.port_settings[14] &= ~0x01;	/* turn off UBX output on this port */
	session->driver.ubx.port_settings[14] |= 0x02;	/* turn on NMEA output on this port */
    } else {			/* MODE_BINARY */
	session->driver.ubx.port_settings[14] &= ~0x02;	/* turn off NMEA output on this port */
	session->driver.ubx.port_settings[14] |= 0x01;	/* turn on UBX output on this port */
    }
    /*@ -charint @*/
    (void)ubx_write(session, 0x06u, 0x00,
		    session->driver.ubx.port_settings,
		    sizeof(session->driver.ubx.port_settings));
}

static bool ubx_speed(struct gps_device_t *session,
		      speed_t speed, char parity, int stopbits)
{
    unsigned long usart_mode;

    /*@+charint@*/
    if ((!session->driver.ubx.have_port_configuration) || (session->driver.ubx.port_settings[0] != 0x01))	/* set baudrate on serial port only */
	return false;

    usart_mode = (unsigned long)getleu32(session->driver.ubx.port_settings, 4);
    usart_mode &= ~0xE00;	/* zero bits 11:9 */
    switch (parity) {
    case (int)'E':
    case 2:
	usart_mode |= 0x00;
	break;
    case (int)'O':
    case 1:
	usart_mode |= 0x01;
	break;
    case (int)'N':
    case 0:
    default:
	usart_mode |= 0x4;	/* 0x5 would work too */
	break;
    }
    /*@-charint@*/
    usart_mode &= ~0x03000;	/* zero bits 13:12 */
    if (stopbits == 2)
	usart_mode |= 0x2000;	/* zero value means 1 stop bit */
    putle32(session->driver.ubx.port_settings, 4, usart_mode);
    putle32(session->driver.ubx.port_settings, 8, speed);
    (void)ubx_write(session, 0x06, 0x00,
		    session->driver.ubx.port_settings,
		    sizeof(session->driver.ubx.port_settings));
    return true;
}

static bool ubx_rate(struct gps_device_t *session, double cycletime)
/* change the sample rate of the GPS */
{
    unsigned short s;
    /*@ -type @*/
    unsigned char msg[6] = {
	0x00, 0x00,		/* U2: Measurement rate (ms) */
	0x00, 0x01,		/* U2: Navigation rate (cycles) */
	0x00, 0x00,		/* U2: Alignment to reference time: 0 = UTC, !0 = GPS */
    };
    /*@ +type @*/

    /* clamp to cycle times that i know work on my receiver */
    if (cycletime > 1000.0)
	cycletime = 1000.0;
    if (cycletime < 200.0)
	cycletime = 200.0;

    gpsd_report(LOG_IO, "UBX rate change, report every %f secs\n", cycletime);
    s = (unsigned short)cycletime;
    msg[0] = (unsigned char)(s >> 8);
    msg[1] = (unsigned char)(s & 0xff);

    return ubx_write(session, 0x06, 0x08, msg, 6);	/* CFG-RATE */
}
#endif /* RECONFIGURE_ENABLE */

/* This is everything we export */
/* *INDENT-OFF* */
const struct gps_type_t ubx_binary = {
    .type_name        = "uBlox UBX binary",    /* Full name of type */
    .packet_type      = UBX_PACKET,	/* associated lexer packet type */
    .flags	      = DRIVER_NOFLAGS,	/* no flags set */
    .trigger          = "$GPTXT,01,01,02,MOD",
    .channels         = 50,             /* Number of satellite channels supported by the device */
    .probe_detect     = NULL,           /* Startup-time device detector */
    .get_packet       = generic_get,    /* Packet getter (using default routine) */
    .parse_packet     = parse_input,    /* Parse message packets */
    .rtcm_writer      = gpsd_write,      /* RTCM handler (using default routine) */
    .event_hook       = ubx_event_hook,	/* Fire on various lifetime events */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher   = ubx_speed,      /* Speed (baudrate) switch */
    .mode_switcher    = ubx_nmea_mode,  /* Switch to NMEA mode */
    .rate_switcher    = ubx_rate,       /* Message delivery rate switcher */
    .min_cycle        = 0.25,           /* Maximum 4Hz sample rate */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send     = ubx_control_send,	/* no control sender yet */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = NULL,		/* no method for NTP fudge factor */
#endif /* NTPSHM_ ENABLE */
};
/* *INDENT-ON* */
#endif /* defined(UBX_ENABLE) && defined(BINARY_ENABLE) */
