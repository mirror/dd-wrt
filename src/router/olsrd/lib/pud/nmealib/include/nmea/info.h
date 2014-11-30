/*
 * This file is part of nmealib.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __NMEA_INFO_H__
#define __NMEA_INFO_H__

#include <stdint.h>
#include <stdbool.h>

/**
 * @file
 * The table below describes which fields are present in the sentences that are
 * supported by the library.
 <pre>
 field/sentence       GPGGA   GPGSA   GPGSV   GPRMC   GPVTG
 present:               x       x       x       x       x
 smask:                 x       x       x       x       x
 utc (date):                                    x
 utc (time):            x                       x
 sig:                   x                       x1
 fix:                           x               x1
 PDOP:                          x
 HDOP:                  x       x
 VDOP:                          x
 lat:                   x                       x
 lon:                   x                       x
 elv:                   x
 speed:                                         x       x
 track:                                         x       x
 mtrack:                                                x
 magvar:                                        x
 satinfo (inuse count): x       x1
 satinfo (inuse):               x
 satinfo (inview):                      x

 x1 = not present in the sentence but the library sets it up.
 </pre>
 */

#define NMEA_SIG_FIRST (NMEA_SIG_BAD)
#define NMEA_SIG_BAD   (0)
#define NMEA_SIG_LOW   (1)
#define NMEA_SIG_MID   (2)
#define NMEA_SIG_HIGH  (3)
#define NMEA_SIG_RTKIN (4)
#define NMEA_SIG_FLRTK (5)
#define NMEA_SIG_ESTIM (6)
#define NMEA_SIG_MAN   (7)
#define NMEA_SIG_SIM   (8)
#define NMEA_SIG_LAST  (NMEA_SIG_SIM)

#define NMEA_FIX_FIRST (NMEA_FIX_BAD)
#define NMEA_FIX_BAD   (1)
#define NMEA_FIX_2D    (2)
#define NMEA_FIX_3D    (3)
#define NMEA_FIX_LAST  (NMEA_FIX_3D)

#define NMEA_MAXSAT    (64)
#define NMEA_SATINPACK (4)
#define NMEA_NSATPACKS (NMEA_MAXSAT / NMEA_SATINPACK)

#define NMEA_DEF_LAT   (0.0)
#define NMEA_DEF_LON   (0.0)

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Date and time data
 * @see nmea_time_now
 */
typedef struct _nmeaTIME {
	int year;						/**< Years since 1900 */
	int mon;						/**< Months since January - [0,11] */
	int day;						/**< Day of the month - [1,31] */
	int hour;						/**< Hours since midnight - [0,23] */
	int min;						/**< Minutes after the hour - [0,59] */
	int sec;						/**< Seconds after the minute - [0,59] */
	int hsec;						/**< Hundredth part of second - [0,99] */
} nmeaTIME;

/**
 * Position data in fractional degrees or radians
 */
typedef struct _nmeaPOS {
	double lat;						/**< Latitude */
	double lon;						/**< Longitude */
} nmeaPOS;

/**
 * Information about satellite
 * @see nmeaSATINFO
 * @see nmeaGPGSV
 */
typedef struct _nmeaSATELLITE {
	int id;							/**< Satellite PRN number */
	int elv;						/**< Elevation in degrees, 90 maximum */
	int azimuth;					/**< Azimuth, degrees from true north, 000 to 359 */
	int sig;						/**< Signal, 00-99 dB */
} nmeaSATELLITE;

/**
 * Information about all satellites in view
 * @see nmeaINFO
 * @see nmeaGPGSV
 */
typedef struct _nmeaSATINFO {
	int inuse;						/**< Number of satellites in use (not those in view) */
	int in_use[NMEA_MAXSAT];		/**< IDs of satellites in use (not those in view) */
	int inview;						/**< Total number of satellites in view */
	nmeaSATELLITE sat[NMEA_MAXSAT]; /**< Satellites information (in view) */
} nmeaSATINFO;

/**
 * Summary GPS information from all parsed packets,
 * used also for generating NMEA stream
 * @see nmea_parse
 * @see nmea_GPGGA2info,  nmea_...2info
 */
typedef struct _nmeaINFO {
	uint32_t present;				/**< Mask specifying which fields are present */

	int smask;						/**< Mask specifying from which sentences data has been obtained */

	nmeaTIME utc;					/**< UTC of position */

	int sig;						/**< GPS quality indicator: 0 = Invalid
	                                                            1 = Fix;
											                    2 = Differential
											                    3 = Sensitive
											                    4 = Real Time Kinematic
											                    5 = Float RTK,
											                    6 = estimated (dead reckoning) (v2.3)
											                    7 = Manual input mode
											                    8 = Simulation mode) */

	int fix;						/**< Operating mode, used for navigation: 1 = Fix not available
	                                                                          2 = 2D
	                                                                          3 = 3D) */

	double PDOP;					/**< Position Dilution Of Precision */
	double HDOP;					/**< Horizontal Dilution Of Precision */
	double VDOP;					/**< Vertical Dilution Of Precision */

	double lat;						/**< Latitude in NDEG:  +/-[degree][min].[sec/60] */
	double lon;						/**< Longitude in NDEG: +/-[degree][min].[sec/60] */
	double elv;						/**< Antenna altitude above/below mean sea level (geoid) in meters */
	double speed;					/**< Speed over the ground in kph */
	double track;					/**< Track angle in degrees True */
	double mtrack;					/**< Magnetic Track angle in degrees True */
	double magvar;					/**< Magnetic variation degrees */

	nmeaSATINFO satinfo;			/**< Satellites information */
} nmeaINFO;

/**
 * Enumeration for the fields names of a nmeaINFO structure.
 * The values are used in the 'present' mask.
 */
typedef enum _nmeaINFO_FIELD {
	SMASK			= (1 << 0),  /* 0x00001 */
	UTCDATE			= (1 << 1),  /* 0x00002 */
	UTCTIME			= (1 << 2),  /* 0x00004 */
	SIG				= (1 << 3),  /* 0x00008 */
	FIX				= (1 << 4),  /* 0x00010 */
	PDOP			= (1 << 5),  /* 0x00020 */
	HDOP			= (1 << 6),  /* 0x00040 */
	VDOP			= (1 << 7),  /* 0x00080 */
	LAT				= (1 << 8),  /* 0x00100 */
	LON				= (1 << 9),  /* 0x00200 */
	ELV				= (1 << 10), /* 0x00400 */
	SPEED			= (1 << 11), /* 0x00800 */
	TRACK			= (1 << 12), /* 0x01000 */
	MTRACK			= (1 << 13), /* 0x02000 */
	MAGVAR			= (1 << 14), /* 0x04000 */
	SATINUSECOUNT	= (1 << 15), /* 0x08000 */
	SATINUSE		= (1 << 16), /* 0x10000 */
	SATINVIEW		= (1 << 17), /* 0x20000 */
	_nmeaINFO_FIELD_LAST = SATINVIEW
} nmeaINFO_FIELD;

#define NMEA_INFO_PRESENT_MASK ((_nmeaINFO_FIELD_LAST << 1) - 1)

void nmea_time_now(nmeaTIME *utc, uint32_t * present);
void nmea_zero_INFO(nmeaINFO *info);

bool nmea_INFO_is_present_smask(int smask, nmeaINFO_FIELD fieldName);
bool nmea_INFO_is_present(uint32_t present, nmeaINFO_FIELD fieldName);
void nmea_INFO_set_present(uint32_t * present, nmeaINFO_FIELD fieldName);
void nmea_INFO_unset_present(uint32_t * present, nmeaINFO_FIELD fieldName);

void nmea_INFO_sanitise(nmeaINFO *nmeaInfo);

void nmea_INFO_unit_conversion(nmeaINFO * nmeaInfo);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEA_INFO_H__ */
