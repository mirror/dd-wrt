/*
 * This file contains two drivers for Garmin receivers and some code
 * shared by both drivers.
 *
 * One driver "garmin_usb_binary" handles the Garmin binary packet
 * format supported by the USB Garmins tested with the Garmin 18 and
 * other models.  (There is also "garmin_usb_binary_old".)  These are ONLY
 * for USB devices reporting as: 091e:0003.
 *
 * The other driver "garmin_ser_binary" is for Garmin receivers via a
 * serial port, whether or not one uses a USB/serial adaptor or a real
 * serial port.  These receivers provide adequate NMEA support, so it
 * often makes sense to just put them into NMEA mode.
 *
 * On Linux, USB Garmins (091e:0003) need the Linux garmin_gps driver and
 * will not function without it.  On other operating systems, it is clear
 * garmin_usb_binary_old does not work since it requires the Linux
 * garmin_gps module.
 *
 * This code has been tested and at least at one time is known to work on
 * big- and little-endian CPUs and 32 and 64 bit cpu modes.
 *
 *
 * Documentation for the Garmin protocols can be found via
 *   http://www.garmin.com/support/commProtocol.html
 * The file IOSDK.zip contains IntfSpec.pdf, which describes the
 * protocol in terms of Application, Link, and Physical.  This
 * identical file is also available at:
 *   http://www.garmin.com/support/pdf/iop_spec.pdf
 * An older version of iop_spec.pdf that describes only Serial Binary
 * is available at:
 *   http://vancouver-webpages.com/pub/peter/iop_spec.pdf
 * Information about the GPS 18
 *   http://www.garmin.com/manuals/425_TechnicalSpecification.pdf
 *
 * There is one physical link protocol for serial which uses DLE/ETX
 * framing.  There is another physical protocol for USB which relies
 * on the packetization intrinsic to USB bulk pipes.
 *
 * There are several link protocols; all devices implement L000.
 * There are then product-specific protocols; most devices implement
 * L001.  Link protocols are the same and carried over either Physical
 * protocol.
 *
 * Application protocols are named A000 and then with different
 * 3-digit numbers.  They are carried over Link protocols.
 *
 * Thus, much of the higher-level code dealing the data formats is
 * shared between USB Binary and Serial Binary.
 *
 * This code is partly from the Garmin IOSDK and partly from the
 * sample code in the Linux garmin_gps driver.
 *
 * bad code by: Gary E. Miller <gem@rellim.com>
 *
 * -D 3 = packet trace
 * -D 4 = packet details
 * -D 5 = more packet details
 * -D 6 = very excessive details
 *
 * limitations:
 *
 * do not have from garmin:
 *      pdop
 *      hdop
 *      vdop
 *	magnetic variation
 *
 * known bugs:
 *      hangs in the fread loop instead of keeping state and returning.
 *
 * TODO:
 *
 *	?? Add probe function for Serial Binary to start PVT output.
 *
 * This file is Copyright (c) 2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 */

#define __USE_POSIX199309 1

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#ifndef S_SPLINT_S
#include <unistd.h>
#endif /* S_SPLINT_S */

#include "gpsd_config.h"

#if defined(HAVE_LIBUSB)
#include <libusb.h>
#endif

#include "gpsd.h"
#include "bits.h"

#define GPSD_LE16TOH(x) getles16((char *)(&(x)), 0)
#define GPSD_LE32TOH(x) getles32((char *)(&(x)), 0)

#ifdef GARMIN_ENABLE

#define USE_RMD 0

/* Used in Serial Physical Layer */
#define ETX 0x03
#define ACK 0x06
#define DLE 0x10
#define NAK 0x15

#define GARMIN_LAYERID_TRANSPORT (uint8_t)  0
#define GARMIN_LAYERID_APPL      (uint32_t) 20
// Linux Garmin USB driver layer-id to use for some control mechanisms
#define GARMIN_LAYERID_PRIVATE  0x01106E4B

// packet ids used in private layer
#define PRIV_PKTID_SET_DEBUG    1
#define PRIV_PKTID_SET_MODE     2
#define PRIV_PKTID_INFO_REQ     3
#define PRIV_PKTID_INFO_RESP    4
#define PRIV_PKTID_RESET_REQ    5
#define PRIV_PKTID_SET_DEF_MODE 6

#define MODE_NATIVE	  0
#define MODE_GARMIN_SERIAL   1

#define GARMIN_PKTID_TRANSPORT_START_SESSION_REQ 5
#define GARMIN_PKTID_TRANSPORT_START_SESSION_RESP 6

#define GARMIN_PKTID_PROTOCOL_ARRAY     253
#define GARMIN_PKTID_PRODUCT_RQST       254
#define GARMIN_PKTID_PRODUCT_DATA       255
/* 0x29 ')' */
#define GARMIN_PKTID_RMD41_DATA	 41
/* 0x33 '3' */
#define GARMIN_PKTID_PVT_DATA	   51
/* 0x33 '4' */
#define GARMIN_PKTID_RMD_DATA	   52
/* 0x72 'r' */
#define GARMIN_PKTID_SAT_DATA	   114

#define GARMIN_PKTID_L001_XFER_CMPLT     12
#define GARMIN_PKTID_L001_COMMAND_DATA   10
#define GARMIN_PKTID_L001_DATE_TIME_DATA 14
#define GARMIN_PKTID_L001_RECORDS	27
#define GARMIN_PKTID_L001_WPT_DATA       35

#define	CMND_ABORT			 0
#define	CMND_START_PVT_DATA		 49
#define	CMND_STOP_PVT_DATA		 50
#define	CMND_START_RM_DATA		 110

#define MAX_BUFFER_SIZE 4096

#define GARMIN_CHANNELS	12

// something magic about 64, garmin driver will not return more than
// 64 at a time.  If you read less than 64 bytes the next read will
// just get the last of the 64 byte buffer.
#define ASYNC_DATA_SIZE 64


#pragma pack(1)
// This is the data format of the satellite data from the garmin USB
typedef struct
{
    uint8_t svid;
    uint16_t snr;		// 0 - 0xffff
    uint8_t elev;
    uint16_t azmth;
    uint8_t status;		// bit 0, has ephemeris, 1, has diff correction
    // bit 2 used in solution
    // bit 3??
} cpo_sat_data;

/* Garmin D800_Pvt_Datetype_Type */
/* packet type:  GARMIN_PKTID_PVT_DATA   52 */
/* This is the data format of the position data from the garmin USB */
typedef struct
{
    float alt;			/* altitude above WGS 84 (meters) */
    float epe;			/* estimated position error, 2 sigma (meters)  */
    float eph;			/* epe, but horizontal only (meters) */
    float epv;			/* epe but vertical only (meters ) */
    int16_t fix;		/* 0 - failed integrity check
				 * 1 - invalid or unavailable fix
				 * 2 - 2D
				 * 3 - 3D
				 * 4 - 2D Diff
				 * 5 - 3D Diff
				 */
    double gps_tow;		/* gps time of week (seconds) */
    double lat;			/* ->latitude (radians) */
    double lon;			/* ->longitude (radians) */
    float lon_vel;		/* velocity east (meters/second) */
    float lat_vel;		/* velocity north (meters/second) */
    float alt_vel;		/* velocity up (meters/sec) */
    // Garmin GPS25 uses pkt_id 0x28 and does not output the
    // next 3 items
    float msl_hght;		/* height of WGS 84 above MSL (meters) */
    int16_t leap_sec;		/* diff between GPS and UTC (seconds) */
    int32_t grmn_days;		/* days from UTC December 31st, 1989 to the
				 * beginning of the current week */
} cpo_pvt_data;

typedef struct
{
    uint32_t cycles;
    double pr;
    uint16_t phase;
    int8_t slp_dtct;
    uint8_t snr_dbhz;
    uint8_t svid;
    int8_t valid;
} cpo_rcv_sv_data;

/* packet type:  GARMIN_PKTID_RMD_DATA   53 */
/* seems identical to the packet id 0x29 from the Garmin GPS 25 */
typedef struct
{
    double rcvr_tow;
    int16_t rcvr_wn;
    cpo_rcv_sv_data sv[GARMIN_CHANNELS];
} cpo_rcv_data;

// This is the packet format to/from the Garmin USB
typedef struct
{
    uint8_t mPacketType;
    uint8_t mReserved1;
    uint16_t mReserved2;
    uint16_t mPacketId;
    uint16_t mReserved3;
    uint32_t mDataSize;
    union
    {
	int8_t chars[MAX_BUFFER_SIZE];
	uint8_t uchars[MAX_BUFFER_SIZE];
	cpo_pvt_data pvt;
	cpo_sat_data sats;
    } mData;
} Packet_t;

// useful funcs to read/write ints
//  floats and doubles are Intel (little-endian) order only...
static inline void set_int16(uint8_t * buf, uint32_t value)
{
    buf[0] = (uint8_t) (0x0FF & value);
    buf[1] = (uint8_t) (0x0FF & (value >> 8));
}

static inline void set_int32(uint8_t * buf, uint32_t value)
{
    buf[0] = (uint8_t) (0x0FF & value);
    buf[1] = (uint8_t) (0x0FF & (value >> 8));
    buf[2] = (uint8_t) (0x0FF & (value >> 16));
    buf[3] = (uint8_t) (0x0FF & (value >> 24));
}

static inline uint16_t get_uint16(const uint8_t * buf)
{
    return (uint16_t) (0xFF & buf[0])
	| ((uint16_t) (0xFF & buf[1]) << 8);
}

static inline uint32_t get_int32(const uint8_t * buf)
{
    return (uint32_t) (0xFF & buf[0])
	| ((uint32_t) (0xFF & buf[1]) << 8)
	| ((uint32_t) (0xFF & buf[2]) << 16)
	| ((uint32_t) (0xFF & buf[3]) << 24);
}

// convert radians to degrees
static inline double radtodeg(double rad)
{
    return (double)(rad * RAD_2_DEG);
}

static gps_mask_t PrintSERPacket(struct gps_device_t *session,
				 unsigned char pkt_id, int pkt_len,
				 unsigned char *buf);
static gps_mask_t PrintUSBPacket(struct gps_device_t *session,
				 Packet_t * pkt);

gps_mask_t PrintSERPacket(struct gps_device_t *session, unsigned char pkt_id,
			  int pkt_len, unsigned char *buf)
{

    gps_mask_t mask = 0;
    int i = 0, j = 0;
    uint16_t prod_id = 0;
    uint16_t ver = 0;
    int maj_ver;
    int min_ver;
    time_t time_l = 0;
    double track;
    char msg_buf[512] = "";
    char *msg = NULL;
    cpo_sat_data *sats = NULL;
    cpo_pvt_data *pvt = NULL;
    cpo_rcv_data *rmd = NULL;

    gpsd_report(LOG_IO, "Garmin: PrintSERPacket(, %#02x, %#02x, )\n", pkt_id,
		pkt_len);

    session->cycle_end_reliable = true;

    switch (pkt_id) {
    case ACK:
	gpsd_report(LOG_PROG, "Garmin: ACK\n");
	break;
    case NAK:
	gpsd_report(LOG_PROG, "Garmin: NAK\n");
	break;
    case GARMIN_PKTID_L001_COMMAND_DATA:
	prod_id = get_uint16((uint8_t *) buf);
	/*@ -branchstate @*/
	switch (prod_id) {
	case CMND_ABORT:
	    msg = "Abort current xfer";
	    break;
	case CMND_START_PVT_DATA:
	    msg = "Start Xmit PVT data";
	    break;
	case CMND_STOP_PVT_DATA:
	    msg = "Stop Xmit PVT data";
	    break;
	case CMND_START_RM_DATA:
	    msg = "Start RMD data";
	    break;
	default:
	    (void)snprintf(msg_buf, sizeof(msg_buf), "Unknown: %u",
			   (unsigned int)prod_id);
	    msg = msg_buf;
	    break;
	}
	/*@ +branchstate @*/
	gpsd_report(LOG_PROG, "Garmin: Appl, Command Data: %s\n", msg);
	break;
    case GARMIN_PKTID_PRODUCT_RQST:
	gpsd_report(LOG_PROG, "Garmin: Appl, Product Data req\n");
	break;
    case GARMIN_PKTID_PRODUCT_DATA:
	prod_id = get_uint16((uint8_t *) buf);
	ver = get_uint16((uint8_t *) & buf[2]);
	maj_ver = (int)(ver / 100);
	min_ver = (int)(ver - (maj_ver * 100));
	gpsd_report(LOG_PROG, "Garmin: Appl, Product Data, sz: %d\n",
		    pkt_len);
	(void)snprintf(session->subtype, sizeof(session->subtype),
		       "%d: %d.%02d", (int)prod_id, maj_ver, min_ver);
	gpsd_report(LOG_INF, "Garmin: Product ID: %d, SoftVer: %d.%02d\n",
		    prod_id, maj_ver, min_ver);
	gpsd_report(LOG_INF, "Garmin: Product Desc: %s\n", &buf[4]);
	mask |= DEVICEID_SET;
	gpsd_report(LOG_DATA, "Garmin: PRODUCT_DATA: subtype=%s\n",
		    session->subtype);
	break;
    case GARMIN_PKTID_PVT_DATA:
	gpsd_report(LOG_PROG, "Garmin: Appl, PVT Data Sz: %d\n", pkt_len);

	pvt = (cpo_pvt_data *) buf;

	// 631065600, unix seconds for 31 Dec 1989 Zulu
	time_l = (time_t) (631065600 + (GPSD_LE32TOH(pvt->grmn_days) * 86400));
	// TODO, convert grmn_days to context->gps_week
	time_l -= GPSD_LE16TOH(pvt->leap_sec);
	session->context->leap_seconds = (int)GPSD_LE16TOH(pvt->leap_sec);
	session->context->valid = LEAP_SECOND_VALID;
	// gps_tow is always like x.999 or x.998 so just round it
	time_l += (time_t) round(pvt->gps_tow);
	session->context->gps_tow = pvt->gps_tow;
	session->newdata.time = (timestamp_t)time_l;
	gpsd_report(LOG_PROG, "Garmin: time_l: %ld\n", (long int)time_l);

	session->newdata.latitude = radtodeg(pvt->lat);
	/* sanity check the lat */
	if (90.0 < session->newdata.latitude) {
	    session->newdata.latitude = 90.0;
	    gpsd_report(LOG_INF, "Garmin: ERROR: Latitude overrange\n");
	} else if (-90.0 > session->newdata.latitude) {
	    session->newdata.latitude = -90.0;
	    gpsd_report(LOG_INF,
			"Garmin: ERROR: Latitude negative overrange\n");
	}
	session->newdata.longitude = radtodeg(pvt->lon);
	/* sanity check the lon */
	if (180.0 < session->newdata.longitude) {
	    session->newdata.longitude = 180.0;
	    gpsd_report(LOG_INF, "Garmin: ERROR: Longitude overrange\n");
	} else if (-180.0 > session->newdata.longitude) {
	    session->newdata.longitude = -180.0;
	    gpsd_report(LOG_INF,
			"Garmin: ERROR: Longitude negative overrange\n");
	}
	// altitude over WGS84 converted to MSL
	session->newdata.altitude = pvt->alt + pvt->msl_hght;

	// geoid separation from WGS 84
	// gpsd sign is opposite of garmin sign
	session->gpsdata.separation = -pvt->msl_hght;

	// Estimated position error in meters.
	// We follow the advice at <http://gpsinformation.net/main/errors.htm>.
	// If this assumption changes here, it should also change in
	// nmea_parse.c where we analyze PGRME.
	session->gpsdata.epe = pvt->epe * (GPSD_CONFIDENCE / CEP50_SIGMA);
	/* eph is a circular error, sqrt(epx**2 + epy**2) */
	session->newdata.epx = session->newdata.epy =
	    pvt->eph * (1 / sqrt(2)) * (GPSD_CONFIDENCE / CEP50_SIGMA);
	session->newdata.epv = pvt->epv * (GPSD_CONFIDENCE / CEP50_SIGMA);

	// convert lat/lon to directionless speed
	session->newdata.speed = hypot(pvt->lon_vel, pvt->lat_vel);

	// keep climb in meters/sec
	session->newdata.climb = pvt->alt_vel;

	track = atan2(pvt->lon_vel, pvt->lat_vel);
	if (track < 0) {
	    track += 2 * GPS_PI;
	}
	session->newdata.track = radtodeg(track);

	switch (GPSD_LE16TOH(pvt->fix)) {
	case 0:
	case 1:
	default:
	    // no fix
	    session->gpsdata.status = STATUS_NO_FIX;
	    session->newdata.mode = MODE_NO_FIX;
	    break;
	case 2:
	    // 2D fix
	    session->gpsdata.status = STATUS_FIX;
	    session->newdata.mode = MODE_2D;
	    break;
	case 3:
	    // 3D fix
	    session->gpsdata.status = STATUS_FIX;
	    session->newdata.mode = MODE_3D;
	    break;
	case 4:
	    // 2D Differential fix
	    session->gpsdata.status = STATUS_DGPS_FIX;
	    session->newdata.mode = MODE_2D;
	    break;
	case 5:
	    // 3D differential fix
	    session->gpsdata.status = STATUS_DGPS_FIX;
	    session->newdata.mode = MODE_3D;
	    break;
	}

	gpsd_report(LOG_PROG, "Garmin: Appl, mode %d, status %d\n",
		    session->newdata.mode, session->gpsdata.status);

	gpsd_report(LOG_INF, "Garmin: UTC Time: %lf\n",
		    session->newdata.time);
	gpsd_report(LOG_INF,
		    "Garmin: Geoid Separation (MSL-WGS84): from garmin %lf, calculated %lf\n",
		    -pvt->msl_hght,
		    wgs84_separation(session->newdata.latitude,
				     session->newdata.longitude));

	gpsd_report(LOG_INF,
		    "Garmin: Alt: %.3f, Epe: %.3f, Eph: %.3f, Epv: %.3f, Fix: %d, Gps_tow: %f, Lat: %.3f, Lon: %.3f, LonVel: %.3f, LatVel: %.3f, AltVel: %.3f, MslHgt: %.3f, Leap: %d, GarminDays: %d\n",
		    pvt->alt, pvt->epe, pvt->eph, pvt->epv, GPSD_LE16TOH(pvt->fix),
		    pvt->gps_tow, session->newdata.latitude,
		    session->newdata.longitude, pvt->lon_vel, pvt->lat_vel,
		    pvt->alt_vel, pvt->msl_hght, GPSD_LE16TOH(pvt->leap_sec),
		    GPSD_LE32TOH(pvt->grmn_days));

	if (session->newdata.mode > MODE_NO_FIX) {
	    /* data only valid with a fix */
	    mask |=
		TIME_SET | LATLON_SET | ALTITUDE_SET | STATUS_SET | MODE_SET |
		SPEED_SET | TRACK_SET | CLIMB_SET | HERR_SET | VERR_SET | PERR_IS |
		CLEAR_IS | REPORT_IS;
	    /*
	     * Garmin documentation says we should wait until four good fixes
	     * have been seen before trying to use the device for precision
	     * time service.
	     */
	    if (session->fixcnt > 3)
		mask |= PPSTIME_IS;
	}
	gpsd_report(LOG_DATA,
		    "Garmin: PVT_DATA: time=%.2f, lat=%.2f lon=%.2f "
		    "speed=%.2f track=%.2f climb=%.2f "
		    "epx=%.2f epy=%.2f epv=%.2f "
		    "mode=%d status=%d\n",
		    session->newdata.time,
		    session->newdata.latitude,
		    session->newdata.longitude,
		    session->newdata.speed,
		    session->newdata.track,
		    session->newdata.climb,
		    session->newdata.epx,
		    session->newdata.epy,
		    session->newdata.epv,
		    session->newdata.mode,
		    session->gpsdata.status);
	break;
    case GARMIN_PKTID_RMD_DATA:
    case GARMIN_PKTID_RMD41_DATA:
	rmd = (cpo_rcv_data *) buf;
	gpsd_report(LOG_IO, "Garmin: PVT RMD Data Sz: %d\n", pkt_len);
	gpsd_report(LOG_PROG, "Garmin: PVT RMD rcvr_tow: %f, rcvr_wn: %d\n",
		    rmd->rcvr_tow, rmd->rcvr_wn);
	for (i = 0; i < GARMIN_CHANNELS; i++) {
	    gpsd_report(LOG_INF,
			"Garmin: PVT RMD Sat: %3u, cycles: %9u, pr: %16.6f, "
			"phase: %7.3f, slp_dtct: %3s, snr: %3u, Valid: %3s\n",
			(int)rmd->sv[i].svid + 1,
			GPSD_LE32TOH(rmd->sv[i].cycles),
			rmd->sv[i].pr,
			(GPSD_LE16TOH(rmd->sv[i].phase) * 360.0) / 2048.0,
			rmd->sv[i].slp_dtct != 0 ? "Yes" : "No",
			rmd->sv[i].snr_dbhz,
			rmd->sv[i].valid != 0 ? "Yes" : "No");
	}
	break;

    case GARMIN_PKTID_SAT_DATA:
	gpsd_report(LOG_PROG, "Garmin: SAT Data Sz: %d\n", pkt_len);
	sats = (cpo_sat_data *) buf;

	session->gpsdata.satellites_visible = 0;
	session->gpsdata.satellites_used = 0;
	memset(session->gpsdata.used, 0, sizeof(session->gpsdata.used));
	gpsd_zero_satellites(&session->gpsdata);
	for (i = 0, j = 0; i < GARMIN_CHANNELS; i++, sats++) {
	    gpsd_report(LOG_INF,
			"Garmin:   Sat %3d, snr: %5u, elev: %2d, Azmth: %3d, Stat: %x\n",
			sats->svid, GPSD_LE16TOH(sats->snr), sats->elev,
			GPSD_LE16TOH(sats->azmth),
			sats->status);

	    if (255 == (int)sats->svid) {
		// Garmin uses 255 for empty
		// gpsd uses 0 for empty
		continue;
	    }

	    if ((int)sats->svid <= 32)
		session->gpsdata.PRN[j] = (int)sats->svid;	/* GPS */
	    else
		session->gpsdata.PRN[j] = (int)sats->svid + 87;	/* SBAS */
	    session->gpsdata.azimuth[j] = (int)GPSD_LE16TOH(sats->azmth);
	    session->gpsdata.elevation[j] = (int)sats->elev;
	    // Garmin does not document this.  snr is in dB*100
	    // Known, but not seen satellites have a dB value of -1*100
	    session->gpsdata.ss[j] = (float)(GPSD_LE16TOH(sats->snr) / 100.0);
	    if (session->gpsdata.ss[j] == -1) {
		continue;
	    }
	    // FIX-ME: Garmin documents this, but Daniel Dorau
	    // <daniel.dorau@gmx.de> says the behavior on his GPSMap60CSX
	    // doesn't match it.
	    if ((uint8_t) 0 != (sats->status & 4)) {
		// used in solution?
		session->gpsdata.used[session->gpsdata.satellites_used++]
		    = session->gpsdata.PRN[j];
	    }
	    session->gpsdata.satellites_visible++;
	    j++;

	}
	session->gpsdata.skyview_time = NAN;
	if (session->gpsdata.satellites_visible > 0)
	    mask |= SATELLITE_SET;
	if (session->gpsdata.satellites_used > 0)
	    mask |= USED_IS;
	gpsd_report(LOG_DATA,
		    "Garmin: SAT_DATA: visible=%d used=%d\n",
		    session->gpsdata.satellites_visible,
		    session->gpsdata.satellites_used);
	break;
    case GARMIN_PKTID_PROTOCOL_ARRAY:
	// this packet is never requested, it just comes, in some case
	// after a GARMIN_PKTID_PRODUCT_RQST
	gpsd_report(LOG_INF, "Garmin: Appl, Product Capability, sz: %d\n",
		    pkt_len);
	for (i = 0; i < pkt_len; i += 3) {
	    gpsd_report(LOG_INF, "Garmin:   %c%03d\n", buf[i],
			get_uint16((uint8_t *) & buf[i + 1]));
	}
	break;
    default:
	gpsd_report(LOG_WARN,
		    "Garmin: Unknown packet id: %#02x, Sz: %#02x\n",
		    pkt_id, pkt_len);
	break;
    }
    gpsd_report(LOG_IO, "Garmin: PrintSERPacket(, %#02x, %#02x, )\n",
		pkt_id, pkt_len);
    return mask;
}


/*@ -branchstate @*/
// For debugging, decodes and prints some known packets.
static gps_mask_t PrintUSBPacket(struct gps_device_t *session, Packet_t * pkt)
{
    gps_mask_t mask = 0;
    int maj_ver;
    int min_ver;
    uint32_t mode = 0;
    uint16_t prod_id = 0;
    uint32_t veri = 0;
    uint32_t serial;
    uint32_t mDataSize = get_int32((uint8_t *) & pkt->mDataSize);

//
    uint8_t *buffer = (uint8_t *) pkt;

    gpsd_report(LOG_PROG, "Garmin: PrintUSBPacket()\n");
// gem
    if (DLE == pkt->mPacketType) {
	gpsd_report(LOG_PROG, "Garmin: really a SER packet!\n");
	return PrintSERPacket(session,
			      (unsigned char)buffer[1],
			      (int)buffer[2], (unsigned char *)(buffer + 3));
    }
// gem
    if (4096 < mDataSize) {
	gpsd_report(LOG_WARN, "Garmin: bogus packet, size too large=%d\n",
		    mDataSize);
	return 0;
    }

    (void)snprintf(session->gpsdata.tag, sizeof(session->gpsdata.tag), "%u",
		   (unsigned int)pkt->mPacketType);
    switch (pkt->mPacketType) {
    case GARMIN_LAYERID_TRANSPORT:
	/* Garmin USB layer specific */
	switch (pkt->mPacketId) {
	case GARMIN_PKTID_TRANSPORT_START_SESSION_REQ:
	    gpsd_report(LOG_PROG, "Garmin: Transport, Start Session req\n");
	    break;
	case GARMIN_PKTID_TRANSPORT_START_SESSION_RESP:
	    mode = get_int32(&pkt->mData.uchars[0]);
	    gpsd_report(LOG_PROG,
			"Garmin: Transport, Start Session resp, unit: 0x%x\n",
			mode);
	    break;
	default:
	    gpsd_report(LOG_PROG,
			"Garmin: Transport, Packet: Type %d %d %d, ID: %d, Sz: %d\n",
			pkt->mPacketType, pkt->mReserved1, pkt->mReserved2,
			pkt->mPacketId, mDataSize);
	    break;
	}
	break;
    case GARMIN_LAYERID_APPL:
	/* raw data transport, shared with Garmin Serial Driver */

	mask = PrintSERPacket(session,
			      (unsigned char)pkt->mPacketId,
			      (int)mDataSize,
			      (unsigned char *)pkt->mData.uchars);
	break;
    case 75:
	// private, garmin USB kernel driver specific
	switch (pkt->mPacketId) {
	case PRIV_PKTID_SET_MODE:
	    prod_id = get_uint16(&pkt->mData.uchars[0]);
	    gpsd_report(LOG_PROG, "Garmin: Private, Set Mode: %d\n", prod_id);
	    break;
	case PRIV_PKTID_INFO_REQ:
	    gpsd_report(LOG_PROG, "Garmin: Private, ID: Info Req\n");
	    break;
	case PRIV_PKTID_INFO_RESP:
	    veri = get_int32(pkt->mData.uchars);
	    maj_ver = (int)(veri >> 16);
	    min_ver = (int)(veri & 0xffff);
	    mode = get_int32(&pkt->mData.uchars[4]);
	    serial = get_int32(&pkt->mData.uchars[8]);
	    gpsd_report(LOG_PROG, "Garmin: Private, ID: Info Resp\n");
	    gpsd_report(LOG_INF,
			"Garmin: USB Driver found, Version %d.%d, Mode: %d, GPS Serial# %u\n",
			maj_ver, min_ver, mode, serial);
	    break;
	default:
	    gpsd_report(LOG_PROG, "Garmin: Private, Packet: ID: %d, Sz: %d\n",
			pkt->mPacketId, mDataSize);
	    break;
	}
	break;
    default:
	gpsd_report(LOG_PROG,
		    "Garmin: Packet: Type %d %d %d, ID: %d, Sz: %d\n",
		    pkt->mPacketType, pkt->mReserved1, pkt->mReserved2,
		    pkt->mPacketId, mDataSize);
	break;
    }

    return mask;
}

/*@ +branchstate @*/


#if defined(__linux__) || defined(S_SPLINT_S)
/* build and send a packet w/ USB protocol */
static void Build_Send_USB_Packet(struct gps_device_t *session,
				  uint32_t layer_id, uint32_t pkt_id,
				  uint32_t length, uint32_t data)
{
    uint8_t *buffer = (uint8_t *) session->driver.garmin.Buffer;
    Packet_t *thePacket = (Packet_t *) buffer;
    ssize_t theBytesReturned = 0;
    ssize_t theBytesToWrite = 12 + (ssize_t) length;

    set_int32(buffer, layer_id);
    set_int32(buffer + 4, pkt_id);
    set_int32(buffer + 8, length);
    if (2 == length) {
	set_int16(buffer + 12, data);
    } else if (4 == length) {
	set_int32(buffer + 12, data);
    }
    (void)PrintUSBPacket(session, thePacket);

    theBytesReturned = gpsd_write(session, (const char *)thePacket,
				  (size_t) theBytesToWrite);
    gpsd_report(LOG_IO, "Garmin: SendPacket(), wrote %zd bytes\n",
		theBytesReturned);

    // Garmin says:
    // If the packet size was an exact multiple of the USB packet
    // size, we must make a final write call with no data

    // as a practical matter no known packets are 64 bytes long so
    // this is untested

    // So here goes just in case
    if (0 == (theBytesToWrite % ASYNC_DATA_SIZE)) {
	char *n = "";
	(void)gpsd_write(session, n, 0);
    }
}
#endif /* __linux__ || S_SPLINT_S */

/* build and send a packet in serial protocol */
/* layer_id unused */
// FIX-ME: This should go through the common message buffer someday
static void Build_Send_SER_Packet(struct gps_device_t *session,
				  uint32_t layer_id UNUSED, uint32_t pkt_id,
				  uint32_t length, uint32_t data)
{
    uint8_t *buffer = (uint8_t *) session->driver.garmin.Buffer;
    uint8_t *buffer0 = buffer;
    Packet_t *thePacket = (Packet_t *) buffer;
    ssize_t theBytesReturned = 0;
    ssize_t theBytesToWrite = 6 + (ssize_t) length;
    uint8_t chksum = 0;

    *buffer++ = (uint8_t) DLE;
    *buffer++ = (uint8_t) pkt_id;
    chksum = pkt_id;
    *buffer++ = (uint8_t) length;
    chksum += length;
    /* ??? What is this doing? */
    if (2 == length) {
	/* carefull!  no DLE stuffing here! */
	set_int16(buffer, data);
	chksum += buffer[0];
	chksum += buffer[1];
    } else if (4 == length) {
	/* carefull!  no DLE stuffing here! */
	set_int32(buffer, data);
	chksum += buffer[0];
	chksum += buffer[1];
	chksum += buffer[2];
	chksum += buffer[3];
    }
    /* ??? How is data copied to the buffer? */
    buffer += length;

    // Add checksum
    *buffer++ = -chksum;
    if (DLE == -chksum) {
	/* stuff another DLE */
	*buffer++ = (uint8_t) DLE;
	theBytesToWrite++;
    }
    // Add DLE, ETX
    *buffer++ = (uint8_t) DLE;
    /* we used to say n++ here, but scan-build complains */
    *buffer = (uint8_t) ETX;

    (void)PrintSERPacket(session,
			 (unsigned char)buffer0[1],
			 (int)buffer0[2], (unsigned char *)(buffer0 + 3));

    theBytesReturned = gpsd_write(session, (const char *)thePacket,
				  (size_t) theBytesToWrite);
    gpsd_report(LOG_IO, "Garmin: SendPacket(), wrote %zd bytes\n",
		theBytesReturned);

}

#if defined(HAVE_LIBUSB)
/*
 * is_usb_device() - is a specified device USB matching given vendor/product?
 *
 * BUG: Doesn't actually match against path yet. Must finish this function
 * by querying /sys/dev/char, either directly or using libudev. Greg KH
 * assures this is possible, though he is vague about how.
 *
 * libudev: http://www.kernel.org/pub/linux/utils/kernel/hotplug/libudev/
 */
/*@-compdef -usedef -nullpass@*/
// cppcheck-suppress unusedFunction
static bool is_usb_device(const char *path UNUSED, int vendor, int product)
{
    // discover devices
    libusb_device **list;
    ssize_t cnt;
    ssize_t i = 0;
    bool found = false;

    gpsd_report(LOG_INF, "attempting USB device enumeration.\n");
    (void)libusb_init(NULL);

    if ((cnt = libusb_get_device_list(NULL, &list)) < 0) {
	gpsd_report(LOG_ERROR, "USB device list call failed.\n");
	libusb_exit(NULL);
	return false;
    }

    for (i = 0; i < cnt; i++) {
	struct libusb_device_descriptor desc;
	libusb_device *dev = list[i];

	int r = libusb_get_device_descriptor(dev, &desc);
	if (r < 0) {
	    gpsd_report(LOG_ERROR,
			"USB descriptor fetch failed on device %zd.\n", i);
	    continue;
	}

	/* we can extract device descriptor data */
	gpsd_report(LOG_INF, "%04x:%04x (bus %d, device %d)\n",
		    desc.idVendor, desc.idProduct,
		    libusb_get_bus_number(dev),
		    libusb_get_device_address(dev));

	/* we match if vendor and product ID are right */
	if (desc.idVendor == (uint16_t)vendor && desc.idProduct == (uint16_t)product) {
	    found = true;
	    break;
	}
    }

    gpsd_report(LOG_SHOUT, "vendor/product match with %04x:%04x %sfound\n",
		vendor, product, found ? "" : "not ");
    libusb_free_device_list(list, 1);
    libusb_exit(NULL);
    return found;
}

/*@-compdef -usedef -nullpass@*/
#endif /* HAVE_LIBUSB */

/*
 * garmin_usb_detect() - detect a Garmin USB device connected to session fd.
 *
 * This is ONLY for USB devices reporting as: 091e:0003.
 *
 * This driver ONLY works in Linux and ONLY when the garmin_gps kernel
 * module is installed.
 *
 * This is only necessary because under Linux Garmin USB devices need a
 * kernel module rather than being normal USB-serial devices.
 *
 * The actual wire protocol from the Garmin device is very strange. There
 * are no delimiters.  End of packet is signaled by a zero-length read
 * on the USB device, and start of packet is the next read.  You can't just
 * ignore the zero reads and pass the data through - you'd never be able
 * to tell where the packet boundaries are.
 *
 * The garmin_usb module's job is to grab the packet and frame it in
 * DLEs (with DLE stuffing).  This makes the USB packets look as
 * though they came from a regular Garmin *serial* device, which is how
 * most of the processing for both types can be unified here.
 *
 * return 1 is device found
 * return 0 if not
 */
static bool garmin_usb_detect(struct gps_device_t *session UNUSED)
{
#if defined(__linux__) || defined(S_SPLINT_S)
    /*
     * Only perform this check if we're looking at a USB-serial
     * device.  This prevents drivers for attached serial GPSes
     * fronm being rudely elbowed aside by this one if they happen
     * to be trying to coexist with the Garmin.
     */
    if (session->sourcetype != source_usb)
	return false;
    else {
#ifdef HAVE_LIBUSB
	if (!is_usb_device(session->gpsdata.dev.path, 0x091e, 0x0003))
	    return false;

	if (!gpsd_set_raw(session)) {
	    gpsd_report(LOG_ERROR,
			"Garmin: garmin_usb_detect: error changing port attributes: %s\n",
			strerror(errno));
	    return false;
	}

	if (sizeof(session->driver.garmin.Buffer) < sizeof(Packet_t)) {
	    /* dunno how this happens, but it does on some compilers */
	    gpsd_report(LOG_ERROR,
			"Garmin: garmin_usb_detect: Compile error, garmin.Buffer too small.\n");
	    return false;
	}

	// FIXME!!! needs to use libusb totally and move garmin_gps aside */
	// set Mode 1, mode 0 is broken somewhere past 2.6.14
	// but how?
	gpsd_report(LOG_PROG, "Garmin: Set garmin_gps driver mode = 0\n");
	Build_Send_USB_Packet(session, GARMIN_LAYERID_PRIVATE,
			      PRIV_PKTID_SET_MODE, 4, MODE_GARMIN_SERIAL);
	// expect no return packet !?

	return true;
#else
	return false;
#endif /* HAVE_LIBUSB */
    }
#else
    return false;
#endif /* __linux__ || S_SPLINT_S */
}

static void garmin_event_hook(struct gps_device_t *session, event_t event)
{
    if (session->context->readonly)
	return;
    /*
     * FIX-ME: It might not be necessary to call this on reactivate.
     * Experiment to see if the holds its settings through a close.
     */
    if (event == event_identified || event == event_reactivate) {
	// Tell the device to send product data
	gpsd_report(LOG_PROG, "Garmin: Get Product Data\n");
	Build_Send_SER_Packet(session, GARMIN_LAYERID_APPL,
			      GARMIN_PKTID_PRODUCT_RQST, 0, 0);

	// turn on PVT data 49
	gpsd_report(LOG_PROG, "Garmin: Set to send reports every 1 second\n");

	Build_Send_SER_Packet(session, GARMIN_LAYERID_APPL,
			      GARMIN_PKTID_L001_COMMAND_DATA, 2,
			      CMND_START_PVT_DATA);

#if USE_RMD
	// turn on RMD data 110
	gpsd_report(LOG_PROG, "Garmin: Set to send Raw sat data\n");
	Build_Send_SER_Packet(session, GARMIN_LAYERID_APPL,
			      GARMIN_PKTID_L001_COMMAND_DATA, 2,
			      CMND_START_RM_DATA);
#endif
    }
    if (event == event_deactivate)
	/* FIX-ME: is any action needed, or is closing the port sufficient? */
	gpsd_report(LOG_PROG, "Garmin: garmin_close()\n");
}

#define Send_ACK()    Build_Send_SER_Packet(session, 0, ACK, 0, 0)
#define Send_NAK()    Build_Send_SER_Packet(session, 0, NAK, 0, 0)

/*@ +charint @*/
gps_mask_t garmin_ser_parse(struct gps_device_t *session)
{
    unsigned char *buf = session->packet.outbuffer;
    size_t len = session->packet.outbuflen;
    unsigned char data_buf[MAX_BUFFER_SIZE];
    unsigned char c;
    int i = 0;
    size_t n = 0;
    int data_index = 0;
    int got_dle = 0;
    unsigned char pkt_id = 0;
    unsigned char pkt_len = 0;
    unsigned char chksum = 0;
    gps_mask_t mask = 0;

    if (session->packet.type == GARMIN_PACKET)
	session->gpsdata.dev.driver_mode = MODE_BINARY;

    gpsd_report(LOG_RAW, "Garmin: garmin_ser_parse()\n");
    if (6 > len) {
	/* WTF? */
	/* minimum packet; <DLE> [pkt id] [length=0] [chksum] <DLE> <STX> */
	Send_NAK();
	gpsd_report(LOG_RAW + 1, "Garmin: serial too short: %zd\n", len);
	return 0;
    }
    /* debug */
    for (i = 0; i < (int)len; i++) {
	gpsd_report(LOG_RAW + 1, "Garmin: Char: %#02x\n", buf[i]);
    }

    if ('\x10' != buf[0]) {
	Send_NAK();
	gpsd_report(LOG_RAW + 1, "Garmin: buf[0] not DLE\n");
	return 0;
    }
    n = 1;
    pkt_id = buf[n++];
    chksum = pkt_id;
    if ('\x10' == pkt_id) {
	if ('\x10' != buf[n++]) {
	    Send_NAK();
	    gpsd_report(LOG_RAW + 1, "Garmin: Bad pkt_id %#02x\n", pkt_id);
	    return 0;
	}
    }

    pkt_len = buf[n++];
    chksum += pkt_len;
    if ('\x10' == pkt_len) {
	if ('\x10' != buf[n++]) {
	    gpsd_report(LOG_RAW + 1, "Garmin: Bad pkt_len %#02x\n", pkt_len);
	    Send_NAK();
	    return 0;
	}
    }
    data_index = 0;
    for (i = 0; i < 256; i++) {

	if ((int)pkt_len == data_index) {
	    // got it all
	    break;
	}
	if (len < n + i) {
	    gpsd_report(LOG_RAW + 1, "Garmin: Packet too short %zd < %zd\n",
			len, n + i);
	    Send_NAK();
	    return 0;
	}
	c = buf[n + i];
	if (got_dle) {
	    got_dle = 0;
	    if ('\x10' != c) {
		Send_NAK();
		gpsd_report(LOG_RAW + 1, "Garmin: Bad DLE %#02x\n", c);
		return 0;
	    }
	} else {
	    chksum += c;
	    data_buf[data_index++] = c;
	    if ('\x10' == c) {
		got_dle = 1;
	    }
	}
    }
    /* get checksum */
    if (len < n + i) {
	Send_NAK();
	gpsd_report(LOG_RAW + 1,
		    "Garmin: No checksum, Packet too short %zd < %zd\n", len,
		    n + i);
	return 0;
    }
    c = buf[n + i++];
    chksum += c;
    /* get final DLE */
    if (len < n + i) {
	Send_NAK();
	gpsd_report(LOG_RAW + 1,
		    "Garmin: No final DLE, Packet too short %zd < %zd\n", len,
		    n + i);
	return 0;
    }
    c = buf[n + i++];
    if ('\x10' != c) {
	Send_NAK();
	gpsd_report(LOG_RAW + 1, "Garmin: Final DLE not DLE\n");
	return 0;
    }
    /* get final ETX */
    if (len < n + i) {
	Send_NAK();
	gpsd_report(LOG_RAW + 1,
		    "Garmin: No final ETX, Packet too short %zd < %zd\n", len,
		    n + i);
	return 0;
    }
    /* we used to say n++ here, but scan-build complains */
    c = buf[n + i];
    if ('\x03' != c) {
	Send_NAK();
	gpsd_report(LOG_RAW + 1, "Garmin: Final ETX not ETX\n");
	return 0;
    }

    /* debug */
    /*@ -usedef -compdef @*/
    for (i = 0; i < data_index; i++) {
	gpsd_report(LOG_RAW + 1, "Garmin: Char: %#02x\n", data_buf[i]);
    }


    gpsd_report(LOG_IO,
		"Garmin: garmin_ser_parse() Type: %#02x, Len: %#02x, chksum: %#02x\n",
		pkt_id, pkt_len, chksum);
    mask = PrintSERPacket(session, pkt_id, pkt_len, data_buf);

    // sending ACK too soon might hang the session
    // so send ACK last, after a pause
    (void)usleep(300);
    Send_ACK();
    /*@ +usedef +compdef @*/
    gpsd_report(LOG_IO, "Garmin: garmin_ser_parse( )\n");
    return mask;
}

/*@ -charint @*/

#ifdef RECONFIGURE_ENABLE
static void settle(void)
{
    struct timespec delay, rem;
    /*@ -type -unrecog @*/
    memset(&delay, 0, sizeof(delay));
    delay.tv_sec = 0;
    delay.tv_nsec = 333000000L;
    nanosleep(&delay, &rem);
    /*@ +type +unrecog @*/
}

static void garmin_switcher(struct gps_device_t *session, int mode)
{
    if (mode == MODE_NMEA) {
	/*@ +charint @*/
	const char switcher[] =
	    { 0x10, 0x0A, 0x02, 0x26, 0x00, 0xCE, 0x10, 0x03 };
	// Note hard-coded string length in the next line...
	ssize_t status = gpsd_write(session, switcher, sizeof(switcher));
	/*@ -charint @*/
	if (status == (ssize_t)sizeof(switcher)) {
	    gpsd_report(LOG_IO,
			"Garmin: => GPS: turn off binary %02x %02x %02x... \n",
			switcher[0], switcher[1], switcher[2]);
	    session->gpsdata.dev.driver_mode = MODE_NMEA;
	} else {
	    gpsd_report(LOG_ERROR, "Garmin: => GPS: FAILED\n");
	}
	settle();		// wait 333mS, essential!

	/* once a sec, no binary, no averaging, NMEA 2.3, WAAS */
	(void)nmea_send(session, "$PGRMC1,1,1");
	//(void)nmea_send(fd, "$PGRMC1,1,1,1,,,,2,W,N");
	(void)nmea_send(session, "$PGRMI,,,,,,,R");
	settle();		// wait 333mS, essential!
    } else {
	(void)nmea_send(session, "$PGRMC1,1,2,1,,,,2,W,N");
	(void)nmea_send(session, "$PGRMI,,,,,,,R");
	session->gpsdata.dev.driver_mode = MODE_BINARY;
	settle();		// wait 333mS, essential!
    }
}
#endif /* RECONFIGURE_ENABLE */

#ifdef CONTROLSEND_ENABLE
static ssize_t garmin_control_send(struct gps_device_t *session,
				   char *buf, size_t buflen)
/* not used by the daemon, it's for gpsctl and friends */
{
    /*@ -mayaliasunique @*/
    session->msgbuflen = buflen;
    (void)memcpy(session->msgbuf, buf, buflen);
    return gpsd_write(session, session->msgbuf, session->msgbuflen);
    /*@ +mayaliasunique @*/
}
#endif /* CONTROLSEND_ENABLE */

#ifdef NTPSHM_ENABLE
static double garmin_ntp_offset(struct gps_device_t *session)
{
    if (session->sourcetype == source_usb) {
	return 0.035;		/* Garmin USB, expect +/- 40mS jitter */
    }
    /* only two sentences ships time */
    /* but the PVT data is always first */
    switch (session->gpsdata.dev.baudrate) {
    case 4800:
	return 0.430;		/* TBD */
    case 9600:
	return 0.430;		/* tested 12Arp10 */
    case 19200:
	return 0.430;		/* TBD */
    case 38400:
	return 0.430;		/* TBD */
    }
    return 0.430;		/* WTF?  WAG */
}
#endif /* NTPSHM_ENABLE */

/* this is everything we export */
#ifdef __UNUSED__
static int GetPacket(struct gps_device_t *session);
//-----------------------------------------------------------------------------
// Gets a single packet.
// this is odd, the garmin usb driver will only return 64 bytes, or less
// at a time, no matter what you ask for.
//
// is you ask for less than 64 bytes then the next packet will include
// just the remaining bytes of the last 64 byte packet.
//
// Reading a packet of length Zero, or less than 64, signals the end of
// the entire packet.
//
// The Garmin sample WinXX code also assumes the same behavior, so
// maybe it is something in the USB protocol.
//
// Return: 0 = got a good packet
//       -1 = error
//       1 = got partial packet
static int GetPacket(struct gps_device_t *session)
{
    struct timespec delay, rem;
    int cnt = 0;
    // int x = 0; // for debug dump

    memset(session->driver.garmin.Buffer, 0, sizeof(Packet_t));
    memset(&delay, 0, sizeof(delay));
    session->driver.garmin.BufferLen = 0;
    session->packet.outbuflen = 0;

    gpsd_report(LOG_IO, "Garmin: GetPacket()\n");

    for (cnt = 0; cnt < 10; cnt++) {
	size_t pkt_size;
	// Read async data until the driver returns less than the
	// max async data size, which signifies the end of a packet

	// not optimal, but given the speed and packet nature of
	// the USB not too bad for a start
	ssize_t theBytesReturned = 0;
	uint8_t *buf = (uint8_t *) session->driver.garmin.Buffer;
	Packet_t *thePacket = (Packet_t *) buf;

	theBytesReturned =
	    read(session->gpsdata.gps_fd,
		 buf + session->driver.garmin.BufferLen, ASYNC_DATA_SIZE);
	// zero byte returned is a legal value and denotes the end of a
	// binary packet.
	if (0 > theBytesReturned) {
	    // read error...
	    // or EAGAIN, but O_NONBLOCK is never set
	    gpsd_report(LOG_ERROR,
			"Garmin: GetPacket() read error=%d, errno=%d\n",
			theBytesReturned, errno);
	    continue;
	}
	gpsd_report(LOG_RAW, "Garmin: got %d bytes\n", theBytesReturned);

	session->driver.garmin.BufferLen += theBytesReturned;
	if (256 <= session->driver.garmin.BufferLen) {
	    // really bad read error...
	    gpsd_report(LOG_ERROR,
			"Garmin: GetPacket() packet too long, %ld > 255 !\n",
			session->driver.garmin.BufferLen);
	    session->driver.garmin.BufferLen = 0;
	    break;
	}
	pkt_size = 12 + get_int32((uint8_t *) & thePacket->mDataSize);
	if (12 <= session->driver.garmin.BufferLen) {
	    // have enough data to check packet size
	    if (session->driver.garmin.BufferLen > pkt_size) {
		// wrong amount of data in buffer
		gpsd_report(LOG_ERROR,
			    "Garmin: GetPacket() packet size wrong! Packet: %ld, s/b %ld\n",
			    session->driver.garmin.BufferLen, pkt_size);
		session->driver.garmin.BufferLen = 0;
		break;
	    }
	}
	if (64 > theBytesReturned) {
	    // zero length, or short, read is a flag for got the whole packet
	    break;
	}


	/*@ ignore @*/
	delay.tv_sec = 0;
	delay.tv_nsec = 3330000L;
	while (nanosleep(&delay, &rem) == -1)
	    continue;
	/*@ end @*/
    }
    // dump the individual bytes, debug only
    // for ( x = 0; x < session->driver.garmin.BufferLen; x++ ) {
    // gpsd_report(LOG_RAW+1, "Garmin: p[%d] = %x\n", x, session->driver.garmin.Buffer[x]);
    // }
    if (10 <= cnt) {
	gpsd_report(LOG_ERROR,
		    "Garmin: GetPacket() packet too long or too slow!\n");
	return -1;
    }

    gpsd_report(LOG_RAW, "Garmin: GotPacket() sz=%d \n",
		session->driver.garmin.BufferLen);
    session->packet.outbuflen = session->driver.garmin.BufferLen;
    return 0;
}

static gps_mask_t garmin_usb_parse(struct gps_device_t *session)
{
    gpsd_report(LOG_PROG, "Garmin: garmin_usb_parse()\n");
    return PrintUSBPacket(session,
			  (Packet_t *) session->driver.garmin.Buffer);
}

static ssize_t garmin_get_packet(struct gps_device_t *session)
{
    return (ssize_t) (0 == GetPacket(session) ? 1 : 0);
}

/* *INDENT-OFF* */
const struct gps_type_t garmin_usb_binary_old =
{
    .type_name      = "Garmin USB binary",	/* full name of type */
    .packet_type    = GARMIN_PACKET;	/* associated lexer packet type */
    .flags	    = DRIVER_NOFLAGS,	/* no flags set */
    .trigger        = NULL,		/* no trigger, it has a probe */
    .channels       = GARMIN_CHANNELS,	/* consumer-grade GPS */
    .probe_detect   = garmin_usb_detect,/* how to detect at startup time */
    .get_packet     = garmin_get_packet,/* how to grab a packet */
    .parse_packet   = garmin_usb_parse,	/* parse message packets */
    .rtcm_writer    = NULL,		/* don't send DGPS corrections */
    .event_hook     = garmin_event_hook,/* lifetime event handler */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher = NULL,		/* no speed switcher */
    .mode_switcher  = NULL,		/* no mode switcher */
    .rate_switcher  = NULL,		/* no sample-rate switcher */
    .min_cycle      = 1,		/* not relevant, no rate switch */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send   = garmin_control_send,	/* send raw bytes */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = garmin_ntp_offset,
#endif /* NTPSHM_ ENABLE */
};
/* *INDENT-ON* */
#endif /* __UNUSED__ */

/* *INDENT-OFF* */
const struct gps_type_t garmin_usb_binary =
{
    .type_name      = "Garmin USB binary",	/* full name of type */
    .packet_type    = GARMIN_PACKET,	/* associated lexer packet type */
    .flags	    = DRIVER_NOFLAGS,	/* no flags set */
    .trigger        = NULL,		/* no trigger, it has a probe */
    .channels       = GARMIN_CHANNELS,	/* consumer-grade GPS */
    .probe_detect   = garmin_usb_detect,/* how to detect at startup time */
    .get_packet     = generic_get,      /* how to grab a packet */
    .parse_packet   = garmin_ser_parse,	/* parse message packets */
    .rtcm_writer    = NULL,		/* don't send DGPS corrections */
    .event_hook     = garmin_event_hook,/* lifetime ebent handler */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher = NULL,		/* no speed switcher */
    .mode_switcher  = NULL,	        /* Garmin USB Binary has no NMEA */
    .rate_switcher  = NULL,		/* no sample-rate switcher */
    .min_cycle      = 1,		/* not relevant, no rate switch */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send   = garmin_control_send,	/* send raw bytes */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = garmin_ntp_offset,
#endif /* NTPSHM_ ENABLE */
};
/* *INDENT-ON* */

/* *INDENT-OFF* */
const struct gps_type_t garmin_ser_binary =
{
    .type_name      = "Garmin Serial binary",	/* full name of type */
    .packet_type    = GARMIN_PACKET,	/* associated lexer packet type */
    .flags	    = DRIVER_NOFLAGS,	/* no flags set */
    .trigger        = NULL,		/* no trigger, it has a probe */
    .channels       = GARMIN_CHANNELS,	/* consumer-grade GPS */
    .probe_detect   = NULL,        	/* how to detect at startup time */
    .get_packet     = generic_get,       /* how to grab a packet */
    .parse_packet   = garmin_ser_parse,	/* parse message packets */
    .rtcm_writer    = NULL,		/* don't send DGPS corrections */
    .event_hook     = NULL,	        /* lifetime event handler */
#ifdef RECONFIGURE_ENABLE
    .speed_switcher = NULL,		/* no speed switcher */
    .mode_switcher  = garmin_switcher,	/* how to change modes */
    .rate_switcher  = NULL,		/* no sample-rate switcher */
    .min_cycle      = 1,		/* not relevant, no rate switch */
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    .control_send   = garmin_control_send,	/* send raw bytes */
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    .ntp_offset     = garmin_ntp_offset,
#endif /* NTPSHM_ ENABLE */
};
/* *INDENT-ON* */

#endif /* GARMIN_ENABLE */
