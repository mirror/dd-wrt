/*
 * sf_preproc_info.h
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2006-2013 Sourcefire, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Description:
 *
 * This file is part of the dynamically loadable preprocessor library.  The
 * items must be globally defined within the source file of a given
 * preprocessor.
 *
 * Author: Steven A. Sturges <ssturges@sourcefire.com>
 *
 * NOTES:
 *
 */
#ifndef SF_PREPROC_INFO_H_
#define SF_PREPROC_INFO_H_

extern const int MAJOR_VERSION;
extern const int MINOR_VERSION;
extern const int BUILD_VERSION;
extern const char *PREPROC_NAME;

extern void DYNAMIC_PREPROC_SETUP(void);

#endif /* SF_PREPROC_INFO_H_ */

