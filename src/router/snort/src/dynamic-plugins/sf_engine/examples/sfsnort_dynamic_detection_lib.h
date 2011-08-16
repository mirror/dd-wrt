/****************************************************************************
 *
 * Copyright (C) 2005-2011 Sourcefire, Inc.
 *
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/
#ifndef SFSNORT_DYNAMIC_DETECTION_LIB_H_
#define SFSNORT_DYNAMIC_DETECTION_LIB_H_

#ifdef WIN32
#ifdef SF_SNORT_DETECTION_DLL
#define BUILDING_SO
#define DETECTION_LINKAGE SO_PUBLIC
#else
#define DETECTION_LINKAGE
#endif
#else /* WIN32 */
#define DETECTION_LINKAGE SO_PUBLIC
#endif

#endif /* SFSNORT_DYNAMIC_DETECTION_LIB_H_ */

