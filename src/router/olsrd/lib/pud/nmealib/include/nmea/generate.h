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

#ifndef __NMEA_GENERATE_H__
#define __NMEA_GENERATE_H__

#include <nmea/sentence.h>

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

int nmea_gen_GPGGA(char *s, const int len, const nmeaGPGGA *pack);
int nmea_gen_GPGSA(char *s, const int len, const nmeaGPGSA *pack);
int nmea_gen_GPGSV(char *s, const int len, const nmeaGPGSV *pack);
int nmea_gen_GPRMC(char *s, const int len, const nmeaGPRMC *pack);
int nmea_gen_GPVTG(char *s, const int len, const nmeaGPVTG *pack);

int nmea_generate(char *s, const int len, const nmeaINFO *info, const int generate_mask);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEA_GENERATE_H__ */
