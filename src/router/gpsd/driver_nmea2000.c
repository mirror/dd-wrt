/*
 * NMEA2000 over CAN.
 *
 * This file is Copyright (c) 2012 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <fcntl.h>
#ifndef S_SPLINT_S
#include <unistd.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#endif /* S_SPLINT_S */

#include "gpsd.h"
#if defined(NMEA2000_ENABLE)
#include "driver_nmea2000.h"
#include "bits.h"

#ifndef S_SPLINT_S
#include <linux/can.h>
#include <linux/can/raw.h>
#endif /* S_SPLINT_S */

#define LOG_FILE 1
#define NMEA2000_NETS 4
#define NMEA2000_UNITS 256
#define CAN_NAMELEN 32
#define MIN(a,b) ((a < b) ? a : b)

#define NMEA2000_DEBUG_AIS 0
#define NMEA2000_FAST_DEBUG 0

static struct gps_device_t *nmea2000_units[NMEA2000_NETS][NMEA2000_UNITS];
static char can_interface_name[NMEA2000_NETS][CAN_NAMELEN];

typedef struct PGN
    {
    unsigned int  pgn;
    unsigned int  fast;
    unsigned int  type;
    gps_mask_t    (* func)(unsigned char *bu, int len, struct PGN *pgn, struct gps_device_t *session);
    const char    *name;
    } PGN;

/*@-nullassign@*/

#if LOG_FILE
FILE *logFile = NULL;
#endif /* of if LOG_FILE */

extern bool __attribute__ ((weak)) gpsd_add_device(const char *device_name, bool flag_nowait);

static void print_data(unsigned char *buffer, int len, PGN *pgn)
{
#ifdef LIBGPS_DEBUG
    /*@-bufferoverflowhigh@*/
    if ((libgps_debuglevel >= LOG_IO) != 0) {
	int   l1, l2, ptr;
	char  bu[128];

        ptr = 0;
        l2 = sprintf(&bu[ptr], "got data:%6u:%3d: ", pgn->pgn, len);
	ptr += l2;
        for (l1=0;l1<len;l1++) {
            if (((l1 % 20) == 0) && (l1 != 0)) {
	        gpsd_report(LOG_IO,"%s\n", bu);
		ptr = 0;
                l2 = sprintf(&bu[ptr], "                   : ");
		ptr += l2;
            }
            l2 = sprintf(&bu[ptr], "%02ux ", (unsigned int)buffer[l1]);
	    ptr += l2;
        }
        gpsd_report(LOG_IO,"%s\n", bu);
    }
    /*@+bufferoverflowhigh@*/
#endif
}

static gps_mask_t get_mode(struct gps_device_t *session)
{
    if (session->driver.nmea2000.mode_valid) {
        session->newdata.mode = session->driver.nmea2000.mode;
    } else {
        session->newdata.mode = MODE_NOT_SEEN;
    }

    return MODE_SET;
}


static int decode_ais_header(unsigned char *bu, int len, struct ais_t *ais, unsigned int mask)
{
    if (len > 4) {
        ais->type   = (unsigned int) ( bu[0]       & 0x3f);
	ais->repeat = (unsigned int) ((bu[0] >> 6) & 0x03);
	ais->mmsi   = (unsigned int)  getleu32(bu, 1);
	ais->mmsi  &= mask;
	gpsd_report(LOG_INF, "NMEA2000 AIS  message type %u, MMSI %09d:\n", ais->type, ais->mmsi);
	printf("NMEA2000 AIS  message type %2u, MMSI %09u:\n", ais->type, ais->mmsi);
	return(1);
    } else {
        ais->type   =  0;
	ais->repeat =  0;
	ais->mmsi   =  0;
	gpsd_report(LOG_ERROR, "NMEA2000 AIS  message type %u, too short message.\n", ais->type);
	printf("NMEA2000 AIS  message type %u, too short message.\n", ais->type);
    }
    return(0);
}


static int ais_turn_rate(int rate)
{
    if (rate < 0) {
        return(-ais_turn_rate(-rate));
    }
    return((int)(4.733 * sqrt(rate * RAD_2_DEG * .0001 * 60.0)));
}


static double ais_direction(unsigned int val, double scale)
{
    if ((val == 0xffff) && (scale == 1.0)) {
        return(511.0);
    }
    return(val * RAD_2_DEG * 0.0001 * scale);
}


static gps_mask_t hnd_059392(unsigned char *bu, int len, PGN *pgn, struct gps_device_t *session)
{
    print_data(bu, len, pgn);
    gpsd_report(LOG_DATA, "pgn %6d(%3d):\n", pgn->pgn, session->driver.nmea2000.unit);
    return(0);
}


static gps_mask_t hnd_060928(unsigned char *bu, int len, PGN *pgn, struct gps_device_t *session)
{
    print_data(bu, len, pgn);
    gpsd_report(LOG_DATA, "pgn %6d(%3d):\n", pgn->pgn, session->driver.nmea2000.unit);
    return(0);
}


static gps_mask_t hnd_126208(unsigned char *bu, int len, PGN *pgn, struct gps_device_t *session)
{
    print_data(bu, len, pgn);
    gpsd_report(LOG_DATA, "pgn %6d(%3d):\n", pgn->pgn, session->driver.nmea2000.unit);
    return(0);
}


static gps_mask_t hnd_126464(unsigned char *bu, int len, PGN *pgn, struct gps_device_t *session)
{
    print_data(bu, len, pgn);
    gpsd_report(LOG_DATA, "pgn %6d(%3d):\n", pgn->pgn, session->driver.nmea2000.unit);
    return(0);
}


static gps_mask_t hnd_126996(unsigned char *bu, int len, PGN *pgn, struct gps_device_t *session)
{
    print_data(bu, len, pgn);
    gpsd_report(LOG_DATA, "pgn %6d(%3d):\n", pgn->pgn, session->driver.nmea2000.unit);
    return(0);
}


static gps_mask_t hnd_129025(unsigned char *bu, int len, PGN *pgn, struct gps_device_t *session)
{
    print_data(bu, len, pgn);
    gpsd_report(LOG_DATA, "pgn %6d(%3d):\n", pgn->pgn, session->driver.nmea2000.unit);

    /*@-type@*//* splint has a bug here */
    session->newdata.latitude = getles32(bu, 0) * 1e-7;
    session->newdata.longitude = getles32(bu, 4) * 1e-7;
    /*@+type@*/

    (void)strlcpy(session->gpsdata.tag, "129025", sizeof(session->gpsdata.tag));

    return LATLON_SET | get_mode(session);
}


static gps_mask_t hnd_129026(unsigned char *bu, int len, PGN *pgn, struct gps_device_t *session)
{
    print_data(bu, len, pgn);
    gpsd_report(LOG_DATA, "pgn %6d(%3d):\n", pgn->pgn, session->driver.nmea2000.unit);

    session->driver.nmea2000.sid[0]  =  bu[0];

    /*@-type@*//* splint has a bug here */
    session->newdata.track           =  getleu16(bu, 2) * 1e-4 * RAD_2_DEG;
    session->newdata.speed           =  getleu16(bu, 4) * 1e-2;
    /*@+type@*/

    (void)strlcpy(session->gpsdata.tag, "129026", sizeof(session->gpsdata.tag));

    return SPEED_SET | TRACK_SET | get_mode(session);
}


static gps_mask_t hnd_126992(unsigned char *bu, int len, PGN *pgn, struct gps_device_t *session)
{
    //uint8_t        sid;
    //uint8_t        source;

    print_data(bu, len, pgn);
    gpsd_report(LOG_DATA, "pgn %6d(%3d):\n", pgn->pgn, session->driver.nmea2000.unit);

    //sid        = bu[0];
    //source     = bu[1] & 0x0f;

    /*@-type@*//* splint has a bug here */
    session->newdata.time = getleu16(bu, 2)*24*60*60 + getleu32(bu, 4)/1e4;
    /*@+type@*/

    (void)strlcpy(session->gpsdata.tag, "126992", sizeof(session->gpsdata.tag));

    return TIME_SET | get_mode(session);
}


static const int mode_tab[] = {MODE_NO_FIX, MODE_2D,  MODE_3D, MODE_NO_FIX,
			       MODE_NO_FIX, MODE_NO_FIX, MODE_NO_FIX, MODE_NO_FIX};

static gps_mask_t hnd_129539(unsigned char *bu, int len, PGN *pgn, struct gps_device_t *session)
{
    gps_mask_t mask;
    unsigned int req_mode;
    unsigned int act_mode;

    print_data(bu, len, pgn);
    gpsd_report(LOG_DATA, "pgn %6d(%3d):\n", pgn->pgn, session->driver.nmea2000.unit);

    mask                             = 0;
    session->driver.nmea2000.sid[1]  = bu[0];

    session->driver.nmea2000.mode_valid = 1;

    req_mode = (unsigned int)((bu[1] >> 0) & 0x07);
    act_mode = (unsigned int)((bu[1] >> 3) & 0x07);

    /* This is a workaround for some GARMIN plotter, actual mode auto makes no sense for me! */
    if ((act_mode == 3) && (req_mode != 3)) {
        act_mode = req_mode;
    }

    session->driver.nmea2000.mode    = mode_tab[act_mode];

    /*@-type@*//* splint has a bug here */
    session->gpsdata.dop.hdop        = getleu16(bu, 2) * 1e-2;
    session->gpsdata.dop.vdop        = getleu16(bu, 4) * 1e-2;
    session->gpsdata.dop.tdop        = getleu16(bu, 6) * 1e-2;
    /*@+type@*/
    mask                            |= DOP_SET;

    gpsd_report(LOG_DATA, "pgn %6d(%3d): sid:%02x hdop:%5.2f vdop:%5.2f tdop:%5.2f\n",
		pgn->pgn,
		session->driver.nmea2000.unit,
		session->driver.nmea2000.sid[1],
		session->gpsdata.dop.hdop,
		session->gpsdata.dop.vdop,
		session->gpsdata.dop.tdop);

    (void)strlcpy(session->gpsdata.tag, "129539", sizeof(session->gpsdata.tag));

    return mask | get_mode(session);
}


static gps_mask_t hnd_129540(unsigned char *bu, int len, PGN *pgn, struct gps_device_t *session)
{
    int         l1, l2;

    print_data(bu, len, pgn);
    gpsd_report(LOG_DATA, "pgn %6d(%3d):\n", pgn->pgn, session->driver.nmea2000.unit);

    session->driver.nmea2000.sid[2]           = bu[0];
    session->gpsdata.satellites_visible       = (int)bu[2];

    for (l2=0;l2<MAXCHANNELS;l2++) {
        session->gpsdata.used[l2] = 0;
    }
    l2 = 0;
    for (l1=0;l1<session->gpsdata.satellites_visible;l1++) {
        int    svt;
        double azi, elev, snr;

	/*@-type@*//* splint has a bug here */
        elev  = getles16(bu, 3+12*l1+1) * 1e-4 * RAD_2_DEG;
        azi   = getleu16(bu, 3+12*l1+3) * 1e-4 * RAD_2_DEG;
        snr   = getles16(bu, 3+12*l1+5) * 1e-2;
	/*@+type@*/

        svt   = (int)(bu[3+12*l1+11] & 0x0f);

        session->gpsdata.elevation[l1]  = (int) (round(elev));
	session->gpsdata.azimuth[l1]    = (int) (round(azi));
        session->gpsdata.ss[l1]         = snr;
        session->gpsdata.PRN[l1]        = (int)bu[3+12*l1+0];
	if ((svt == 2) || (svt == 5)) {
	    session->gpsdata.used[l2] = session->gpsdata.PRN[l1];
	    l2 += 1;
	}
    }
    return  SATELLITE_SET | USED_IS;
}


static gps_mask_t hnd_129029(unsigned char *bu, int len, PGN *pgn, struct gps_device_t *session)
{
    gps_mask_t mask;

    print_data(bu, len, pgn);
    gpsd_report(LOG_DATA, "pgn %6d(%3d):\n", pgn->pgn, session->driver.nmea2000.unit);

    mask                             = 0;
    session->driver.nmea2000.sid[3]  = bu[0];

    /*@-type@*//* splint has a bug here */
    session->newdata.time            = getleu16(bu,1) * 24*60*60 + getleu32(bu, 3)/1e4;
    /*@+type@*/
    mask                            |= TIME_SET;

    /*@-type@*//* splint has a bug here */
    session->newdata.latitude        = getles64(bu, 7) * 1e-16;
    session->newdata.longitude       = getles64(bu, 15) * 1e-16;
    /*@+type@*/
    mask                            |= LATLON_SET;

    /*@-type@*//* splint has a bug here */
    session->newdata.altitude        = getles64(bu, 23) * 1e-6;
    /*@+type@*/
    mask                            |= ALTITUDE_SET;

//  printf("mode %x %x\n", (bu[31] >> 4) & 0x0f, bu[31]);
    switch ((bu[31] >> 4) & 0x0f) {
    case 0:
        session->gpsdata.status      = STATUS_NO_FIX;
	break;
    case 1:
        session->gpsdata.status      = STATUS_FIX;
	break;
    case 2:
        session->gpsdata.status      = STATUS_DGPS_FIX;
	break;
    case 3:
    case 4:
    case 5:
        session->gpsdata.status      = STATUS_FIX; /* Is this correct ? */
	break;
    default:
        session->gpsdata.status      = STATUS_NO_FIX;
	break;
    }
    mask                            |= STATUS_SET;

    /*@-type@*//* splint has a bug here */
    session->gpsdata.separation      = getles32(bu, 38) / 100.0;
    /*@+type@*/
    session->newdata.altitude       -= session->gpsdata.separation;

    session->gpsdata.satellites_used = (int)bu[33];

    /*@-type@*//* splint has a bug here */
    session->gpsdata.dop.hdop        = getleu16(bu, 34) * 0.01;
    session->gpsdata.dop.pdop        = getleu16(bu, 36) * 0.01;
    /*@+type@*/
    mask                            |= DOP_SET;

    (void)strlcpy(session->gpsdata.tag, "129029", sizeof(session->gpsdata.tag));

    return mask | get_mode(session);
}


static gps_mask_t hnd_129038(unsigned char *bu, int len, PGN *pgn, struct gps_device_t *session)
{
    struct ais_t *ais;

    ais =  &session->gpsdata.ais;
    print_data(bu, len, pgn);
    gpsd_report(LOG_DATA, "pgn %6d(%3d):\n", pgn->pgn, session->driver.nmea2000.unit);

    if (decode_ais_header(bu, len, ais, 0xffffffff) != 0) {
        ais->type1.lon       = (int)          (getles32(bu, 5) * 0.06);
	ais->type1.lat       = (int)          (getles32(bu, 9) * 0.06);
	ais->type1.accuracy  = (bool)         ((bu[13] >> 0) & 0x01);
	ais->type1.raim      = (bool)         ((bu[13] >> 1) & 0x01);
	ais->type1.second    = (unsigned int) ((bu[13] >> 2) & 0x3f);
	ais->type1.course    = (unsigned int)  ais_direction((unsigned int)getleu16(bu, 14), 10.0);
	ais->type1.speed     = (unsigned int) (getleu16(bu, 16) * MPS_TO_KNOTS * 0.01 / 0.1);
	ais->type1.radio     = (unsigned int) (getleu32(bu, 18) & 0x7ffff);
	ais->type1.heading   = (unsigned int)  ais_direction((unsigned int)getleu16(bu, 21), 1.0);
	ais->type1.turn      =                 ais_turn_rate((int)getles16(bu, 23));
	ais->type1.status    = (unsigned int) ((bu[25] >> 0) & 0xff);
	ais->type1.maneuver  = 0; /* Not transmitted ???? */

	return(ONLINE_SET | AIS_SET);
    }
    return(0);
}


static gps_mask_t hnd_129039(unsigned char *bu, int len, PGN *pgn, struct gps_device_t *session)
{
    struct ais_t *ais;

    ais =  &session->gpsdata.ais;
    print_data(bu, len, pgn);
    gpsd_report(LOG_DATA, "pgn %6d(%3d):\n", pgn->pgn, session->driver.nmea2000.unit);

    if (decode_ais_header(bu, len, ais, 0xffffffff) != 0) {
        ais->type18.lon      = (int)          (getles32(bu, 5) * 0.06);
	ais->type18.lat      = (int)          (getles32(bu, 9) * 0.06);
	ais->type18.accuracy = (bool)         ((bu[13] >> 0) & 0x01);
	ais->type18.raim     = (bool)         ((bu[13] >> 1) & 0x01);
	ais->type18.second   = (unsigned int) ((bu[13] >> 2) & 0x3f);
	ais->type18.course   = (unsigned int)  ais_direction((unsigned int) getleu16(bu, 14), 10.0);
	ais->type18.speed    = (unsigned int) (getleu16(bu, 16) * MPS_TO_KNOTS * 0.01 / 0.1);
	ais->type18.radio    = (unsigned int) (getleu32(bu, 18) & 0x7ffff);
	ais->type18.heading  = (unsigned int)  ais_direction((unsigned int) getleu16(bu, 21), 1.0);    
	ais->type18.reserved = 0;
	ais->type18.regional = (unsigned int) ((bu[24] >> 0) & 0x03);
	ais->type18.cs	     = (bool)         ((bu[24] >> 2) & 0x01);
	ais->type18.display  = (bool)         ((bu[24] >> 3) & 0x01);
	ais->type18.dsc      = (bool)         ((bu[24] >> 4) & 0x01);
	ais->type18.band     = (bool)         ((bu[24] >> 5) & 0x01);
	ais->type18.msg22    = (bool)         ((bu[24] >> 6) & 0x01);
	ais->type18.assigned = (bool)         ((bu[24] >> 7) & 0x01);

	return(ONLINE_SET | AIS_SET);
    }
    return(0);
}


/* No test case for this message at the moment */
static gps_mask_t hnd_129040(unsigned char *bu, int len, PGN *pgn, struct gps_device_t *session)
{
    struct ais_t *ais;

    ais =  &session->gpsdata.ais;
    print_data(bu, len, pgn);
    gpsd_report(LOG_DATA, "pgn %6d(%3d):\n", pgn->pgn, session->driver.nmea2000.unit);

    if (decode_ais_header(bu, len, ais, 0xffffffff) != 0) {
        uint16_t length, beam, to_bow, to_starboard;
	int l;

        ais->type19.lon          = (int)          (getles32(bu, 5) * 0.06);
	ais->type19.lat          = (int)          (getles32(bu, 9) * 0.06);
	ais->type19.accuracy     = (bool)         ((bu[13] >> 0) & 0x01);
	ais->type19.raim         = (bool)         ((bu[13] >> 1) & 0x01);
	ais->type19.second       = (unsigned int) ((bu[13] >> 2) & 0x3f);
	ais->type19.course       = (unsigned int)  ais_direction((unsigned int) getleu16(bu, 14), 10.0);
	ais->type19.speed        = (unsigned int) (getleu16(bu, 16) * MPS_TO_KNOTS * 0.01 / 0.1);
	ais->type19.reserved     = (unsigned int) ((bu[18] >> 0) & 0xff);
	ais->type19.regional     = (unsigned int) ((bu[19] >> 0) & 0x0f);
	ais->type19.shiptype     = (unsigned int) ((bu[20] >> 0) & 0xff);
	ais->type19.heading      = (unsigned int)  ais_direction((unsigned int) getleu16(bu, 21), 1.0);
	length                   =                 getleu16(bu, 24);
	beam                     =                 getleu16(bu, 26);
        to_starboard             =                 getleu16(bu, 28);
        to_bow                   =                 getleu16(bu, 30);
	if ((length == 0xffff) || (to_bow       == 0xffff)) {
	    length       = 0;
	    to_bow       = 0;
	}
	if ((beam   == 0xffff) || (to_starboard == 0xffff)) {
	    beam         = 0;
	    to_starboard = 0;
	}
	ais->type19.to_bow       = (unsigned int) (to_bow/10);
	ais->type19.to_stern     = (unsigned int) ((length-to_bow)/10);
	ais->type19.to_port      = (unsigned int) ((beam-to_starboard)/10);
	ais->type19.to_starboard = (unsigned int) (to_starboard/10);
	ais->type19.epfd         = (unsigned int) ((bu[23] >> 4) & 0x0f);
	ais->type19.dte          = (unsigned int) ((bu[52] >> 0) & 0x01);
	ais->type19.assigned     = (bool)         ((bu[52] >> 1) & 0x01);
	for (l=0;l<AIS_SHIPNAME_MAXLEN;l++) {
	    ais->type19.shipname[l] = (char) bu[32+l];
	}
	ais->type19.shipname[AIS_SHIPNAME_MAXLEN] = (char) 0;
	
	return(ONLINE_SET | AIS_SET);
    }
    return(0);
}


static gps_mask_t hnd_129794(unsigned char *bu, int len, PGN *pgn, struct gps_device_t *session)
{
    struct ais_t *ais;

    ais =  &session->gpsdata.ais;
    print_data(bu, len, pgn);
    gpsd_report(LOG_DATA, "pgn %6d(%3d):\n", pgn->pgn, session->driver.nmea2000.unit);

    if (decode_ais_header(bu, len, ais, 0xffffffff) != 0) {
        uint16_t  length, beam, to_bow, to_starboard, date;
	int       l;
	uint32_t  time;
	time_t    date1;
        struct tm date2;

        ais->type5.ais_version   = (unsigned int) ((bu[73] >> 0) & 0x03);
	ais->type5.imo           = (unsigned int)  getleu32(bu,  5);
	if (ais->type5.imo == 0xffffffff) {
	    ais->type5.imo       = 0;
	}
	ais->type5.shiptype      = (unsigned int) ((bu[36] >> 0) & 0xff);
	length                   =                 getleu16(bu, 37);
	beam                     =                 getleu16(bu, 39);
        to_starboard             =                 getleu16(bu, 41);
        to_bow                   =                 getleu16(bu, 43);
	if ((length == 0xffff) || (to_bow       == 0xffff)) {
	    length       = 0;
	    to_bow       = 0;
	}
	if ((beam   == 0xffff) || (to_starboard == 0xffff)) {
	    beam         = 0;
	    to_starboard = 0;
	}
	ais->type5.to_bow        = (unsigned int) (to_bow/10);
	ais->type5.to_stern      = (unsigned int) ((length-to_bow)/10);
	ais->type5.to_port       = (unsigned int) ((beam-to_starboard)/10);
	ais->type5.to_starboard  = (unsigned int) (to_starboard/10);
	ais->type5.epfd          = (unsigned int) ((bu[73] >> 2) & 0x0f);
	date                     =                 getleu16(bu, 45);
	time                     =                 getleu32(bu, 47);
        date1                    = (time_t)       (date*24*60*60);
	(void) gmtime_r(&date1, &date2);
	ais->type5.month         = (unsigned int) (date2.tm_mon+1);
	ais->type5.day           = (unsigned int) (date2.tm_mday);
	ais->type5.minute        = (unsigned int) (time/(10000*60));
	ais->type5.hour          = (unsigned int) (ais->type5.minute/60);
	ais->type5.minute        = (unsigned int) (ais->type5.minute-(ais->type5.hour*60));

	ais->type5.draught       = (unsigned int) (getleu16(bu, 51)/10);
	ais->type5.dte           = (unsigned int) ((bu[73] >> 6) & 0x01);

	for (l=0;l<7;l++) {
	    ais->type5.callsign[l] = (char) bu[9+l];
	}
	ais->type5.callsign[7]   = (char) 0;

	for (l=0;l<AIS_SHIPNAME_MAXLEN;l++) {
	    ais->type5.shipname[l] = (char) bu[16+l];
	}
	ais->type5.shipname[AIS_SHIPNAME_MAXLEN] = (char) 0;

	for (l=0;l<20;l++) {
	    ais->type5.destination[l] = (char) bu[53+l];
	}
	ais->type5.destination[20] = (char) 0;
#if NMEA2000_DEBUG_AIS
	printf("AIS: MMSI:  %09u\n",
	       ais->mmsi);
	printf("AIS: name:  %-20.20s i:%8u c:%-8.8s b:%6u s:%6u p:%6u s:%6u dr:%4.1f\n",
	       ais->type5.shipname,
	       ais->type5.imo,
	       ais->type5.callsign,
	       ais->type5.to_bow,
	       ais->type5.to_stern,
	       ais->type5.to_port,
	       ais->type5.to_starboard,
	       ais->type5.draught/10.0);
	printf("AIS: arival:%-20.20s at %02u-%02u-%04d %02u:%0u\n",
	       ais->type5.destination, 
	       ais->type5.day,
	       ais->type5.month,
	       date2.tm_year+1900,
	       ais->type5.hour,
	       ais->type5.minute);
#endif /* of #if NMEA2000_DEBUG_AIS */
        return(ONLINE_SET | AIS_SET);
    }
    return(0);
}


/* No test case for this message at the moment */
static gps_mask_t hnd_129798(unsigned char *bu, int len, PGN *pgn, struct gps_device_t *session)
{
    struct ais_t *ais;

    ais =  &session->gpsdata.ais;
    print_data(bu, len, pgn);
    gpsd_report(LOG_DATA, "pgn %6d(%3d):\n", pgn->pgn, session->driver.nmea2000.unit);

    if (decode_ais_header(bu, len, ais, 0xffffffff) != 0) {
        ais->type9.lon       = (int)          (getles32(bu, 5) * 0.06);
	ais->type9.lat       = (int)          (getles32(bu, 9) * 0.06);
	ais->type9.accuracy  = (bool)         ((bu[13] >> 0) & 0x01);
	ais->type9.raim      = (bool)         ((bu[13] >> 1) & 0x01);
	ais->type9.second    = (unsigned int) ((bu[13] >> 2) & 0x3f);
	ais->type9.course    = (unsigned int)  ais_direction((unsigned int) getleu16(bu, 14), 10.0);
	ais->type9.speed     = (unsigned int) (getleu16(bu, 16) * MPS_TO_KNOTS * 0.01 / 0.1);
	ais->type9.radio     = (unsigned int) (getleu32(bu, 18) & 0x7ffff);
	ais->type9.alt       = (unsigned int) (getleu64(bu, 21)/1000000);
	ais->type9.regional  = (unsigned int) ((bu[29] >> 0) & 0xff);
	ais->type9.dte	     = (unsigned int) ((bu[30] >> 0) & 0x01);
/*      ais->type9.spare     = (bu[30] >> 1) & 0x7f; */
	ais->type9.assigned  = 0; /* Not transmitted ???? */

        return(ONLINE_SET | AIS_SET);
    }
    return(0);
}


/* No test case for this message at the moment */
static gps_mask_t hnd_129802(unsigned char *bu, int len, PGN *pgn, struct gps_device_t *session)
{
    struct ais_t *ais;

    ais =  &session->gpsdata.ais;
    print_data(bu, len, pgn);
    gpsd_report(LOG_DATA, "pgn %6d(%3d):\n", pgn->pgn, session->driver.nmea2000.unit);

    if (decode_ais_header(bu, len, ais, 0x3fffffff) != 0) {
        int                   l;

/*      ais->type14.channel = (bu[ 5] >> 0) & 0x1f; */
	for (l=0;l<36;l++) {
	    ais->type14.text[l] = (char) bu[6+l];
	}
	ais->type14.text[36] = (char) 0;

        return(ONLINE_SET | AIS_SET);
    }
    return(0);
}


static gps_mask_t hnd_129809(unsigned char *bu, int len, PGN *pgn, struct gps_device_t *session)
{
    struct ais_t *ais;

    ais =  &session->gpsdata.ais;
    print_data(bu, len, pgn);
    gpsd_report(LOG_DATA, "pgn %6d(%3d):\n", pgn->pgn, session->driver.nmea2000.unit);

    if (decode_ais_header(bu, len, ais, 0xffffffff) != 0) {
        int                   l;
	int                   index   =  session->aivdm[0].type24_queue.index;
        struct ais_type24a_t *saveptr = &session->aivdm[0].type24_queue.ships[index];

	gpsd_report(LOG_PROG, "NMEA2000: AIS message 24A from %09u stashed.\n", ais->mmsi);

	saveptr->mmsi = ais->mmsi;

	for (l=0;l<AIS_SHIPNAME_MAXLEN;l++) {
	    saveptr->shipname[l] = (char) bu[ 5+l];
	}
	saveptr->shipname[AIS_SHIPNAME_MAXLEN] = (char) 0;

	index += 1;
	index %= MAX_TYPE24_INTERLEAVE;
	session->aivdm[0].type24_queue.index = index;
        return(0);
    }
    return(0);
}


static gps_mask_t hnd_129810(unsigned char *bu, int len, PGN *pgn, struct gps_device_t *session)
{
    struct ais_t *ais;

    ais =  &session->gpsdata.ais;
    print_data(bu, len, pgn);
    gpsd_report(LOG_DATA, "pgn %6d(%3d):\n", pgn->pgn, session->driver.nmea2000.unit);

    if (decode_ais_header(bu, len, ais, 0xffffffff) != 0) {
	int i, l;

        for (i = 0; i < MAX_TYPE24_INTERLEAVE; i++) {
	    if (session->aivdm[0].type24_queue.ships[i].mmsi == ais->mmsi) {
	        for (l=0;l<AIS_SHIPNAME_MAXLEN;l++) {
		    ais->type24.shipname[l] = (char) (session->aivdm[0].type24_queue.ships[i].shipname[l]);
		}
		ais->type24.shipname[AIS_SHIPNAME_MAXLEN] = (char) 0;

		ais->type24.shiptype = (unsigned int) ((bu[ 5] >> 0) & 0xff);

	        for (l=0;l<7;l++) {
		    ais->type24.vendorid[l] = (char) bu[ 6+l];
		}
		ais->type24.vendorid[7] = (char) 0;

	        for (l=0;l<7;l++) {
		    ais->type24.callsign[l] = (char) bu[13+l];
		}
		ais->type24.callsign[7] = (char )0;

		if (AIS_AUXILIARY_MMSI(ais->mmsi)) {
		    ais->type24.mothership_mmsi   = (unsigned int) (getleu32(bu, 28));
		} else {
		    uint16_t length, beam, to_bow, to_starboard;

		    length                        =                 getleu16(bu, 20);
		    beam                          =                 getleu16(bu, 22);
		    to_starboard                  =                 getleu16(bu, 24);
		    to_bow                        =                 getleu16(bu, 26);
		    ais->type24.dim.to_bow        = (unsigned int) (to_bow/10);
		    ais->type24.dim.to_stern      = (unsigned int) ((length-to_bow)/10);
		    ais->type24.dim.to_port       = (unsigned int) ((beam-to_starboard)/10);
		    ais->type24.dim.to_starboard  = (unsigned int) (to_starboard/10);
		    if ((length == 0xffff) || (to_bow       == 0xffff)) {
		        length       = 0;
			to_bow       = 0;
		    }
		    if ((beam   == 0xffff) || (to_starboard == 0xffff)) {
		        beam         = 0;
			to_starboard = 0;
		    }
		}

		gpsd_report(LOG_PROG, "NMEA2000: AIS 24B from %09u matches a 24A.\n", ais->mmsi);
		/* prevent false match if a 24B is repeated */
		session->aivdm[0].type24_queue.ships[i].mmsi = 0;
#if NMEA2000_DEBUG_AIS
		printf("AIS: MMSI:  %09u\n",
		       ais->mmsi);
		printf("AIS: name:  %-20.20s v:%-8.8s c:%-8.8s b:%6u s:%6u p:%6u s:%6u\n",
		       ais->type24.shipname,
		       ais->type24.vendorid,
		       ais->type24.callsign,
		       ais->type24.dim.to_bow,
		       ais->type24.dim.to_stern,
		       ais->type24.dim.to_port,
		       ais->type24.dim.to_starboard);
#endif /* of #if NMEA2000_DEBUG_AIS */
		return(ONLINE_SET | AIS_SET);
	    }
	}
	gpsd_report(LOG_WARN, "NMEA2000: AIS 24B from %09u can't be matched to a 24A.\n", ais->mmsi);
        return(0);
    }
    return(0);
}


/*@-usereleased@*/
static const char msg_059392[] = {"ISO  Acknowledgment"};
static const char msg_060928[] = {"ISO  Address Claim"};
static const char msg_126208[] = {"NMEA Command/Request/Acknowledge"};
static const char msg_126464[] = {"ISO  Transmit/Receive PGN List"};
static const char msg_126992[] = {"GNSS System Time"};
static const char msg_126996[] = {"ISO  Product Information"};
static const char msg_129025[] = {"GNSS Position Rapid Update"};
static const char msg_129026[] = {"GNSS COG and SOG Rapid Update"};
static const char msg_129029[] = {"GNSS Positition Data"};
static const char msg_129539[] = {"GNSS DOPs"};
static const char msg_129540[] = {"GNSS Satellites in View"};
static const char msg_129038[] = {"AIS  Class A Position Report"};
static const char msg_129039[] = {"AIS  Class B Position Report"};
static const char msg_129040[] = {"AIS  Class B Extended Position Report"};
static const char msg_129794[] = {"AIS  Class A Static and Voyage Related Data"};
static const char msg_129798[] = {"AIS  SAR Aircraft Position Report"};
static const char msg_129802[] = {"AIS  Safty Related Broadcast Message"};
static const char msg_129809[] = {"AIS  Class B CS Static Data Report, Part A"};
static const char msg_129810[] = {"AIS  Class B CS Static Data Report, Part B"};
static const char msg_error [] = {"**error**"};

static PGN gpspgn[] = {{ 59392, 0, 0, hnd_059392, &msg_059392[0]},
		       { 60928, 0, 0, hnd_060928, &msg_060928[0]},
		       {126208, 0, 0, hnd_126208, &msg_126208[0]},
		       {126464, 1, 0, hnd_126464, &msg_126464[0]},
		       {126992, 0, 0, hnd_126992, &msg_126992[0]},
		       {126996, 1, 0, hnd_126996, &msg_126996[0]},
		       {129025, 0, 1, hnd_129025, &msg_129025[0]},
		       {129026, 0, 1, hnd_129026, &msg_129026[0]},
		       {129029, 1, 1, hnd_129029, &msg_129029[0]},
		       {129539, 0, 1, hnd_129539, &msg_129539[0]},
		       {129540, 1, 1, hnd_129540, &msg_129540[0]},
		       {0     , 0, 0, NULL,       &msg_error [0]}};

static PGN aispgn[] = {{ 59392, 0, 0, hnd_059392, &msg_059392[0]},
		       { 60928, 0, 0, hnd_060928, &msg_060928[0]},
		       {126208, 0, 0, hnd_126208, &msg_126208[0]},
		       {126464, 1, 0, hnd_126464, &msg_126464[0]},
		       {126992, 0, 0, hnd_126992, &msg_126992[0]},
		       {126996, 1, 0, hnd_126996, &msg_126996[0]},
		       {129038, 1, 2, hnd_129038, &msg_129038[0]},
		       {129039, 1, 2, hnd_129039, &msg_129039[0]},
		       {129040, 1, 2, hnd_129040, &msg_129040[0]},
		       {129794, 1, 2, hnd_129794, &msg_129794[0]},
		       {129798, 1, 2, hnd_129798, &msg_129798[0]},
		       {129802, 1, 2, hnd_129802, &msg_129802[0]},
		       {129809, 1, 2, hnd_129809, &msg_129809[0]},
		       {129810, 1, 2, hnd_129810, &msg_129810[0]},
		       {0     , 0, 0, NULL,       &msg_error [0]}};
/*@+usereleased@*/

/*@-immediatetrans@*/
static /*@null@*/ PGN *search_pgnlist(unsigned int pgn, PGN *pgnlist)
{
    int l1;
    PGN *work;

    l1 = 0;
    work = NULL;
    while (pgnlist[l1].pgn != 0) {
        if (pgnlist[l1].pgn == pgn) {
	    work = &pgnlist[l1];
	    break;
	} else {
	    l1 = l1 + 1;
	    }
	}
    return work;
}
/*@+immediatetrans@*/

/*@-nullstate -branchstate -globstate -mustfreeonly@*/
static void find_pgn(struct can_frame *frame, struct gps_device_t *session)
{
    PGN *work;
    unsigned int can_net;

    session->driver.nmea2000.workpgn = NULL;
    can_net = session->driver.nmea2000.can_net;
    if (can_net > (NMEA2000_NETS-1)) {
        gpsd_report(LOG_ERROR, "NMEA2000 find_pgn: Invalid can network %d.\n", can_net);
        return;
    }

    /*@ignore@*//* because the CAN include files choke splint */
    if (frame->can_id & 0x80000000) {
	// cppcheck-suppress unreadVariable
	unsigned int source_prio UNUSED;
	unsigned int daddr UNUSED;
	// cppcheck-suppress unreadVariable
	unsigned int source_pgn;
	unsigned int source_unit;

#if LOG_FILE
        if (logFile != NULL) {
	    struct timespec  msgTime;

	    clock_gettime(CLOCK_REALTIME, &msgTime);
	    fprintf(logFile,
		    "(%010d.%06d) can0 %08x#",
		    (unsigned int)msgTime.tv_sec,
		    (unsigned int)msgTime.tv_nsec/1000,
		    frame->can_id & 0x1ffffff);
	    if ((frame->can_dlc & 0x0f) > 0) {
		int l1;
	        for(l1=0;l1<(frame->can_dlc & 0x0f);l1++) {
		    fprintf(logFile, "%02x", frame->data[l1]);
		}
	    }
	    fprintf(logFile, "\n");
	}
#endif /* of if LOG_FILE */
	/*@end@*/
	session->driver.nmea2000.can_msgcnt += 1;
	/*@ignore@*//* because the CAN include files choke splint */
	source_pgn = (frame->can_id >> 8) & 0x1ffff;
	source_prio = (frame->can_id >> 26) & 0x7;
	source_unit = frame->can_id & 0x0ff;
	/*@end@*/

	if ((source_pgn >> 8) < 240) {
	    daddr  = source_pgn & 0x000ff;
	    source_pgn  = source_pgn & 0x1ff00;
	} else {
	    daddr = 0;
	}

	if (session->driver.nmea2000.unit_valid == 0) {
	    unsigned int l1, l2;
	    
	    for (l1=0;l1<NMEA2000_NETS;l1++) {
	        for (l2=0;l2<NMEA2000_UNITS;l2++) {
		    if (session == nmea2000_units[l1][l2]) {
		        session->driver.nmea2000.unit = l2;
		        session->driver.nmea2000.unit_valid = 1;
			session->driver.nmea2000.can_net = l1;
			can_net = l1;
		    }
		}
	    }
	}

	if (session->driver.nmea2000.unit_valid == 0) {
	    session->driver.nmea2000.unit = source_unit;
	    session->driver.nmea2000.unit_valid = 1;
	    nmea2000_units[can_net][source_unit] = session;
	}

	if (source_unit == session->driver.nmea2000.unit) {
	    if (session->driver.nmea2000.pgnlist != NULL) {
	        work = search_pgnlist(source_pgn, session->driver.nmea2000.pgnlist);
	    } else {
	        PGN *pgnlist;

		pgnlist = &gpspgn[0];
		work = search_pgnlist(source_pgn, pgnlist);
		if (work == NULL) {
		    pgnlist = &aispgn[0];
		    work = search_pgnlist(source_pgn, pgnlist);
		}
		if ((work != 0) && (work->type > 0)) {
		    session->driver.nmea2000.pgnlist = pgnlist;
		}
	    }
	    if (work != NULL) {
	        if (work->fast == 0) {
		    size_t l2;

		    gpsd_report(LOG_DATA, "pgn %6d:%s \n", work->pgn, work->name);
		    session->driver.nmea2000.workpgn = (void *) work;
		    /*@i1@*/session->packet.outbuflen =  frame->can_dlc & 0x0f;
		    for (l2=0;l2<session->packet.outbuflen;l2++) {
		        /*@i3@*/session->packet.outbuffer[l2]= frame->data[l2];
		    }
		}
		/*@i2@*/else if ((frame->data[0] & 0x1f) == 0) {
		    unsigned int l2;

		    /*@i2@*/session->driver.nmea2000.fast_packet_len = frame->data[1];
		    /*@i2@*/session->driver.nmea2000.idx = frame->data[0];
#if NMEA2000_FAST_DEBUG
		    gpsd_report(LOG_ERROR, "Set idx    %2x    %2x %2x %6d\n", frame->data[0],
                                                                              session->driver.nmea2000.unit,
				                                              frame->data[1],
                                                                              source_pgn);
#endif /* of #if NMEA2000_FAST_DEBUG */
		    session->packet.inbuflen = 0;
		    session->driver.nmea2000.idx += 1;
		    for (l2=2;l2<8;l2++) {
		        /*@i3@*/session->packet.inbuffer[session->packet.inbuflen++] = frame->data[l2];
		    }
		    gpsd_report(LOG_DATA, "pgn %6d:%s \n", work->pgn, work->name);
		}
		/*@i2@*/else if (frame->data[0] == session->driver.nmea2000.idx) {
		    unsigned int l2;

		    for (l2=1;l2<8;l2++) {
		        if (session->driver.nmea2000.fast_packet_len > session->packet.inbuflen) {
			    /*@i3@*/session->packet.inbuffer[session->packet.inbuflen++] = frame->data[l2];
			}
		    }
		    if (session->packet.inbuflen == session->driver.nmea2000.fast_packet_len) {
#if NMEA2000_FAST_DEBUG
		        gpsd_report(LOG_ERROR, "Fast done  %2x %2x %2x %2x %6d\n", session->driver.nmea2000.idx,
				                                                   /*@i1@*/frame->data[0],
				                                                   session->driver.nmea2000.unit,
				                                                   (unsigned int) session->driver.nmea2000.fast_packet_len,
				                                                   source_pgn);
#endif /* of #if  NMEA2000_FAST_DEBUG */
			session->driver.nmea2000.workpgn = (void *) work;
		        session->packet.outbuflen = session->driver.nmea2000.fast_packet_len;
			for(l2=0;l2 < (unsigned int)session->packet.outbuflen; l2++) {
			    session->packet.outbuffer[l2] = session->packet.inbuffer[l2];
			}
			session->driver.nmea2000.fast_packet_len = 0;
		    } else {
		        session->driver.nmea2000.idx += 1;
		    }
		} else {
		    gpsd_report(LOG_ERROR, "Fast error %2x %2x %2x %2x %6d\n", session->driver.nmea2000.idx,
				                                               /*@i1@*/frame->data[0],
				                                               session->driver.nmea2000.unit,
				                                               (unsigned int) session->driver.nmea2000.fast_packet_len,
				                                               source_pgn);
		}
	    } else {
	        gpsd_report(LOG_WARN, "PGN not found %08d %08x \n", source_pgn, source_pgn);
	    }
	} else {
	    // we got a unknown unit number
	    if (nmea2000_units[can_net][source_unit] == NULL) {
	        char buffer[32];

		(void) snprintf(buffer,
				sizeof(buffer),
				"nmea2000://%s:%u",
				can_interface_name[can_net],
				source_unit);
		if (gpsd_add_device != NULL) {
		    (void) gpsd_add_device(buffer, true);
		}
	    }
	}
    } else {
        // we got RTR or 2.0A CAN frame, not used
    }
}
/*@+nullstate +branchstate +globstate +mustfreeonly@*/


static ssize_t nmea2000_get(struct gps_device_t *session)
{
    struct can_frame frame;
    ssize_t          status;

//  printf("NMEA2000 get: enter\n");
    session->packet.outbuflen = 0;
    status = read(session->gpsdata.gps_fd, &frame, sizeof(frame));
    if (status == (ssize_t)sizeof(frame)) {
        session->packet.type = NMEA2000_PACKET;
	find_pgn(&frame, session);
//	printf("NMEA2000 get: exit(%d)\n", status);
	if (session->driver.nmea2000.workpgn == NULL) {
	    status = 0;
	}
        return frame.can_dlc & 0x0f;
    }
//  printf("NMEA2000 get: exit(EXIT_SUCCESS)\n");
    return 0;
}

/*@-mustfreeonly@*/
static gps_mask_t nmea2000_parse_input(struct gps_device_t *session)
{
    gps_mask_t mask;
    PGN *work;

//  printf("NMEA2000 parse_input called\n");
    mask = 0;
    work = (PGN *) session->driver.nmea2000.workpgn;

    if (work != NULL) {
        mask = (work->func)(&session->packet.outbuffer[0], (int)session->packet.outbuflen, work, session);
        session->driver.nmea2000.workpgn = NULL;
    }
    session->packet.outbuflen = 0;

    return mask;
}
/*@+mustfreeonly@*/

/*@+nullassign@*/

#ifndef S_SPLINT_S

int nmea2000_open(struct gps_device_t *session)
{
    char interface_name[strlen(session->gpsdata.dev.path)];
    socket_t sock;
    int status;
    int unit_number;
    int can_net;
    unsigned int l;
    struct ifreq ifr;
    struct sockaddr_can addr;
    char *unit_ptr;

    INVALIDATE_SOCKET(session->gpsdata.gps_fd);

    session->driver.nmea2000.can_net = 0;
    can_net = -1;

    unit_number = -1;

    (void)strlcpy(interface_name, session->gpsdata.dev.path + 11, sizeof(interface_name));
    unit_ptr = NULL;
    for (l=0;l<strnlen(interface_name,sizeof(interface_name));l++) {
        if (interface_name[l] == ':') {
	    unit_ptr = &interface_name[l+1];
	    interface_name[l] = 0;
	    continue;
	}
	if (unit_ptr != NULL) {
	    if (isdigit(interface_name[l]) == 0) {
	        gpsd_report(LOG_ERROR, "NMEA2000 open: Invalid character in unit number.\n");
	        return -1;
	    }
	}
    }

    if (unit_ptr != NULL) {
        unit_number = atoi(unit_ptr);
	if ((unit_number < 0) || (unit_number > (NMEA2000_UNITS-1))) {
	    gpsd_report(LOG_ERROR, "NMEA2000 open: Unit number out of range.\n");
	    return -1;
	}
	for (l = 0; l < NMEA2000_NETS; l++) {
	    if (strncmp(can_interface_name[l], 
			interface_name,
			MIN(sizeof(interface_name), sizeof(can_interface_name[l]))) == 0) {
	        can_net = l;
		break;
	    }
	}
	if (can_net < 0) {
	    gpsd_report(LOG_ERROR, "NMEA2000 open: CAN device not open: %s .\n", interface_name);
	    return -1;
	}
    } else {
	for (l = 0; l < NMEA2000_NETS; l++) {
	    if (strncmp(can_interface_name[l], 
			interface_name,
			MIN(sizeof(interface_name), sizeof(can_interface_name[l]))) == 0) {
	        gpsd_report(LOG_ERROR, "NMEA2000 open: CAN device duplicate open: %s .\n", interface_name);
		return -1;
	    }
	}
	for (l = 0; l < NMEA2000_NETS; l++) {
	    if (can_interface_name[l][0] == 0) {
	        can_net = l;
		break;
	    }
	}
	if (can_net < 0) {
	    gpsd_report(LOG_ERROR, "NMEA2000 open: Too many CAN networks open.\n");
	    return -1;
	}
    }

    /* Create the socket */
    sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
 
    if (BAD_SOCKET(sock)) {
        gpsd_report(LOG_ERROR, "NMEA2000 open: can not get socket.\n");
	return -1;
    }

    status = fcntl(sock, F_SETFL, O_NONBLOCK);
    if (status != 0) {
        gpsd_report(LOG_ERROR, "NMEA2000 open: can not set socket to O_NONBLOCK.\n");
	close(sock);
	return -1;
    }

    /* Locate the interface you wish to use */
    strlcpy(ifr.ifr_name, interface_name, sizeof(ifr.ifr_name));
    status = ioctl(sock, SIOCGIFINDEX, &ifr); /* ifr.ifr_ifindex gets filled
					       * with that device's index */

    if (status != 0) {
        gpsd_report(LOG_ERROR, "NMEA2000 open: can not find CAN device.\n");
	close(sock);
	return -1;
    }

    /* Select that CAN interface, and bind the socket to it. */
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    status = bind(sock, (struct sockaddr*)&addr, sizeof(addr) );
    if (status != 0) {
        gpsd_report(LOG_ERROR, "NMEA2000 open: bind failed.\n");
	close(sock);
	return -1;
    }

    gpsd_switch_driver(session, "NMEA2000");
    session->gpsdata.gps_fd = sock;
    session->sourcetype = source_can;
    session->servicetype = service_sensor;
    session->driver.nmea2000.can_net = can_net;

    if (unit_ptr != NULL) {
        nmea2000_units[can_net][unit_number] = session;
	session->driver.nmea2000.unit = unit_number;
	session->driver.nmea2000.unit_valid = 1;
    } else {
        strncpy(can_interface_name[can_net],
		interface_name, 
		MIN(sizeof(can_interface_name[0]), sizeof(interface_name)));
	session->driver.nmea2000.unit_valid = 0;
	for (l=0;l<NMEA2000_UNITS;l++) {
	    nmea2000_units[can_net][l] = NULL;	  
	}
    }
    session->gpsdata.dev.parity = 'n';
    session->gpsdata.dev.baudrate = 250000;
    session->gpsdata.dev.stopbits = 0;
    return session->gpsdata.gps_fd;
}
#endif /* of ifndef S_SPLINT_S */

void nmea2000_close(struct gps_device_t *session)
{
    if (!BAD_SOCKET(session->gpsdata.gps_fd)) {
	gpsd_report(LOG_SPIN, "close(%d) in nmea2000_close(%s)\n",
		    session->gpsdata.gps_fd, session->gpsdata.dev.path);
	(void)close(session->gpsdata.gps_fd);
	INVALIDATE_SOCKET(session->gpsdata.gps_fd);
    }
}

/* *INDENT-OFF* */
const struct gps_type_t nmea2000 = {
    .type_name      = "NMEA2000",       /* full name of type */
    .packet_type    = NMEA2000_PACKET,	/* associated lexer packet type */
    .flags	    = DRIVER_NOFLAGS,	/* no rollover or other flags */
    .trigger	    = NULL,		/* detect their main sentence */
    .channels       = 12,		/* not an actual GPS at all */
    .probe_detect   = NULL,
    .get_packet     = nmea2000_get,	/* how to get a packet */
    .parse_packet   = nmea2000_parse_input,	/* how to interpret a packet */
    .rtcm_writer    = NULL,		/* Don't send RTCM to this */
    .event_hook     = NULL,
#ifdef RECONFIGURE_ENABLE
    .speed_switcher = NULL,		/* no speed switcher */
    .mode_switcher  = NULL,		/* no mode switcher */
    .rate_switcher  = NULL,		/* no rate switcher */
    .min_cycle      = 1,		/* nominal 1-per-second GPS cycle */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send   = NULL,		/* how to send control strings */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = NULL,
#endif /* NTPSHM_ ENABLE */
};
/* *INDENT-ON* */

/* end */

#endif /* of  defined(NMEA2000_ENABLE) */
