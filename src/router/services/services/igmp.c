/*
 * igmp.c
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
#ifdef HAVE_MULTICAST
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>

void stop_igmp_proxy( void );
void start_igmp_proxy( void )
{
    int ret = 0;
    pid_t pid;
    char name[80], *next, *svbuf;
    char *argv[] = { "igmprt", NULL };

    int ifcount = 0;

/*
    if (nvram_match("dtag_vlan8","1"))
	{
	FILE *fp = fopen( "/tmp/igmpproxy_tv.conf", "wb" );
	fprintf( fp, "quickleave\nphyint %s upstream  ratelimit 0  threshold 1\n",nvram_safe_get( "tvnicfrom" ) );
	fprintf( fp, "phyint %s downstream  ratelimit 0  threshold 1\n",nvram_safe_get( "lan_ifname")); 
	char ifnames[256];
	getIfLists( ifnames, 256 );
	foreach( name, ifnames, next )
	{
	    if (!nvram_match("tvnicfrom",name) && !nvram_match("lan_ifname",name))
		fprintf( fp, "phyint %s disabled\n", name );
	}
	fprintf( fp, "phyint lo disabled\n" );
	fclose(fp);
	eval("igmprt","-c","/tmp/igmpproxy_tv.conf");
	return;
	}*/
    FILE *fp = fopen( "/tmp/igmpproxy.conf", "wb" );

    if (nvram_match("dtag_vlan8","1"))
	{
	fprintf( fp, "quickleave\nphyint %s upstream  ratelimit 0  threshold 1\n",nvram_safe_get( "tvnicfrom" ) );
	}
	else
	{
        fprintf( fp, "quickleave\nphyint %s upstream  ratelimit 0  threshold 1\n",get_wan_face() );
	}
    if( nvram_match( "block_multicast", "0" ) )
    {
	fprintf( fp, "phyint %s downstream  ratelimit 0  threshold 1\n",
		 nvram_safe_get( "lan_ifname" ) );
	ifcount++;
    }
    else
    {
	fprintf( fp, "phyint %s disabled\n"
		 "phyint %s:0 disabled\n", nvram_safe_get( "lan_ifname" ),
		 nvram_safe_get( "lan_ifname" ) );
    }
    char ifnames[256];

    getIfLists( ifnames, 256 );
    foreach( name, ifnames, next )
    {
	if( strcmp( get_wan_face(  ), name )
	    && strcmp( nvram_safe_get( "lan_ifname" ), name ) && strcmp(nvram_safe_get("tvnicfrom"),name))
	{
	    if( nvram_nmatch( "0", "%s_bridged", name )
		&& nvram_nmatch( "1", "%s_multicast", name ) )
	    {
		fprintf( fp,
			 "phyint %s downstream  ratelimit 0  threshold 1\n",
			 name );
		ifcount++;
	    }
	    else
		fprintf( fp, "phyint %s disabled\n", name );
	}
    }
    fprintf( fp, "phyint lo disabled\n" );
    fclose( fp );
    if( nvram_match( "wan_proto", "disabled" ) )	// todo: add upstream 
	// config
    {
	// ret = _evalpid (igmp_proxybr_argv, NULL, 0, &pid);
	return;
    }
    else
    {
	if( ifcount )
	{
	    if( pidof( "igmprt" ) < 1)
		ret = _evalpid( argv, NULL, 0, &pid );
		dd_syslog( LOG_INFO,
		       "igmprt : multicast daemon successfully started\n" );
	}
    }

    cprintf( "done\n" );
    return;
}

void stop_igmp_proxy( void )
{
    int ret = 0;

    if( pidof( "igmprt" ) > 0 )
    {
	syslog( LOG_INFO,
		"igmprt : multicast daemon successfully stopped\n" );
	ret = killall( "igmprt", SIGKILL );
    }
    cprintf( "done\n" );
    return;
}
#endif
