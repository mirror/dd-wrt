/*
 * bmh.h
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
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
 *
 * Author: Marc Norton
 *
 * Date: 5/2005
 *
 * Boyer-Moore-Horsepool for small pattern groups
 *
 */
#ifndef BOYER_MOORE_HORSPOOL
#define BOYER_MOORE_HORSPOOL

#define HBM_STATIC 

typedef struct {

 unsigned char *P;
 unsigned char *Pnc;
 int            M;
 int            bcShift[256];
 int            nocase;
}HBM_STRUCT;



HBM_STATIC HBM_STRUCT    * hbm_prep( unsigned char * pat, int m, int nocase );
HBM_STATIC int hbm_prepx( HBM_STRUCT *p, unsigned char * pat, int m, int nocase );
HBM_STATIC const unsigned char * hbm_match( HBM_STRUCT *p, const unsigned char * text, int n );
HBM_STATIC void            hbm_free( HBM_STRUCT *p );

#endif
