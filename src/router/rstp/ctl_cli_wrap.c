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

#include "ctl_functions.h"
#include "ctl_socket.h"
#include "ctl_socket_client.h"
#include "log.h"

CLIENT_SIDE_FUNCTION(enable_bridge_rstp)
CLIENT_SIDE_FUNCTION(get_bridge_status)
CLIENT_SIDE_FUNCTION(set_bridge_config)
CLIENT_SIDE_FUNCTION(get_port_status)
CLIENT_SIDE_FUNCTION(set_port_config)
CLIENT_SIDE_FUNCTION(port_mcheck)
CLIENT_SIDE_FUNCTION(set_debug_level)

void Dprintf(int level, const char *fmt, ...)
{
	char logbuf[256];
	logbuf[sizeof(logbuf) - 1] = 0;
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(logbuf, sizeof(logbuf) - 1, fmt, ap);
	va_end(ap);
	printf("%s\n", logbuf);
}
