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
 
/*
**  sfmemcap.h
*/
#ifndef __SF_MEMCAP_H__
#define __SF_MEMCAP_H__

typedef struct
{
   unsigned memused;
   unsigned memcap;
   int      nblocks;

}MEMCAP;

void     sfmemcap_init(MEMCAP * mc, unsigned nbytes);
MEMCAP * sfmemcap_new( unsigned nbytes );
void     sfmemcap_delete( MEMCAP * mc );
void   * sfmemcap_alloc(MEMCAP * mc, unsigned nbytes);
void     sfmemcap_showmem(MEMCAP * mc );
void     sfmemcap_free( MEMCAP * mc, void * memory);
char   * sfmemcap_strdup(MEMCAP * mc, const char *str);
void   * sfmemcap_dupmem(MEMCAP * mc, void * src, int n );

#endif
