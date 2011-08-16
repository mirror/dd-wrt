/****************************************************************************
 *
 * Copyright (C) 2003-2011 Sourcefire, Inc.
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
 
/**
**  @file           hi_hbm.h
**  
**  @author         Marc Norton <mnorton@sourcefire.com>
**  
**  @brief          Header file for Horspool type Boyer-Moore implementation
*/
#ifndef __HI_HBM_H__
#define __HI_HBM_H__

typedef struct {

 unsigned char *P;
 int            M;
 short          bcShift[256];

}HBM_STRUCT;

HBM_STRUCT * hbm_prep(unsigned char * pat, int m);
unsigned char * hbm_match(HBM_STRUCT * px, unsigned char *text, int n);

#endif
