/*
 * stabridge.c
 *
 * Copyright (C) 2006 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <net/route.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/sysinfo.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <rc.h>
#include <code_pattern.h>
#include <cy_conf.h>
#include <utils.h>

void start_stabridge( void )
{

    if( getWET(  ) )
    {
	insmod( "ebtables" );
	insmod( "ebtables" );
	insmod( "ebtable_filter" );
	insmod( "ebtable_nat" );
	insmod( "ebtable_broute" );
	insmod( "ebt_arpnat" );
	insmod( "ebt_broute" );
	eval( "ebtables", "-t", "nat", "-A", "PREROUTING", "--in-interface",
	      getWET(  ), "-j", "arpnat", "--arpnat-target", "ACCEPT" );
	eval( "ebtables", "-t", "nat", "-A", "POSTROUTING", "--out-interface",
	      getWET(  ), "-j", "arpnat", "--arpnat-target", "ACCEPT" );
	eval( "ebtables", "-t", "broute", "-A", "BROUTING", "--protocol",
	      "0x888e", "--in-interface", getWET(  ), "-j", "DROP" );
    }
}

void stop_stabridge( void )
{
    if( getWET(  ) )
    {
	eval( "ebtables", "-t", "broute", "-D", "BROUTING", "--protocol",
	      "0x888e", "--in-interface", getWET(  ), "-j", "DROP" );
	eval( "ebtables", "-t", "nat", "-D", "POSTROUTING", "--out-interface",
	      getWET(  ), "-j", "arpnat", "--arpnat-target", "ACCEPT" );
	eval( "ebtables", "-t", "nat", "-D", "PREROUTING", "--in-interface",
	      getWET(  ), "-j", "arpnat", "--arpnat-target", "ACCEPT" );
    }
    // flush the tables, since getWET will not find the interface
    // in the nvram (if changed from client-bridge to whatever)
    // Fix, cause the rmmod command does not
    // remove the modules (..if rules are in?).
    eval( "ebtables", "-t", "broute", "-F" );
    eval( "ebtables", "-t", "nat", "-F" );
    rmmod( "ebt_broute" );
    rmmod( "ebt_arpnat" );
    rmmod( "ebtable_broute" );
    rmmod( "ebtable_nat" );
    rmmod( "ebtable_filter" );
    rmmod( "ebtables" );
}
