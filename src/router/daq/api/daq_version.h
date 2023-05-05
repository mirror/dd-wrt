/*
** Copyright (C) 2016 Sourcefire, Inc.
** Author: Michael R. Altizer <mialtize@cisco.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef _DAQ_VERSION_H
#define _DAQ_VERSION_H

#define DAQ_VERSION_MAJOR 2
#define DAQ_VERSION_MINOR 2
#define DAQ_VERSION_PATCH 2

#define DAQ_VERSION_NUMERIC ((DAQ_VERSION_MAJOR << 24) | (DAQ_VERSION_MINOR << 16) | (DAQ_VERSION_PATCH << 8) | 0)

#define DAQ_VERSION_STRING_C_(major, minor, patch) \
            #major "." #minor "." #patch

#define DAQ_VERSION_STRING_C(major, minor, patch) \
            DAQ_VERSION_STRING_C_(major, minor, patch)

#define DAQ_VERSION_STRING DAQ_VERSION_STRING_C( \
                DAQ_VERSION_MAJOR, DAQ_VERSION_MINOR, \
                DAQ_VERSION_PATCH)

#endif /* _DAQ_VERSION_H */
