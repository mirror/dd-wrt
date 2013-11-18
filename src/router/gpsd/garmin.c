/* $Id: garmin.c 4117 2006-12-11 05:36:49Z garyemiller $ */
/*
 * Handle the Garmin binary packet format supported by the USB Garmins
 * tested with the Garmin 18 and other models.  This driver is NOT for
 * serial port connected Garmins, they provide adequate NMEA support.
 *
 * This code is partly from the Garmin IOSDK and partly from the
 * sample code in the Linux garmin_gps driver.
 *
 * This code supports both Garmin on a serial port and USB Garmins.
 *
 * USB Garmins need the Linux garmin_gps driver and will not function
 * without it.  This code has been tested and at least at one time is
 * known to work on big- and little-endian CPUs and 32 and 64 bit cpu
 * modes.
 *
 * Protocol info from:
 *	 425_TechnicalSpecification.pdf
 *	 ( formerly GPS18_TechnicalSpecification.pdf )
 *	 iop_spec.pdf
 * http://www.garmin.com/support/commProtocol.html
 *
 * bad code by: Gary E. Miller <gem@rellim.com>
 * all rights abandoned, a thank would be nice if you use this code.
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
 *      may or may not work on a little-endian machine
 */

#define __USE_POSIX199309 1
#include <sys/types.h>
#include <time.h> // for nanosleep()

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>

#include "gpsd_config.h"
#if defined (HAVE_SYS_SELECT_H)
#include <sys/select.h>
#endif

#if defined(HAVE_STRINGS_H)
#include <strings.h>
#endif

#include "gpsd.h"
#include "gps.h"

#ifdef GARMIN_ENABLE

#define USE_RMD 0

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

#define MODE_NATIVE          0
#define MODE_GARMIN_SERIAL   1

#define GARMIN_PKTID_TRANSPORT_START_SESSION_REQ 5
#define GARMIN_PKTID_TRANSPORT_START_SESSION_RESP 6

#define GARMIN_PKTID_PROTOCOL_ARRAY     253
#define GARMIN_PKTID_PRODUCT_RQST       254
#define GARMIN_PKTID_PRODUCT_DATA       255
/* 0x29 ')' */
#define GARMIN_PKTID_RMD41_DATA         41
/* 0x33 '3' */
#define GARMIN_PKTID_PVT_DATA           51
/* 0x33 '4' */
#define GARMIN_PKTID_RMD_DATA           52
/* 0x72 'r' */
#define GARMIN_PKTID_SAT_DATA           114

#define GARMIN_PKTID_L001_XFER_CMPLT     12
#define GARMIN_PKTID_L001_COMMAND_DATA   10
#define GARMIN_PKTID_L001_DATE_TIME_DATA 14
#define GARMIN_PKTID_L001_RECORDS        27
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
typedef struct {
	uint8_t  svid;
	int16_t snr; // 0 - 0xffff
	uint8_t  elev;
	uint16_t azmth;
	uint8_t  status; // bit 0, has ephemeris, 1, has diff correction
                               // bit 2 used in solution
			       // bit 3??
} cpo_sat_data;

/* Garmin D800_Pvt_Datetype_Type */
/* packet type:  GARMIN_PKTID_PVT_DATA   52 */
/* This is the data format of the position data from the garmin USB */
typedef struct {
	float alt;  /* altitude above WGS 84 (meters) */
	float epe;  /* estimated position error, 2 sigma (meters)  */
	float eph;  /* epe, but horizontal only (meters) */
	float epv;  /* epe but vertical only (meters ) */
	int16_t	fix; /* 0 - failed integrity check
                      * 1 - invalid or unavailable fix
                      * 2 - 2D
                      * 3 - 3D
		      * 4 - 2D Diff
                      * 5 - 3D Diff
                      */
	double	gps_tow; /* gps time  os week (seconds) */
	double	lat;     /* ->latitude (radians) */
	double	lon;     /* ->longitude (radians) */
	float	lon_vel; /* velocity east (meters/second) */
	float	lat_vel; /* velocity north (meters/second) */
	float	alt_vel; /* velocity up (meters/sec) */
        // Garmin GPS25 uses pkt_id 0x28 and does not output the 
        // next 3 items
	float	msl_hght; /* height of WGS 84 above MSL (meters) */
	int16_t	leap_sec; /* diff between GPS and UTC (seconds) */
	int32_t	grmn_days;
} cpo_pvt_data;

typedef struct {
	uint32_t cycles;
	double	 pr;
	uint16_t phase;
	int8_t slp_dtct;
	uint8_t snr_dbhz;
	uint8_t  svid;
	int8_t valid;
} cpo_rcv_sv_data;

/* packet type:  GARMIN_PKTID_RMD_DATA   53 */
/* seems identical to the packet id 0x29 from the Garmin GPS 25 */
typedef struct {
	double rcvr_tow;
	int16_t	rcvr_wn;
	cpo_rcv_sv_data sv[GARMIN_CHANNELS];
} cpo_rcv_data;

// This is the packet format to/from the Garmin USB
typedef struct {
    uint8_t  mPacketType;
    uint8_t  mReserved1;
    uint16_t mReserved2;
    uint16_t mPacketId;
    uint16_t mReserved3;
    uint32_t  mDataSize;
    union {
	    int8_t chars[MAX_BUFFER_SIZE];
	    uint8_t uchars[MAX_BUFFER_SIZE];
            cpo_pvt_data pvt;
            cpo_sat_data sats;
    } mData;
} Packet_t;

// useful funcs to read/write ints
//  floats and doubles are Intel order only...
static inline void set_int16(uint8_t *buf, uint32_t value)
{
        buf[0] = (uint8_t)(0x0FF & value);
        buf[1] = (uint8_t)(0x0FF & (value >> 8));
}

static inline void set_int32(uint8_t *buf, uint32_t value)
{
        buf[0] = (uint8_t)(0x0FF & value);
        buf[1] = (uint8_t)(0x0FF & (value >> 8));
        buf[2] = (uint8_t)(0x0FF & (value >> 16));
        buf[3] = (uint8_t)(0x0FF & (value >> 24));
}

static inline uint16_t get_uint16(const uint8_t *buf)
{
        return  (uint16_t)(0xFF & buf[0]) 
		| ((uint16_t)(0xFF & buf[1]) << 8);
}

static inline uint32_t get_int32(const uint8_t *buf)
{
        return  (uint32_t)(0xFF & buf[0]) 
		| ((uint32_t)(0xFF & buf[1]) << 8) 
		| ((uint32_t)(0xFF & buf[2]) << 16) 
		| ((uint32_t)(0xFF & buf[3]) << 24);
}

// convert radians to degrees
static inline double  radtodeg( double rad) {
	return (double)(rad * RAD_2_DEG );
}

float rintf(float a)
    {
    return rint(a);
    }
static gps_mask_t PrintSERPacket(struct gps_device_t *session, unsigned char pkt_id, int pkt_len, unsigned char *buf );
static gps_mask_t PrintUSBPacket(struct gps_device_t *session, Packet_t *pkt );

gps_mask_t PrintSERPacket(struct gps_device_t *session, unsigned char pkt_id
	, int pkt_len, unsigned char *buf ) 
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

    gpsd_report(LOG_IO, "PrintSERPacket(, %#02x, %#02x, )\n", pkt_id, pkt_len);

    switch( pkt_id ) {
    case ACK:
	gpsd_report(LOG_PROG, "ACK\n");
	break;
    case NAK:
	gpsd_report(LOG_PROG, "NAK\n");
	break;
    case GARMIN_PKTID_L001_COMMAND_DATA:
	prod_id = get_uint16((uint8_t *)buf);
	/*@ -branchstate @*/
	switch ( prod_id ) {
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
	gpsd_report(LOG_PROG, "Appl, Command Data: %s\n", msg);
	break;
    case GARMIN_PKTID_PRODUCT_RQST:
	gpsd_report(LOG_PROG, "Appl, Product Data req\n");
	break;
    case GARMIN_PKTID_PRODUCT_DATA:
	prod_id = get_uint16((uint8_t *)buf);
	ver = get_uint16((uint8_t *)&buf[2]);
	maj_ver = (int)(ver / 100);
	min_ver = (int)(ver - (maj_ver * 100));
	gpsd_report(LOG_PROG, "Appl, Product Data, sz: %d\n", pkt_len);
	(void)snprintf(session->subtype, sizeof(session->subtype),
		       "%d: %d.%02d"
		       , (int)prod_id, maj_ver, min_ver);
	gpsd_report(LOG_INF, "Garmin Product ID: %d, SoftVer: %d.%02d\n"
		, prod_id, maj_ver, min_ver);
	gpsd_report(LOG_INF, "Garmin Product Desc: %s\n"
		, &buf[4]);
	mask |= DEVICEID_SET;
	break;
    case GARMIN_PKTID_PVT_DATA:
	gpsd_report(LOG_PROG, "Appl, PVT Data Sz: %d\n", pkt_len);

	pvt = (cpo_pvt_data*) buf;

	// 631065600, unix seconds for 31 Dec 1989 Zulu 
	time_l = (time_t)(631065600 + (pvt->grmn_days * 86400));
	time_l -= pvt->leap_sec;
	session->context->leap_seconds = pvt->leap_sec;
	session->context->valid = LEAP_SECOND_VALID;
	// gps_tow is always like x.999 or x.998 so just round it
	time_l += (time_t) round(pvt->gps_tow);
	session->gpsdata.fix.time 
	  = session->gpsdata.sentence_time 
	  = (double)time_l;
	gpsd_report(LOG_PROG, "time_l: %ld\n", (long int)time_l);

	session->gpsdata.fix.latitude = radtodeg(pvt->lat);
	session->gpsdata.fix.longitude = radtodeg(pvt->lon);

	// altitude over WGS84 converted to MSL
	session->gpsdata.fix.altitude = pvt->alt + pvt->msl_hght;

	// geoid separation from WGS 84
	// gpsd sign is opposite of garmin sign
	session->gpsdata.separation = -pvt->msl_hght;

	// Estimated position error in meters.
	// We follow the advice at <http://gpsinformation.net/main/errors.htm>.
	// If this assumption changes here, it should also change in 
	// nmea_parse.c where we analyze PGRME.
	session->gpsdata.epe = pvt->epe * (GPSD_CONFIDENCE/CEP50_SIGMA);
	session->gpsdata.fix.eph = pvt->eph * (GPSD_CONFIDENCE/CEP50_SIGMA);
	session->gpsdata.fix.epv = pvt->epv * (GPSD_CONFIDENCE/CEP50_SIGMA);

	// convert lat/lon to directionless speed
	session->gpsdata.fix.speed = hypot(pvt->lon_vel, pvt->lat_vel);

	// keep climb in meters/sec
	session->gpsdata.fix.climb = pvt->alt_vel;

	track = atan2(pvt->lon_vel, pvt->lat_vel);
	if (track < 0) {
	    track += 2 * PI;
	}
	session->gpsdata.fix.track = radtodeg(track);

	switch ( pvt->fix) {
	case 0:
	case 1:
	default:
	    // no fix
	    session->gpsdata.status = STATUS_NO_FIX;
	    session->gpsdata.fix.mode = MODE_NO_FIX;
	    break;
	case 2:
	    // 2D fix
	    session->gpsdata.status = STATUS_FIX;
	    session->gpsdata.fix.mode = MODE_2D;
	    break;
	case 3:
	    // 3D fix
	    session->gpsdata.status = STATUS_FIX;
	    session->gpsdata.fix.mode = MODE_3D;
	    break;
	case 4:
	    // 2D Differential fix
	    session->gpsdata.status = STATUS_DGPS_FIX;
	    session->gpsdata.fix.mode = MODE_2D;
	    break;
	case 5:
	    // 3D differential fix
	    session->gpsdata.status = STATUS_DGPS_FIX;
	    session->gpsdata.fix.mode = MODE_3D;
	    break;
	}
#ifdef NTPSHM_ENABLE
	if (session->context->enable_ntpshm && session->gpsdata.fix.mode > MODE_NO_FIX)
	    (void) ntpshm_put(session, session->gpsdata.fix.time);
#endif /* NTPSHM_ENABLE */

	gpsd_report(LOG_PROG, "Appl, mode %d, status %d\n"
	    , session->gpsdata.fix.mode
	    , session->gpsdata.status);

	gpsd_report(LOG_INF, "UTC Time: %lf\n", session->gpsdata.fix.time);
	gpsd_report(LOG_INF
	    , "Geoid Separation (MSL-WGS84): from garmin %lf, calculated %lf\n"
	    , -pvt->msl_hght
	    , wgs84_separation(session->gpsdata.fix.latitude
	    , session->gpsdata.fix.longitude));

	gpsd_report(LOG_INF, "Alt: %.3f, Epe: %.3f, Eph: %.3f, Epv: %.3f, Fix: %d, Gps_tow: %f, Lat: %.3f, Lon: %.3f, LonVel: %.3f, LatVel: %.3f, AltVel: %.3f, MslHgt: %.3f, Leap: %d, GarminDays: %ld\n"
	    , pvt->alt
	    , pvt->epe
	    , pvt->eph
	    , pvt->epv
	    , pvt->fix
	    , pvt->gps_tow
	    , session->gpsdata.fix.latitude
	    , session->gpsdata.fix.longitude
	    , pvt->lon_vel
	    , pvt->lat_vel
	    , pvt->alt_vel
	    , pvt->msl_hght
	    , pvt->leap_sec
	    , pvt->grmn_days);

	mask |= TIME_SET | LATLON_SET | ALTITUDE_SET | STATUS_SET | MODE_SET | SPEED_SET | TRACK_SET | CLIMB_SET | HERR_SET | VERR_SET | PERR_SET | CYCLE_START_SET;
	break;
    case GARMIN_PKTID_RMD_DATA:
    case GARMIN_PKTID_RMD41_DATA:
	rmd = (cpo_rcv_data *) buf;
	gpsd_report(LOG_IO, "PVT RMD Data Sz: %d\n", pkt_len);
	gpsd_report(LOG_PROG, "PVT RMD rcvr_tow: %f, rcvr_wn: %d\n"
		, rmd->rcvr_tow, rmd->rcvr_wn);
        for ( i = 0 ; i < GARMIN_CHANNELS ; i++ ) {
	    gpsd_report(LOG_INF, "PVT RMD Sat: %3u, cycles: %9lu, pr: %16.6f, phase: %7.3f, slp_dtct: %3s, snr: %3u, Valid: %3s\n"
		, rmd->sv[i].svid + 1, rmd->sv[i].cycles, rmd->sv[i].pr
                , (rmd->sv[i].phase * 360.0)/2048.0
                , rmd->sv[i].slp_dtct!='\0' ? "Yes" : "No"
                , rmd->sv[i].snr_dbhz, rmd->sv[i].valid!='\0' ? "Yes" : "No");
	}
	break;

    case GARMIN_PKTID_SAT_DATA:
	gpsd_report(LOG_PROG, "SAT Data Sz: %d\n", pkt_len);
	sats = (cpo_sat_data *)buf;

	session->gpsdata.satellites_used = 0;
	memset(session->gpsdata.used,0,sizeof(session->gpsdata.used));
	gpsd_zero_satellites(&session->gpsdata);
	for ( i = 0, j = 0 ; i < GARMIN_CHANNELS ; i++, sats++ ) {
	    gpsd_report(LOG_INF,"  Sat %3d, snr: %5d, elev: %2d, Azmth: %3d, Stat: %x\n"
		, sats->svid
		, sats->snr
		, sats->elev
		, sats->azmth
		, sats->status);

	    if ( 255 == (int)sats->svid ) {
		// Garmin uses 255 for empty
		// gpsd uses 0 for empty
		continue;
	    }

	    session->gpsdata.PRN[j]       = (int)sats->svid;
	    session->gpsdata.azimuth[j]   = (int)sats->azmth;
	    session->gpsdata.elevation[j] = (int)sats->elev;
            // Garmin does not document this.  snr is in dB*100
	    // Known, but not seen satellites have a dB value of -1*100
            session->gpsdata.ss[j] = (int)round((float)sats->snr / 100);
	    if (session->gpsdata.ss[j] < 0) {
		session->gpsdata.ss[j] = 0;
	    }
	    // FIXME: Garmin documents this, but Daniel Dorau 
	    // <daniel.dorau@gmx.de> says the behavior on his GPSMap60CSX
	    // doesn't match it.
	    if ( (uint8_t)0 != (sats->status & 4 ) )  {
	        // used in solution?
	        session->gpsdata.used[session->gpsdata.satellites_used++]
		    = (int)sats->svid;
	    }
	    session->gpsdata.satellites++;
	    j++;

	}
	mask |= SATELLITE_SET | USED_SET;
	break;
    case GARMIN_PKTID_PROTOCOL_ARRAY:
	// this packet is never requested, it just comes, in some case
	// after a GARMIN_PKTID_PRODUCT_RQST 
	gpsd_report(LOG_INF, "Appl, Product Capability, sz: %d\n", pkt_len);
	for ( i = 0; i < pkt_len ; i += 3 ) {
	    gpsd_report(LOG_INF, "  %c%03d\n", buf[i], get_uint16((uint8_t *)&buf[i+1] ) );
	}
	break;
    default:
	gpsd_report(LOG_WARN, "Unknown packet id: %#02x, Sz: %#02x, pkt:%s\n"
		    , pkt_id, pkt_len, gpsd_hexdump(buf, (size_t)pkt_len));
	break;
    }
    gpsd_report(LOG_IO, "PrintSERPacket(, %#02x, %#02x, ) = %#02x\n"
	, pkt_id, pkt_len, mask);
    return mask;
}


/*@ -branchstate @*/
// For debugging, decodes and prints some known packets.
static gps_mask_t PrintUSBPacket(struct gps_device_t *session, Packet_t *pkt)
{
    gps_mask_t mask = 0;
    int maj_ver;
    int min_ver;
    uint32_t mode = 0;
    uint16_t prod_id = 0;
    uint32_t veri = 0;
    uint32_t serial;
    uint32_t mDataSize = get_int32( (uint8_t*)&pkt->mDataSize);

//
    uint8_t *buffer = (uint8_t*)pkt;

    gpsd_report(LOG_PROG, "PrintUSBPacket()\n");
// gem
    if ( DLE == pkt->mPacketType) {
	    gpsd_report(LOG_PROG, "really a SER packet!\n");
            return PrintSERPacket ( session,  
				    (unsigned char)buffer[1], 
				    (int)buffer[2], 
				    (unsigned char*)(buffer + 3));
    }

// gem
    if ( 4096 < mDataSize) {
	gpsd_report(LOG_WARN, "bogus packet, size too large=%d\n", mDataSize);
	return 0;
    }

    (void)snprintf(session->gpsdata.tag, sizeof(session->gpsdata.tag), "%u"
	, (unsigned int)pkt->mPacketType);
    switch ( pkt->mPacketType ) {
    case GARMIN_LAYERID_TRANSPORT:
        /* Garmin USB layer specific */
	switch( pkt->mPacketId ) {
	case GARMIN_PKTID_TRANSPORT_START_SESSION_REQ:
	    gpsd_report(LOG_PROG, "Transport, Start Session req\n");
	    break;
	case GARMIN_PKTID_TRANSPORT_START_SESSION_RESP:
	    mode = get_int32(&pkt->mData.uchars[0]);
	    gpsd_report(LOG_PROG, "Transport, Start Session resp, unit: 0x%x\n"
		, mode);
	    break;
	default:
	    gpsd_report(LOG_PROG, "Transport, Packet: Type %d %d %d, ID: %d, Sz: %d\n"
			, pkt->mPacketType
			, pkt->mReserved1
			, pkt->mReserved2
			, pkt->mPacketId
			, mDataSize);
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
	switch( pkt->mPacketId ) {
	case PRIV_PKTID_SET_MODE:
	    prod_id = get_uint16(&pkt->mData.uchars[0]);
	    gpsd_report(LOG_PROG, "Private, Set Mode: %d\n", prod_id);
	    break;
	case PRIV_PKTID_INFO_REQ:
	    gpsd_report(LOG_PROG, "Private, ID: Info Req\n");
	    break;
	case PRIV_PKTID_INFO_RESP:
	    veri = get_int32(pkt->mData.uchars);
	    maj_ver = (int)(veri >> 16);
	    min_ver = (int)(veri & 0xffff);
	    mode = get_int32(&pkt->mData.uchars[4]);
	    serial = get_int32(&pkt->mData.uchars[8]);
	    gpsd_report(LOG_PROG, "Private, ID: Info Resp\n");
	    gpsd_report(LOG_INF, "Garmin USB Driver found, Version %d.%d, Mode: %d, GPS Serial# %u\n"
			,  maj_ver, min_ver, mode, serial);
	    break;
	default:
	    gpsd_report(LOG_PROG, "Private, Packet: ID: %d, Sz: %d\n"
			, pkt->mPacketId
			, mDataSize);
	    break;
	}
	break;
    default:
	gpsd_report(LOG_PROG, "Packet: Type %d %d %d, ID: %d, Sz: %d\n"
		    , pkt->mPacketType
		    , pkt->mReserved1
		    , pkt->mReserved2
		    , pkt->mPacketId
		    , mDataSize);
	break;
    }

    return mask;
}
/*@ +branchstate @*/


/* build and send a packet w/ USB protocol */
static void Build_Send_USB_Packet( struct gps_device_t *session,
       uint32_t layer_id, uint32_t pkt_id, uint32_t length, uint32_t data ) 
{
        uint8_t *buffer = (uint8_t *)session->driver.garmin.Buffer;
	Packet_t *thePacket = (Packet_t*)buffer;
	ssize_t theBytesReturned = 0;
	ssize_t theBytesToWrite = 12 + (ssize_t)length;

	set_int32(buffer, layer_id);
	set_int32(buffer+4, pkt_id);
	set_int32(buffer+8, length); 
        if ( 2 == length ) {
		set_int16(buffer+12, data);
        } else if ( 4 == length ) {
		set_int32(buffer+12, data);
	}

#if 0
        gpsd_report(LOG_IO, "SendPacket(), writing %d bytes: %s\n"
		, theBytesToWrite, gpsd_hexdump(thePacket, theBytesToWrite));
#endif
        (void)PrintUSBPacket ( session,  thePacket);

	theBytesReturned = write( session->gpsdata.gps_fd
				  , thePacket, (size_t)theBytesToWrite);
	gpsd_report(LOG_IO, "SendPacket(), wrote %d bytes\n", theBytesReturned);

	// Garmin says:
	// If the packet size was an exact multiple of the USB packet
	// size, we must make a final write call with no data

	// as a practical matter no known pckets are 64 bytes long so
        // this is untested

	// So here goes just in case
	if( 0 == (theBytesToWrite % ASYNC_DATA_SIZE) ) {
		char *n = "";
		theBytesReturned = write( session->gpsdata.gps_fd
		    , &n, 0);
	}
}
/* build and send a packet in serial protocol */
/* layer_id unused */
static void Build_Send_SER_Packet( struct gps_device_t *session,
       uint32_t layer_id UNUSED, uint32_t pkt_id, uint32_t length, uint32_t data ) 
{
        uint8_t *buffer = (uint8_t *)session->driver.garmin.Buffer;
        uint8_t *buffer0 = buffer;
	Packet_t *thePacket = (Packet_t*)buffer;
	ssize_t theBytesReturned = 0;
	ssize_t theBytesToWrite = 6 + (ssize_t)length;
        uint8_t chksum = 0;

	*buffer++ = (uint8_t)DLE;
	*buffer++ = (uint8_t)pkt_id;
	chksum = pkt_id;
	*buffer++ = (uint8_t)length; 
	chksum += length;
        if ( 2 == length ) {
		/* carefull!  no DLE stuffing here! */
		set_int16(buffer, data);
		chksum += buffer[0];
		chksum += buffer[1];
        } else if ( 4 == length ) {
		/* carefull!  no DLE stuffing here! */
		set_int32(buffer, data);
		chksum += buffer[0];
		chksum += buffer[1];
		chksum += buffer[2];
		chksum += buffer[3];
	}
	buffer += length;

	// Add checksum
	*buffer++ = -chksum;
	if ( DLE == -chksum ) {
		/* stuff another DLE */
		*buffer++ = (uint8_t)DLE;
		theBytesToWrite++;
	}		
	
	// Add DLE, ETX
	*buffer++ = (uint8_t)DLE;
	*buffer++ = (uint8_t)ETX;

#if 1
        gpsd_report(LOG_IO, "SendPacket(), writing %d bytes: %s\n"
		    , theBytesToWrite, gpsd_hexdump(thePacket, (size_t)theBytesToWrite));
#endif
        (void)PrintSERPacket ( session,  
			       (unsigned char)buffer0[1], 
			       (int)buffer0[2], 
			       (unsigned char *)(buffer0 + 3));

	theBytesReturned = write( session->gpsdata.gps_fd
				  , thePacket, (size_t)theBytesToWrite);
	gpsd_report(LOG_IO, "SendPacket(), wrote %d bytes\n", theBytesReturned);

}


/*
 * garmin_detect()
 *
 * check that the garmin_gps driver is installed in the kernel
 * and that an active USB device is using it.
 *
 * It does not yet check that the currect device is the one 
 * attached to the garmin.  So if you have a garmin and another 
 * gps this could be a problem.
 *
 * this is very linux specific.  
 *
 * return 1 if garmin_gps device found
 * return 0 if not
 *
 */
static bool garmin_detect(struct gps_device_t *session)
{

    FILE *fp = NULL;
    char buf[256];
    bool ok = false;

    /* check for garmin USB serial driver -- very Linux-specific */
    if (access("/sys/module/garmin_gps", R_OK) != 0) {
	gpsd_report(LOG_WARN, "garmin_gps not active.\n"); 
        return false;
    }
    // check for a garmin_gps device in /proc
    if ( !(fp = fopen( "/proc/bus/usb/devices", "r") ) ) {
	gpsd_report(LOG_ERROR, "Can't open /proc/bus/usb/devices\n");
        return false;
    }

    ok = false;
    while ( 0 != fgets( buf, (int)sizeof(buf), fp ) ) {
	if ( strstr( buf, "garmin_gps") ) {
		ok = true;
		break;
	}
    }
    (void)fclose(fp);
    if ( !ok ) {
        // no device using garmin now
	gpsd_report(LOG_WARN, "garmin_gps not in /proc/bus/usb/devices.\n"); 
	return false;
    }

    if (!gpsd_set_raw(session)) {
	gpsd_report(LOG_ERROR, "garmin_detect: error changing port attributes: %s\n",
             strerror(errno));
	return false;
    }

#ifdef __UNUSED
    Packet_t *thePacket = NULL;
    uint8_t *buffer = NULL;
    /* reset the buffer and buffer length */
    memset( session->driver.garmin.Buffer, 0, sizeof(session->driver.garmin.Buffer) );
    session->driver.garmin.BufferLen = 0;

    if (sizeof(session->driver.garmin.Buffer) < sizeof(Packet_t)) {
	gpsd_report(LOG_ERROR, "garmin_detect: Compile error, garmin.Buffer too small.\n",
             strerror(errno));
	return false;
    }

    buffer = (uint8_t *)session->driver.garmin.Buffer;
    thePacket = (Packet_t*)buffer;
#endif /* __UNUSED__ */

    // set Mode 1, mode 0 is broken somewhere past 2.6.14
    // but how?
    gpsd_report(LOG_PROG, "Set garmin_gps driver mode = 0\n");
    Build_Send_USB_Packet( session, GARMIN_LAYERID_PRIVATE
        , PRIV_PKTID_SET_MODE, 4, MODE_GARMIN_SERIAL);
    // expect no return packet !?

    return true;
}

static void garmin_probe_subtype(struct gps_device_t *session, unsigned int seq)
{
    if (seq == 0) {
        // Tell the device to send product data
        gpsd_report(LOG_PROG, "Get Garmin Product Data\n");
        Build_Send_SER_Packet(session, GARMIN_LAYERID_APPL
           , GARMIN_PKTID_PRODUCT_RQST, 0, 0);

	// turn on PVT data 49
	gpsd_report(LOG_PROG, "Set Garmin to send reports every 1 second\n");

        Build_Send_SER_Packet(session, GARMIN_LAYERID_APPL
	    , GARMIN_PKTID_L001_COMMAND_DATA, 2, CMND_START_PVT_DATA);

#if USE_RMD
	// turn on RMD data 110
	gpsd_report(LOG_PROG, "Set Garmin to send Raw sat data\n");
        Build_Send_SER_Packet(session, GARMIN_LAYERID_APPL
	      , GARMIN_PKTID_L001_COMMAND_DATA, 2, CMND_START_RM_DATA);
#endif
    }
}

static void garmin_close(struct gps_device_t *session UNUSED) 
{
    /* FIXME -- do we need to put the garmin to sleep?  or is closing the port
       sufficient? */
    gpsd_report(LOG_PROG, "garmin_close()\n");
    return;
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

    gpsd_report(LOG_RAW, "garmin_ser_parse()\n");
    if (  6 > len ) {
	/* WTF? */
        /* minimum packet; <DLE> [pkt id] [length=0] [chksum] <DLE> <STX> */
	Send_NAK();
	gpsd_report(LOG_RAW+1, "Garmin serial too short: %#2x\n", len);
	return 0;
    }
    /* debug */
    for ( i = 0 ; i < (int)len ; i++ ) {
	gpsd_report(LOG_RAW+1, "Char: %#02x\n", buf[i]);
    }
    
    if ( '\x10' != buf[0] ) {
	Send_NAK();
	gpsd_report(LOG_RAW+1, "buf[0] not DLE\n", buf[0]);
        return 0;
    }
    n = 1;
    pkt_id = buf[n++];
    chksum = pkt_id;
    if ( '\x10' == pkt_id ) {
        if ( '\x10' != buf[n++] ) {
	    Send_NAK();
	    gpsd_report(LOG_RAW+1, "Bad pkt_id %#02x\n", pkt_id);
	    return 0;
        }
    }

    pkt_len = buf[n++];
    chksum += pkt_len;
    if ( '\x10' == pkt_len ) {
        if ( '\x10' != buf[n++] ) {
	    gpsd_report(LOG_RAW+1, "Bad pkt_len %#02x\n", pkt_len);
	    Send_NAK();
	    return 0;
        }
    }
    data_index = 0;
    for ( i = 0; i < 256 ; i++ ) {

	if ( (int)pkt_len == data_index )  {
		// got it all
		break;
	}
        if ( len < n + i ) {
	    gpsd_report(LOG_RAW+1, "Packet too short %#02x < %#0x\n", len, n + i);
	    Send_NAK();
	    return 0;
        }
	c = buf[n + i];
        if ( got_dle ) {
	    got_dle = 0;
            if ( '\x10' != c ) {
		Send_NAK();
	        gpsd_report(LOG_RAW+1, "Bad DLE %#02x\n", c);
	        return 0;
            }
	} else {
            chksum += c;
	    data_buf[ data_index++ ] = c;
            if ( '\x10' == c ) {
		got_dle = 1;
	    }
	}
    }
    /* get checksum */
    if ( len < n + i ) {
	Send_NAK();
        gpsd_report(LOG_RAW+1, "No checksum, Packet too short %#02x < %#0x\n"
	    , len, n + i);
        return 0;
    }
    c = buf[n + i++];
    chksum += c;
    /* get final DLE */
    if ( len < n + i ) {
	Send_NAK();
        gpsd_report(LOG_RAW+1, "No final DLE, Packet too short %#02x < %#0x\n"
	    , len, n + i);
        return 0;
    }
    c = buf[n + i++];
    if ( '\x10' != c ) {
	Send_NAK();
	gpsd_report(LOG_RAW+1, "Final DLE not DLE\n", c);
        return 0;
    }
    /* get final ETX */
    if ( len < n + i ) {
	Send_NAK();
        gpsd_report(LOG_RAW+1, "No final ETX, Packet too short %#02x < %#0x\n"
	    , len, n + i);
        return 0;
    }
    c = buf[n + i++];
    if ( '\x03' != c ) {
	Send_NAK();
	gpsd_report(LOG_RAW+1, "Final ETX not ETX\n", c);
        return 0;
    }

    /* debug */
    /*@ -usedef -compdef @*/
    for ( i = 0 ; i < data_index ; i++ ) {
	gpsd_report(LOG_RAW+1, "Char: %#02x\n", data_buf[i]);
    }


    gpsd_report(LOG_IO
	, "garmin_ser_parse() Type: %#02x, Len: %#02x, chksum: %#02x\n"
        , pkt_id, pkt_len, chksum);

    mask = PrintSERPacket(session, pkt_id, pkt_len, data_buf);

    // sending ACK too soon might hang the session
    // so send ACK last, after a pause
    usleep(300);
    Send_ACK();
    /*@ +usedef +compdef @*/
    return mask;
}
/*@ -charint @*/

#ifdef ALLOW_RECONFIGURE
static void settle(void)
{
    struct timespec delay, rem;
    /*@ -type -unrecog @*/
    memset( &delay, 0, sizeof(delay));
    delay.tv_sec = 0;
    delay.tv_nsec = 333000000L;
    nanosleep(&delay, &rem);
    /*@ +type +unrecog @*/
}
#endif /* ALLOW_RECONFIGURE */

static void garmin_switcher(struct gps_device_t *session, int mode)
{
#ifdef ALLOW_RECONFIGURE
    if (mode == 0) {
	const char *switcher = "\x10\x0A\x02\x26\x00\xCE\x10\x03";
	int status = (int)write(session->gpsdata.gps_fd, 
				switcher, strlen(switcher));
	if (status == (int)strlen(switcher)) {
	    gpsd_report(LOG_IO, "=> GPS: turn off binary %02x %02x %02x... \n"
			, switcher[0], switcher[1], switcher[2]);
	} else {
	    gpsd_report(LOG_ERROR, "=> GPS: FAILED\n");
	}
	settle(); // wait 333mS, essential!

	/* once a sec, no binary, no averaging, NMEA 2.3, WAAS */
	(void)nmea_send(session->gpsdata.gps_fd, "$PGRMC1,1,1");
	//(void)nmea_send(fd, "$PGRMC1,1,1,1,,,,2,W,N");
	(void)nmea_send(session->gpsdata.gps_fd, "$PGRMI,,,,,,,R");
	settle();    // wait 333mS, essential!
    } else {
	(void)nmea_send(session->gpsdata.gps_fd, "$PGRMC1,1,2,1,,,,2,W,N");
	(void)nmea_send(session->gpsdata.gps_fd, "$PGRMI,,,,,,,R");
	// garmin serial binary is 9600 only!
	gpsd_report(LOG_ERROR, "NOTE: Garmin binary is 9600 baud only!\n");
	settle();	// wait 333mS, essential!
    }
#endif /* ALLOW_RECONFIGURE */
}

/* this is everything we export */
#ifdef __UNUSED__
static int GetPacket (struct gps_device_t *session );
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
//         -1 = error
//         1 = got partial packet
static int GetPacket (struct gps_device_t *session ) 
{
    struct timespec delay, rem;
    int cnt = 0;
    // int x = 0; // for debug dump

    memset( session->driver.garmin.Buffer, 0, sizeof(Packet_t));
    memset( &delay, 0, sizeof(delay));
    session->driver.garmin.BufferLen = 0;
    session->packet.outbuflen = 0;

    gpsd_report(LOG_IO, "GetPacket()\n");

    for( cnt = 0 ; cnt < 10 ; cnt++ ) {
	size_t pkt_size;
	// Read async data until the driver returns less than the
	// max async data size, which signifies the end of a packet

	// not optimal, but given the speed and packet nature of
	// the USB not too bad for a start
	ssize_t theBytesReturned = 0;
	uint8_t *buf = (uint8_t *)session->driver.garmin.Buffer;
	Packet_t *thePacket = (Packet_t*)buf;

	theBytesReturned = read(session->gpsdata.gps_fd
		, buf + session->driver.garmin.BufferLen
		, ASYNC_DATA_SIZE);
	// zero byte returned is a legal value and denotes the end of a 
        // binary packet.
        if ( 0 >  theBytesReturned ) {
	    // read error...
            // or EAGAIN, but O_NONBLOCK is never set
	    gpsd_report(LOG_ERROR, "GetPacket() read error=%d, errno=%d\n"
		, theBytesReturned, errno);
	    continue;
	}
	gpsd_report(LOG_RAW, "got %d bytes\n", theBytesReturned);
#if 1
        gpsd_report(LOG_IO, "getPacket(), got %d bytes: %s\n"
		, theBytesReturned, gpsd_hexdump(thePacket, theBytesReturned));
#endif

	session->driver.garmin.BufferLen += theBytesReturned;
	if ( 256 <=  session->driver.garmin.BufferLen ) {
	    // really bad read error...
	    gpsd_report(LOG_ERROR, "GetPacket() packet too long, %ld > 255 !\n"
		    , session->driver.garmin.BufferLen);
	    session->driver.garmin.BufferLen = 0;
	    break;
	}
	pkt_size = 12 + get_int32((uint8_t*)&thePacket->mDataSize);
	if ( 12 <= session->driver.garmin.BufferLen) {
	    // have enough data to check packet size
	    if ( session->driver.garmin.BufferLen > pkt_size) {
	        // wrong amount of data in buffer
	        gpsd_report(LOG_ERROR
		    , "GetPacket() packet size wrong! Packet: %ld, s/b %ld\n"
		    , session->driver.garmin.BufferLen
		    , pkt_size);
	        session->driver.garmin.BufferLen = 0;
	        break;
	    }
	}
	if ( 64 > theBytesReturned ) {
	    // zero length, or short, read is a flag for got the whole packet
            break;
	}
		

	/*@ ignore @*/
	delay.tv_sec = 0;
	delay.tv_nsec = 3330000L;
	while (nanosleep(&delay, &rem) < 0)
	    continue;
	/*@ end @*/
    }
    // dump the individual bytes, debug only
    // for ( x = 0; x < session->driver.garmin.BufferLen; x++ ) {
        // gpsd_report(LOG_RAW+1, "p[%d] = %x\n", x, session->driver.garmin.Buffer[x]);
    // }
    if ( 10 <= cnt ) {
	    gpsd_report(LOG_ERROR, "GetPacket() packet too long or too slow!\n");
	    return -1;
    }

    gpsd_report(LOG_RAW, "GotPacket() sz=%d \n", session->driver.garmin.BufferLen);
    session->packet.outbuflen = session->driver.garmin.BufferLen;
    return 0;
}
static gps_mask_t garmin_usb_parse(struct gps_device_t *session)
{
    gpsd_report(LOG_PROG, "garmin_usb_parse()\n");
    return PrintUSBPacket(session, (Packet_t*)session->driver.garmin.Buffer);
}

static ssize_t garmin_get_packet(struct gps_device_t *session) 
{
    return (ssize_t)( 0 == GetPacket( session ) ? 1 : 0);
}

struct gps_type_t garmin_usb_binary_old =
{
    .typename       = "Garmin USB binary",	/* full name of type */
    .trigger        = NULL,		/* no trigger, it has a probe */
    .channels       = GARMIN_CHANNELS,	/* consumer-grade GPS */
    .probe_wakeup   = NULL,		/* no wakeup to be done before hunt */
    .probe_detect   = garmin_detect,	/* how to detect at startup time */
    .probe_subtype  = garmin_probe_subtype,	/* get subtype info */
#ifdef ALLOW_RECONFIGURE
    .configurator   = garmin_usb_configure,	/* eable what we need */
#endif /* ALLOW_RECONFIGURE */
    .get_packet     = garmin_get_packet,/* how to grab a packet */
    .parse_packet   = garmin_usb_parse,	/* parse message packets */
    .rtcm_writer    = NULL,		/* don't send DGPS corrections */
    .speed_switcher = NULL,		/* no speed switcher */
    .mode_switcher  = NULL,		/* no mode switcher */
    .rate_switcher  = NULL,		/* no sample-rate switcher */
    .cycle_chars    = -1,		/* not relevant, no rate switch */
#ifdef ALLOW_RECONFIGURE
    .revert         = NULL,		/* no setting-reversion method */
#endif /* ALLOW_RECONFIGURE */
    .wrapup         = garmin_close,	/* close hook */
    .cycle          = 1,		/* updates every second */
};
#endif /* __UNUSED__ */

struct gps_type_t garmin_usb_binary =
{
    .typename       = "Garmin USB binary",	/* full name of type */
    .trigger        = NULL,		/* no trigger, it has a probe */
    .channels       = GARMIN_CHANNELS,	/* consumer-grade GPS */
    .probe_wakeup   = NULL,		/* no wakeup to be done before hunt */
    .probe_detect   = garmin_detect,	/* how to detect at startup time */
    .probe_subtype  = garmin_probe_subtype,	/* get subtype info */
#ifdef ALLOW_RECONFIGURE
    .configurator   = NULL,	        /* enable what we need */
#endif /* ALLOW_RECONFIGURE */
    .get_packet     = generic_get,      /* how to grab a packet */
    .parse_packet   = garmin_ser_parse,	/* parse message packets */
    .rtcm_writer    = NULL,		/* don't send DGPS corrections */
    .speed_switcher = NULL,		/* no speed switcher */
    .mode_switcher  = NULL,		/* no mode switcher */
    .rate_switcher  = NULL,		/* no sample-rate switcher */
    .cycle_chars    = -1,		/* not relevant, no rate switch */
#ifdef ALLOW_RECONFIGURE
    .revert         = NULL,		/* no setting-reversion method */
#endif /* ALLOW_RECONFIGURE */
    .wrapup         = garmin_close,	/* close hook */
    .cycle          = 1,		/* updates every second */
};

struct gps_type_t garmin_ser_binary =
{
    .typename       = "Garmin Serial binary",	/* full name of type */
    .trigger        = NULL,		/* no trigger, it has a probe */
    .channels       = GARMIN_CHANNELS,	/* consumer-grade GPS */
    .probe_wakeup   = NULL,		/* no wakeup to be done before hunt */
    .probe_detect   = NULL,        	/* how to detect at startup time */
    .probe_subtype  = NULL,        	/* initialize the device */
#ifdef ALLOW_RECONFIGURE
    .configurator   = NULL,	        /* enable what we need */
#endif /* ALLOW_RECONFIGURE */
    .get_packet     = generic_get,       /* how to grab a packet */
    .parse_packet   = garmin_ser_parse,	/* parse message packets */
    .rtcm_writer    = NULL,		/* don't send DGPS corrections */
    .speed_switcher = NULL,		/* no speed switcher */
    .mode_switcher  = garmin_switcher,	/* how to change modes */
    .rate_switcher  = NULL,		/* no sample-rate switcher */
    .cycle_chars    = -1,		/* not relevant, no rate switch */
#ifdef ALLOW_RECONFIGURE
    .revert         = NULL,		/* no setting-reversion method */
#endif /* ALLOW_RECONFIGURE */
    .wrapup         = NULL,	        /* close hook */
    .cycle          = 1,		/* updates every second */
};

#endif /* GARMIN_ENABLE */

