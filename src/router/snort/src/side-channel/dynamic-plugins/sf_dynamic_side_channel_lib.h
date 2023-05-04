/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2012-2013 Sourcefire, Inc.
 *
 * Author: Michael Altizer <maltizer@sourcefire.com>
 *
 */

#ifndef SFSNORT_DYNAMIC_SIDE_CHANNEL_LIB_H_
#define SFSNORT_DYNAMIC_SIDE_CHANNEL_LIB_H_

#ifdef WIN32
#ifdef SF_SNORT_SIDE_CHANNEL_DLL
#define BUILDING_SO
#define SIDE_CHANNEL_LINKAGE SF_SO_PUBLIC
#else
#define SIDE_CHANNEL_LINKAGE
#endif
#else /* WIN32 */
#define SIDE_CHANNEL_LINKAGE SF_SO_PUBLIC
#endif

#endif /* SFSNORT_DYNAMIC_SIDE_CHANNEL_LIB_H_ */
