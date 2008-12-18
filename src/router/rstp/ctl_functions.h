/*****************************************************************************
  Copyright (c) 2006 EMC Corporation.

  This program is free software; you can redistribute it and/or modify it 
  under the terms of the GNU General Public License as published by the Free 
  Software Foundation; either version 2 of the License, or (at your option) 
  any later version.
  
  This program is distributed in the hope that it will be useful, but WITHOUT 
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
  more details.
  
  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59 
  Temple Place - Suite 330, Boston, MA  02111-1307, USA.
  
  The full GNU General Public License is included in this distribution in the
  file called LICENSE.

  Authors: Srinivas Aji <Aji_Srinivas@emc.com>

******************************************************************************/

#ifndef CTL_FUNCTIONS_H
#define CTL_FUNCTIONS_H

#include "rstp.h"

int CTL_enable_bridge_rstp(int br_index, int enable);

int CTL_get_bridge_status(int br_index, STP_BridgeStatus *status);

int CTL_set_bridge_config(int br_index,  STP_BridgeConfig *cfg);

int CTL_get_port_status(int br_index, int port_index, STP_PortStatus *status);

int CTL_set_port_config(int br_index, int port_index, STP_PortConfig *cfg);

int CTL_port_mcheck(int br_index, int port_index);

int CTL_set_debug_level(int level);

#endif
