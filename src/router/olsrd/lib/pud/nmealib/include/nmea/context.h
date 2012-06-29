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

#ifndef __NMEA_CONTEXT_H__
#define __NMEA_CONTEXT_H__

#define NMEA_DEF_PARSEBUFF  (1024)
#define NMEA_MIN_PARSEBUFF  (256)

#ifdef  __cplusplus
extern "C" {
#endif

typedef void (*nmeaTraceFunc)(const char *str, int str_size);
typedef void (*nmeaErrorFunc)(const char *str, int str_size);

typedef struct _nmeaPROPERTY {
	nmeaTraceFunc trace_func;
	nmeaErrorFunc error_func;
	int parse_buff_size;

} nmeaPROPERTY;

nmeaPROPERTY * nmea_property(void);

void nmea_trace(const char *str, ...) __attribute__ ((format(printf, 1, 2)));
void nmea_trace_buff(const char *buff, int buff_size);
void nmea_error(const char *str, ...) __attribute__ ((format(printf, 1, 2)));

#ifdef  __cplusplus
}
#endif

#endif /* __NMEA_CONTEXT_H__ */
