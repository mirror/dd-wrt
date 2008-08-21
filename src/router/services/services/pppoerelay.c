/*
 * pppoerelay.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */
#ifdef HAVE_PPPOERELAY
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
void start_pppoerelay( void )
{
    killall( "pppoe-relay", SIGTERM );
    if( nvram_match( "pppoerelay_enable", "1" ) )
    {
	if( getSTA(  ) )
	    eval( "pppoe-relay", "-S", getSTA(  ), "-C", "br0" );
	else
	    eval( "pppoe-relay", "-S", nvram_safe_get( "wan_ifname" ), "-C",
		  "br0" );

	syslog( LOG_INFO, "pppoe-relay successfully started\n" );
    }
}
void stop_pppoerelay( void )
{
    if( pidof( "pppoe-relay" ) > 0 )
    {
	syslog( LOG_INFO, "pppoe-relay successfully stopped\n" );
	killall( "pppoe-relay", SIGTERM );
    }
}
#endif
