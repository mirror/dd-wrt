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

#ifndef __NMEA_UNITS_H__
#define __NMEA_UNITS_H__

/*
 * Distance units
 */

#define NMEA_TUD_YARDS (1.0936) /**< Yards, meter * NMEA_TUD_YARDS = yard */
#define NMEA_TUD_KNOTS (1.852)  /**< Knots, kilometer / NMEA_TUD_KNOTS = knot */
#define NMEA_TUD_MILES (1.609)  /**< Miles, kilometer / NMEA_TUD_MILES = mile */

/*
 * Speed units
 */

#define NMEA_TUS_MS    (3.6)    /**< Meters per seconds, (k/h) / NMEA_TUS_MS= (m/s) */

#endif /* __NMEA_UNITS_H__ */
