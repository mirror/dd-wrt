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

#include <nmea/sentence.h>

#include <string.h>

void nmea_zero_GPGGA(nmeaGPGGA *pack) {
	memset(pack, 0, sizeof(nmeaGPGGA));
	nmea_time_now(&pack->utc, &pack->present);
	pack->ns = 'N';
	pack->ew = 'E';
	pack->elv_units = 'M';
	pack->diff_units = 'M';
}

void nmea_zero_GPGSA(nmeaGPGSA *pack) {
	memset(pack, 0, sizeof(nmeaGPGSA));
	pack->fix_mode = 'A';
	pack->fix_type = NMEA_FIX_BAD;
}

void nmea_zero_GPGSV(nmeaGPGSV *pack) {
	memset(pack, 0, sizeof(nmeaGPGSV));
}

void nmea_zero_GPRMC(nmeaGPRMC *pack) {
	memset(pack, 0, sizeof(nmeaGPRMC));
	nmea_time_now(&pack->utc, &pack->present);
	pack->status = 'V';
	pack->ns = 'N';
	pack->ew = 'E';
	pack->magvar_ew = 'E';
	pack->mode = 'N';
}

void nmea_zero_GPVTG(nmeaGPVTG *pack) {
	memset(pack, 0, sizeof(nmeaGPVTG));
	pack->track_t = 'T';
	pack->mtrack_m = 'M';
	pack->spn_n = 'N';
	pack->spk_k = 'K';
}
