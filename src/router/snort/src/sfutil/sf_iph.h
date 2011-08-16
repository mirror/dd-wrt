/*  
** Copyright (C) 2007-2011 Sourcefire, Inc.
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
** along with this program; if nto, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Bosotn, MA 02111-1307, USA.
*/

#ifndef SFIPH_H
#define SFIPH_H

#ifdef SUP_IP6
void sfiph_build(Packet *p, const void *hdr, int family);
void sfiph_orig_build(Packet *p, const void *hdr, int family);
#endif

#endif
