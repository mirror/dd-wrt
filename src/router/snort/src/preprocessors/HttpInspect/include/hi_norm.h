/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2003-2013 Sourcefire, Inc.
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
 ****************************************************************************/
 
/**
**  @file       hi_norm.h
**
**  @author     Daniel Roelker <droelker@sourcefire.com>
**  
**  @brief      Contains function prototypes for normalization routines.
**  
**  Contains stuctures and headers for normalization routines.
**  
**  NOTES:
**      - Initial development.  DJR
*/
#ifndef __HI_NORM_H__
#define __HI_NORM_H__

#include <sys/types.h>

#include "hi_include.h"
#include "hi_ui_config.h"
#include "hi_si.h"

int hi_normalization(HI_SESSION *Session, int iInspectMode, HttpSessionData *hsd);
int hi_norm_uri(HI_SESSION *Session, u_char *uribuf,int *uribuf_size,
                const u_char *uri, int uri_size, uint16_t *encodeType);
unsigned int NormalizeRandomNulls (unsigned char *src, unsigned int src_len, unsigned char *dst);

#endif
