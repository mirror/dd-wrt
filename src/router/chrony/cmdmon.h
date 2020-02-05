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

  Header file for the control and monitoring module in the software
  */

#ifndef GOT_CMDMON_H
#define GOT_CMDMON_H

#include "addressing.h"

extern void CAM_Initialise(int family);

extern void CAM_Finalise(void);

extern void CAM_OpenUnixSocket(void);
extern int CAM_AddAccessRestriction(IPAddr *ip_addr, int subnet_bits, int allow, int all);
extern int CAM_CheckAccessRestriction(IPAddr *ip_addr);

#endif /* GOT_CMDMON_H */
