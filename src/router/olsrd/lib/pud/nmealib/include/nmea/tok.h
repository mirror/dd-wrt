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

#ifndef __NMEA_TOK_H__
#define __NMEA_TOK_H__

#ifdef  __cplusplus
extern "C" {
#endif

int nmea_calc_crc(const char *buff, int buff_sz);
int nmea_atoi(const char *str, int str_sz, int radix);
double nmea_atof(const char *str, int str_sz);
int nmea_printf(char *buff, int buff_sz, const char *format, ...) __attribute__ ((format(printf, 3, 4)));
int nmea_scanf(const char *buff, int buff_sz, const char *format, ...);

#ifdef  __cplusplus
}
#endif

#endif /* __NMEA_TOK_H__ */
