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

#ifndef __NMEA_GENERATE_H__
#define __NMEA_GENERATE_H__

#include <nmea/info.h>
#include <nmea/sentence.h>

#ifdef  __cplusplus
extern "C" {
#endif

int nmea_generate(char *buff, int buff_sz, const nmeaINFO *info,
		int generate_mask);

int nmea_gen_GPGGA(char *buff, int buff_sz, nmeaGPGGA *pack);
int nmea_gen_GPGSA(char *buff, int buff_sz, nmeaGPGSA *pack);
int nmea_gen_GPGSV(char *buff, int buff_sz, nmeaGPGSV *pack);
int nmea_gen_GPRMC(char *buff, int buff_sz, nmeaGPRMC *pack);
int nmea_gen_GPVTG(char *buff, int buff_sz, nmeaGPVTG *pack);

void nmea_info2GPGGA(const nmeaINFO *info, nmeaGPGGA *pack);
void nmea_info2GPGSA(const nmeaINFO *info, nmeaGPGSA *pack);
void nmea_info2GPRMC(const nmeaINFO *info, nmeaGPRMC *pack);
void nmea_info2GPVTG(const nmeaINFO *info, nmeaGPVTG *pack);

int nmea_gsv_npack(int sat_count);
void nmea_info2GPGSV(const nmeaINFO *info, nmeaGPGSV *pack, int pack_idx);

#ifdef  __cplusplus
}
#endif

#endif /* __NMEA_GENERATE_H__ */
