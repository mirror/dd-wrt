/*
 * sputnik.c
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
#ifdef HAVE_SPUTNIK_APD
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>

/*
 * Sputnik APD Service Handling 
 */
int start_sputnik( void )
{
    int ret;

    // Only start if enabled
    if( !nvram_invmatch( "apd_enable", "0" ) )
	return 0;
    insmod( "ipt_mark" );
    insmod( "ipt_mac" );
    insmod( "xt_mark" );
    insmod( "xt_mac" );

    ret = eval( "sputnik" );
    dd_syslog( LOG_INFO, "sputnik : sputnik daemon successfully started\n" );
    cprintf( "done\n" );
    return ret;
}

int stop_sputnik( void )
{
    int ret = 0;

    if( pidof( "sputnik" ) > 0 )
    {
	dd_syslog( LOG_INFO,
		   "sputnik : sputnik daemon successfully stopped\n" );
	ret = killall( "sputnik", SIGTERM );

	cprintf( "done\n" );
    }
    return ret;
}

/*
 * END Sputnik Service Handling 
 */

#endif
