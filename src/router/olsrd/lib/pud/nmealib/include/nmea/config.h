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

#ifndef __NMEA_CONFIG_H__
#define __NMEA_CONFIG_H__

#define NMEA_POSIX(x)  (x)
#define NMEA_INLINE    inline

#if !defined(NDEBUG)
#include <assert.h>
#define NMEA_ASSERT(x)   assert(x)
#else
#define NMEA_ASSERT(x)
#endif

#endif /* __NMEA_CONFIG_H__ */
