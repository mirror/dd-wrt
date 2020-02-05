/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2002
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  Header for pktlength.c, routines for working out the expected length
  of a network command/reply packet.

  */

#ifndef GOT_PKTLENGTH_H
#define GOT_PKTLENGTH_H

#include "candm.h"

extern int PKL_CommandLength(CMD_Request *r);

extern int PKL_CommandPaddingLength(CMD_Request *r);

extern int PKL_ReplyLength(CMD_Reply *r);

#endif /* GOT_PKTLENGTH_H */
