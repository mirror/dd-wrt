/****************************************************************************
 * Copyright (C) 2006-2011 Sourcefire, Inc.
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
 ****************************************************************************
 * Description:
 *
 * Standard dynamic preprocessor include file.
 * 
 ****************************************************************************/

#ifndef SF_PREPROC_INFO_H
#define SF_PREPROC_INFO_H

#define MAJOR_VERSION   1
#define MINOR_VERSION   0
#define BUILD_VERSION   3
#ifdef SUP_IP6
#define PREPROC_NAME    "SF_DCERPC2 (IPV6)"
#else
#define PREPROC_NAME    "SF_DCERPC2"
#endif

#define DYNAMIC_PREPROC_SETUP DCE2_RegisterPreprocessor
extern void DCE2_RegisterPreprocessor(void);

#endif /* SF_PREPROC_INFO_H */

