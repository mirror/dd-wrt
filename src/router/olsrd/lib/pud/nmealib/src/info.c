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

#include <nmea/info.h>

#include <nmea/sentence.h>
#include <nmea/gmath.h>

#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>

/**
 * Reset the time to now
 *
 * @param utc a pointer to the time structure
 * @param present a pointer to a present field. when non-NULL then the UTCDATE
 * and UTCTIME flags are set in it.
 */
void nmea_time_now(nmeaTIME *utc, uint32_t * present) {
	struct timeval tp;
	struct tm tt;

	assert(utc);

	gettimeofday(&tp, NULL );
	gmtime_r(&tp.tv_sec, &tt);

	utc->year = tt.tm_year;
	utc->mon = tt.tm_mon;
	utc->day = tt.tm_mday;
	utc->hour = tt.tm_hour;
	utc->min = tt.tm_min;
	utc->sec = tt.tm_sec;
	utc->hsec = (tp.tv_usec / 10000);
	if (present) {
	  nmea_INFO_set_present(present, UTCDATE | UTCTIME);
	}
}

/**
 * Clear an info structure.
 * Resets the time to now, sets up the signal as BAD, the FIX as BAD, and
 * signals presence of these fields.
 * Resets all other fields to 0.
 *
 * @param info a pointer to the structure
 */
void nmea_zero_INFO(nmeaINFO *info) {
	if (!info) {
		return;
	}

	memset(info, 0, sizeof(nmeaINFO));
	nmea_time_now(&info->utc, &info->present);

	info->sig = NMEA_SIG_BAD;
	nmea_INFO_set_present(&info->present, SIG);

	info->fix = NMEA_FIX_BAD;
	nmea_INFO_set_present(&info->present, FIX);
}

/**
 * Determine if a nmeaINFO structure has a certain field (from the smask).
 * Note: this is not the complete truth as fields may be absent in certain
 * sentences and are presented as 'present' by this function.
 *
 * @deprecated use nmea_INFO_is_present instead
 * @param smask the smask field
 * @param fieldName use a name from nmeaINFO_FIELD
 * @return a boolean, true when the structure has the requested field
 */
bool nmea_INFO_is_present_smask(int smask, nmeaINFO_FIELD fieldName) {
	switch (fieldName) {
	case SMASK:
		return true;

	case ELV:
		return ((smask & GPGGA) != 0);

	case PDOP:
	case VDOP:
	case SATINUSE:
		return ((smask & GPGSA) != 0);

	case SATINVIEW:
		return ((smask & GPGSV) != 0);

	case UTCDATE:
	case MAGVAR:
		return ((smask & GPRMC) != 0);

	case MTRACK:
		return ((smask & GPVTG) != 0);

	case SATINUSECOUNT:
	case HDOP:
		return ((smask & (GPGGA | GPGSA)) != 0);

	case UTCTIME:
	case SIG:
	case LAT:
	case LON:
		return ((smask & (GPGGA | GPRMC)) != 0);

	case FIX:
		return ((smask & (GPGSA | GPRMC)) != 0);

	case SPEED:
	case TRACK:
		return ((smask & (GPRMC | GPVTG)) != 0);

	default:
		return false;
	}
}

/**
 * Determine if a nmeaINFO structure has a certain field
 *
 * @param present the presence field
 * @param fieldName use a name from nmeaINFO_FIELD
 * @return a boolean, true when the structure has the requested field
 */
bool nmea_INFO_is_present(uint32_t present, nmeaINFO_FIELD fieldName) {
	return ((present & fieldName) != 0);
}

/**
 * Flag a nmeaINFO structure to contain a certain field
 *
 * @param present a pointer to the presence field
 * @param fieldName use a name from nmeaINFO_FIELD
 */
void nmea_INFO_set_present(uint32_t * present, nmeaINFO_FIELD fieldName) {
	assert(present);
	*present |= fieldName;
}

/**
 * Flag a nmeaINFO structure to NOT contain a certain field
 *
 * @param present a pointer to the presence field
 * @param fieldName use a name from nmeaINFO_FIELD
 */
void nmea_INFO_unset_present(uint32_t * present, nmeaINFO_FIELD fieldName) {
	assert(present);
	*present &= ~fieldName;
}

/**
 * Sanitise the NMEA info, make sure that:
 * - sig is in the range [0, 8],
 * - fix is in the range [1, 3],
 * - DOPs are positive,
 * - latitude is in the range [-9000, 9000],
 * - longitude is in the range [-18000, 18000],
 * - speed is positive,
 * - track is in the range [0, 360>.
 * - mtrack is in the range [0, 360>.
 * - magvar is in the range [0, 360>.
 * - satinfo:
 *   - inuse and in_use are consistent (w.r.t. count)
 *   - inview and sat are consistent (w.r.t. count/id)
 *   - in_use and sat are consistent (w.r.t. count/id)
 *   - elv is in the range [0, 90]
 *   - azimuth is in the range [0, 359]
 *   - sig is in the range [0, 99]
 *
 * Time is set to the current time when not present.
 * Fields are reset to their defaults (0) when not signaled as being present.
 *
 * @param nmeaInfo
 * the NMEA info structure to sanitise
 */
void nmea_INFO_sanitise(nmeaINFO *nmeaInfo) {
	double lat = 0;
	double lon = 0;
	double speed = 0;
	double track = 0;
	double mtrack = 0;
	double magvar = 0;
	bool latAdjusted = false;
	bool lonAdjusted = false;
	bool speedAdjusted = false;
	bool trackAdjusted = false;
	bool mtrackAdjusted = false;
	bool magvarAdjusted = false;
	nmeaTIME utc;
	int inuseIndex;
	int inviewIndex;

	if (!nmeaInfo) {
		return;
	}

	nmeaInfo->present = nmeaInfo->present & NMEA_INFO_PRESENT_MASK;

	if (!nmea_INFO_is_present(nmeaInfo->present, SMASK)) {
		nmeaInfo->smask = 0;
	}

	if (!nmea_INFO_is_present(nmeaInfo->present, UTCDATE) || !nmea_INFO_is_present(nmeaInfo->present, UTCTIME)) {
		nmea_time_now(&utc, NULL);
	}

	if (!nmea_INFO_is_present(nmeaInfo->present, UTCDATE)) {
		nmeaInfo->utc.year = utc.year;
		nmeaInfo->utc.mon = utc.mon;
		nmeaInfo->utc.day = utc.day;
	}

	if (!nmea_INFO_is_present(nmeaInfo->present, UTCTIME)) {
		nmeaInfo->utc.hour = utc.hour;
		nmeaInfo->utc.min = utc.min;
		nmeaInfo->utc.sec = utc.sec;
		nmeaInfo->utc.hsec = utc.hsec;
	}

	if (!nmea_INFO_is_present(nmeaInfo->present, SIG)) {
		nmeaInfo->sig = NMEA_SIG_BAD;
	} else {
		if ((nmeaInfo->sig < NMEA_SIG_BAD) || (nmeaInfo->sig > NMEA_SIG_SIM)) {
			nmeaInfo->sig = NMEA_SIG_BAD;
		}
	}

	if (!nmea_INFO_is_present(nmeaInfo->present, FIX)) {
		nmeaInfo->fix = NMEA_FIX_BAD;
	} else {
		if ((nmeaInfo->fix < NMEA_FIX_BAD) || (nmeaInfo->fix > NMEA_FIX_3D)) {
			nmeaInfo->fix = NMEA_FIX_BAD;
		}
	}

	if (!nmea_INFO_is_present(nmeaInfo->present, PDOP)) {
		nmeaInfo->PDOP = 0;
	} else {
		nmeaInfo->PDOP = fabs(nmeaInfo->PDOP);
	}

	if (!nmea_INFO_is_present(nmeaInfo->present, HDOP)) {
		nmeaInfo->HDOP = 0;
	} else {
		nmeaInfo->HDOP = fabs(nmeaInfo->HDOP);
	}

	if (!nmea_INFO_is_present(nmeaInfo->present, VDOP)) {
		nmeaInfo->VDOP = 0;
	} else {
		nmeaInfo->VDOP = fabs(nmeaInfo->VDOP);
	}

	if (!nmea_INFO_is_present(nmeaInfo->present, LAT)) {
		nmeaInfo->lat = 0;
	}

	if (!nmea_INFO_is_present(nmeaInfo->present, LON)) {
		nmeaInfo->lon = 0;
	}

	if (!nmea_INFO_is_present(nmeaInfo->present, ELV)) {
		nmeaInfo->elv = 0;
	}

	if (!nmea_INFO_is_present(nmeaInfo->present, SPEED)) {
		nmeaInfo->speed = 0;
	}

	if (!nmea_INFO_is_present(nmeaInfo->present, TRACK)) {
		nmeaInfo->track = 0;
	}

	if (!nmea_INFO_is_present(nmeaInfo->present, MTRACK)) {
		nmeaInfo->mtrack = 0;
	}

	if (!nmea_INFO_is_present(nmeaInfo->present, MAGVAR)) {
		nmeaInfo->magvar = 0;
	}

	if (!nmea_INFO_is_present(nmeaInfo->present, SATINUSECOUNT)) {
		nmeaInfo->satinfo.inuse = 0;
	}
	if (!nmea_INFO_is_present(nmeaInfo->present, SATINUSE)) {
		memset(&nmeaInfo->satinfo.in_use, 0, sizeof(nmeaInfo->satinfo.in_use));
	}
	if (!nmea_INFO_is_present(nmeaInfo->present, SATINVIEW)) {
		nmeaInfo->satinfo.inview = 0;
		memset(&nmeaInfo->satinfo.sat, 0, sizeof(nmeaInfo->satinfo.sat));
	}

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
	track = nmeaInfo->track;
	mtrack = nmeaInfo->mtrack;

	if (speed < 0.0) {
		speed = -speed;
		track += 180.0;
		mtrack += 180.0;
		speedAdjusted = true;
		trackAdjusted = true;
		mtrackAdjusted = true;
	}

	/* speed is now in [0, max> */

	if (speedAdjusted) {
		nmeaInfo->speed = speed;
	}

	/*
	 * track
	 */

	/* force track in [0, 360> */
	while (track < 0.0) {
		track += 360.0;
		trackAdjusted = true;
	}
	while (track >= 360.0) {
		track -= 360.0;
		trackAdjusted = true;
	}

	/* track is now in [0, 360> */

	if (trackAdjusted) {
		nmeaInfo->track = track;
	}

	/*
	 * mtrack
	 */

	/* force mtrack in [0, 360> */
	while (mtrack < 0.0) {
		mtrack += 360.0;
		mtrackAdjusted = true;
	}
	while (mtrack >= 360.0) {
		mtrack -= 360.0;
		mtrackAdjusted = true;
	}

	/* mtrack is now in [0, 360> */

	if (mtrackAdjusted) {
		nmeaInfo->mtrack = mtrack;
	}

	/*
	 * magvar
	 */

	magvar = nmeaInfo->magvar;

	/* force magvar in [0, 360> */
	while (magvar < 0.0) {
		magvar += 360.0;
		magvarAdjusted = true;
	}
	while (magvar >= 360.0) {
		magvar -= 360.0;
		magvarAdjusted = true;
	}

	/* magvar is now in [0, 360> */

	if (magvarAdjusted) {
		nmeaInfo->magvar = magvar;
	}

	/*
	 * satinfo
	 */

	nmeaInfo->satinfo.inuse = 0;
	for (inuseIndex = 0; inuseIndex < NMEA_MAXSAT; inuseIndex++) {
		if (nmeaInfo->satinfo.in_use[inuseIndex])
			nmeaInfo->satinfo.inuse++;
	}

	nmeaInfo->satinfo.inview = 0;
	for (inviewIndex = 0; inviewIndex < NMEA_MAXSAT; inviewIndex++) {
		if (nmeaInfo->satinfo.sat[inviewIndex].id) {
			nmeaInfo->satinfo.inview++;

			/* force elv in [-180, 180] */
			while (nmeaInfo->satinfo.sat[inviewIndex].elv < -180) {
				nmeaInfo->satinfo.sat[inviewIndex].elv += 360;
			}
			while (nmeaInfo->satinfo.sat[inviewIndex].elv > 180) {
				nmeaInfo->satinfo.sat[inviewIndex].elv -= 360;
			}

			/* elv is now in [-180, 180] */

			/* force elv from <90, 180] in [90, 0] */
			if (nmeaInfo->satinfo.sat[inviewIndex].elv > 90) {
				nmeaInfo->satinfo.sat[inviewIndex].elv = 180 - nmeaInfo->satinfo.sat[inviewIndex].elv;
			}

			/* force elv from [-180, -90> in [0, -90] */
			if (nmeaInfo->satinfo.sat[inviewIndex].elv < -90) {
				nmeaInfo->satinfo.sat[inviewIndex].elv = -180 - nmeaInfo->satinfo.sat[inviewIndex].elv;
			}

			/* elv is now in [-90, 90] */

			if (nmeaInfo->satinfo.sat[inviewIndex].elv < 0) {
				nmeaInfo->satinfo.sat[inviewIndex].elv = -nmeaInfo->satinfo.sat[inviewIndex].elv;
			}

			/* elv is now in [0, 90] */

			/* force azimuth in [0, 360> */
			while (nmeaInfo->satinfo.sat[inviewIndex].azimuth < 0) {
				nmeaInfo->satinfo.sat[inviewIndex].azimuth += 360;
			}
			while (nmeaInfo->satinfo.sat[inviewIndex].azimuth >= 360) {
				nmeaInfo->satinfo.sat[inviewIndex].azimuth -= 360;
			}
			/* azimuth is now in [0, 360> */

			/* force sig in [0, 99] */
			if (nmeaInfo->satinfo.sat[inviewIndex].sig < 0)
				nmeaInfo->satinfo.sat[inviewIndex].sig = 0;
			if (nmeaInfo->satinfo.sat[inviewIndex].sig > 99)
				nmeaInfo->satinfo.sat[inviewIndex].sig = 99;
		}
	}

	/* make sure the in_use IDs map to sat IDs */
	for (inuseIndex = 0; inuseIndex < NMEA_MAXSAT; inuseIndex++) {
		int inuseID = nmeaInfo->satinfo.in_use[inuseIndex];
		if (inuseID) {
			bool found = false;
			for (inviewIndex = 0; inviewIndex < NMEA_MAXSAT; inviewIndex++) {
				int inviewID = nmeaInfo->satinfo.sat[inviewIndex].id;
				if (inuseID == inviewID) {
					found = true;
					break;
				}
			}
			if (!found) {
				/* clear the id, did not find it */
				nmeaInfo->satinfo.in_use[inuseIndex] = 0;
				if (nmeaInfo->satinfo.inuse)
					nmeaInfo->satinfo.inuse--;
			}
		}
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

	if (nmea_INFO_is_present(nmeaInfo->present, PDOP)) {
		nmeaInfo->PDOP = nmea_dop2meters(nmeaInfo->PDOP);
	}

	if (nmea_INFO_is_present(nmeaInfo->present, HDOP)) {
		nmeaInfo->HDOP = nmea_dop2meters(nmeaInfo->HDOP);
	}

	if (nmea_INFO_is_present(nmeaInfo->present, VDOP)) {
		nmeaInfo->VDOP = nmea_dop2meters(nmeaInfo->VDOP);
	}

	if (nmea_INFO_is_present(nmeaInfo->present, LAT)) {
		nmeaInfo->lat = nmea_ndeg2degree(nmeaInfo->lat);
	}

	if (nmea_INFO_is_present(nmeaInfo->present, LON)) {
		nmeaInfo->lon = nmea_ndeg2degree(nmeaInfo->lon);
	}

	/* elv (already in correct format) */
	/* speed (already in correct format) */
	/* track (already in correct format) */
	/* mtrack (already in correct format) */
	/* magvar (already in correct format) */

	/* satinfo (already in correct format) */
}
