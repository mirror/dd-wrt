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

#include <nmea/time.h>

#include <stddef.h>
#include <time.h>
#include <sys/time.h>

void nmea_time_now(nmeaTIME *stm) {
	struct timeval tp;
	struct tm tt;

	gettimeofday(&tp, NULL);
	gmtime_r(&tp.tv_sec, &tt);

	stm->year = tt.tm_year;
	stm->mon = tt.tm_mon;
	stm->day = tt.tm_mday;
	stm->hour = tt.tm_hour;
	stm->min = tt.tm_min;
	stm->sec = tt.tm_sec;
	stm->hsec = (tp.tv_usec / 10000);
}
