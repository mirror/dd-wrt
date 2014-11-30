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

#include <nmea/generate.h>

#include <nmea/tok.h>
#include <nmea/conversions.h>

#include <stdio.h>
#include <stdbool.h>

/**
 * Generate a GPGGA sentence from an nmeaGPGGA structure
 *
 * @param s a pointer to the buffer to generate the string in
 * @param len the size of the buffer
 * @param pack the structure
 * @return the length of the generated sentence
 */
int nmea_gen_GPGGA(char *s, const int len, const nmeaGPGGA *pack) {
	char sTime[16];
	char sLat[16];
	char sNs[2];
	char sLon[16];
	char sEw[2];
	char sSig[4];
	char sSatInUse[4];
	char sHdop[16];
	char sElv[16];
	char sElvUnit[2];

	sTime[0] = 0;
	sLat[0] = 0;
	sNs[0] = sNs[1] = 0;
	sLon[0] = 0;
	sEw[0] = sEw[1] = 0;
	sSig[0] = 0;
	sSatInUse[0] = 0;
	sHdop[0] = 0;
	sElv[0] = 0;
	sElvUnit[0] = sElvUnit[1] = 0;

	if (nmea_INFO_is_present(pack->present, UTCTIME)) {
		snprintf(&sTime[0], sizeof(sTime), "%02d%02d%02d.%02d", pack->utc.hour, pack->utc.min, pack->utc.sec,
				pack->utc.hsec);
	}
	if (nmea_INFO_is_present(pack->present, LAT)) {
		snprintf(&sLat[0], sizeof(sLat), "%09.4f", pack->lat);
		sNs[0] = pack->ns;
	}
	if (nmea_INFO_is_present(pack->present, LON)) {
		snprintf(&sLon[0], sizeof(sLon), "%010.4f", pack->lon);
		sEw[0] = pack->ew;
	}
	if (nmea_INFO_is_present(pack->present, SIG)) {
		snprintf(&sSig[0], sizeof(sSig), "%1d", pack->sig);
	}
	if (nmea_INFO_is_present(pack->present, SATINUSECOUNT)) {
		snprintf(&sSatInUse[0], sizeof(sSatInUse), "%02d", pack->satinuse);
	}
	if (nmea_INFO_is_present(pack->present, HDOP)) {
		snprintf(&sHdop[0], sizeof(sHdop), "%03.1f", pack->HDOP);
	}
	if (nmea_INFO_is_present(pack->present, ELV)) {
		snprintf(&sElv[0], sizeof(sElv), "%03.1f", pack->elv);
		sElvUnit[0] = pack->elv_units;
	}

	return nmea_printf(s, len, "$GPGGA,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,,,,", &sTime[0], &sLat[0], &sNs[0],
			&sLon[0], &sEw[0], &sSig[0], &sSatInUse[0], &sHdop[0], &sElv[0], &sElvUnit[0]);
}

/**
 * Generate a GPGSA sentence from an nmeaGPGSA structure
 *
 * @param s a pointer to the buffer to generate the string in
 * @param len the size of the buffer
 * @param pack the structure
 * @return the length of the generated sentence
 */
int nmea_gen_GPGSA(char *s, const int len, const nmeaGPGSA *pack) {
	int i;
	char sFixMode[2];
	char sFixType[2];
	char sSatPrn[(NMEA_MAXSAT * 4) + 1];
	char sPdop[16];
	char sHdop[16];
	char sVdop[16];

	char * psSatPrn = &sSatPrn[0];
	int ssSatPrn = sizeof(sSatPrn) - 1;

	bool satinuse = nmea_INFO_is_present(pack->present, SATINUSE);

	sFixMode[0] = sFixMode[1] = 0;
	sFixType[0] = sFixType[1] = 0;
	sSatPrn[0] = 0;
	sPdop[0] = 0;
	sHdop[0] = 0;
	sVdop[0] = 0;

	if (nmea_INFO_is_present(pack->present, FIX)) {
		sFixMode[0] = pack->fix_mode;
		snprintf(&sFixType[0], sizeof(sFixType), "%1d", pack->fix_type);
	}

	for (i = 0; i < NMEA_MAXSAT; i++) {
		if (satinuse && pack->sat_prn[i]) {
			int cnt = snprintf(psSatPrn, ssSatPrn, "%d", pack->sat_prn[i]);
			if (cnt >= ssSatPrn) {
				ssSatPrn = 0;
				psSatPrn = &sSatPrn[sizeof(sSatPrn) - 1];
				*psSatPrn = '\0';
				break;
			} else {
				ssSatPrn -= cnt;
				psSatPrn += cnt;
			}
		}
		if (i < (NMEA_MAXSAT - 1)) {
			*psSatPrn = ',';
			psSatPrn++;
			ssSatPrn--;
			*psSatPrn = '\0';
		}
	}

	if (nmea_INFO_is_present(pack->present, PDOP)) {
		snprintf(&sPdop[0], sizeof(sPdop), "%03.1f", pack->PDOP);
	}
	if (nmea_INFO_is_present(pack->present, HDOP)) {
		snprintf(&sHdop[0], sizeof(sHdop), "%03.1f", pack->HDOP);
	}
	if (nmea_INFO_is_present(pack->present, VDOP)) {
		snprintf(&sVdop[0], sizeof(sVdop), "%03.1f", pack->VDOP);
	}

	return nmea_printf(s, len, "$GPGSA,%s,%s,%s,%s,%s,%s", &sFixMode[0], &sFixType[0], &sSatPrn[0], &sPdop[0],
			&sHdop[0], &sVdop[0]);
}

/**
 * Generate a GPGSV sentence from an nmeaGPGSV structure
 *
 * @param s a pointer to the buffer to generate the string in
 * @param len the size of the buffer
 * @param pack the structure
 * @return the length of the generated sentence
 */
int nmea_gen_GPGSV(char *s, const int len, const nmeaGPGSV *pack) {
	char sCount[2];
	char sIndex[2];
	char sSatCount[4];
	char sSatInfo[(NMEA_SATINPACK * 4) + 1];
	char * psSatInfo = &sSatInfo[0];
	int ssSatInfo = sizeof(sSatInfo) - 1;
	bool satinview = nmea_INFO_is_present(pack->present, SATINVIEW);
	int i;

	sCount[0] = 0;
	sIndex[0] = 0;
	sSatCount[0] = 0;
	sSatInfo[0] = 0;

	if (satinview) {
		snprintf(&sCount[0], sizeof(sCount), "%1d", pack->pack_count);
		snprintf(&sIndex[0], sizeof(sIndex), "%1d", pack->pack_index);
		snprintf(&sSatCount[0], sizeof(sSatCount), "%02d", pack->sat_count);
	}
	for (i = 0; i < NMEA_SATINPACK; i++) {
		int cnt = 0;
		if (satinview && pack->sat_data[i].id) {
			cnt = snprintf(psSatInfo, ssSatInfo, "%02d,%02d,%03d,%02d", pack->sat_data[i].id, pack->sat_data[i].elv,
					pack->sat_data[i].azimuth, pack->sat_data[i].sig);
		} else {
			cnt = snprintf(psSatInfo, ssSatInfo, ",,,");
		}
		if (cnt >= ssSatInfo) {
			ssSatInfo = 0;
			psSatInfo = &sSatInfo[sizeof(sSatInfo) - 1];
			*psSatInfo = '\0';
			break;
		} else {
			ssSatInfo -= cnt;
			psSatInfo += cnt;
		}
		if (i < (NMEA_SATINPACK - 1)) {
			*psSatInfo = ',';
			psSatInfo++;
			ssSatInfo--;
			*psSatInfo = '\0';
		}
	}

	return nmea_printf(s, len, "$GPGSV,%s,%s,%s,%s", &sCount[0], &sIndex[0], &sSatCount[0], &sSatInfo[0]);
}

/**
 * Generate a GPRMC sentence from an nmeaGPRMC structure
 *
 * @param s a pointer to the buffer to generate the string in
 * @param len the size of the buffer
 * @param pack the structure
 * @return the length of the generated sentence
 */
int nmea_gen_GPRMC(char *s, const int len, const nmeaGPRMC *pack) {
	char sTime[16];
	char sDate[16];
	char sLat[16];
	char sNs[2];
	char sLon[16];
	char sEw[2];
	char sSpeed[16];
	char sTrack[16];
	char sMagvar[16];
	char sMagvar_ew[2];

	sTime[0] = 0;
	sDate[0] = 0;
	sLat[0] = 0;
	sNs[0] = sNs[1] = 0;
	sLon[0] = 0;
	sEw[0] = sEw[1] = 0;
	sSpeed[0] = 0;
	sTrack[0] = 0;
	sMagvar[0] = 0;
	sMagvar_ew[0] = sMagvar_ew[1] = 0;

	if (nmea_INFO_is_present(pack->present, UTCDATE)) {
		snprintf(&sDate[0], sizeof(sDate), "%02d%02d%02d", pack->utc.day, pack->utc.mon + 1, pack->utc.year - 100);
	}
	if (nmea_INFO_is_present(pack->present, UTCTIME)) {
		snprintf(&sTime[0], sizeof(sTime), "%02d%02d%02d.%02d", pack->utc.hour, pack->utc.min, pack->utc.sec,
				pack->utc.hsec);
	}
	if (nmea_INFO_is_present(pack->present, LAT)) {
		snprintf(&sLat[0], sizeof(sLat), "%09.4f", pack->lat);
		sNs[0] = pack->ns;
	}
	if (nmea_INFO_is_present(pack->present, LON)) {
		snprintf(&sLon[0], sizeof(sLon), "%010.4f", pack->lon);
		sEw[0] = pack->ew;
	}
	if (nmea_INFO_is_present(pack->present, SPEED)) {
		snprintf(&sSpeed[0], sizeof(sSpeed), "%03.1f", pack->speed);
	}
	if (nmea_INFO_is_present(pack->present, TRACK)) {
		snprintf(&sTrack[0], sizeof(sTrack), "%03.1f", pack->track);
	}
	if (nmea_INFO_is_present(pack->present, MAGVAR)) {
		snprintf(&sMagvar[0], sizeof(sMagvar), "%03.1f", pack->magvar);
		sMagvar_ew[0] = pack->magvar_ew;
	}

	return nmea_printf(s, len, "$GPRMC,%s,%c,%s,%s,%s,%s,%s,%s,%s,%s,%s,%c", &sTime[0], pack->status, &sLat[0], &sNs[0],
			&sLon[0], &sEw[0], &sSpeed[0], &sTrack[0], &sDate[0], &sMagvar[0], &sMagvar_ew[0], pack->mode);
}

/**
 * Generate a GPVTG sentence from an nmeaGPVTG structure
 *
 * @param s a pointer to the buffer to generate the string in
 * @param len the size of the buffer
 * @param pack the structure
 * @return the length of the generated sentence
 */
int nmea_gen_GPVTG(char *s, const int len, const nmeaGPVTG *pack) {
	char sTrackT[16];
	char sTrackM[16];
	char sSpeedN[16];
	char sSpeedK[16];
	char sUnitT[2];
	char sUnitM[2];
	char sUnitN[2];
	char sUnitK[2];

	sTrackT[0] = 0;
	sTrackM[0] = 0;
	sSpeedN[0] = 0;
	sSpeedK[0] = 0;
	sUnitT[0] = sUnitT[1] = 0;
	sUnitM[0] = sUnitM[1] = 0;
	sUnitN[0] = sUnitN[1] = 0;
	sUnitK[0] = sUnitK[1] = 0;

	if (nmea_INFO_is_present(pack->present, TRACK)) {
		snprintf(&sTrackT[0], sizeof(sTrackT), "%03.1f", pack->track);
		sUnitT[0] = 'T';
	}
	if (nmea_INFO_is_present(pack->present, MTRACK)) {
		snprintf(&sTrackM[0], sizeof(sTrackM), "%03.1f", pack->mtrack);
		sUnitM[0] = 'M';
	}
	if (nmea_INFO_is_present(pack->present, SPEED)) {
		snprintf(&sSpeedN[0], sizeof(sSpeedN), "%03.1f", pack->spn);
		sUnitN[0] = 'N';
		snprintf(&sSpeedK[0], sizeof(sSpeedK), "%03.1f", pack->spk);
		sUnitK[0] = 'K';
	}

	return nmea_printf(s, len, "$GPVTG,%s,%s,%s,%s,%s,%s,%s,%s", &sTrackT[0], &sUnitT[0], &sTrackM[0],
			&sUnitM[0], &sSpeedN[0], &sUnitN[0], &sSpeedK[0], &sUnitK[0]);
}

/**
 * Generate a number of sentences from an nmeaINFO structure.
 *
 * @param s a pointer to the buffer in which to generate the sentences
 * @param len the size of the buffer
 * @param info the structure
 * @param generate_mask the mask of which sentences to generate
 * @return the total length of the generated sentences
 */
int nmea_generate(char *s, const int len, const nmeaINFO *info, const int generate_mask) {
	int gen_count = 0;
	int pack_mask = generate_mask;

	if (!s || !len || !info || !generate_mask)
		return 0;

	while (pack_mask) {
		if (pack_mask & GPGGA) {
			nmeaGPGGA gga;

			nmea_info2GPGGA(info, &gga);
			gen_count += nmea_gen_GPGGA(s + gen_count, len - gen_count, &gga);
			pack_mask &= ~GPGGA;
		} else if (pack_mask & GPGSA) {
			nmeaGPGSA gsa;

			nmea_info2GPGSA(info, &gsa);
			gen_count += nmea_gen_GPGSA(s + gen_count, len - gen_count, &gsa);
			pack_mask &= ~GPGSA;
		} else if (pack_mask & GPGSV) {
			nmeaGPGSV gsv;
			int gsv_it;
			int gsv_count = nmea_gsv_npack(info->satinfo.inview);

			for (gsv_it = 0; gsv_it < gsv_count && len - gen_count > 0; gsv_it++) {
				nmea_info2GPGSV(info, &gsv, gsv_it);
				gen_count += nmea_gen_GPGSV(s + gen_count, len - gen_count, &gsv);
			}
			pack_mask &= ~GPGSV;
		} else if (pack_mask & GPRMC) {
			nmeaGPRMC rmc;

			nmea_info2GPRMC(info, &rmc);
			gen_count += nmea_gen_GPRMC(s + gen_count, len - gen_count, &rmc);
			pack_mask &= ~GPRMC;
		} else if (pack_mask & GPVTG) {
			nmeaGPVTG vtg;

			nmea_info2GPVTG(info, &vtg);
			gen_count += nmea_gen_GPVTG(s + gen_count, len - gen_count, &vtg);
			pack_mask &= ~GPVTG;
		} else {
			/* no more known sentences to process */
			break;
		}

		if (len - gen_count <= 0)
			break;
	}

	return gen_count;
}
