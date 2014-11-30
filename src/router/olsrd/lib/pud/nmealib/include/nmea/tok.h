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

#ifndef __NMEA_TOK_H__
#define __NMEA_TOK_H__

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

int nmea_calc_crc(const char *s, const int len);
int nmea_atoi(const char *s, const int len, const int radix);
double nmea_atof(const char *s, const int len);
int nmea_printf(char *s, int len, const char *format, ...) __attribute__ ((format(printf, 3, 4)));
int nmea_scanf(const char *s, int len, const char *format, ...);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEA_TOK_H__ */
