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

#include <nmea/conversions.h>

#include <nmea/gmath.h>

#include <assert.h>
#include <string.h>
#include <math.h>

/**
 * Determine the number of GSV sentences needed for a number of sats
 *
 * @param sats the number of sats
 * @return the number of GSV sentences needed
 */
int nmea_gsv_npack(int sats) {
	int pack_count = sats / NMEA_SATINPACK;

	if ((sats % NMEA_SATINPACK) > 0)
		pack_count++;

	if (!pack_count)
		pack_count++;

	return pack_count;
}

/**
 * Fill nmeaINFO structure from GGA packet structure
 *
 * @param pack a pointer to the packet structure
 * @param info a pointer to the nmeaINFO structure
 */
void nmea_GPGGA2info(const nmeaGPGGA *pack, nmeaINFO *info) {
	assert(pack);
	assert(info);

	info->present |= pack->present;
	nmea_INFO_set_present(&info->present, SMASK);
	info->smask |= GPGGA;
	if (nmea_INFO_is_present(pack->present, UTCTIME)) {
		info->utc.hour = pack->utc.hour;
		info->utc.min = pack->utc.min;
		info->utc.sec = pack->utc.sec;
		info->utc.hsec = pack->utc.hsec;
	}
	if (nmea_INFO_is_present(pack->present, LAT)) {
		info->lat = ((pack->ns == 'N') ? pack->lat : -pack->lat);
	}
	if (nmea_INFO_is_present(pack->present, LON)) {
		info->lon = ((pack->ew == 'E') ? pack->lon : -pack->lon);
	}
	if (nmea_INFO_is_present(pack->present, SIG)) {
		info->sig = pack->sig;
	}
	if (nmea_INFO_is_present(pack->present, SATINUSECOUNT)) {
		info->satinfo.inuse = pack->satinuse;
	}
	if (nmea_INFO_is_present(pack->present, HDOP)) {
		info->HDOP = pack->HDOP;
	}
	if (nmea_INFO_is_present(pack->present, ELV)) {
		info->elv = pack->elv;
	}
	/* ignore diff and diff_units */
	/* ignore dgps_age and dgps_sid */
}

/**
 * Convert an nmeaINFO structure into an nmeaGPGGA structure
 *
 * @param info a pointer to the nmeaINFO structure
 * @param pack a pointer to the nmeaGPGGA structure
 */
void nmea_info2GPGGA(const nmeaINFO *info, nmeaGPGGA *pack) {
	assert(pack);
	assert(info);

	nmea_zero_GPGGA(pack);

	pack->present = info->present;
	nmea_INFO_unset_present(&pack->present, SMASK);
	if (nmea_INFO_is_present(info->present, UTCTIME)) {
		pack->utc.hour = info->utc.hour;
		pack->utc.min = info->utc.min;
		pack->utc.sec = info->utc.sec;
		pack->utc.hsec = info->utc.hsec;
	}
	if (nmea_INFO_is_present(info->present, LAT)) {
		pack->lat = fabs(info->lat);
		pack->ns = ((info->lat > 0) ? 'N' : 'S');
	}
	if (nmea_INFO_is_present(info->present, LON)) {
		pack->lon = fabs(info->lon);
		pack->ew = ((info->lon > 0) ? 'E' : 'W');
	}
	if (nmea_INFO_is_present(info->present, SIG)) {
		pack->sig = info->sig;
	}
	if (nmea_INFO_is_present(info->present, SATINUSECOUNT)) {
		pack->satinuse = info->satinfo.inuse;
	}
	if (nmea_INFO_is_present(info->present, HDOP)) {
		pack->HDOP = info->HDOP;
	}
	if (nmea_INFO_is_present(info->present, ELV)) {
		pack->elv = info->elv;
		pack->elv_units = 'M';
	}
	/* defaults for (ignored) diff and diff_units */
	pack->diff = 0;
	pack->diff_units = 'M';
	/* defaults for (ignored) dgps_age and dgps_sid */
	pack->dgps_age = 0;
	pack->dgps_sid = 0;
}

/**
 * Fill nmeaINFO structure from GSA packet structure
 *
 * @param pack a pointer to the packet structure
 * @param info a pointer to the nmeaINFO structure
 */
void nmea_GPGSA2info(const nmeaGPGSA *pack, nmeaINFO *info) {
	int i = 0;

	assert(pack);
	assert(info);

	info->present |= pack->present;
	nmea_INFO_set_present(&info->present, SMASK);
	info->smask |= GPGSA;
	if (nmea_INFO_is_present(pack->present, FIX)) {
		/* fix_mode is ignored */
		info->fix = pack->fix_type;
	}
	if (nmea_INFO_is_present(pack->present, SATINUSE)) {
		info->satinfo.inuse = 0;
		for (i = 0; i < NMEA_MAXSAT; i++) {
			info->satinfo.in_use[i] = pack->sat_prn[i];
			if (pack->sat_prn[i]) {
				info->satinfo.inuse++;
			}
		}
		nmea_INFO_set_present(&info->present, SATINUSECOUNT);
	}
	if (nmea_INFO_is_present(pack->present, PDOP)) {
		info->PDOP = pack->PDOP;
	}
	if (nmea_INFO_is_present(pack->present, HDOP)) {
		info->HDOP = pack->HDOP;
	}
	if (nmea_INFO_is_present(pack->present, VDOP)) {
		info->VDOP = pack->VDOP;
	}
}

/**
 * Convert an nmeaINFO structure into an nmeaGPGSA structure
 *
 * @param info a pointer to the nmeaINFO structure
 * @param pack a pointer to the nmeaGPGSA structure
 */
void nmea_info2GPGSA(const nmeaINFO *info, nmeaGPGSA *pack) {
	assert(pack);
	assert(info);

	nmea_zero_GPGSA(pack);

	pack->present = info->present;
	nmea_INFO_unset_present(&pack->present, SMASK);
	if (nmea_INFO_is_present(info->present, FIX)) {
		pack->fix_mode = 'A';
		pack->fix_type = info->fix;
	}
	if (nmea_INFO_is_present(info->present, SATINUSE)) {
		memcpy(pack->sat_prn, info->satinfo.in_use, sizeof(pack->sat_prn));
	}
	if (nmea_INFO_is_present(info->present, PDOP)) {
		pack->PDOP = info->PDOP;
	}
	if (nmea_INFO_is_present(info->present, HDOP)) {
		pack->HDOP = info->HDOP;
	}
	if (nmea_INFO_is_present(info->present, VDOP)) {
		pack->VDOP = info->VDOP;
	}
}

/**
 * Fill nmeaINFO structure from GSV packet structure
 *
 * @param pack a pointer to the packet structure
 * @param info a pointer to the nmeaINFO structure
 */
void nmea_GPGSV2info(const nmeaGPGSV *pack, nmeaINFO *info) {
	int pack_index;

	assert(pack);
	assert(info);

	pack_index = pack->pack_index;
	if (pack_index < 1)
		pack_index = 1;

	if (pack_index > pack->pack_count)
		pack_index = pack->pack_count;

	if ((pack_index * NMEA_SATINPACK) > NMEA_MAXSAT)
		pack_index = NMEA_NSATPACKS;

	info->present |= pack->present;
	nmea_INFO_set_present(&info->present, SMASK);
	info->smask |= GPGSV;
	if (nmea_INFO_is_present(pack->present, SATINVIEW)) {
		int sat_index;

		/* index of 1st sat in pack */
		int sat_offset = (pack_index - 1) * NMEA_SATINPACK;
		/* the number of sats in this sentence */
		int sat_count = ((sat_offset + NMEA_SATINPACK) > pack->sat_count) ? (pack->sat_count - sat_offset) : NMEA_SATINPACK;

		for (sat_index = 0; sat_index < sat_count; sat_index++) {
			info->satinfo.sat[sat_offset + sat_index].id = pack->sat_data[sat_index].id;
			info->satinfo.sat[sat_offset + sat_index].elv = pack->sat_data[sat_index].elv;
			info->satinfo.sat[sat_offset + sat_index].azimuth = pack->sat_data[sat_index].azimuth;
			info->satinfo.sat[sat_offset + sat_index].sig = pack->sat_data[sat_index].sig;
		}

		info->satinfo.inview = pack->sat_count;
	}
}

/**
 * Convert an nmeaINFO structure into an nmeaGPGSV structure
 *
 * @param info a pointer to the nmeaINFO structure
 * @param pack a pointer to the nmeaGPGSV structure
 * @param pack_idx pack index (zero based)
 */
void nmea_info2GPGSV(const nmeaINFO *info, nmeaGPGSV *pack, int pack_idx) {
	assert(pack);
	assert(info);

	nmea_zero_GPGSV(pack);

	pack->present = info->present;
	nmea_INFO_unset_present(&pack->present, SMASK);
	if (nmea_INFO_is_present(info->present, SATINVIEW)) {
		int sit;
		int pit;
		int toskip;

		pack->sat_count = (info->satinfo.inview < NMEA_MAXSAT) ? info->satinfo.inview : NMEA_MAXSAT;
		pack->pack_count = nmea_gsv_npack(pack->sat_count);

		if (pack_idx >= pack->pack_count)
			pack->pack_index = pack->pack_count;
		else
			pack->pack_index = pack_idx + 1;

		/* now skip the first ((pack->pack_index - 1) * NMEA_SATINPACK) in view sats */
		toskip = ((pack->pack_index - 1) * NMEA_SATINPACK);
		sit = 0;
		while ((toskip > 0) && (sit < NMEA_MAXSAT)) {
			if (info->satinfo.sat[sit].id) {
				toskip--;
			}
			sit++;
		}

		for (pit = 0; pit < NMEA_SATINPACK; sit++) {
			if (sit < NMEA_MAXSAT) {
				if (info->satinfo.sat[sit].id) {
					pack->sat_data[pit] = info->satinfo.sat[sit];
					pit++;
				}
			} else {
				memset(&pack->sat_data[pit], 0, sizeof(pack->sat_data[pit]));
				pit++;
			}
		}
	}
}

/**
 * Fill nmeaINFO structure from RMC packet structure
 *
 * @param pack a pointer to the packet structure
 * @param info a pointer to the nmeaINFO structure
 */
void nmea_GPRMC2info(const nmeaGPRMC *pack, nmeaINFO *info) {
	assert(pack);
	assert(info);

	info->present |= pack->present;
	nmea_INFO_set_present(&info->present, SMASK);
	info->smask |= GPRMC;
	if (nmea_INFO_is_present(pack->present, UTCDATE)) {
		info->utc.year = pack->utc.year;
		info->utc.mon = pack->utc.mon;
		info->utc.day = pack->utc.day;
	}
	if (nmea_INFO_is_present(pack->present, UTCTIME)) {
		info->utc.hour = pack->utc.hour;
		info->utc.min = pack->utc.min;
		info->utc.sec = pack->utc.sec;
		info->utc.hsec = pack->utc.hsec;
	}
	nmea_INFO_set_present(&info->present, SIG);
	nmea_INFO_set_present(&info->present, FIX);
	if (pack->status == 'A') {
		if (info->sig == NMEA_SIG_BAD) {
			info->sig = NMEA_SIG_MID;
		}
		if (info->fix == NMEA_FIX_BAD) {
			info->fix = NMEA_FIX_2D;
		}
	} else {
		info->sig = NMEA_SIG_BAD;
		info->fix = NMEA_FIX_BAD;
	}
	if (nmea_INFO_is_present(pack->present, LAT)) {
		info->lat = ((pack->ns == 'N') ? pack->lat : -pack->lat);
	}
	if (nmea_INFO_is_present(pack->present, LON)) {
		info->lon = ((pack->ew == 'E') ? pack->lon : -pack->lon);
	}
	if (nmea_INFO_is_present(pack->present, SPEED)) {
		info->speed = pack->speed * NMEA_TUD_KNOTS;
	}
	if (nmea_INFO_is_present(pack->present, TRACK)) {
		info->track = pack->track;
	}
	if (nmea_INFO_is_present(pack->present, MAGVAR)) {
		info->magvar = ((pack->magvar_ew == 'E') ? pack->magvar : -pack->magvar);
	}
	/* mode is ignored */
}

/**
 * Convert an nmeaINFO structure into an nmeaGPRMC structure
 *
 * @param info a pointer to the nmeaINFO structure
 * @param pack a pointer to the nmeaGPRMC structure
 */
void nmea_info2GPRMC(const nmeaINFO *info, nmeaGPRMC *pack) {
	assert(pack);
	assert(info);

	nmea_zero_GPRMC(pack);

	pack->present = info->present;
	nmea_INFO_unset_present(&pack->present, SMASK);
	if (nmea_INFO_is_present(info->present, UTCDATE)) {
		pack->utc.year = info->utc.year;
		pack->utc.mon = info->utc.mon;
		pack->utc.day = info->utc.day;
	}
	if (nmea_INFO_is_present(info->present, UTCTIME)) {
		pack->utc.hour = info->utc.hour;
		pack->utc.min = info->utc.min;
		pack->utc.sec = info->utc.sec;
		pack->utc.hsec = info->utc.hsec;
	}
	if (nmea_INFO_is_present(info->present, SIG)) {
		pack->status = ((info->sig != NMEA_SIG_BAD) ? 'A' : 'V');
	} else {
		pack->status = 'V';
	}
	if (nmea_INFO_is_present(info->present, LAT)) {
		pack->lat = fabs(info->lat);
		pack->ns = ((info->lat > 0) ? 'N' : 'S');
	}
	if (nmea_INFO_is_present(info->present, LON)) {
		pack->lon = fabs(info->lon);
		pack->ew = ((info->lon > 0) ? 'E' : 'W');
	}
	if (nmea_INFO_is_present(info->present, SPEED)) {
		pack->speed = info->speed / NMEA_TUD_KNOTS;
	}
	if (nmea_INFO_is_present(info->present, TRACK)) {
		pack->track = info->track;
	}
	if (nmea_INFO_is_present(info->present, MAGVAR)) {
		pack->magvar = fabs(info->magvar);
		pack->magvar_ew = ((info->magvar > 0) ? 'E' : 'W');
	}
	if (nmea_INFO_is_present(info->present, SIG)) {
		pack->mode = ((info->sig != NMEA_SIG_BAD) ? 'A' : 'N');
	} else {
		pack->mode = 'N';
	}
}

/**
 * Fill nmeaINFO structure from VTG packet structure
 *
 * @param pack a pointer to the packet structure
 * @param info a pointer to the nmeaINFO structure
 */
void nmea_GPVTG2info(const nmeaGPVTG *pack, nmeaINFO *info) {
	assert(pack);
	assert(info);

	info->present |= pack->present;
	nmea_INFO_set_present(&info->present, SMASK);
	info->smask |= GPVTG;
	if (nmea_INFO_is_present(pack->present, SPEED)) {
		info->speed = pack->spk;
	}
	if (nmea_INFO_is_present(pack->present, TRACK)) {
		info->track = pack->track;
	}
	if (nmea_INFO_is_present(pack->present, MTRACK)) {
		info->mtrack = pack->mtrack;
	}
}

/**
 * Convert an nmeaINFO structure into an nmeaGPVTG structure
 *
 * @param info a pointer to the nmeaINFO structure
 * @param pack a pointer to the nmeaGPRMC structure
 */
void nmea_info2GPVTG(const nmeaINFO *info, nmeaGPVTG *pack) {
	assert(pack);
	assert(info);

	nmea_zero_GPVTG(pack); /* also sets up units */

	pack->present = info->present;
	nmea_INFO_unset_present(&pack->present, SMASK);
	if (nmea_INFO_is_present(info->present, TRACK)) {
		pack->track = info->track;
	}
	if (nmea_INFO_is_present(info->present, MTRACK)) {
		pack->mtrack = info->mtrack;
	}
	if (nmea_INFO_is_present(info->present, SPEED)) {
		pack->spn = info->speed / NMEA_TUD_KNOTS;
		pack->spk = info->speed;
	}
}
