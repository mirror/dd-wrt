/*
 * This file is part of nmealib.
 *
 * Copyright (c) 2008 Timur Sinitsyn
 * Copyright (c) 2011 Ferry Huberts
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

#include <nmea/info.h>

#include <nmea/sentence.h>
#include <nmea/gmath.h>
#include <nmea/time.h>

#include <string.h>
#include <math.h>

void nmea_zero_INFO(nmeaINFO *info)
{
	if (!info) {
		return;
	}

    memset(info, 0, sizeof(nmeaINFO));
    nmea_time_now(&info->utc);
    info->sig = NMEA_SIG_BAD;
    info->fix = NMEA_FIX_BAD;
}

/**
 * Determine whether a given nmeaINFO structure has a certain field.
 *
 * nmeaINFO dependencies:
 <pre>
 field/sentence GPGGA   GPGSA   GPGSV   GPRMC   GPVTG
 smask:         x       x       x       x       x
 utc:           x                       x
 sig:           x                       x
 fix:                   x               x
 PDOP:                  x
 HDOP:          x       x
 VDOP:                  x
 lat:           x                       x
 lon:           x                       x
 elv:           x
 speed:                                 x       x
 direction:                             x       x
 declination:                                   x
 satinfo:               x       x
 </pre>
 *
 * @param smask
 * the smask of a nmeaINFO structure
 * @param fieldName
 * the field name
 *
 * @return
 * - true when the nmeaINFO structure has the field
 * - false otherwise
 */
bool nmea_INFO_has_field(int smask, nmeaINFO_FIELD fieldName) {
	switch (fieldName) {
		case SMASK:
			return true;

		case UTC:
		case SIG:
		case LAT:
		case LON:
			return ((smask & (GPGGA | GPRMC)) != 0);

		case FIX:
			return ((smask & (GPGSA | GPRMC)) != 0);

		case PDOP:
		case VDOP:
			return ((smask & GPGSA) != 0);

		case HDOP:
			return ((smask & (GPGGA | GPGSA)) != 0);

		case ELV:
			return ((smask & GPGGA) != 0);

		case SPEED:
		case DIRECTION:
			return ((smask & (GPRMC | GPVTG)) != 0);

		case DECLINATION:
			return ((smask & GPVTG) != 0);

		case SATINFO:
			return ((smask & (GPGSA | GPGSV)) != 0);

		default:
			return false;
	}
}

/**
 * Sanitise the NMEA info, make sure that:
 * - latitude is in the range [-9000, 9000],
 * - longitude is in the range [-18000, 18000],
 * - DOPs are positive,
 * - speed is positive,
 * - direction is in the range [0, 360>.
 *
 * Time is set to the current time when not present.
 *
 * When a field is not present then it is reset to its default (NMEA_SIG_BAD,
 * NMEA_FIX_BAD, 0).
 *
 * Satinfo is not touched.
 *
 * @param nmeaInfo
 * the NMEA info structure to sanitise
 */
void nmea_INFO_sanitise(nmeaINFO *nmeaInfo) {
	double lat = 0;
	double lon = 0;
	double speed = 0;
	double direction = 0;
	bool latAdjusted = false;
	bool lonAdjusted = false;
	bool speedAdjusted = false;
	bool directionAdjusted = false;

	if (!nmeaInfo) {
		return;
	}

	if (!nmea_INFO_has_field(nmeaInfo->smask, UTC)) {
		nmea_time_now(&nmeaInfo->utc);
	}

	if (!nmea_INFO_has_field(nmeaInfo->smask, SIG)) {
		nmeaInfo->sig = NMEA_SIG_BAD;
	}

	if (!nmea_INFO_has_field(nmeaInfo->smask, FIX)) {
		nmeaInfo->fix = NMEA_FIX_BAD;
	}

	if (!nmea_INFO_has_field(nmeaInfo->smask, PDOP)) {
		nmeaInfo->PDOP = 0;
	} else {
		nmeaInfo->PDOP = fabs(nmeaInfo->PDOP);
	}

	if (!nmea_INFO_has_field(nmeaInfo->smask, HDOP)) {
		nmeaInfo->HDOP = 0;
	} else {
		nmeaInfo->HDOP = fabs(nmeaInfo->HDOP);
	}

	if (!nmea_INFO_has_field(nmeaInfo->smask, VDOP)) {
		nmeaInfo->VDOP = 0;
	} else {
		nmeaInfo->VDOP = fabs(nmeaInfo->VDOP);
	}

	if (!nmea_INFO_has_field(nmeaInfo->smask, LAT)) {
		nmeaInfo->lat = 0;
	}

	if (!nmea_INFO_has_field(nmeaInfo->smask, LON)) {
		nmeaInfo->lon = 0;
	}

	if (!nmea_INFO_has_field(nmeaInfo->smask, ELV)) {
		nmeaInfo->elv = 0;
	}

	if (!nmea_INFO_has_field(nmeaInfo->smask, SPEED)) {
		nmeaInfo->speed = 0;
	}

	if (!nmea_INFO_has_field(nmeaInfo->smask, DIRECTION)) {
		nmeaInfo->direction = 0;
	}

	if (!nmea_INFO_has_field(nmeaInfo->smask, DECLINATION)) {
		nmeaInfo->declination = 0;
	}

	/* satinfo is not used */

	/*
	 * lat
	 */

	lat = nmeaInfo->lat;
	lon = nmeaInfo->lon;

	/* force lat in [-18000, 18000] */
	while (lat < -18000.0) {
		lat += 36000.0;
		latAdjusted = true;
	}
	while (lat > 18000.0) {
		lat -= 36000.0;
		latAdjusted = true;
	}

	/* lat is now in [-18000, 18000] */

	/* force lat from <9000, 18000] in [9000, 0] */
	if (lat > 9000.0) {
		lat = 18000.0 - lat;
		lon += 18000.0;
		latAdjusted = true;
		lonAdjusted = true;
	}

	/* force lat from [-18000, -9000> in [0, -9000] */
	if (lat < -9000.0) {
		lat = -18000.0 - lat;
		lon += 18000.0;
		latAdjusted = true;
		lonAdjusted = true;
	}

	/* lat is now in [-9000, 9000] */

	if (latAdjusted) {
		nmeaInfo->lat = lat;
	}

	/*
	 * lon
	 */

	/* force lon in [-18000, 18000] */
	while (lon < -18000.0) {
		lon += 36000.0;
		lonAdjusted = true;
	}
	while (lon > 18000.0) {
		lon -= 36000.0;
		lonAdjusted = true;
	}

	/* lon is now in [-18000, 18000] */

	if (lonAdjusted) {
		nmeaInfo->lon = lon;
	}

	/*
	 * speed
	 */

	speed = nmeaInfo->speed;
	direction = nmeaInfo->direction;

	if (speed < 0.0) {
		speed = -speed;
		direction += 180.0;
		speedAdjusted = true;
		directionAdjusted = true;
	}

	/* speed is now in [0, max> */

	if (speedAdjusted) {
		nmeaInfo->speed = speed;
	}

	/*
	 * direction
	 */

	/* force direction in [0, 360> */
	while (direction < 0.0) {
		direction += 360.0;
		directionAdjusted = true;
	}
	while (direction >= 360.0) {
		direction -= 360.0;
		directionAdjusted = true;
	}

	/* direction is now in [0, 360> */

	if (directionAdjusted) {
		nmeaInfo->direction = direction;
	}
}

/**
 * Converts the position fields to degrees and DOP fields to meters so that
 * all fields use normal metric units.
 *
 * @param nmeaInfo
 * the nmeaINFO
 */
void nmea_INFO_unit_conversion(nmeaINFO * nmeaInfo) {
	if (!nmeaInfo) {
		return;
	}

	/* smask (already in correct format) */

	/* utc (already in correct format) */

	/* sig (already in correct format) */
	/* fix (already in correct format) */

	if (nmea_INFO_has_field(nmeaInfo->smask, PDOP)) {
		nmeaInfo->PDOP = nmea_dop2meters(nmeaInfo->PDOP);
	}

	if (nmea_INFO_has_field(nmeaInfo->smask, HDOP)) {
		nmeaInfo->HDOP = nmea_dop2meters(nmeaInfo->HDOP);
	}

	if (nmea_INFO_has_field(nmeaInfo->smask, VDOP)) {
		nmeaInfo->VDOP = nmea_dop2meters(nmeaInfo->VDOP);
	}

	if (nmea_INFO_has_field(nmeaInfo->smask, LAT)) {
		nmeaInfo->lat = nmea_ndeg2degree(nmeaInfo->lat);
	}

	if (nmea_INFO_has_field(nmeaInfo->smask, LON)) {
		nmeaInfo->lon = nmea_ndeg2degree(nmeaInfo->lon);
	}

	/* elv (already in correct format) */
	/* speed (already in correct format) */
	/* direction (already in correct format) */
	/* declination (already in correct format) */

	/* satinfo (not used) */
}
